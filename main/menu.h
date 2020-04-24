
#ifndef MENU_H
#define MENU_H

#include "config.h"
#include "encoder.h"
#include "lcd.h"
#include "motor.h"
#include "move.h"
#include "util.h"

int8_t homing(void);

int8_t choose_action(void);

int8_t choose_control_type(void);

int8_t choose_speed_profile(void);

int32_t user_set_time(void);

int8_t user_set_reps(void);

int8_t user_set_loop(void);

int8_t user_set_accel(void);

void fail_message(void);

#endif /* MENU_H */