/*
 * it.c
 *
 * STM32 Interrupt Service Routines (ISR)
 * Handles system tick, UART, CAN, Timer, and EXTI interrupts.
 *
 * Created on: Aug 22, 2025
 * Author: Barış Can Coşkun
 */

#include "main.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htimer6;;
extern CAN_HandleTypeDef hcan1;

/**
  * @brief Handles System tick interrupt for HAL timekeeping
  */
void SysTick_Handler(void)
{
	 HAL_IncTick();
	 HAL_SYSTICK_IRQHandler();
}

/**
  * @brief Handles UART2 interrupts (Rx/Tx complete, errors)
  */
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2);
}

/**
  * @brief Handles CAN1 Transmit interrupt
  */
void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

/**
  * @brief Handles CAN1 Receive FIFO0 interrupt
  */
void CAN1_RX0_IRQHandler()
{
	HAL_CAN_IRQHandler(&hcan1);
}

/**
  * @brief Handles CAN1 Receive FIFO1 interrupt
  */
void CAN1_RX1_IRQHandler()
{
	HAL_CAN_IRQHandler(&hcan1);
}

/**
  * @brief Handles CAN1 Status Change/Error interrupt
  */
void CAN1_SCE_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

/**
  * @brief Handles Timer 6 interrupt and DAC underrun interrupts (used as 1 Hz time base).
  */
void TIM6_DAC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htimer6);
}

/**
  * @brief Handles EXTI line[15:10] interrupts.
  * In this project: user button (PC13).
  */
void EXTI15_10_IRQHandler(void)
{
	HAL_TIM_Base_Start_IT(&htimer6);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}








