/*
 * msp.c
 *
 * STM32 Low-level hardware initialization
 * - NVIC setup
 * - Peripheral clocks
 * - GPIO pin configuration for UART, CAN, and Timer
 *
 * Created on: Aug 20, 2025
 * Author: Barış Can Coşkun
 */

#include "main.h"

/**
  * @brief processor specific initialization
  */
void HAL_MspInit(void)
{
//  Here low level processor specific inits
//	1. Set up the priority grouping of the arm cortex mx processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

//	2. Enable the required system exceptions of the arm cortex mx processor
	SCB->SHCSR |= 0x7 << 16; // usg fault, mem fault, bus fault exceptions

//	3. Configure the priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);

}

/**
  * @brief peripheral specific initialization: UART2
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
//	here do the low level inits of USART2 peripheral
	GPIO_InitTypeDef gpio_uart;
//	1. enable the clock for the USART2 peripheral as well as for GPIOA peripheral
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
//	2. do the pin muxing configurations
	gpio_uart.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	gpio_uart.Mode = GPIO_MODE_AF_PP;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart.Alternate = GPIO_AF7_USART2;	// UART2_Tx, UART2_Rx
	HAL_GPIO_Init(GPIOA, &gpio_uart);
//	3. enable the IRQ and set up the priority (NVIC settings)
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
}

/**
  * @brief peripheral specific initialization: CAN1
  */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
	GPIO_InitTypeDef gpio_can;

	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

//	CAN1 GPIO Configuration
//	PB8	---> CAN1_RX
//	PB9	---> CAN1_TX

	gpio_can.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	gpio_can.Mode = GPIO_MODE_AF_PP;
	gpio_can.Pull = GPIO_NOPULL;
	gpio_can.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_can.Alternate = GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOB, &gpio_can);

	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 15, 0);

	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);

}

/**
  * @brief peripheral specific initialization: Timer6
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htimer)
{
  //1. enable the clock for the TIM6 peripheral
  __HAL_RCC_TIM6_CLK_ENABLE();

  //2. Enable the IRQ of TIM6
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

  //3. setup the priority for TIM6_DAC_IRQn
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn,15,0);
}









