
#ifndef LCD_H_
#define LCD_H_

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "motor.h"

#include <stdint.h>

/******************************************************************************
*******************	C O N S T A N T S  D E F I N I T I O N S ******************
******************************************************************************/

#define LCD_CLEAR_DISPLAY		0b00000001
#define LCD_FUNCTION_SET		0b00101000		// Data length 4 bits; Display lines 2; Character font 5x7 dots
#define LCD_DISPLAY_OFF			0b00001000		// Display off; Cursor off; blink off
#define LCD_DISPLAY_ON			0b00001100		// Display on; Cursor off; blink off
#define LCD_ENTRY_MODE			0b00000110		// Increment mode; No display shift

// Display Screens:
typedef enum {
	SCREEN_WELCOME,
	SCREEN_HOMING,
	SCREEN_HOMING_DONE,
	SCREEN_CHOOSE_ACTION,
	SCREEN_CHOOSE_CONTROL_TYPE,
	SCREEN_MOTOR_POSITION,
	SCREEN_MOTOR_SPEED,
	SCREEN_CHOOSE_SPEED_PROFILE,
	SCREEN_INITIAL_POSITION,
	SCREEN_FINAL_POSITION,
	SCREEN_CHOOSE_TIME,
	SCREEN_CHOOSE_REPS,
	SCREEN_CHOOSE_LOOP,
	SCREEN_CHOOSE_ACCEL,
	SCREEN_WAIT_TO_GO,
	SCREEN_GO,
	SCREEN_FINISHED,
	SCREEN_STOP,
	SCREEN_FAIL_MESSAGE
} screen_t;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void lcd_send_byte(uint8_t rs, uint8_t data);
void lcd_init(void);
void lcd_write_char(char c);
void lcd_write_str(char *c);
void lcd_set_cursor(uint8_t row, uint8_t column);
void lcd_clear_screen(void);

void lcd_screen(screen_t screen);
void lcd_update_speed(uint16_t speed);
void lcd_update_position(int32_t pos);
void lcd_update_time(float t);
void lcd_update_reps(uint8_t r);
void lcd_update_loop(uint8_t l);
void lcd_update_time_moving(uint16_t t);
void lcd_update_percent(int8_t percentage);

void lcd_write_loop(void);

#endif /* LCD_H_ */