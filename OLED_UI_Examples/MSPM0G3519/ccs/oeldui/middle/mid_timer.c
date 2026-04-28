#include "mid_timer.h"
#include "mid_button.h"
#include "mid_debug_led.h"
#include "OLED_UI.h"
#include "OLED_UI_MenuData.h"
#include "mid_music.h"
#include "hw_ws2812_effects.h"

// 定义枚举类型
typedef enum {
    TASK_DISABLE=0, 
    TASK_ENABLE   
} Task_state;

static volatile Task_state tack_enable_flag = TASK_ENABLE;

// 全局毫秒计数器，5ms中断累加
static volatile uint32_t sys_tick_ms = 0;

uint32_t get_sys_tick_ms(void)
{
    return sys_tick_ms;
}

//使能任务调度
void enable_task_interrupt(void)
{
	tack_enable_flag = TASK_ENABLE;
}
//失能任务调度
void disable_task_interrupt(void)
{
	tack_enable_flag = TASK_DISABLE;
}
//获取任务状态
Task_state get_task_status(void)
{
	return tack_enable_flag;
}

void timer_init(void)
{
    //定时器中断
	NVIC_ClearPendingIRQ(TIMER_TICK_INST_INT_IRQN);
	NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);
}

//5ms定时器中断服务函数
void TIMER_TICK_INST_IRQHandler(void)
{
	static char oled_time_num = 0, buzzer_time_num = 0;
	//5ms归零中断触发
	if( DL_TimerA_getPendingInterrupt(TIMER_TICK_INST) == DL_TIMER_IIDX_ZERO )
	{
		// 全局计时 +5ms
		sys_tick_ms += 5;

		//按键扫描
		button_ticks();

		// RGB流水灯计时
		ws2812_effect_tick();

		// 屏幕刷新 20x5ms=1000ms
		if( (oled_time_num++) >= 20 )
		{
			oled_time_num = 0;
			OLED_UI_InterruptHandler();
			set_debug_led_toggle();
		}

		// 蜂鸣器音调控制 2x5ms=10ms
		if( (buzzer_time_num++) >= 1 )
		{
			buzzer_time_num = 0;
			Beeper_Proc();
		}
	}
}