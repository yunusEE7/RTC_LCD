/*
 * stm32f407xx_gpio.c
 *
 *  Created on: Apr 7, 2024
 *      Author: yunus
 */


//#include "stm32f407xx_gpio_driver.h"
#include "stm32f407xx.h"

/*********************************************************************
 * @fn      		  - GPIO_PeriClockControl
 *
 * @brief             - This function enables or disables peripheral clock for the given GPIO port
 *
 * @param[in]         - base address of the gpio peripheral
 * @param[in]         - ENABLE or DISABLE macros
 * @param[in]         -
 *
 * @return            -  none
 *
 * @Note              -  none

 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi) {

	if(EnorDi == ENABLE) {
		if(pGPIOx == GPIOA) {
			GPIOA_PCLK_EN();
		}else if(pGPIOx == GPIOB) {
			GPIOB_PCLK_EN();
		}else if(pGPIOx == GPIOC) {
			GPIOC_PCLK_EN();
		}else if(pGPIOx == GPIOD) {
			GPIOD_PCLK_EN();
		}else if(pGPIOx == GPIOE) {
			GPIOE_PCLK_EN();
		}else if(pGPIOx == GPIOF) {
			GPIOF_PCLK_EN();
		}else if(pGPIOx == GPIOG) {
			GPIOG_PCLK_EN();
		}else if(pGPIOx == GPIOH) {
			GPIOH_PCLK_EN();
		}else if(pGPIOx == GPIOI) {
			GPIOI_PCLK_EN();
		}
	}else {
		if(pGPIOx == GPIOA) {
			GPIOA_PCLK_DI();
		}else if(pGPIOx == GPIOB) {
			GPIOB_PCLK_DI();
		}else if(pGPIOx == GPIOC) {
			GPIOC_PCLK_DI();
		}else if(pGPIOx == GPIOD) {
			GPIOD_PCLK_DI();
		}else if(pGPIOx == GPIOE) {
			GPIOE_PCLK_DI();
		}else if(pGPIOx == GPIOF) {
			GPIOF_PCLK_DI();
		}else if(pGPIOx == GPIOG) {
			GPIOG_PCLK_DI();
		}else if(pGPIOx == GPIOH) {
			GPIOH_PCLK_DI();
		}else if(pGPIOx == GPIOI) {
			GPIOI_PCLK_DI();
		}
	}
}


/*********************************************************************
 * @fn				  - GPIO_Init
 *
 * @brief             - Initializes GPIO peripheral pin
 *
 * @param[in]         - GPIO Handle structure of GPIO pin
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - none
 *
 * @Note              - none

 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle) {

	uint32_t temp=0; //temp. reg.

	//enable the peripheral clock
	GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);

	//config the mode of gpio pin
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG) {
		temp = ( pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ));
		pGPIOHandle->pGPIOx->MODER &= ~( 0x3 << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ));	//bit clear
		pGPIOHandle->pGPIOx->MODER |= temp;		//bit set

	}else {
		if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT) {
			//1. config. the FTSR
			EXTI->FTSR |= ( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
			//Clear the corresponding RTSR bit
			EXTI->RTSR &= ~( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;

		}else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT) {
			//1. config. the RTSR
			EXTI->RTSR |= ( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
			//Clear the corresponding FTSR bit
			EXTI->FTSR &= ~( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;

		}else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT) {
			//1. config. both FTSR and RTSR
			EXTI->RTSR |= ( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
			EXTI->FTSR |= ( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
		}

		//2. config. the GPIO port selection in SYSCFG_EXTICR
		uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4;
		uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;
		uint8_t portcode = GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);
		SYSCFG_PCLK_EN();
		SYSCFG->EXTICR[temp1] = ( portcode << ( temp2 *4 ));

		//3. enable the EXTI interrupt delivery using IMR
		EXTI->IMR |= ( 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;

	}



	temp = 0;

	//config the speed
	temp = ( pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ));
	pGPIOHandle->pGPIOx->OSPEEDR &= ~( 0x3 << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ));	//bit clear
	pGPIOHandle->pGPIOx->OSPEEDR |= temp;

	temp = 0;

	//config the pupd setting
	temp = ( pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ));
	pGPIOHandle->pGPIOx->PUPDR &= ~( 0x3 << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ));	//bit clear
	pGPIOHandle->pGPIOx->PUPDR |= temp;

	temp = 0;

	//config the optype
	temp = ( pGPIOHandle->GPIO_PinConfig.GPIO_PinOpType << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber );
	pGPIOHandle->pGPIOx->OTYPER &= ~( 0x1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber );	//bit clear
	pGPIOHandle->pGPIOx->OTYPER |= temp;

	temp = 0;

	//config the alt functionality
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN ) {
		//configure the alt function registers.
		uint8_t temp1, temp2;

		temp1 = (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) / 8;
		temp2 = (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) % 8;

		pGPIOHandle->pGPIOx->AFR[temp1] &= ~( 0xF << ( 4 * temp2 ));
		pGPIOHandle->pGPIOx->AFR[temp1] |= ( pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << ( 4 * temp2 ));
	}
}

/*********************************************************************
 * @fn      		  - GPIO_DeInit
 *
 * @brief             - resets GPIO peripheral port
 *
 * @param[in]         - base address of GPIO port
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - none
 *
 * @Note              - none

 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx) {
	if(pGPIOx == GPIOA) {
		GPIOA_REG_RESET();
	}else if(pGPIOx == GPIOB) {
		GPIOB_REG_RESET();
	}else if(pGPIOx == GPIOC) {
		GPIOC_REG_RESET();
	}else if(pGPIOx == GPIOD) {
		GPIOD_REG_RESET();
	}else if(pGPIOx == GPIOE) {
		GPIOE_REG_RESET();
	}else if(pGPIOx == GPIOF) {
		GPIOF_REG_RESET();
	}else if(pGPIOx == GPIOG) {
		GPIOG_REG_RESET();
	}else if(pGPIOx == GPIOH) {
		GPIOH_REG_RESET();
	}else if(pGPIOx == GPIOI) {
		GPIOI_REG_RESET();
	}
}


/*********************************************************************
 * @fn      		  - ReadFromInputPin
 *
 * @brief             - Read bit of a GPIO pin from Input data register
 *
 * @param[in]         - GPIO peripheral base address
 * @param[in]         - GPIO pin number
 * @param[in]         -
 *
 * @return            - 0 or 1
 *
 * @Note              - none
 */
