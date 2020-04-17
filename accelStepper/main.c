
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

	boot();	

	uart_send_string("\n\r----------------------------------");
	uart_send_string("\n\rHello World!");

	stepper_init();

	sei();
	
	stepper_move_to_pos(-10000, REL);
	while(current_pos > -5000);
	stepper_move_to_pos(0, REL);
	//stepper_stop();
	
	while(1);

	return 0;
}