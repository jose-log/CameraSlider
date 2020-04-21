
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "encoder.h"

#include <avr/io.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

#define BUTTON			(PINC & (1<<PINC3))
#define SWITCH			(PINC & (1<<PINC4))

// Button time counts: These macros determine the time it takes for 
// different flags within btnXYZ structure to be set (in milliseconds)
#define BTN_LOCK_TIME	30		// lock time after button released
#define BTN_DLY1_TIME	300		// time for delay 1
#define BTN_DLY2_TIME	65		// time for delay 2
#define BTN_DLY3_TIME	2000	// time for delay 3
#define BTN_BEEP_TIME	50		// duration of beep sound

#define SWITCH_TIMEOUT 	100		// milliseconds

/******************************************************************************
****************** V A R I A B L E S   D E F I N I T I O N S ******************
******************************************************************************/

struct enc_s encoder;
struct btn_s btn;

static volatile uint8_t limit_switch;

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void init_defaults(void);

void encoder_init(void){
/*	
*	Enc_A		- PD2 - INT0
*	Enc_B		- PD3 - INT1
*	Enc_btn		- PC3 - PCINT11 | PCI1
*	Only Enc_A signal is necessary to trigger the ISR.
*/
	// Falling edge of INT0 triggers ISR
	EICRA |= (1<<ISC01);

	EIMSK |= (1<<INT0);
	// Clear any previous interrupt
	EIFR |= (1<<INTF0);

	// Enables pin toggle interrupts for Encoder button
	PCICR |= (1<<PCIE1);
	PCMSK1 |= (1<<PCINT11);

	init_defaults();
}

void limit_switch_init(void)
{
/*
*	SW	- PC4 - PCINT12 | -> PCI1
*/
	PCICR |= (1<<PCIE1);	// Enables pin toggle interrupt
	PCMSK1 |= (1<<PCINT12);
}

uint8_t encoder_get_update(void)
{
	return enc.update;
}

void encoder_set_update(uint8_t state)
{
	enc.update = state;
}

uint8_t encoder_get_dir(void)
{
	return enc.dir;
}

/*----------------------------------------------------------------------------*
*----------------------------------------------------------------------------*/

struct btn_s *button_get(void)
{
	return &btn;
}

struct enc_s *encoder_get(void)
{
	return &encoder;
}

void button_check(void)
{ 
	switch(btn.state){

		case BTN_IDLE:
			if(!BUTTON) btn.count++;
			else if(btn.count > 0) btn.count--;

			if(btn.count == 7){
				btn.action = TRUE;
				btn.lock = TRUE;
				btn.state = BTN_PUSHED;
				btn.count = 0;
				//buzzer_set(ENABLE, N_C8);
			} else if(btn.count == 0){
				btn.action = FALSE;
				btn.lock = FALSE;
				btn.query = FALSE;
			}
			break;

		case BTN_PUSHED:
			if(!BUTTON){
				btn.count++;	
			} else {
				btn.count = 0;
				btn.state = BTN_RELEASED;
			}
			if(btn.count == BTN_BEEP_TIME); //buzzer_set(DISABLE, N_C8);
			if(btn.count == BTN_DLY1_TIME) btn.delay1 = TRUE;
			if((btn.delay1) && (!(btn.count % BTN_DLY2_TIME))) btn.delay2 = TRUE;
			if(btn.count >= BTN_DLY3_TIME) btn.delay3 = TRUE;
			break;

		case BTN_RELEASED:
			if(BUTTON)
				btn.count++;
			if(btn.count == BTN_LOCK_TIME){
				btn.query = FALSE;
				btn.action = FALSE;
				btn.lock = FALSE;
				btn.state = BTN_IDLE;
				btn.count = 0;
				btn.delay1 = FALSE;
				btn.delay2 = FALSE;
				btn.delay3 = FALSE;
				//buzzer_set(DISABLE, N_C8);
			}
			break;
		default:
			break;
	}
}

static void init_defaults(void)
{
	enc.update = FALSE;
	enc.dir = CW;

	btn.query = FALSE;
	btn.action = FALSE;
	btn.lock = FALSE;
	btn.state = BTN_IDLE;
	btn.count = 0;
	btn.delay1 = FALSE;
	btn.delay2 = FALSE;
	btn.delay3 = FALSE;
}

/******************************************************************************
*******************************************************************************

                    I N T E R R U P T   H A N D L E R S

*******************************************************************************
******************************************************************************/

ISR(INT0_vect){
	
	enc.update = TRUE;

	if(PIND & (1<<PIND3)) enc.dir = CW;
	else enc.dir = CCW;
}

ISR(PCINT1_vect){
/*
	BTN - PC3 - PCINT11 | -> PCI1 (Encoder push button)
	SW	- PC4 - PCINT12 | -> PCI1 (Slider Limit Switch)
*/
    if((!SWITCH) && (!slider.sw)) {
    	limit_switch = TRUE;
    	// Here, there must be a more elaborate piece of code to handle
    	// the slider crushing against the switch. Maybe implement a shutdown
    	// and a reset condition.
    }
    if((!BUTTON) && (!btn.lock))
        btn.query = TRUE;
}
