
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/
#include "motor.h"
#include "config.h"
#include "driver.h"
#include "timers.h"
#include "uart.h"

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

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

// motor structure instance
motor_s motor;

float cn;
float c0;
float cmin;
uint16_t n;
volatile uint8_t spd;		// state variable

volatile int32_t current_pos;
volatile int32_t target_pos;
uint8_t dir;

int32_t queue_pos;
uint8_t queue_full;

// timer frequency: CPU_clk / prescaler
static const float f = 16000000 / 8;

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void compute_c(void);
static void pulse(void);
static void queue_motion(int32_t p);
static void set_accel(float a);

void motor_init(void)
{
	drv_reset();
	drv_set(ENABLE);
	drv_step_mode(MODE_EIGHTH_STEP);
	drv_dir(CW, &dir);
	
	current_pos = 0;
	target_pos = 0;

	// minimum counter value to get max speed
	cmin = 249.0;		// Valid for MODE_EIGHTH_STEPPING and 2MHz clk
	set_accel(8000.0);	// Based on the MODE_EIGHTH_STEPPING parameter
	cn = c0;
	n = 0;
	spd = SPEED_HALT;
	queue_full = FALSE;
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
	if (spd == SPEED_HALT) {
		
		if (target_pos > current_pos) drv_dir(CW, &dir);
		else drv_dir(CCW, &dir);
			
		drv_set(ENABLE);
		cn = c0;
		speed_timer_set(ENABLE, (uint16_t)cn);
		pulse();
		compute_c();	// computes the next cn for the next cycle
		spd = SPEED_UP;
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
						queue_motion(target_pos);
						motor_stop();
					}
				}
			} else {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					queue_motion(target_pos);
					motor_stop();
				}
			}
		} else {
			if (dir == CW) {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					queue_motion(target_pos);
					motor_stop();
				}
			} else {
				if (abs(target_pos - current_pos) < (int32_t)n) {	// motor too close to target to stop
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
						queue_motion(target_pos);
						motor_stop();
					}
				}
			}
		}
	}
}

void motor_move_to_pos_block(int32_t p, uint8_t mode) 
{

	motor_move_to_pos(p, mode);
	while(spd != SPEED_HALT);
}

void motor_stop(void) 
{

	if (spd != SPEED_HALT) {
		if (dir == CW) target_pos = current_pos + (int32_t)n;
		else target_pos = current_pos - (int32_t)n;
	}
}

int8_t motor_set_maxspeed_percent(uint8_t speed) 
{

	// check for a valid value and state
	// Updates cannot happen while motor is moving!
	if ((speed > 100) && (spd != SPEED_HALT)) return -1;

	float a, b;

	a = (float)speed;
	a = a / 100.0;		// percentage
	b = 8000.0 * a;		// Fraction of max speed

	cmin = (f / b) - 1.0;

	return 0;
}

int8_t motor_set_accel_percent(uint8_t accel) 
{

	// check for a valid value and state
	// Updates cannot happen while motor is moving!
	if ((accel > 100) && (spd != SPEED_HALT)) return -1;

	float a, b;

	a = (float)accel;
	a = a / 100.0;		// percentage
	b = 8000.0 * a;		// Fraction of max acceleration

	set_accel(b);

	return 0;
}

static void compute_c(void)
{

	int32_t steps_ahead = labs(target_pos - current_pos);

	if (steps_ahead > (int32_t)n) {
		if (cn <= cmin) spd = SPEED_FLAT;
		else spd = SPEED_UP;
	} else {
		spd = SPEED_DOWN;
	}
	
	switch (spd) {

		case SPEED_UP:
			n++;
			cn = cn - (2.0 * cn) / (4.0 * (float)n + 1.0);
			if (cn <= cmin) {
				cn = cmin;
				spd = SPEED_FLAT;
			}
			break;

		case SPEED_FLAT:
			cn = cmin;
			if (steps_ahead <= (int32_t)n) {
				spd = SPEED_DOWN;
				cn = cn - (2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0);
			}
			break;

		case SPEED_DOWN:
			n = (uint16_t)steps_ahead;
			if (n > 0) {
				cn = cn - (2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0);
			} else {
				cn = c0;
				speed_timer_set(DISABLE, (uint16_t)c0);	
				spd = SPEED_HALT;
				
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

static void set_accel(float a)
{
	
	c0 = 0.676 * f * sqrt(2.0 / a);		// Correction based on David Austin paper

	char str[6];
	ltoa((uint32_t)c0, str, 10);
	uart_send_string("\n\rc0: ");
	uart_send_string(str);
}

static void queue_motion(int32_t p) 
{

	queue_pos = p;
	queue_full = TRUE;
	uart_send_string("\n\r>");	// Debug
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
	compute_c();

}

ISR(TIMER0_COMPA_vect) 
{
/*
* Miscelaneous Timer
* Used to generate "software-like" interrupts
*/
	aux_timer_set(DISABLE, 100);
	motor_move_to_pos(queue_pos, ABS);
	queue_full = FALSE;
	uart_send_string("\n\rTMR0");
}