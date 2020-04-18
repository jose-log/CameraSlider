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
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

volatile uint8_t tmr = 0;

// Declaration of global structures
slider_s slider;
encoder_s encoder;
btn_s btn;

volatile static state_t system_state = STATE_HOMING;
volatile uint16_t ms = 0;
volatile uint8_t speed_update = FALSE;

/******************************************************************************
*************************** M A I N   P R O G R A M ***************************
******************************************************************************/

int main(void)
{
	boot();

	// Enable global Interrupts
	sei();

	// misc vars for retrieved arguments in menu functions
	uint8_t x = FALSE;
	uint8_t y = FALSE;

	while(TRUE){

		switch(system_state){

			case STATE_HOMING:
				x = homing();
				if(!x) system_state = STATE_CHOOSE_MOVEMENT;
				else system_state = STATE_FAIL;
				break;

			case STATE_CHOOSE_MOVEMENT:		// Automatic or Manual movement
				system_state = choose_movement();
				break;

			case STATE_CHOOSE_MANUAL_CONTROL:
				y = choose_manual_control();
				system_state = STATE_CHOOSE_MANUAL_MOVEMENT;
				break;

			case STATE_CHOOSE_MANUAL_MOVEMENT:
				x = choose_manual_movement();
				if(x == QUIT_MENU){
					system_state = STATE_CHOOSE_MOVEMENT;	
				} else {
					if(y == CONTROL_SPEED) system_state = STATE_MANUAL_SPEED;
					else if(y == CONTROL_POS) system_state = STATE_MANUAL_POSITION;
				}
				break;

			case STATE_MANUAL_SPEED:
				manual_speed(x);
				system_state = STATE_CHOOSE_MOVEMENT;
				break;

			case STATE_MANUAL_POSITION:
				manual_position(x);
				system_state = STATE_CHOOSE_MOVEMENT;
				break;

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
				system_state = STATE_CHOOSE_MOVEMENT;
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
                    E X T E R N A L   I N T E R R U P T S
-----------------------------------------------------------------------------*/

ISR(INT0_vect){
	
	encoder.update = TRUE;

	if(PIND & (1<<PIND3))
		encoder.dir = CW;
	else
		encoder.dir = CCW;
}

ISR(PCINT1_vect){
/*
	BTN - PC3 - PCINT11 | -> PCI1 (Encoder push button)
	SW	- PC4 - PCINT12 | -> PCI1 (Slider Limit Switch)
*/
    if((!SWITCH) && (!slider.sw))
    	slider.sw = TRUE;
    if((!BUTTON) && (!btn.lock))
        btn.query = TRUE;
}

/*-----------------------------------------------------------------------------
                   		 T I M E R   C O U N T E R S
-----------------------------------------------------------------------------*/

static void pulse(void){

	DRV_STEP_PORT |= (1<<DRV_STEP_PIN);
	_delay_us(2);
	DRV_STEP_PORT &= ~(1<<DRV_STEP_PIN);
}

ISR(TIMER1_COMPA_vect){
/*
* Speed Timer.
*/
	if(system_state == STATE_HOMING){
		pulse();
	} else {
		// speed limit correction based on position
		slider.speed = speed_limit_correction();

		if((speed_update) || (slider.marginal_zone)){
			OCR1A = slider.speed;
			speed_update = FALSE;
		}
		if(slider.spin == CW){
			if(slider.position < MAX_COUNT){
				pulse();
				slider.position++;
				slider.out_of_bounds = FALSE;
			} else {
				slider.out_of_bounds = TRUE;
			}
		} else if(slider.spin == CCW){
			if(slider.position > 0){
				pulse();
				slider.position--;	
				slider.out_of_bounds = FALSE;
			} else {
				slider.out_of_bounds = TRUE;
			}
		}
	}

}

ISR(TIMER2_COMPA_vect){
/*
* General Timer. T=1ms
*/
	ms++;
}