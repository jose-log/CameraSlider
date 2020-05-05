

/*
* This file contains all the system states' functions that handle the motor
* when it's moving.
* This is in constrast to the move.c file that includes the system states 
* functions where the motor is moving.
*/

#include "move.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

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

int8_t manual_speed(void)
{
	int8_t i = 0;
	uint16_t x, xi = 0;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();

	// LCD screen:
	lcd_screen(SCREEN_MOTOR_SPEED);
	lcd_update_speed(motor_get_speed());
	uart_send_string_p(PSTR("\n\r> Speed Control"));

	// Trim motor parameters
	motor_set_maxspeed_percent(100);
	motor_set_accel_percent(50);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		xi++;
		
		if(encoder->update){
			encoder->update = FALSE;

			if(encoder->dir == CW) {
				i = motor_get_speed_percent() + 5;
				if ((i > 0) && (i < 5)) i = 0;	// force zero speed when transitioning from + to -
			} else if (encoder->dir == CCW) {
				i = motor_get_speed_percent() - 5;
				if ((i > -5) && (i < 0)) i = 0;	// force zero speed when transitioning from + to -
			}
			
			if(i > 100) i = 100;
			else if(i < -100) i = -100;

			motor_move_at_speed(i);
		}

		// update display every 100ms
		if (xi == 100) {
			lcd_update_speed(motor_get_speed());
			xi = 0;
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if(btn->action && btn->delay3){
			btn->action = FALSE;
			motor_move_at_speed(0);
			break;
		}
	}

	return 0;
}

uint8_t manual_position(void)
{
	uint16_t x, xi = 0;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();

	// LCD screen:
	lcd_screen(SCREEN_MOTOR_POSITION);
	lcd_update_position(motor_get_position());
	uart_send_string_p(PSTR("\n\r> Position Control"));

	// Trim motor parameters
	motor_set_maxspeed_percent(100);
	motor_set_accel_percent(50);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		xi++;
		
		if(encoder->update){
			encoder->update = FALSE;

			if(encoder->dir == CW) motor_move_to_pos(600, REL, TRUE);
			else if (encoder->dir == CCW) motor_move_to_pos(-600, REL, TRUE);			
		}

		// update display every 100ms
		if (xi == 100) {
			lcd_update_position(motor_get_position());
			xi = 0;
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if(btn->action && btn->delay3){
			btn->action = FALSE;
			motor_move_at_speed(0);
			break;
		}
	}

	return 0;
}

int32_t user_set_position(uint8_t p)
{
	int8_t i = 0;
	uint16_t x, xi = 0;
	uint8_t out = TRUE;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();

	// LCD screen:
	if (p) {	// If TRUE
		lcd_screen(SCREEN_FINAL_POSITION);
		uart_send_string_p(PSTR("\n\r> Initial Position"));
	} else {	// If FALSE
		lcd_screen(SCREEN_INITIAL_POSITION);
		uart_send_string_p(PSTR("\n\r> Final Position"));
	}
	lcd_update_position(motor_get_position());

	// Trim motor parameters
	motor_set_speed_profile(PROFILE_LINEAR);
	motor_set_maxspeed_percent(100);
	motor_set_accel_percent(100);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		xi++;
		
		if(encoder->update){
			encoder->update = FALSE;

			if(encoder->dir == CW) {
				i = motor_get_speed_percent() + 5;
				if ((i > 0) && (i < 5)) i = 0;	// force zero speed when transitioning from + to -
			} else if (encoder->dir == CCW) {
				i = motor_get_speed_percent() - 5;
				if ((i > -5) && (i < 0)) i = 0;	// force zero speed when transitioning from + to -
			}

			if(i > 100) i = 100;
			else if(i < -100) i = -100;

			motor_move_at_speed(i);
		}

		// update display every 100ms
		if (xi == 100) {
			lcd_update_position(motor_get_position());
			xi = 0;
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			out = TRUE;
			break;
		}

		if(btn->action && btn->delay3){
			btn->action = FALSE;
			out = FALSE;
			break;
		}
	}
	motor_move_at_speed(0);

	return (out ? motor_get_position() : -1);
}

int8_t user_go_to_init(int32_t pos)
{
	int8_t out = FALSE;
	uint16_t x, xi = 0;
	struct btn_s *btn = button_get();

	// LCD screen:
	lcd_screen(SCREEN_WAIT_TO_GO);

	// Trim motor parameters
	// acceleration was already set by the user.
	// Max speed is computed and set properly in user_gogogo()
	motor_set_maxspeed_percent(100);
	
	motor_move_to_pos_block(pos, ABS, TRUE);

	while (TRUE) {

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		xi++;

		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			out = TRUE;
			break;
		}

		if(btn->action && btn->delay3){
			btn->action = FALSE;
			out = -1;
			break;
		}
	}

	return out;
}

int8_t user_gogogo(struct auto_s m)
{
	int8_t out = FALSE;
	uint16_t x, xi = 0;
	uint16_t secs = 0;
	int32_t steps_ahead = m.final_pos - m.initial_pos;
	struct btn_s *btn = button_get();

	int8_t state = 0;

	// LCD screen:
	lcd_screen(SCREEN_GO);
	uart_send_string_p(PSTR("\n\r> Go go go!"));
	lcd_update_time_remaining(secs);
	
	// Trim motor parameters. Requires revision: Integer operations may lead to lack of resolution
	// when dealing with very low speeds
	
	//debug
	char str[12];
	ltoa(m.speed, str, 10);
	uart_send_string("\n\rspd: ");
	uart_send_string(str);
	ltoa(steps_ahead, str, 10);
	uart_send_string(" | ahead: ");
	uart_send_string(str);

	motor_set_speed_profile(PROFILE_LINEAR);
	motor_set_maxspeed((float)m.speed);
	motor_set_accel_percent((uint8_t)m.accel);

	uint8_t current_rep = 0;

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		xi++;

		switch (state) {
			case 0:
				// Go without blocking movement
				motor_move_to_pos(steps_ahead, ABS, TRUE);
				state = 1;
				break;

			case 1:
				// poll until it reaches the final position.
				if (motor_get_position() == m.final_pos) {
					if (m.reps == 1) {
						// Finished.
						state = 4;
					} else {
						// go back to init_pos
						steps_ahead = m.initial_pos - motor_get_position();
						state = 2;
					}
				}
				break;

			case 2:
				// Go without blocking movement
				motor_move_to_pos(steps_ahead, ABS, TRUE);
				state = 3;
				break;

			case 3:
				// poll until it reaches the final position.
				if (motor_get_position() == m.initial_pos) {
					current_rep++;
					if (m.reps == current_rep) {
						state = 4;
					} else {
						steps_ahead = m.final_pos - motor_get_position();
						state = 0;
					}
				}
				break;

			case 4:
				uart_send_string_p(PSTR("\n\r < FIISHED >"));
				state = 5;
				break;

			case 5:
				break;
				
		}
		
		// update display every 100ms
		if (!(xi % 1000) && (state != 5)) {
			secs++;
			lcd_update_time_remaining(secs);
			xi = 0;
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			out = TRUE;
			motor_stop(HARD_STOP);
			break;
		}

		if(btn->action && btn->delay3){
			btn->action = FALSE;
			out = -1;
			motor_stop(SOFT_STOP);
			break;
		}
	}

	return out;
}