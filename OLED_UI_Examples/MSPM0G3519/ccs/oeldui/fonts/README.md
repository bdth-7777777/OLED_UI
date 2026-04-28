# 外部 Flash 字库 — 生成、烧录与使用
## 概述

本方案将 HZK12/HZK16/HZK20 点阵字库烧录到外部 W25Q128 SPI Flash，
显示时从 Flash 读取字模数据，从而节省 MCU 内部 Flash 空间。

同时支持 Unicode→GB2312 映射表，使得源文件保持 UTF-8 编码也能直接写中文字符串。

## 目录结构

```
fonts/
├── README.md                 ← 本文档
├── tools/                    ← 工具脚本
│   ├── font_generator.py     ← 字库生成工具（带 GUI）
│   └── unicode_to_gb2312_table.py  ← Unicode→GB2312 映射表生成脚本
└── output/                   ← 生成产物（bin / 索引文件）
    ├── HZK12.bin             ← 12×12 字库
    ├── HZK12.TXT             ← 12×12 索引
    ├── HZK16.bin             ← 16×16 字库
    ├── HZK16.TXT             ← 16×16 索引
    ├── HZK20.bin             ← 20×20 字库
    ├── HZK20.TXT             ← 20×20 索引
    ├── unicode_gb2312_map.bin← Unicode→GB2312 映射表
    ├── ARK-Pixel-12.bin      ← 备用：方舟像素 12×12 字库
    └── ARK-Pixel-20.bin      ← 备用：方舟像素 20×20 字库
```

## 字模格式规范

| 项目 | 说明 |
|------|------|
| 编码标准 | GB2312-80，87 区（0xA1–0xF7）× 94 位 = 8178 字 |
| 点阵格式 | 阴码（1 = 像素亮） |
| 取模方式 | 列行式（按页组织，每页 8 行，页内逐列存储） |
| 取模走向 | 逆向 / 低位在前（bit0 = 页内最上行，bit7 = 页内最下行） |

### 字节布局示意

以 16×16 为例（2 页 × 16 列 = 32 字节）：

```
字节 0  → 第 0 列, 第 0–7 行   bit0=row0, bit1=row1, ..., bit7=row7
字节 1  → 第 1 列, 第 0–7 行
...
字节 15 → 第 15 列, 第 0–7 行
字节 16 → 第 0 列, 第 8–15 行  bit0=row8, bit1=row9, ..., bit7=row15
...
字节 31 → 第 15 列, 第 8–15 行
```

### 各字号参数

| 字号 | 像素 | 页数 | 每字字节 | 总字符 | bin 文件大小 |
|------|------|------|---------|--------|-------------|
| 12 | 12×12 | 2 | 24 | 8178 | 196,274 字节 |
| 16 | 16×16 | 2 | 32 | 8178 | 261,698 字节 |
| 20 | 20×20 | 3 | 60 | 8178 | 490,682 字节 |

> bin 文件末尾有 2 字节 0x00 padding，兼容 PCtoLCD2002 格式。

### GB2312 偏移计算公式

```c
uint32_t offset = ((high - 0xA1) * 94 + (low - 0xA1)) * char_size;
```

## Flash 地址分配（W25Q128）

| 区域 | 起始地址 | 结束地址 | 大小 |
|------|---------|---------|------|
| HZK16 字库 | `0x000000` | `0x03FFFF` | 256 KB |
| Unicode→GB2312 映射表 | `0x040000` | `0x04A3FF` | ~42 KB |
| HZK12 字库 | `0x050000` | `0x07FFFF` | 192 KB |
| HZK20 字库 | `0x080000` | `0x0FFFFF` | 512 KB |
| 系统参数 | `0xFFF000` | `0xFFFFFF` | 4 KB |

## 字库生成

### 方式一：字库生成工具（推荐）

依赖：Python 3 + Pillow

```bash
pip install Pillow
cd fonts/tools
python font_generator.py
```

工具功能：
- 自动扫描 Windows 系统字体，优先选择宋体/黑体
- 支持手动浏览选择 .ttf / .ttc / .otf 字体文件
- 字体下拉框支持键盘上下键快速切换，实时预览
- 预览文字输入框实时更新，从字节还原验证显示效果
- 支持 12/16/20 单独生成或一键全部生成
- 自动生成索引 TXT 文件
- 输出目录默认为 `fonts/output/`

