
#ifndef ENCODER_H
#define ENCODER_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S ****************
******************************************************************************/

struct btn_s {
	volatile uint8_t query; // flag; query button state
	uint8_t action;			// flag; button action activated
	uint8_t lock;			// flag; button locked
	uint8_t state;			// button state: IDLE, PUSHED, RELEASED
	uint16_t count;			// time counter
	uint8_t delay1;			// flag; delay 1 elapsed
	uint8_t delay2;			// flag; delay 2 elapsed
	uint8_t delay3;			// flag; delay 3 elapsed
};

struct enc_s {
	volatile uint8_t update;	// flag: encoder position has changed
	volatile uint8_t dir;		// rotation direction
};

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

// BUTTON STATE MACROS
#define BTN_IDLE		0xEE
#define BTN_PUSHED		0xDD
#define BTN_RELEASED	0xAA

/******************************************************************************
****************** F U N C T I O N   D E C L A R A T I O N S ******************
******************************************************************************/

// Initialization
void encoder_init(void);
void limit_switch_init(void);

// Encoder related functions
uint8_t encoder_get_update(void);
void encoder_set_update(uint8_t state);
uint8_t encoder_get_dir(void);
struct enc_s *encoder_get(void);

// Limit switch related fuctions
void limit_switch_ISR(uint8_t state);
volatile uint8_t *limit_switch_get(void);
uint8_t limit_switch_test(void);

// Encoder button related functions
struct btn_s *button_get(void);
uint8_t button_test(void);
void button_check(void);

#endif /* ENCODER_H */