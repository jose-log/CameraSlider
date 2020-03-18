/*
 * lcd.c
 *
 * Created: 15-Oct-18 9:25:48 PM
 *  Author: josel
 */ 

#include "lcd.h"
#include "config.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

const uint8_t ascii_table[] = {
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
};

static void lcd_enable(void){

	PORTB |= (1<<PORTB4);
	_delay_us(1);
	PORTB &= ~(1<<PORTB4);
	_delay_us(1);
}

static void lcd_send_nibble(uint8_t rs, uint8_t data){

	// RS
	if(rs) PORTB |= (1<<PORTB2);
	else PORTB &= ~(1<<PORTB2);
	// D4
	if(data & 0x01)	PORTC |= (1<<PORTC2);
	else PORTC &= ~(1<<PORTC2);
	// D5
	if(data & 0x02)	PORTC |= (1<<PORTC1);
	else PORTC &= ~(1<<PORTC1);
	// D6
	if(data & 0x04)	PORTC |= (1<<PORTC0);
	else PORTC &= ~(1<<PORTC0);
	// D7
	if(data & 0x08)	PORTB |= (1<<PORTB5);
	else PORTB &= ~(1<<PORTB5);

	_delay_us(1);
	lcd_enable();
}

void lcd_send_byte(uint8_t rs, uint8_t data){

	uint8_t nibble;
	nibble = (data >> 4);
	lcd_send_nibble(rs, nibble);
	nibble = (data & 0x0F);
	lcd_send_nibble(rs, nibble);
	_delay_us(40);
}

void lcd_init(void){

	_delay_ms(15);
	lcd_send_nibble(0, 0x3);
	_delay_ms(5);
	lcd_send_nibble(0, 0x3);
	_delay_us(100);
	lcd_send_nibble(0, 0x3);
	_delay_ms(5);
	lcd_send_nibble(0, 0x2);
	_delay_us(40);

	lcd_send_byte(0, LCD_FUNCTION_SET);
	lcd_send_byte(0, LCD_DISPLAY_OFF);
	lcd_send_byte(0, LCD_CLEAR_DISPLAY);
	_delay_ms(2);
	lcd_send_byte(0, LCD_ENTRY_MODE);
	lcd_send_byte(0, LCD_DISPLAY_ON);
}

void lcd_write_char(char c){
	
	lcd_send_byte(1, c);
}

void lcd_write_str(char *c){
	
	while(*c != '\0'){
		lcd_write_char(*c);
		c++;
	}
}

void lcd_set_cursor(uint8_t row, uint8_t column){

	uint8_t addr;
	
	if(row != 0) addr = 0x40 + column;
	else addr = column;

	lcd_send_byte(0, ((1<<7) | addr));
	_delay_ms(5);
}

void lcd_clear_screen(void){

	lcd_send_byte(0, LCD_CLEAR_DISPLAY);
	_delay_ms(2);
}

void lcd_welcome_screen(void){

	lcd_set_cursor(0,2);
	lcd_write_str("Slider PRO");
	lcd_set_cursor(1,1);
	lcd_write_str("David Logreira");
	_delay_ms(1000);
}

void lcd_screen(uint8_t screen){

	switch(screen){

		case SCREEN_WELCOME:
			lcd_set_cursor(0,2);
			lcd_write_str("Slider PRO");
			lcd_set_cursor(1,1);
			lcd_write_str("David Logreira");
			_delay_ms(1000);
			break;

		case SCREEN_HOMING:
			lcd_clear_screen();
			lcd_set_cursor(0,3);
			lcd_write_str("< Homing >");
			break;

		case SCREEN_HOMING_DONE:
			lcd_set_cursor(1,5);
			lcd_write_str("DONE!");
			_delay_ms(1000);
			break;
	
		case SCREEN_LINEAR_SPEED:
			lcd_clear_screen();	
			lcd_set_cursor(0,10);
			lcd_write_str("linear");
			lcd_set_cursor(1,0);
			lcd_write_str("Speed:");
			lcd_set_cursor(1,11);
			lcd_write_str("cms/s");
			break;

		case SCREEN_EXPONENTIAL_SPEED:
			lcd_clear_screen();	
			lcd_set_cursor(0,5);
			lcd_write_str("exponential");
			lcd_set_cursor(1,0);
			lcd_write_str("Speed:");
			lcd_set_cursor(1,11);
			lcd_write_str("cms/s");
			break;

		case SCREEN_CHOOSE_MOVEMENT:
			lcd_clear_screen();
			lcd_write_str(">Create Movement");
			lcd_set_cursor(1,0);
			lcd_write_str(" Manual Movement");
			break;

		case SCREEN_CHOOSE_MANUAL_MOVEMENT:
			lcd_clear_screen();
			lcd_write_str("> Linear");
			lcd_set_cursor(1,0);
			lcd_write_str("  Exponential");
			break;
	}
}

#define TIMER_FREQUENCY  	250.0e3
#define PULSES_PER_REV 		3.2e3
#define CMS_PER_REVOL 		4.0

void lcd_update_speed(uint16_t speed){

	// used to convert OCR1A value to cms_per_second
	float freq, rps, f_speed;
	// used to display speed
	uint8_t integer, decimal;
	char integer_str[5], decimal_str[5];
	float float_part;

	f_speed = (float)speed;
	if(speed != 0){
		freq = TIMER_FREQUENCY/(f_speed + 1.0);	// pulses frequency
		rps = freq / PULSES_PER_REV;			// revolutions per second
		f_speed = rps * CMS_PER_REVOL;			// centimeters per second
	}

	integer = (uint8_t)f_speed;				// Extract integer part
	float_part = f_speed - (float)integer;	// Extract floating part
	decimal = (uint8_t)(float_part * 100.0);// two decimal digits

	itoa(integer, integer_str, 10);			// convert integer part to string
	itoa(decimal, decimal_str, 10);			// convert decimal part to string

	lcd_set_cursor(1,6);
	lcd_write_str("     ");
	lcd_set_cursor(1,6);
	lcd_write_str(integer_str);
	lcd_write_char('.');
	if(decimal < 10)
		lcd_write_char('0');	
	lcd_write_str(decimal_str);
}

// debug ------------

void lcd_update_cnt(uint8_t cnt){

	lcd_set_cursor(0,0);
	uint8_t h, t, u;

	h = cnt / 100;
	t = (cnt % 100) / 10;
	u = cnt % 10;

	lcd_write_char(ascii_table[h]);
	lcd_write_char(ascii_table[t]);
	lcd_write_char(ascii_table[u]);
}