### 方式二：Unicode→GB2312 映射表

```bash
cd fonts/tools
python unicode_to_gb2312_table.py
```

生成 `unicode_gb2312_map.bin`（41,984 字节），覆盖 U+4E00–U+9FFF。

### 方式三：PCtoLCD2002（第三方工具）

使用 PCtoLCD2002 生成字库文件。

在其字模选项中，设置点阵格式为阴码，取模方式为列行式，取模走向为逆向，输出数制为十六进制，C51格式。

完成字模选项的配置后，在首页设置对应的字宽和字高（12/16/20）。

完成后选择生成文本字库的图标（在字模选项设置图标的左边），在其页面中，勾选生成索引文件、生成二进制字库文件、保持原始顺序。最后点击右下角的生成国标汉字库（就是GB2312格式的字库）。

## 烧录步骤

### 串口工具设置（SSCOM V5.13.1）

- 波特率 9600
- 取消勾选 RTS 和 DTR
- 勾选 HEX 发送
- 取消勾选加时间戳、分包显示、加回车换行
- 发送 → 发送文件延迟设置 → 连续发送

### 烧录流程

在 `empty.c` 中，每次只取消一个烧录函数的注释，编译下载运行，串口打印 `READY` 后发送对应 bin 文件，完成后打印 `Done!`，再注释回来进行下一步。

| 步骤 | 取消注释的函数 | 发送的文件 | 大小 | 约耗时 |
|------|--------------|-----------|------|--------|
| 1 | `font_burner_hzk16_run()` | `output/HZK16.bin` | 256 KB | ~4.5 分钟 |
| 2 | `font_burner_hzk12_run()` | `output/HZK12.bin` | 192 KB | ~3.5 分钟 |
| 3 | `font_burner_hzk20_run()` | `output/HZK20.bin` | 480 KB | ~8.5 分钟 |
| 4 | `font_burner_map_run()` | `output/unicode_gb2312_map.bin` | 42 KB | ~45 秒 |

## 显示测试

在 `empty.c` 中取消测试代码注释：

```c
OLED_Init();
OLED_Clear();
OLED_ShowChineseExt(0, 0, "你好", 12);                            // 12×12
OLED_ShowMixStringExt(0, 16, "Hi世界", 16, OLED_8X16_HALF);       // 16×16
OLED_ShowChineseExt(0, 36, "测试", 20);                            // 20×20
OLED_Update();
while(1);
```

## API 参考

所有函数位于 `oledUI/oled_ext_font.c`，自动识别 UTF-8 / GB2312 编码。

### 纯中文显示

```c
OLED_ShowChineseExt(int16_t X, int16_t Y, char *Chinese, uint8_t fontSize);
OLED_ShowChineseAreaExt(RangeX, RangeY, RangeW, RangeH, X, Y, Chinese, fontSize);
```

### 中英文混合显示

```c
OLED_ShowMixStringExt(X, Y, String, chineseFontSize, asciiFontSize);
OLED_ShowMixStringAreaExt(RangeX, RangeY, RangeW, RangeH, X, Y, String, chineseFontSize, asciiFontSize);
```

### 字号与 ASCII 字体对应

| 中文字号 | ASCII 字体 |
|---------|-----------|
| 12 | `OLED_7X12_HALF` |
| 16 | `OLED_8X16_HALF` |
| 20 | `OLED_10X20_HALF` |

### 底层读取

```c
ExtFont_ReadChinese(uint8_t high, uint8_t low, uint8_t *buf, uint8_t fontSize);
```

## 涉及源文件

| 文件 | 说明 |
|------|------|
| `hardware/hw_w25qxx.c/.h` | Flash 驱动，含字库地址宏定义 |
| `middle/mid_font_burner.c/.h` | 串口烧录模块 |
| `oledUI/oled_ext_font.c/.h` | 外部字库读取与显示 |
| `oledUI/OLED_Fonts.c/.h` | 内置字体（ASCII + 少量中文） |
| `empty.c` | 主程序入口（烧录/测试代码） |
