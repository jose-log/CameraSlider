
#include "config.h"
#include "driver.h"
#include "init.h"
#include "timers.h"
#include "motor.h"
#include "uart.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

int main(void){

	boot();	

	uart_send_string("\n\r----------------------------------");
	uart_send_string("\n\rHello World!");

	motor_init();

	sei();
	
	/*
	motor_move_to_pos_block(10000, REL);
	motor_set_accel_percent(20);
	motor_move_to_pos_block(10000, REL);
	motor_set_maxspeed_percent(20);
	motor_move_to_pos_block(10000, REL);
	*/
	motor_move_at_speed(10);
	_delay_ms(2000);
	motor_move_at_speed(50);
	_delay_ms(2000);
	motor_move_at_speed(100);
	_delay_ms(2000);
	motor_move_at_speed(50);
	_delay_ms(2000);
	motor_move_at_speed(-100);
	while(1);

	return 0;
}