/**
 * oled_ext_font.h
 * External Flash font interface — Chinese character and mixed CN/EN string display
 * using font bitmaps stored in external SPI Flash.
 */
#ifndef _OLED_EXT_FONT_H_
#define _OLED_EXT_FONT_H_

#include "ti_msp_dl_config.h"
#include <stdint.h>

/**
 * @brief 从外部 Flash 读取 GB2312 汉字字模（支持 12x12, 16x16, 20x20）
 * @param high     GB2312 编码高字节
 * @param low      GB2312 编码低字节
 * @param buf      输出缓冲区
 * @param fontSize 字体大小: 12, 16, 20
 */
void ExtFont_ReadChinese(uint8_t high, uint8_t low, uint8_t* buf, uint8_t fontSize);

/**
 * @brief 显示外部字库中文字符串（自动识别 UTF-8 / GB2312）
 * @param fontSize 中文字体大小: 12, 16, 20
 */
void OLED_ShowChineseExt(int16_t X, int16_t Y, char *Chinese, uint8_t fontSize);

/**
 * @brief 中英文混合显示（中文走外部 Flash，ASCII 走内部字模）
 * @param chineseFontSize 中文字体大小: 12, 16, 20
 * @param asciiFontSize   ASCII 字体大小（如 OLED_8X16_HALF）
 */
void OLED_ShowMixStringExt(int16_t X, int16_t Y, char *String,
                            uint8_t chineseFontSize, uint8_t asciiFontSize);

/**
 * @brief 带区域裁剪的外部字库中文显示
 */
void OLED_ShowChineseAreaExt(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight,
                              int16_t X, int16_t Y, char *Chinese, uint8_t fontSize);

/**
 * @brief 带区域裁剪的中英文混合显示
 */
void OLED_ShowMixStringAreaExt(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight,
                                int16_t X, int16_t Y, char *String,
                                uint8_t chineseFontSize, uint8_t asciiFontSize);

#endif
