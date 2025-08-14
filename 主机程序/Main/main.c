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

	// ��ʼ��
	ms5148t_init();
	ms5148t_config_sensor(0, PGA_GAIN1, DR_40SPS);

	while(1)
	{
		// �ȴ���·�ȶ�
		Delay_ms(100);

		//����ת��
		gpio_bit_set(GPIOB, GPIO_PIN_10);
		// ��ȡADCֵ
		adc_value = ms5148t_read_data();
		
		//�ر�ת��
		gpio_bit_reset(GPIOB, GPIO_PIN_10);
		//���DRDY
		ms5148t_read_data();

		// ת��Ϊ��ѹ
		voltage = (adc_value * 2.048f) / (8388608.0f);
		//�ر�ת��
		
		
		adc_value = ((adc_value & 0x00FFFFFF) >> 8); //ȡ16λ
		s16Code = (short)adc_value;
		sprintf(txt, "��ѹ:%.5f,��ֵ;%d", voltage, s16Code);
		txt[31] = 0;
		usart_sendData(USART_PORT_1, (u8 *)txt, strlen(txt));
	}

}
