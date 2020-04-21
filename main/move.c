

#include "move.h"
#include "config.h"
#include "driver.h"
#include "timers.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

uint8_t move_to_position(uint16_t pos, uint16_t speed){

	if(slider.position < pos)
		drv_spin_direction(CW);
	else
		drv_spin_direction(CCW);

	speed_set(speed);
	while(slider.position != pos);
	speed_set(0);

	return TRUE;
}

uint8_t move_relative(uint16_t rel_pos, uint8_t dir, uint16_t speed){

	uint16_t pos;

	if(dir == CW){
		drv_spin_direction(CW);
		pos = slider.position + rel_pos;
	} else if(dir == CCW){
		drv_spin_direction(CCW);
		pos = slider.position - rel_pos;
	} else {
		pos = slider.position;
	}

	speed_set(14);
	while(slider.position != pos);
	speed_set(0);

	return TRUE;
}

void homing_cycle(void){
	
	// spin towards limit switch
	drv_spin_direction(CCW);
	timer_speed_set(ENABLE, (480/15));
	// Wait until switch is pressed
	while(!slider.sw);
	// stop immediately
	timer_speed_set(DISABLE, 0);
	// wait for 100ms
	_delay_ms(100);
	slider.sw = FALSE;
	
	
	// pull-off movement:
	// get away from the switch to un-press it
	drv_spin_direction(CW);
	timer_speed_set(ENABLE, (480/10));
	_delay_ms(200);
	timer_speed_set(DISABLE, 0);
	_delay_ms(50);
	// approach the switch again, but slower
	drv_spin_direction(CCW);
	timer_speed_set(ENABLE, (480/5));
	// wait until switch is pressed
	while(!slider.sw);
	// stop immediately
	timer_speed_set(DISABLE, 0);
	// wait for 100ms
	_delay_ms(100);

	//pull-off again, to avoid permanent contact with the switch
	drv_spin_direction(CW);
	timer_speed_set(ENABLE, (480/5));
	_delay_ms(200);
	timer_speed_set(DISABLE, 0);

	// ZERO position
	slider.position = 0;
	slider.out_of_bounds = TRUE;
}

void speed_set(uint16_t speed){
/*
* If speed is zero: disable speed-timer
* If speed is non zero: check whether the speed-timer is already running
* 	- if so: enable speed update in the next speed-timer interrupt
*	- if not: enale immediately the speed-timer, resetting internal counter
*	  to zero
*/
	if(speed == 0){
		timer_speed_set(DISABLE, speed);
	} else {
		slider.speed = speed;
		if(timer_speed_check()){
			speed_update = TRUE;
		} else {
			slider.speed = speed_limit_correction();
			timer_speed_set(ENABLE, slider.speed);
		}
	}
}
/*	
const uint16_t speed_limit_vector[] PROGMEM = {
	700,500,378,217,152,117,95,80,69,61,54,49,45,41,38,35,33,31,29,28,26,25,
	24,23,22,21,20,19,19,18,17,17,16,16,15,15,14,14
};
*/
const uint16_t speed_limit_vector[] PROGMEM = {
	150,79,58,47,41,37,34,32,30,28,27,25,24,23,23,22,21,21,20,19,19,18,18,18,
	17,17,17,16,16,16,16,15,15,15,15,14,14,14
};


#define UPPER_ZONE 	0xAA
#define LOWER_ZONE 	0xFF

uint16_t speed_limit_correction(void){
/* 
* Checks for current position and corrects speed if it's moving towards the
* limit boundaries. If it's moving away from the boundary, does not limit
* speed.
*/
	uint32_t pos;
	uint16_t speed_limit;
	uint8_t speed_limit_flag = TRUE;
	uint8_t slider_zone;


	if(slider.position < SAFETY_DISTANCE){
		pos = slider.position;
		slider.marginal_zone = TRUE;
		slider_zone = LOWER_ZONE;
	} else if((MAX_COUNT - slider.position) < SAFETY_DISTANCE){
		pos = MAX_COUNT - slider.position;
		slider.marginal_zone = TRUE;
		slider_zone = UPPER_ZONE;
	} else {
		speed_limit_flag = FALSE;
		slider.marginal_zone = FALSE;
	}

	if(speed_limit_flag){
		if(((slider_zone == LOWER_ZONE) && (slider.spin == CCW)) ||
		   ((slider_zone == UPPER_ZONE) && (slider.spin == CW))){
			pos >>= 6;
			speed_limit = pgm_read_word(speed_limit_vector + pos);
			if(slider.speed < speed_limit){
				slider.speed = speed_limit;
			}
		}
	}

	return slider.speed;
}