## 例子总结

**开发环境**

该工程基于 TI（德州仪器）的 MSPM0G3519 制作。使用 CCS20.1.1 编译，XDS110 烧录。

**硬件参数**

| 参数 | 说明 |
| --- | --- |
| MCU | MSPM0G3519 (ARM Cortex-M0+) |
| RAM | 128KB |
| FLASH | 512KB |
| 屏幕 | SSD1312 128×64 单色 OLED |
| 通信方式 | Software I2C |
| IMU | LSM6DS3 六轴（加速度+陀螺仪） |
| 外部晶振 | 40MHz HFXT |

**实现功能**

通过两个按键 + 旋转编码器控制菜单：
* 左键为 BACK 与 UP 键，BACK 为长按效果，UP 为单击效果。
* 右键为 ENTER 与 DOWN 键，ENTER 为长按效果，DOWN 为单击效果。
* 编码器旋转 = UP/DOWN，编码器SW按键 = ENTER（单击）/ BACK（长按）。
* 编码器支持 QEI 硬件解码与 GPIO 软件解码两种后端，通过 `USE_QEI_ENCODER` 宏切换。

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
│   ├── 灯光亮度
│   └── 灯珠数量
├── 陀螺仪（全屏3D姿态显示，基于LSM6DS3+AHRS融合算法）
├── 串口（串口监视器，实时显示收发数据）
├── 机器人（机器人表情动画，27种表情）
├── 小游戏 (列表)
│   ├── 小恐龙（Chrome T-Rex 风格跑酷）
│   ├── 水管鸟（Flappy Bird）
│   └── 飞机大战（纵向射击）
├── 主题
└── 更多 (列表)
    ├── 串口监视器
    ├── 开机动画样式（4种：字符解码/粒子/涟漪圆/故障艺术）
    ├── 菜单类型样式（5种：列表/磁贴/纵深磁贴/HOPE磁贴/弧形磁贴）
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
- 上键/下键（或编码器旋转）：切换表情（循环）
- 确认键（或编码器SW单击）：下一个表情
- 返回键（或编码器SW长按）：退出回主菜单

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
| 蜂鸣器 | PB27 | PWM声音提醒 |
| 左键 (KEY0) | PA18 | BACK（长按）和 UP（单击）键 |
| 右键 (KEY1) | PB21 | ENTER（长按）和 DOWN（单击）键 |
| 编码器 A相 | PA30 | 旋转编码器 A 相（QEI ccp1 / GPIO 中断） |
| 编码器 B相 | PA29 | 旋转编码器 B 相（QEI ccp0 / GPIO 中断） |
| 编码器 SW | PA31 | 编码器按键（单击=确认，长按=返回） |
| I2C屏幕 SDA | PA0 | 单色I2C屏幕通信数据引脚 |
| I2C屏幕 SCL | PA1 | 单色I2C屏幕通信时钟引脚 |
| I2C IMU SDA | PA28 | LSM6DS3 IMU I2C 数据引脚 |
| I2C IMU SCL | PA27 | LSM6DS3 IMU I2C 时钟引脚 |
| UART0-TX | PA10 | 串口调试-串口发送引脚 |
| UART0-RX | PA11 | 串口调试-串口接收引脚 |
| UART7-TX | PB17 | 2.4G无线串口-发送引脚 |
| UART7-RX | PB18 | 2.4G无线串口-接收引脚 |
| 无线连接状态 | PB23 | 2.4G无线模块连接状态指示 |
| DEBUGSS SWCLK | PA20 | SW烧录时钟引脚 |
| DEBUGSS SWDIO | PA19 | SW烧录数据引脚 |
| 外部晶振 HFXIN | PA5 | 40MHz 外部晶振输入 |
| 外部晶振 HFXOUT | PA6 | 40MHz 外部晶振输出 |
| WS2812 RGB灯 | PB26 | PWM (TIMA1) WS2812 RGB灯带控制 |
| SPI Flash CS | PB6 | W25Q128 片选引脚 |
| SPI Flash SCLK | PB9 | W25Q128 SPI时钟引脚 |
| SPI Flash MOSI | PB8 | W25Q128 SPI数据输出引脚 |
| SPI Flash MISO | PB7 | W25Q128 SPI数据输入引脚 |
| 指示灯 | PB22 | 低电平亮指示灯 |

## 编码器系统

编码器采用双后端架构，通过 `USE_QEI_ENCODER` 宏在编译时选择：

