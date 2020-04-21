
#ifndef MENU_H
#define MENU_H

#include "config.h"
#include "encoder.h"
#include "motor.h"

int8_t homing(void);

int8_t choose_action(void);

int8_t choose_control_type(void);

uint8_t choose_speed_profile(void);

uint8_t manual_speed(void);

uint8_t manual_position(void);

void user_movement(void);

void fail_message(void);

#endif /* MENU_H */