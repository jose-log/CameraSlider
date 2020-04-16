

#include "stepper.h"
#include "config.h"
#include "uart.h"

#include <stdlib.h>
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Stepper structure instance
stepper_s stepper;

int32_t cn;
int32_t c0;
int32_t cmin;
int32_t n;
uint8_t maxspeed = FALSE;	// flag: stepper reached max speed
uint8_t moving = FALSE;		// Flag: stepper is moving?

int32_t current_pos;
int32_t target_pos;
int32_t accel;
int32_t accel_max;

// timer frequency: CPU_clk / prescaler
static const uint32_t f = 16000000 / 64;

static void compute_c(void);

// extenal function pointer that sets the timer
void (*timer_set)(uint8_t state, uint16_t t);
void (*timer_set_raw)(uint16_t c);

static void pulse(void){

	DRV_STEP_PORT |= (1<<DRV_STEP_PIN);
	_delay_us(2);
	DRV_STEP_PORT &= ~(1<<DRV_STEP_PIN);
	current_pos++;
}

void stepper_init(void (*timer)(uint8_t state, uint16_t t), void (*timer_raw)(uint16_t c)) {

	current_pos = 0;
	target_pos = 0;
	timer_set = timer;
	timer_set_raw = timer_raw;

	// minimum counter value to get max speed
	cmin = 14;
	//cmin = 249.0;
	stepper_set_accel(9667);	// just a sample value
	//stepper_set_accel(8334);	// just a sample value
	cn = c0;
	n = 0;
	accel_max = accel;
}

void stepper_set_accel(int32_t a){

	float tmp1, tmp2, tmp3;
	tmp1 = (float)f;
	tmp2 = (float)a;
	tmp3 = tmp1 * sqrt(2.0 / tmp2);
	c0 = (int32_t)tmp3;
	accel_max = a;

	char str[7];
	ltoa(c0, str, 10);
	uart_send_string("\n\rc0: ");
	uart_send_string(str);
}

#define UP 			0xF1
#define CONSTANT	0xF2
#define DOWN 		0xF3
uint8_t st = UP;

uint8_t stepper_move_to_pos(int32_t p){

	target_pos = p;

	// first its necessary to check whether the motor is moving or not, to
	// either ENABLE the timer and start from c0 or if the motor is moving
	// start from the appropiate value of cn
	if (!moving) {
		(*timer_set)(ENABLE, (uint16_t)c0);
		pulse();	
		// computes the next cn for the next cycle
		compute_c();
		moving = TRUE;
		st = UP;
		//n = 0;
	} 

	return 0;
}

char str[8];
int32_t x, y, z;

static void compute_c(void){

	//float tmp;
	int32_t steps_ahead = target_pos - current_pos;

	//if (!maxspeed) PORTB |= (1<<PORTB5);
	//else PORTB &= ~(1<<PORTB5);

	/*
	if (steps_ahead > n) {
		if (cn <= cmin) {
			st = CONSTANT;
		} else {
			st = UP;
		}
	} else {
		st = DOWN;
	}
	*/

	if (st == UP) {
		n++;
		x = (4 * n) + 1;
		y = 2 * cn;
		z = y / x;
		cn = cn - z;
		//cn = cn - (2 * cn) / (4 * (float)n + 1);
		//cn = cn - ((2.0 * cn) / (4.0 * (float)n + 1.0));
		if (cn <= cmin) {
			cn = cmin;
			st = CONSTANT;
			PORTB |= (1<<PORTB5);
		}
		//PORTB |= (1<<PORTB5);
		
		ltoa(cn, str, 10);
		uart_send_string("\n\rcn: ");
		uart_send_string(str);
		ltoa(x, str, 10);
		uart_send_string("\n\rx: ");
		uart_send_string(str);
		ltoa(y, str, 10);
		uart_send_string("\n\ry: ");
		uart_send_string(str);
		ltoa(z, str, 10);
		uart_send_string("\n\rz: ");
		uart_send_string(str);
		
	} else if (st == CONSTANT) {
		//PORTB |= (1<<PORTB5);
		if (steps_ahead <= n) {
			st = DOWN;
			x = (-4 * n) + 1;
			y = 2 * cn;
			z = y / x;
			cn = cn - z;
			//cn = cn - (2 * cn) / (4 * (float)n * (-1) + 1);
			//cn = cn - ((2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0));
		}
	} else if (st == DOWN) {
		//PORTB &= ~(1<<PORTB5);
		n = steps_ahead;
		if (n > 0) {
			x = (4 * n) + 1;
			y = 2 * cn;
			z = y / x;
			cn = cn - z;
			//cn = cn - (2 * cn) / (4 * (float)n * (-1) + 1);
			//cn = cn - ((2.0 * cn) / (4.0 * (float)n * (-1.0) + 1.0));
		} else {
			cn = c0;
			(*timer_set)(DISABLE, (uint16_t)c0);	
			moving = FALSE;
			
			char str[8];
			ltoa(current_pos, str, 10);
			uart_send_string("\n\rFinal pos: ");
			uart_send_string(str);
		}
	}

/*

	if (steps_ahead > n) {
		if (!maxspeed)
			n++;
		tmp = cn - (2 * cn) / (4 * n + 1);
		if (steps_ahead != n) {
			if (tmp > cmin) {
				cn = tmp;
			} else {
				cn = cmin;
				maxspeed = TRUE;
			}
		}
	} else {
		PORTB |= (1<<PORTB5);
		maxspeed = FALSE;
		n = steps_ahead;
		if (n > 0)
			cn = cn - (2 * cn) / (4 * n * (-1) + 1);
		else {
			cn = c0;
			(*timer_set)(DISABLE, (uint16_t)c0);	
			char str[8];
			ltoa(current_pos, str, 10);
			uart_send_string("\n\rFinal pos: ");
			uart_send_string(str);
			moving = FALSE;
		}
	}*/
}

//****************************************************************


ISR(TIMER1_COMPA_vect){
/*
* Speed Timer.
*/
	pulse();
	// set the new timing delay (based on computation of cn)
	(*timer_set_raw)((uint16_t)cn);
	// compute the timing delay for the next cycle
	compute_c();

}