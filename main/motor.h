
#ifndef STEPPER_H_
#define STEPPER_H_

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "driver.h"
#include "encoder.h"
#include "timers.h"
#include "uart.h"

/******************************************************************************
***************** G L O B A L   S C O P E   V A R I A B L E S *****************
******************************************************************************/

#define PROFILE_LINEAR		0x01
#define PROFILE_QUADRATIC	0x02

#define REL 				0x11
#define ABS 				0x12

#define POSITION_CONTROL	0x71
#define SPEED_CONTROL 		0x70

#define MAX_LENGHT_CMS	((uint32_t) 80)
#define CMS_PER_REV 	((uint32_t) 4)
#define STEPS_PER_REV	((uint32_t) 1600)	// EIGHTH stepping
#define MAX_COUNT		((uint32_t) (MAX_LENGHT_CMS * (STEPS_PER_REV / CMS_PER_REV)))

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void motor_init(void);

void motor_move_to_pos(int32_t p, uint8_t mode);

int8_t motor_move_to_pos_block(int32_t pos, uint8_t mode);

void motor_move_at_speed(int8_t s);

uint16_t motor_get_speed(void);

uint8_t motor_get_profile(void);

uint8_t motor_get_control(void);

int32_t motor_get_position(void);

volatile uint8_t *motor_get_dir(void);

void motor_set_position(int32_t p);

void motor_stop(uint8_t type);

int8_t motor_set_maxspeed_percent(uint8_t speed);

int8_t motor_set_accel_percent(uint8_t accel);

void motor_set_speed_profile(uint8_t p);

#endif