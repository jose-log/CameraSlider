

#include "move.h"

#include <util/delay.h>
#include <avr/pgmspace.h>

int8_t homing_cycle(void){
	
	int8_t x = -1;

	// Shortcut: 
	// If encoder button is pressed, jump the HOMING routine and exit successfully
	if (button_test()) {
		goto exit;
		x = 0;
	}

	motor_set_maxspeed_percent(30);	// 30% of max speed
	motor_set_accel_percent(30);	// 30% of max accel
	// Check state of the switch. If it's pressed, get away from it
	if (limit_switch_test())
		motor_move_to_pos_block(800, REL, FALSE);
		_delay_ms(100);
	
	// spin towards limit switch
	if (motor_move_to_pos_block(-80000, REL, FALSE) >= 0) {	// move up to 80.000 steps
		uart_send_string_p(PSTR("ERROR. Can't detect limit"));
		goto exit;
	}
	limit_switch_ISR(DISABLE);		// disable ISR while slider pulls back again
	uart_send_string_p(PSTR("|"));
	_delay_ms(100);

	// pull-off movement:
	// get away from the switch to un-press it
	motor_move_to_pos_block(400, REL, FALSE);
	_delay_ms(50);

	// approach the switch again, but slower
	limit_switch_ISR(ENABLE);		// enable ISR again
	motor_set_maxspeed_percent(10);	// 10% of max speed
	motor_set_accel_percent(10);	// 10% of max accel
	if (motor_move_to_pos_block(-1600, REL, FALSE) >= 0) {	// move up to 1600 steps
		uart_send_string_p(PSTR("ERROR. Can't detect limit"));
		goto exit;
	}
	limit_switch_ISR(DISABLE);		// disable ISR while slider pulls back again
	uart_send_string_p(PSTR("|"));
	_delay_ms(200);

	//pull-off again, to avoid permanent contact with the switch
	limit_switch_ISR(ENABLE);		// enable ISR again
	motor_set_maxspeed_percent(100);// 100% of max speed
	motor_set_accel_percent(100);	// 100% of max accel
	motor_move_to_pos_block(400, REL, FALSE);
	_delay_ms(100);

	// ZERO position
	motor_set_position(0);
	x = 0;

exit:
	return x;
}