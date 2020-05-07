
#ifndef MENU_H
#define MENU_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "encoder.h"
#include "lcd.h"
#include "motor.h"
#include "move.h"
#include "util.h"
#include "uart.h"

/******************************************************************************
****************** F U N C T I O N   D E C L A R A T I O N S ******************
******************************************************************************/

int8_t choose_action(void);

// Manual movement related functions
int8_t choose_control_type(void);
int8_t choose_speed_profile(void);

// Automatic movement related functions
int32_t user_set_time(int32_t xi, int32_t xo);
int8_t user_set_reps(void);
int8_t user_set_loop(void);
int8_t user_set_accel(void);

void fail_message(void);

#endif /* MENU_H */