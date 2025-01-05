/*
 * stm32f407xx_rcc_driver.c
 *
 *  Created on: Dec 4, 2024
 *      Author: yunus
 */

#include "stm32f407xx_rcc_driver.h"


uint16_t AHB_PreScalar[8] = {2,4,8,16,64,128,256,512};
uint8_t	APB_PreScalar[4] = {2,4,8,16};



uint32_t RCC_GetPCLK1Value(void) {

	uint32_t pclk1, SystemClk;
	uint8_t clksrc, temp, ahbp, apb1p;

	clksrc = (RCC->CFGR >> 2) & 0x3;
	if(clksrc == 0) {
		//HSI oscillator used as the system clock
		SystemClk = 16000000U;

	}else if(clksrc == 1) {
		//HSE oscillator used as the system clock
		SystemClk = 80000000U;

	}else if(clksrc == 2){
		//PLL used as the system clock
		SystemClk = RCC_GetPLLOutputClock();
	}

	//for AHB
	temp = (RCC->CFGR >> 4) & 0xF;
	if(temp < 8) {
		ahbp = 1;

	}else {
		ahbp = AHB_PreScalar[temp-8];
	}

	//for APB1
	temp = (RCC->CFGR >> 10) & 0x7;
	if(temp < 4) {
		apb1p = 1;

	}else {
		apb1p = APB_PreScalar[temp-4];
	}

	pclk1 = SystemClk / ahbp / apb1p;

	return(pclk1);
}

uint32_t RCC_GetPCLK2Value(void) {

	uint32_t pclk2, SystemClk;
	uint8_t clksrc, temp, ahbp, apb2p;

	clksrc = (RCC->CFGR >> 2) & 0x3;
	if(clksrc == 0) {
		//HSI oscillator used as the system clock
		SystemClk = 16000000U;

	}else if(clksrc == 1) {
		//HSE oscillator used as the system clock
		SystemClk = 80000000U;

	}else if(clksrc == 2){
		//PLL used as the system clock
		SystemClk = RCC_GetPLLOutputClock();
	}

	//for AHB
	temp = (RCC->CFGR >> 4) & 0xF;
	if(temp < 8) {
		ahbp = 1;

	}else {
		ahbp = AHB_PreScalar[temp-8];
	}

	//for APB2
	temp = (RCC->CFGR >> 13) & 0x7;
	if(temp < 4) {
		apb2p = 1;

	}else {
		apb2p = APB_PreScalar[temp-4];
	}

	pclk2 = SystemClk / ahbp / apb2p;

	return(pclk2);
}


uint32_t RCC_GetPLLOutputClock(void) {
	uint32_t PLLClk = 0;
	return(PLLClk);
}
