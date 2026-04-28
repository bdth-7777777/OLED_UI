#include "oled_ext_font.h"
#include "OLED.h"
#include "hw_w25qxx.h"

/**
 * @brief 通过外部 Flash 映射表将 Unicode 码位转换为 GB2312 编码
 */
static int8_t Unicode_to_GB2312(uint16_t unicode, uint8_t* gb_high, uint8_t* gb_low)
{
    if (unicode < U2G_MAP_UNICODE_START || unicode > U2G_MAP_UNICODE_END)
        return -1;

    uint32_t offset = (uint32_t)(unicode - U2G_MAP_UNICODE_START) * 2;
    uint8_t gb[2];
    W25Q128_read(gb, U2G_MAP_BASE_ADDR + offset, 2);

    if (gb[0] == 0x00 && gb[1] == 0x00)
        return -1;

    *gb_high = gb[0];
    *gb_low  = gb[1];
    return 0;
}

/**
 * @brief 根据字体大小获取 Flash 基地址和每字字节数
 */
static void ExtFont_GetParams(uint8_t fontSize, uint32_t* baseAddr, uint8_t* charSize)
{
    switch (fontSize)
    {
        case 12:
            *baseAddr = FONT12_BASE_ADDR;
            *charSize = FONT12_CHAR_SIZE;
            break;
        case 20:
            *baseAddr = FONT20_BASE_ADDR;
            *charSize = FONT20_CHAR_SIZE;
            break;
        default:  // 16
            *baseAddr = FONT16_BASE_ADDR;
            *charSize = FONT16_CHAR_SIZE;
            break;
    }
}

/**
 * @brief 从外部 Flash 读取 GB2312 汉字字模
 * @note  字库按区位码顺序排列：
 *        地址 = baseAddr + ((高字节 - 0xA1) * 94 + (低字节 - 0xA1)) * charSize
 */
void ExtFont_ReadChinese(uint8_t high, uint8_t low, uint8_t* buf, uint8_t fontSize)
{
    uint32_t baseAddr;
    uint8_t charSize;
    ExtFont_GetParams(fontSize, &baseAddr, &charSize);

    uint32_t offset = ((uint32_t)(high - 0xA1) * 94 + (low - 0xA1)) * charSize;
    W25Q128_read(buf, baseAddr + offset, charSize);
}

/**
 * @brief 从字符串中提取下一个汉字的 GB2312 编码（自动识别 UTF-8 / GB2312）
 * @return 0=汉字, 1=ASCII, -1=结束或错误
 */
static int8_t ExtFont_NextChinese(char** str, uint8_t* gb_high, uint8_t* gb_low)
{
    uint8_t b0 = (uint8_t)**str;
    if (b0 == '\0') return -1;

    // UTF-8 三字节: 1110xxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF0) == 0xE0)
    {
        uint8_t b1 = (uint8_t)(*str)[1];
        uint8_t b2 = (uint8_t)(*str)[2];
        if (b1 == '\0' || b2 == '\0') return -1;

        uint16_t unicode = ((uint16_t)(b0 & 0x0F) << 12)
                         | ((uint16_t)(b1 & 0x3F) << 6)
                         | (uint16_t)(b2 & 0x3F);
        *str += 3;

        if (Unicode_to_GB2312(unicode, gb_high, gb_low) == 0)
            return 0;
        return -1;
    }

    // GB2312 双字节: 高字节 >= 0xA1
    if (b0 >= 0xA1)
    {
        uint8_t b1 = (uint8_t)(*str)[1];
        if (b1 == '\0') return -1;

        *gb_high = b0;
        *gb_low  = b1;
        *str += 2;
        return 0;
    }

    // ASCII
    return 1;
}

/**
 * @brief 显示外部字库中文字符串
 */
void OLED_ShowChineseExt(int16_t X, int16_t Y, char *Chinese, uint8_t fontSize)
{
    uint8_t fontBuf[60];  // 最大 20x20 = 60 字节
    uint8_t gb_high, gb_low;

    while (*Chinese != '\0')
    {
        int8_t ret = ExtFont_NextChinese(&Chinese, &gb_high, &gb_low);
        if (ret == 0)
        {
            ExtFont_ReadChinese(gb_high, gb_low, fontBuf, fontSize);
            OLED_ShowImage(X, Y, fontSize, fontSize, fontBuf);
            X += fontSize;
        }
        else if (ret == 1)
        {
            Chinese++;
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief 中英文混合显示
 */
void OLED_ShowMixStringExt(int16_t X, int16_t Y, char *String,
                            uint8_t chineseFontSize, uint8_t asciiFontSize)
{
    uint8_t fontBuf[60];
    uint8_t gb_high, gb_low;

    while (*String != '\0')
    {
        int8_t ret = ExtFont_NextChinese(&String, &gb_high, &gb_low);
        if (ret == 0)
        {
            ExtFont_ReadChinese(gb_high, gb_low, fontBuf, chineseFontSize);
            OLED_ShowImage(X, Y, chineseFontSize, chineseFontSize, fontBuf);
            X += chineseFontSize;
        }
        else if (ret == 1)
        {
            OLED_ShowChar(X, Y, *String, asciiFontSize);
            X += asciiFontSize;
            String++;
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief 带区域裁剪的外部字库中文显示
 */
void OLED_ShowChineseAreaExt(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight,
                              int16_t X, int16_t Y, char *Chinese, uint8_t fontSize)
{
    uint8_t fontBuf[60];
    uint8_t gb_high, gb_low;

    while (*Chinese != '\0')
    {
        int8_t ret = ExtFont_NextChinese(&Chinese, &gb_high, &gb_low);
        if (ret == 0)
        {
            ExtFont_ReadChinese(gb_high, gb_low, fontBuf, fontSize);
            OLED_ShowImageArea(X, Y, fontSize, fontSize,
                               RangeX, RangeY, RangeWidth, RangeHeight, fontBuf);
            X += fontSize;
        }
        else if (ret == 1)
        {
            Chinese++;
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief 带区域裁剪的中英文混合显示
 */
void OLED_ShowMixStringAreaExt(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight,
                                int16_t X, int16_t Y, char *String,
                                uint8_t chineseFontSize, uint8_t asciiFontSize)
{
    uint8_t fontBuf[60];
    uint8_t gb_high, gb_low;

    while (*String != '\0')
    {
        int8_t ret = ExtFont_NextChinese(&String, &gb_high, &gb_low);
        if (ret == 0)
        {
            ExtFont_ReadChinese(gb_high, gb_low, fontBuf, chineseFontSize);
            OLED_ShowImageArea(X, Y, chineseFontSize, chineseFontSize,
                               RangeX, RangeY, RangeWidth, RangeHeight, fontBuf);
            X += chineseFontSize;
        }
        else if (ret == 1)
        {
            OLED_ShowCharArea(RangeX, RangeY, RangeWidth, RangeHeight,
                              X, Y, *String, asciiFontSize);
            X += asciiFontSize;
            String++;
        }
        else
        {
            break;
        }
    }
}
