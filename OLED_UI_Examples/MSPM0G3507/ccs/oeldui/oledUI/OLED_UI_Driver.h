/**
 * OLED_UI_Driver.h
 * Platform abstraction layer interface — timer, key, encoder, and delay function
 * declarations. Port these implementations to your target MCU.
 */
#ifndef __OLED_UI_DRIVER_H
#define __OLED_UI_DRIVER_H
#include "ti_msp_dl_config.h"           // Device header

// 按键状态获取函数 — 实现在 app/oled_driver_port.c
// 按下返回 0，松开返回 1
extern uint8_t oled_key_get_enter(void);
extern uint8_t oled_key_get_back(void);
extern uint8_t oled_key_get_up(void);
extern uint8_t oled_key_get_down(void);

#define Key_GetEnterStatus()    oled_key_get_enter()
#define Key_GetBackStatus()     oled_key_get_back()
#define Key_GetUpStatus()       oled_key_get_up()
#define Key_GetDownStatus()     oled_key_get_down()

//定时器中断初始化函数
void Timer_Init(void);

//按键初始化函数
void Key_Init(void);

//编码器初始化函数
void Encoder_Init(void);

// 编码器使能函数
void Encoder_Enable(void);

//编码器失能函数
void Encoder_Disable(void);

//读取编码器的增量值
int16_t Encoder_Get(void);

//延时函数
void Delay_us(uint32_t xus);
void Delay_ms(uint32_t xms);
void Delay_s(uint32_t xs);

#endif
