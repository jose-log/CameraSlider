
/*
* UART module driver.
* It handles all uart-related functions
*/
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "uart.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/pgmspace.h>	/* Program Memory Strings handling */
#include <util/delay.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define BAUD_REGISTER 	((uint16_t)(F_CPU/(8*BAUD)-1))

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

// prototype functions with local scope
static uint8_t uart_flush(void);

/*===========================================================================*/
void uart_init(void){

		/*Set baud rate */
	UBRR0H = (uint8_t) (BAUD_REGISTER>>8);
	UBRR0L = (uint8_t) (BAUD_REGISTER & 0x00FF);

	/* Double transmission speed */
	UCSR0A |= (1<<U2X0);	// It provides more precise Baud rate

	/* Asynchronous operation, no parity, 1 stop bit */
	UCSR0C &= ~((1<<UMSEL01) | (1<<UMSEL00) | (1<<UPM01) | (1<<UPM00) | (1<<USBS0));

	/* Set frame format: 8data */
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
}

/*===========================================================================*/
void uart_send_char( char data ){
	
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );

	/* Put data into buffer, sends the data */
	UDR0 = data;

	/* Wait empty buffer again  */
	while ( !( UCSR0A & (1<<UDRE0)) );
}

/*===========================================================================*/
void uart_send_string(const char *s){

	while (*s != '\0')
	{
		uart_send_char(*s);
		s++;
	}
}

/*===========================================================================*/
void uart_send_string_p(const char *s){

	while (pgm_read_byte(s) != '\0')
	{
		uart_send_char(pgm_read_byte(s));
		s++;
	}
}

/*===========================================================================*/
char uart_read_char( void ){

	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));

	/* Get and return received data from buffer */
	return UDR0;
}

/*===========================================================================*/
void uart_set(uint8_t state){

	if(state){
		/* Enable receiver and transmitter */
		UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
		uart_flush();
	} else {
		/* Disable receiver and transmitter */
		UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));
	}
}

/*===========================================================================*/
static uint8_t uart_flush(void){

	char trash;

	while ((UCSR0A & (1<<RXC0))){
		trash = UDR0;
	}

	return trash;
}