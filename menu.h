
#ifndef MENU_H
#define MENU_H

#include "config.h"
#include <stdlib.h>

uint8_t homing(void);

state_t choose_movement(void);

uint8_t choose_manual_movement(void);

uint8_t manual_movement(uint8_t type);

void user_movement(void);

#endif /* MENU_H */