# IMU 姿态解算方案说明

基于 [xioTechnologies/Fusion](https://github.com/xioTechnologies/Fusion) 库实现的 6 轴 IMU 姿态解算方案。

## 目录

- [方案概述](#方案概述)
- [文件结构](#文件结构)
- [代码分析](#代码分析)
- [移植指南](#移植指南)
- [九轴（磁力计）扩展](#九轴磁力计扩展)
- [调参指南](#调参指南)
- [已知限制](#已知限制)

---

## 方案概述

本方案使用 LSM6DS3 六轴传感器（3 轴加速度计 + 3 轴陀螺仪），通过 Fusion AHRS 算法将原始传感器数据融合为欧拉角（Roll/Pitch/Yaw）。

**数据流：**

```
LSM6DS3 传感器
    |
    v
I2C 读取原始数据（hw_lsm6ds3.c）
    |
    v
单位转换：加速度 mg->g，陀螺仪 mdps->dps
    |
    v
FusionOffset 零偏校正（运行时自动检测静止并修正陀螺仪零偏）
    |
    v
FusionAhrs 姿态解算（互补滤波，四元数积分）
    |
    v
四元数 -> 欧拉角（Roll/Pitch/Yaw，单位：度）
    |
    v
显示到 OLED / 驱动 3D 立方体
```

**关键特性：**
- X/Y 轴（Roll/Pitch）：加速度计 + 陀螺仪互补滤波，有重力绝对参考，精度高
- Z 轴（Yaw）：无磁力计，仅靠陀螺仪积分，静止时稳定，运动后有累积误差
- 运行时自动零偏校正，无需启动时长时间静止校准
- 使用实测 deltaTime，不依赖硬编码时间常量

---

## 文件结构

```
oeldui/
├── hardware/
│   ├── hw_lsm6ds3.c       # 传感器驱动 + 姿态解算调用
│   └── hw_lsm6ds3.h       # 传感器接口定义、Angle 结构体
├── middle/
│   ├── FusionAhrs.c        # Fusion AHRS 算法（姿态解算核心）
│   ├── FusionAhrs.h        # AHRS 接口定义
│   ├── FusionOffset.c      # Fusion 零偏校正算法（已调参）
│   ├── FusionOffset.h      # 零偏校正接口定义
│   ├── FusionMath.h        # 向量/四元数/欧拉角数学库（纯头文件）
│   ├── FusionConvention.h  # 坐标系约定（NWU/ENU/NED）
│   ├── mid_timer.c         # 5ms 定时器中断 + 全局毫秒计数器
│   └── mid_timer.h
├── app/
│   └── app_gyroscope.c     # 陀螺仪界面（3D 立方体 + 角度显示）
```

---

## 代码分析

### 1. 传感器初始化（hw_lsm6ds3.c: `lsm6ds3_init`）

```c
// 传感器配置
lsm6ds3_set_accelerometer_rate(LSM6DS3TRC_ACC_RATE_52HZ);  // 加速度计 52Hz
lsm6ds3_set_gyroscope_rate(LSM6DS3TRC_GYR_RATE_52HZ);      // 陀螺仪 52Hz
lsm6ds3_set_accelerometer_fullscale(LSM6DS3TRC_ACC_FSXL_2G); // 加速度计 ±2G
lsm6ds3_set_gyroscope_fullscale(LSM6DS3TRC_GYR_FSG_2000);    // 陀螺仪 ±2000dps

// Fusion 算法初始化
FusionOffsetInitialise(&fusionOffset, 16);  // 零偏校正，16Hz 实际采样率
FusionAhrsInitialise(&fusionAhrs);          // AHRS 初始化
FusionAhrsSetSettings(&fusionAhrs, &settings); // 配置参数
```

### 2. 姿态解算循环（hw_lsm6ds3.c: `lsm6ds3_get_angle`）

每帧执行流程：

```c
// 1. 计算真实 deltaTime（秒），使用 5ms 定时器中断提供的毫秒计数
deltaTime = (float)(now_ms - last_tick_ms) / 1000.0f;

// 2. 读取传感器并转换单位
//    加速度：mg -> g（Fusion 要求输入单位为 g）
//    陀螺仪：mdps -> dps（Fusion 要求输入单位为 dps）
lsm6ds3_get_acceleration(LSM6DS3TRC_ACC_FSXL_2G, acc);
for (i = 0; i < 3; i++) acc[i] /= 1000.0f;
lsm6ds3_get_gyroscope(LSM6DS3TRC_GYR_FSG_2000, gyr);
for (i = 0; i < 3; i++) gyr[i] /= 1000.0f;

// 3. 零偏校正（自动检测静止状态，缓慢修正陀螺仪零偏）
gyroscope = FusionOffsetUpdate(&fusionOffset, gyroscope);

// 4. AHRS 更新（6 轴，无磁力计）
FusionAhrsUpdateNoMagnetometer(&fusionAhrs, gyroscope, accelerometer, deltaTime);

// 5. 提取欧拉角
euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&fusionAhrs));
```

### 3. Yaw 归零机制

由于六轴 IMU 的 Yaw 轴没有绝对参考，启动后零偏校正需要时间收敛。收敛期间 Yaw 会漂移几度。解决方案：

```c
// AHRS 初始化期（1秒高增益快速收敛）结束后，
// 再等待 70 帧（约 4 秒）让零偏校正充分收敛，
// 然后调用 FusionAhrsSetHeading(0) 将 Yaw 归零
if (!fusionAhrs.initialising && !yaw_zeroed) {
    post_init_count++;
    if (post_init_count >= 70) {
        yaw_zeroed = 1;
        FusionAhrsSetHeading(&fusionAhrs, 0.0f);
    }
}
```

### 4. 多次采样优化（app_gyroscope.c）

为提高快速运动时的积分精度，在 OLED 渲染间隙（20ms）内循环多次读取 IMU：

```c
while (1) {
    // 20ms 内尽可能多次采样（约 3-4 次）
    uint32_t frame_start = get_sys_tick_ms();
    while ((get_sys_tick_ms() - frame_start) < 20) {
        lsm6ds3_get_angle(&angle_new);
    }
    // 渲染最新角度
    render_frame(&angle, buf);
    OLED_Update();
}
```

### 5. FusionOffset 参数（FusionOffset.c，已调参）

```c
#define CUTOFF_FREQUENCY (0.1f)  // 低通滤波截止频率（Hz），越大收敛越快
#define TIMEOUT          (1)     // 静止检测超时（秒），静止超过此时间开始修正
#define THRESHOLD        (8.0f)  // 静止判断阈值（dps），任一轴超过此值视为运动
```

> 注意：原始 Fusion 库默认值为 `CUTOFF=0.02, TIMEOUT=5, THRESHOLD=3`。
> 本项目传感器未校准零偏约 2-4 dps，因此 THRESHOLD 需大于零偏值。

### 6. FusionAhrs 初始化期（FusionAhrs.c，已调参）

```c
#define INITIAL_GAIN          (10.0f)  // 初始化期高增益，快速收敛
#define INITIALISATION_PERIOD (1.0f)   // 初始化期时长（秒），原始默认 3.0
```

---

## 移植指南

### 移植到其他 MCU + 其他 IMU 传感器

**第一步：复制 Fusion 库文件**

将以下文件复制到你的工程中（建议放在中间件/算法层目录）：

```
FusionAhrs.c / FusionAhrs.h
FusionOffset.c / FusionOffset.h
FusionMath.h
FusionConvention.h
```

这些文件是纯 C 实现，无平台依赖，支持任何 C99 编译器。

**第二步：提供毫秒级时间戳**

Fusion 需要精确的 `deltaTime`（秒）。你需要一个毫秒计数器：

```c
// 方案 A：定时器中断累加（本项目使用的方式）
static volatile uint32_t sys_tick_ms = 0;
void Timer_IRQHandler(void) {
    sys_tick_ms += 5; // 5ms 中断
}

// 方案 B：使用 HAL 库
uint32_t now_ms = HAL_GetTick(); // STM32
```

**第三步：读取传感器数据并转换单位**

Fusion 要求的输入单位：
| 传感器 | 单位 | 说明 |
|--------|------|------|
| 陀螺仪 | dps（度/秒） | Fusion 内部转换为 rad/s |
| 加速度计 | g（重力加速度） | 1g = 9.81 m/s² |
| 磁力计 | 任意单位 | 只关心方向，不关心幅值 |

根据你的传感器数据手册做转换，例如：

```c
// MPU6050 示例
float acc_g[3], gyr_dps[3];
acc_g[0] = (float)raw_acc[0] / 16384.0f;  // ±2G 量程
gyr_dps[0] = (float)raw_gyr[0] / 131.0f;  // ±250dps 量程

// LSM6DS3 示例（本项目）
acc_g[0] = (float)raw_acc[0] * 0.061f / 1000.0f;  // ±2G，0.061 mg/LSB
gyr_dps[0] = (float)raw_gyr[0] * 70.0f / 1000.0f; // ±2000dps，70 mdps/LSB
```

**第四步：初始化 Fusion**

```c
#include "FusionAhrs.h"
#include "FusionOffset.h"

FusionOffset offset;
FusionAhrs ahrs;

void imu_init(void)
{
    // 初始化零偏校正，参数为实际调用频率（Hz）
    FusionOffsetInitialise(&offset, 100); // 100Hz 采样

    // 初始化 AHRS
    FusionAhrsInitialise(&ahrs);

    const FusionAhrsSettings settings = {
        .convention = FusionConventionNwu,
        .gain = 0.5f,                       // 互补滤波增益
        .gyroscopeRange = 2000.0f,          // 匹配传感器量程
        .accelerationRejection = 10.0f,     // 加速度拒绝阈值（度）
        .magneticRejection = 0.0f,          // 无磁力计设为 0
        .recoveryTriggerPeriod = 5 * 100,   // 5秒 x 采样率
    };
    FusionAhrsSetSettings(&ahrs, &settings);
}
```

**第五步：循环调用**

```c
void imu_update(void)
{
    // 1. 读取传感器（转换为 dps 和 g）
    FusionVector gyroscope = {gyr_dps[0], gyr_dps[1], gyr_dps[2]};
    FusionVector accelerometer = {acc_g[0], acc_g[1], acc_g[2]};

    // 2. 零偏校正
    gyroscope = FusionOffsetUpdate(&offset, gyroscope);

    // 3. 计算 deltaTime
    float deltaTime = calculate_delta_time_seconds();

    // 4. AHRS 更新
    FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);

    // 5. 获取欧拉角
    FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    float roll  = euler.angle.roll;   // X 轴旋转
    float pitch = euler.angle.pitch;  // Y 轴旋转
    float yaw   = euler.angle.yaw;    // Z 轴旋转
}
```

### 移植注意事项

1. **deltaTime 必须准确**：这是影响精度的最关键因素。不要用硬编码常量，必须实测。
2. **传感器轴向**：不同传感器/PCB 安装方向不同，可能需要翻转某些轴（取反）或交换轴。
3. **FusionOffset 阈值**：如果你的传感器未校准零偏较大（>3 dps），需要增大 `THRESHOLD`。
4. **采样率**：`FusionOffsetInitialise` 和 `recoveryTriggerPeriod` 的参数要匹配实际采样率。

---

## 九轴（磁力计）扩展

如果你的硬件有磁力计（如 LSM9DS1、MPU9250、BMM150 等），可以获得 Yaw 轴的绝对参考，彻底解决 Yaw 漂移问题。

### 改动点

**1. 读取磁力计数据**

```c
// 磁力计单位：任意（Fusion 只关心方向）
// 常见单位：uT（微特斯拉）或 mGauss
FusionVector magnetometer = {mag[0], mag[1], mag[2]};
```

**2. 修改 AHRS 设置**

```c
const FusionAhrsSettings settings = {
    .convention = FusionConventionNwu,
    .gain = 0.5f,
    .gyroscopeRange = 2000.0f,
    .accelerationRejection = 10.0f,
    .magneticRejection = 10.0f,     // 启用磁力计拒绝（原来是 0）
    .recoveryTriggerPeriod = 5 * 16,
};
```

**3. 更换更新函数**

```c
// 6 轴（当前）
FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);

// 改为 9 轴
FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);
```

**4. 移除 Yaw 归零逻辑**

九轴有磁力计提供 Yaw 绝对参考，不再需要手动归零：

```c
// 删除以下代码
if (!fusionAhrs.initialising && !yaw_zeroed) {
    post_init_count++;
    if (post_init_count >= 70) {
        yaw_zeroed = 1;
        FusionAhrsSetHeading(&fusionAhrs, 0.0f);
    }
}
```

### 九轴的优势

| 特性 | 6 轴 | 9 轴 |
|------|------|------|
| Roll/Pitch 精度 | 高 | 高 |
| Yaw 静止漂移 | 需要零偏校正 | 无漂移 |
| Yaw 运动后回零 | 有累积误差（2-5度/次） | 自动回零 |
| 磁干扰敏感 | 无 | 有（电机、金属附近会干扰） |
| 启动时间 | 需等待零偏收敛 | 快速收敛 |

### 磁力计校准

磁力计需要校准硬铁和软铁偏差，Fusion 提供了校准函数：

```c
// 硬铁偏差（通过旋转传感器采集数据，取各轴最大最小值的中点）
const FusionVector hardIronOffset = {offset_x, offset_y, offset_z};

// 软铁矩阵（通常为单位矩阵，精确校准需要椭球拟合）
const FusionMatrix softIronMatrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

// 在 AHRS 更新前应用校准
magnetometer = FusionCalibrationMagnetic(magnetometer, softIronMatrix, hardIronOffset);
```

> 使用磁力计校准需要额外引入 `FusionCalibration.h`。

---

## 调参指南

| 参数 | 位置 | 默认值 | 作用 | 调整建议 |
|------|------|--------|------|----------|
| `gain` | AHRS settings | 0.5 | 加速度计对姿态的修正力度 | 增大→更信任加速度计，减小→更信任陀螺仪 |
| `gyroscopeRange` | AHRS settings | 2000.0 | 陀螺仪量程，超过 98% 触发恢复 | 匹配传感器实际量程 |
| `accelerationRejection` | AHRS settings | 10.0 | 加速度误差超过此角度则忽略加速度计 | 运动剧烈时可增大 |
| `INITIALISATION_PERIOD` | FusionAhrs.c | 1.0 | AHRS 初始化期时长（秒） | 越短启动越快，但初始精度略低 |
| `CUTOFF_FREQUENCY` | FusionOffset.c | 0.1 | 零偏低通滤波截止频率 | 越大收敛越快，但对噪声更敏感 |
| `TIMEOUT` | FusionOffset.c | 1 | 静止检测超时（秒） | 越短开始修正越快 |
| `THRESHOLD` | FusionOffset.c | 8.0 | 静止判断阈值（dps） | 必须大于传感器未校准零偏 |
| `post_init_count` | hw_lsm6ds3.c | 70 | 归零等待帧数 | 减小→启动更快，增大→归零更准 |

---

## 已知限制

1. **Yaw 漂移**：六轴 IMU 的 Yaw 轴没有绝对参考，运动后会有 2-5 度/次的累积误差。这是物理限制，只有加磁力计才能根本解决。
2. **采样率**：传感器 ODR 为 52Hz，但主循环受 OLED 渲染限制实际约 16Hz。通过多次采样优化到约 50Hz。
3. **软件 I2C**：当前使用 GPIO 模拟 I2C，每次读取有 delay 开销。改用硬件 I2C/DMA 可进一步提高采样率。
4. **启动等待**：进入陀螺仪界面后需等待约 5 秒（AHRS 初始化 1 秒 + 零偏收敛 4 秒）才能归零 Yaw。
