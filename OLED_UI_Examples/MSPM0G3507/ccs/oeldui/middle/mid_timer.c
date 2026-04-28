/**
 * mid_timer.c
 * System tick timer — 5ms periodic interrupt, callback registration for 5/10/20ms tasks,
 * and global millisecond counter.
 */
#include "mid_timer.h"

// 定义枚举类型
typedef enum {
    TASK_DISABLE=0,
    TASK_ENABLE
} Task_state;

static volatile Task_state tack_enable_flag = TASK_ENABLE;

// 全局毫秒计数器，5ms中断累加
static volatile uint32_t sys_tick_ms = 0;

// 回调函数指针
static mid_timer_cb_t cb_5ms  = NULL;
static mid_timer_cb_t cb_10ms = NULL;
static mid_timer_cb_t cb_20ms = NULL;

uint32_t mid_get_sys_tick_ms(void)
{
    return sys_tick_ms;
}

void enable_task_interrupt(void)
{
    tack_enable_flag = TASK_ENABLE;
}

void disable_task_interrupt(void)
{
    tack_enable_flag = TASK_DISABLE;
}

void mid_timer_register_cb_5ms(mid_timer_cb_t cb)
{
    cb_5ms = cb;
}

void mid_timer_register_cb_10ms(mid_timer_cb_t cb)
{
    cb_10ms = cb;
}

void mid_timer_register_cb_20ms(mid_timer_cb_t cb)
{
    cb_20ms = cb;
}

void mid_timer_init(void)
{
    NVIC_ClearPendingIRQ(TIMER_TICK_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);
}

// 5ms定时器中断服务函数
void TIMER_TICK_INST_IRQHandler(void)
{
    static char cb_20ms_cnt = 0, cb_10ms_cnt = 0;

    if (DL_TimerA_getPendingInterrupt(TIMER_TICK_INST) == DL_TIMER_IIDX_ZERO)
    {
        sys_tick_ms += 5;

        // 5ms 回调
        if (cb_5ms) cb_5ms();

        // 10ms 回调 (每2次触发)
        if (++cb_10ms_cnt >= 2) {
            cb_10ms_cnt = 0;
            if (cb_10ms) cb_10ms();
        }

        // 20ms 回调 (每4次触发)
        if (++cb_20ms_cnt >= 4) {
            cb_20ms_cnt = 0;
            if (cb_20ms) cb_20ms();
        }
    }
}
