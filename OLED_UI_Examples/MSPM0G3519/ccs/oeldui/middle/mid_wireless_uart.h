#ifndef _MID_WIRELESS_UART_H_
#define _MID_WIRELESS_UART_H_

#include "ti_msp_dl_config.h"
#include <stdbool.h>

#define WIRELESS_RX_BUF_MAX  512

// 2.4G无线连接状态引脚（PB23）
// SysConfig 生成的宏为 WIRELESS_PORT + WIRELESS_LINK_PIN
#ifndef WIRELESS_LINK_PORT
#define WIRELESS_LINK_PORT   WIRELESS_PORT
#endif

void wireless_uart_init(void);
void wireless_uart_send_char(char ch);
void wireless_uart_send_string(char *str);
char* wireless_uart_get_rx_data(void);
void wireless_uart_clear_rx_data(void);
bool wireless_uart_is_linked(void);

#endif
