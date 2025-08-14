/*******************************************************************************
* File Name          : timer.h
* Author             : WZH
* Version            : V0.1
* Date               : 2021/11/17
* Description        : 系统时钟配置头文件
*******************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "gd32f10x.h"

#ifndef  _TIMER_C_
	#define  TIMER_EXT	extern
#else
	#define  TIMER_EXT
#endif

//闪灯
#define GPIO_LED		GPIOB
#define LED_PIN			GPIO_PIN_0


void Delay_ms(ulong u32Count); 														//延迟ms级
void Delay_us(ulong u32Count);														//延迟us级
void Led_Init(ulong u32Delay);														//开机闪灯
void Timer2_Init(void);
void Timer3_Init(void);

#endif
