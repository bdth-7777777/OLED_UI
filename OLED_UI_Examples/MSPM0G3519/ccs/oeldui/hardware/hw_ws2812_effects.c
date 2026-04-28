#include "hw_ws2812_effects.h"
#include "hw_ws2812.h"
#include "hw_delay.h"

WS2812_Effect_Param effect_param = {
    .mode = EFFECT_STATIC,
    .speed = 50,
    .brightness = 255
};

/* 灯光模式：0=关, 1=流水灯, 2=跑马灯 */
int16_t ws2812_light_mode = 0;

static volatile bool need_update = false;

/* 流水灯状态 */
static uint16_t flowing_step = 0;
#define TOTAL_STEPS 1536

/* 跑马灯状态 */
static uint8_t  running_led   = 0;
static uint16_t running_step  = 0;
static uint16_t running_color = 0;
static uint32_t running_buf[WS2812B_NUM] = {0};

/* gamma 2.2 校正表 */
static const uint8_t gamma_table[256] = {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
      3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,
      6,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,  10,  11,  11,  11,  12,
     12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,
     20,  20,  21,  22,  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,
     30,  30,  31,  32,  33,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,
     42,  43,  43,  44,  45,  46,  47,  48,  49,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,
     73,  74,  75,  76,  77,  78,  79,  81,  82,  83,  84,  85,  87,  88,  89,  90,
     91,  93,  94,  95,  97,  98,  99, 100, 102, 103, 105, 106, 107, 109, 110, 111,
    113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135,
    137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159, 161,
    163, 165, 166, 168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188, 190,
    192, 194, 196, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
    223, 225, 227, 229, 231, 234, 236, 238, 240, 242, 244, 246, 248, 251, 253, 255,
};

static uint32_t apply_brightness(uint32_t grb)
{
    uint16_t br = (uint16_t)ws2812_brightness;
    if (br >= 100) return grb;
    uint8_t g = (uint8_t)(((grb >> 16) & 0xFF) * br / 100);
    uint8_t r = (uint8_t)(((grb >>  8) & 0xFF) * br / 100);
    uint8_t b = (uint8_t)(( grb        & 0xFF) * br / 100);
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

static uint32_t wheel_color(uint16_t step)
{
    uint8_t r, g, b;
    uint8_t pos = step & 0xFF;

    switch (step >> 8) {
        case 0:  g = 255;       r = pos;       b = 0;         break;
        case 1:  g = 255 - pos; r = 255;       b = 0;         break;
        case 2:  g = 0;         r = 255;       b = pos;       break;
        case 3:  g = 0;         r = 255 - pos; b = 255;       break;
        case 4:  g = pos;       r = 0;         b = 255;       break;
        default: g = 255;       r = 0;         b = 255 - pos; break;
    }

    r = gamma_table[r];
    g = gamma_table[g];
    b = gamma_table[b];

    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

static uint32_t color_lerp(uint32_t from, uint32_t to, uint8_t pos)
{
    int16_t g0 = (from >> 16) & 0xFF;
    int16_t r0 = (from >>  8) & 0xFF;
    int16_t b0 =  from        & 0xFF;
    int16_t g1 = (to   >> 16) & 0xFF;
    int16_t r1 = (to   >>  8) & 0xFF;
    int16_t b1 =  to          & 0xFF;

    uint8_t g = (uint8_t)(g0 + (g1 - g0) * pos / 255);
    uint8_t r = (uint8_t)(r0 + (r1 - r0) * pos / 255);
    uint8_t b = (uint8_t)(b0 + (b1 - b0) * pos / 255);

    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

/* ===== 流水灯 ===== */
static void do_flowing_effect(uint16_t speed)
{
    if (!need_update) return;
    need_update = false;

    uint32_t grb = apply_brightness(wheel_color(flowing_step));
    WS2812B_Write_24Bits(WS2812B_NUM, 0x000000);   /* 先清空所有灯 */
    WS2812B_Write_24Bits(ws2812_led_num, grb);
    WS2812B_Show();

    uint16_t step_size = 1 + (speed - 1) * 7 / 99;
    flowing_step += step_size;
    if (flowing_step >= TOTAL_STEPS) flowing_step -= TOTAL_STEPS;
}

/* ===== 跑马灯 ===== */
static void do_running_effect(uint16_t speed)
{
    if (!need_update) return;
    need_update = false;

    /* 安全检查：led_num减少时running_led可能越界 */
    if (running_led >= (uint8_t)ws2812_led_num) {
        running_led = 0;
    }

    uint16_t next_color = running_color + 256;
    if (next_color >= TOTAL_STEPS) next_color -= TOTAL_STEPS;

    uint32_t from = wheel_color(running_color);
    uint32_t to   = wheel_color(next_color);

    running_buf[running_led] = color_lerp(from, to, (uint8_t)(running_step > 255 ? 255 : running_step));

    uint32_t out_buf[WS2812B_NUM];
    int i;
    for (i = 0; i < ws2812_led_num && i < WS2812B_NUM; i++)
        out_buf[i] = apply_brightness(running_buf[i]);

    WS2812B_Write_24Bits(WS2812B_NUM, 0x000000);   /* 先清空所有灯 */
    WS2812B_Write_24Bits_independence(ws2812_led_num, out_buf);
    WS2812B_Show();

    uint16_t step_size = 1 + (speed - 1) * 7 / 99;
    running_step += step_size;

    if (running_step >= 256) {
        running_buf[running_led] = to;
        running_step = 0;
        running_led++;

        if (running_led >= (uint8_t)ws2812_led_num) {
            running_led = 0;
            running_color = next_color;
        }
    }
}

/* ===== tick（5ms中断调用）===== */
void ws2812_effect_tick(void)
{
    if (ws2812_light_mode == 0 || !ws2812_enable || ws2812_led_num == 0) return;
    need_update = true;
}

/* ===== 模式切换 ===== */
void ws2812_set_effect_mode(WS2812_Effect_Mode mode)
{
    effect_param.mode = mode;
    flowing_step  = 0;
    running_led   = 0;
    running_step  = 0;
    running_color = 0;
    need_update   = false;

    if (mode == EFFECT_RUNNING) {
        uint32_t init_color = wheel_color(0);
        int i;
        for (i = 0; i < WS2812B_NUM; i++) running_buf[i] = init_color;
    }
}

void ws2812_set_effect_speed(uint16_t speed)
{
    if (speed > 100) speed = 100;
    if (speed < 1)   speed = 1;
    effect_param.speed = speed;
}

/* ===== 主循环调用 ===== */
void ws2812_effect_update(void)
{
    static bool prev_off = false;

    bool off = (!ws2812_enable || ws2812_led_num == 0);

    if (off) {
        if (!prev_off) {
            need_update = false;
            WS2812B_Write_24Bits(WS2812B_NUM, 0x000000);
            WS2812B_Show();
            prev_off = true;
        }
        return;
    }

    if (prev_off) {
        prev_off = false;
        need_update = false;
    }

    switch (ws2812_light_mode) {
        case 1: do_flowing_effect(effect_param.speed); break;
        case 2: do_running_effect(effect_param.speed); break;
        default: ws2812_update(); break;
    }
}
