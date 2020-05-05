/*
 * dev.c
 *
 * Created: 15-Oct-18 8:06:03 PM
 * Author : josel
 */ 

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "init.h"
#include "menu.h"
#include "timers.h"
#include "util.h"

#include "motor.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

typedef enum {
	STATE_HOMING,
	STATE_CHOOSE_ACTION,
	STATE_MANUAL_MOVEMENT,
	STATE_CREATE_MOVEMENT,
	STATE_FAIL
} state_t;

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

volatile static state_t system_state = STATE_HOMING;
volatile uint16_t ms = 0;
struct auto_s automatic;

/******************************************************************************
*************************** M A I N   P R O G R A M ***************************
******************************************************************************/

int main(void)
{
	boot();

	// Enable global Interrupts
	sei();

	// misc vars for retrieved arguments in menu functions
	int32_t x = 0;

	while(TRUE){

		/*
		* SYSTEM MENU STRUCTURE:
		*
		* - Create Movement: (under construction)
		* - Manual Movement:
		* 	- Position control
		*		- Linear profile
		*		- Quadratic profile
		*	- Speed control
		*		- Linear profile
		*		- Quadratic profile
		*/
		switch(system_state){

			/* 
			* HOMING. Initial positioning sequence to place the slider at the
			* beginning of the rails, and set the reference position for all
			* other movements.
			*/
			case STATE_HOMING:
				if(!homing()) system_state = STATE_CHOOSE_ACTION;
				else system_state = STATE_FAIL;
				break;

			/*
			* CHOOSE ACTION: Two options are displayed:
			*	- Create movement: Create a movement profile to be executed
			*		automatically by the slider
			* 	- Manual movement: real-time control of the slider by using
			*		the rotary encoder
			*/
			case STATE_CHOOSE_ACTION:		// Automatic or Manual movement
				if (choose_action()) system_state = STATE_MANUAL_MOVEMENT;	// Manual Movement
				else system_state = STATE_CREATE_MOVEMENT;					// Create Movement
				break;

			case STATE_MANUAL_MOVEMENT:

				/*
				* CONTROL TYPE: Two options are displayed:
				* 	- Position control: encoder varies the slider position 
				* 	- Speed control: encoder varies the slider speed
				*/
				x = choose_control_type();
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				}

				/*
				* CHOOSE SPEED PROFILE: Two options are displayed:
				* 	- Linear: Speed increases/decreases linearly
				* 	- Quadratic: Speed increases/decreases as in a squared function
				* 		This profile is less sensible at low speeds and more
				*		sensible at high speeds
				*/
				// STATE_CHOOSE_SPEED_PROFILE
				if(choose_speed_profile() < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				}

				if (x == 0) {
					/*
					* POSITION CONTROL: MOTOR CAN BE MOVED and the rotary encoder 
					*	changes its position
					*/
					manual_position();
				} else if (x == 1) {
					/*
					* SPEED CONTROL: MOTOR CAN BE MOVED and the rotary encoder changes
					*	its velocity
					*/
					manual_speed();
				}
				
				system_state = STATE_CHOOSE_ACTION;
				break;

			/*
			* CREATE MOVEMENT: under construction
			*/
			case STATE_CREATE_MOVEMENT:
			
				/*
				* INITIAL POSITION: Moves motor using speed control, and
				* user places the motor at the initial point of movement 
				*/
				x = user_set_position(FALSE);	// FALSE means initial pos
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.initial_pos = x;
				}

				/*
				* FINAL POSITION: Moves motor using speed control, and
				* user places the motor at the final point of movement 
				*/
				x = user_set_position(TRUE);	// TRUE means initial pos
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.final_pos = x;
				}

				/*
				* ACCELERATION: percentage of acceleration range
				*/
				x = user_set_accel();
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.accel = (int8_t)x;
				}

				/*
				* TIME MOVING: Motor stands still. User enters the time
				* required to perform the movement. It directly determines the
				* max speed at which the slider will move. Returns speed.
				*/
				x = user_set_time(automatic.initial_pos, automatic.final_pos);
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.speed = x;
				}

				/*
				* REPETITIONS: up to 20 repetitions of the same movement.
				* if reps is 0. No repetitions will be performed
				*/
				x = user_set_reps();
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.reps = (uint8_t)x;
				}

				/*
				* LOOP: if active, movement will go from init position to
				* final position, and back again to initial position
				*/
				x = user_set_loop();
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.loop = (uint8_t)x;
				}

				/*
				* Go to initial position:
				*/
				x = user_go_to_init(automatic.initial_pos);
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				}

				/*
				* START automatic movement
				*/
				x = user_gogogo(automatic);
				if (x < 0) {
					system_state = STATE_FAIL;
					break;
				} else {
					system_state = STATE_CHOOSE_ACTION;
				}

				system_state = STATE_CHOOSE_ACTION;
				break;

			case STATE_FAIL:
				fail_message();
				system_state = STATE_CHOOSE_ACTION;
				break;
			
			default:
				break;
		}
	}
}

/******************************************************************************
*******************************************************************************

                    I N T E R R U P T   H A N D L E R S

*******************************************************************************
******************************************************************************/

/*-----------------------------------------------------------------------------
                   		 T I M E R   C O U N T E R S
-----------------------------------------------------------------------------*/

ISR(TIMER2_COMPA_vect){
/*
* General Timer. T=1ms
*/
	ms++;
}