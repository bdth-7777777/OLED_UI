#ifndef LSM6DS3_H
#define LSM6DS3_H

/************************ 陀螺仪 ************************/
#define LSM6DS3TRC_I2CADDR 0x6A//SA0接GND，如果接的是VCC，则地址是0x6B
#define LSM6DS3TRC_WHO_AM_I		0x0F //Who am I
#define LSM6DS3TRC_CTRL3_C		0x12

//加速度计控制寄存器
#define LSM6DS3TRC_CTRL1_XL		0x10

//线性加速输出数据速率
#define LSM6DS3TRC_ACC_RATE_0	    0x00
#define LSM6DS3TRC_ACC_RATE_1HZ6	0xB0
#define LSM6DS3TRC_ACC_RATE_12HZ5	0x10
#define LSM6DS3TRC_ACC_RATE_26HZ	0x20
#define LSM6DS3TRC_ACC_RATE_52HZ	0x30
#define LSM6DS3TRC_ACC_RATE_104HZ	0x40
#define LSM6DS3TRC_ACC_RATE_208HZ	0x50
#define LSM6DS3TRC_ACC_RATE_416HZ	0x60
#define LSM6DS3TRC_ACC_RATE_833HZ	0x70
#define LSM6DS3TRC_ACC_RATE_1660HZ	0x80
#define LSM6DS3TRC_ACC_RATE_3330HZ	0x90
#define LSM6DS3TRC_ACC_RATE_6660HZ	0xA0

//陀螺仪控制寄存器
#define LSM6DS3TRC_CTRL2_G		0x11

//线性陀螺仪输出数据速率
#define LSM6DS3TRC_GYR_RATE_0	    0x00
#define LSM6DS3TRC_GYR_RATE_1HZ6	0xB0
#define LSM6DS3TRC_GYR_RATE_12HZ5	0x10
#define LSM6DS3TRC_GYR_RATE_26HZ	0x20
#define LSM6DS3TRC_GYR_RATE_52HZ	0x30
#define LSM6DS3TRC_GYR_RATE_104HZ	0x40
#define LSM6DS3TRC_GYR_RATE_208HZ	0x50
#define LSM6DS3TRC_GYR_RATE_416HZ	0x60
#define LSM6DS3TRC_GYR_RATE_833HZ	0x70
#define LSM6DS3TRC_GYR_RATE_1660HZ	0x80
#define LSM6DS3TRC_GYR_RATE_3330HZ	0x90
#define LSM6DS3TRC_GYR_RATE_6660HZ	0xA0

//加速度计全量程
#define LSM6DS3TRC_ACC_FSXL_2G	0x00
#define LSM6DS3TRC_ACC_FSXL_16G	0x04
#define LSM6DS3TRC_ACC_FSXL_4G	0x08
#define LSM6DS3TRC_ACC_FSXL_8G	0x0C

//陀螺仪全量程
#define LSM6DS3TRC_GYR_FSG_245	0x00
#define LSM6DS3TRC_GYR_FSG_500	0x04
#define LSM6DS3TRC_GYR_FSG_1000	0x08
#define LSM6DS3TRC_GYR_FSG_2000	0x0C

#define LSM6DS3TRC_CTRL1_XL		0x10

//加速度计的模拟链带宽
#define LSM6DS3TRC_ACC_BW0XL_1500HZ	0x00
#define LSM6DS3TRC_ACC_BW0XL_400HZ	0x01

#define LSM6DS3TRC_CTRL8_XL		0x17

//加速度计带宽选择
//低通滤波器
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_50	    0x88
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_100  	0xA8
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_9	    0xC8
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_400	    0xE8
//高通滤波器
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_50  	0x04
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_100	0x24
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_9 	    0x44
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_400	0x64

//用户界面的状态数据寄存器
#define LSM6DS3TRC_STATUS_REG	0x1E
#define LSM6DS3TRC_STATUS_GYROSCOPE		0x02
#define LSM6DS3TRC_STATUS_ACCELEROMETER	0x01

//加速度计输出接口XYZ
#define LSM6DS3TRC_OUTX_L_XL		0x28
#define LSM6DS3TRC_OUTX_H_XL		0x29
#define LSM6DS3TRC_OUTY_L_XL		0x2A
#define LSM6DS3TRC_OUTY_H_XL		0x2B
#define LSM6DS3TRC_OUTZ_L_XL		0x2C
#define LSM6DS3TRC_OUTZ_H_XL		0x2D

//陀螺仪输出接口XYZ
#define LSM6DS3TRC_OUTX_L_G		0x22
#define LSM6DS3TRC_OUTX_H_G		0x23
#define LSM6DS3TRC_OUTY_L_G		0x24
#define LSM6DS3TRC_OUTY_H_G		0x25
#define LSM6DS3TRC_OUTZ_L_G		0x26
#define LSM6DS3TRC_OUTZ_H_G		0x27

typedef struct {
    float x;
    float y;
    float z;
    float w;
} quaternion_yuandian;

typedef struct {
    float x;
    float y;
    float z;
} Angle;

extern Angle angle_new;

unsigned char lsm6ds3_init(void);
void lsm6ds3_angle_return_zero(void);
void lsm6ds3_get_angle(Angle* angle);
void lsm6ds3_getAngle(Angle* angle);
void float_to_string(float num, char *str);
#endif