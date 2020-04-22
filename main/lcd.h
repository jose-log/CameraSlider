
#ifndef LCD_H_
#define LCD_H_

#include "config.h"
#include "motor.h"

#include <stdint.h>

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
	SCREEN_FAIL_MESSAGE
} screen_t;

void lcd_send_byte(uint8_t rs, uint8_t data);
void lcd_init(void);
void lcd_write_char(char c);
void lcd_write_str(char *c);
void lcd_set_cursor(uint8_t row, uint8_t column);
void lcd_clear_screen(void);

void lcd_screen(screen_t screen);
void lcd_update_speed(uint16_t speed);
void lcd_update_position(int32_t pos);

// debug ----------
void lcd_update_cnt(uint8_t cnt);

#endif /* LCD_H_ */