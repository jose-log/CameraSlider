
/*
* Motor module. 
* This module includes all functions encharged to perform all the motor 
* movement variations. It controls its position, speed and acceleration.
* Some mathematical processing is required to build the speed profiles.
*
* For a full understanding of this module, refer to the David Austin paper
* titled "Generate stepper-motor speed profiles in real time". The paper
* describes the algorithm and its foundation. This implementation is based on
* Interrupt Service Routines, which I think presents some advantages with
* respect to the popular AccelStepper Arduino library, which is based on
* polling to achieve smooth movements.
*/

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "motor.h"

#include <stdlib.h>
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/atomic.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

#define SPEED_UP	0xF1
#define SPEED_FLAT	0xF2
#define SPEED_DOWN	0xF3
#define SPEED_HALT 	0xF0

#define CMIN_EIGHTH_STEPPING 	249.0

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

static float cn;
static float c0;
static float cmin;
static float c_target;
static uint16_t n;
volatile static uint8_t state;		// state variable

volatile static int32_t current_pos;
volatile static int32_t target_pos;
volatile static uint8_t dir;

static int32_t queue_pos;
static int8_t queue_speed;
static uint8_t queue_full;

static uint8_t speed_stop;
static uint8_t speed_profile;
static uint8_t ctl;

static const float f = F_MOTOR;

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void compute_c_position(void);
static void pulse(void);
static void queue_position_motion(int32_t p);
static void queue_speed_motion(int8_t s);
static float get_cmin(uint8_t percent);
static void next_cn(void);

/*===========================================================================*/
/*
* Sets initial motor parameters.
* Sets stepping mode as EIGHTH STEPPING. Even though any other stepping mode
* can be selected, all subsequent calculations are based on this stepping mode.
*
* Eighth stepping mode was chosen as a compromise between some factors:
* - pros:
*	- smoother movements at low speeds
*	- less motor noise (paramount for video recording)
* 	- reasonable max/min speed, 
*	- reasonable acceleration
* - cons:
*	- less time for the CPU between interrupts (i.e. for calculations)
*	- limited max speed
* 	- way more heat dissipation in the driver MOSFETS (bigger heatsink required)
*/
void motor_init(void)
{
	drv_reset();
	drv_step_mode(MODE_EIGHTH_STEP);
	drv_dir(CW, &dir);
	
	current_pos = 0;
	target_pos = 0;

	// minimum counter value to get max speed
	cmin = CMIN_EIGHTH_STEPPING;
	motor_set_speed_profile(PROFILE_LINEAR);
	cn = c0;
	n = 0;
	state = SPEED_HALT;
	queue_full = FALSE;
}

/*===========================================================================*/
/*
* Sets speed profile
* 	- linear profile
*	- quadratic profile
* Any change in speed profile requires recalculation of c0, which is performed
* within motor_set_accel_percent(). Thus, notice that after every speed profile
* change, acceleration is always maximum
*/
void motor_set_speed_profile(uint8_t p)
{
	if (p == PROFILE_LINEAR) {
		speed_profile = PROFILE_LINEAR;
	} else if (p == PROFILE_QUADRATIC) {
		speed_profile = PROFILE_QUADRATIC;
	}
	// whenever the speed profile is chose, maximum acceleration is chosen
	motor_set_accel_percent(100);
}

/*===========================================================================*/
/*
* Max speed is mechanically constrained: By testing, higher speeds provoke
* the motor to run out of sync
* Min speed is limited by the resolution of the percent given (which is an
* integer), thus, 0% is the minimum (but doesn't mean zero speed)
*/
int8_t motor_set_maxspeed_percent(uint8_t speed) 
{
	// check for a valid value and state
	// Updates cannot happen while motor is moving!
	if ((speed > 100) || (speed == 0) || (state != SPEED_HALT)) return -1;

	float a, b;

	a = (float)speed;
	a = a / 100.0;			// percentage
	b = SPEED_MAX * a;		// Fraction of max speed

	motor_set_maxspeed(b);

	return 0;
}

