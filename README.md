README

Project Aim: Microcontroller is configured to continously read the data from DS1307 RTC module and display the time and date on 1602 LCD display and sync RTC clock with PC clock if required.

Components used in this project:
1. STM32F407 Discovery Board
2. DS1307 RTC
3. 1602 LCD
4. CP2102 USB to UART TTL Converter module
5. Jumper wires
6. Bread board

Interfacing
In this project, microcontroller is interfaced with DS1307 RTC via I2C, and LCD with 4 wire configuration, CP2102 USB to UART is used for syncing PC time with RTC time.

Bugs encountered: Syncing once works fine, multiple time syncing causes RTC data corruption and shows random time and date.
