
#ifndef DRIVER_H
#define DRIVER_H

#include <avr/io.h>

#define MODE_FULL_STEP 		0
#define MODE_HALF_STEP		1
#define MODE_QUARTER_STEP	2
#define MODE_EIGHTH_STEP	3
#define MODE_SIXTEENTH_STEP	4

void drv_step_mode(uint8_t mode);

void drv_dir(uint8_t dir, uint8_t *var);

void drv_set(uint8_t state);

void drv_reset(void);

extern uint8_t spin;

#endif /* DRIVER_H */