/*===========================================================================*/
/*
* Speed is controlled through the timer compare register value. 
* The function makes sure not to select a speed higher than the highest 
* possible by making sure that the timer compare register value may not 
* overflow.
*/
int8_t motor_set_maxspeed(float speed)
{
/*
* Check for minimum speed: 
* 	- max cmin: 2^16 - 1
*	- speed min: f_timer / (max_cmin + 1) = 30,52Hz
*/
	if (speed > SPEED_MIN)
		cmin = (f / speed) - 1.0;
	else
		cmin = CMIN_MAX;

	return 0;
}

/*===========================================================================*/
/*
* Acceleration varies between a certain max and min range, determined by the
* physical constraints:
* max value: mechanically determined. Higher acceleration values may not
*	produce the required torque to move the motor shaft
* min value: the size of the OCR1A register. For a given Timer frequency, 
* 	lower acceleration values may represent cn values greater than the maximum
* 	that can be stored in OCR1A.
* This function changes the value of acceleration and re-computes c0 only
* for the linear ramp speed profile.
*/	
int8_t motor_set_accel_percent(uint8_t accel) 
{
	// check for a valid value and state
	// Updates cannot happen while motor is moving!
	if ((accel > 100) || (state != SPEED_HALT)) return -1;

	float a, b;

	a = (float)accel;
	a = a / 100.0;		// percentage
	b = ((ACCEL_MAX - ACCEL_MIN) * a) + ACCEL_MIN;	// acceleration within the allowed range

	if (speed_profile == PROFILE_LINEAR) {
		c0 = 0.676 * f * sqrt(2.0 / b);		// Correction based on David Austin paper
	} else if (speed_profile == PROFILE_QUADRATIC) {
		// c0 = f * pow((3.0 / a), (1.0/3.0));	// way too high
		// using the formula produces way too high integers. Thus, only
		// possible value is the maximum OCR1A can store
		c0 = 65535.0;
	}
	// max c0 value is (2^16)-1
	if (c0 > 65535.0) c0 =  65535.0;

	//debug
	char str[6];
	ltoa((int32_t)c0, str, 10);
	uart_send_string("\n\rc0: ");
	uart_send_string(str);

	return 0;
}

/*===========================================================================*/
/*
* Returns acceleration value in units of steps/sec^2. 
* The constant 0.676 accounts for a correction aimed at getting the right
* acceleration value, since the same constant was used when computing c0.
*/
int16_t motor_get_accel(void)
{
	float f_mot = (float)F_MOTOR;
	float acc =  2.0 * pow((f_mot / ((c0 + 1) / 0.676)), 2.0);

	return (int16_t)acc;
}

/*===========================================================================*/
uint16_t motor_get_speed(void)
{
	return timer_speed_get();
}

/*===========================================================================*/
int8_t motor_get_speed_percent(void)
{
	float percent = 0.0;

	// If timer is disabled, do nothing.
	if (timer_speed_check()) {
		uint16_t s = motor_get_speed();
		float pulses = f / (float)(s + 1);

		percent = 100.0 * pulses / SPEED_MAX;
		if (dir == CCW)
			percent *= -1.0;
	}

	return (int8_t)percent;
}

/*===========================================================================*/
uint8_t motor_get_profile(void)
{
	return speed_profile;
}

/*===========================================================================*/
int32_t motor_get_position(void)
{
	return current_pos;
}

/*===========================================================================*/
void motor_set_position(int32_t p)
{
	current_pos = p;
}

/*===========================================================================*/
uint8_t motor_get_dir(void)
{
	return dir;
}

/*===========================================================================*/
/*
* Stopping the motor. It can be a sudden stop, or a smooth one.
*/ 
void motor_stop(uint8_t type) 
{
	if (type == SOFT_STOP) {
		if (state != SPEED_HALT) {
			if (dir == CW) target_pos = current_pos + (int32_t)n;
			else target_pos = current_pos - (int32_t)n;
		}
	} else if (type == HARD_STOP) {
		// target position is overwritten with the next step.
		if (dir == CW) target_pos = current_pos + 1;
		else if (dir == CCW) target_pos = current_pos - 1;
	}
}

