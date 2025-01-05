/*
 * 017rtc_lcd.c
 *
 *  Created on: Dec 5, 2024
 *      Author: yunus
 */

#include <stdio.h>
#include "ds1307.h"
#include "lcd.h"

#define PRINT_LCD
#define SYSTICK_TIM_CLK		16000000UL

static void delay_ms(uint32_t cnt);

void init_systick_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

    /* calculation of reload value */
    uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;

    //Clear the value of SVR
    *pSRVR &= ~(0x00FFFFFFFF);

    //load the value in to SVR
    *pSRVR |= count_value;

    //do some settings
    *pSCSR |= ( 1 << 1); //Enables SysTick exception request:
    *pSCSR |= ( 1 << 2);  //Indicates the clock source, processor clock source

    //enable the systick
    *pSCSR |= ( 1 << 0); //enables the counter

}


char* get_day_of_week(uint8_t d) {
	char* days[] = {" <SUN>", " <MON>", " <TUE>", " <WED>", " <THU>", " <FRI>", " <SAT>"};
	return(days[d-1]);
}


void num_to_str(uint8_t num, char* buf) {
	if(num < 10 ) {
		buf[0] = '0';
		buf[1] = num + 48;
	}else if(num >= 10 && num <= 99) {
		buf[0] = (num/10) + 48;
		buf[1] = (num%10) + 48;
	}
}


//hh:mm:ss
char* time_to_str(RTC_time_t *rtc_time) {

	static char buf[9];

	buf[2] = ':';
	buf[5] = ':';

	num_to_str(rtc_time->hours, buf);
	num_to_str(rtc_time->minutes, &buf[3]);
	num_to_str(rtc_time->seconds, &buf[6]);

	buf[8] = '\0';

	return buf;
}


//dd/mm/yy
char* date_to_str(RTC_date_t *rtc_date) {

	static char buf[9];

	buf[2] = '/';
	buf[5] = '/';

	num_to_str(rtc_date->date, buf);
	num_to_str(rtc_date->month, &buf[3]);
	num_to_str(rtc_date->year, &buf[6]);

	buf[8] = '\0';

	return buf;
}

int main(void) {

	RTC_date_t current_date;
	RTC_time_t current_time;

	printf("RTC test\n");

	lcd_init();

	lcd_print_string("RTC Test...");

	delay_ms(2000);

	lcd_display_clear();
	lcd_display_return_home();

	if(ds1307_init()) {
		printf("RTC init has failed\n");
		while(1);
	}

	init_systick_timer(1);

	//set current date
	current_date.date = 7;
	current_date.month = 12;
	current_date.year = 24;
	current_date.day = SATURDAY;

	//set current time
	current_time.seconds = 45;
	current_time.minutes = 59;
	current_time.hours = 11;
	current_time.time_format = TIME_FORMAT_12HRS_PM;

	ds1307_set_current_date(&current_date);
	ds1307_set_current_time(&current_time);

	ds1307_get_current_date(&current_date);
	ds1307_get_current_time(&current_time);

	//print current time
	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HRS) {
		am_pm = (current_time.time_format) ?  " PM" : " AM";
		//printf("Current time: %s %s\n", time_to_str(&current_time), am_pm);	//04:30:15 PM
		lcd_print_string(time_to_str(&current_time));
		lcd_print_string(am_pm);
	}else {
		//printf("Current time: %s\n", time_to_str(&current_time));	//16:30:15
		lcd_print_string(time_to_str(&current_time));
	}

	//print current date
	//format: DD/MM/YY <DAY>
	//printf("Current date: %s <%s>\n", date_to_str(&current_date), get_day_of_week(current_date.day));
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_str(&current_date));
	lcd_print_string(get_day_of_week(current_date.day));

	return(0);
}

void SysTick_Handler(void) {

	RTC_date_t current_date;
	RTC_time_t current_time;

	ds1307_get_current_date(&current_date);
	ds1307_get_current_time(&current_time);

	lcd_display_return_home();

	//print current time
	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HRS) {
		am_pm = (current_time.time_format) ?  " PM" : " AM";
		//printf("Current time: %s %s\n", time_to_str(&current_time), am_pm);	//04:30:15 PM
		lcd_print_string(time_to_str(&current_time));
		lcd_print_string(am_pm);
	}else {
		//printf("Current time: %s\n", time_to_str(&current_time));	//16:30:15
		lcd_print_string(time_to_str(&current_time));
	}

	//print current date
	//format: DD/MM/YY <DAY>
	//printf("Current date: %s <%s>\n", date_to_str(&current_date), get_day_of_week(current_date.day));
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_str(&current_date));
	lcd_print_string(get_day_of_week(current_date.day));

}


static void delay_ms(uint32_t cnt) {
	for(uint32_t i = 0; i < (cnt * 1000); i++);
}

