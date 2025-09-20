#include "LCD.h"


void LCD_init(void)
{
	LCD_GPIO_init();
	
	LCD_RW_LOW;

	LCD_RS_LOW;
	delay_ms(10);
	toggle_EN_pin();

	LCD_send(0x33, CMD);
	LCD_send(0x32, CMD);

	LCD_send((_4_pin_interface | _2_row_display | _5x7_dots), CMD);
	LCD_send((display_on | cursor_off | blink_off), CMD);
	LCD_clear_home();
	LCD_send((cursor_direction_inc | display_no_shift), CMD);
}


void LCD_send(uint8_t value, uint8_t mode)
{
	switch(mode)
	{
		case DAT:
		{
			LCD_RS_HIGH;
			break;
		}
		case CMD:
		{
			LCD_RS_LOW;
			break;
		}
	}

	LCD_4bit_send(value);
}


void LCD_4bit_send(uint8_t lcd_data)
{
	uint8_t temp = 0;
	
	temp = ((lcd_data & 0x80) >> 7);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB7_HIGH;
			break;
		}
		default:
		{
			LCD_DB7_LOW;
			break;
		}
	}
	
	temp = ((lcd_data & 0x40) >> 6);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB6_HIGH;
			break;
		}
		default:
		{
			LCD_DB6_LOW;
			break;
		}
	}
	
	temp = ((lcd_data & 0x20) >> 5);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB5_HIGH;
			break;
		}
		default:
		{
			LCD_DB5_LOW;
			break;
		}
	}
	
	temp = ((lcd_data & 0x10) >> 4);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB4_HIGH;
			break;
		}
		default:
		{
			LCD_DB4_LOW;
			break;
		}
	}

	toggle_EN_pin();

	temp = ((lcd_data & 0x08) >> 3);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB7_HIGH;
			break;
		}
		default:
		{
			LCD_DB7_LOW;
			break;
		}
	}
	
	temp = ((lcd_data & 0x04) >> 2);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB6_HIGH;
			break;
		}
		default:
		{
			LCD_DB6_LOW;
			break;
		}
	}
	
	temp = ((lcd_data & 0x02) >> 1);
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB5_HIGH;
			break;
		}
		default:
		{
			LCD_DB5_LOW;
			break;
		}
	}
	
	temp = ((lcd_data & 0x01));
	
	switch(temp)
	{
		case 1:
		{ 
			LCD_DB4_HIGH;
			break;
		}
		default:
		{
			LCD_DB4_LOW;
			break;
		}
	}

	toggle_EN_pin();
}


void LCD_putchar(uint8_t x_pos, uint8_t y_pos, char char_data)
{
	LCD_goto(x_pos, y_pos);	
	LCD_send(char_data, DAT);
}


void LCD_text(uint8_t x_pos, uint8_t y_pos, const char *lcd_string)
{
	while(*lcd_string != '\0')
	{
		LCD_putchar(x_pos++, y_pos, *lcd_string++);
	}
}


void LCD_clear_home(void)
{
	LCD_send(clear_display, CMD);
	LCD_send(goto_home, CMD);
}


void LCD_goto(uint8_t x_pos, uint8_t y_pos)
{
	switch(y_pos)
	{
		case 1:
		{
			LCD_send((0x80 | (line_2_y_pos + x_pos)), CMD);
			break;
		}
		case 2:
		{
			LCD_send((0x80 | (line_3_y_pos + x_pos)), CMD);
			break;
		}
		case 3:
		{
			LCD_send((0x80 | (line_4_y_pos + x_pos)), CMD);
			break;
		}
		default:
		{
			LCD_send((0x80 | (line_1_y_pos + x_pos)), CMD);
			break;
		}
	}
}


void toggle_EN_pin(void)
{
	LCD_EN_HIGH;
	delay_ms(1);
	LCD_EN_LOW;
	delay_ms(1);
}
