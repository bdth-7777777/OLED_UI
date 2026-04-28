#ifndef APP_UART_MONITOR_H
#define APP_UART_MONITOR_H

#include "stdbool.h"
#include "stdint.h"

void uart_monitor_loop(void);
void uart_monitor_request_exit(void);
void uart_monitor_request_clear(void);
bool uart_monitor_should_exit(void);

void uart_monitor_init(void);
void uart_monitor_tick(void);
void uart_monitor_fade_tick(int8_t level);
void uart_monitor_on_exit(void);

#endif
