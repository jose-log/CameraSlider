/*
 * config.h
 *
 * Created: 15-Oct-18 9:28:37 PM
 *  Author: josel
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include <avr/io.h>
#include <stdint.h>

/******************************************************************************
***************** G L O B A L   S C O P E   V A R I A B L E S *****************
******************************************************************************/

extern volatile uint16_t ms;

// SYSTEMS' FREQUENCIES
#define F_CPU	16000000UL		// 16MHz ceramic resonator, no prescaler
#define BAUD 	115200UL		// A standard baud rate for serial interface
#define F_MOTOR (16000000 / 8)	// Speed Timer Frequency. Prescaler 8

#define TRUE	1
#define FALSE	0
#define ENABLE 	TRUE
#define DISABLE FALSE
#define CW		TRUE
#define CCW		FALSE

// Debug
#define DEBUG(x) 	uart_send_string(x);

#endif /* CONFIG_H_ */