/*===========================================================================*/
uint8_t motor_working(void)
{
	return timer_speed_check();
}

/*===========================================================================*/
/*
* Position control function
* This is a non-blocking control function, meaning that it sets all required
* parameters, and initializes the motor movement, and relies on the subsequent
* ISR computations to keep moving the motor at the right speed towards the 
* target position.
*
* There's a catch: if the motor is already moving, the movement is not reset
* but instead a new speed is computed and, depending on the new target position
* relative to the current position, the slider may need to stop and move in the
* opposite direction. In these cases, two movements need to be done, thus the
* second movement is queuded, and executed once the first (stop) movement is
* executed. The second movement does not require a call to this function, but
* its automatically issued by means of an auxiliary ISR that triggers once the
* first movement finishes.
*
* Parameters:
*	- p: new position value
*	- mode: absolute (relative to origin) or relative (relative to current pos)
* 	- limits: flag. Indicates whether to check for slider boundary limits. 
*/
void motor_move_to_pos(int32_t p, uint8_t mode, uint8_t limits)
{
	ctl = POSITION_CONTROL;

	// If slider limits flag is TRUE, then check the slider position to avoid
	// crashing. If FALSE, do not check limits. Useful for HOMING cycle.
	if (limits) {
		if (mode == ABS) {
			// Check valid target position (avoid crashing the slider)
			if ((p <= MAX_COUNT) && (p >= 0))
				target_pos = p;
			else if (p > MAX_COUNT)
				target_pos = MAX_COUNT;
			else if (p < 0)
				target_pos = 0;
		} else if (mode == REL) {
			int32_t np = current_pos + p;
			// Check valid target position (avoid crashing the slider)
			if ((np <= MAX_COUNT) && (np >= 0))
				target_pos = np;
			else if (np > MAX_COUNT)
				target_pos = MAX_COUNT;
			else if (np < 0)
				target_pos = 0;
		}
	} else {
		if (mode == ABS) target_pos = p;
		else if (mode == REL) target_pos = current_pos + p;
	}

	if (target_pos == current_pos) return;	//discard if position is the same as target

	// Determine how's the motor moving:
	if (state == SPEED_HALT) {
		
		if (target_pos > current_pos) drv_dir(CW, &dir);
		else drv_dir(CCW, &dir);
			
		drv_set(ENABLE);
		cn = c0;
		timer_speed_set(ENABLE, (uint16_t)cn);
		pulse();
		compute_c_position();	// computes the next cn for the next cycle
		state = SPEED_UP;
		queue_full = FALSE;

	} else {
		// All possible cases of target position vs current position & movement direction:
		//	- target_pos >= current_pos: moving CW - towards the final position - check closeness to target
		//								moving CCW - away from the final position
		//	- target_pos < current_pos: moving CW - away from the final position
		//								moving CCW - towards the final position - check closeness to target
		// 
		if (target_pos >= current_pos) {
			if (dir == CW) {
				if ((target_pos - current_pos) < (int32_t)n) {	// motor too close to target to stop
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {	// No interrupt should occur
						queue_position_motion(target_pos);
						motor_stop(SOFT_STOP);
					}
				}
			} else {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					queue_position_motion(target_pos);
					motor_stop(SOFT_STOP);
				}
			}
		} else {
			if (dir == CW) {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					queue_position_motion(target_pos);
					motor_stop(SOFT_STOP);
				}
			} else {
				if (abs(target_pos - current_pos) < (int32_t)n) {	// motor too close to target to stop
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
						queue_position_motion(target_pos);
						motor_stop(SOFT_STOP);
					}
				}
			}
		}
	}
}

