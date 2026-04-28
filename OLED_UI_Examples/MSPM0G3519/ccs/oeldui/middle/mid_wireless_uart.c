#include "mid_wireless_uart.h"
#include <string.h>

static struct {
    char buf[WIRELESS_RX_BUF_MAX];
    int  len;
} wireless_rx;

// PB23: 低电平=未连接，高电平=已连接，收发数据不影响电平

void wireless_uart_init(void)
{
    NVIC_ClearPendingIRQ(UART_WIRELESS_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_WIRELESS_INST_INT_IRQN);
    wireless_uart_clear_rx_data();
}

void wireless_uart_send_char(char ch)
{
    while (DL_UART_isBusy(UART_WIRELESS_INST));
    DL_UART_Main_transmitData(UART_WIRELESS_INST, ch);
}

void wireless_uart_send_string(char *str)
{
    while (*str) wireless_uart_send_char(*str++);
}

char* wireless_uart_get_rx_data(void)
{
    return wireless_rx.buf;
}

void wireless_uart_clear_rx_data(void)
{
    wireless_rx.len = 0;
    memset(wireless_rx.buf, 0, WIRELESS_RX_BUF_MAX);
}

bool wireless_uart_is_linked(void)
{
    return DL_GPIO_readPins(WIRELESS_LINK_PORT, WIRELESS_LINK_PIN) != 0;
}

void UART_WIRELESS_INST_IRQHandler(void)
{
    if (DL_UART_getPendingInterrupt(UART_WIRELESS_INST) == DL_UART_IIDX_RX) {
        wireless_rx.buf[wireless_rx.len++] = DL_UART_Main_receiveData(UART_WIRELESS_INST);
        if (wireless_rx.len >= WIRELESS_RX_BUF_MAX - 1)
            wireless_rx.len = 0;
    }
}
