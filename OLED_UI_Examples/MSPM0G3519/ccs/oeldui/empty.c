#include "ti_msp_dl_config.h"
#include "string.h"
#include "stdio.h"

#include "mid_timer.h"
#include "mid_debug_uart.h"
#include "mid_wireless_uart.h"
#include "mid_button.h"
#include "mid_music.h"
#include "mid_font_burner.h"

#include "OLED_UI.h"
#include "OLED_UI_MenuData.h"
#include "oled_ext_font.h"

#include "hw_delay.h"
#include "hw_lsm6ds3.h"
#include "hw_ws2812.h"
#include "hw_ws2812_effects.h"
#include "hw_w25qxx.h"

#include "app_bird_game.h"
#include "app_dino_game.h"

extern bool ColorMode;
extern bool OLED_UI_ShowFps;
extern BEEPER_Tag Beeper0;
extern int16_t OLED_UI_Brightness;


int main(void)
{
	int time_num = 0;

	// 图形化外设配置初始化
	SYSCFG_DL_init();	

	// 串口初始化 9600
	debug_uart_init();

	// 2.4G无线串口初始化
	wireless_uart_init();

	Delay_ms(500);

	/*=== 字库烧录模式（每次只取消一个注释，烧录完再注释回来）===*/
	// font_burner_hzk16_run();   // 烧录 HZK16 字库 (发送 HZK16.bin)
	// font_burner_hzk12_run();   // 烧录 HZK12 字库 (发送 HZK12.bin)
	// font_burner_hzk20_run();   // 烧录 HZK20 字库 (发送 HZK20.bin)
	// font_burner_map_run();     // 烧录 Unicode→GB2312 映射表 (发送 unicode_gb2312_map.bin)

	// 蜂鸣器初始化
	Beeper_Init();

	// 按键初始化
	user_keyBSP_init();

	// 5ms定时器中断
	timer_init();

	// 蜂鸣器音乐
	Beeper_Perform(BEEP1);

	PWM_WS2812B_Init();

	/*=== 外部字库显示测试（三种字号）===*/
	// {
	// 	OLED_Init();
	// 	OLED_Clear();
	// 	OLED_ShowChineseExt(0, 0, "你好", 12);                                // 12x12
	// 	OLED_ShowMixStringExt(0, 16, "Hi世界", 16, OLED_8X16_HALF);           // 16x16
	// 	OLED_ShowChineseExt(0, 36, "测试", 20);                                // 20x20
	// 	OLED_Update();
	// 	while(1);
	// }

	// 读取系统参数（默认值在前，settings_load 若无有效记录则保持默认）
	// 亮度100、音量50、蜂鸣器开启、颜色模式白色、显示FPS、RGB开/R0/G0/B0/4灯/灯光模式关/渐变速度50/灯光亮度100
	uint8_t temp[13] = {100, 50, 0, 0, 1, 1, 0, 0, 0, 4, 0, 50, 100};
	settings_load(temp);
	OLED_UI_Brightness       = (uint16_t)temp[0];
	Beeper0.Sound_Loud       = temp[1];
	Beeper0.Beeper_Enable    = temp[2];
	ColorMode                = (bool)temp[3];
	OLED_UI_ShowFps          = (bool)temp[4];
	ws2812_enable            = (bool)temp[5];
	ws2812_r                 = (int16_t)temp[6];
	ws2812_g                 = (int16_t)temp[7];
	ws2812_b                 = (int16_t)temp[8];
	ws2812_led_num           = (int16_t)temp[9];
	ws2812_light_mode        = (int16_t)temp[10];
	effect_param.speed       = (uint16_t)temp[11];
	ws2812_brightness        = (int16_t)temp[12];

	// UI初始化
	OLED_UI_Init(&MainMenuPage);



	while (1)
	{
		OLED_UI_MainLoop();
		ws2812_effect_update();
	}
}
