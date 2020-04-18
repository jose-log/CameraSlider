

#ifndef STEPPER_H_
#define STEPPER_H_

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

#include <stdint.h>

/******************************************************************************
******************* S T R U C T U R E   P R O T O T Y P E S *******************
******************************************************************************/

typedef struct {
	
} motor_s;

/******************************************************************************
***************** G L O B A L   S C O P E   V A R I A B L E S *****************
******************************************************************************/

extern volatile int32_t current_pos;
extern volatile int32_t target_pos;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void motor_init(void);

void motor_move_to_pos(int32_t p, uint8_t mode);

void motor_move_to_pos_block(int32_t p, uint8_t mode);

void motor_stop(void);

int8_t motor_set_maxspeed_percent(uint8_t speed);

int8_t motor_set_accel_percent(uint8_t accel);

#endif