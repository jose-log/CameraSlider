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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

typedef enum {
	STATE_HOMING,
	STATE_CHOOSE_ACTION,
	STATE_CHOOSE_CONTROL_TYPE,
	STATE_CHOOSE_SPEED_PROFILE,
	STATE_MANUAL_SPEED,
	STATE_MANUAL_POSITION,
	STATE_CREATE_MOVEMENT,
	STATE_TEST_ACCELSTEPPER,
	STATE_FAIL
} state_t;

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

volatile static state_t system_state = STATE_HOMING;
volatile uint16_t ms = 0;

/******************************************************************************
*************************** M A I N   P R O G R A M ***************************
******************************************************************************/

int main(void)
{
	boot();

	// Enable global Interrupts
	sei();

	// misc vars for retrieved arguments in menu functions
	int8_t x;

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
				if (choose_action()) system_state = STATE_CHOOSE_CONTROL_TYPE;
				else system_state = STATE_CREATE_MOVEMENT;
				break;

			/*
			* CONTROL TYPE: Two options are displayed:
			* 	- Position control: encoder varies the slider position 
			* 	- Speed control: encoder varies the slider speed
			*/
			case STATE_CHOOSE_CONTROL_TYPE:
				x = choose_control_type();
				if (x < 0) system_state = STATE_FAIL;
				else system_state = STATE_CHOOSE_SPEED_PROFILE;
				break;

			/*
			* CHOOSE SPEED PROFILE: Two options are displayed:
			* 	- Linear: Speed increases/decreases linearly
			* 	- Quadratic: Speed increases/decreases as in a squared function
			* 		This profile is less sensible at low speeds and more
			*		sensible at high speeds
			*/
			case STATE_CHOOSE_SPEED_PROFILE:
				if(choose_speed_profile() < 0){
					system_state = STATE_CHOOSE_ACTION;	
				} else {
					if(x == 1) system_state = STATE_MANUAL_POSITION;
					else if(x == 0) system_state = STATE_MANUAL_SPEED;
				}
				break;

			/*
			* SPEED CONTROL: MOTOR CAN BE MOVED and the rotary encoder changes
			*	its velocity
			*/
			case STATE_MANUAL_SPEED:
				manual_speed();
				system_state = STATE_CHOOSE_ACTION;
				break;

			/*
			* POSITION CONTROL: MOTOR CAN BE MOVED and the rotary encoder 
			*	changes its position
			*/
			case STATE_MANUAL_POSITION:
				manual_position();
				system_state = STATE_CHOOSE_ACTION;
				break;

			/*
			* CREATE MOVEMENT: under construction
			*/
			case STATE_CREATE_MOVEMENT:
				user_movement();
				system_state = STATE_FAIL;
				break;

			case STATE_TEST_ACCELSTEPPER:
				//accel_stepper();
				break;

			case STATE_FAIL:
				fail_message();
				_delay_ms(1000);
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