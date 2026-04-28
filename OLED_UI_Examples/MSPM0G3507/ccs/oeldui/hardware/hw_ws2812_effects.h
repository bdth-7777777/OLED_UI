/**
 * hw_ws2812_effects.h
 * WS2812 LED effects interface — effect mode enum, param struct, effect control functions.
 */
#ifndef __HW_WS2812_EFFECTS_H__
#define __HW_WS2812_EFFECTS_H__

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    EFFECT_STATIC = 0,
    EFFECT_FLOWING = 1,
    EFFECT_RUNNING = 2,
} WS2812_Effect_Mode;

typedef struct {
    WS2812_Effect_Mode mode;
    uint16_t speed;
    uint8_t brightness;
} WS2812_Effect_Param;

extern WS2812_Effect_Param effect_param;
extern int16_t ws2812_light_mode;  /* 0=关, 1=流水灯, 2=跑马灯 */

void ws2812_set_effect_mode(WS2812_Effect_Mode mode);
void ws2812_set_effect_speed(uint16_t speed);
void ws2812_effect_update(void);
void ws2812_effect_tick(void);

#endif
