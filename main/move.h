
#ifndef MOVE_H
#define MOVE_H

#include "config.h"
#include "driver.h"
#include "lcd.h"
#include "motor.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

#include <stdint.h>

struct auto_s {
	int32_t initial_pos;
	int32_t final_pos;
	int32_t speed;
	uint8_t reps;
	uint8_t loop;		// flag
	int8_t accel;
	uint8_t go; 		// flag
};

int8_t homing_cycle(void);

int8_t manual_speed(void);

uint8_t manual_position(void);

int32_t user_set_position(uint8_t p);

int8_t user_go_to_init(int32_t pos);

int8_t user_gogogo(struct auto_s m);

#endif /* MOVE_H */