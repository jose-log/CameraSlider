

#include "timers.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t ms = 0;

void timer_speed_init(void)
{
	// TIMER COUNTER 1: 16-bit counter
	TCCR1B |= (1<<WGM12);		// CTC mode, TOP: OCR1A
	TIMSK1 |= (1<<OCIE1A);		// Set interrupts
	TIFR1 |= (1<<OCF1A);		// Clear any previous interrupt
	OCR1A = 0;					// Clear timer compare
	TCNT1 = 0;					// Clear counter
}

void timer_general_init(void)
{
	// TIMER COUNTER 2: 8-bit counter
	TCCR2A |= (1 << WGM21);		// Sets CTC mode
	TIMSK2 |= (1 << OCIE2A); 	// Set interrupts
	TIFR2 |= (1<<OCF2A);		// Clear any previous interrupt
	OCR2A = 124;				// Interrupt period T = 1ms
	TCNT2 = 0;					// Clear counter
}

void timer_aux_init(void)
{
	// TIMER COUNTER 0: 8-bit counter
	TCCR0A |= (1<<WGM01);		// CTC mode, TOP: OCR0A
	TIMSK0 |= (1<<OCIE0A);		// Set interrupts
	TIFR0 |= (1<<OCF0A);		// Clear any previous interrupt
	OCR0A = 0;					// Clear timer compare
	TCNT0 = 0;					// Clear counter
}

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

void timer_speed_set_raw(uint16_t c){

	OCR1A = c;
}

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

uint8_t timer_speed_check(void)
{
/*
* If the speed-timer prescaler is NOT enabled, it will return FALSE,
* otherwise, it will return non-zero.
*/
	uint8_t r = TCCR1B & ((1<<CS12) | (1<<CS11) | (1<<CS10));
	
	return r;
}

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

ISR(TIMER2_COMPA_vect){
/*
* General Timer. T=1ms
*/
	ms++;
}