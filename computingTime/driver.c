
#include "driver.h"
#include "config.h"

#include <avr/io.h>

void drv_step_mode(uint8_t mode){

	switch(mode){

		case MODE_FULL_STEP:
			DRV_MS3_PORT &= ~(1<<DRV_MS3_PIN);
			DRV_MS2_PORT &= ~(1<<DRV_MS2_PIN);
			DRV_MS1_PORT &= ~(1<<DRV_MS1_PIN);
			break;

		case MODE_HALF_STEP:
			DRV_MS3_PORT &= ~(1<<DRV_MS3_PIN);
			DRV_MS2_PORT &= ~(1<<DRV_MS2_PIN);
			DRV_MS1_PORT |= (1<<DRV_MS1_PIN);
			break;

		case MODE_QUARTER_STEP:
			DRV_MS3_PORT &= ~(1<<DRV_MS3_PIN);
			DRV_MS2_PORT |= (1<<DRV_MS2_PIN);
			DRV_MS1_PORT &= ~(1<<DRV_MS1_PIN);
			break;

		case MODE_EIGHTH_STEP:
			DRV_MS3_PORT &= ~(1<<DRV_MS3_PIN);
			DRV_MS2_PORT |= (1<<DRV_MS2_PIN);
			DRV_MS1_PORT |= (1<<DRV_MS1_PIN);
			break;

		case MODE_SIXTEENTH_STEP:
			DRV_MS3_PORT |= (1<<DRV_MS3_PIN);
			DRV_MS2_PORT |= (1<<DRV_MS2_PIN);
			DRV_MS1_PORT |= (1<<DRV_MS1_PIN);
			break;

		default:
			DRV_MS3_PORT &= ~(1<<DRV_MS3_PIN);
			DRV_MS2_PORT &= ~(1<<DRV_MS2_PIN);
			DRV_MS1_PORT &= ~(1<<DRV_MS1_PIN);
			break;
	}
}

void drv_spin_direction(uint8_t dir){

	if(dir) {
		DRV_DIR_PORT |= (1<<DRV_DIR_PIN);
	} else {
		DRV_DIR_PORT &= ~(1<<DRV_DIR_PIN);
	}
}	