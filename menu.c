
#include "menu.h"
#include "config.h"
#include "driver.h"
#include "lcd.h"
#include "move.h"
#include "timers.h"
#include "util.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

uint8_t homing(void){
/*
* Homing cycle:
*/
	lcd_screen(SCREEN_HOMING);
	homing_cycle();
	lcd_screen(SCREEN_HOMING_DONE);

	// should implement a security bounds check while homing
	return 0;
}

state_t choose_movement(void){
/*
* Select movement
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	state_t state = STATE_CREATE_MOVEMENT;

	lcd_screen(SCREEN_CHOOSE_MOVEMENT);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();

		// lcd options
		if(encoder.update){
			encoder.update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				state = STATE_CHOOSE_MANUAL_MOVEMENT;
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				state = STATE_CREATE_MOVEMENT;
			}
		}
		
		// Check encoder button
		if(btn.query) btn_check();		
		
		// Check action to be taken
		if((btn.action) && (btn.state == BTN_RELEASED) && (!btn.delay1)){
			btn.action = FALSE;
			break;
		}
	}	

	return state;
}

uint8_t choose_manual_movement(void){
/*
* Select either linear or exponential movement
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	uint8_t type = SPEED_LINEAR;
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_MANUAL_MOVEMENT);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder.update){
			encoder.update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				type = SPEED_EXPONENTIAL;
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				type = SPEED_LINEAR;
			}
		}
		
		// Check encoder button
		if(btn.query) btn_check();
		
		// Check action to be taken
		if((btn.action) && (btn.state == BTN_RELEASED) && (!btn.delay1)){
			btn.action = FALSE;
			break;
		}
		if((btn.action) && (btn.delay3)){
			btn.action = FALSE;
			type = QUIT_MENU;
			break;
		}
	}

	return type;
}

/*
const uint16_t exponential_speed[] PROGMEM = {
	1645,1470,1308,1160,1025,902,790,689,598,516,443,378,
	320,270,226,187,154,126,102,82,66,53,42,34,27,23,19,
	17,16,15,14,14,14
	};
*/
const uint16_t exponential_speed[] PROGMEM = {
	14336,3584,1593,896,573,398,293,224,177,143,118,100,85,
	73,64,56,50,44,40,36,33,30,27,25,23,21,20,18,17,16,15,14
	};

uint8_t manual_movement(uint8_t type){

	int8_t n = 0;
	uint16_t x;
	uint16_t speed = 0;
	uint8_t aux = FALSE;

	// LCD screen:
	if(type == SPEED_LINEAR)
		lcd_screen(SCREEN_LINEAR_SPEED);	
	else if(type == SPEED_EXPONENTIAL)
		lcd_screen(SCREEN_EXPONENTIAL_SPEED);
	lcd_update_speed(speed);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		// !!! there's still a problem with the speed the LCD shows: when in
		// marginal zone, the function lcd_update speed doesn't know that the
		// speed was corrected!. THus, it shows a wrong speed!
		if(encoder.update){
			encoder.update = FALSE;
			
			if(slider.out_of_bounds)
				n = 0;
			if(encoder.dir) n--;
			else n++;
			

			if(n > 32) n = 32;
			else if(n < -32) n = -32;

			if(n < 0) drv_spin_direction(CCW);
			else drv_spin_direction(CW);
			
			if(n == 0){
				speed = 0;
				speed_set(speed);
			} else {
				if((!slider.out_of_bounds) ||
				   ((slider.spin == CW) && (slider.position < MAX_COUNT)) ||
				   ((slider.spin == CCW) && (slider.position > 0))){
					if(type == SPEED_LINEAR) 
						speed = 448/abs(n);
					else if(type == SPEED_EXPONENTIAL) 
						speed = pgm_read_word(exponential_speed + abs(n) - 1);
					speed_set(speed);
				} else {
					speed = 0;
				}
			}
			lcd_update_speed(speed);
		}
		
		// this little snippet should be re-arranged. Not elegant.
		if(slider.out_of_bounds){
			if(!aux){
				speed = 0;
				lcd_update_speed(speed);
				aux = TRUE;
			}
		} else {
			aux = FALSE;
		}

		
		// Check encoder button
		if(btn.query) btn_check();
		
		// Check action to be taken
		if((btn.action) && (btn.delay3)){
			btn.action = FALSE;
			speed_set(0);
			break;
		}
	}

	return 0;
}

void user_movement(void){

	// LCD screen:
	//lcd_screen(SCREEN_INITIAL_POS);	
	
}