uint8_t ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber) {
	uint8_t value;
	value = (uint8_t)(( pGPIOx->IDR >> PinNumber ) & ( 0x00000001 ));
	return value;
}


/*********************************************************************
 * @fn      		  - ReadFromInputPort
 *
 * @brief             - Reads value of entire GPIO port
 *
 * @param[in]         - GPIO peripheral base address
 * @param[in]         -
 * @param[in]         -
 *
 * @return            - half word integer
 *
 * @Note              - none

 */
uint16_t ReadFromInputPort(GPIO_RegDef_t *pGPIOx) {
	uint16_t value;
	value = (uint16_t)(pGPIOx->IDR);
	return value;
}


/*********************************************************************
 * @fn      		  - WriteToOutputPin
 *
 * @brief             - SET or RESET GPIO pin value
 *
 * @param[in]         - Base address of GPIO peripheral
 * @param[in]         - Pin number of GPIO peripheral
 * @param[in]         - value to be written
 *
 * @return            - none
 *
 * @Note              - none

 */
void WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value) {
	if(Value == GPIO_PIN_SET){
		pGPIOx->ODR |= ( 1 << PinNumber );
	}else {
		pGPIOx->ODR &= ~( 1 << PinNumber );
	}
}


/*********************************************************************
 * @fn      		  - WriteToOutputPort
 *
 * @brief             - Writes to entire GPIO port
 *
 * @param[in]         - Base address of GPIO peripheral
 * @param[in]         - value to be written
 * @param[in]         -
 *
 * @return            - none
 *
 * @Note              - none

 */
void WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value) {
	pGPIOx->ODR = (uint32_t)(Value);
}


/*********************************************************************
 * @fn      		  - GPIO_ToggleOutputPin
 *
 * @brief             - Toggles bit of a GPIO pin
 *
 * @param[in]         - Base address of GPIO peripheral
 * @param[in]         - Pin number of GPIO peripheral
 * @param[in]         -
 *
 * @return            - none
 *
 * @Note              - none

 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber) {
	pGPIOx->ODR ^= ( 1 << PinNumber );
}


/*********************************************************************
 * @fn      		  - GPIO_IRQConfig
 *
 * @brief             - Configures IRQs in MCU NVIC registers
 *
 * @param[in]         - Interrupt IRQ number
 * @param[in]         - Interrupt priority
 * @param[in]         -	ENABLE or DISABLE macros
 *
 * @return            - none
 *
 * @Note              - none

 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi) {

	if(EnorDi == ENABLE) {
		if(IRQNumber <= 31) {
			//Program ISER0 register
			*NVIC_ISER0 |= ( 1 << IRQNumber);

		}else if(IRQNumber >= 32 && IRQNumber < 64) {
			//Program ISER1 register
			*NVIC_ISER1 |= ( 1 << ( IRQNumber % 32 ));

		}else if(IRQNumber >= 64 && IRQNumber < 96) {
			//Program ISER2 register
			*NVIC_ISER2 |= ( 1 << ( IRQNumber % 64 ));
		}

	}else {
		if(IRQNumber <= 31) {
			//Program ICER0 register
			*NVIC_ICER0 |= ( 1 << IRQNumber);

		}else if(IRQNumber >= 32 && IRQNumber <= 64) {
			//Program ICER1 register
			*NVIC_ICER1 |= ( 1 << ( IRQNumber % 32 ));

		}else if(IRQNumber >= 64 && IRQNumber <= 96) {
			//Program ICER2 register
			*NVIC_ICER2 |= ( 1 << ( IRQNumber % 64 ));
		}
	}
}

void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority) {
	//find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED));
	//*(NVIC_PR_BASEADDR + (iprx * 4)) |= ( IRQPriority << shift_amount); error: pointer addition
	*(NVIC_PR_BASEADDR + iprx) |= ( IRQPriority << shift_amount);
}

/*********************************************************************
 * @fn      		  -
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
void GPIO_IRQHandling(uint8_t PinNumber) {
	//clear the exti pr register corresponding to the pin number
	if(EXTI->PR & ( 1 << PinNumber)) {
		//clear
		EXTI->PR |= ( 1 << PinNumber);
	}
}


