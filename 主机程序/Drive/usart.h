/*******************************************************************************
* File Name          : usart.h
* Author             : WZH
* Version            : V1.0
* Date               : 2025/02/22
* Description        : 串口驱动头文件
*******************************************************************************/

#ifndef _USART_H_
#define _USART_H_

#include "gd32f10x.h"
#include "stdbool.h"

#define OS_ENABLE		0
#define MAX_COMM_LEN	128		//单次最大通讯长度

typedef enum{
	USART_PORT_1 = 0,
	USART_PORT_2,
	USART_PORT_MAX,
}USART_Port;

typedef enum {
	STOPBIT_1 = 0,
	STOPBIT_2,
}USART_StopBit;

typedef enum {
	PARITY_NONE = 0,
	PARITY_ODD,
	PARITY_EVEN
}USART_Parity;


void usart_init(USART_Port port, ulong ulBaudrate, USART_StopBit stopBit, USART_Parity parity);

void usart_config(USART_Port port, ulong ulBaudrate, 
						USART_StopBit stopBit, USART_Parity parity);

void usart_sendData(USART_Port port, uchar* pBuf, ushort usLen);

ushort usart_receiveData(USART_Port port, uchar* pBuf, ushort usLen);

#if OS_ENABLE
uchar usart_waitForSemaphore(USART_Port port, TickType_t ticks);
#else
uchar usart_getFlag(USART_Port port);
#endif

uchar usart1_processIRQ(void);

#endif

