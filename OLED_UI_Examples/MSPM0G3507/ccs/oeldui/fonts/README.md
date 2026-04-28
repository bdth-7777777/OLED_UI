# 外部 Flash 字库烧录与使用指南

## 概述

本方案将 HZK12/HZK16/HZK20 点阵字库烧录到外部 W25Q128 SPI Flash，
显示时从 Flash 读取字模数据，从而节省 MCU 内部 Flash 空间。

同时支持 Unicode→GB2312 映射表，使得源文件保持 UTF-8 编码也能直接写中文字符串。


1 新增外部 Flash 字库方案，支持 HZK12/HZK16/HZK20 三种字号
2 新增 Unicode→GB2312 映射表，UTF-8 源文件可直接写中文字符串
3 新增串口烧录模块 (mid_font_burner)，支持分别烧录各字库和映射表
4 新增外部字库读取模块 (oled_ext_font)，自动识别 UTF-8/GB2312 编码
5 修改 OLED.c 框架层，12/16/20 中文显示改为读取外部 Flash 字库
6 注释掉 OLED_Fonts.c 中 12/16/20 中文字模数据，仅保留 8x8 和 ASCII 字体
7 新增 fonts/ 目录，包含字库文件、映射表生成脚本和烧录使用文档

## Flash 地址分配

| 区域 | 地址范围 | 大小 | 用途 |
|------|---------|------|------|
| HZK16 | 0x000000 ~ 0x03FFFF | 256KB | 16x16 字库 |
| 映射表 | 0x040000 ~ 0x04A3FF | ~42KB | Unicode→GB2312 映射表 |
| HZK12 | 0x050000 ~ 0x07FFFF | 192KB | 12x12 字库 |
| HZK20 | 0x080000 ~ 0x0FFFFF | 512KB | 20x20 字库 |
| 预留区 | 0x100000 ~ 0xFFEFFF | ~15MB | 未来扩展 |
| 参数区 | 0xFFF000 ~ 0xFFFFFF | 4KB | 系统参数 (wear-leveling) |

## 字库生成

使用 PCtoLCD2002 生成字库文件。

在其字模选项中，设置点阵格式为阴码，取模方式为列行式，取模走向为逆向，输出数制为十六进制，C51格式。

完成字模选项的配置后，在首页设置对应的字宽和字高（12/16/20）。

完成后选择生成文本字库的图标（在字模选项设置图标的左边），在其页面中，勾选生成索引文件、生成二进制字库文件、保持原始顺序。最后点击右下角的生成国标汉字库（就是GB2312格式的字库）。

## 烧录工具设置

串口工具软件使用的是 SSCOM V5.13.1

设置波特率9600、取消勾选RTS和DTR、勾选HEX发送、取消勾选加时间戳和分包显示、取消勾选加回车换行。

在上方导航栏中，设置：发送->发送文件延迟设置->连续发送。

完成设置后，首页点击打开文件，选择要传输的字库，等待MCU准备完成后，点击发送文件。

## 烧录步骤

每次只取消一个烧录函数的注释，烧录完再注释回来，再烧下一个。

### 准备文件

| 文件 | 说明 |
|------|------|
| `HZK12.bin` | 12x12 字库 |
| `HZK16.bin` | 16x16 字库 |
| `HZK20.bin` | 20x20 字库 |
| `unicode_gb2312_map.bin` | 映射表 (运行 `python unicode_to_gb2312_table.py` 生成) |

### 烧录顺序

在 `empty.c` 中，每次取消一个函数注释，编译下载，串口打印 `READY` 后发送对应 bin 文件：

| 步骤 | 取消注释的函数 | 发送的文件 | 大小 | 约耗时 |
|------|--------------|-----------|------|--------|
| 1 | `font_burner_hzk16_run()` | HZK16.bin | 256KB | ~4.5分钟 |
| 2 | `font_burner_hzk12_run()` | HZK12.bin | 192KB | ~3.5分钟 |
| 3 | `font_burner_hzk20_run()` | HZK20.bin | 480KB | ~8.5分钟 |
| 4 | `font_burner_map_run()` | unicode_gb2312_map.bin | 42KB | ~45秒 |

每步完成后串口打印 `Done!`，重新注释该函数再进行下一步。

## 显示测试

在 `empty.c` 中取消测试代码的注释:

```c
OLED_Init();
OLED_Clear();
OLED_ShowChineseExt(0, 0, "你好", 12);                            // 12x12
OLED_ShowMixStringExt(0, 16, "Hi世界", 16, OLED_8X16_HALF);       // 16x16
OLED_ShowChineseExt(0, 36, "测试", 20);                            // 20x20
OLED_Update();
while(1);
```

## API 说明

所有函数位于 `oledUI/oled_ext_font.c`，头文件 `oledUI/oled_ext_font.h`。
函数自动识别 UTF-8 和 GB2312 编码，无需手动指定。

### 纯中文显示

```c
// fontSize: 12, 16, 20
OLED_ShowChineseExt(int16_t X, int16_t Y, char *Chinese, uint8_t fontSize);

// 带区域裁剪
OLED_ShowChineseAreaExt(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, Chinese, fontSize);
```

### 中英文混合显示

```c
// chineseFontSize: 12, 16, 20
// asciiFontSize: OLED_6X8_HALF, OLED_7X12_HALF, OLED_8X16_HALF, OLED_10X20_HALF
OLED_ShowMixStringExt(X, Y, String, chineseFontSize, asciiFontSize);

// 带区域裁剪
OLED_ShowMixStringAreaExt(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, String, chineseFontSize, asciiFontSize);
```

### 字号与 ASCII 字体对应关系

| 中文字号 | 对应 ASCII 字体 |
|---------|----------------|
| 12 | OLED_7X12_HALF |
| 16 | OLED_8X16_HALF |
| 20 | OLED_10X20_HALF |

### 底层读取

```c
// 从 Flash 读取单个汉字的字模数据
ExtFont_ReadChinese(uint8_t high, uint8_t low, uint8_t* buf, uint8_t fontSize);
```

## 涉及文件

| 文件 | 说明 |
|------|------|
| `hardware/hw_w25qxx.c/.h` | Flash 驱动 (含字库/映射表地址宏) |
| `middle/mid_font_burner.c/.h` | 串口烧录模块 |
| `oledUI/oled_ext_font.c/.h` | 外部字库读取与显示 |
| `fonts/HZK12.bin` | 12x12 字库文件 |
| `fonts/HZK16.bin` | 16x16 字库文件 |
| `fonts/HZK20.bin` | 20x20 字库文件 |
| `fonts/unicode_gb2312_map.bin` | Unicode→GB2312 映射表 |
| `fonts/unicode_to_gb2312_table.py` | 映射表生成脚本 |
| `empty.c` | 主程序入口 (烧录/测试代码) |
