/*
 * config.h
 *
 * Created: 15-Oct-18 9:28:37 PM
 *  Author: josel
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>
#include <stdint.h>

#define F_CPU	16000000UL

#define TRUE	1
#define FALSE	0
#define ENABLE 	TRUE
#define DISABLE FALSE

#define CW		TRUE
#define CCW		FALSE

#define MAX_LENGHT_CMS	((uint32_t) 80)
#define CMS_PER_REV 	((uint32_t) 4)
#define STEPS_PER_REV	((uint32_t) 3200)	// SIXTEENTH stepping
#define MAX_COUNT		((uint32_t) (MAX_LENGHT_CMS * (STEPS_PER_REV / CMS_PER_REV)))
#define SAFETY_DISTANCE_CMS 	3
#define SAFETY_DISTANCE ((uint32_t)	(SAFETY_DISTANCE_CMS * (STEPS_PER_REV / CMS_PER_REV)))

#define SWITCH_TIMEOUT 	100		// milliseconds

// DRIVER ports
#define DRV_EN_PORT 	PORTD
#define DRV_MS1_PORT 	PORTC
#define DRV_MS2_PORT	PORTD
#define DRV_MS3_PORT	PORTD
#define DRV_RST_PORT	PORTD
#define DRV_SLEEP_PORT	PORTD
#define DRV_STEP_PORT	PORTB
#define DRV_DIR_PORT	PORTB

#define DRV_EN_PIN 		PORTD7
#define DRV_MS1_PIN 	PORTC5
#define DRV_MS2_PIN		PORTD4
#define DRV_MS3_PIN		PORTD5
#define DRV_RST_PIN		PORTD6
#define DRV_SLEEP_PIN	PORTD7
#define DRV_STEP_PIN	PORTB0
#define DRV_DIR_PIN		PORTB1

#define BUTTON			(PINC & (1<<PINC3))
#define SWITCH			(PINC & (1<<PINC4))

// BUTTON STATE MACROS
#define BTN_IDLE		0xEE
#define BTN_PUSHED		0xDD
#define BTN_RELEASED	0xAA

// MANUAL MOVEMENT TYPES
#define SPEED_LINEAR		1
#define SPEED_EXPONENTIAL 	2
#define QUIT_MENU 			0

typedef enum {
	STATE_HOMING,
	STATE_CHOOSE_MOVEMENT,
	STATE_CHOOSE_MANUAL_MOVEMENT,
	STATE_CHOOSE_CONTROL,
	STATE_MANUAL_MOVEMENT,
	STATE_CREATE_MOVEMENT,
	STATE_FAIL
} state_t;

typedef struct {
	volatile uint32_t position;
	volatile uint16_t speed;
	volatile uint16_t target_speed;
	volatile uint8_t marginal_zone;
	volatile uint8_t out_of_bounds;
	volatile uint8_t sw;
	volatile uint8_t spin;
} slider_s;

typedef struct {
	volatile uint8_t update;
	volatile uint8_t dir;
} encoder_s;

typedef struct {
	volatile uint8_t query; // flag; query button state
	uint8_t action;			// flag; button action activated
	uint8_t lock;			// flag; button locked
	uint8_t state;			// button state: IDLE, PUSHED, RELEASED
	uint16_t count;			// time counter
	uint8_t delay1;			// flag; delay 1 elapsed
	uint8_t delay2;			// flag; delay 2 elapsed
	uint8_t delay3;			// flag; delay 3 elapsed
} btn_s;

extern slider_s slider;
extern encoder_s encoder;
extern btn_s btn;
extern volatile uint16_t ms;
extern volatile uint8_t speed_update;

#endif /* CONFIG_H_ */