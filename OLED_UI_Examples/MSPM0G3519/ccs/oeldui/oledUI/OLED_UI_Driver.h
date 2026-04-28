#ifndef __OLED_UI_DRIVER_H
#define __OLED_UI_DRIVER_H
/*【如果您需要移植此项目，则需要更改以下函数的实现方式。】 */
#include "ti_msp_dl_config.h"           // Device header
#include "hw_key.h"
#include "app_key_task.h"

//获取确认，取消，上，下按键状态的函数(【Q：为什么使用宏定义而不是函数？A：因为这样可以提高效率，减少代码量】)
//按下0 松开1
#define Key_GetEnterStatus()    key_menu.enter
#define Key_GetBackStatus()     key_menu.back
#define Key_GetUpStatus()       key_menu.up
#define Key_GetDownStatus()     key_menu.down

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
