/*******************************************************************************
* File Name          : timer.c
* Author             : WZH
* Version            : V0.1
* Date               : 2021/11/17
* Description        : 系统时钟配置
*******************************************************************************/

#define _TIMER_C_

#include "timer.h"

/****************************************************************
// Summary: 	us级延迟
// Parameter: 	[in/u32]u32Count 数值
//
// return:		[void]
****************************************************************/
void Delay_us(ulong u32Count)
{
	while(u32Count --)
	{
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
	}
}

/****************************************************************
// Summary: 	ms级延迟
// Parameter: 	[in/u32]u32Count 数值
//
// return:		[void]
****************************************************************/
void Delay_ms(ulong u32Count)
{
	while(u32Count--)
	{
		Delay_us(1000);
	}
}

/****************************************************************
// Summary: 	开机闪灯
// Parameter: 	[in/u32]u32Delay 间隔时间
//
// return:		[void]
****************************************************************/
void Led_Init(ulong u32Delay)
{
	ulong i;

	rcu_periph_clock_enable(RCU_GPIOB);

	gpio_init(GPIO_LED, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LED_PIN);
	gpio_bit_set(GPIO_LED, LED_PIN);

	for(i = 0; i < 15; i++)
	{
		gpio_bit_set(GPIO_LED, LED_PIN);
		Delay_ms(u32Delay);
		gpio_bit_reset(GPIO_LED, LED_PIN);
		Delay_ms(u32Delay);
	}
	gpio_bit_set(GPIO_LED, LED_PIN);
}

/****************************************************************
// Summary: 	Timer初始化
// Parameter: 	[void]
//
// return:		[void]
****************************************************************/
void Timer2_Init(void)
{
	timer_parameter_struct initpara;

	timer_deinit(TIMER2);
	initpara.prescaler = 19999;
	initpara.alignedmode = TIMER_COUNTER_EDGE;
	initpara.counterdirection = TIMER_COUNTER_UP;
	initpara.period = 499;
	initpara.clockdivision = TIMER_CKDIV_DIV1;
	initpara.repetitioncounter = 0;
	timer_init(TIMER2, &initpara);

	timer_auto_reload_shadow_enable(TIMER2);
	timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMER2, TIMER_INT_UP);
	timer_enable(TIMER2);
}

/****************************************************************
// Summary: 	Timer初始化
// Parameter: 	[void]
//
// return:		[void]
****************************************************************/
void Timer3_Init(void)
{
	timer_parameter_struct initpara;

	timer_deinit(TIMER3);
	initpara.prescaler = 3999;
	initpara.alignedmode = TIMER_COUNTER_EDGE;
	initpara.counterdirection = TIMER_COUNTER_UP;
	initpara.period = 499;
	initpara.clockdivision = TIMER_CKDIV_DIV1;
	initpara.repetitioncounter = 0;
	timer_init(TIMER3, &initpara);

	timer_auto_reload_shadow_enable(TIMER3);
	timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMER3, TIMER_INT_UP);
	timer_disable(TIMER3);
}

