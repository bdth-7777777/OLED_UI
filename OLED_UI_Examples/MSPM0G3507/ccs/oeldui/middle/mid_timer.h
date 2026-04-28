/**
 * mid_timer.h
 * System tick timer interface — init, callback registration for 5/10/20ms, and millisecond counter.
 */
#ifndef __MID_TIMER_H__
#define __MID_TIMER_H__

#include "ti_msp_dl_config.h"

typedef void (*mid_timer_cb_t)(void);

void mid_timer_init(void);
void mid_timer_register_cb_5ms(mid_timer_cb_t cb);
void mid_timer_register_cb_10ms(mid_timer_cb_t cb);
void mid_timer_register_cb_20ms(mid_timer_cb_t cb);
void enable_task_interrupt(void);
void disable_task_interrupt(void);
uint32_t mid_get_sys_tick_ms(void);

#endif
