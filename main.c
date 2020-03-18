/*
 * dev.c
 *
 * Created: 15-Oct-18 8:06:03 PM
 * Author : josel
 */ 

#include "config.h"
#include "driver.h"
#include "init.h"
#include "lcd.h"
#include "menu.h"
#include "move.h"
#include "timers.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

volatile uint8_t tmr = 0;

// Declaration of global structures
slider_s slider;
encoder_s encoder;
btn_s btn;
volatile static state_t system_state = STATE_HOMING;
volatile uint16_t ms = 0;
volatile uint8_t speed_update = FALSE;

int main(void)
{
	boot();
	
	lcd_screen(SCREEN_WELCOME);
	
	drv_step_mode(MODE_SIXTEENTH_STEP);

	general_timer_set(ENABLE);
	sei();

	uint8_t x = FALSE;

	while(TRUE){

		switch(system_state){

			case STATE_HOMING:
				x = homing();
				if(!x) system_state = STATE_CHOOSE_MOVEMENT;
				else system_state = STATE_FAIL;
				break;

			case STATE_CHOOSE_MOVEMENT:
				system_state = choose_movement();
				break;

			case STATE_CHOOSE_MANUAL_MOVEMENT:
				x = choose_manual_movement();
				if(x == 0) system_state = STATE_CHOOSE_MOVEMENT;
				else system_state = STATE_MANUAL_MOVEMENT;
				break;

			case STATE_MANUAL_MOVEMENT:
				manual_movement(x);
				system_state = STATE_CHOOSE_MOVEMENT;
				break;

			case STATE_CREATE_MOVEMENT:
				user_movement();
				break;

			case STATE_CHOOSE_CONTROL:
				

			case STATE_FAIL:
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