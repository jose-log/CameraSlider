
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

#define POSITION_CONTROL	0x71
#define SPEED_CONTROL 		0x70

#define SPEED_MAX 		8000.0		// Valid for MODE_EIGHTH_STEP
#define ACCEL_MAX 		8000.0		// Valid for MODE_EIGHTH_STEP
#define ACCEL_MIN 		1862.0		// Valid for 2MHz timer frequency

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

// motor structure instance
motor_s motor;

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
static void set_accel(float a);
static void next_cn(void);

void motor_init(void)
{
	drv_reset();
	drv_set(ENABLE);
	drv_step_mode(MODE_EIGHTH_STEP);
	drv_dir(CW, &dir);
	
	current_pos = 0;
	target_pos = 0;

	// minimum counter value to get max speed
	cmin = CMIN_EIGHTH_STEPPING;
	motor_set_accel_percent(100);
	cn = c0;
	n = 0;
	state = SPEED_HALT;
	queue_full = FALSE;
}

void motor_move_to_pos_block(int32_t p, uint8_t mode) 
{
	motor_move_to_pos(p, mode);
	while(state != SPEED_HALT);
}

void motor_stop(void) 
{

	if (state != SPEED_HALT) {
		if (dir == CW) target_pos = current_pos + (int32_t)n;
		else target_pos = current_pos - (int32_t)n;
	}
}

void motor_speed_profile(uint8_t p)
{
	if (p == PROFILE_LINEAR) {
		speed_profile = PROFILE_LINEAR;
	} else if (p == PROFILE_QUADRATIC) {
		speed_profile = PROFILE_QUADRATIC;
	}
	// whenever the speed profile is chose, maximum acceleration is chosen
	motor_set_accel_percent(100);
}

int8_t motor_set_maxspeed_percent(uint8_t speed) 
{
/*
* Max speed is mechanically constrained: By testing, higher speeds provoke
* the motor to run out of sync
* Min speed is limited by the resolution of the percent given (which is an
* integer), thus, 1% is the minimum
*/
	// check for a valid value and state
	// Updates cannot happen while motor is moving!
	if ((speed > 100) || (speed == 0) || (state != SPEED_HALT)) return -1;

	float a, b;

	a = (float)speed;
	a = a / 100.0;		// percentage
	b = SPEED_MAX * a;		// Fraction of max speed

	cmin = (f / b) - 1.0;

	char str[6];
	ltoa((uint32_t)c0, str, 10);
	uart_send_string("\n\rc0: ");
	uart_send_string(str);

	return 0;
}

int8_t motor_set_accel_percent(uint8_t accel) 
{
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
	// check for a valid value and state
	// Updates cannot happen while motor is moving!
	if ((accel > 100) || (speed != SPEED_HALT)) return -1;

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

	return 0;
}

void motor_move_to_pos(int32_t p, uint8_t mode)
{
	if (mode == ABS) {
		if (current_pos == p) return;	//discard if position is the same as target
		else target_pos = p;
	} else if (mode == REL) {
		if (p == 0) return;				//discard if position is the same as target
		else target_pos = current_pos + p;
	}

	// Determine how's the motor moving:
	if (state == SPEED_HALT) {
		
		if (target_pos > current_pos) drv_dir(CW, &dir);
		else drv_dir(CCW, &dir);
			
		drv_set(ENABLE);
		cn = c0;
		speed_timer_set(ENABLE, (uint16_t)cn);
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
						motor_stop();
					}
				}
			} else {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					queue_position_motion(target_pos);
					motor_stop();
				}
			}
		} else {
			if (dir == CW) {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					queue_position_motion(target_pos);
					motor_stop();
				}
			} else {
				if (abs(target_pos - current_pos) < (int32_t)n) {	// motor too close to target to stop
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
						queue_position_motion(target_pos);
						motor_stop();
					}
				}
			}
		}
	}
}

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
			drv_set(ENABLE);
			cn = c0;
			speed_timer_set(ENABLE, (uint16_t)cn);
			state = SPEED_UP;
			speed_stop = FALSE;
			cmin = c;
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
						motor_stop();
					}
				}
			} else if (newdir == CCW) {
				if (dir == CW) {
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
						state = SPEED_DOWN;
						speed_stop = TRUE;
						queue_speed_motion(s);
						motor_stop();
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
				speed_timer_set(DISABLE, (uint16_t)c0);	
				state = SPEED_HALT;
				
				drv_set(DISABLE);
				
				// check queue:
				if (queue_full)
					aux_timer_set(ENABLE, 100);	// software ISR to execute queued movement	
			}
			break;

		default:
			break;
	}
}

static void compute_c_speed(void)
{
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
			n--;
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
				speed_timer_set(DISABLE, (uint16_t)c0);	
				state = SPEED_HALT;
				
				drv_set(DISABLE);

				if (queue_full)
					aux_timer_set(ENABLE, 100);	// software ISR to execute queued movement	
			}
			break;

		default:
			break;
	}
}

static void next_cn(void)
{
	if (speed_profile == PROFILE_LINEAR) {
		if (spd == SPEED_UP) 
			cn = cn - (2.0 * cn) / (4.0 * (float)n + 1.0);
		else 
			cn = cn - (2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0);
	} else if (speed_profile == PROFILE_QUADRATIC) {
		if (n == 1) {
			cn = 0.9 * c0;		// correction for quadratic profile. See David Austin paper
		} else {
			if (spd == SPEED_UP)
				cn = cn - (6.0 * cn) / (9.0 * (float)n + 3.0);
			else
				cn = cn - (6.0 * cn) / (9.0 * (float)n * (-1.0) + 3.0);
		}
	}	
}

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

static void queue_position_motion(int32_t p) 
{

	queue_pos = p;
	queue_full = TRUE;
	uart_send_string("\n\r>");	// Debug
}

static void queue_speed_motion(int8_t s)
{
	queue_speed = s;
	queue_full = TRUE;
	uart_send_string("\n\r>>");	// Debug
}

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

ISR(TIMER1_COMPA_vect) 
{
/*
* Speed Timer.
*/
	pulse();
	// set the new timing delay (based on computation of cn)
	speed_timer_set_raw((uint16_t)cn);
	// compute the timing delay for the next cycle
	if (ctl == POSITION_CONTROL) compute_c_position();
	else if (ctl == SPEED_CONTROL) compute_c_speed();
}

ISR(TIMER0_COMPA_vect) 
{
/*
* Miscelaneous Timer
* Used to generate "software-like" interrupts
*/
	aux_timer_set(DISABLE, 100);
	if (ctl == POSITION_CONTROL) motor_move_to_pos(queue_pos, ABS);
	else if (ctl == SPEED_CONTROL) motor_move_at_speed(queue_speed);
	queue_full = FALSE;
	uart_send_string("\n\rTMR0");
}