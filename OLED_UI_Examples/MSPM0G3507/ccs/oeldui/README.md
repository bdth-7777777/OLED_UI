## 例子总结

**开发环境**

该工程基于 TI（德州仪器）的 MSPM0G3507 制作。使用 CCS20.1.1 编译，XDS110 烧录。

**硬件参数**

| 参数 | 说明 |
| --- | --- |
| MCU | MSPM0G3507 (ARM Cortex-M0+) |
| RAM | 32KB |
| FLASH | 128KB |
| 屏幕 | SSD1312 128×64 单色 OLED |
| 通信方式 | Software I2C |

**实现功能**

通过两个按键控制菜单的动作：
* 左键为back与up键，back为长按效果，up为单击效果。
* 右键为enter与down键，enter为长按效果，down为单击效果。

> 按键功能基于开源项目 [MultiButton](https://github.com/0x1abin/MultiButton) 移植，实现单击与长按效果。

## 菜单结构

主菜单采用 TILES_HOPE 磁贴风格（XOR选择框+底部揭示条），包含以下入口：

```
主菜单 (磁贴)
├── 设置 (列表)
│   ├── 亮度调节（滑动条窗口，范围5-255）
│   ├── 音量调节（滑动条窗口，范围5-100）
│   ├── 声音开关（单选框）
│   ├── 黑暗模式（单选框）
│   ├── 显示帧率（单选框）
│   ├── 保存数据（写入W25Qxx Flash）
│   ├── 此设备（设备信息）
│   └── 关于OLED UI（作者信息）
├── RGB灯 (列表)
│   ├── 灯光模式（0=关/1=流水灯/2=跑马灯）
│   ├── 渐变速度
│   ├── RGB开关
│   ├── 红色/绿色/蓝色 分量调节
│   └── 灯珠数量
├── 陀螺仪（全屏3D姿态显示，带淡入淡出动画）
├── 串口（串口监视器，实时显示收发数据）
├── 机器人（机器人表情动画，27种表情）
├── 小游戏 (列表)
│   ├── 小恐龙（Chrome T-Rex 风格跑酷）
│   ├── 水管鸟（Flappy Bird）
│   └── 飞机大战（纵向射击）
├── 主题
└── 更多 (列表)
    ├── 串口监视器
    ├── 字体高度 8/12/16/20 demo
    ├── 超长文本 demo
    ├── 回弹动画 demo
    ├── 超长菜单列表 demo
    ├── 超小区域菜单 demo
    └── 各类窗口 demo
```

## 机器人表情系统

机器人表情界面采用矢量绘图+参数插值方案，使用 OLED 绘图原语（矩形、圆、直线等）程序化绘制表情，表情切换时通过 Q8.8 定点数缓动插值实现丝滑过渡。所有运算均为整数运算，适配无 FPU 的 Cortex-M0+。

**核心特性：**
- 呼吸脉动：眼睛微幅缩放，查表 sin 驱动
- 自动眨眼：随机间隔 1.2-3 秒触发，2帧快速闭合+2帧恢复
- 表情过渡：切换时所有参数（眼睛大小、间距、眉毛角度、嘴巴形状等）平滑插值

**操作方式：**
- 上键/下键：切换表情（循环）
- 确认键：下一个表情
- 返回键：退出回主菜单

**支持的 27 种表情：**

| 序号 | 表情 | 英文名 | 特效说明 |
| --- | --- | --- | --- |
| 1 | 普通/平静 | Normal | 标准圆角矩形眼睛 |
| 2 | 开心/高兴 | Happy | 眯眯眼 + 上弯嘴角 |
| 3 | 伤心/悲伤 | Sad | 八字眉下垂 + 下弯嘴角 |
| 4 | 生气/愤怒 | Angry | 倒八字眉 + 下弯嘴 |
| 5 | 惊讶 | Surprise | 大圆眼 + O型嘴 |
| 6 | 困倦/打瞌睡 | Sleepy | 眼睛慢慢闭合→猛然睁大（150%）惊醒 + 惊醒时掉口水 |
| 7 | 怀疑/疑惑 | Suspect | 左眼眯起（45%）+ 右眼正常 + 单侧眉毛 |
| 8 | 喜爱/爱慕 | Love | 心形眼睛 + 上弯嘴角 |
| 9 | 谨慎/小心 | Cautious | 眼球在眼眶内左右来回摆动 + 小瞳孔 |
| 10 | 哭泣/流泪 | Crying | 悲伤八字眉 + 双眼流泪动画 |
| 11 | 傻笑/坏笑 | Smirk | 左眼眯起 + 嘴角上翘 |
| 12 | 尴尬/窘迫 | Awkward | 忧虑眉 + 波浪嘴 + 右上方流汗动画 |
| 13 | 害羞/羞涩 | Shy | 微眯眼 + 双颊腮红 + 上弯嘴 |
| 14 | 晕眩/迷糊 | Dizzy | 螺旋圈圈眼（多层同心圆+旋转点）+ 波浪嘴 |
| 15 | 得意/骄傲 | Proud | 眯眯眼 + 大笑嘴 + 嘴角上翘点缀 |
| 16 | 疯狂/抓狂 | Crazy | 大眼 + 愤怒眉 + 画面抖动 + 右上方井字愤怒符号 |
| 17 | 无奈/无语 | Helpless | 半闭眼 + 无奈眉 + 倒三角嘴 + 两侧耸肩摊手 |
| 18 | 吃惊/震惊 | Shocked | 一大一小不对称眼 + 正三角嘴 |
| 19 | 睡觉/熟睡 | Sleeping | 闭眼（线状）+ 鼻涕泡膨胀缩小动画 + 浮动 ZZZ |
| 20 | 无聊/打哈欠 | Bored | 眼睛慢闭+嘴巴圆形渐大→惊醒+右眼掉泪 |
| 21 | 期待/期望 | Expect | 大眼上移 + 倒三角嘴 |
| 22 | 安逸/舒适 | Cozy | 微眯眼 + 小倒三角嘴 |
| 23 | 叹气 | Sigh | 无奈眉 + O型嘴 + 气泡飘散动画 |
| 24 | 嫌弃 | Disdain | 微眯眼 + 波浪嘴快速抖动 |
| 25 | 吃东西/咀嚼 | Chewing | 微眯眼 + 嘴巴张合动画 + 食物碎屑 + 食物飞入 |
| 26 | 讨厌/嫌恶 | Disgust | 左眼眯起 + 倒八字眉 + 撇嘴 |
| 27 | 犹豫/纠结 | Hesitant | 左右不对称眼 + 眼球左右摆动 + S形嘴左右晃动 |

## 应用框架

每个独立应用（游戏、陀螺仪、串口监视器、机器人表情等）通过 `AppTaskDef` 结构体注册，由 `app_task.c` 统一管理生命周期：

```c
typedef struct {
    void (*init)(void);           // 初始化
    void (*tick)(void);           // 主循环（每帧调用）
    void (*sample)(void);         // 传感器采样（可选）
    bool (*should_exit)(void);    // 退出检测
    void (*on_exit)(void);        // 清理
    void (*fade_tick)(int8_t);    // 淡入淡出（可选）
    int8_t fade_steps;            // 淡入淡出帧数
    uint32_t frame_interval_ms;   // 帧间隔（ms）
} AppTaskDef;
```

状态机流程：`IDLE → FADE_IN → RUNNING → FADE_OUT → IDLE`

## 外设与引脚配置

| 外设 | 引脚 | 功能 |
| --- | --- | --- |
| 蜂鸣器 | PB27 | 声音提醒 |
| 左键 | PA18 | BACK（长按）和 UP（单击）键 |
| 右键 | PB21 | ENTER（长按）和 DOWN（单击）键 |
| I2C屏幕 SDA | PA0 | 单色I2C屏幕通信数据引脚 |
| I2C屏幕 SCL | PA1 | 单色I2C屏幕通信时钟引脚 |
| UART0-TX | PA10 | 串口调试-串口发送引脚 |
| UART0-RX | PA11 | 串口调试-串口接收引脚 |
| DEBUGSS SWCLK | PA20 | SW烧录时钟引脚 |
| DEBUGSS SWDIO | PA19 | SW烧录数据引脚 |
| WS2812 RGB灯 | - | WS2812 RGB灯带控制 |
| SPI Flash CS | PB6 | W25Q128 片选引脚 |
| SPI Flash SCLK | PB9 | W25Q128 SPI时钟引脚 |
| SPI Flash MOSI | PB8 | W25Q128 SPI数据输出引脚 |
| SPI Flash MISO | PB7 | W25Q128 SPI数据输入引脚 |
| 指示灯 | PB22 | 低电平亮指示灯 |

## 硬件原理图

TODO

## 工程目录结构

```
oeldui/
├── app/                          # 应用层
│   ├── app_task.c/h              # 应用生命周期管理框架
│   ├── app_key_task.c/h          # 按键任务（MultiButton）
│   ├── app_robot_face.c/h        # 机器人表情动画（27种表情）
│   ├── app_dino_game.c/h         # 小恐龙跑酷游戏
│   ├── app_bird_game.c/h         # 水管鸟（Flappy Bird）游戏
│   ├── app_plane_game.c/h        # 飞机大战游戏
│   ├── app_gyroscope.c/h         # 陀螺仪3D姿态显示
│   └── app_uart_monitor.c/h      # 串口监视器
├── oledUI/                       # OLED UI 框架层
│   ├── OLED.c/h                  # OLED底层驱动与绘图API
│   ├── OLED_driver.c             # I2C底层驱动
│   ├── OLED_UI.c/h               # 菜单引擎（列表/磁贴/窗口）
│   ├── OLED_UI_MenuData.c/h      # 菜单数据定义与回调注册
│   └── OLED_Fonts.c/h            # 字体与图标位图数据
├── hardware/                     # 硬件驱动层
│   ├── hw_delay.c/h              # 延时函数
│   ├── hw_w25qxx.c/h             # W25Qxx Flash存储
│   └── hw_ws2812.c/h             # WS2812 RGB灯驱动
├── middle/                       # 中间件层
│   ├── mid_timer.c/h             # 系统定时器
│   ├── mid_music.c/h             # 蜂鸣器音乐
│   └── Fusion*.c/h               # IMU姿态融合算法
└── empty.c                       # 主入口（main函数）
```

## 绘图 API 概览

OLED 驱动提供以下绘图原语，所有操作在显存缓冲区进行，调用 `OLED_Update()` 刷新到屏幕：

| 函数 | 说明 |
| --- | --- |
| `OLED_Clear()` | 清空显存 |
| `OLED_ClearArea(x,y,w,h)` | 清空指定区域 |
| `OLED_DrawPoint(x,y)` | 画点 |
| `OLED_DrawLine(x0,y0,x1,y1)` | 画线 |
| `OLED_DrawRectangle(x,y,w,h,filled)` | 画矩形，filled: OLED_FILLED / OLED_UNFILLED |
| `OLED_DrawRoundedRectangle(x,y,w,h,r,filled)` | 画圆角矩形 |
| `OLED_DrawCircle(x,y,r,filled)` | 画圆 |
| `OLED_DrawEllipse(x,y,a,b,filled)` | 画椭圆 |
| `OLED_DrawArc(x,y,r,start,end,filled)` | 画弧线 |
| `OLED_DrawTriangle(x0,y0,x1,y1,x2,y2,filled)` | 画三角形 |
| `OLED_ShowChar(x,y,ch,font)` | 显示单个 ASCII 字符 |
| `OLED_ShowString(x,y,str,font)` | 显示字符串（6x8/7x12/8x16/10x20 半宽或全宽） |
| `OLED_ShowNum(x,y,num,len,font)` | 显示数字 |
| `OLED_ShowImage(x,y,w,h,img)` | 显示位图 |
| `OLED_ShowImageArea(x,y,w,h,rx,ry,rw,rh,img)` | 带区域裁剪的位图显示 |
| `OLED_ReverseArea(x,y,w,h)` | 反色指定区域 |
| `OLED_Update()` | 刷新显存到屏幕 |
| `OLED_UI_FadeOut_Masking(x,y,w,h,level)` | 淡入淡出遮罩，level 越大遮罩越深 |

字体常量：`OLED_6X8_HALF`, `OLED_7X12_HALF`, `OLED_8X16_HALF`, `OLED_10X20_HALF`（半宽）；
`OLED_6X8_FULL`, `OLED_8X16_FULL`（全宽）。

## 添加新应用的步骤

下面以创建 "MyApp" 为例，逐步说明如何添加一个新应用到菜单中。

### 第 1 步：创建应用文件

在 `app/` 目录创建 `app_myapp.c` 和 `app_myapp.h`。

**app_myapp.h** — 声明应用回调函数：

```c
#ifndef __APP_MYAPP_H__
#define __APP_MYAPP_H__
#include <stdbool.h>
#include <stdint.h>

void app_myapp_init(void);
void app_myapp_tick(void);
bool app_myapp_should_exit(void);
void app_myapp_on_exit(void);

#endif
```

**app_myapp.c** — 实现应用逻辑：

```c
#include "app_myapp.h"
#include "app_key_task.h"   // 读取 key_menu 按键状态
#include "OLED.h"           // 绘图 API
#include "OLED_UI.h"        // 淡入淡出遮罩

static bool exit_requested = false;

void app_myapp_init(void)
{
    exit_requested = false;
    OLED_Clear();
    OLED_ShowString(10, 20, "MyApp Started!", OLED_8X16_HALF);
    OLED_Update();
}

void app_myapp_tick(void)
{
    // 读取按键
    if (key_menu.up == PRESS) {
        key_menu.up = RELEASE;
        // 处理 UP
    }
    if (key_menu.back == PRESS) {
        key_menu.back = RELEASE;
        exit_requested = true;   // 长按 KEY0 退出
    }

    // 渲染画面
    OLED_Clear();
    OLED_ShowString(10, 20, "MyApp Running", OLED_8X16_HALF);
    OLED_Update();
}

bool app_myapp_should_exit(void) { return exit_requested; }

void app_myapp_on_exit(void)
{
    OLED_Clear();
    OLED_Update();
}
```

> 如果应用不需要淡入淡出，可以不实现 `fade_tick`。`sample` 用于传感器采样（如陀螺仪），一般应用可设为 NULL。

### 第 2 步：在中间层桥接文件中声明

在 `middle/mid_ui_bridge.h` 末尾添加 extern 声明：

```c
void app_myapp_init(void);
void app_myapp_tick(void);
bool app_myapp_should_exit(void);
void app_myapp_on_exit(void);
```

> `mid_ui_bridge.h` 是 oledUI 菜单系统与 app 层之间的桥梁，所有菜单回调涉及的应用函数都应在此声明。

### 第 3 步：注册 AppTaskDef 并创建菜单回调

在 `oledUI/OLED_UI_MenuData.c` 中：

```c
#include "app_myapp.h"  // 确保 MID_UI_BRIDGE 已声明则此处可选

static const AppTaskDef myapp_app = {
    .init              = app_myapp_init,
    .tick              = app_myapp_tick,
    .sample            = NULL,
    .should_exit       = app_myapp_should_exit,
    .on_exit           = app_myapp_on_exit,
    .fade_tick         = NULL,
    .fade_steps        = 0,
    .frame_interval_ms = 30,
};

void MyAppStart(void){
    app_task_start(&myapp_app);
}
```

### 第 4 步：在菜单中添加入口

在 `MainMenuItems[]` 数组中添加一项：

```c
{.General_item_text = "我的应用", .General_callback = MyAppStart,
 .General_SubMenuPage = NULL, .Tiles_Icon = gImage_gyro},
```

### 第 5 步：在头文件中声明启动函数

在 `oledUI/OLED_UI_MenuData.h` 中添加：

```c
void MyAppStart(void);
```

---

完成以上 5 步后，编译下载，即可在主菜单中看到 "我的应用"，点击进入你的应用。

## 按键输入系统

系统使用两个实体按键控制菜单和应用，底层基于 [MultiButton](https://github.com/0x1abin/MultiButton) 实现单击/长按/重复按键检测。

### 按键对应关系

| 按键 | 短按 (单击) | 长按 |
| --- | --- | --- |
| KEY0 (左键) | UP（上） | BACK（返回/退出） |
| KEY1 (右键) | DOWN（下） | ENTER（确认） |

### 在应用中读取按键

全局变量 `key_menu`（类型 `KEY_MENU_STATUS`）存储当前按键状态，包含 4 个位字段：

```c
typedef struct {
    unsigned int enter : 1;  // KEY1 长按
    unsigned int back  : 1;  // KEY0 长按
    unsigned int up    : 1;  // KEY0 单击
    unsigned int down  : 1;  // KEY1 单击
} KEY_MENU_STATUS;
```

`PRESS = 0` 表示按下，`RELEASE = 1` 表示松开。

**标准用法示例：**

```c
// 在 tick() 中检测按键
if (key_menu.up == PRESS) {
    key_menu.up = RELEASE;       // 手动清除，防止重复触发
    // 执行"上"操作
}

if (key_menu.enter == PRESS) {
    key_menu.enter = RELEASE;
    // 执行"确认"操作
}

if (key_menu.back == PRESS) {
    key_menu.back = RELEASE;
    exit_requested = true;       // 请求退出应用
}
```

> 长按 KEY0 的处理已在 `app_key_task.c` 的 `key0_long_press_start_handler()` 中统一实现——它会依次调用所有注册应用的 `request_exit()` 函数，因此如果你的应用只需要"退出"功能，无需在应用内部单独处理长按。

## 系统参数持久化

系统参数通过 `settings_save()` 写入 W25Q128 Flash 的最后一个 4KB 扇区，采用 wear-leveling 策略（循环写入，扇区写满后再擦除），避免频繁擦写同一位置。

### 保存参数

在 `OLED_UI_MenuData.c` 的 `SavedataWindow()` 中定义要保存的数据：

```c
void SavedataWindow(void){
    uint8_t temp[13];          // 当前参数数组（13 字节）
    temp[0]  = (uint8_t)OLED_UI_Brightness;   // 屏幕亮度
    temp[1]  = mid_beeper0.sound_loud;        // 音量
    temp[2]  = mid_beeper0.enable;            // 声音开关
    temp[3]  = (uint8_t)ColorMode;            // 深色/浅色模式
    temp[4]  = (uint8_t)OLED_UI_ShowFps;      // 显示帧率
    temp[5]  = (uint8_t)ws2812_enable;        // RGB 开关
    temp[6]  = (uint8_t)ws2812_r;             // RGB 红色
    temp[7]  = (uint8_t)ws2812_g;             // RGB 绿色
    temp[8]  = (uint8_t)ws2812_b;             // RGB 蓝色
    temp[9]  = (uint8_t)ws2812_led_num;       // 灯珠数量
    temp[10] = (uint8_t)ws2812_light_mode;    // 灯光模式
    temp[11] = (uint8_t)effect_param.speed;   // 渐变速度
    temp[12] = (uint8_t)ws2812_brightness;    // 灯光亮度
    settings_save(temp);
}
```

### 加载参数

在 `empty.c` 的 `main()` 中：

```c
uint8_t temp[13] = {100, 50, 0, 0, 1, 1, 0, 0, 0, 4, 0, 50, 100};  // 默认值
settings_load(temp);
OLED_UI_Brightness       = (uint16_t)temp[0];
mid_beeper0.sound_loud   = temp[1];
mid_beeper0.enable       = temp[2];
ColorMode                = (bool)temp[3];
// ... 依此类推
```

### 如何扩展自定义参数

1. 将 `hw_w25qxx.h` 中的 `SETTINGS_DATA_SIZE` 和 `SETTINGS_RECORD_SIZE` 加 1
2. 在 `temp[]` 数组末尾增加对应字节
3. 在 `SavedataWindow()` 和 `main()` 中分别写入和读取该字节

> 修改 `SETTINGS_DATA_SIZE` 或 `SETTINGS_MAGIC` 后，Flash 中的旧数据会自动失效，下次保存会重新写入新记录。

## 移植建议

- OLED_UI 框架与硬件解耦，移植时只需替换 `OLED_driver.c` 中的 I2C 底层实现
- 按键部分替换 `app_key_task.c` 中的 GPIO 读取即可
- 菜单数据（`OLED_UI_MenuData.c`）可根据需求自由增删
- 详细的移植步骤和菜单创建教程请参考 [OLED_UI 根目录 README](../../../../README.md)

OLED_UI 框架支持的屏幕型号：

| 屏幕主控芯片 | 分辨率 | 通讯方式 |
| --- | --- | --- |
| SSD1306 | 128x32 | IIC |
| SSD1305 | 128x32 | SPI |
| SSD1306 | 128x64 | IIC |
| SSD1312 | 128x64 | IIC |
| SH1106 | 128x64 | SPI / IIC |
| SH1107 | 128x128 | SPI / IIC |

## 致谢

- OLED UI 框架原作者：bilibili @上nm网课呢
- 开源地址：https://github.com/LaoGuaiGe/OLED_UI
- 按键库：[MultiButton](https://github.com/0x1abin/MultiButton)
- 机器人表情参考：[esp32-sh1106-oled-emojis-web](https://github.com/lsa709680-source/esp32-sh1106-oled-emojis-web)、[dog](https://github.com/wangergou2023/dog)

---

## Example Summary

**Development Environment**

The project is developed based on TI's MSPM0G3507 microcontroller (ARM Cortex-M0+, 32KB RAM, 128KB Flash). Compiled using CCS 20.1.1 and programmed via XDS110 debug probe.

**Feature Implementation**

Controlling menu actions with two buttons:

* The left button serves as **Back** (long press) and **Up** (single click).
* The right button serves as **Enter** (long press) and **Down** (single click).

> The button functionality is implemented based on the open-source project [MultiButton](https://github.com/0x1abin/MultiButton), supporting single click and long press effects.

**Main Features:**
- Animated tile-style menu system (TILES_HOPE) with PID-curve smooth scrolling
- Robot face expression system with 27 animated expressions and smooth transitions
- 3 built-in games: Dino Runner, Flappy Bird, Plane Shooter
- Gyroscope 3D attitude display with IMU fusion
- UART serial monitor
- WS2812 RGB LED control (solid color, flowing, marquee effects)
- System settings with Flash persistence (brightness, volume, color mode, etc.)

## Peripherals & Pin Assignments

| Peripheral       | Pin   | Function                                  |
| :--------------- | :---- | :----------------------------------------- |
| Buzzer           | PB27  | Sound reminder                             |
| Left Button      | PA18  | BACK (long press) and UP (single click)    |
| Right Button     | PB21  | ENTER (long press) and DOWN (single click) |
| I2C Screen SDA   | PA0   | Data pin for monochrome I2C OLED           |
| I2C Screen SCL   | PA1   | Clock pin for monochrome I2C OLED          |
| UART0-TX         | PA10  | Serial transmit pin                        |
| UART0-RX         | PA11  | Serial receive pin                         |
| DEBUGSS SWCLK    | PA20  | SW programming clock pin                   |
| DEBUGSS SWDIO    | PA19  | SW programming data pin                    |
| Indicator Light  | PB22  | Indicator light (active low)               |

## Drawing API Overview

The OLED driver provides the following drawing primitives. All operations write to a frame buffer; call `OLED_Update()` to flush to screen:

| Function | Description |
| --- | --- |
| `OLED_Clear()` | Clear the frame buffer |
| `OLED_ClearArea(x,y,w,h)` | Clear a rectangular area |
| `OLED_DrawPoint(x,y)` | Draw a single pixel |
| `OLED_DrawLine(x0,y0,x1,y1)` | Draw a line |
| `OLED_DrawRectangle(x,y,w,h,filled)` | Draw a rectangle (OLED_FILLED / OLED_UNFILLED) |
| `OLED_DrawRoundedRectangle(x,y,w,h,r,filled)` | Draw a rounded rectangle |
| `OLED_DrawCircle(x,y,r,filled)` | Draw a circle |
| `OLED_DrawEllipse(x,y,a,b,filled)` | Draw an ellipse |
| `OLED_DrawArc(x,y,r,start,end,filled)` | Draw an arc |
| `OLED_DrawTriangle(x0,y0,x1,y1,x2,y2,filled)` | Draw a triangle |
| `OLED_ShowChar(x,y,ch,font)` | Display a single ASCII character |
| `OLED_ShowString(x,y,str,font)` | Display a string (6x8/7x12/8x16/10x20 fonts) |
| `OLED_ShowNum(x,y,num,len,font)` | Display a number |
| `OLED_ShowImage(x,y,w,h,img)` | Display a bitmap |
| `OLED_ShowImageArea(x,y,w,h,rx,ry,rw,rh,img)` | Display a bitmap with region clipping |
| `OLED_ReverseArea(x,y,w,h)` | Invert a rectangular area |
| `OLED_Update()` | Flush frame buffer to the display |
| `OLED_UI_FadeOut_Masking(x,y,w,h,level)` | Fade-in/out masking (higher level = heavier mask) |

Font constants: `OLED_6X8_HALF`, `OLED_7X12_HALF`, `OLED_8X16_HALF`, `OLED_10X20_HALF` (half-width);
`OLED_6X8_FULL`, `OLED_8X16_FULL` (full-width).

## Adding a New Application

Follow this step-by-step guide to create "MyApp" and add it to the menu.

### Step 1: Create the application files

Create `app/app_myapp.c` and `app/app_myapp.h`.

**app_myapp.h** — declare callback functions:

```c
#ifndef __APP_MYAPP_H__
#define __APP_MYAPP_H__
#include <stdbool.h>
#include <stdint.h>

void app_myapp_init(void);
void app_myapp_tick(void);
bool app_myapp_should_exit(void);
void app_myapp_on_exit(void);

#endif
```

**app_myapp.c** — implement application logic:

```c
#include "app_myapp.h"
#include "app_key_task.h"   // access key_menu for button state
#include "OLED.h"           // drawing API
#include "OLED_UI.h"        // fade transitions

static bool exit_requested = false;

void app_myapp_init(void)
{
    exit_requested = false;
    OLED_Clear();
    OLED_ShowString(10, 20, "MyApp Started!", OLED_8X16_HALF);
    OLED_Update();
}

void app_myapp_tick(void)
{
    // Read buttons
    if (key_menu.up == PRESS) {
        key_menu.up = RELEASE;
        // handle UP
    }
    if (key_menu.back == PRESS) {
        key_menu.back = RELEASE;
        exit_requested = true;   // long-press KEY0 to exit
    }

    // Render frame
    OLED_Clear();
    OLED_ShowString(10, 20, "MyApp Running", OLED_8X16_HALF);
    OLED_Update();
}

bool app_myapp_should_exit(void) { return exit_requested; }

void app_myapp_on_exit(void)
{
    OLED_Clear();
    OLED_Update();
}
```

> If your app doesn't need fade transitions, omit `fade_tick`. The `sample` callback is for sensor sampling (e.g. gyroscope); set to NULL for typical apps.

### Step 2: Declare in the middleware bridge

Add extern declarations to `middle/mid_ui_bridge.h`:

```c
void app_myapp_init(void);
void app_myapp_tick(void);
bool app_myapp_should_exit(void);
void app_myapp_on_exit(void);
```

> `mid_ui_bridge.h` is the bridge between the oledUI menu system and the app layer. All app functions referenced by menu callbacks must be declared here.

### Step 3: Register AppTaskDef and create a menu callback

In `oledUI/OLED_UI_MenuData.c`:

```c
static const AppTaskDef myapp_app = {
    .init              = app_myapp_init,
    .tick              = app_myapp_tick,
    .sample            = NULL,
    .should_exit       = app_myapp_should_exit,
    .on_exit           = app_myapp_on_exit,
    .fade_tick         = NULL,
    .fade_steps        = 0,
    .frame_interval_ms = 30,
};

void MyAppStart(void){
    app_task_start(&myapp_app);
}
```

### Step 4: Add a menu entry

Add an item to the `MainMenuItems[]` array:

```c
{.General_item_text = "My App", .General_callback = MyAppStart,
 .General_SubMenuPage = NULL, .Tiles_Icon = gImage_gyro},
```

### Step 5: Declare the start function

In `oledUI/OLED_UI_MenuData.h`:

```c
void MyAppStart(void);
```

---

After these 5 steps, compile and flash. "My App" will appear in the main menu.

## Button Input System

The system uses two physical buttons to control menus and applications, built on [MultiButton](https://github.com/0x1abin/MultiButton) for single-click, long-press, and repeat detection.

### Button Mapping

| Button | Single-click | Long-press |
| --- | --- | --- |
| KEY0 (Left) | UP | BACK (exit) |
| KEY1 (Right) | DOWN | ENTER (confirm) |

### Reading Buttons in Your App

The global variable `key_menu` (type `KEY_MENU_STATUS`) holds the current button state with 4 bit-fields:

```c
typedef struct {
    unsigned int enter : 1;  // KEY1 long-press
    unsigned int back  : 1;  // KEY0 long-press
    unsigned int up    : 1;  // KEY0 single-click
    unsigned int down  : 1;  // KEY1 single-click
} KEY_MENU_STATUS;
```

`PRESS = 0` means pressed; `RELEASE = 1` means released.

**Standard usage in tick():**

```c
if (key_menu.up == PRESS) {
    key_menu.up = RELEASE;       // clear flag to avoid repeat
    // handle "up" action
}

if (key_menu.enter == PRESS) {
    key_menu.enter = RELEASE;
    // handle "confirm" action
}

if (key_menu.back == PRESS) {
    key_menu.back = RELEASE;
    exit_requested = true;       // request app exit
}
```

> KEY0 long-press handling is centralized in `app_key_task.c` (`key0_long_press_start_handler`). It calls every registered app's `request_exit()` function, so if your app only needs the exit function, you don't need extra handling.

## System Settings Persistence

System settings are saved to the last 4KB sector of W25Q128 Flash using a wear-leveling strategy (cyclic writes, erase only when the sector is full).

### Saving Settings

In `OLED_UI_MenuData.c` `SavedataWindow()`:

```c
void SavedataWindow(void){
    uint8_t temp[13];          // current settings (13 bytes)
    temp[0]  = (uint8_t)OLED_UI_Brightness;   // screen brightness
    temp[1]  = mid_beeper0.sound_loud;        // volume
    temp[2]  = mid_beeper0.enable;            // sound on/off
    temp[3]  = (uint8_t)ColorMode;            // dark/light mode
    temp[4]  = (uint8_t)OLED_UI_ShowFps;      // show FPS
    temp[5]  = (uint8_t)ws2812_enable;        // RGB switch
    temp[6]  = (uint8_t)ws2812_r;             // RGB red
    temp[7]  = (uint8_t)ws2812_g;             // RGB green
    temp[8]  = (uint8_t)ws2812_b;             // RGB blue
    temp[9]  = (uint8_t)ws2812_led_num;       // LED count
    temp[10] = (uint8_t)ws2812_light_mode;    // light mode
    temp[11] = (uint8_t)effect_param.speed;   // effect speed
    temp[12] = (uint8_t)ws2812_brightness;    // brightness
    settings_save(temp);
}
```

### Loading Settings

In `empty.c` `main()`:

```c
uint8_t temp[13] = {100, 50, 0, 0, 1, 1, 0, 0, 0, 4, 0, 50, 100};  // defaults
settings_load(temp);
OLED_UI_Brightness       = (uint16_t)temp[0];
mid_beeper0.sound_loud   = temp[1];
mid_beeper0.enable       = temp[2];
ColorMode                = (bool)temp[3];
// ... and so on
```

### Extending with Custom Parameters

1. Increment `SETTINGS_DATA_SIZE` and `SETTINGS_RECORD_SIZE` in `hw_w25qxx.h` by 1
2. Add the extra byte(s) at the end of `temp[]`
3. Write/read the byte in both `SavedataWindow()` and `main()`

> After changing `SETTINGS_DATA_SIZE` or `SETTINGS_MAGIC`, old data in Flash is automatically invalidated and will be re-written on next save.

## Device Migration Recommendations

- Replace `OLED_driver.c` I2C implementation for different MCU platforms
- Replace GPIO reads in `app_key_task.c` for different button hardware
- Menu data in `OLED_UI_MenuData.c` can be freely customized

## FAQ
