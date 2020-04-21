

#include "move.h"

#include <util/delay.h>
#include <avr/pgmspace.h>

void homing_cycle(void){
	
	volatile uint8_t *sw = limit_switch_get();
	volatile uint8_t *dir = motor_get_dir();

	uart_send_string_p(PSTR("\n\r> Homing..."));

	// spin towards limit switch
	drv_dir(CCW, dir);
	timer_speed_set(ENABLE, (480/15));
	// Wait until switch is pressed
	while(!(*sw));
	// stop immediately
	timer_speed_set(DISABLE, 0);
	// wait for 100ms
	_delay_ms(100);
	*sw = FALSE;
	
	
	// pull-off movement:
	// get away from the switch to un-press it
	drv_dir(CW, dir);
	timer_speed_set(ENABLE, (480/10));
	_delay_ms(200);
	timer_speed_set(DISABLE, 0);
	_delay_ms(50);
	// approach the switch again, but slower
	drv_dir(CCW, dir);
	timer_speed_set(ENABLE, (480/5));
	// wait until switch is pressed
	while(!(*sw));
	// stop immediately
	timer_speed_set(DISABLE, 0);
	// wait for 100ms
	_delay_ms(100);

	//pull-off again, to avoid permanent contact with the switch
	drv_dir(CW, dir);
	timer_speed_set(ENABLE, (480/5));
	_delay_ms(200);
	timer_speed_set(DISABLE, 0);

	// ZERO position
	motor_set_position(0);
	uart_send_string_p(PSTR(" DONE!"));
}