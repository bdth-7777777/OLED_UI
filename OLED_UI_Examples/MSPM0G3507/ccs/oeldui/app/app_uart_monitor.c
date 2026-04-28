/**
 * app_uart_monitor.c
 * 串口数据监视器 - 128x64 OLED UI
 *
 * 布局：
 *   Y=0~9   : 顶部状态栏（UART标题 + 波特率 + RX指示 + 字节数）
 *   Y=10~52 : 中间加粗双线矩形框 + 数据内容（4行）
 *   Y=53~63 : 底部按键提示栏
 *
 * 按键：
 *   KEY0 单击  = 向上滚动
 *   KEY1 单击  = 向下滚动
 *   KEY1 长按  = 清空缓冲区
 *   KEY0 长按  = 退出
 */

#include "app_uart_monitor.h"
#include "app_key_task.h"
#include "OLED.h"
#include "OLED_UI.h"
#include "mid_debug_uart.h"
#include "mid_timer.h"
#include "hw_delay.h"
#include <string.h>
#include <stdio.h>

#define UART_MON_MAX_LINES   64
#define UART_MON_LINE_WIDTH  20     // 内容区宽度（3~122px，6px×20=120）
#define UART_MON_VISIBLE     4      // 可见行数
#define UART_MON_LINE_H      8

// 布局常量
#define STATUS_H        10          // 状态栏高度
#define FRAME_Y         10          // 外框顶部Y
#define FRAME_H         43          // 外框高度（Y=10~52）
#define CONTENT_X       3           // 内容起始X
#define CONTENT_Y       13          // 内容起始Y（框内留2px边距）
#define FOOTER_SEP_Y    53          // 底部分隔线Y
#define FOOTER_Y        55          // 底部文字Y

#define UART_BAUD_RATE  9600

static bool exit_requested = false;
static bool clear_requested = false;

static char line_buf[UART_MON_MAX_LINES][UART_MON_LINE_WIDTH + 1];
static int  line_count = 0;
static int  line_head  = 0;
static int  scroll_offset = 0;
static int  total_bytes = 0;
static bool auto_scroll = true;
static int  last_processed_len = 0;

static bool     rx_blink = false;
static uint32_t rx_blink_timer = 0;

void app_uart_monitor_request_exit(void)  { exit_requested = true; }
void app_uart_monitor_request_clear(void) { clear_requested = true; }
bool app_uart_monitor_should_exit(void)   { return exit_requested; }

static void clear_lines(void)
{
    line_count = 0;
    line_head  = 0;
    scroll_offset = 0;
    last_processed_len = 0;
    total_bytes = 0;
    clear_debug_uart_receive_data();
}

static void push_line(const char *s, int len)
{
    if(len <= 0) return;
    int idx = (line_head + line_count) % UART_MON_MAX_LINES;
    if(line_count < UART_MON_MAX_LINES){
        line_count++;
    } else {
        line_head = (line_head + 1) % UART_MON_MAX_LINES;
    }
    int copy_len = len < UART_MON_LINE_WIDTH ? len : UART_MON_LINE_WIDTH;
    memcpy(line_buf[idx], s, copy_len);
    line_buf[idx][copy_len] = '\0';
}

static void parse_new_data(void)
{
    char *buf = get_debug_uart_receive_data();
    int   total_len = strlen(buf);

    if(total_len < last_processed_len) last_processed_len = 0;
    if(total_len <= last_processed_len) return;

    const char *p = buf + last_processed_len;
    int new_len = total_len - last_processed_len;
    total_bytes += new_len;
    rx_blink = true;
    rx_blink_timer = mid_get_sys_tick_ms();

    const char *line_start = p;
    for(int i = 0; i < new_len; i++){
        char c = p[i];
        if(c == '\n' || c == '\r'){
            int seg_len = (int)(p + i - line_start);
            if(seg_len > 0) push_line(line_start, seg_len);
            if(c == '\r' && i + 1 < new_len && p[i+1] == '\n') i++;
            line_start = p + i + 1;
        } else {
            int seg_len = (int)(p + i - line_start) + 1;
            if(seg_len >= UART_MON_LINE_WIDTH){
                push_line(line_start, UART_MON_LINE_WIDTH);
                line_start = p + i + 1;
            }
        }
    }
    int remain = (int)(p + new_len - line_start);
    if(remain > 0) push_line(line_start, remain);

    last_processed_len = total_len;
    if(auto_scroll) scroll_offset = 0;
}

