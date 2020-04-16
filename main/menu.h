
#ifndef MENU_H
#define MENU_H

#include "config.h"
#include <stdlib.h>

uint8_t homing(void);

state_t choose_movement(void);

uint8_t choose_manual_control(void);

uint8_t choose_manual_movement(void);

uint8_t manual_speed(uint8_t type);

uint8_t manual_position(uint8_t type);

void user_movement(void);

void fail_message(void);

#endif /* MENU_H */