/**
 * hw_lsm6ds3.c
 * LSM6DS3TR-C 6-axis IMU hardware driver — I2C register R/W,
 * raw accelerometer/gyroscope data readout, sensor init and reset.
 */
#include "hw_lsm6ds3.h"
#include "hw_i2c_soft.h"
#include "hw_delay.h"



static void delay_syms(long X)     
{
	delay_ms(X);
}

unsigned char lsm6ds3_read_one_byte(unsigned char reg_addr)
{
	uint8_t dat = 0;	
		
    iic_start();//发送起始信号
    
    iic_send_byte((LSM6DS3TRC_I2CADDR<<1) | 0x00);//从设备地址    
    delay_syms(1);	
    		
  	if(iic_wait_ack())	/* 检测设备的ACK应答 */
	{
		iic_stop();//产生一个停止条件
	}
	
    iic_send_byte(reg_addr);//寄存器地址
    delay_syms(1);			
    
  	if(iic_wait_ack())	/* 检测设备的ACK应答 */
	{
		 iic_stop();//产生一个停止条件	  			
	}

		
    iic_start();//发送重复起始信号，准备读取数据
    
	iic_send_byte((LSM6DS3TRC_I2CADDR<<1) | 0x01);//从设备地址（读取模式）
    delay_syms(1);	
    		
  	if(iic_wait_ack())	/* 检测设备的ACK应答 */
	{
		iic_stop();//产生一个停止条件	  			
	}
	
    dat = iic_read_byte(0);
    
	iic_stop();//发送停止信号	
	  
    return dat;		
}

void lsm6ds3_read_command(uint8_t reg_addr, uint8_t *rev_data, uint8_t length)
{	
	while(length)
	{
		*rev_data++ = lsm6ds3_read_one_byte(reg_addr++);
		length--; 
	}		
}

void lsm6ds3_write_command(uint8_t reg_addr, uint8_t *send_data, uint16_t length)
{
	iic_start();	
		
	delay_syms(10);	

	iic_send_byte((LSM6DS3TRC_I2CADDR<<1) | 0x00);//发送设备地址	

	if(iic_wait_ack())	/* 检测设备的ACK应答 */
	{
		iic_stop();//产生一个停止条件	  								
	}
	else
	{	
	}	
		
	delay_syms(10);			
			
	iic_send_byte(reg_addr);//发送寄存器地址	

	delay_syms(10);	

	if(iic_wait_ack())	/* 检测设备的ACK应答 */
	{
		iic_stop();//产生一个停止条件	  					
	}
	else
	{	
	}	
			
	delay_syms(10);		
			
	iic_send_byte(*send_data);//发送数据		

	delay_syms(10);	

	if(iic_wait_ack())	/* 检测设备的ACK应答 */
	{
		iic_stop();//产生一个停止条件	  						
	}
	else
	{	
	}	
			
	delay_syms(10);	

	iic_stop();//产生一个停止条件	
}

uint8_t i2c_check_device(uint8_t _Address)
{
	uint8_t ucAck;
 
	
    iic_start();		/* 发送启动信号 */

    iic_send_byte(_Address );
    ucAck = iic_wait_ack();	/* 检测设备的ACK应答 */

    iic_stop();			/* 发送停止信号 */

    return ucAck;

}

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
		iic_stop();
		return 0;
	}
}

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

void lsm6ds3_set_accelerometer_rate(uint8_t rate)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= rate;//设置加速度计的数据采样率
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
}

void lsm6ds3_set_gyroscope_rate(uint8_t rate)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL2_G, buf, 1);
	buf[0] |= rate;//设置陀螺仪数据速率
	lsm6ds3_write_command(LSM6DS3TRC_CTRL2_G, buf, 1);
}

void lsm6ds3_set_accelerometer_fullscale(uint8_t value)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= value;//设置加速度计的满量程
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
}

void lsm6ds3_set_gyroscope_fullscale(uint8_t value)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL2_G, buf, 1);
	buf[0] |= value;//设置陀螺仪的满量程
	lsm6ds3_write_command(LSM6DS3TRC_CTRL2_G, buf, 1);
}

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

uint8_t lsm6ds3_get_status(void)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_STATUS_REG, buf, 1);
	return buf[0];
}



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

unsigned char lsm6ds3_hw_init(void)
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
    return 0;
}