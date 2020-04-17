

#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>
#include <stdint.h>

// 16MHz ceramic resonator, no prescaler
#define F_CPU	16000000UL
// A standard baud rate for serial interface
#define BAUD 	115200UL

#define TRUE	1
#define FALSE	0
#define ENABLE 	TRUE
#define DISABLE FALSE

#define CW		TRUE
#define CCW		FALSE

// DRIVER ports
#define DRV_EN_PORT 	PORTD
#define DRV_MS1_PORT 	PORTC
#define DRV_MS2_PORT	PORTD
#define DRV_MS3_PORT	PORTD
#define DRV_RST_PORT	PORTD
#define DRV_STEP_PORT	PORTB
#define DRV_DIR_PORT	PORTB

#define DRV_EN_PIN 		PORTD7
#define DRV_MS1_PIN 	PORTC5
#define DRV_MS2_PIN		PORTD4
#define DRV_MS3_PIN		PORTD5
#define DRV_RST_PIN		PORTD6
#define DRV_STEP_PIN	PORTB0
#define DRV_DIR_PIN		PORTB1

// Movement mode:
#define ABS 		0xAB
#define REL			0xEA

#endif
