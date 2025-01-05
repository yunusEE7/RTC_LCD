/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Jul 24, 2024
 *      Author: yunus
 */


#include "stm32f407xx_spi_driver.h"

//helper functions (private to this file)
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);


/* Peripheral clock setup */
/*********************************************************************
 * @fn				- SPI_PeriClockControl
 *
 * @brief			- Enables or disables SPI peripheral clock
 *
 * @param[in]
 * @*pSPIx			- Base address of the SPI peripheral
 * @EnorDi			- ENABLE or DISABLE macros
 *
 * @return			- none
 *
 * @Note			- none

 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi) {

	if(EnorDi == ENABLE) {
		if(pSPIx == SPI1) {
			SPI1_PCLK_EN();
		}else if(pSPIx == SPI2_I2S) {
			SPI2_PCLK_EN();
		}else if(pSPIx == SPI3_I2S) {
			SPI3_PCLK_EN();
		}
	}else {
		if(pSPIx == SPI1) {
			SPI1_PCLK_DI();
		}else if(pSPIx == SPI2_I2S) {
			SPI2_PCLK_DI();
		}else if(pSPIx == SPI3_I2S) {
			SPI3_PCLK_DI();
		}
	}

}



/* Init and DeInit */
/*********************************************************************
 * @fn      		  - SPI_Init
 *
 * @brief             - Initializes SPI peripheral
 *
 * @param[in]
 * @*pSPIHandle       - Handle structure for a SPI peripheral
 *
 * @return            - none
 *
 * @Note              - none
 *
 */
void SPI_Init(SPI_Handle_t *pSPIHandle) {

	//configure SPI_CR1 register

	uint32_t tempreg = 0;

	//enable the peripheral clock
	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

	//1. configure the device mode
	tempreg |= pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR;

	//2. configure the bus config
	if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD) {
		//BIDI mode should be cleared
		tempreg &= ~( 1 << SPI_CR1_BIDI_MODE );

		//specific debug
		//tempreg |= ( 1 << SPI_CR1_BIDI_OE );


	}else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD) {
		//BIDI mode should be set
		tempreg |= ( 1 << SPI_CR1_BIDI_MODE );

	}else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIM_RXONLY) {
		//BIDI mode should be cleared
		tempreg &= ~( 1 << SPI_CR1_BIDI_MODE );
		//RXONLY bit should be set
		tempreg |= ( 1 << SPI_CR1_RXONLY );
	}

	//3. configure SPI serial clock speed (baud rate)
	tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR;

	//4. configure data frame format
	tempreg |= pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;

	//5. configure clock polarity(CPOL)
	tempreg |= pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;

	//6. configure clock phase(CPHA)
	tempreg |= pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

	//7. configure software slave management(SSM)
	tempreg |= pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM;

	pSPIHandle->pSPIx->CR[0] = tempreg;

}


/*********************************************************************
 * @fn      		  - SPI_DeInit
 *
 * @brief             - Resets SPIx peipheral configuration value
 *
 * @param[in]
 * @*pSPIx			  - Base address of SPIx peripheral
 *
 * @return            - none
 *
 * @Note              - none

 */
void SPI_DeInit(SPI_RegDef_t *pSPIx) {
	if(pSPIx == SPI1){
		SPI1_REG_RESET();
	}
	else if(pSPIx == SPI2_I2S){
		SPI2_I2S_REG_RESET();
	}
	else if(pSPIx == SPI3_I2S){
		SPI3_I2S_REG_RESET();
	}
}


uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName) {

	if(pSPIx->SR & FlagName) {
		return(FLAG_SET);
	}
	else {
		return(FLAG_RESET);
	}

}

/* Data send and receive */

/*1. Blocking Type API*/

/*********************************************************************
 * @fn      		  - SPI_SendData
 *
 * @brief             - fn to send data over SPI protocol
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - None
 *
 * @Note              - This is a blocking call / polling type

 */
/*
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len) {

	__vo uint8_t TXE = ( (pSPIx->SR >> SPI_SR_TXE) & 1 );
	uint32_t i = Len;
	while(Len > 0) {
		while(!TXE); //wait until Tx buffer is empty
		while(i>0) {
			if( ((pSPIx->CR[0] >> SPI_CR1_DFF) & 1) == 0) {
				pSPIx->DR = *pTxBuffer; //Load data to shift register
				pTxBuffer++;
				Len--;
			}
			else {
				pSPIx->DR = *pTxBuffer;
				pTxBuffer += 2;
				Len -= 2;
			}
		}
	}
*/

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len) {

	while(Len > 0) {

		//wait until Tx buffer is empty
		while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

		//Check the DFF bit in CR1
		if( (pSPIx->CR[0] & ( 1 << SPI_CR1_DFF)) ) {
			//16 bit DFF
			//Load data to shift register
			pSPIx->DR = *((uint16_t*)pTxBuffer);
			Len -=2;
			(uint16_t*)pTxBuffer++;
		}else {
			//8 bit DFF
			pSPIx->DR = *pTxBuffer;
			Len--;
			pTxBuffer++;
		}
	} //while(Len > 0) END
}


