
/*
* This file contains all the system states' functions that handle the motor
* movement configuration menus, but the motor itself is not movig with these
* functions. 
* This is in constrast to the move.c file that includes the system states 
* functions where the motor is moving.
*/

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
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
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

	return toggle;
}

int8_t choose_control_type(void){
/*
* Select either position or speed control
*/
	int8_t toggle = FALSE;
	uint16_t x;
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
			} else {		// Position control
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
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
			toggle = -1;
			break;
		}
	}

	return toggle;
}

int8_t choose_speed_profile(void){
/*
* Select either linear or exponential movement
*/
	int8_t toggle = FALSE;
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
			toggle = -1;
			break;
		}
	}

	return toggle;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

static const uint16_t t[] PROGMEM = {
	30, 45, 60, 80, 100, 120, 180, 300, 600, 1200, 2400,
	3600, 7200,	14400, 21600, 28800, 36000, 43200
};

int32_t user_set_time(void)
{
	uint8_t i = 1;
	int32_t out = 1;
	uint16_t x;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_TIME);
	lcd_update_time(out);
	uart_send_string_p(PSTR("\n\r> Time duration"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if (encoder->update) {
			encoder->update = FALSE;
			if (encoder->dir == CW) {
				if (i < 20 + (sizeof(t)/sizeof(uint16_t)))
					i++;
			} else if (encoder->dir == CCW) {
				if (i > 1)
					i--;
			}
			if (i <= 20) out = (uint16_t)i;
			else out = pgm_read_word(&t[i - 21]);
			lcd_update_time(out);
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

int8_t user_set_reps(void)
{
	int8_t i = 0;
	uint16_t x;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_REPS);
	lcd_update_reps(i);
	uart_send_string_p(PSTR("\n\r> Repetitions"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if (encoder->update) {
			encoder->update = FALSE;
			if (encoder->dir == CW) {
				if (i < 20)
					i++;
			} else if (encoder->dir == CCW) {
				if (i > 0)
					i--;
			}
			lcd_update_reps(i);
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
			i = -1;
			break;
		}
	}

	return i;
}

int8_t user_set_loop(void)
{
	uint8_t toggle = FALSE;
	uint16_t x;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_LOOP);
	uart_send_string_p(PSTR("\n\r> Loop"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder->update){
			encoder->update = FALSE;
			toggle ^= 1;
			lcd_update_loop(toggle);
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
			toggle = -1;
			break;
		}
	}

	return toggle;
}

int8_t user_set_accel(void)
{
	int8_t i = 100;
	uint16_t x;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_ACCEL);
	motor_set_accel_percent(i);
	lcd_update_reps(i);
	uart_send_string_p(PSTR("\n\r> Acceleration"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder->update){
			encoder->update = FALSE;
			if (encoder->dir == CW) {
				if (i < 100)
					i += 5;
			} else if (encoder->dir == CCW) {
				if (i > 1)
					i -= 5;
			}
			motor_set_accel_percent(i);
			lcd_update_reps(i);
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
			i = -1;
			break;
		}
	}

	return i;
}

void fail_message(void){

	lcd_screen(SCREEN_FAIL_MESSAGE);	
}