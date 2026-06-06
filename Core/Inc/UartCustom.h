#ifndef INC_UARTCUSTOM_H_
#define INC_UARTCUSTOM_H_

//------------------------------------------------------------------------------------//

#include "freertos.h"
#include "queue.h"
#include "stm32f1xx_hal.h"
//------------------------------------------------------------------------------------//

#define UART_HEADER_LENGTH														12

//------------------------------------------------------------------------------------//

typedef struct{
	uint8_t *content;
	uint32_t length;
} Msg;

typedef struct{
	Msg *pool;
	QueueHandle_t free;
	QueueHandle_t ready;
} MsgQueue;

//------------------------------------------------------------------------------------//

void UartCustomInit(UART_HandleTypeDef *huart, uint32_t contentMsgMax);
void MsgQueueInit(MsgQueue *queue, uint32_t sizeQueue, uint32_t lengthContent);
void UartStartRxTranfer(TaskHandle_t task, Msg *msg);
void UartStartTxTransfer(TaskHandle_t task, Msg *msg);
BaseType_t PushMsgToMsgFreeQueue(MsgQueue *queue, Msg *msg, TickType_t delay);
BaseType_t PushMsgToMsgReadyQueue(MsgQueue *queue, Msg *msg, TickType_t delay);
BaseType_t GetMsgFromMsgFreeQueue(MsgQueue *queue, Msg **msg, TickType_t delay);
BaseType_t GetMsgFromMsgReadyQueue(MsgQueue *queue, Msg **msg, TickType_t delay);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

#endif /* INC_UARTCUSTOM_H_ */
