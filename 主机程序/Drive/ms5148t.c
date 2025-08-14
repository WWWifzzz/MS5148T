/*******************************************************************************
* File Name          : ms5148t.h
* Author             : WZH
* Version            : V1.0
* Date               : 2025/08/11
* Description        : AD芯片驱动
*******************************************************************************/

#include "ms5148t.h"
#include "timer.h"

// 寄存器地址定义
#define REG_MUX0         0x00
#define REG_VBIAS        0x01
#define REG_MUX1         0x02
#define REG_SYS0         0x03
#define REG_OFC0         0x04
#define REG_OFC1         0x05
#define REG_OFC2         0x06
#define REG_FSC0         0x07
#define REG_FSC1         0x08
#define REG_FSC2         0x09
#define REG_IDAC0        0x0A
#define REG_IDAC1        0x0B


#define MS5148T_PORT			GPIOB
#define MS5148T_RESET			GPIO_PIN_9		//低电平有效
#define MS5148T_START			GPIO_PIN_10		//高电平有效
#define MS5148T_DRDY			GPIO_PIN_11		//低电平有效

#define SPI_PORT				MS5148T_PORT
#define SPI_GPIO_CS				GPIO_PIN_12		//低电平有效
#define SPI_GPIO_CLK			GPIO_PIN_13
#define SPI_GPIO_MISO			GPIO_PIN_14		//DRDY低电平时可读
#define SPI_GPIO_MOSI			GPIO_PIN_15

#define SPI_CLK_0()				gpio_bit_reset(SPI_PORT, SPI_GPIO_CLK)
#define SPI_CLK_1()				gpio_bit_set(SPI_PORT, SPI_GPIO_CLK)
#define SPI_MOSI_0()			gpio_bit_reset(SPI_PORT, SPI_GPIO_MOSI)
#define SPI_MOSI_1()			gpio_bit_set(SPI_PORT, SPI_GPIO_MOSI)
#define SPI_CS_0()				gpio_bit_reset(SPI_PORT, SPI_GPIO_CS)
#define SPI_CS_1()				gpio_bit_set(SPI_PORT, SPI_GPIO_CS)
#define SPI_MISO_READ()			gpio_input_bit_get(SPI_PORT, SPI_GPIO_MISO)
#define START_CONVERSION()		gpio_bit_set(MS5148T_PORT, MS5148T_START)
#define STOP_CONVERSION()		gpio_bit_reset(MS5148T_PORT, MS5148T_START)

/****************************************************************
// Summary: 模拟SPI引脚配置
****************************************************************/
static void gpioConfig(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);

	gpio_init(MS5148T_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MS5148T_RESET);
	gpio_init(MS5148T_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MS5148T_START);
	gpio_init(MS5148T_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, MS5148T_DRDY);

	gpio_init(SPI_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_GPIO_CS);
	gpio_init(SPI_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_GPIO_CLK);
	gpio_init(SPI_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, SPI_GPIO_MISO);
	gpio_init(SPI_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_GPIO_MOSI);
}

/****************************************************************
// Summary: 模拟SPI写
****************************************************************/
static void spi_write(u8 ucData)
{
	u8 i;

	for(i = 0; i < 8; i++)
	{
		SPI_CLK_1();
		if(ucData & 0x80)
		{
			SPI_MOSI_1();
		}
		else
		{
			SPI_MOSI_0();
		}
		
		SPI_CLK_0();
		ucData <<= 1;
	}
}

/****************************************************************
// Summary: 模拟SPI读
****************************************************************/
static u8 spi_read(void)
{
	u8 i;
	u8 ucRead = 0;

	for(i = 0; i < 8; i++)
	{
		SPI_CLK_1();
		ucRead <<= 1;
		if(SPI_MISO_READ())
		{
			ucRead |= 1;
		}
		
		SPI_CLK_0();
		
	}

	return ucRead;
}