/*===========================================================================*/
/*
* Blocking position control function
* This is a blocking control function, meaning that it uses the non-blocking
* counterpart, and polls the motor state until it is completely halted to 
* finish execution.
*
* Parameters:
*	- p: new position value
*	- mode: absolute (relative to origin) or relative (relative to current pos)
* 	- limits: flag. Indicates whether to check for slider boundary limits. 
*/
int8_t motor_move_to_pos_block(int32_t pos, uint8_t mode, uint8_t limits) 
{
	int8_t x = 0;
	volatile uint8_t *p = limit_switch_get();

	motor_move_to_pos(pos, mode, limits);
	while(state != SPEED_HALT) {
		if ((*p)) {
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
				motor_stop(HARD_STOP);
				(*p) = FALSE;
				x = -1;
			}
			break;
		}
	}
	return x;
}

/*===========================================================================*/
/*
* Speed control function
* This is a non-blocking control function, meaning that it sets all required
* parameters, and initializes the motor movement, and relies on the subsequent
* ISR computations to keep moving the motor at the right speed and taking care
* of the slider rail limits.
*
* There's a catch: if the motor is already moving, the movement is not reset
* but instead a new speed is computed and, depending on the new target speed
* relative to the current speed, the slider may need to stop and move in the
* opposite direction. In these cases, two movements need to be done, thus the
* second movement is queuded, and executed once the first (stop) movement is
* executed. The second movement does not require a call to this function, but
* its automatically issued by means of an auxiliary ISR that triggers once the
* first movement finishes.
*
* Parameters:
*	- p: new speed value 
*/
void motor_move_at_speed(int8_t s)
{
	ctl = SPEED_CONTROL;

	float c;
	uint8_t newdir;

	if (s > 0) {			// positive speed
		newdir = CW;
		c = get_cmin(s);
	} else if (s < 0) {		// negative speed
		newdir = CCW;
		c = get_cmin((-1 * s));	
	} else {				// speed = 0
		c = -1.0;			// Any negative value.
	}
	
	if (state == SPEED_HALT) {
		if (s != 0) {
			if (newdir == CW) drv_dir(CW, &dir);
			else if (newdir == CCW) drv_dir(CCW, &dir);
			// Check limits before starting motion.
			if (((current_pos >= 0) && (current_pos < MAX_COUNT) && (dir == CW)) ||
				((current_pos > 0) && (current_pos <= MAX_COUNT) && (dir == CCW))) {
				drv_set(ENABLE);
				cn = c0;
				timer_speed_set(ENABLE, (uint16_t)cn);
				state = SPEED_UP;
				speed_stop = FALSE;
				cmin = c;
			}
		}
	} else {
		if (s == 0) {					// if target speed is 0
			state = SPEED_DOWN;
			speed_stop = TRUE;
		} else {
			if (newdir == CW) {
				if (dir == CW) {		// if new speed goes in the same rotation direction
					if (c < cmin) {
						cmin = c;
						state = SPEED_UP;
					} else if (c > cmin){
						c_target = c;
						state = SPEED_DOWN;
					}
				} else if (dir == CCW) {	// if new speed goes in opposite rotation direction
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
						state = SPEED_DOWN;
						speed_stop = TRUE;
						queue_speed_motion(s);
						motor_stop(SOFT_STOP);
					}
				}
			} else if (newdir == CCW) {
				if (dir == CW) {
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
						state = SPEED_DOWN;
						speed_stop = TRUE;
						queue_speed_motion(s);
						motor_stop(SOFT_STOP);
					}
				} else if (dir == CCW) {	// if new speed goes in opposite rotation direction
					if (c < cmin) {
						cmin = c;
						state = SPEED_UP;
					} else if (c > cmin){
						c_target = c;
						state = SPEED_DOWN;
					}
				}
			}	
		}	
	}
}

