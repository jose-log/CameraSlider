

#include "config.h"
#include "init.h"
#include "timers.h"
#include "uart.h"

#include <avr/io.h>

static void ports_init(void);

void boot(void){

	ports_init();
	uart_init();
	uart_set(ENABLE);
	speed_timer_init();
	aux_timer_init();
	
}

static void ports_init(void){
	
	// Encoder pins (INT0/1)
	DDRD &= ~(1<<DDD2);		// INT0
	DDRD &= ~(1<<DDD3);		// INT1
	// Encoder button
	DDRC &= ~(1<<DDC3);
	PORTC |= (1<<PORTC3);	// pull-up enabled
	
	// Display pins
	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
	DDRB |= (1<<DDB4);
	DDRB |= (1<<DDB5);
	DDRC |= (1<<DDC0);
	DDRC |= (1<<DDC1);
	DDRC |= (1<<DDC2);
	PORTB &= ~(1<<PORTB3);	// RW to GND

	// Driver pins
	DDRD |= (1<<DDD7);	// ~ENABLE - D1
	DDRC |= (1<<DDC5);	// MS1 - D0
	DDRD |= (1<<DDD4); 	// MS2 - D4
	DDRD |= (1<<DDD5);	// MS3 - D5
	DDRD |= (1<<DDD6);	// ~RST - D6
	DDRB |= (1<<DDB0);	// STEP - D8
	DDRB |= (1<<DDB1);	// DIR - D9
	// ~SLEEP - Discarded for use. It's not useful
	// connected to Vdd by default

	DRV_DIR_PORT |= (1<<DRV_DIR_PIN);
	DRV_STEP_PORT &= ~(1<<DRV_STEP_PIN);
	DRV_RST_PORT |= (1<<DRV_RST_PIN);
	DRV_MS3_PORT &= ~(1<<DRV_MS3_PIN);
	DRV_MS2_PORT &= ~(1<<DRV_MS2_PIN);
	DRV_MS1_PORT &= ~(1<<DRV_MS1_PIN);
	DRV_EN_PORT |= (1<<DRV_EN_PIN);		// Initially disabled

	// Limit switch pin
	DDRC &= ~(1<<DDC4);
	PORTC |= (1<<PORTC4); 	// Pull-up enabled

	// debug Serial port
	DDRD |= (1<<DDD1);		// TXD
	DDRD |= (1<<DDD0);		// RXD
	PORTD |= (1<<PORTD1);	// initial value HIGH
	PORTD |= (1<<PORTD0);	// initial value HIGH
}
