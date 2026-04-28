/**
 * mid_ui_bridge.h
 * UI abstraction bridge — consolidates all app callback and hardware variable declarations
 * needed by oledUI menus, so oledUI/ has zero direct dependency on app/ or hardware/.
 */
#ifndef __MID_UI_BRIDGE_H__
#define __MID_UI_BRIDGE_H__

#include "mid_app_task.h"

/* ===== 应用回调函数声明（各 app_*.h 提供实现） ===== */
void app_gyroscope_init(void);
void app_gyroscope_tick(void);
void app_gyroscope_sample(void);
bool app_gyroscope_should_exit(void);
void app_gyroscope_on_exit(void);
void app_gyroscope_fade_tick(int8_t level);

void app_uart_monitor_init(void);
void app_uart_monitor_tick(void);
bool app_uart_monitor_should_exit(void);
void app_uart_monitor_on_exit(void);
void app_uart_monitor_fade_tick(int8_t level);

void dino_game_init(void);
void dino_game_tick(void);
bool dino_game_should_exit(void);
void dino_game_on_exit(void);

void app_bird_game_init(void);
void app_bird_game_tick(void);
bool app_bird_game_should_exit(void);
void app_bird_game_on_exit(void);

void plane_game_init(void);
void plane_game_tick(void);
bool plane_game_should_exit(void);
void plane_game_on_exit(void);

void app_robot_face_init(void);
void app_robot_face_tick(void);
bool app_robot_face_should_exit(void);
void app_robot_face_on_exit(void);

/* ===== 蜂鸣器 ===== */
#include "mid_music.h"

/* ===== WS2812 RGB LED ===== */
#include "hw_ws2812.h"

/* ===== 设置持久化 ===== */
#include "hw_w25qxx.h"

#endif
