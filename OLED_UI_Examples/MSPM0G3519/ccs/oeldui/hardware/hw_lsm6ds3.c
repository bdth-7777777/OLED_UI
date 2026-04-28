#include "hw_lsm6ds3.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "myiic.h"
#include "hw_delay.h"
#include "mid_timer.h"
#include "FusionAhrs.h"
#include "FusionOffset.h"


Angle angle_new;

// 上次调用时间戳（ms）
static uint32_t last_tick_ms = 0;

static void delay_syms(long X)     
{
	delay_ms(X);
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_ReadOneByte
 * 描述  ：从LSM6DS3TRC指定地址处开始读取一个字节数据
 * 输入  ：reg_addr地址
 * 输出  ：读取的数据dat
 * 调用  ：
 * 备注  ：
 *******************************************************************************/
unsigned char lsm6ds3_read_one_byte(unsigned char reg_addr)
{
	uint8_t dat = 0;	
		
    IIC_Start();//发送起始信号
    
    IIC_Send_Byte((LSM6DS3TRC_I2CADDR<<1) | 0x00);//从设备地址    
    delay_syms(1);	
    		
  	if(IIC_Wait_Ack())	/* 检测设备的ACK应答 */
	{
		IIC_Stop();//产生一个停止条件
	}
	
    IIC_Send_Byte(reg_addr);//寄存器地址
    delay_syms(1);			
    
  	if(IIC_Wait_Ack())	/* 检测设备的ACK应答 */
	{
		 IIC_Stop();//产生一个停止条件	  			
	}

		
    IIC_Start();//发送重复起始信号，准备读取数据
    
	IIC_Send_Byte((LSM6DS3TRC_I2CADDR<<1) | 0x01);//从设备地址（读取模式）
    delay_syms(1);	
    		
  	if(IIC_Wait_Ack())	/* 检测设备的ACK应答 */
	{
		IIC_Stop();//产生一个停止条件	  			
	}
	
    dat = IIC_Read_Byte(0);
    
	IIC_Stop();//发送停止信号	
	  
    return dat;		
}

/*******************************************************************************
 * 函数名：lsm6ds3_ReadCommand
 * 描述  ：对LSM6DS3TRC读取数据
 * 输入  ：uint8_t reg_addr, uint8_t *rev_data, uint8_t length
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_read_command(uint8_t reg_addr, uint8_t *rev_data, uint8_t length)
{	
	while(length)
	{
		*rev_data++ = lsm6ds3_read_one_byte(reg_addr++);
		length--; 
	}		
}

/*******************************************************************************
 * 函数名：lsm6ds3_WriteCommand
 * 描述  ：往LSM6DS3TRC写入命令
 * 输入  ：uint8_t reg_addr, uint8_t *send_data, uint16_t length
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_write_command(uint8_t reg_addr, uint8_t *send_data, uint16_t length)
{
	IIC_Start();	
		
	delay_syms(10);	

	IIC_Send_Byte((LSM6DS3TRC_I2CADDR<<1) | 0x00);//发送设备地址	

	if(IIC_Wait_Ack())	/* 检测设备的ACK应答 */
	{
		IIC_Stop();//产生一个停止条件	  								
	}
	else
	{	
	}	
		
	delay_syms(10);			
			
	IIC_Send_Byte(reg_addr);//发送寄存器地址	

	delay_syms(10);	

	if(IIC_Wait_Ack())	/* 检测设备的ACK应答 */
	{
		IIC_Stop();//产生一个停止条件	  					
	}
	else
	{	
	}	
			
	delay_syms(10);		
			
	IIC_Send_Byte(*send_data);//发送数据		

	delay_syms(10);	

	if(IIC_Wait_Ack())	/* 检测设备的ACK应答 */
	{
		IIC_Stop();//产生一个停止条件	  						
	}
	else
	{	
	}	
			
	delay_syms(10);	

	IIC_Stop();//产生一个停止条件	
}

/*******************************************************************************
 * 函数名：IIC_CheckDevice
 * 描述  ：检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
 * 输入  ：_Address：设备的I2C总线地址
 * 输出  ：
 * 调用  ：
 * 备注  ：
 *******************************************************************************/
