#ifndef __HW_WS2812_H__
#define __HW_WS2812_H__

#include "ti_msp_dl_config.h"
#include "hw_ws2812_effects.h"

#define WS2812B_ARR 105 // TIM2的自动重装值//使得PWM输出频率在800kHz
#define T0H 35          // 0编码高电平时间占1/3
#define T1H 70          // 1编码高电平时间占2/3

/*使用灯珠的个数*/
#define WS2812B_NUM 4
#define DATA_SIZE 24 // WS2812B传输一个数据的大小是3个字节（24bit）

/* RGB控制参数（供菜单UI绑定） */
extern bool     ws2812_enable;   // RGB灯总开关
extern int16_t  ws2812_r;        // 红色分量 0-255
extern int16_t  ws2812_g;        // 绿色分量 0-255
extern int16_t  ws2812_b;        // 蓝色分量 0-255
extern int16_t  ws2812_led_num;  // 亮灯个数 1-4
extern int16_t  ws2812_brightness; // 灯光亮度 0-100
extern WS2812_Effect_Param effect_param;  // 效果参数

void PWM_WS2812B_Init(void);
void WS2812B_Write_24Bits(uint16_t num, uint32_t GRB_Data);
void WS2812B_Show(void);
void PWM_WS2812B_Red(uint16_t num);
void PWM_WS2812B_Green(uint16_t num);
void PWM_WS2812B_Blue(uint16_t num);

void WS2812B_Write_24Bits_independence(uint16_t num, uint32_t *GRB_Data); // 独立写像素的颜色
void set_ws2812_breathing(uint8_t index);                                 // 呼吸灯
void ws2812_update(void);                                                 // 根据当前参数刷新LED

#endif
