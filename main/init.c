
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "init.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void ports_init(void);

void boot(void)
{
	ports_init();
	timer_speed_init();
	timer_general_init();
	timer_aux_init();
	encoder_init();
	limit_switch_init();
	lcd_init();
	uart_init();
	motor_init();
	
	uart_set(ENABLE);
	timer_general_set(ENABLE);

	// Messasges:
	lcd_screen(SCREEN_WELCOME);
	uart_send_string_p(PSTR("#--------------------------\n\r"));
	uart_send_string_p(PSTR("Hello World!\n\r"));
}

static void ports_init(void)
{	
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
	DRV_EN_PORT |= (1<<DRV_EN_PIN);		// Disabled Initially

	// Limit switch pin
	DDRC &= ~(1<<DDC4);
	PORTC |= (1<<PORTC4); 	// Pull-up enabled

	// debug Serial port
	DDRD |= (1<<DDD1);		// TXD
	DDRD |= (1<<DDD0);		// RXD
	PORTD |= (1<<PORTD1);	// initial value HIGH
	PORTD |= (1<<PORTD0);	// initial value HIGH
}