/*********************************************************************
 * @fn      		  - SPI_ReceiveData
 *
 * @brief             - fn to receive data over SPI protocol
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - None
 *
 * @Note              - This is a blocking call / polling type

 */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len) {

	while(Len > 0) {

		//wait until Rx buffer is full
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

		//Check the DFF bit in CR1
		if( (pSPIx->CR[0] & ( 1 << SPI_CR1_DFF)) ) {
			//16 bit DFF
			//Load data from DR to RxBuffer address
			*((uint16_t*)pRxBuffer) = pSPIx->DR;
			Len -=2;
			(uint16_t*)pRxBuffer++;
		}else {
			//8 bit DFF
			*pRxBuffer = pSPIx->DR;
			Len--;
			pRxBuffer++;
		}
	} //while(Len > 0) END
}


/*********************************************************************
 * @fn      		  - SPI_SendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - uint8_t state
 *
 * @Note              - Interrupt based SPI peripheral

 */
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len) {

	uint8_t state = pSPIHandle->TxState;

	if(state != SPI_BUSY_IN_TX) {
		//1. Save the Tx buffer address and Len info. in some
		//global variables(i.e. Handle structure)
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;

		//2. Mark the SPI state as busy in Tx. so that
		//no other code can take over same SPI periph. until Tx. is over.
		pSPIHandle->TxState = SPI_BUSY_IN_TX;

		//3. Enable TXEIE ctrl. bit to get interrupt whenever TXE flag is SET in SR.
		pSPIHandle->pSPIx->CR[1] |= ( 1 << SPI_CR2_TXEIE);

		//4. Data Tx. will be handled by the ISR code. (will be implemented later)
	}
	return(state);
}


/*********************************************************************
 * @fn      		  - SPI_ReceiveDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - uint8_t state
 *
 * @Note              - Interrupt based SPI peripheral

 */
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len) {

	uint8_t state = pSPIHandle->RxState;

	if(state != SPI_BUSY_IN_RX) {
		//1. Save the Tx buffer address and Len info. in some
		//global variables(i.e. Handle structure)
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;

		//2. Mark the SPI state as busy in Tx. so that
		//no other code can take over same SPI periph. until Tx. is over.
		pSPIHandle->RxState = SPI_BUSY_IN_RX;

		//3. Enable RXNEIE ctrl. bit to get interrupt whenever RXNE flag is RESET in SR.
		pSPIHandle->pSPIx->CR[1] |= ( 1 << SPI_CR2_RXNEIE);

		//4. Data Tx. will be handled by the ISR code. (will be implemented later)
	}
	return(state);
}


/* IRQ configuration and ISR handling */
/*********************************************************************
 * @fn      		  - SPI_IRQInterruptConfig
 *
 * @brief             - configure SPI interrupts
 *
 * @param[in]         -
 * @IRQNumber         - IRQ Number
 * @EnorDi            - Enable or Disable Macro
 *
 * @return            - None
 *
 * @Note              -

 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi) {

	/* since SP1, SPI2, SPI3 IRQ numbers are 35, 36, 51 respectively,
	 * they all lie in ISER1/ICER1.
	 */
	if(EnorDi == ENABLE) {
		//Program ISER1 register
		*NVIC_ISER1 |= ( 1 << ( IRQNumber % 32 ));
	}else {
		//Program ICER1 register
		*NVIC_ICER1 |= ( 1 << ( IRQNumber % 32 ));
	}

}


/*********************************************************************
 * @fn      		  - SPI_IRQPriorityConfig
 *
 * @brief             - Configure priority of the interrupts
 *
 * @param[in]         -
 * @IRQNumber         - IRQ Number
 * @IRQPriority       - IRQ Priority number
 *
 * @return            - None
 *
 * @Note              -

 */
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority) {

	//find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED));
	//*(NVIC_PR_BASEADDR + (iprx * 4)) |= ( IRQPriority << shift_amount); error: pointer addition
	*(NVIC_PR_BASEADDR + iprx) |= ( IRQPriority << shift_amount);

}


