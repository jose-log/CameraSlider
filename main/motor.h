
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

#define SPEED_MAX 			8000.0		// Valid for MODE_EIGHTH_STEP
#define ACCEL_MAX 			8000.0		// Valid for MODE_EIGHTH_STEP
#define ACCEL_MIN 			1862.0		// Valid for 2MHz timer frequency
#define SPEED_MIN 		((float)F_MOTOR/(CMIN_MAX + 1.0))

#define PROFILE_LINEAR		0x01
#define PROFILE_QUADRATIC	0x02

#define REL 				0x11
#define ABS 				0x12

#define POSITION_CONTROL	0x71
#define SPEED_CONTROL 		0x70

#define SOFT_STOP 			0x30
#define HARD_STOP 			0x31

#define CMIN_MAX  		65535.0		// 2^16 - 1: max OCR1A value
#define MAX_LENGHT_CMS	((int32_t) 80)
#define CMS_PER_REV 	((int32_t) 4)
#define STEPS_PER_REV	((int32_t) 1600)	// EIGHTH stepping
#define MAX_COUNT		(MAX_LENGHT_CMS * (STEPS_PER_REV / CMS_PER_REV))

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void motor_init(void);

void motor_move_to_pos(int32_t p, uint8_t mode, uint8_t limits);
int8_t motor_move_to_pos_block(int32_t pos, uint8_t mode, uint8_t limits);
void motor_move_at_speed(int8_t s);

uint16_t motor_get_speed(void);
int8_t motor_get_speed_percent(void);
int16_t motor_get_accel(void);
uint8_t motor_get_profile(void);
int32_t motor_get_position(void);
uint8_t motor_get_dir(void);

void motor_stop(uint8_t type);

void motor_set_position(int32_t p);
int8_t motor_set_maxspeed_percent(uint8_t speed);
int8_t motor_set_maxspeed(float speed);
int8_t motor_set_accel_percent(uint8_t accel);
void motor_set_speed_profile(uint8_t p);

uint8_t motor_working(void);

#endif