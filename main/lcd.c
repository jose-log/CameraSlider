/*
 * lcd.c
 *
 * Created: 15-Oct-18 9:25:48 PM
 *  Author: josel
 */ 

#include "lcd.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

static void lcd_enable(void)
{
	PORTB |= (1<<PORTB4);
	_delay_us(1);
	PORTB &= ~(1<<PORTB4);
	_delay_us(1);
}

static void lcd_send_nibble(uint8_t rs, uint8_t data)
{
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

void lcd_send_byte(uint8_t rs, uint8_t data)
{
	uint8_t nibble;
	nibble = (data >> 4);
	lcd_send_nibble(rs, nibble);
	nibble = (data & 0x0F);
	lcd_send_nibble(rs, nibble);
	_delay_us(40);
}

void lcd_init(void)
{
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

void lcd_write_char(char c)
{	
	lcd_send_byte(1, c);
}

void lcd_write_str(char *c)
{
	while(*c != '\0'){
		lcd_write_char(*c);
		c++;
	}
}

void lcd_set_cursor(uint8_t row, uint8_t column)
{
	uint8_t addr;
	
	if(row != 0) addr = 0x40 + column;
	else addr = column;

	lcd_send_byte(0, ((1<<7) | addr));
	_delay_ms(5);
}

void lcd_clear_screen(void)
{
	lcd_send_byte(0, LCD_CLEAR_DISPLAY);
	_delay_ms(2);
}

void lcd_screen(screen_t screen)
{
	uint8_t pro;

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
	
		case SCREEN_MOTOR_POSITION:
			pro = motor_get_profile();
			lcd_clear_screen();
			lcd_set_cursor(1,0);
			lcd_write_str("Pos:");
			lcd_set_cursor(1,8);
			lcd_write_str("mm");
			if (pro == PROFILE_LINEAR) {
				lcd_set_cursor(0,10);
				lcd_write_str("linear");
			} else if (pro == PROFILE_QUADRATIC) {
				lcd_set_cursor(0,7);
				lcd_write_str("quadratic");
			}
			break;

		case SCREEN_MOTOR_SPEED:
			pro = motor_get_profile();
			lcd_clear_screen();
			lcd_set_cursor(1,0);
			lcd_write_str("Speed:");
			lcd_set_cursor(1,11);
			lcd_write_str("cms/s");
			if (pro == PROFILE_LINEAR) {
				lcd_set_cursor(0,10);
				lcd_write_str("linear");
			} else if (pro == PROFILE_QUADRATIC) {
				lcd_set_cursor(0,7);
				lcd_write_str("quadratic");
			}
			break;

		case SCREEN_CHOOSE_ACTION:
			lcd_clear_screen();
			lcd_write_str(">Create Movement");
			lcd_set_cursor(1,0);
			lcd_write_str(" Manual Movement");
			break;

		case SCREEN_CHOOSE_CONTROL_TYPE:
			lcd_clear_screen();
			lcd_write_str(">Position ctl.");
			lcd_set_cursor(1,0);
			lcd_write_str(" Speed ctl.");
			break;

		case SCREEN_CHOOSE_SPEED_PROFILE:
			lcd_clear_screen();
			lcd_write_str("> Linear");
			lcd_set_cursor(1,0);
			lcd_write_str("  Quadratic");
			break;

		case SCREEN_FAIL_MESSAGE:
			lcd_clear_screen();
			lcd_write_str(" ...ooops =(");
			lcd_set_cursor(1,0);
			lcd_write_str(" <ERROR>");
			break;

		case SCREEN_INITIAL_POSITION:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("Initial");
			lcd_set_cursor(1,0);
			lcd_write_str("Pos:");
			lcd_set_cursor(1,8);
			lcd_write_str("mm");
			break;

		case SCREEN_FINAL_POSITION:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("Final");
			lcd_set_cursor(1,0);
			lcd_write_str("Pos:");
			lcd_set_cursor(1,8);
			lcd_write_str("mm");
			break;

		case SCREEN_CHOOSE_TIME:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("Duration:");
			break;

		case SCREEN_CHOOSE_REPS:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("Repetitions:");
			lcd_set_cursor(1,0);
			lcd_write_str("    times");
			break;

		case SCREEN_CHOOSE_LOOP:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("Loop:");
			lcd_set_cursor(1,0);
			lcd_write_str(" FALSE");
			break;

		case SCREEN_CHOOSE_ACCEL:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("Accel:");
			lcd_set_cursor(1,0);
			lcd_write_str("    %");
			break;

		case SCREEN_WAIT_TO_GO:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("  PRESS TO GO!  ");
			lcd_set_cursor(1,0);
			lcd_write_str(" cam @ init pos ");
			break;

		case SCREEN_GO:
			lcd_clear_screen();
			lcd_set_cursor(0,0);
			lcd_write_str("> GO!!!");
			lcd_set_cursor(0,15);
			lcd_write_str("%");
			lcd_set_cursor(1,0);
			lcd_write_str("time:");
			break;		
	}
}

void lcd_update_speed(uint16_t speed)
{
	// used to convert OCR1A value to cms_per_second
	float freq, rps, f_speed;
	// used to display speed
	uint8_t integer, decimal;
	char integer_str[5], decimal_str[5];
	float float_part;
	// system constants
	static const float f_motor = (float)F_MOTOR;
	static const float steps_per_rev = (float)STEPS_PER_REV;
	static const float cms_per_rev = (float)CMS_PER_REV;

	f_speed = (float)speed;
	if(speed != 0){
		freq = f_motor/(f_speed + 1.0);		// pulses frequency
		rps = freq / steps_per_rev;			// revolutions per second
		f_speed = rps * cms_per_rev;		// centimeters per second
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

void lcd_update_position(int32_t pos)
{
	// used to display speed
	char str[5];
	
	pos =	pos / ((STEPS_PER_REV / CMS_PER_REV) / 10);

	itoa(pos, str, 10);			// convert to string
	
	lcd_set_cursor(1,4);
	lcd_write_str("    ");
	if (pos < 10) lcd_set_cursor(1,7);			// one digit
	else if (pos < 100) lcd_set_cursor(1,6);	// two digits
	else if (pos < 1000) lcd_set_cursor(1,5);	// three digits
	else if (pos < 10000) lcd_set_cursor(1,4);	// four digits
	lcd_write_str(str);
}

void lcd_update_time(float time)
{
	char str[6];

	lcd_set_cursor(1,0);
	lcd_write_str("      ");
	lcd_set_cursor(1,0);
	if (time < 60.0) {
		// display as is
		float time_int = (int32_t)time;
		if ((time - time_int) == 0.0) 
			ltoa((int16_t)time, str, 10);
		else 
			dtostrf(time, 3, 1, str);
		lcd_write_str(str);
		lcd_write_char('s');
	} else if (time < 3600.0) {
		// display minutes and seconds
		uint16_t mins = (uint16_t)time / 60;
		uint16_t secs = (uint16_t)time % 60;
		itoa((int32_t)mins, str, 10);
		lcd_write_str(str);
		lcd_write_char('m');
		if (secs) {
			itoa((int16_t)secs, str, 10);
			lcd_write_str(str);
			lcd_write_char('s');			
		}
	} else {
		// display hours
		uint16_t hours = (uint16_t)time / 3600.0;
		itoa((int32_t)hours, str, 10);
		lcd_write_str(str);
		lcd_write_char('h');
	}
}

void lcd_update_reps(uint8_t r)
{
	char str[6];

	lcd_set_cursor(1,0);
	lcd_write_str("    ");
	lcd_set_cursor(1,1);
	itoa((int16_t)r, str, 10);
	lcd_write_str(str);
}

void lcd_update_loop(uint8_t l)
{
	lcd_set_cursor(1,1);
	if (l) lcd_write_str("TRUE ");
	else lcd_write_str("FALSE");
}

void lcd_update_time_remaining(uint16_t t)
{
	char str[6];

	lcd_set_cursor(1,5);
	lcd_write_str("      ");
	lcd_set_cursor(1,5);

	ltoa((uint32_t)t, str, 10);
	lcd_write_str(str);
	lcd_write_char('s');
}

// debug ------------

const uint8_t ascii_table[] = {
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
};

void lcd_update_cnt(uint8_t cnt)
{
	lcd_set_cursor(0,0);
	uint8_t h, t, u;

	h = cnt / 100;
	t = (cnt % 100) / 10;
	u = cnt % 10;

	lcd_write_char(ascii_table[h]);
	lcd_write_char(ascii_table[t]);
	lcd_write_char(ascii_table[u]);
}