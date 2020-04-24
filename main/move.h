
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
	int32_t time;
	uint8_t reps;
	uint8_t loop;		// flag
	int8_t ramp;
	uint8_t go; 		// flag
};

int8_t homing_cycle(void);

int8_t manual_speed(void);

uint8_t manual_position(void);

int32_t user_set_position(uint8_t p);


#endif /* MOVE_H */