| 后端 | 宏值 | 实现文件 | 原理 |
| --- | --- | --- | --- |
| QEI 硬件 | `1`（默认） | `hw_encoder_qei.c` | 使用 TIMG8 硬件正交编码器，DMA 自动计数 |
| GPIO 软件 | `0` | `hw_encoder_gpio.c` | PA29/PA30 双边沿中断 + Gray 码状态转移表 |

编码器SW按键通过 MultiButton（key2）处理：
- **单击** → ENTER（确认）
- **长按** → BACK（返回/退出）

> QEI 后端占用 TIMG8 + DMA_CH0，如果与其它外设冲突可切换到 GPIO 后端。

## 字库烧录

W25Q128 Flash 支持存储外部中文字库，通过串口烧录。在 `empty.c` 中取消对应注释即可进入烧录模式：

```c
// font_burner_hzk16_run();   // 烧录 HZK16 字库 (发送 HZK16.bin)
// font_burner_hzk12_run();   // 烧录 HZK12 字库 (发送 HZK12.bin)
// font_burner_hzk20_run();   // 烧录 HZK20 字库 (发送 HZK20.bin)
// font_burner_map_run();     // 烧录 Unicode→GB2312 映射表
```

字库文件位于 `fonts/output/` 目录。烧录后使用 `OLED_ShowChineseExt()` / `OLED_ShowMixStringExt()` 显示中文。

## 硬件原理图

TODO

## 工程目录结构

