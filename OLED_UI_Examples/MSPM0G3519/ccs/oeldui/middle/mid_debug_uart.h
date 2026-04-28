#ifndef _MID_DEBUG_UART_H_
#define _MID_DEBUG_UART_H_

#include "ti_msp_dl_config.h"

#define REVEIVE_BUFFER_MAX 	512

typedef struct {
	char receive_buffer[ REVEIVE_BUFFER_MAX ];
	int receive_data_length;
}DEBUG_UART_STRUCT;


void debug_uart_init(void);
void debug_uart_send_char(char ch);
void debug_uart_send_string(char* str);
char* get_debug_uart_receive_data(void);
void clear_debug_uart_receive_data(void);
#endif
