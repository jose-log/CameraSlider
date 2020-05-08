
/*
* This file contains all the system states' functions that handle the motor
* movement configuration menus, and the motor needs to be moved either by the
* user or automatically.
*
* This is in constrast to the menu.c file that includes the menu system states
* functions where the motor is not moving.
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

// Automatic movement motor states.
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

/*-----------------------------------------------------------------------------
-------------------- FUNCTIONS RELATED TO MANUAL MOVEMENT ---------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* Manual speed: handles the manual speed control performed by the user through
* the rotary encoder. It constantly polls the rotary encoder state to read
* changes in speed, and issues a new speed movement.
*
* With the encoder button the user can return to the main menu
*/
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

/*===========================================================================*/
/*
* Manual position: handles the manual position control performed by the user
* through the rotary encoder. It constantly polls the rotary encoder state to
* read changes in position, and issues a new position movement.
*
* With the encoder button the user can return to the main menu
*/
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

/*-----------------------------------------------------------------------------
------------------ FUNCTIONS RELATED TO AUTOMATIC MOVEMENT --------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* Set initial/final position by the user: 
* The user can move the motor using speed control and the rotary encoder to
* position the motor at an initial/final point.
*
* With the encoder button the user can return to the main menu
*/
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

/*===========================================================================*/
/*
* Go to the initial position: Performs a blocking position movement until the
* slider is positioned at the initial point set by the user
*/
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

/*===========================================================================*/
/*
* Automatic movement execution.
* It reads the parameters stored in the 'm' structure and sets all parameters
* to automatically perform the desired movement.
*
* If 'Loop' is TRUE, the 'reps' parameter is ignored and the movement is an
* infinite loop. If it's FALSE, then the movement will repeat according to the
* value stored in 'reps'. 
* 
* Since the movement is based on going from initial position to final position
* and back, the coordination of these movements is handled within the switch()
* statement.
*
* The display is updated periodically with the elapsed time and percentage of 
* movement completed
*/
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

	DEBUG_P("\n\r> Go go go!");

	// Percentage calculation:
	total_steps = fabs(m.final_pos - m.initial_pos);
	if (m.reps > 1) total_steps *= 2 * (int32_t)m.reps;
	// debug:
	ltoa(total_steps, str, 10);
	DEBUG("\n\rtot steps: ");
	DEBUG(str);
	ltoa(m.initial_pos, str, 10);
	DEBUG(" | init: ");
	DEBUG(str);
	ltoa(m.final_pos, str, 10);
	DEBUG(" | final: ");
	DEBUG(str);

	// LCD screen:
	lcd_screen(SCREEN_GO);
	lcd_update_time_moving(secs);
	if (m.loop) lcd_write_loop();
	
	//debug
	ltoa(m.speed, str, 10);
	DEBUG("\n\rspd: ");
	DEBUG(str);
	ltoa(total_steps, str, 10);
	DEBUG(" | total: ");
	DEBUG(str);

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

		// movement coordination based on a series of states that depend on
		// the amount of repetitions and the Loop flag value.
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

/*===========================================================================*/
/*
* Sequence of Homing movements that move towards the beginning of the slider
* rail.
*/
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