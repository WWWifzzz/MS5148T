/*******************************************************************************
* File Name          : ms5148t.h
* Author             : WZH
* Version            : V1.0
* Date               : 2025/08/11
* Description        : AD芯片驱动
*******************************************************************************/

#ifndef _MS5148T_H_
#define _MS5148T_H_

#include "gd32f10x.h"

// 系统参数配置位
#define PGA_GAIN1        (0 << 4)
#define PGA_GAIN2        (1 << 4)
#define PGA_GAIN4        (2 << 4)
#define PGA_GAIN8        (3 << 4)
#define PGA_GAIN16       (4 << 4)
#define PGA_GAIN32       (5 << 4)
#define PGA_GAIN64       (6 << 4)
#define PGA_GAIN128      (7 << 4)

#define DR_5SPS          0x00
#define DR_10SPS         0x01
#define DR_20SPS         0x02
#define DR_40SPS         0x03
#define DR_80SPS         0x04
#define DR_160SPS        0x05
#define DR_320SPS        0x06
#define DR_640SPS        0x07
#define DR_1000SPS       0x08
#define DR_2000SPS       0x09



void ms5148t_init(void);
void ms5148t_config_sensor(uchar input_chn, uchar gain, uchar data_rate);
s32 ms5148t_read_data(void);



























































#endif
















