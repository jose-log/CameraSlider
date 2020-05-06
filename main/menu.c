
/*
* This file contains all the system states' functions that handle the motor
* movement configuration menus, but the motor itself is not movig with these
* functions. 
* This is in constrast to the move.c file that includes the system states 
* functions where the motor is moving.
*/

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "menu.h"

#include <avr/pgmspace.h>
#include <util/delay.h>
//debug
#include <stdlib.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

static const uint16_t t[] PROGMEM = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	30, 45, 60, 80, 100, 120, 180, 300, 600, 1200, 2400, 3600, 7200, 14400,
	21600, 28800, 36000, 43200
};

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static int32_t find_speed_from_time(float a, float t, float x);

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

/******************************************************************************
*				FUNCTIONS RELATED TO AUTOMATIC MOVEMENT
******************************************************************************/

int32_t user_set_time(int32_t xi, int32_t xo)
{
/*
* The time selected by the user determines the speed at which the slider will
* move. According to the travel distance, there'll be a minimum time allowed,
* constrained by the maximum speed at which the slider can travel. 
* Here, the minimum time allowed is computed and shown to the user, based on
* the initial and final point, and the acceleration value set by the user.
*/
	uint8_t i = 0;
	float time;
	int32_t speed = 0;
	uint16_t x;
	struct btn_s *btn = button_get();
	struct enc_s *encoder = encoder_get();

	// Compute minimum time allowed based on the initial/final slider positions
	// and the acceleration/deceleration value.
	float ac = (float)motor_get_accel();
	float t_ramp = (SPEED_MAX / ac);
	float x_ramp = ac * pow(t_ramp, 2.0) / 2.0;
	float x_tot = fabs(xo - xi);		// absolute value
	float t_min;

	/* 
	* Two speed profiles are possible according to acceleration and xi/xo:
	*  v        Profile 1            v  Profile 2
	*  |      ______________         |
	*  |     /|            |\        |    /|\
	*  |    / |            | \       |   / | \
	*  |___/__|____________|__\__t   |__/__|__\____ t
	*       t1      t2      t1           t1 t1
	*
	* Profile 1: The slider reaches max speed and slided at constant speed for
	*		some x steps.
	* Profile 2: The slider does not reach max speed, but it accelerates and
	* 		decelerates without reaching constant speeds.
	* For each profiles, the minimum time is computed differently:
	*/
	if (x_tot > 2.0 * x_ramp)
		t_min = (2.0 * t_ramp) + (x_tot - (2.0 * x_ramp)) / SPEED_MAX;
	else
		t_min = 2.0 * sqrt((2.0 * (x_tot / 2.0)) / ac);

	// Compute maximum time allowed based on the minimum speed at which the
	// slider is able to move (maximum OCR1A value, w/out changing f)
	float t_max = x_tot / SPEED_MIN;
		
	//DEBUG:
	char str[12];
	ltoa((int32_t)ac, str, 10);
	uart_send_string("\n\rac: ");
	uart_send_string(str);
	dtostre(t_ramp, str, 3, 0x00);
	uart_send_string("\n\rt_ramp: ");
	uart_send_string(str);
	ltoa((int32_t)x_ramp, str, 10);
	uart_send_string("\n\rx_ramp: ");
	uart_send_string(str);
	ltoa((int32_t)x_tot, str, 10);
	uart_send_string("\n\rx_tot: ");
	uart_send_string(str);
	dtostre(t_min, str, 3, 0x00);
	uart_send_string("\n\rt_min: ");
	uart_send_string(str);
	dtostre(t_max, str, 3, 0x00);
	uart_send_string("\n\rt_max: ");
	uart_send_string(str);

	// minimum index is the value of minimum time (in seconds) rounded to 
	// the lower integer
	for (uint8_t m = 0; m < sizeof(t)/sizeof(uint16_t); m++) {
		float v = pgm_read_word(&t[m]);
		if (v > t_min) {
			if (m > 0) i = m - 1;
			else i = 0;
			break;
		}
	}
	
	// LCD screen
	time = t_min;
	lcd_screen(SCREEN_CHOOSE_TIME);
	lcd_update_time(time);

	uart_send_string_p(PSTR("\n\r> Time duration"));

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if (encoder->update) {
			encoder->update = FALSE;
			float v = (float)pgm_read_word(&t[i]);
			if (encoder->dir == CW) {
				if (i < sizeof(t)/sizeof(uint16_t))
					if (v < t_max)
						i++;
			} else if (encoder->dir == CCW) {
				if (i > 0)
					if (v > t_min)
						i--;
			}

			v = (float)pgm_read_word(&t[i]);
			if (v >= t_max)	time = t_max;
			else if (v <= t_min) time = t_min;
			else time = v;

			lcd_update_time(time);
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
			speed = -1;
			break;
		}
	}

	// Find the speed value from the time the user chose.
	if (speed != -1) {
		if (time == t_min)
			speed = SPEED_MAX;
		else
			speed = find_speed_from_time(ac, time, x_tot);
	}
	
	return speed;
}

int8_t user_set_reps(void)
{
	int8_t i = 1;
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
				if (i > 1)
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
	lcd_update_reps(i);
	uart_send_string_p(PSTR("\n\r> Acceleration"));

	motor_set_accel_percent(i);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder->update){
			encoder->update = FALSE;
			if (encoder->dir == CW) {
				if (i < 100) i += 5;
				else i = 100;
			} else if (encoder->dir == CCW) {
				if (i > 1) i -= 5;
				else i = 0;
			}
			lcd_update_reps(i);
			motor_set_accel_percent(i);
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
	_delay_ms(1000);
}

static int32_t find_speed_from_time(float a, float t, float x)
{
/* To find the speed at which the slider must travel in order to spend t time
* a quadratic expression must be solved.
*  v
*  |      ______________
*  |     /|            |\
*  |    / |            | \
*  |___/__|____________|__\__t
*       t1      t2      t1
*/
	float _a = a;
	float _b = -1.0 * a * t;
	float _c = x;

	float _b2 = pow(_b, 2.0);
	float _4ac = 4.0 * _a * _c;

	float root = sqrt(_b2 - _4ac);
	float t1 = ((-1.0 * _b) - root) / (2.0 * _a);

	int32_t v = (int32_t)(t1 * a);

	return v;
}