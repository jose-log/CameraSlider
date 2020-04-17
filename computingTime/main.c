
#include "config.h"
#include "driver.h"
#include "init.h"
#include "timers.h"
#include "stepper.h"
#include "uart.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
/*
* COMPUTING EXECUTION TIME:
* The intention of this project is to compare the execution time between 
* different calculation methods using different approaches:
* - exact method: square root calculation using float variables
* - approximation method: arithmetic operations using float
* - approximation method: arithmetic operations using integers
*
* This project is also useful to compute a large amount of delay coefficients
* cn, and compare how they differ from the exact calculation method (using
* the sqare root calculation with floating point variables)
*
* It's advised to test different optimization levels
*/

int main(void){

	boot();
	uart_send_string("\n\r----------------------------------");
	uart_send_string("\n\rHello World!");

	// Size of the vectors containing the delay coefficients
	uint8_t sz = 120;
	// Frequency yo be used in the final application
	float f = (float)F_CPU / 64.0; 		// Timer Prescaler 64
	float a = 8000.0;					// Acceleration value (steps per second per second)
	float c0 = f * sqrt(2.0 / a);

	sei();

	/*
	*	FLOATING POINT CALCULATION
	*/
	float cn;
	float v[sz];
	char str[7];
	uint16_t t;	

	// Initial empiric correction. See David Austin Paper
	cn = 0.676 * c0;	

	uart_send_string("\n\rc0: ");
	itoa((uint16_t)cn, str, 10);
	uart_send_string(str);

	uart_send_string("\n\r***** Floating point *****");
	general_timer_init();
	itoa(TCNT2, str, 10);
	uart_send_string("\n\rTCNT init: ");
	uart_send_string(str);
	//general_timer_set(ENABLE);

	for (int8_t i = 1; i <= sz; i++) {
		// Approximation method using arithmetic operations
		cn = cn - (2.0 * cn) / (4.0 * (float)i + 1.0);
		// Exact method using arithmetic operations
		//cn = c0 * (sqrt((float)i + 1.0) - sqrt((float)i));
		v[i - 1] = cn;
	}

	t = TCNT2;
	general_timer_set(DISABLE);
	itoa(t, str, 10);
	uart_send_string("\n\rTCNT final: ");
	uart_send_string(str);	

	for (uint8_t i = 0; i < sz; i++) {
		itoa((uint16_t)v[i], str, 10);
		uart_send_string("\n\rcn: ");
		uart_send_string(str);
	}

	/*
	*	INTEGER CALCULATION
	*/

	uint16_t x,y,z;
 	uint16_t vi[sz];
 	uint16_t cni = (uint16_t)(0.676 * c0);

 	uart_send_string("\n\r***** Integer 16 *****");
	general_timer_init();
 	itoa(TCNT2, str, 10);
 	uart_send_string("\n\rTCNT init: ");
 	uart_send_string(str);
 	//general_timer_set(ENABLE);

	for (uint8_t i = 1; i <= sz; i++) {
		x = (4 * i) + 1;
		y = 2 * cni;
		z = y / x;
		cni = cni - z;
		vi[i - 1] = cni;
	}

	t = TCNT2;
	general_timer_set(DISABLE);
	itoa(t, str, 10);
	uart_send_string("\n\rTCNT final: ");
	uart_send_string(str);	

	for (uint8_t i = 0; i < sz; i++) {
		itoa(vi[i], str, 10);
		uart_send_string("\n\rcn: ");
		uart_send_string(str);
	}

	uart_send_string("\n\rFINISH!");
	return 0;
}

ISR(TIMER2_OVF_vect){

	uart_send_string("\n\rERROR: Timer Overflowed!.");
	// do not re-enter this ISR:
	TIMSK2 &= ~(1 << TOIE2);
}