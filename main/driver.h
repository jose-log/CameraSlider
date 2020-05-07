
#ifndef DRIVER_H
#define DRIVER_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

#include <stdint.h>

/******************************************************************************
********************** M A C R O S   D E F I N I T I O N **********************
******************************************************************************/

// Stepping modes
#define MODE_FULL_STEP 		0
#define MODE_HALF_STEP		1
#define MODE_QUARTER_STEP	2
#define MODE_EIGHTH_STEP	3
#define MODE_SIXTEENTH_STEP	4

// DRIVER ports
#define DRV_EN_PORT 	PORTD
#define DRV_MS1_PORT 	PORTC
#define DRV_MS2_PORT	PORTD
#define DRV_MS3_PORT	PORTD
#define DRV_RST_PORT	PORTD
#define DRV_SLEEP_PORT	PORTD
#define DRV_STEP_PORT	PORTB
#define DRV_DIR_PORT	PORTB

#define DRV_EN_PIN 		PORTD7
#define DRV_MS1_PIN 	PORTC5
#define DRV_MS2_PIN		PORTD4
#define DRV_MS3_PIN		PORTD5
#define DRV_RST_PIN		PORTD6
#define DRV_SLEEP_PIN	PORTD7
#define DRV_STEP_PIN	PORTB0
#define DRV_DIR_PIN		PORTB1

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void drv_step_mode(uint8_t mode);
void drv_dir(uint8_t dir, volatile uint8_t *var);
void drv_set(uint8_t state);
void drv_reset(void);

#endif /* DRIVER_H */