uint8_t i2c_check_device(uint8_t _Address)
{
	uint8_t ucAck;
 
	
    IIC_Start();		/* 发送启动信号 */

    IIC_Send_Byte(_Address );
    ucAck = IIC_Wait_Ack();	/* 检测设备的ACK应答 */

    IIC_Stop();			/* 发送停止信号 */

    return ucAck;

}

/*******************************************************************************
 * 函数名：lsm6ds3_CheckOk
 * 描述  ：判断LSM6DS3TRC是否正常
 * 输入  ：void
 * 输出  ： 1 表示正常， 0 表示不正常
 * 调用  ：
 * 备注  ：
 *******************************************************************************/
uint8_t lsm6ds3_check_ok(void)
{
	if(i2c_check_device( LSM6DS3TRC_I2CADDR ) == 1)
	{
	    //printf("Device exist\r\n");
		return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
	    //printf("Device not exist\r\n");		
		IIC_Stop();
		return 0;
	}
}

/*******************************************************************************
 * 函数名：lsm6ds3_GetChipID
 * 描述  ：读取LSM6DS3TRC器件ID
 * 输入  ：void
 * 输出  ：返回值true表示0x6a，返回false表示不是0x6a
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
uint8_t lsm6ds3_get_Chip_id(void)
{
	uint8_t buf = 0;

	lsm6ds3_read_command(LSM6DS3TRC_WHO_AM_I, &buf, 1);//Who I am ID
	//printf("buf 0x%02X\r\n",buf);		
	if (buf == 0x6a)
	{
	    //printf("ID ok\r\n");	
		return 1;
	}
	else
	{
	    //printf("ID error\r\n");	
		return 0;
	}
}

/*******************************************************************************
 * 函数名：lsm6ds3_Reset
 * 描述  ：LSM6DS3TRC重启和重置寄存器
 * 输入  ：void
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_reset(void)
{
	uint8_t buf[1] = {0};
	//reboot modules
	buf[0] = 0x80;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);//BOOT->1
    delay_syms(15);
	//reset register
	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);//读取SW_RESET状态
	buf[0] |= 0x01;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);//将CTRL3_C寄存器的SW_RESET位设为1
	while (buf[0] & 0x01)
	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);//等到CTRL3_C寄存器的SW_RESET位返回0
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_BDU
 * 描述  ：LSM6DS3TRC设置块数据更新
 * 输入  ：uint8_t flag
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_set_BDU(uint8_t flag)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);

	if (flag == 1)
	{
		buf[0] |= 0x40;//启用BDU
		lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);
	}
	else
	{
		buf[0] &= 0xbf;//禁用BDU
		lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);
	}

	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Accelerometer_Rate
 * 描述  ：LSM6DS3TRC设置加速度计的数据采样率
 * 输入  ：uint8_t rate
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_set_accelerometer_rate(uint8_t rate)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= rate;//设置加速度计的数据采样率
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Gyroscope_Rate
 * 描述  ：LSM6DS3TRC设置陀螺仪数据速率
 * 输入  ：uint8_t rate
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_set_gyroscope_rate(uint8_t rate)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL2_G, buf, 1);
	buf[0] |= rate;//设置陀螺仪数据速率
	lsm6ds3_write_command(LSM6DS3TRC_CTRL2_G, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Accelerometer_Fullscale
 * 描述  ：LSM6DS3TRC加速度计满量程选择
 * 输入  ：uint8_t value
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_set_accelerometer_fullscale(uint8_t value)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= value;//设置加速度计的满量程
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Gyroscope_Fullscale
 * 描述  ：LSM6DS3TRC陀螺仪满量程选择
 * 输入  ：uint8_t value
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
void lsm6ds3_set_gyroscope_fullscale(uint8_t value)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL2_G, buf, 1);
	buf[0] |= value;//设置陀螺仪的满量程
	lsm6ds3_write_command(LSM6DS3TRC_CTRL2_G, buf, 1);
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Set_Accelerometer_Bandwidth
 * 描述  ：LSM6DS3TRC设置加速度计模拟链带宽
 * 输入  ：uint8_t BW0XL, uint8_t ODR
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：BW0XL模拟链带宽, ODR输出数据率
 *******************************************************************************/
