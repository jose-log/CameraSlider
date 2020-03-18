

#include "timers.h"

#include <avr/io.h>

void speed_timer_init(void){

	// TIMER COUNTER 1: 16-bit counter
	TCCR1B |= (1<<WGM12);		// CTC mode, TOP: OCR1A
	TIMSK1 |= (1<<OCIE1A);		// Set interrupts
	TIFR1 |= (1<<OCF1A);		// Clear any previous interrupt
	OCR1A = 0;					// Clear timer compare
	TCNT1 = 0;					// Clear counter
	// Prescaler: 1/64. Start timer
	//TCCR1B |= (1<<CS11) | (1<<CS10);
}

void general_timer_init(void){

	// TIMER COUNTER 2: 8-bit counter
	TCCR2A |= (1 << WGM21);		// Sets CTC mode
	TIMSK2 |= (1 << OCIE2A); 	// Set interrupts
	TIFR2 |= (1<<OCF2A);		// Clear any previous interrupt
	OCR2A = 124;				// Interrupt period T = 1ms
	TCNT2 = 0;					// Clear counter
	//TCCR2B |= (1<<CS22) | (1<<CS20);	// Prescaler: 128. Start timer
}


void speed_timer_set(uint8_t state, uint16_t t){

	TCNT1 = 0;
	if(state){
		OCR1A = t;
		TCCR1B |= (1<<CS11) | (1<<CS10);	// Prescaler: 1/64. Start timer
	} else {
		TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
		OCR1A = 0;
	}
}

void general_timer_set(uint8_t state){

	TCNT2 = 0;
	if(state){
		OCR2A = 124;
		TCCR2B |= (1<<CS22) | (1<<CS20); // Prescaler: 128. Start timer
	} else {
		TCCR2B &= ~((1<<CS22) | (1<<CS21) | (1<<CS20));
		OCR2A = 0;
	}
}

uint8_t check_speed_timer(void){
/*
* If the speed-timer prescaler is NOT enabled, it will return FALSE,
* otherwise, it will return non-zero.
*/

	uint8_t r = TCCR1B & ((1<<CS12) | (1<<CS11) | (1<<CS10));
	
	return r;
}