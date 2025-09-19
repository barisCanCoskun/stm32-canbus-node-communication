/*
 * it.c
 *
 *  Created on: Aug 20, 2025
 *      Author: baris
 */

#include "main.h"
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htimer6;
extern CAN_HandleTypeDef hcan1;

void SysTick_Handler(void)
{
	 HAL_IncTick();
	 HAL_SYSTICK_IRQHandler();
}

void USART2_IRQHandler(void)
{
//	Below func handles UART interrupt request, identify the reason
	HAL_UART_IRQHandler(&huart2);
}

void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_RX0_IRQHandler()
{
	HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_RX1_IRQHandler()
{
	HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_SCE_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

/**
  * @brief This function handles Timer 6 interrupt and DAC underrun interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htimer6);
}

/**
  * @brief This function handles EXTI line 0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
	HAL_TIM_Base_Start_IT(&htimer6);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}