void lsm6ds3_set_accelerometer_bandwidth(uint8_t BW0XL, uint8_t ODR)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= BW0XL;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);

	lsm6ds3_read_command(LSM6DS3TRC_CTRL8_XL, buf, 1);
	buf[0] |= ODR;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL8_XL, buf, 1);
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Status
 * 描述  ：从LSM6DS3TRC状态寄存器获取数据状态
 * 输入  ：void
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：
 *******************************************************************************/
uint8_t lsm6ds3_get_status(void)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_STATUS_REG, buf, 1);
	return buf[0];
}



/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Acceleration
 * 描述  ：从LSM6DS3TRC读取加速度计数据
 * 输入  ：uint8_t fsxl, float *acc_float
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：转换为浮点数的加速度值
 *******************************************************************************/
void lsm6ds3_get_acceleration(uint8_t fsxl, float *acc_float)
{
	uint8_t buf[6];
	int16_t acc[3];
	lsm6ds3_read_command(LSM6DS3TRC_OUTX_L_XL, buf, 6);//获取加速度计原始数据
	acc[0] = buf[1] << 8 | buf[0];
	acc[1] = buf[3] << 8 | buf[2];
	acc[2] = buf[5] << 8 | buf[4];

	switch (fsxl)//根据不同量程来选择输出的数据的转换系数
	{
	case LSM6DS3TRC_ACC_FSXL_2G:
		acc_float[0] = ((float)acc[0] * 0.061f);
		acc_float[1] = ((float)acc[1] * 0.061f);
		acc_float[2] = ((float)acc[2] * 0.061f);		
		break;

	case LSM6DS3TRC_ACC_FSXL_16G:
		acc_float[0] = ((float)acc[0] * 0.488f);
		acc_float[1] = ((float)acc[1] * 0.488f);
		acc_float[2] = ((float)acc[2] * 0.488f);
		break;

	case LSM6DS3TRC_ACC_FSXL_4G:
		acc_float[0] = ((float)acc[0] * 0.122f);
		acc_float[1] = ((float)acc[1] * 0.122f);
		acc_float[2] = ((float)acc[2] * 0.122f);
		break;

	case LSM6DS3TRC_ACC_FSXL_8G:
		acc_float[0] = ((float)acc[0] * 0.244f);
		acc_float[1] = ((float)acc[1] * 0.244f);
		acc_float[2] = ((float)acc[2] * 0.244f);
		break;
	}
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Gyroscope
 * 描述  ：从LSM6DS3TRC读取陀螺仪数据
 * 输入  ：uint8_t fsg, float *gry_float
 * 输出  ：void
 * 调用  ：内部调用
 * 备注  ：转换为浮点数的角速度值
 *******************************************************************************/
void lsm6ds3_get_gyroscope(uint8_t fsg, float *gry_float)
{
	uint8_t buf[6];
	int16_t gry[3];
	
	lsm6ds3_read_command(LSM6DS3TRC_OUTX_L_G, buf, 6);//获取陀螺仪原始数据

	gry[0] = buf[1] << 8 | buf[0];
	gry[1] = buf[3] << 8 | buf[2];
	gry[2] = buf[5] << 8 | buf[4];		
	switch (fsg)//根据不同量程来选择输出的数据的转换系数
	{
	case LSM6DS3TRC_GYR_FSG_245:
		gry_float[0] = ((float)gry[0] * 8.750f);
		gry_float[1] = ((float)gry[1] * 8.750f);
		gry_float[2] = ((float)gry[2] * 8.750f);
		break;
	case LSM6DS3TRC_GYR_FSG_500:
		gry_float[0] = ((float)gry[0] * 17.50f);
		gry_float[1] = ((float)gry[1] * 17.50f);
		gry_float[2] = ((float)gry[2] * 17.50f);
		break;
	case LSM6DS3TRC_GYR_FSG_1000:
		gry_float[0] = ((float)gry[0] * 35.00f);
		gry_float[1] = ((float)gry[1] * 35.00f);
		gry_float[2] = ((float)gry[2] * 35.00f);
		break;
	case LSM6DS3TRC_GYR_FSG_2000:
		gry_float[0] = ((float)gry[0] * 70.00f);
		gry_float[1] = ((float)gry[1] * 70.00f);
		gry_float[2] = ((float)gry[2] * 70.00f);
		break;
	}
}

/*******************************************************************************
 * Fusion AHRS 姿态融合相关变量
 * 参考：https://github.com/xioTechnologies/Fusion
 * - FusionOffset: 运行时陀螺仪零偏校正
 * - FusionAhrs:   AHRS姿态解算（6轴，无磁力计）
 *******************************************************************************/
static FusionOffset fusionOffset;
static FusionAhrs   fusionAhrs;
static uint8_t      yaw_zeroed = 0;        // Yaw归零标志
static unsigned int post_init_count = 0;    // 初始化结束后的帧计数

//字符顺序调转
static void reverse(char *str, int len) {
    int i;
    for (i= 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}
//整型转字符串
static void int_to_str(int num, char *str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
    } else {
        while (num > 0) {
            str[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    str[i] = '\0';
    reverse(str, i);
}
//浮点型转字符串
void float_to_string(float num, char *str) {
    char int_str[20];
    char dec_str[7]; // 存储6位小数

    int str_index = 0;

    // 处理负数
    if (num < 0) {
        str[str_index++] = '-';
        num = -num;
    }

    float num_abs = num;
    int int_part = (int)num_abs;
    float decimal_part = num_abs - (float)int_part;

    // 转换整数部分
    int_to_str(int_part, int_str);
    int int_len = strlen(int_str);
    memcpy(str + str_index, int_str, int_len);
    str_index += int_len;

    // 转换小数部分
    int i;
    for ( i = 0; i < 2; i++) {
        decimal_part *= 10.0f;
        int digit = (int)decimal_part;
        dec_str[i] = '0' + digit;
        decimal_part -= (float)digit;
    }
    dec_str[2] = '\0';

    // 去除末尾的零
    int decimal_len = 2;
    while (decimal_len > 0 && dec_str[decimal_len - 1] == '0') {
        decimal_len--;
    }

    // 添加小数点和有效小数位
    if (decimal_len > 0) {
        str[str_index++] = '.';
        memcpy(str + str_index, dec_str, decimal_len);
        str_index += decimal_len;
    }

    str[str_index] = '\0'; // 终止符
}

static float acc[3] = {0, 0, 0};
static float gyr[3] = {0, 0, 0};

/*******************************************************************************
 * 函数名：lsm6ds3_get_angle
 * 描述  ：读取IMU数据并通过Fusion AHRS解算姿态角
 * 输入  ：Angle* angle 输出的欧拉角（度）
 * 输出  ：void
 * 备注  ：X/Y由加速度计+陀螺仪互补滤波，Z(Yaw)无磁力计仅靠陀螺仪积分
 *******************************************************************************/
void lsm6ds3_get_angle(Angle *angle)
{
    int i;
    uint8_t status;
    float deltaTime;
    uint32_t now_ms;
    FusionVector gyroscope;
    FusionVector accelerometer;
    FusionEuler euler;

    status = lsm6ds3_get_status();
    if (!((status & LSM6DS3TRC_STATUS_ACCELEROMETER) &&
          (status & LSM6DS3TRC_STATUS_GYROSCOPE))) {
        return;
    }

    // 计算真实deltaTime（秒）
    now_ms = get_sys_tick_ms();
    deltaTime = (float)(now_ms - last_tick_ms) / 1000.0f;
    if (deltaTime <= 0.0f || deltaTime > 0.5f) deltaTime = 0.02f;
    last_tick_ms = now_ms;

    // 读取加速度（mg -> g）
    lsm6ds3_get_acceleration(LSM6DS3TRC_ACC_FSXL_2G, acc);
    for (i = 0; i < 3; i++) acc[i] /= 1000.0f;

    // 读取陀螺仪（mdps -> dps）
    lsm6ds3_get_gyroscope(LSM6DS3TRC_GYR_FSG_2000, gyr);
    for (i = 0; i < 3; i++) gyr[i] /= 1000.0f;

    // Fusion零偏校正（运行时自动检测静止并修正）
    gyroscope.axis.x = gyr[0];
    gyroscope.axis.y = gyr[1];
    gyroscope.axis.z = gyr[2];
    gyroscope = FusionOffsetUpdate(&fusionOffset, gyroscope);

    // 加速度计赋值
    accelerometer.axis.x = acc[0];
    accelerometer.axis.y = acc[1];
    accelerometer.axis.z = acc[2];

    // Fusion AHRS更新（6轴，无磁力计）
    FusionAhrsUpdateNoMagnetometer(&fusionAhrs, gyroscope, accelerometer, deltaTime);

    // 提取欧拉角（度）
    euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&fusionAhrs));
    angle->x = euler.angle.roll;
    angle->y = -euler.angle.pitch;
    angle->z = -euler.angle.yaw;

    // 等AHRS初始化结束后再计数70帧（~4秒），让offset收敛后归零Yaw
    if (!fusionAhrs.initialising && !yaw_zeroed) {
        post_init_count++;
        if (post_init_count >= 70) {
            yaw_zeroed = 1;
            FusionAhrsSetHeading(&fusionAhrs, 0.0f);
        }
    }
}

/*******************************************************************************
 * 函数名：lsm6ds3_angle_return_zero
 * 描述  ：角度归零，重置AHRS状态
 * 输入  ：void
 * 输出  ：void
 *******************************************************************************/
void lsm6ds3_angle_return_zero(void)
{
    angle_new.x = 0;
    angle_new.y = 0;
    angle_new.z = 0;
    FusionAhrsReset(&fusionAhrs);
    yaw_zeroed = 0;
    post_init_count = 0;
    lsm6ds3_reset();
}

/*******************************************************************************
 * 函数名：lsm6ds3_init
 * 描述  ：初始化LSM6DS3传感器及Fusion AHRS算法
 * 输入  ：void
 * 输出  ：0成功，1失败
 *******************************************************************************/
unsigned char lsm6ds3_init(void)
{
    // 检测设备是否存在
    if (lsm6ds3_check_ok() == 0) return 1;

    // 设备重启
    lsm6ds3_reset();

    // 在读取MSB和LSB之前不更新输出寄存器
    lsm6ds3_set_BDU(1);

    // 设置加速度计数据采样率 52Hz
    lsm6ds3_set_accelerometer_rate(LSM6DS3TRC_ACC_RATE_52HZ);

    // 设置陀螺仪数据采样率 52Hz
    lsm6ds3_set_gyroscope_rate(LSM6DS3TRC_GYR_RATE_52HZ);

    // 设置加速度计满量程 2G
    lsm6ds3_set_accelerometer_fullscale(LSM6DS3TRC_ACC_FSXL_2G);

    // 设置陀螺仪满量程 2000dps
    lsm6ds3_set_gyroscope_fullscale(LSM6DS3TRC_GYR_FSG_2000);

    // 设置加速度计模拟链带宽
    lsm6ds3_set_accelerometer_bandwidth(LSM6DS3TRC_ACC_BW0XL_400HZ, LSM6DS3TRC_ACC_LOW_PASS_ODR_100);

    delay_syms(100);

    // 初始化Fusion零偏校正（实际循环约16Hz）
    FusionOffsetInitialise(&fusionOffset, 16);

    // 初始化Fusion AHRS
    FusionAhrsInitialise(&fusionAhrs);
    yaw_zeroed = 0;
    post_init_count = 0;
    {
        const FusionAhrsSettings settings = {
            .convention = FusionConventionNwu,
            .gain = 0.5f,
            .gyroscopeRange = 2000.0f,          // 匹配LSM6DS3 2000dps量程
            .accelerationRejection = 10.0f,     // 加速度拒绝阈值（度）
            .magneticRejection = 0.0f,          // 无磁力计
            .recoveryTriggerPeriod = 5 * 16,    // 恢复触发周期（5秒 x 16Hz）
        };
        FusionAhrsSetSettings(&fusionAhrs, &settings);
    }

    // 初始化时间戳
    last_tick_ms = get_sys_tick_ms();
    return 0;
}