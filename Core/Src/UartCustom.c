#include "UartCustom.h"

#include "freertos.h"
#include "queue.h"
#include <stdlib.h>


UART_HandleTypeDef *_uart = NULL;
static uint32_t _contentMsgMax;
static TaskHandle_t taskWaitReceive;
static TaskHandle_t taskWaitSend;
static uint32_t *ptrMsgLength = NULL;

void UartStartRxTranfer(TaskHandle_t task, Msg *msg){
	taskWaitReceive = task;
	ptrMsgLength = &msg->length;
	HAL_UARTEx_ReceiveToIdle_DMA(_uart, msg->content, _contentMsgMax);
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
}

void UartStartTxTransfer(TaskHandle_t task, Msg *msg){
	taskWaitSend = task;
	HAL_UART_Transmit_DMA(_uart, msg->content, msg->length);
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
}

BaseType_t PushMsgToMsgFreeQueue(MsgQueue *queue, Msg *msg, TickType_t delay){
    BaseType_t result = xQueueSend(queue->free, &msg, delay);
    return result;
}

BaseType_t PushMsgToMsgReadyQueue(MsgQueue *queue, Msg *msg, TickType_t delay){
    BaseType_t result = xQueueSend(queue->ready, &msg, delay);
    return result;
}

BaseType_t GetMsgFromMsgFreeQueue(MsgQueue *queue, Msg **msg, TickType_t delay){
	BaseType_t result = xQueueReceive(queue->free, msg, delay);
	return result;
}

BaseType_t GetMsgFromMsgReadyQueue(MsgQueue *queue, Msg **msg, TickType_t delay){
	BaseType_t result = xQueueReceive(queue->ready, msg, delay);
	return result;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart == _uart){
	    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    vTaskNotifyGiveFromISR(taskWaitSend, &xHigherPriorityTaskWoken);
	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if(_uart == huart){
	    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    vTaskNotifyGiveFromISR(taskWaitReceive, &xHigherPriorityTaskWoken);
	    *ptrMsgLength = Size;
	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void UartCustomInit(UART_HandleTypeDef *huart, uint32_t contentMsgMax){
	_uart = huart;
	_contentMsgMax = contentMsgMax;
}

void MsgQueueInit(MsgQueue *queue, uint32_t sizeQueue, uint32_t lengthContent){
	queue->free = xQueueCreate(sizeQueue, sizeof(Msg*));
	queue->ready = xQueueCreate(sizeQueue, sizeof(Msg*));
	queue->pool = malloc(sizeof(Msg) * sizeQueue);
	Msg *msg;
	for(uint32_t i = 0; i < sizeQueue; i++){
		(queue->pool + i)->content = malloc(sizeof(uint8_t) * lengthContent);
		msg = queue->pool + i;
		xQueueSend(queue->free, &msg, portMAX_DELAY);
	}
}
