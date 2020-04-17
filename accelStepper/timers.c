
#include "timers.h"

#include <avr/io.h>

void speed_timer_init(void){

	// TIMER COUNTER 1: 16-bit counter
	TCCR1B |= (1<<WGM12);		// CTC mode, TOP: OCR1A
	TIMSK1 |= (1<<OCIE1A);		// Set interrupts
	TIFR1 |= (1<<OCF1A);		// Clear any previous interrupt
	OCR1A = 0;					// Clear timer compare
	TCNT1 = 0;					// Clear counter
	// Prescaler: 	
	//TCCR1B |= (1<<CS11) | (1<<CS10);	// 1/64. Start timer
	//TCCR1B |= (1<<CS11);				// 1/8. Start timer
}

void speed_timer_set(uint8_t state, uint16_t t){

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

void speed_timer_set_raw(uint16_t c){

	OCR1A = c;
}

void aux_timer_init(void){

	// TIMER COUNTER 0: 8-bit counter
	TCCR0A |= (1<<WGM01);		// CTC mode, TOP: OCR0A
	TIMSK0 |= (1<<OCIE0A);		// Set interrupts
	TIFR0 |= (1<<OCF0A);		// Clear any previous interrupt
	OCR0A = 0;					// Clear timer compare
	TCNT0 = 0;					// Clear counter
}

void aux_timer_set(uint8_t state, uint8_t t){

	TCNT0 = 0;
	if(state){
		OCR0A = t;
		TCCR0B |= (1<<CS00);	// Prescaler: 1. Start timer
	} else {
		TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
		OCR0A = 0;
	}
}