static void render_frame(void)
{
    OLED_Clear();

    // ---- 顶部状态栏（反色填充）----
    OLED_DrawRectangle(0, 0, 128, STATUS_H, OLED_FILLED);
    OLED_ReverseArea(0, 0, 128, STATUS_H);

    // 左侧：UART标签
    OLED_ShowString(1, 1, "UART", OLED_6X8_HALF);

    // 波特率
    char baud_str[10];
    snprintf(baud_str, sizeof(baud_str), "%d", UART_BAUD_RATE);
    OLED_ShowString(26, 1, baud_str, OLED_6X8_HALF);

    // RX闪烁指示（小实心圆点）
    if(rx_blink){
        OLED_DrawCircle(80, 5, 3, OLED_FILLED);
        if(mid_get_sys_tick_ms() - rx_blink_timer > 150) rx_blink = false;
    } else {
        OLED_DrawCircle(80, 5, 3, OLED_UNFILLED);
    }

    // 右侧：字节计数
    char cnt_str[10];
    if(total_bytes < 10000){
        snprintf(cnt_str, sizeof(cnt_str), "%dB", total_bytes);
    } else {
        snprintf(cnt_str, sizeof(cnt_str), "%dK", total_bytes / 1024);
    }
    int cnt_x = 127 - (int)strlen(cnt_str) * 6;
    OLED_ShowString(cnt_x, 1, cnt_str, OLED_6X8_HALF);

    // ---- 中间加粗双线矩形框 ----
    // 外框
    OLED_DrawRectangle(0, FRAME_Y, 128, FRAME_H, OLED_UNFILLED);
    // 内框（偏移1px，形成加粗效果）
    OLED_DrawRectangle(1, FRAME_Y + 1, 126, FRAME_H - 2, OLED_UNFILLED);

    // ---- 内容区数据 ----
    if(line_count == 0){
        // 等待数据提示（居中）
        OLED_ShowString(10, CONTENT_Y + 12, "Waiting data...", OLED_6X8_HALF);
    } else {
        int display_lines = line_count < UART_MON_VISIBLE ? line_count : UART_MON_VISIBLE;
        int start_from_bottom = scroll_offset + display_lines - 1;
        if(start_from_bottom >= line_count) start_from_bottom = line_count - 1;

        for(int row = 0; row < display_lines; row++){
            int line_idx_from_end = start_from_bottom - row;
            if(line_idx_from_end < 0) break;
            int abs_idx = (line_head + (line_count - 1 - line_idx_from_end)) % UART_MON_MAX_LINES;
            int y = CONTENT_Y + row * UART_MON_LINE_H;
            OLED_ShowString(CONTENT_X, y, line_buf[abs_idx], OLED_6X8_HALF);
        }
    }

    // ---- 右侧滚动条（在框内右边缘）----
    if(line_count > UART_MON_VISIBLE){
        int bar_x   = 124;
        int bar_y   = FRAME_Y + 3;
        int bar_h   = FRAME_H - 6;
        int thumb_h = bar_h * UART_MON_VISIBLE / line_count;
        if(thumb_h < 3) thumb_h = 3;
        int thumb_y = bar_y + (bar_h - thumb_h) * scroll_offset / (line_count - UART_MON_VISIBLE);
        OLED_DrawLine(bar_x, bar_y, bar_x, bar_y + bar_h - 1);
        OLED_DrawRectangle(bar_x - 1, thumb_y, 3, thumb_h, OLED_FILLED);
    }

    // ---- 底部按键提示栏 ----
    OLED_DrawLine(0, FOOTER_SEP_Y, 127, FOOTER_SEP_Y);

    // 左侧：滚动模式
    if(auto_scroll){
        OLED_ShowString(0, FOOTER_Y, "AUTO", OLED_6X8_HALF);
    } else {
        OLED_ShowString(0, FOOTER_Y, "HOLD", OLED_6X8_HALF);
    }

    // 中间：操作提示
    OLED_ShowString(30, FOOTER_Y, "H1:CLR", OLED_6X8_HALF);
    OLED_ShowString(78, FOOTER_Y, "H0:EXIT", OLED_6X8_HALF);

    OLED_Update();
}

void app_uart_monitor_init(void)
{
    exit_requested = false;
    clear_requested = false;
    clear_lines();
}

void app_uart_monitor_tick(void)
{
    parse_new_data();

    if(key_menu.up == PRESS){
        key_menu.up = RELEASE;
        if(scroll_offset < line_count - UART_MON_VISIBLE){
            scroll_offset++;
            auto_scroll = false;
        }
    }
    if(key_menu.down == PRESS){
        key_menu.down = RELEASE;
        if(scroll_offset > 0) scroll_offset--;
        if(scroll_offset == 0) auto_scroll = true;
    }

    if(clear_requested){
        clear_requested = false;
        clear_lines();
    }

    render_frame();
}

void app_uart_monitor_fade_tick(int8_t level)
{
    render_frame();
    OLED_UI_FadeOut_Masking(0, 0, 128, 64, level);
    OLED_Update();
}

void app_uart_monitor_on_exit(void)
{
    OLED_Clear();
    OLED_Update();
}


void app_uart_monitor_loop(void)
{
    exit_requested = false;
    clear_requested = false;
    clear_lines();

    // 淡入
    int8_t i;
    for(i = 5; i >= 1; i--){
        render_frame();
        OLED_UI_FadeOut_Masking(0, 0, 128, 64, i);
        OLED_Update();
        delay_ms(FADEOUT_TIME);
    }

    while(1){
        // 检测新串口数据
        parse_new_data();

        // 按键处理
        if(key_menu.up == PRESS){
            key_menu.up = RELEASE;
            // 向上滚动（看更旧的数据）
            if(scroll_offset < line_count - UART_MON_VISIBLE){
                scroll_offset++;
                auto_scroll = false;
            }
        }
        if(key_menu.down == PRESS){
            key_menu.down = RELEASE;
            // 向下滚动（看更新的数据）
            if(scroll_offset > 0){
                scroll_offset--;
            }
            if(scroll_offset == 0) auto_scroll = true;
        }

        // 清空（由 key1 长按触发，在 app_key_task 中调用 app_uart_monitor_request_clear）
        if(clear_requested){
            clear_requested = false;
            clear_lines();
        }

        render_frame();

        if(app_uart_monitor_should_exit()) break;

        delay_ms(30);
    }

    // 淡出
    for(i = 1; i <= 5; i++){
        render_frame();
        OLED_UI_FadeOut_Masking(0, 0, 128, 64, i);
        OLED_Update();
        delay_ms(FADEOUT_TIME);
    }
    OLED_Clear();
    OLED_Update();
}