/*-----------------------------------------------------------------------------
--------------------- I N T E R N A L   F U N C T I O N S ---------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* Position control, Cn computation
* The algorithm is based on the ability to compute the next Cn coefficient,
* based on the current Cn and the value of n itself. Cn is based on an 
* arithmetic progression described on the David Austin paper on Real Time 
* Calculations for Stepper Motors. The boundary for the arithmetic progression
* is the maximum allowd speed. At that point the motor will not accelerate but
* remain at a constant speed. 
*
* This function is constantly called from the motor timer ISR to compute every
* time the new value of Cn.
*
* The arithmetic progression has slightly different form when the motor is
* accelerating compared to when the motor is decelerating. The value of n also
* depends on the current state of the movement. This function handles all these
* cases, as well as handling the case when multiple movement commands are 
* issued before the motor stops completely
*/
static void compute_c_position(void)
{
	int32_t steps_ahead = labs(target_pos - current_pos);

	if (steps_ahead > (int32_t)n) {
		if (cn <= cmin) state = SPEED_FLAT;
		else state = SPEED_UP;
	} else {
		state = SPEED_DOWN;
	}
	
	switch (state) {

		case SPEED_UP:
			n++;
			next_cn();
			if (cn <= cmin) {
				cn = cmin;
				state = SPEED_FLAT;
			}
			break;

		case SPEED_FLAT:
			cn = cmin;
			if (steps_ahead <= (int32_t)n) {
				state = SPEED_DOWN;
				next_cn();
			}
			break;

		case SPEED_DOWN:
			n = (uint16_t)steps_ahead;
			if (n > 0) {
				next_cn();
			} else {
				cn = c0;
				timer_speed_set(DISABLE, (uint16_t)c0);	
				state = SPEED_HALT;
				
				drv_set(DISABLE);

				// check queue:
				if (queue_full)
					timer_aux_set(ENABLE, 100);	// software ISR to execute queued movement	
			}
			break;

		default:
			break;
	}
}

/*===========================================================================*/
/*
* Speed control, Cn computation
* The algorithm is based on the ability to compute the next Cn coefficient,
* based on the current Cn and the value of n itself. Cn is based on an 
* arithmetic progression described on the David Austin paper on Real Time 
* Calculations for Stepper Motors. The boundary for the arithmetic progression
* is the maximum allowd speed. At that point the motor will not accelerate but
* remain at a constant speed. 
*
* This function is constantly called from the motor timer ISR to compute every
* time the new value of Cn.
*
* The arithmetic progression has slightly different form when the motor is
* accelerating compared to when the motor is decelerating. The value of n also
* depends on the current state of the movement. This function handles all these
* cases, as well as handling the case when multiple movement commands are 
* issued before the motor stops completely
*/
static void compute_c_speed(void)
{
	// limits of the slider: avoid crashing with the boundaries
	if ((current_pos <= n) && (dir == CCW)) {
		state = SPEED_DOWN;
	} else if (((MAX_COUNT - current_pos) <= n) && (dir == CW)) {
		state = SPEED_DOWN;
	}

	switch (state) {
		case SPEED_UP:
			n++;
			next_cn();
			if (cn <= cmin) {
				cn = cmin;
				state = SPEED_FLAT;
			}
			break;

		case SPEED_FLAT:
			cn = cmin;
			break;

		case SPEED_DOWN:
			if (n > 0) {
				next_cn();
				if (!speed_stop) {			// if motor is not issued a stop instruction
					if (cn >= c_target) {
						cmin = cn;
						state = SPEED_FLAT;
					}
				}
			} else {
				cn = c0;
				timer_speed_set(DISABLE, (uint16_t)c0);	
				state = SPEED_HALT;
				
				drv_set(DISABLE);

				char str[6];
				ltoa(current_pos, str, 10);
				uart_send_string("\n\rpos: ");
				uart_send_string(str);

				if (queue_full)
					timer_aux_set(ENABLE, 100);	// software ISR to execute queued movement	
			}
			if (n > 0)
				n--;
			break;

		default:
			break;
	}
}

