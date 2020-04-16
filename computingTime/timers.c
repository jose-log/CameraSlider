
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
		TCCR1B |= (1<<CS11) | (1<<CS10);	// Prescaler: 1/64. Start timer
		//TCCR1B |= (1<<CS11);				// Prescaler: 1/8. Start timer
	} else {
		TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
		OCR1A = 0;
	}
}

void speed_timer_set_raw(uint16_t c){

	OCR1A = c;
}

void general_timer_init(void){

	// TIMER COUNTER 2: 8-bit counter
	TCCR2A &= ~((1 << WGM21) | (1<<WGM20));		// Sets Normal mode
	TIMSK2 |= (1 << TOIE2); 	// Set interrupts when OVERFLOWED
	TIFR2 |= (1<<TOV2);		// Clear any previous interrupt
	TCNT2 = 0;					// Clear counter
	//TCCR2B |= (1<<CS20);	// Prescaler: 128. Start timer
}

void general_timer_set(uint8_t state){

	TCNT2 = 0;
	if(state){
		TCCR2B |= (1<<CS22); // Prescaler: 64. Start timer
	} else {
		TCCR2B &= ~((1<<CS22) | (1<<CS21) | (1<<CS20));
		OCR2A = 0;
	}
}