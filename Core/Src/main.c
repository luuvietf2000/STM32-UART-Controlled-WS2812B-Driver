/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "WB2812.h"
#include "UartCustom.h"
#include "message.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
	UART_CODE_NOT_USED														= 0x11,
	UART_CODE_HEADER														= 0x89,
	UART_CODE_GET_PIXEL_LED													= 0x100,
	UART_CODE_SET_COLOR														= 0x999,
	UART_CODE_SET_COLOR_OK													= 0x1001
} opCodeUartEnum;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WB2812_CODE_0_DUTY														4
#define WB2812_CODE_1_DUTY														8
#define WB2812_WIDTH															2
#define WB2812_HEIGTH															1
#define WB2812_TIME_DUTY_CODE_0													4
#define WB2812_TIME_DUTY_CODE_1													7
#define WB2812_RED_BALANCE														1
#define WB2812_GREEN_BALANCE													1
#define WB2812_BLUE_BALANCE														1
#define WB2812_GAMMA															2.4
#define MSG_LENGTH_MAX															128
#define QUEUE_SIZE_MAX															3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* Definitions for Wb2812TranferTa */
osThreadId_t Wb2812TranferTaHandle;
const osThreadAttr_t Wb2812TranferTa_attributes = {
  .name = "Wb2812TranferTa",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for UartReceiveTask */
osThreadId_t UartReceiveTaskHandle;
const osThreadAttr_t UartReceiveTask_attributes = {
  .name = "UartReceiveTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UartSendTask */
osThreadId_t UartSendTaskHandle;
const osThreadAttr_t UartSendTask_attributes = {
  .name = "UartSendTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for UartHandleTask */
osThreadId_t UartHandleTaskHandle;
const osThreadAttr_t UartHandleTask_attributes = {
  .name = "UartHandleTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* USER CODE BEGIN PV */
MsgQueue uartMsgQueueReceive;
MsgQueue uartMsgQueueSend;
WB2812Image_t image;
TaskHandle_t wb2812Task;
TaskHandle_t wb2812requestTask;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void Wb2812TranferCallback(void *argument);
void UartReceiveCallback(void *argument);
void UartSendCallback(void *argument);
void UartHandleCallback(void *argument);

/* USER CODE BEGIN PFP */
static void WB2812Config(void);
void HandleMsgUart(uint8_t *content);
void CopyWb2812ImageByte(uint8_t *content, uint32_t size, Msg *msg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	UartCustomInit(&huart2, MSG_LENGTH_MAX);
	WB2812Config();
	WB2812ImageInit(&image);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	MsgQueueInit(&uartMsgQueueReceive, QUEUE_SIZE_MAX, MSG_LENGTH_MAX);
	MsgQueueInit(&uartMsgQueueSend, QUEUE_SIZE_MAX, MSG_LENGTH_MAX);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Wb2812TranferTa */
  Wb2812TranferTaHandle = osThreadNew(Wb2812TranferCallback, NULL, &Wb2812TranferTa_attributes);

  /* creation of UartReceiveTask */
  UartReceiveTaskHandle = osThreadNew(UartReceiveCallback, NULL, &UartReceiveTask_attributes);

  /* creation of UartSendTask */
  UartSendTaskHandle = osThreadNew(UartSendCallback, NULL, &UartSendTask_attributes);

  /* creation of UartHandleTask */
  UartHandleTaskHandle = osThreadNew(UartHandleCallback, NULL, &UartHandleTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL10;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 12;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void WB2812Config(void) {
	WB2812_SetSize(WB2812_WIDTH, WB2812_HEIGTH);
	WB2812_SetLutGamma(WB2812_GREEN_BALANCE, WB2812_RED_BALANCE,
	WB2812_BLUE_BALANCE, WB2812_GAMMA);
	WB2812_SetTimAndChannel(&htim2, TIM_CHANNEL_1);
	WB2812_SetDutyCode(WB2812_TIME_DUTY_CODE_0, WB2812_TIME_DUTY_CODE_1);
}

void CopyWb2812ImageByte(uint8_t *content, uint32_t size, Msg *msg){
	memcpy(image.imageByte, content, size);
	xTaskNotifyGive(wb2812Task);
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	AddUlongToMessage(msg->content + HEADER_MESSAGE, UART_CODE_SET_COLOR_OK, UART_CODE_NOT_USED);
	msg->length += (ULONG_MESSAGE_TYPE + CONTENT_FIELD_MESSAGE_START);
}



void HandleMsgUart(uint8_t *content){
	Msg *msg;
	HeaderFieldMessage info;
	GetMsgFromMsgFreeQueue(&uartMsgQueueSend, &msg, portMAX_DELAY);
	msg->length = 0;
	GetFieldInMessage(content, &info);
	switch(info.code){
		case UART_CODE_GET_PIXEL_LED:
			AddUlongToMessage(msg->content + HEADER_MESSAGE, UART_CODE_GET_PIXEL_LED, WB2812_WIDTH * WB2812_HEIGTH);
			msg->length += (ULONG_MESSAGE_TYPE + CONTENT_FIELD_MESSAGE_START);
			break;
		case UART_CODE_SET_COLOR:
			CopyWb2812ImageByte(content + CONTENT_FIELD_MESSAGE_START, WB2812_WIDTH * WB2812_HEIGTH * WB2812_COLOR_BYTE, msg);
			break;
	}
	msg->length += HEADER_MESSAGE;
	AddUlongToMessage(msg->content, UART_CODE_HEADER, msg->length);
	PushMsgToMsgReadyQueue(&uartMsgQueueSend, msg, portMAX_DELAY);
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_Wb2812TranferCallback */
/**
 * @brief  Function implementing the Wb2812TranferTa thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_Wb2812TranferCallback */
void Wb2812TranferCallback(void *argument)
{
  /* USER CODE BEGIN 5 */
	/* Infinite loop */
	WB2812Duty_t duty;
	WB2812DutyInit(&duty);
	wb2812Task = xTaskGetCurrentTaskHandle();
	for (;;) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		WB2812_ConvertDutyFormImage(&image, &duty);
		WB2812_Show(&duty);
		xTaskNotifyGive(wb2812requestTask);
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_UartReceiveCallback */
/**
 * @brief Function implementing the UartReceiveTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_UartReceiveCallback */
void UartReceiveCallback(void *argument)
{
  /* USER CODE BEGIN UartReceiveCallback */
	Msg *msg;
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	/* Infinite loop */
	for (;;) {
		GetMsgFromMsgFreeQueue(&uartMsgQueueReceive, &msg, portMAX_DELAY);
		UartStartRxTranfer(task, msg);
		PushMsgToMsgReadyQueue(&uartMsgQueueReceive, msg, portMAX_DELAY);
	}
  /* USER CODE END UartReceiveCallback */
}

/* USER CODE BEGIN Header_UartSendCallback */
/**
 * @brief Function implementing the UartSendTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_UartSendCallback */
void UartSendCallback(void *argument)
{
  /* USER CODE BEGIN UartSendCallback */
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	Msg *msg;
	/* Infinite loop */
	for (;;) {
		GetMsgFromMsgReadyQueue(&uartMsgQueueSend, &msg, portMAX_DELAY);
		UartStartTxTransfer(task, msg);
		PushMsgToMsgFreeQueue(&uartMsgQueueSend, msg, portMAX_DELAY);
	}
  /* USER CODE END UartSendCallback */
}

/* USER CODE BEGIN Header_UartHandleCallback */
/**
 * @brief Function implementing the UartHandleTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_UartHandleCallback */
void UartHandleCallback(void *argument)
{
  /* USER CODE BEGIN UartHandleCallback */
	Msg *msg;
	wb2812requestTask = xTaskGetCurrentTaskHandle();
	HeaderFieldMessage info;
	/* Infinite loop */
	for (;;) {
		GetMsgFromMsgReadyQueue(&uartMsgQueueReceive, &msg, portMAX_DELAY);
		GetFieldInMessage(msg->content, &info);
		switch(info.code){
			case UART_CODE_HEADER:
				HandleMsgUart(msg->content + HEADER_MESSAGE);
				break;
			default:
				break;
		}
		PushMsgToMsgFreeQueue(&uartMsgQueueReceive, msg, portMAX_DELAY);
	}
  /* USER CODE END UartHandleCallback */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
