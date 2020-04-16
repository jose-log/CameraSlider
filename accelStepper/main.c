
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

int main(void){

	sei();
	boot();
	//speed_timer_init();
	drv_step_mode(MODE_SIXTEENTH_STEP);
	//drv_step_mode(MODE_EIGHTH_STEP);
	drv_spin_direction(CW);

	uart_send_string("\n\r----------------------------------");
	uart_send_string("\n\rHello World!");
	//stepper_init(speed_timer_set, speed_timer_set_raw);

	/*
	int32_t i = 1;
	while (1) {
		
		stepper_move_to_pos(70000 * i);
		_delay_ms(10000);
		i++;
		
	}*/

	uint8_t sz = 100;


	float c0, a;
	float cn;
	float v[sz];
	char str[7];
	uint16_t t;	

	a = 8334.0;
	c0 = 250000.0 * sqrt(2.0 / a);
	cn = c0;

	uart_send_string("\n\r***** Floating point *****");
	general_timer_init();
	itoa(TCNT2, str, 10);
	uart_send_string("\n\rTCNT init: ");
	uart_send_string(str);
	//general_timer_set(ENABLE);

	for (int8_t i = 1; i <= sz; i++) {
		cn = cn - (2.0 * cn) / (4.0 * (float)i + 1.0);
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

	uint16_t x,y,z;
 	uint16_t vi[sz];
 	uint16_t cni = (uint16_t)c0;

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
/*
* General Timer.
*/
	uart_send_string("\n\rERROR: Overflowed!.");
}