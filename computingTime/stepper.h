

#ifndef STEPPER_H_
#define STEPPER_H_

#include "config.h"

#include <stdint.h>

typedef struct {
	
} stepper_s;


void stepper_init(void (*timer)(uint8_t state, uint16_t t), void (*timer_raw)(uint16_t c));

uint8_t stepper_move_to_pos(int32_t p);

void stepper_set_accel(int32_t a);

#endif