
#ifndef CONFIG_H_
#define CONFIG_H_

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include <avr/io.h>
#include <stdint.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

// SYSTEMS' FREQUENCIES
#define F_CPU	16000000UL		// 16MHz ceramic resonator, no prescaler
#define BAUD 	115200UL		// A standard baud rate for serial interface
#define F_MOTOR (16000000 / 8)	// Speed Timer Frequency. Prescaler 8

// Miscelanous global definitions
#define TRUE	1
#define FALSE	0
#define ENABLE 	TRUE
#define DISABLE FALSE
#define CW		TRUE
#define CCW		FALSE

// Debug
#define DEBUG(x) 	uart_send_string(x);
#define DEBUG_P(x) 	uart_send_string(PSTR(x));

#endif /* CONFIG_H_ */