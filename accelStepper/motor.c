

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

#define SPEED_UP	0xF1
#define SPEED_FLAT	0xF2
#define SPEED_DOWN	0xF3
#define SPEED_HALT 	0xF0

#define POSITION_CONTROL	0x71
#define SPEED_CONTROL 		0x70
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

static int32_t queue_pos;
static int8_t queue_speed;
static uint8_t queue_full;

static uint8_t speed_stop;
static uint8_t ctl;

// timer frequency: CPU_clk / prescaler
static const float f = 16000000 / 8;

static void compute_c(void);
static void set_accel(float a);

static void pulse(void){

	DRV_STEP_PORT |= (1<<DRV_STEP_PIN);
	_delay_us(2);
	DRV_STEP_PORT &= ~(1<<DRV_STEP_PIN);
	if (dir == CW) current_pos++;
	else current_pos--;
}

static void queue_motion(int32_t p) {

	queue_pos = p;
	queue_full = TRUE;
	uart_send_string("\n\r>");	// Debug
}

static void queue_speed_motion(int32_t s)
{
	queue_speed = s;
	queue_full = TRUE;
	uart_send_string("\n\r>>");	// Debug
}

void motor_init(void) {

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

static void set_accel(float a){
	
	c0 = 0.676 * f * sqrt(2.0 / a);		// Correction based on David Austin paper

	char str[6];
	ltoa((uint32_t)c0, str, 10);
	uart_send_string("\n\rc0: ");
	uart_send_string(str);
}

void motor_move_to_pos(int32_t p, uint8_t mode){

	ctl = POSITION_CONTROL;

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

void motor_move_to_pos_block(int32_t p, uint8_t mode) {

	motor_move_to_pos(p, mode);
	while(spd != SPEED_HALT);
}

void motor_stop(void) {

	if (spd != SPEED_HALT) {
		if (dir == CW) target_pos = current_pos + (int32_t)n;
		else target_pos = current_pos - (int32_t)n;
	}

	// debug
	char str[8];
	int32_t a = current_pos;
	int32_t b = (int32_t)n;
	int32_t c = target_pos;

	NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE){
		ltoa(a, str, 10);
		uart_send_string("\n\rpos: ");
		uart_send_string(str);
		ltoa(b, str, 10);
		uart_send_string("\n\rn: ");
		uart_send_string(str);
		ltoa(c, str, 10);
		uart_send_string("\n\rtarget: ");
		uart_send_string(str);
	}
	
}

int8_t motor_set_maxspeed_percent(uint8_t speed) {

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

int8_t motor_set_accel_percent(uint8_t accel) {

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

static float get_cmin(uint8_t percent)
{
	// check for a valid value and state
	if (percent > 100) return -1.0;

	float a, b;

	a = (float)percent;
	a = a / 100.0;		// percentage
	b = 8000.0 * a;		// Fraction of max speed

	return (f / b) - 1.0;
}
//char str[8];
//uint16_t nv[100];
//uint16_t cv[100];
//uint16_t i = 0;
float c_target;

void motor_move_at_speed(int8_t s)
{
	ctl = SPEED_CONTROL;

	float c;
	uint8_t newdir;

	if (s > 0) {
		newdir = CW;
		c = get_cmin(s);
	} else if (s < 0) {
		newdir = CCW;
		c = get_cmin((-1 * s));	
	} 

	char str[6];
	itoa((int16_t)c, str, 10);
	uart_send_string("\n\rcmin: ");
	uart_send_string(str);
	
	if (spd == SPEED_HALT) {
		if (newdir == CW) drv_dir(CW, &dir);
		else if (newdir == CCW) drv_dir(CCW, &dir);
		drv_set(ENABLE);
		cn = c0;
		speed_timer_set(ENABLE, (uint16_t)cn);
		spd = SPEED_UP;
		speed_stop = FALSE;
		cmin = c;
	} else {
		if (newdir == CW) {
			if (dir == CW) {		// if new speed goes in the same rotation direction
				if (c < cmin) {
					cmin = c;
					spd = SPEED_UP;
				} else if (c > cmin){
					c_target = c;
					spd = SPEED_DOWN;
				}
			} else if (dir == CCW) {	// if new speed goes in opposite rotation direction
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					spd = SPEED_DOWN;
					speed_stop = TRUE;
					queue_speed_motion(s);
					motor_stop();
				}
			}
		} else if (newdir == CCW) {
			if (dir == CW) {
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {		//No interrupt should occur
					spd = SPEED_DOWN;
					speed_stop = TRUE;
					queue_speed_motion(s);
					motor_stop();
				}
			} else if (dir == CCW) {	// if new speed goes in opposite rotation direction
				if (c < cmin) {
					cmin = c;
					spd = SPEED_UP;
				} else if (c > cmin){
					c_target = c;
					spd = SPEED_DOWN;
				}
			}
		}		
	}
}

static void compute_c_speed(void)
{
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
			break;

		case SPEED_DOWN:
			n--;
			if (n > 0) {
				cn = cn - (2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0);
				if (!speed_stop) {			// if motor is not issued a stop instruction
					if (cn >= c_target) {
						cmin = cn;
						spd = SPEED_FLAT;
					}
				}
			} else {
				cn = c0;
				speed_timer_set(DISABLE, (uint16_t)c0);	
				spd = SPEED_HALT;
				
				drv_set(DISABLE);

				if (queue_full)
					aux_timer_set(ENABLE, 100);	// software ISR to execute queued movement	
			}
			break;

		default:
			break;
	}
}

static void compute_c(void){

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

				// debug message
				char str[8];
				ltoa(current_pos, str, 10);
				uart_send_string("\n\rFinal pos: ");
				uart_send_string(str);
				
				// check queue:
				if (queue_full)
					aux_timer_set(ENABLE, 100);	// software ISR to execute queued movement	
			}
			break;

		default:
			break;
	}

	/*
	if (steps_ahead > (uint32_t)n) {
		if (!maxspeed)
			n++;
		x = (4 * n) + 1;
		y = 2 * cn;
		z = y / x;
		tmp = cn - z;
		if (steps_ahead != n) {
			if (tmp > cmin) {
				cn = tmp;
			} else {
				cn = cmin;
				maxspeed = TRUE;
			}
		}
		//////////
		if ((n > 100) && (i < 100)){
			nv[i] = n;
			cv[i] = cn;
			i++;
		}
	} else {
		maxspeed = FALSE;
		n = (uint16_t)steps_ahead;
		if (n > 0) {
			x = (4 * n) + 1;
			y = 2 * cn;
			z = y / x;
			cn = cn - z;
		} else {
			cn = c0;
			(*timer_set)(DISABLE, (uint16_t)c0);	
			char str[8];
			ltoa(current_pos, str, 10);
			uart_send_string("\n\rFinal pos: ");
			uart_send_string(str);
			moving = FALSE;
			///////////
			for (i = 0; i < 100; i++) {
				itoa(nv[i], str, 10);
				uart_send_string("\n\rn: ");
				uart_send_string(str);
				itoa(cv[i], str, 10);
				uart_send_string(" cn: ");
				uart_send_string(str);
			}
		}
	}*/
}

//****************************************************************


ISR(TIMER1_COMPA_vect)
{
/*
* Speed Timer.
*/
	pulse();
	// set the new timing delay (based on computation of cn)
	speed_timer_set_raw((uint16_t)cn);
	// compute the timing delay for the next cycle
	if (ctl == POSITION_CONTROL) compute_c();
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