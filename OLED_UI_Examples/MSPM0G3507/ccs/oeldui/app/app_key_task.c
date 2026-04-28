/**
 * app_key_task.c
 * Key input task — initializes buttons, binds key event handlers, and dispatches
 * key actions to active games and the menu system.
 */
#include "app_key_task.h"
#include "mid_button.h"
#include "hw_key.h"
# include "mid_music.h"
#include "app_dino_game.h"
#include "app_bird_game.h"
#include "app_plane_game.h"
#include "app_gyroscope.h"
#include "app_uart_monitor.h"

/*user_add_handle*/
static Button key0;
static Button key1;

/*user_add_param*/
KEY_MENU_STATUS key_menu={1,1,1,1};

uint8_t read_mid_button_gpio(uint8_t mid_button_id)
{
    switch (mid_button_id) {
        case 0:
            return key_scan().back;
		case 1:
            return key_scan().enter;
        default:
            return 1;
    }
}

/*user_add_funtion*/
void user_key_bsp_init(void)
{
	mid_button_init(&key0, read_mid_button_gpio, 0, 0);
	mid_button_init(&key1, read_mid_button_gpio, 0, 1);

	mid_button_attach(&key0, BTN_PRESS_UP, 			(BtnCallback)key0_press_up_handler);
	mid_button_attach(&key0, BTN_LONG_PRESS_START, 	(BtnCallback)key0_long_press_start_handler);
	mid_button_attach(&key0, BTN_SINGLE_CLICK, 		(BtnCallback)key0_single_click_handler);
	mid_button_attach(&key0, BTN_PRESS_REPEAT, 		(BtnCallback)key0_press_repeat_handler);

	mid_button_attach(&key1, BTN_PRESS_UP, 			(BtnCallback)key1_press_up_handler);
	mid_button_attach(&key1, BTN_LONG_PRESS_START, 	(BtnCallback)key1_long_press_start_handler);
	mid_button_attach(&key1, BTN_SINGLE_CLICK, 		(BtnCallback)key1_single_click_handler);
	mid_button_attach(&key1, BTN_PRESS_REPEAT, 		(BtnCallback)key1_press_repeat_handler);

	mid_button_start(&key0);
	mid_button_start(&key1);
}


void key0_press_up_handler(void *btn)
{
//	printf("***> key0 press up! <***\r\n");
    key_menu.back = RELEASE;
    key_menu.up = RELEASE;
}

void key0_press_repeat_handler(void *btn)
{
	key_menu.back = RELEASE;
    key_menu.up = PRESS;
    plane_game_set_up();
    mid_beeper_perform(song_keypress);
//	printf("***> key0 press repeat! <***\r\n");
}

void key0_single_click_handler(void *btn)
{
	key_menu.back = RELEASE;
    key_menu.up = PRESS;
    plane_game_set_up();
    mid_beeper_perform(song_keypress);
//	printf("***> key0 single click! <***\r\n");
}

void key0_long_press_start_handler(void *btn)
{
    key_menu.back = PRESS;
    key_menu.up = RELEASE;
    mid_beeper_perform(song_warning);
    
    // 请求退出水管鸟游戏
    app_bird_game_request_exit();
    //请求退出小恐龙游戏
    dino_game_request_exit();
    //请求退出飞机大战游戏
    plane_game_request_exit();
    // 请求退出陀螺仪显示
    app_gyroscope_request_exit();
    // 请求退出串口监视器
    app_uart_monitor_request_exit();
//    printf("***> key0 long press <***\r\n");
}







void key1_press_up_handler(void *btn)
{
    key_menu.enter = RELEASE;
    key_menu.down = RELEASE;  
//	printf("***> key1 press up! <***\r\n");
}

void key1_press_repeat_handler(void *btn)
{
//	printf("***> key1 press repeat! <***\r\n");
	key_menu.enter = RELEASE;
    key_menu.down = PRESS;  
    app_bird_game_set_jump();//水管鸟游戏单击事件
    dino_game_set_click();//小恐龙游戏单击事件
    plane_game_set_down();
    mid_beeper_perform(song_keypress);
}

void key1_single_click_handler(void *btn)
{
//	printf("***> key1 single click! <***\r\n");
	key_menu.enter = RELEASE;
    key_menu.down = PRESS;  
    app_bird_game_set_jump();//水管鸟游戏单击事件
    dino_game_set_click();//小恐龙游戏单击事件
    plane_game_set_down();
    mid_beeper_perform(song_keypress);
}

void key1_long_press_start_handler(void *btn)
{
    key_menu.enter = PRESS;
    key_menu.down = RELEASE;
    plane_game_set_click();//飞机大战游戏单击事件（实际是长按开始游戏）
    app_uart_monitor_request_clear(); // 串口监视器长按清空
    mid_beeper_perform(song_tritone);
    
//	printf("***> key1 long press <***\r\n");
}


/*user_add_funtion*/
