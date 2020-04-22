

#include "menu.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

int8_t homing(void){
/*
* Homing cycle:
*/
	lcd_screen(SCREEN_HOMING);
	uart_send_string_p(PSTR("\n\r> Homing..."));
	homing_cycle();
	lcd_screen(SCREEN_HOMING_DONE);
	uart_send_string_p(PSTR(" DONE!"));

	// should implement a security bounds check while homing
	return 0;
}

int8_t choose_action(void){
/*
* Select what to do:
* - Create a movement
* - Perform a manual movement
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	int8_t out = 0;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();

	lcd_screen(SCREEN_CHOOSE_ACTION);
	uart_send_string_p(PSTR("\n\r> Automatic or Manual Action"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();

		// lcd options
		if(encoder->update){
			encoder->update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				out = 1;
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				out = 0;
			}
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			break;
		}
	}	

	return out;
}

int8_t choose_control_type(void){
/*
* Select either position or speed control
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	int8_t out = 0;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_CONTROL_TYPE);
	uart_send_string_p(PSTR("\n\r> Position or Speed Control"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder->update){
			encoder->update = FALSE;
			toggle ^= 1;
			if(toggle){		// Speed control
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				out = 1;
			} else {		// Position control
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				out = 0;
			}
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			break;
		}
		if((btn->action) && (btn->delay3)){
			btn->action = FALSE;
			out = -1;
			break;
		}
	}

	return out;
}

int8_t choose_speed_profile(void){
/*
* Select either linear or exponential movement
*/
	uint8_t toggle = FALSE;
	int8_t out = 0;
	uint16_t x;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_SPEED_PROFILE);
	uart_send_string_p(PSTR("\n\r> Linear or Quadratic profile"));

	// default profile: Linear
	motor_set_speed_profile(PROFILE_LINEAR);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder->update){
			encoder->update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				motor_set_speed_profile(PROFILE_QUADRATIC);
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				motor_set_speed_profile(PROFILE_LINEAR);
			}
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			break;
		}
		if((btn->action) && (btn->delay3)){
			btn->action = FALSE;
			out = -1;
			break;
		}
	}

	return out;
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

			if(encoder->dir == CW) 
				i = motor_get_speed_percent() + 5;
				//i += 5;
			else if (encoder->dir == CCW) 
				i = motor_get_speed_percent() - 5;
				//i -= 5;
			
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

void user_movement(void){

	// LCD screen:
	//lcd_screen(SCREEN_INITIAL_POS);	
	
}

void fail_message(void){

	lcd_screen(SCREEN_FAIL_MESSAGE);	
}