```
oeldui/
├── app/                          # 应用层
│   ├── app_task.c/h              # 应用生命周期管理框架
│   ├── app_key_task.c/h          # 按键任务（MultiButton，含编码器SW）
│   ├── app_robot_face.c/h        # 机器人表情动画（27种表情）
│   ├── app_dino_game.c/h         # 小恐龙跑酷游戏
│   ├── app_bird_game.c/h         # 水管鸟（Flappy Bird）游戏
│   ├── app_plane_game.c/h        # 飞机大战游戏
│   ├── app_gyroscope.c/h         # 陀螺仪3D姿态显示
│   └── app_uart_monitor.c/h      # 串口监视器
├── oledUI/                       # OLED UI 框架层
│   ├── OLED.c/h                  # OLED底层驱动与绘图API
│   ├── OLED_driver.c/h           # I2C底层驱动
│   ├── OLED_UI.c/h               # 菜单引擎（列表/磁贴/窗口/开机动画）
│   ├── OLED_UI_Driver.c/h        # 屏幕驱动适配层
│   ├── OLED_UI_MenuData.c/h      # 菜单数据定义与回调注册
│   ├── OLED_Fonts.c/h            # 字体与图标位图数据
│   └── oled_ext_font.c/h         # 外部Flash字库读取
├── hardware/                     # 硬件驱动层
│   ├── hw_buzzer.c/h             # 蜂鸣器驱动
│   ├── hw_delay.c/h              # 延时函数
│   ├── hw_encoder.h              # 编码器接口（双后端选择）
│   ├── hw_encoder_qei.c          # QEI 硬件编码器后端
│   ├── hw_encoder_gpio.c         # GPIO 软件编码器后端
│   ├── hw_key.c/h                # 按键硬件抽象
│   ├── hw_lsm6ds3.c/h            # LSM6DS3 IMU驱动
│   ├── hw_w25qxx.c/h             # W25Qxx Flash存储
│   ├── hw_ws2812.c/h             # WS2812 RGB灯驱动
│   ├── hw_ws2812_effects.c/h     # WS2812 灯效控制
│   └── myiic.c/h                 # 软件I2C底层
├── middle/                       # 中间件层
│   ├── mid_button.c/h            # MultiButton 按键库
│   ├── mid_timer.c/h             # 系统定时器
│   ├── mid_music.c/h             # 蜂鸣器音乐
│   ├── mid_debug_uart.c/h        # 串口调试
│   ├── mid_debug_led.c/h         # 调试指示灯
│   ├── mid_wireless_uart.c/h     # 2.4G无线串口
│   ├── mid_font_burner.c/h       # 字库烧录
│   └── Fusion*.c/h               # IMU姿态融合算法
├── fonts/                        # 字库文件
│   └── output/
│       ├── HZK12.bin             # 12x12 中文字库
│       ├── HZK16.bin             # 16x16 中文字库
│       ├── HZK20.bin             # 20x20 中文字库
│       └── unicode_gb2312_map.bin # Unicode→GB2312 映射表
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
| `OLED_DrawRectangle(x,y,w,h,filled)` | 画矩形 |
| `OLED_DrawRoundedRectangle(x,y,w,h,r,filled)` | 画圆角矩形 |
| `OLED_DrawCircle(x,y,r,filled)` | 画圆 |
| `OLED_DrawEllipse(x,y,a,b,filled)` | 画椭圆 |
| `OLED_DrawArc(x,y,r,start,end,filled)` | 画弧线 |
| `OLED_DrawTriangle(x0,y0,x1,y1,x2,y2,filled)` | 画三角形 |
| `OLED_ShowString(x,y,str,font)` | 显示字符串 |
| `OLED_ShowChineseExt(x,y,str,size)` | 显示外部字库中文（12/16/20） |
| `OLED_ShowMixStringExt(x,y,str,cn_size,en_font)` | 中英文混显 |
| `OLED_ShowImage(x,y,w,h,img)` | 显示位图 |
| `OLED_Update()` | 刷新显存到屏幕 |

## 添加新应用的步骤

1. 在 `app/` 目录创建 `app_xxx.c` 和 `app_xxx.h`
2. 实现 `xxx_init()`、`xxx_tick()`、`xxx_should_exit()`、`xxx_on_exit()` 四个函数
3. 在 `OLED_UI_MenuData.c` 中定义 `AppTaskDef` 并创建启动函数
4. 在 `MainMenuItems[]` 中添加菜单项
5. 在 `OLED_UI_MenuData.h` 中声明启动函数

## 移植建议

- OLED_UI 框架与硬件解耦，移植时只需替换 `OLED_driver.c` 中的 I2C 底层实现
- 按键部分替换 `app_key_task.c` 中的 GPIO 读取即可
- 编码器部分根据目标平台保留或移除 `hw_encoder_*.c`，并通过 `USE_QEI_ENCODER` 选择后端
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

## 与 MSPM0G3507 例程的主要差异

| 特性 | MSPM0G3507 | MSPM0G3519 |
| --- | --- | --- |
| 按键输入 | 2 键（左键/右键） | 2 键 + 旋转编码器（含SW按键） |
| IMU | 无/基础 | LSM6DS3 六轴 + AHRS 融合算法 |
| 无线通信 | 无 | 2.4G 无线串口（UART7） |
| 外部晶振 | 无 | 40MHz HFXT |
| 字库 | 内置 | 内置 + 外部 Flash 字库烧录 |
| 开机动画 | 基础 | 4 种可选（字符解码/粒子/涟漪圆/故障艺术） |
| 菜单样式 | 列表 + 单一磁贴 | 5 种：列表/磁贴/纵深磁贴/HOPE磁贴/弧形磁贴 |
| WS2812 | 基础控制 | 独立灯效模块（流水/跑马灯 + 亮度控制） |
| DMA | 无 | SPI Flash DMA 传输 |

## 致谢

- OLED UI 框架原作者：bilibili @上nm网课呢
- 开源地址：https://github.com/LaoGuaiGe/OLED_UI
- 按键库：[MultiButton](https://github.com/0x1abin/MultiButton)
- 姿态融合：[Fusion AHRS](https://github.com/xioTechnologies/Fusion)
- 机器人表情参考：[esp32-sh1106-oled-emojis-web](https://github.com/lsa709680-source/esp32-sh1106-oled-emojis-web)、[dog](https://github.com/wangergou2023/dog)

---

## Example Summary

**Development Environment**

The project is developed based on TI's MSPM0G3519 microcontroller (ARM Cortex-M0+, 32KB RAM, 128KB Flash). Compiled using CCS 20.1.1 and programmed via XDS110 debug probe. Equipped with LSM6DS3 6-axis IMU and 40MHz external crystal.

**Feature Implementation**

Menu control via two buttons + rotary encoder:

* Left button (KEY0): **Back** (long press) and **Up** (single click).
* Right button (KEY1): **Enter** (long press) and **Down** (single click).
* Encoder rotation = UP/DOWN; encoder SW button = ENTER (click) / BACK (long press).
* Encoder supports dual backend: QEI hardware (TIMG8) or GPIO interrupt decoding, switchable via `USE_QEI_ENCODER` macro.

> Button functionality is based on the open-source project [MultiButton](https://github.com/0x1abin/MultiButton), supporting single click and long press.

**Main Features:**
- Animated tile-style menu system (TILES_HOPE) with PID-curve smooth scrolling and 5 menu styles
- 4 selectable boot animations (decode, particle, ripple-circle, glitch art)
- Robot face expression system with 27 animated expressions and smooth transitions
- 3 built-in games: Dino Runner, Flappy Bird, Plane Shooter
- Gyroscope 3D attitude display with LSM6DS3 + AHRS fusion algorithm
- UART serial monitor (debug UART + 2.4G wireless UART)
- External Flash Chinese font storage and burning (HZK12/16/20)
- WS2812 RGB LED control with dedicated effects engine (solid, flowing, marquee, brightness)
- System settings with Flash persistence (brightness, volume, color mode, etc.)

## Peripherals & Pin Assignments

| Peripheral | Pin | Function |
| :--- | :--- | :--- |
| Buzzer | PB27 | PWM sound output |
| Left Button (KEY0) | PA18 | BACK (long press) and UP (single click) |
| Right Button (KEY1) | PB21 | ENTER (long press) and DOWN (single click) |
| Encoder Phase A | PA30 | Rotary encoder QEI ccp1 / GPIO interrupt |
| Encoder Phase B | PA29 | Rotary encoder QEI ccp0 / GPIO interrupt |
| Encoder SW | PA31 | Encoder button (click=ENTER, long press=BACK) |
| I2C OLED SDA | PA0 | OLED data pin |
| I2C OLED SCL | PA1 | OLED clock pin |
| I2C IMU SDA | PA28 | LSM6DS3 I2C data |
| I2C IMU SCL | PA27 | LSM6DS3 I2C clock |
| UART0-TX | PA10 | Debug serial TX |
| UART0-RX | PA11 | Debug serial RX |
| UART7-TX | PB17 | 2.4G wireless serial TX |
| UART7-RX | PB18 | 2.4G wireless serial RX |
| Wireless Link | PB23 | 2.4G wireless module link status |
| DEBUGSS SWCLK | PA20 | SW programming clock |
| DEBUGSS SWDIO | PA19 | SW programming data |
| HFXIN | PA5 | 40MHz external crystal input |
| HFXOUT | PA6 | 40MHz external crystal output |
| WS2812 RGB LED | PB26 | PWM (TIMA1) RGB LED strip |
| SPI Flash CS | PB6 | W25Q128 chip select |
| SPI Flash SCLK | PB9 | W25Q128 SPI clock |
| SPI Flash MOSI | PB8 | W25Q128 SPI data out |
| SPI Flash MISO | PB7 | W25Q128 SPI data in |
| Indicator LED | PB22 | Active-low debug LED |

## Drawing API Overview

The OLED driver provides the following drawing primitives. All operations write to a frame buffer; call `OLED_Update()` to flush to screen:

| Function | Description |
| --- | --- |
| `OLED_Clear()` | Clear the frame buffer |
| `OLED_ClearArea(x,y,w,h)` | Clear a rectangular area |
| `OLED_DrawPoint(x,y)` | Draw a single pixel |
| `OLED_DrawLine(x0,y0,x1,y1)` | Draw a line |
| `OLED_DrawRectangle(x,y,w,h,filled)` | Draw a rectangle |
| `OLED_DrawRoundedRectangle(x,y,w,h,r,filled)` | Draw a rounded rectangle |
| `OLED_DrawCircle(x,y,r,filled)` | Draw a circle |
| `OLED_DrawEllipse(x,y,a,b,filled)` | Draw an ellipse |
| `OLED_DrawArc(x,y,r,start,end,filled)` | Draw an arc |
| `OLED_DrawTriangle(x0,y0,x1,y1,x2,y2,filled)` | Draw a triangle |
| `OLED_ShowString(x,y,str,font)` | Display a string |
| `OLED_ShowChineseExt(x,y,str,size)` | Display Chinese chars from external font (12/16/20) |
| `OLED_ShowMixStringExt(x,y,str,cn_size,en_font)` | Mixed Chinese/English display |
| `OLED_ShowImage(x,y,w,h,img)` | Display a bitmap |
| `OLED_Update()` | Flush frame buffer to the display |

## Device Migration Recommendations

- Replace `OLED_driver.c` I2C implementation for different MCU platforms
- Replace GPIO reads in `app_key_task.c` for different button hardware
- Remove or adapt `hw_encoder_*.c` for platforms without rotary encoder
- Menu data in `OLED_UI_MenuData.c` can be freely customized

## FAQ
