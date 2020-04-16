
#include "util.h"
#include "config.h"

#include <stdint.h>

// Button time counts: These macros determine the time it takes for 
// different flags within btnXYZ structure to be set (in milliseconds)
#define BTN_LOCK_TIME	30		// lock time after button released
#define BTN_DLY1_TIME	300		// time for delay 1
#define BTN_DLY2_TIME	65		// time for delay 2
#define BTN_DLY3_TIME	2000	// time for delay 3
#define BTN_BEEP_TIME	50		// duration of beep sound

void btn_check(void){ 

	switch(btn.state){

		case BTN_IDLE:
			if(!BUTTON) btn.count++;
			else if(btn.count > 0) btn.count--;

			if(btn.count == 7){
				btn.action = TRUE;
				btn.lock = TRUE;
				btn.state = BTN_PUSHED;
				btn.count = 0;
				//buzzer_set(ENABLE, N_C8);
			} else if(btn.count == 0){
				btn.action = FALSE;
				btn.lock = FALSE;
				btn.query = FALSE;
			}
			break;

		case BTN_PUSHED:
			if(!BUTTON){
				btn.count++;	
			} else {
				btn.count = 0;
				btn.state = BTN_RELEASED;
			}
			if(btn.count == BTN_BEEP_TIME); //buzzer_set(DISABLE, N_C8);
			if(btn.count == BTN_DLY1_TIME) btn.delay1 = TRUE;
			if((btn.delay1) && (!(btn.count % BTN_DLY2_TIME))) btn.delay2 = TRUE;
			if(btn.count >= BTN_DLY3_TIME) btn.delay3 = TRUE;
			break;

		case BTN_RELEASED:
			if(BUTTON)
				btn.count++;
			if(btn.count == BTN_LOCK_TIME){
				btn.query = FALSE;
				btn.action = FALSE;
				btn.lock = FALSE;
				btn.state = BTN_IDLE;
				btn.count = 0;
				btn.delay1 = FALSE;
				btn.delay2 = FALSE;
				btn.delay3 = FALSE;
				//buzzer_set(DISABLE, N_C8);
			}
			break;
		default:
			break;
	}
}

uint16_t millis(void){

	return ms;
}

void clear_millis(void){

	ms = 0;
}