#ifndef	__MID_TIMER_H__
#define __MID_TIMER_H__

#include "ti_msp_dl_config.h"


void timer_init(void);
void enable_task_interrupt(void);
void disable_task_interrupt(void);
uint32_t get_sys_tick_ms(void);

#endif
