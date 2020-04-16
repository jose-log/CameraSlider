
#include "external_int.h"

#include <avr/io.h>


void encoder_init(void){
/*	
	Enc_A		- PD2 - INT0
	Enc_B		- PD3 - INT1
	Enc_btn		- PC3 - PCINT11 | PCI1
	Only Enc_A signal is necessary to trigger the ISR.
*/
	// Falling edge of INT0 triggers ISR
	EICRA |= (1<<ISC01);

	EIMSK |= (1<<INT0);
	// Clear any previous interrupt
	EIFR |= (1<<INTF0);

	// Enables pin toggle interrupts for Encoder button
	PCICR |= (1<<PCIE1);
	PCMSK1 |= (1<<PCINT11);
}

void limit_switch_init(void){
/*
	SW	- PC4 - PCINT12 | -> PCI1
*/
	PCICR |= (1<<PCIE1);	// Enables pin toggle interrupt
	PCMSK1 |= (1<<PCINT12);
}