/*
 * lcd.c
 *
 *  Created on: Dec 7, 2024
 *      Author: yunus
 */


#include "lcd.h"


static void write_4_bits(uint8_t value);
static void lcd_enable();
static void delay_ms(uint32_t cnt);
static void delay_us(uint32_t cnt);




void lcd_send_command(uint8_t cmd) {

	//RS = 0, For LCD command
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);

	//R/nW = 0, Writing to LCD
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(cmd >> 4);
	write_4_bits(cmd & 0x0F);

}


/*
 *This function sends a character to the LCD
 *Here we used 4 bit parallel data transmission.
 *First higher nibble of the data will be sent on to the data lines D4,D5,D6,D7
 *Then lower nibble of the data will be set on to the data lines D4,D5,D6,D7
 */
void lcd_print_char(uint8_t data) {

	//RS = 1, For user data
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_SET);

	//R/nW = 0, Writing to LCD
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(data >> 4);	//higher nibble
	write_4_bits(data & 0x0F);	//lower nibble

}


void lcd_print_string(char *message) {

	do {
		lcd_print_char((uint8_t)*message++);
	}while(*message != '\0');
}


void lcd_init(void) {

	//1. Configure the gpio pins which are used for lcd connections
	GPIO_Handle_t lcd_signal;

	lcd_signal.pGPIOx = LCD_GPIO_PORT;
	lcd_signal.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	lcd_signal.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
	lcd_signal.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	lcd_signal.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_RS;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_RW;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_EN;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D4;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D5;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D6;
	GPIO_Init(&lcd_signal);

	lcd_signal.GPIO_PinConfig.GPIO_PinNumber = LCD_GPIO_D7;
	GPIO_Init(&lcd_signal);

	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_RESET);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D4, GPIO_PIN_RESET);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D5, GPIO_PIN_RESET);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D6, GPIO_PIN_RESET);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D7, GPIO_PIN_RESET);

	//2. Do the LCD initialization

	delay_ms(40);

	//RS = 0, For LCD command
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RS, GPIO_PIN_RESET);

	//R/nW = 0, Writing to LCD
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_RW, GPIO_PIN_RESET);

	write_4_bits(0x3);	//0 0 1 1
	delay_ms(5);

	write_4_bits(0x3);	//0 0 1 1
	delay_us(150);

	write_4_bits(0x3);	//0 0 1 1
	write_4_bits(0x2);	//0 0 1 0

	//function set command
	lcd_send_command(LCD_CMD_4DL_2N_5X8F);

	//disply ON and cursor ON
	lcd_send_command(LCD_CMD_DON_CURON);

	//display clear
	lcd_display_clear();

	//entry mode set
	lcd_send_command(LCD_CMD_INCADD);
}

//Writes 4 bits of data/command on to D4,D5,D6,D7 lines
static void write_4_bits(uint8_t value) {

	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D4, ((value >> 0) & 0x1) );
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D5, ((value >> 1) & 0x1) );
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D6, ((value >> 2) & 0x1) );
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_D7, ((value >> 3) & 0x1) );

	lcd_enable();	//latching signal
}


void lcd_display_clear(void) {
	//Display clear
	lcd_send_command(LCD_CMD_DIS_CLEAR);

	/*
	 * check page number 24 of datasheet.
	 * display clear command execution wait time is around 2ms
	 */

	delay_ms(2);
}


void lcd_display_return_home(void) {

	lcd_send_command(LCD_CMD_DIS_RETURN_HOME);

	/*
	 * check page number 24 of datasheet.
	 * return home command execution wait time is around 2ms
	 */
	delay_ms(2);
}


/**
  *   Set Lcd to a specified location given by row and column information
  *   Row Number (1 to 2)
  *   Column Number (1 to 16) Assuming a 2 X 16 characters display
  */
void lcd_set_cursor(uint8_t row, uint8_t column) {
	column--;
	switch (row) {

		case 1:
			/* Set cursor to 1st row address and add index*/
			lcd_send_command((column |= 0x80));
			break;

		case 2:
			/* Set cursor to 2nd row address and add index*/
			lcd_send_command((column |= 0xC0));
			break;

		default:
			break;
	}
}


static void lcd_enable() {
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_SET);
	delay_us(10);
	WriteToOutputPin(LCD_GPIO_PORT, LCD_GPIO_EN, GPIO_PIN_RESET);
	delay_us(150); //execution time > 37 micro seconds
}


static void delay_ms(uint32_t cnt) {
	for(uint32_t i=0; i < (cnt * 1000); i++);
}

static void delay_us(uint32_t cnt) {
	for(uint32_t i=0; i < (cnt * 1); i++);
}