/*********************************************************************
 * @fn      		  - SPI_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void SPI_IRQHandling(SPI_Handle_t *pSPIHandle) {

	uint8_t temp1, temp2;
	//first lets check for TXE flag
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_TXE);
	temp2 = pSPIHandle->pSPIx->CR[1] & (1 << SPI_CR2_TXEIE);

	if( temp1 && temp2) {
		//handle TXE
		spi_txe_interrupt_handle(pSPIHandle);
	}

	//check for RXNE flag
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_RXNE);
	temp2 = pSPIHandle->pSPIx->CR[1] & (1 << SPI_CR2_RXNEIE);

	if( temp1 && temp2) {
		//handle RXNE
		spi_rxne_interrupt_handle(pSPIHandle);
	}

	//check for OVR flag
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_OVR);
	temp2 = pSPIHandle->pSPIx->CR[1] & (1 << SPI_CR2_ERRIE);

	if( temp1 && temp2) {
		//handle OVR flag
		spi_ovr_err_interrupt_handle(pSPIHandle);
	}

	/*
	uint32_t tempreg = pSPIHandle->pSPIx->SR;

	if(tempreg & (1 << SPI_SR_TXE)) {
		//Interrupt due to TXE flag

	}else if(tempreg & (1 << SPI_SR_RXNE)){
		//Interrupt due to RXNE flag
	}

	else if(tempreg & ((1<<SPI_SR_FRE) | (1 << SPI_SR_OVR) | (1 << SPI_SR_CRCERR))) {
		//Interrupt due to error flags
	}
	*/

}


//some helper functions
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle) {
	//Check the DFF bit in CR1
	if( (pSPIHandle->pSPIx->CR[0] & ( 1 << SPI_CR1_DFF)) ) {
		//16 bit DFF
		//Load data to shift register
		pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen -=2;
		(uint16_t*)pSPIHandle->pTxBuffer++;
	}else {
		//8 bit DFF
		pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}

	if( !(pSPIHandle->TxLen)) {
		//TxLen is zero, so close the spi communication
		//and inform the application that Tx is over

		//this prevents interrupts from setting up of TXE flag
		/*
		pSPIHandle->pSPIx->CR[1] &= ~( 1 << SPI_CR2_TXEIE);
		pSPIHandle->pTxBuffer = NULL;
		pSPIHandle->TxLen = 0;
		pSPIHandle->TxState = SPI_READY;
		*/
		SPI_CloseTransmission(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
	}
}


static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle) {
	//Check the DFF bit in CR1
	if( (pSPIHandle->pSPIx->CR[0] & ( 1 << SPI_CR1_DFF)) ) {
		//16 bit DFF
		//Load data to shift register
		*((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen -=2;
		(uint16_t*)pSPIHandle->pRxBuffer++;
	}else {
		//8 bit DFF
		*pSPIHandle->pRxBuffer = (uint8_t)pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;
	}

	if( !(pSPIHandle->RxLen)) {
		//RxLen is zero, so close the spi communication
		//and inform the application that Rx is over

		//this prevents interrupts from setting up of TXE flag
		/*
		pSPIHandle->pSPIx->CR[1] &= ~( 1 << SPI_CR2_RXNEIE);
		pSPIHandle->pRxBuffer = NULL;
		pSPIHandle->RxLen = 0;
		pSPIHandle->RxState = SPI_READY;
		*/
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
	}
}


static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle) {

	uint32_t temp;
	//1. clear the OVR flag
	if(pSPIHandle->TxState != SPI_SR_BSY) {
		temp = pSPIHandle->pSPIx->DR;
		temp = pSPIHandle->pSPIx->SR;
	}
	(void)temp;
	//2. inform the application
	SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}


void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle) {
	pSPIHandle->pSPIx->CR[1] &= ~( 1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;
}


void SPI_CloseReception(SPI_Handle_t *pSPIHandle) {
	pSPIHandle->pSPIx->CR[1] &= ~( 1 << SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;
}


void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx) {
	uint8_t temp;
	temp = pSPIx->DR;
	temp = pSPIx->SR;
	(void)temp;
}


__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv) {
		//This is a weak implementation. The application may override
		//this function


}


/*Other functions */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi) {
	if(EnorDi == ENABLE) {
		pSPIx->CR[0] |= (1 << SPI_CR1_SPE);
	}else {
		pSPIx->CR[0] &= ~(1 << SPI_CR1_SPE);
	}
}


void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi) {
	if(EnorDi == ENABLE) {
		pSPIx->CR[0] |= (1 << SPI_CR1_SSI);
	}else {
		pSPIx->CR[0] &= ~(1 << SPI_CR1_SSI);
	}
}


void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnorDi) {
	if(EnorDi == ENABLE) {
		pSPIx->CR[1] |= (1 << SPI_CR2_SSOE);
	}else {
		pSPIx->CR[1] &= ~(1 << SPI_CR2_SSOE);
	}
}

