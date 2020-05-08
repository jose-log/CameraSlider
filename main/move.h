
#ifndef MOVE_H
#define MOVE_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "driver.h"
#include "lcd.h"
#include "motor.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

#include <stdint.h>

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S ****************
******************************************************************************/

struct auto_s {
	int32_t initial_pos;
	int32_t final_pos;
	int32_t speed;
	uint8_t reps;
	uint8_t loop;		// flag
	int8_t accel;
	uint8_t go; 		// flag
};

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

int8_t homing(void);

// functions related to manual movement
int8_t manual_speed(void);
uint8_t manual_position(void);

// functions related to automatic movement
int32_t user_set_position(uint8_t p);
int8_t user_go_to_init(int32_t pos);
int8_t user_gogogo(struct auto_s m);

#endif /* MOVE_H */