/*===========================================================================*/
/*
* Cn arithmetic progression. Four cases are considered:
* - Linear speed profile:
*	- Motor accelerating
*	- Motor deceleating
* - Quadratic speed profile:
*	- Motor accelerating
*	- Motor deceleating
*/
static void next_cn(void)
{
	if (speed_profile == PROFILE_LINEAR) {
		if (state == SPEED_UP) 
			cn = cn - (2.0 * cn) / (4.0 * (float)n + 1.0);
		else 
			cn = cn - (2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0);
	} else if (speed_profile == PROFILE_QUADRATIC) {
		if (n == 1) {
			cn = 0.9 * c0;		// correction for quadratic profile. See David Austin paper
		} else {
			if (state == SPEED_UP)
				cn = cn - (6.0 * cn) / (9.0 * (float)n + 3.0);
			else
				cn = cn - (6.0 * cn) / (9.0 * (float)n * (-1.0) + 3.0);
		}
	}	
}

/*===========================================================================*/
/*
* Based on a speed percentage, get the minimum value of Cn, which is equivalent
* to the maximum speed allowed
*/
static float get_cmin(uint8_t percent)
{
	// check for a valid value and state
	if ((percent > 100) || (percent == 0)) return -1.0;

	float a, b;

	a = (float)percent;
	a = a / 100.0;		// percentage
	b = ACCEL_MAX * a;		// Fraction of max speed

	return (f / b) - 1.0;
}

/*===========================================================================*/
/*
* Queue motion. If the slider is moving in a certain direction and requires to
* change direction, queue the motion to procede with the motor stop
*/
static void queue_position_motion(int32_t p) 
{
	queue_pos = p;
	queue_full = TRUE;
	DEBUG("\n\r>");	// Debug
}

/*===========================================================================*/
/*
* Queue motion. If the slider is moving in a certain direction and requires to
* change direction, queue the motion to procede with the motor stop
*/
static void queue_speed_motion(int8_t s)
{
	queue_speed = s;
	queue_full = TRUE;
	DEBUG("\n\r>>");	// Debug
}

/*===========================================================================*/
/*
* Motor Driver Pulse function.
* Toggles the driver step pin to generate a step in the motor. Called from the
* motor timer ISR
*/
static void pulse(void) 
{
	DRV_STEP_PORT |= (1<<DRV_STEP_PIN);
	_delay_us(2);
	DRV_STEP_PORT &= ~(1<<DRV_STEP_PIN);
	if (dir == CW) current_pos++;
	else current_pos--;
}

/******************************************************************************
*******************************************************************************

					I N T E R R U P T   H A N D L E R S

*******************************************************************************
******************************************************************************/

/******************************************************************************
********************** T I M E R S   I N T E R R U P T S **********************
******************************************************************************/

/*===========================================================================*/
/*
* Motor timer interrupt. Whenever a new pulse needs to be issued (based on the
* value of Cn), this ISR triggers. It steps the motor, sets the new Cn value
* and computes the future Cn value.
*/
ISR(TIMER1_COMPA_vect) 
{
	pulse();
	// set the new timing delay (based on computation of cn)
	timer_speed_set_raw((uint16_t)cn);
	// compute the timing delay for the next cycle
	if (ctl == POSITION_CONTROL) compute_c_position();
	else if (ctl == SPEED_CONTROL) compute_c_speed();
}

/*===========================================================================*/
/*
* Miscelaneous Timer. Used to generate "software-like" interrupts.
* When a movement has been queued, the way to trigger the new movement is by
* enablig this interrupt at the end of the current movement. It will trigger
* the missing movement using the non-blocking function (either for a speed
* movement or a position movement)
*/
ISR(TIMER0_COMPA_vect) 
{
	timer_aux_set(DISABLE, 100);
	if (ctl == POSITION_CONTROL) motor_move_to_pos(queue_pos, ABS, TRUE);
	else if (ctl == SPEED_CONTROL) motor_move_at_speed(queue_speed);
	queue_full = FALSE;
	DEBUG("\n\rTMR0");
}