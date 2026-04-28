#include "app_key_task.h"
#include "stdio.h"
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
static Button key2;

/*user_add_param*/
KEY_MENU_STATUS key_menu={1,1,1,1};

uint8_t read_button_gpio(uint8_t button_id)
{
    switch (button_id) {
        case 0:
            return key_scan().back;
		case 1:
            return key_scan().enter;
		case 2:
            return key_scan().encoder_sw;
        default:
            return 1;
    }
}

/*user_add_funtion*/
void user_keyBSP_init(void)
{
	button_init(&key0, read_button_gpio, 0, 0);
	button_init(&key1, read_button_gpio, 0, 1);

	button_attach(&key0, BTN_PRESS_UP, 			(BtnCallback)key0_press_up_Handler);
	button_attach(&key0, BTN_LONG_PRESS_START, 	(BtnCallback)key0_long_press_start_Handler);
	button_attach(&key0, BTN_SINGLE_CLICK, 		(BtnCallback)key0_single_click_Handler);
	button_attach(&key0, BTN_PRESS_REPEAT, 		(BtnCallback)key0_press_repeat_Handler);

	button_attach(&key1, BTN_PRESS_UP, 			(BtnCallback)key1_press_up_Handler);
	button_attach(&key1, BTN_LONG_PRESS_START, 	(BtnCallback)key1_long_press_start_Handler);
	button_attach(&key1, BTN_SINGLE_CLICK, 		(BtnCallback)key1_single_click_Handler);
	button_attach(&key1, BTN_PRESS_REPEAT, 		(BtnCallback)key1_press_repeat_Handler);

	button_init(&key2, read_button_gpio, 0, 2);
	button_attach(&key2, BTN_PRESS_UP, 			(BtnCallback)key2_press_up_Handler);
	button_attach(&key2, BTN_SINGLE_CLICK, 		(BtnCallback)key2_single_click_Handler);
	button_attach(&key2, BTN_LONG_PRESS_START, 	(BtnCallback)key2_long_press_start_Handler);

	button_start(&key0);
	button_start(&key1);
	button_start(&key2);
}


void key0_press_up_Handler(void *btn)
{
//	printf("***> key0 press up! <***\r\n");
    key_menu.back = RELEASE;
    key_menu.up = RELEASE;
}

void key0_press_repeat_Handler(void *btn)
{
	key_menu.back = RELEASE;
    key_menu.up = PRESS;
    plane_game_set_up();
    Beeper_Perform(BEEPER_KEYPRESS);
//	printf("***> key0 press repeat! <***\r\n");
}

void key0_single_click_Handler(void *btn)
{
	key_menu.back = RELEASE;
    key_menu.up = PRESS;
    plane_game_set_up();
    Beeper_Perform(BEEPER_KEYPRESS);
//	printf("***> key0 single click! <***\r\n");
}

void key0_long_press_start_Handler(void *btn)
{
    key_menu.back = PRESS;
    key_menu.up = RELEASE;
    Beeper_Perform(BEEPER_WARNING);
    
    // 请求退出水管鸟游戏
    game_request_exit();
    //请求退出小恐龙游戏
    dino_game_request_exit();
    //请求退出飞机大战游戏
    plane_game_request_exit();
    // 请求退出陀螺仪显示
    gyroscope_request_exit();
    // 请求退出串口监视器
    uart_monitor_request_exit();
//    printf("***> key0 long press <***\r\n");
}







void key1_press_up_Handler(void *btn)
{
    key_menu.enter = RELEASE;
    key_menu.down = RELEASE;  
//	printf("***> key1 press up! <***\r\n");
}

void key1_press_repeat_Handler(void *btn)
{
//	printf("***> key1 press repeat! <***\r\n");
	key_menu.enter = RELEASE;
    key_menu.down = PRESS;  
    game_set_jump();//水管鸟游戏单击事件
    dino_game_set_click();//小恐龙游戏单击事件
    plane_game_set_down();
    Beeper_Perform(BEEPER_KEYPRESS);
}

void key1_single_click_Handler(void *btn)
{
//	printf("***> key1 single click! <***\r\n");
	key_menu.enter = RELEASE;
    key_menu.down = PRESS;  
    game_set_jump();//水管鸟游戏单击事件
    dino_game_set_click();//小恐龙游戏单击事件
    plane_game_set_down();
    Beeper_Perform(BEEPER_KEYPRESS);
}

void key1_long_press_start_Handler(void *btn)
{
    key_menu.enter = PRESS;
    key_menu.down = RELEASE;
    plane_game_set_click();//飞机大战游戏单击事件（实际是长按开始游戏）
    uart_monitor_request_clear(); // 串口监视器长按清空
    Beeper_Perform(BEEPER_TRITONE);
    
//	printf("***> key1 long press <***\r\n");
}


/* 编码器SW按键回调 */
void key2_press_up_Handler(void *btn)
{
    key_menu.enter = RELEASE;
    key_menu.back = RELEASE;
}

void key2_single_click_Handler(void *btn)
{
    key_menu.enter = PRESS;
    key_menu.back = RELEASE;
    game_set_jump();
    dino_game_set_click();
    plane_game_set_click();
    Beeper_Perform(BEEPER_TRITONE);
}

void key2_long_press_start_Handler(void *btn)
{
    key_menu.enter = RELEASE;
    key_menu.back = PRESS;
    Beeper_Perform(BEEPER_WARNING);
    game_request_exit();
    dino_game_request_exit();
    plane_game_request_exit();
    gyroscope_request_exit();
    uart_monitor_request_exit();
}
