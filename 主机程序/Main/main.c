#include "gd32f10x.h"
#include "systick.h"
#include "usart.h"
#include "string.h"
#include "ms5148t.h"
#include "timer.h"
#include "stdio.h"

int main(void)
{
	static s32 adc_value;
	static float voltage;
	s16 s16Code;
	char txt[32] = {0};

	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	systick_config();
	usart_init(USART_PORT_1, 9600, STOPBIT_1, PARITY_NONE);

	// 初始化
	ms5148t_init();
	ms5148t_config_sensor(0, PGA_GAIN1, DR_40SPS);

	while(1)
	{
		// 等待电路稳定
		Delay_ms(100);

		//开启转换
		gpio_bit_set(GPIOB, GPIO_PIN_10);
		// 读取ADC值
		adc_value = ms5148t_read_data();
		
		//关闭转换
		gpio_bit_reset(GPIOB, GPIO_PIN_10);
		//清除DRDY
		ms5148t_read_data();

		// 转换为电压
		voltage = (adc_value * 2.048f) / (8388608.0f);
		//关闭转换
		
		
		adc_value = ((adc_value & 0x00FFFFFF) >> 8); //取16位
		s16Code = (short)adc_value;
		sprintf(txt, "电压:%.5f,码值;%d", voltage, s16Code);
		txt[31] = 0;
		usart_sendData(USART_PORT_1, (u8 *)txt, strlen(txt));
	}

}
