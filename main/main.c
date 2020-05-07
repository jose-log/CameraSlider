/*
* MAIN FILE
*
* The whole program is organized as a pseudo-FSM where the states are 
* organized such that the user may be able to switch between them easily
* through the menu structure.
* Every menu is built into a function that handles all possible user interac-
* tions with the slider configuration.
*/ 

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "init.h"
#include "menu.h"
#include "motor.h"
#include "timers.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdlib.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

typedef enum {
	STATE_HOMING,
	STATE_CHOOSE_ACTION,
	STATE_MANUAL_MOVEMENT,
	STATE_CREATE_MOVEMENT,
	STATE_START_MOVEMENT,
	STATE_FAIL
} state_t;

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

// System state for the Finite States Machine
volatile static state_t system_state = STATE_HOMING;
// Structure that stores all user-programmed movement parameters to be
// automatically executed by the slider.
struct auto_s automatic;

/******************************************************************************
*************************** M A I N   P R O G R A M ***************************
******************************************************************************/

int main(void)
{
	// Load all initial configuration
	boot();

	// Enable global Interrupts
	sei();

	// misc variable for retrieved arguments in menu functions
	int32_t x = 0;

	while(TRUE){

		/*
		* SYSTEM MENU STRUCTURE:
		*
		* - Homing: initial callibration
		* - Create Movement:
		* 	- Initial position
		*	- Final position
		*	- Acceleration (ramp up/down)
		*	- Movement time
		*	- Set N° repetitions
		*	- Set LOOP flag
		* 	- Go to initial position
		*	- Execute pre-programmed movement
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
			* CREATE MOVEMENT: The user chooses from among a set of parameters
			* that are stored in the "automatic" structure, and then passed on
			* to the final function which executes the movement.
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
				x = user_set_position(TRUE);	// TRUE means final pos
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.final_pos = x;
				}

				/*
				* ACCELERATION: percentage of acceleration range.
				* Maximum and minimum acceleration are constrained by physical
				* and mathematical restrictions of the motor and the algorithm
				* respectively.
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
				* If reps = 1: slider moves from init to final position.
				* If reps > 1: slider moves back and forth from init to final
				* 	position, once per repetition selected.
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
				* final position, and back again to initial position, and repeat
				* this cycle forever, ignoring the REPEAT parameter
				*/
				x = user_set_loop();
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				} else {
					automatic.loop = (uint8_t)x;
				}

				system_state = STATE_START_MOVEMENT;
				break;

			case STATE_START_MOVEMENT:
				/*
				* GO TO INITIAL POSITION. Slider will move towards the initial
				* position point chosen by the user, and get ready to execute
				* the user-programmed movement
				*/
				x = user_go_to_init(automatic.initial_pos);
				if (x < 0) {
					system_state = STATE_CHOOSE_ACTION;
					break;
				}

				/*
				* START automatic movement. All movement parameters are passed
				* within the "automatic" structure.
				*/
				x = user_gogogo(automatic);
				if (x < 0)
					system_state = STATE_CHOOSE_ACTION;
				else if (x == TRUE)
					system_state = STATE_START_MOVEMENT;
				else
					system_state = STATE_FAIL;

				break;

			/*
			* FAIL SCREEN. If some error code is retrieved from some menu
			* function, then the execution flow should fall into the FAIL
			* screen message
			*/
			case STATE_FAIL:
				fail_message();
				system_state = STATE_CHOOSE_ACTION;
				break;
			
			default:
				break;
		}
	}
}

