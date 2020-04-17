
#include "driver.h"
#include "config.h"

#include <avr/io.h>
#include <util/delay.h>

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

void drv_dir(uint8_t dir, uint8_t *var){

	if(dir == CW) {
		DRV_DIR_PORT |= (1<<DRV_DIR_PIN);
		*var = CW;
	} else {
		DRV_DIR_PORT &= ~(1<<DRV_DIR_PIN);
		*var = CCW;
	}
}

void drv_set(uint8_t state) {

	if (state) {
		DRV_EN_PORT &= ~(1<<DRV_EN_PIN);
	} else {
		DRV_EN_PORT |= (1<<DRV_EN_PIN);
	}
	_delay_us(100);

}

void drv_reset(void) {

	DRV_RST_PORT &= ~(1<<DRV_RST_PIN);
	_delay_us(5);
	DRV_RST_PORT |= (1<<DRV_RST_PIN);
	_delay_us(95);
}