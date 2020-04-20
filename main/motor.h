

#ifndef STEPPER_H_
#define STEPPER_H_

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "driver.h"
#include "timers.h"
#include "uart.h"

/******************************************************************************
******************* S T R U C T U R E   P R O T O T Y P E S *******************
******************************************************************************/

typedef struct {
	
} motor_s;

/******************************************************************************
***************** G L O B A L   S C O P E   V A R I A B L E S *****************
******************************************************************************/

#define PROFILE_LINEAR		0x01
#define PROFILE_QUADRATIC	0x02

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void motor_init(void);

void motor_move_to_pos(int32_t p, uint8_t mode);

void motor_move_to_pos_block(int32_t p, uint8_t mode);

void motor_move_at_speed(int8_t s);

void motor_stop(void);

int8_t motor_set_maxspeed_percent(uint8_t speed);

int8_t motor_set_accel_percent(uint8_t accel);

#endif