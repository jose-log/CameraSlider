

#ifndef STEPPER_H_
#define STEPPER_H_

#include "config.h"

#include <stdint.h>

typedef struct {
	
} motor_s;

#define PROFILE_LINEAR		0x01
#define PROFILE_QUADRATIC	0x02


void motor_init(void);

void motor_move_to_pos(int32_t p, uint8_t mode);

void motor_move_to_pos_block(int32_t p, uint8_t mode);

void motor_stop(void);

int8_t motor_set_maxspeed_percent(uint8_t speed);

int8_t motor_set_accel_percent(uint8_t accel);

void motor_move_at_speed(int8_t s);

void motor_speed_profile(uint8_t p);

extern volatile int32_t current_pos;
extern volatile int32_t target_pos;

#endif