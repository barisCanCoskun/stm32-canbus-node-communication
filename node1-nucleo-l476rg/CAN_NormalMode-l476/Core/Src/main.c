/*
 * main.c
 *
 * STM32 CAN Communication (Node1 - NUCLEO-L476RG)
 *
 * Role of Node1:
 *   - Send LED command (Data Frame, ID=0x65D, 1 byte payload) every 1 second
 *   - Send Remote Frame (ID=0x651) every 4 seconds requesting 2 bytes of data
 *   - Blink onboard LED on each transmission
 *   - Print debug info via UART2
 *
 * Created on: Aug 22, 2025
 * Author: Barış Can Coşkun
 */

#include "main.h"
#include <stdio.h>
#include <string.h>

/* --- Peripheral handles --- */
UART_HandleTypeDef huart2;
TIM_HandleTypeDef  htimer6;
CAN_HandleTypeDef  hcan1;

/* --- Global vars --- */
uint8_t req_counter = 0;  // counts 1s ticks, sends remote frame every 4s
uint8_t led_no = 0;       // rotates LED number 1-4

/* --- Function prototypes --- */
void SystemClock_Config(void);
void GPIO_Init(void);
void UART2_Init(void);
void TIMER6_Init(void);
void CAN1_Init(void);
void CAN_Filter_Config(void);
void Error_Handler(void);
void CAN1_Tx(void);
void CAN1_Request(void);


/**
  * @brief  The application entry point.
  * @retval int
  */
int main()
{
	HAL_Init();              // Reset peripherals, init HAL library
	SystemClock_Config();    // Configure system clock (HSE + PLL)
	GPIO_Init();             // Init LED + push button
	UART2_Init();            // UART for debug prints
	TIMER6_Init();           // 1 Hz periodic timer
	CAN1_Init();             // Init CAN peripheral
	CAN_Filter_Config();     // Accept all messages

	/* Enable CAN interrupts (TX complete, RX pending, Bus-Off detection) */
	if(HAL_CAN_ActivateNotification(&hcan1,
			CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_BUSOFF) != HAL_OK)
	{
		Error_Handler();
	}

	/* Start CAN peripheral */
	if(HAL_CAN_Start(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}

	/* Main loop (all work handled in ISRs & callbacks) */
	while(1);

	return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 21;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Configure GPIO for:
  * - PA5: LED output (blinks on CAN Tx)
  * - PC13: User button (EXTI falling edge)
  * @retval None
  */
void GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef ledgpio;
  ledgpio.Pin = GPIO_PIN_5;
  ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
  ledgpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA,&ledgpio);

  GPIO_InitTypeDef pushButton;
  pushButton.Pin = GPIO_PIN_13;
  pushButton.Mode = GPIO_MODE_IT_FALLING;
  pushButton.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC,&pushButton);

  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief Configure UART2 (115200 baud, 8N1)
  * Used for debug messages printed to serial terminal
  * @retval None
  */
void UART2_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	if(HAL_UART_Init(&huart2) != HAL_OK){
//		There is a problem
		Error_Handler();
	}
}

/**
  * @brief Configure TIM6 to generate update event every 1 second
  * @retval None
  */
void TIMER6_Init(void)
{
  htimer6.Instance = TIM6;
  htimer6.Init.Prescaler = 4199;
  htimer6.Init.Period = 10000-1;
  if( HAL_TIM_Base_Init(&htimer6) != HAL_OK )
  {
    Error_Handler();
  }
}

/**
  * @brief Initialize CAN1 peripheral
  * - Normal mode
  * - Bitrate: 500 kbps (Prescaler=6, BS1=11, BS2=2)
  * @retval None
  */
void CAN1_Init(void)
{
	hcan1.Instance = CAN1;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.AutoBusOff = ENABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;

	//	settings related to CAN bit timings
	hcan1.Init.Prescaler = 6;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
	if(HAL_CAN_Init(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief Configure CAN filters
  * - Accept all IDs (mask = 0x0000)
  * - Route to FIFO0
  * @retval None
  */
void CAN_Filter_Config(void)
{
	CAN_FilterTypeDef can1_filter_init;
	can1_filter_init.FilterActivation = CAN_FILTER_ENABLE;
	can1_filter_init.FilterBank = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0x0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	if(HAL_CAN_ConfigFilter(&hcan1, &can1_filter_init) != HAL_OK)
	{
		Error_Handler();
	}
}

/* ---------------- CAN FUNCTIONS ---------------- */

/**
  * @brief Transmit LED command
  *
  * CAN ID: 0x65D
  * DLC: 1
  * Payload: LED number (0–3)
  *
  * Node2 will blink corresponding LED upon reception
  * @retval None
  */
void CAN1_Tx(void)
{
	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;

	uint8_t message;

	TxHeader.DLC = 1;
	TxHeader.StdId = 0x65D;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	message = ++led_no;

	if(led_no == 4)
	{
		led_no = 0;	 // wrap around
	}

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);  // blink onboard LED for debug

	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
	{
		Error_Handler();
	}

}

/**
  * @brief Send Remote Frame to request 2 bytes from Node2
  *
  * CAN ID: 0x651
  * DLC: 2 (requesting 2 bytes)
  * RTR: Remote
  * @retval None
  */
void CAN1_Request(void)
{
	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;

//	no meaning for remote frame
	uint8_t message = 0;

//  node1 demanding 2 bybtes of reply
	TxHeader.DLC = 2;
	TxHeader.StdId = 0x651;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_REMOTE;

	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
	{
		Error_Handler();
	}


}

/* ---------------- CALLBACKS ---------------- */

/**
  * @brief UART debug print on TX complete via Mailbox0
  */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	char msg[50];
	sprintf(msg, "Message Transmitted from Mailbox0\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
	// NOTE: Currently using blocking UART transmit for simplicity.
	// In production, HAL_UART_Transmit_IT() (non-blocking) would be preferred.
}

/**
  * @brief UART debug print on TX complete via Mailbox1
  */
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	char msg[50];
	sprintf(msg, "Message Transmitted from Mailbox1\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
  * @brief UART debug print on TX complete via Mailbox2
  */
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	char msg[50];
	sprintf(msg, "Message Transmitted from Mailbox2\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
  * @brief Callback when a CAN frame is received in FIFO0
  *
  * - If Data Frame with ID 0x65D → extract LED command and update LEDs.
  * - If Remote Frame with ID 0x651 → send a 2-byte response back.
  * - If Data Frame with ID 0x651 → treat as reply (for debugging).
  * - Debug messages are printed via UART2.
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t rcvd_message[5];
	char msg[50];

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_message) != HAL_OK)
	{
		Error_Handler();
	}

	if(RxHeader.StdId == 0x651 && RxHeader.RTR == CAN_RTR_DATA)
	{
		sprintf(msg, "Reply Received: 0X%X\r\n", rcvd_message[0] << 8 | rcvd_message[1]);
	}
	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

}

/**
  * @brief TIM6 periodic interrupt callback
  *
  * Every tick (1 second):
  *   - Send LED command
  * Every 4th tick:
  *   - Send Remote Frame request
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	CAN1_Tx();

	if(++req_counter == 4)
	{
		CAN1_Request();
		req_counter = 0;
	}
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	char msg[50];
	sprintf(msg, "CAN Error Detected\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
  * @brief Error Handler
  *
  * Currently: blocks execution in an infinite loop.
  *
  * Future work:
  *   - Print detailed error messages over UART
  *   - Blink error codes on an LED
  *   - Reset the MCU automatically
  */
void Error_Handler(void) {
    while (1);
}













