
/*
* This file includes all functions related to the timers used in this project
* and the configuration for their ISRs
*/
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "timers.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t ms = 0;

/*===========================================================================*/
/*
* Motor timer initialization.
* It does NOT start the timer.
*/
void timer_speed_init(void)
{
	// TIMER COUNTER 1: 16-bit counter
	TCCR1B |= (1<<WGM12);		// CTC mode, TOP: OCR1A
	TIMSK1 |= (1<<OCIE1A);		// Set interrupts
	TIFR1 |= (1<<OCF1A);		// Clear any previous interrupt
	OCR1A = 0;					// Clear timer compare
	TCNT1 = 0;					// Clear counter
}

/*===========================================================================*/
/*
* General timer initialization. Typically a 1ms period timer.
* It does NOT start the timer.
*/
void timer_general_init(void)
{
	// TIMER COUNTER 2: 8-bit counter
	TCCR2A |= (1 << WGM21);		// Sets CTC mode
	TIMSK2 |= (1 << OCIE2A); 	// Set interrupts
	TIFR2 |= (1<<OCF2A);		// Clear any previous interrupt
	OCR2A = 124;				// Interrupt period T = 1ms
	TCNT2 = 0;					// Clear counter
}

/*===========================================================================*/
/*
* Auxiliary timer initialization. It's a one-shot timer. it only runs to 
* trigger the ISR function that handles queued movements.
* It does NOT start the timer.
*/
void timer_aux_init(void)
{
	// TIMER COUNTER 0: 8-bit counter
	TCCR0A |= (1<<WGM01);		// CTC mode, TOP: OCR0A
	TIMSK0 |= (1<<OCIE0A);		// Set interrupts
	TIFR0 |= (1<<OCF0A);		// Clear any previous interrupt
	OCR0A = 0;					// Clear timer compare
	TCNT0 = 0;					// Clear counter
}

/*===========================================================================*/
/*
* Motor timer start/stop.
*/
void timer_speed_set(uint8_t state, uint16_t t)
{
	TCNT1 = 0;
	if(state){
		OCR1A = t;
		//TCCR1B |= (1<<CS11) | (1<<CS10);	// Prescaler: 1/64. Start timer
		TCCR1B |= (1<<CS11);				// Prescaler: 1/8. Start timer
	} else {
		TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
		OCR1A = 0;
	}
}

/*===========================================================================*/
/*
* Motor timer. Modifies Cn without stopping or resetting the timer.
*/
void timer_speed_set_raw(uint16_t c){

	OCR1A = c;
}

/*===========================================================================*/
/*
* General timer start/stop
*/
void timer_general_set(uint8_t state)
{
	TCNT2 = 0;
	if(state){
		OCR2A = 124;
		TCCR2B |= (1<<CS22) | (1<<CS20); // Prescaler: 128. Start timer
	} else {
		TCCR2B &= ~((1<<CS22) | (1<<CS21) | (1<<CS20));
		OCR2A = 0;
	}
}

/*===========================================================================*/
/*
* Auxiliary timer start/stop
*/
void timer_aux_set(uint8_t state, uint8_t t)
{
	TCNT0 = 0;
	if(state){
		OCR0A = t;
		TCCR0B |= (1<<CS00);	// Prescaler: 1. Start timer
	} else {
		TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
		OCR0A = 0;
	}
}

/*===========================================================================*/
/*
* Motor timer check. Some functions require to check whether the motor timer
* is runnig in order to properly execute.
* If the speed-timer prescaler is NOT enabled, it will return FALSE,
* otherwise, it will return non-zero.
*/
uint8_t timer_speed_check(void)
{
	uint8_t r = TCCR1B & ((1<<CS12) | (1<<CS11) | (1<<CS10));
	
	return r;
}

/*===========================================================================*/
/*
* Retrieve raw value of timer compare register
*/
uint16_t timer_speed_get(void)
{
	return OCR1A;
}

/******************************************************************************
*******************************************************************************

                    I N T E R R U P T   H A N D L E R S

*******************************************************************************
******************************************************************************/

/*-----------------------------------------------------------------------------
                   		 T I M E R   C O U N T E R S
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* General Timer. T=1ms
*/
ISR(TIMER2_COMPA_vect)
{
	ms++;
}