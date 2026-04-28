/* OLED_UI_Driver 端口实现：按键状态获取 */
#include "app_key_task.h"

uint8_t oled_key_get_enter(void) { return key_menu.enter; }
uint8_t oled_key_get_back(void)  { return key_menu.back;  }
uint8_t oled_key_get_up(void)    { return key_menu.up;    }
uint8_t oled_key_get_down(void)  { return key_menu.down;  }
