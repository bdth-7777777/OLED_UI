/**
 * hw_i2c_soft.h
 * Software bit-banged I2C interface — GPIO SCL/SDA timing for OLED and IMU.
 */
#ifndef __HW_I2C_SOFT_H__
#define __HW_I2C_SOFT_H__

#include "ti_msp_dl_config.h"

// 设置SDA输出模式
#define SDA_OUT()   {                                                \
                        DL_GPIO_initDigitalOutput(OLED_SDA_IOMUX);    \
                        DL_GPIO_setPins(OLED_PORT, OLED_SDA_PIN);      \
                        DL_GPIO_enableOutput(OLED_PORT, OLED_SDA_PIN); \
                    }
// 设置SDA输入模式
#define SDA_IN()    { DL_GPIO_initDigitalInput(OLED_SDA_IOMUX); }

// 获取SDA引脚的电平变化
#define SDA_GET()   ( ( ( DL_GPIO_readPins(OLED_PORT,OLED_SDA_PIN) & OLED_SDA_PIN ) > 0 ) ? 1 : 0 )
// SDA与SCL输出
#define IIC_SDA(x)  ( (x) ? (DL_GPIO_setPins(OLED_PORT,OLED_SDA_PIN)) : (DL_GPIO_clearPins(OLED_PORT,OLED_SDA_PIN)) )
#define IIC_SCL(x)  ( (x) ? (DL_GPIO_setPins(OLED_PORT,OLED_SCL_PIN)) : (DL_GPIO_clearPins(OLED_PORT,OLED_SCL_PIN)) )

void iic_start(void);
void iic_stop(void);
void iic_send_byte(unsigned char txd);
unsigned char iic_read_byte(unsigned char ack);
unsigned char iic_wait_ack(void);
void iic_ack(void);
void iic_nack(void);

#endif
