/*******************************************************************************
* File Name          : USART.c
* Author             : WZH
* Version            : V1.0
* Date               : 2025/02/22
* Description        : 串口驱动 发送DMA方式  接收 IDLE中断+DMA
*******************************************************************************/

#include "usart.h"
#include "timer.h"
#include "string.h"

//#define SET_USART1_SEND_MODE			gpio_bit_set(GPIOA, GPIO_PIN_8)
//#define SET_USART1_RECV_MODE			gpio_bit_reset(GPIOA, GPIO_PIN_8)

__align(8) static uchar l_ucUsartBuffer[USART_PORT_MAX][MAX_COMM_LEN];
static ushort l_usUsartRecvLen[USART_PORT_MAX] = {0, 0};

#if OS_ENABLE
static SemaphoreHandle_t l_pSemaphore[USART_PORT_MAX] = {NULL, NULL};
#else
static uchar l_ucRecvFlag[USART_PORT_MAX] = {false, false};
#endif

/****************************************************************
// Summary: 串口1引脚配置
****************************************************************/
static void usart1GpioConfig(void)
{
	/* enable GPIO clock */
	rcu_periph_clock_enable(RCU_GPIOA);

	/* connect port to USART0 Tx */
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	/* connect port to USART0 Rx */
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
}

/****************************************************************
// Summary: 引脚配置
****************************************************************/
static void gpioConfig(USART_Port port)
{
	if(port == USART_PORT_1)
	{
		usart1GpioConfig();
	}
	else if(port == USART_PORT_2)
	{
	}
}


/****************************************************************
// Summary: 串口1 串口参数配置
****************************************************************/
static void usart1Config(ulong ulBaudrate, USART_StopBit stopBit, USART_Parity parity)
{
	/* enable USART clock */
	rcu_periph_clock_enable(RCU_USART0);

	usart_deinit(USART0);
	usart_baudrate_set(USART0, ulBaudrate);

	if(stopBit == STOPBIT_1)
	{
		usart_stop_bit_set(USART0, USART_STB_1BIT);
	}
	else 
	{
		usart_stop_bit_set(USART0, USART_STB_2BIT);
	}

	if(parity == PARITY_NONE)
	{
		usart_parity_config(USART0, USART_PM_NONE);
		usart_word_length_set(USART0, USART_WL_8BIT);
	}
	else if(parity == PARITY_ODD)
	{
		usart_parity_config(USART0, USART_PM_ODD);
		usart_word_length_set(USART0, USART_WL_9BIT);
	}
	else
	{
		usart_parity_config(USART0, USART_PM_EVEN);
		usart_word_length_set(USART0, USART_WL_9BIT);
	}

	usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

	usart_flag_clear(USART0, USART_FLAG_TC);
	usart_flag_clear(USART0, USART_FLAG_IDLEF);
	usart_enable(USART0);
}

/****************************************************************
// Summary: 串口配置
****************************************************************/
static void usartConfig(USART_Port port, ulong ulBaudrate, USART_StopBit stopBit, USART_Parity parity)
{
	if(port == USART_PORT_1)
	{
		usart1Config(ulBaudrate, stopBit, parity);
	}
	else if(port == USART_PORT_2)
	{
	}
}

/****************************************************************
// Summary: 串口1中断配置
****************************************************************/
static void usart1InterruptConfig(void)
{
	nvic_irq_enable(USART0_IRQn, 1, 0);
	usart_interrupt_enable(USART0, USART_INT_IDLE);
	usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
}

/****************************************************************
// Summary: 中断配置
****************************************************************/
static void interruptConfig(USART_Port port)
{
	if(port == USART_PORT_1)
	{
		usart1InterruptConfig();
	}
	else if(port == USART_PORT_2)
	{
		
	}
}

#define USART0_DATA_ADDRESS    (&USART_DATA(USART0))
/****************************************************************
// Summary: 串口1 DMA配置
****************************************************************/
static void usart1DmaConfig(void)
{
	dma_parameter_struct dma_init_struct;

	/* enable DMA clock */
	rcu_periph_clock_enable(RCU_DMA0);

	/* initialize DMA parameters */
	dma_struct_para_init(&dma_init_struct);
	/* initialize DMA channel3(USART0 tx) */
	dma_deinit(DMA0, DMA_CH3);
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_addr = (uint32_t)l_ucUsartBuffer[USART_PORT_1];
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
	dma_init_struct.number = 0;
	dma_init_struct.periph_addr = (uint32_t)USART0_DATA_ADDRESS;
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
	dma_init(DMA0, DMA_CH3, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA0, DMA_CH3);
	dma_memory_to_memory_disable(DMA0, DMA_CH3);

	dma_deinit(DMA0, DMA_CH4);
	dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
	dma_init_struct.number = MAX_COMM_LEN;
	dma_init_struct.memory_addr = (uint32_t)l_ucUsartBuffer[USART_PORT_1];
	dma_init(DMA0, DMA_CH4, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA0, DMA_CH4);
	dma_memory_to_memory_disable(DMA0, DMA_CH4);
	/* enable USART0 DMA channel transmission and reception */
	dma_channel_enable(DMA0, DMA_CH3);
	dma_channel_enable(DMA0, DMA_CH4);

	/* USART DMA enable for transmission and reception */
	usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
	usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_DISABLE);

	dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);
	dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF);
}

/****************************************************************
// Summary: DMA配置
****************************************************************/
static void dmaConfig(USART_Port port)
{
	if(port == USART_PORT_1)
	{
		usart1DmaConfig();
	}
	else if(port == USART_PORT_2)
	{
		
	}
}

