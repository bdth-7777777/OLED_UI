/**
 * app_uart_monitor.h
 * UART serial monitor interface — init, tick, fade, exit, and scroll/clear request callbacks.
 */
#ifndef __APP_UART_MONITOR_H__
#define __APP_UART_MONITOR_H__

#include "stdbool.h"
#include "stdint.h"

void app_uart_monitor_loop(void);
void app_uart_monitor_request_exit(void);
void app_uart_monitor_request_clear(void);
bool app_uart_monitor_should_exit(void);

void app_uart_monitor_init(void);
void app_uart_monitor_tick(void);
void app_uart_monitor_fade_tick(int8_t level);
void app_uart_monitor_on_exit(void);

#endif
