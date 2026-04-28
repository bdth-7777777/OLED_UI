/**
 * mid_font_burner.c
 * Font flash programmer — receives font binary over UART and writes to external SPI Flash.
 */
#include "mid_font_burner.h"
#include "mid_debug_uart.h"
#include "hw_delay.h"
#include "stdio.h"

/**
 * @brief 通用烧录函数：擦除指定区域，串口接收数据写入 Flash
 * @param start_addr  起始地址
 * @param total_size  总字节数
 * @param name        烧录名称（用于串口提示）
 */
static void flash_burn(uint32_t start_addr, uint32_t total_size, const char* name)
{
    char msg[64];
    uint32_t write_addr = start_addr;
    uint32_t end_addr = start_addr + total_size;
    uint32_t total_written = 0;
    uint8_t page_buf[256];
    uint16_t buf_index = 0;
    uint32_t sector;

    // 禁用 UART 中断
    NVIC_DisableIRQ(UART_DEBUG_INST_INT_IRQN);

    // 1. 打印 Flash ID
    sprintf(msg, "FLASH ID: 0x%04X\r\n", w25q128_read_id());
    debug_uart_send_string(msg);

    // 2. 擦除目标区域
    sprintf(msg, "Erasing %s area...\r\n", name);
    debug_uart_send_string(msg);
    for (sector = start_addr / 4096; sector < (end_addr + 4095) / 4096; sector++)
    {
        w25q128_erase_sector(sector);
    }
    debug_uart_send_string("Erase done.\r\n");

    // 3. 提示发送文件
    sprintf(msg, "READY: Send %s bin now.\r\n", name);
    debug_uart_send_string(msg);

    // 4. 接收数据，256 字节批量页写入
    while (write_addr < end_addr)
    {
        page_buf[buf_index++] = DL_UART_Main_receiveDataBlocking(UART_DEBUG_INST);

        if (buf_index >= 256)
        {
            w25q128_write_page(page_buf, write_addr, 256);
            write_addr += 256;
            total_written += 256;
            buf_index = 0;
        }
    }

    // 剩余不足 256 字节的尾部数据
    if (buf_index > 0)
    {
        w25q128_write_page(page_buf, write_addr, buf_index);
        total_written += buf_index;
    }

    // 5. 完成
    sprintf(msg, "Done! Total: %lu bytes\r\n", total_written);
    debug_uart_send_string(msg);

    // 恢复 UART 中断
    NVIC_EnableIRQ(UART_DEBUG_INST_INT_IRQN);

    while (1);
}

void font_burner_hzk16_run(void)
{
    flash_burn(FONT16_BASE_ADDR, FONT16_TOTAL_SIZE, "HZK16");
}

void font_burner_hzk12_run(void)
{
    flash_burn(FONT12_BASE_ADDR, FONT12_TOTAL_SIZE, "HZK12");
}

void font_burner_hzk20_run(void)
{
    flash_burn(FONT20_BASE_ADDR, FONT20_TOTAL_SIZE, "HZK20");
}

void font_burner_map_run(void)
{
    flash_burn(U2G_MAP_BASE_ADDR, U2G_MAP_SIZE, "U2G_MAP");
}