#define WAIT_DMA_TC_MS		200
static void usart1DmaSendData(uchar* pBuf, ushort usLen)
{
	ushort usCount;

//	SET_USART1_SEND_MODE;

	dma_channel_disable(DMA0, DMA_CH3);
	memcpy(l_ucUsartBuffer[USART_PORT_1], pBuf, usLen);
	dma_transfer_number_config(DMA0, DMA_CH3, usLen);							//设置发送字节数
	dma_channel_enable(DMA0, DMA_CH3);
	usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);

	usCount = 0;
	while(dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF) == RESET)
	{
		usCount ++;
		Delay_ms(1);
		if(usCount >= WAIT_DMA_TC_MS)
			break;
	}
	dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);									//发送完成标志
	dma_channel_disable(DMA0, DMA_CH3);												//关闭DMA
	usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_DISABLE);
}

/****************************************************************
// Summary: DMA模式发送
****************************************************************/
static void sendDataByDma(USART_Port port, uchar* pBuf, ushort usLen)
{
	if(port == USART_PORT_1)
	{
		usart1DmaSendData(pBuf, usLen);
	}
	else if(port == USART_PORT_2)
	{
	}
}

/****************************************************************
// Summary: DMA模式接收
****************************************************************/
static void startRecv(USART_Port port)
{
	if(port == USART_PORT_1)
	{
//		SET_USART1_RECV_MODE;
		dma_transfer_number_config(DMA0, DMA_CH3, MAX_COMM_LEN);
		dma_channel_enable(DMA0, DMA_CH4);
	}
	else if(port == USART_PORT_2)
	{
	}
}


/****************************************************************
// Summary: 串口1中断处理函数
****************************************************************/
uchar usart1_processIRQ(void)
{
#if OS_ENABLE
	BaseType_t xHigherPriorityTaskWoken;
	BaseType_t xReturn;
#endif

	if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
	{
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
		usart_data_receive(USART0);
		dma_channel_disable(DMA0, DMA_CH4);

		l_usUsartRecvLen[USART_PORT_1] = MAX_COMM_LEN - dma_transfer_number_get(DMA0, DMA_CH4); 					//实际获取的数据

		dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF | DMA_FLAG_HTF | DMA_FLAG_ERR);
		dma_transfer_number_config(DMA0, DMA_CH4, MAX_COMM_LEN);
		dma_channel_enable(DMA0, DMA_CH4);

		// 发送信号量通知任务
#if OS_ENABLE
		xReturn = xSemaphoreGiveFromISR(l_pSemaphore[USART_PORT_1], &xHigherPriorityTaskWoken);
		if(xReturn == pdPASS && xHigherPriorityTaskWoken == pdTRUE)
		{
			return true;
		}
#else
		l_ucRecvFlag[USART_PORT_1] = true;
#endif
	}

	return false;
}

//////////////////////////////////对外接口//////////////////////////////////////
/****************************************************************
// Summary: 串口初始化
****************************************************************/
void usart_init(USART_Port port, ulong ulBaudrate, 
	USART_StopBit stopBit, USART_Parity parity)
{
	gpioConfig(port);
	usartConfig(port, ulBaudrate, stopBit, parity);
	dmaConfig(port);
	interruptConfig(port);

#if OS_ENABLE
	if(l_pSemaphore[port] == NULL)
	{
		l_pSemaphore[port] = xSemaphoreCreateBinary();
	}
#else
	l_ucRecvFlag[port] = false;
#endif

//	if(port == USART_PORT_1)
//		SetInterruptHandler(INT_USART1, usart1_processIRQ);
//	else if(port == USART_PORT_2)

	startRecv(port);
}

/****************************************************************
// Summary: 设置串口参数
****************************************************************/
void usart_config(USART_Port port, ulong ulBaudrate, 
						USART_StopBit stopBit, USART_Parity parity)
{
	usartConfig(port, ulBaudrate, stopBit, parity);
}

/****************************************************************
// Summary: 接收数据
****************************************************************/
ushort usart_receiveData(USART_Port port, uchar* pBuf, ushort usLen)
{
	ushort usRecvLen = 0;

	if(l_usUsartRecvLen[port] > usLen)
	{
		usRecvLen = usLen;
	}
	else
	{
		usRecvLen = l_usUsartRecvLen[port];
	}

	if(pBuf)
	{
		memcpy(pBuf, l_ucUsartBuffer[port], usRecvLen);
	}

	l_usUsartRecvLen[port] = 0;
	return usRecvLen;
}

/****************************************************************
// Summary: 发送数据
****************************************************************/
void usart_sendData(USART_Port port, uchar* pBuf, ushort usLen)
{
//	if(port == USART_PORT_1)
//		SET_USART1_SEND_MODE;
//	else if(port == USART_PORT_2)

	sendDataByDma(port, pBuf, usLen);

#if OS_ENABLE
	vTaskDelay(3);  //等待485芯片发送完成需要3ms
#else
	Delay_ms(3);
#endif

	startRecv(port);
}

#if OS_ENABLE
uchar usart_waitForSemaphore(USART_Port port, TickType_t ticks)
{
	BaseType_t xReturn = pdPASS;

	if(port >= USART_PORT_MAX)
		return false;

	xReturn = xSemaphoreTake(l_pSemaphore[port], ticks);

	return xReturn;
}

#else
uchar usart_getFlag(USART_Port port)
{
	uchar ucFlag;

	ucFlag = l_ucRecvFlag[port];

	if(ucFlag)
		l_ucRecvFlag[port] = false;

	return ucFlag; 
}
#endif







