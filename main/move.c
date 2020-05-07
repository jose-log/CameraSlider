/*
* This file contains all the system states' functions that handle the motor
* when it's moving.
* This is in constrast to the move.c file that includes the system states 
* functions where the motor is moving.
*/
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "move.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

enum {
	ST_MOVE_TO_XO,
	ST_MOVE_TO_XI,
	ST_POLLING_XO,
	ST_POLLING_XI,
	ST_FINISH,
	ST_STOP,
	ST_IDLE
};

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static int8_t homing_cycle(void);

/*===========================================================================*/
/*
* Homing cycle: performs the initial calibration routine
*/
int8_t homing(void)
{
	lcd_screen(SCREEN_HOMING);
	uart_send_string_p(PSTR("\n\r> Homing..."));
	homing_cycle();
	lcd_screen(SCREEN_HOMING_DONE);
	uart_send_string_p(PSTR(" DONE!"));
	_delay_ms(1000);

	// should implement a security bounds check while homing
	return 0;
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
	int32_t total_steps, percentage;
	uint8_t n_move = 0;
	int32_t steps_completed = 0;
	struct btn_s *btn = button_get();

	int8_t state = 0;

	//debug
	char str[12];

	// Percentage calculation:
	total_steps = fabs(m.final_pos - m.initial_pos);
	if (m.reps > 1) total_steps *= 2 * (int32_t)m.reps;
	// debug:
	ltoa(total_steps, str, 10);
	uart_send_string("\n\rtot steps: ");
	uart_send_string(str);
	ltoa(m.initial_pos, str, 10);
	uart_send_string(" | init: ");
	uart_send_string(str);
	ltoa(m.final_pos, str, 10);
	uart_send_string(" | final: ");
	uart_send_string(str);

	// LCD screen:
	lcd_screen(SCREEN_GO);
	uart_send_string_p(PSTR("\n\r> Go go go!"));
	lcd_update_time_moving(secs);
	if (m.loop) lcd_write_loop();
	
	//debug
	ltoa(m.speed, str, 10);
	uart_send_string("\n\rspd: ");
	uart_send_string(str);
	ltoa(total_steps, str, 10);
	uart_send_string(" | total: ");
	uart_send_string(str);

	// Trim motor parameters.
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
			case ST_MOVE_TO_XO:
				// Go without blocking movement
				motor_move_to_pos(m.final_pos, ABS, TRUE);
				state = ST_POLLING_XO;
				break;

			case ST_POLLING_XO:
				// poll until it reaches the final position.
				if (motor_get_position() == m.final_pos) {
					n_move++;
					if ((m.reps == 1) && (!m.loop))	
						state = ST_FINISH;	
					else	
						state = ST_MOVE_TO_XI;	
				}
				break;

			case ST_MOVE_TO_XI:
				// Go without blocking movement
				motor_move_to_pos(m.initial_pos, ABS, TRUE);
				state = ST_POLLING_XI;
				break;

			case ST_POLLING_XI:
				// poll until it reaches the final position.
				if (motor_get_position() == m.initial_pos) {
					n_move++;
					current_rep++;
					if ((m.reps == current_rep) && (!m.loop))
						state = ST_FINISH;
					else
						state = ST_MOVE_TO_XO;
				}
				break;

			case ST_FINISH:
				lcd_screen(SCREEN_FINISHED);
				uart_send_string_p(PSTR("\n\r < FINISHED >"));
				state = ST_IDLE;
				break;

			case ST_STOP:
				lcd_screen(SCREEN_STOP);
				uart_send_string_p(PSTR("\n\r < STOPPED >"));
				state = ST_IDLE;
				break;

			default:
				break;
		}
		
		// update display every 100ms
		if (!(xi % 1000) && (state != ST_FINISH) && (state != ST_IDLE)) {
			secs++;
			xi = 0;
			lcd_update_time_moving(secs);
			if (!m.loop) {
				// Compute steps completed
				steps_completed = fabs(m.final_pos - m.initial_pos) * n_move;
				if (m.final_pos > m.initial_pos) {
					if (motor_get_dir() == CW)
						steps_completed += motor_get_position() - m.initial_pos;
					else
						steps_completed += m.final_pos - motor_get_position();
				} else {
					if (motor_get_dir() == CW)
						steps_completed += motor_get_position() - m.final_pos;
					else
						steps_completed += m.initial_pos - motor_get_position();
				}
				ltoa(steps_completed, str, 10);
				uart_send_string("\n\rcompleted: ");
				uart_send_string(str);
				percentage = (steps_completed * 100) / total_steps;
				lcd_update_percent((int8_t)percentage);
			}			
		}
		
		// Check encoder button
		if(btn->query) button_check();
		
		// Check action to be taken
		if((btn->action) && (btn->state == BTN_RELEASED) && (!btn->delay1)){
			btn->action = FALSE;
			if (timer_speed_check()) {
				// motor still moving. PANIC BUTTON.
				motor_stop(HARD_STOP);
				state = ST_STOP;
			} else {
				// motor already finished. Repeat movement
				out = TRUE;
				break;
			}
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

/*-----------------------------------------------------------------------------
--------------------- I N T E R N A L   F U N C T I O N S ---------------------
-----------------------------------------------------------------------------*/

static int8_t homing_cycle(void){
	
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
		uart_send_string_p(PSTR("\n\rERROR. Can't detect limit"));
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