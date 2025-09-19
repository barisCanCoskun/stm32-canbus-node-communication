/*
 * main.c
 *
 * STM32 CAN Communication (Node2 - STM32F407G-DISC1)
 *
 * Role of Node2: CAN Slave
 *   - Receives LED commands from Node1 (Data Frame, ID: 0x65D)
 *   - Responds to Remote Frames (ID: 0x651) with 2-byte reply (0xAB, 0xCD)
 *   - Blinks onboard LEDs (PD12–PD15) depending on received command
 *   - Sends debug messages over UART2 (via ST-LINK VCP)
 *
 * Created on: Aug 20, 2025
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
uint8_t led_no = 0;

/* --- Function prototypes --- */
void SystemClock_Config(void);
void Error_Handler(void);
void GPIO_Init(void);
void UART2_Init(void);
void TIMER6_Init(void);
void CAN1_Init(void);
void CAN1_Tx(void);
void CAN_Filter_Config(void);

void LED_Manage_Output(uint8_t led_number);
void Send_Response(uint32_t StdId);


/**
  * @brief  The application entry point.
  * @retval int
  */
int main()
{
	HAL_Init();
	SystemClock_Config();
	GPIO_Init();
	UART2_Init();
	TIMER6_Init();
	CAN1_Init();
	CAN_Filter_Config();

	/* Enable CAN interrupts */
	if(HAL_CAN_ActivateNotification(&hcan1,
			CAN_IT_TX_MAILBOX_EMPTY |
			CAN_IT_RX_FIFO0_MSG_PENDING |
			CAN_IT_BUSOFF) != HAL_OK)
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Configure GPIO pins for LEDs, Pushbutton
  * @retval None
  */
void GPIO_Init(void)
{
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef ledgpio;
  ledgpio.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
  ledgpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD,&ledgpio);

  GPIO_InitTypeDef pushButtongpio;
  pushButtongpio.Pin = GPIO_PIN_0;
  pushButtongpio.Mode = GPIO_MODE_IT_FALLING;
  pushButtongpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &pushButtongpio);

  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
  * @brief UART2 Init (115200-8-N-1)
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
  * @brief CAN1 Init (Normal mode, ~500 kbps)
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
  * @brief CAN Filter Init (accept all IDs)
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
  * @brief CAN TX: send 1-byte LED command (demo/debug)
  * for node2 this is not being called
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
		led_no = 0;
	}

	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);

	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief Manage LED outputs (D12–D15)
  * led_numbers: 1 : D12(Green), 2 : D13(Orange), 3 : D14(Red), 4 : D15(Blue)
  * @retval None
  */
void LED_Manage_Output(uint8_t led_number)
{
	switch(led_number)
	{
	case 1:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		break;
	case 2:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		break;
	case 3:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		break;
	case 4:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
		break;
	default:
		Error_Handler();
		break;
	}
}

/**
  * @brief Respond to Remote Frame (0x651) with Data Frame (0xAB,0xCD)
  * @retval None
  */
void Send_Response(uint32_t StdId)
{
	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;

	uint8_t response[2] = {0xAB, 0xCD};

	TxHeader.DLC = 2;
	TxHeader.StdId = StdId;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, response, &TxMailbox) != HAL_OK)
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
	uint8_t rcvd_message[8];
	char msg[50];

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_message) != HAL_OK)
	{
		Error_Handler();
	}

	if(RxHeader.StdId == 0x65D && RxHeader.RTR == CAN_RTR_DATA)
	{
//		This is DATA frame sent by node1 to node2
		LED_Manage_Output(rcvd_message[0]);
		sprintf(msg, "Message Received: #%X\r\n", rcvd_message[0]);
	}
	else if(RxHeader.StdId == 0x651 && RxHeader.RTR == CAN_RTR_REMOTE)
	{
//		This is REMOTE frame sent by node1 to node2
		Send_Response(RxHeader.StdId);
		return;
	}
	else if(RxHeader.StdId == 0x651 && RxHeader.RTR == CAN_RTR_DATA)
	{
//		This is a REPLY (data frame) by node2 to node1
		sprintf(msg, "Reply Received: #%X\r\n", rcvd_message[0] << 8 | rcvd_message[1]);
	}

	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

}

/**
  * @brief 1-second tick → transmit CAN frame
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	CAN1_Tx();
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