/****************************************************************
// Summary: 寄存器写入（优化时序）
****************************************************************/
void ms5148t_write_reg(u8 reg_addr, u8 data)
{
	SPI_CS_1();
	SPI_MOSI_1();
	SPI_CLK_0();
	SPI_CS_0();

	// 发送写命令格式：0100 rrrr
	spi_write(0x40 | (reg_addr & 0x0F));

	// 单寄存器写入（nnnn=0）
	spi_write(0x01);

	// 写入数据
	spi_write(data);
	spi_write(0xFF);

	SPI_CS_1();
	SPI_MOSI_1();;
	SPI_CLK_1();

}

/****************************************************************
// Summary: 寄存器读取
****************************************************************/
u8 ms5148t_read_reg(u8 reg_addr)
{
	u8 data;

	SPI_CS_1();
	SPI_MOSI_1();
	SPI_CLK_0();
	SPI_CS_0();

	// 发送读命令格式：0010 rrrr
	spi_write(0x20 | (reg_addr & 0x0F));

	// 单寄存器读取（nnnn=0）
	spi_write(0x01);

	// 读取数据
	data = spi_read();

	SPI_CS_1();
	SPI_MOSI_1();
	SPI_CLK_1();

	return data;
}

/****************************************************************
// Summary: 读取ADC数据（24位）
****************************************************************/
s32 ms5148t_read_data(void)
{
	s32 result;
	u8 byte1, byte2, byte3;

	// 等待DRDY变低
	while(gpio_input_bit_get(MS5148T_PORT, MS5148T_DRDY) == SET)
	{
		Delay_us(10);
	};

	SPI_CS_1();
	SPI_MOSI_1();
	SPI_CLK_0();
	SPI_CS_0();

	// 发送读数据指令（0x12）
	spi_write(0x12);
	byte1 = spi_read();
	byte2 = spi_read();
	byte3 = spi_read();

	SPI_CS_1();
	SPI_MOSI_1();
	SPI_CLK_1();

	// 组合24位数据
	result = (byte1 << 16) | (byte2 << 8) | byte3;

	// 符号位扩展
	if(result & 0x00800000)
	{
		result |= 0xFF000000;
	}

	return result;
}

/****************************************************************
// Summary: MS5148T复位
****************************************************************/
static void ms5148t_reset(void)
{
	SPI_CS_1();
	SPI_MOSI_1();
	SPI_CLK_0();
	STOP_CONVERSION();

	gpio_bit_reset(MS5148T_PORT, MS5148T_RESET);
	Delay_ms(10);
	START_CONVERSION();
	gpio_bit_set(MS5148T_PORT, MS5148T_RESET);

	ms5148t_write_reg(0x06, 0xFF);
	Delay_ms(16);
}

void ms5148t_config_sensor(u8 input_chn, u8 gain, u8 data_rate)
{
	static u8 mux0_config;

	mux0_config = ms5148t_read_reg(REG_IDAC0);

	// 配置MUX0寄存器
	mux0_config = (input_chn << 3) | (input_chn + 1);
	ms5148t_write_reg(REG_MUX0, mux0_config);
	mux0_config = ms5148t_read_reg(REG_MUX0);

	ms5148t_write_reg(REG_MUX1, 0x38);
	mux0_config = ms5148t_read_reg(REG_MUX1);

	ms5148t_write_reg(REG_IDAC0, 0x01); // 1mA激励电流
	mux0_config = ms5148t_read_reg(REG_IDAC0);
	ms5148t_write_reg(REG_IDAC1, 0x8C); // IEXC1输出
	mux0_config = ms5148t_read_reg(REG_IDAC1);

	// 配置SYS0寄存器
	mux0_config = gain | data_rate;
	ms5148t_write_reg(REG_SYS0, mux0_config);
	mux0_config = ms5148t_read_reg(REG_SYS0);
}

/****************************************************************
// Summary: MS5148T初始化
****************************************************************/
void ms5148t_init(void)
{
	gpioConfig();
	ms5148t_reset();
}






