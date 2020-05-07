
/*
* Driver A4988 interface module
*/

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "driver.h"

#include <avr/io.h>
#include <util/delay.h>

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
/*
* STEPPING MODE:
* Even though all possible stepping modes included in the driver are available
* to be selected, all movement computations are based on the MODE_EIGHTH_STEP.
*
* Toggles the Driver Pins to select the proper stepping mode.
*/
void drv_step_mode(uint8_t mode)
{
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

/*===========================================================================*/
/*
* Driver spin direction.
* - Toggles the driver direction pin to select the proper direction
* - Modifies the "dir" variable value, to avoid manually doing it outside of
* 	this function.
* 
* Parameters:
*	- dir: Clock Wise or Counter Clock Wise
* 	- *var: direction variable pointer
*/
void drv_dir(uint8_t dir, volatile uint8_t *var)
{
	if(dir == CW) {
		DRV_DIR_PORT |= (1<<DRV_DIR_PIN);
		*var = CW;
	} else {
		DRV_DIR_PORT &= ~(1<<DRV_DIR_PIN);
		*var = CCW;
	}
}

/*===========================================================================*/
/*
* Driver ENABLE/DISABLE
* Toggles driver ENable pin
*/
void drv_set(uint8_t state)
{
	if (state) {
		DRV_EN_PORT &= ~(1<<DRV_EN_PIN);
	} else {
		DRV_EN_PORT |= (1<<DRV_EN_PIN);
	}
	_delay_us(100);

}

/*===========================================================================*/
/*
* Driver Reset
* Toggles driver Reset pin to reset the internal driver counter.
* When used, the driver forgets the current motor windings energization state
* and resets the internal counter.
*/
void drv_reset(void)
{
	DRV_RST_PORT &= ~(1<<DRV_RST_PIN);
	_delay_us(5);
	DRV_RST_PORT |= (1<<DRV_RST_PIN);
	_delay_us(95);
}