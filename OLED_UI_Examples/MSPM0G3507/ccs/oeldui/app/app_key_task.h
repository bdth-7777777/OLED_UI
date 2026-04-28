/**
 * app_key_task.h
 * Key input task interface — KEY_MENU_STATUS struct and key handler function declarations.
 */
#ifndef __APP_KEY_TASK_H__
#define __APP_KEY_TASK_H__

#include "ti_msp_dl_config.h"

typedef struct {
    unsigned int enter : 1; // 使用位字段来节省空间
    unsigned int back  : 1;
    unsigned int up    : 1;
    unsigned int down  : 1;
} KEY_MENU_STATUS;

#define PRESS 	 0		//按下
#define RELEASE  1		//松开

extern KEY_MENU_STATUS key_menu;

void user_key_bsp_init(void);

void key0_press_down_handler(void *btn);

void key0_press_up_handler(void* btn);

void key0_press_repeat_handler(void *btn);

void key0_single_click_handler(void *btn);

void key0_long_press_start_handler(void* btn);

void key0_long_press_hold_handler(void *btn);

void key1_press_down_handler(void* btn);

void key1_press_up_handler(void* btn);

void key1_press_repeat_handler(void *btn);

void key1_single_click_handler(void *btn);

void key1_long_press_start_handler(void *btn);

void key1_long_press_hold_handler(void *btn);
#endif
