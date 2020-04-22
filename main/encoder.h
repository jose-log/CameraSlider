

#ifndef ENCODER_H
#define ENCODER_H

#include "config.h"

struct btn_s {
	volatile uint8_t query; // flag; query button state
	uint8_t action;			// flag; button action activated
	uint8_t lock;			// flag; button locked
	uint8_t state;			// button state: IDLE, PUSHED, RELEASED
	uint16_t count;			// time counter
	uint8_t delay1;			// flag; delay 1 elapsed
	uint8_t delay2;			// flag; delay 2 elapsed
	uint8_t delay3;			// flag; delay 3 elapsed
};

struct enc_s {
	volatile uint8_t update;
	volatile uint8_t dir;
};

// BUTTON STATE MACROS
#define BTN_IDLE		0xEE
#define BTN_PUSHED		0xDD
#define BTN_RELEASED	0xAA

void encoder_init(void);

void limit_switch_init(void);

void limit_switch_ISR(uint8_t state);

uint8_t encoder_get_update(void);

void encoder_set_update(uint8_t state);

uint8_t encoder_get_dir(void);

struct btn_s *button_get(void);

struct enc_s *encoder_get(void);

volatile uint8_t *limit_switch_get(void);

uint8_t limit_switch_test(void);

uint8_t button_test(void);

void button_check(void);

#endif /* ENCODER_H */