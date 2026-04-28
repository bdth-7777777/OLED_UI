#ifndef __MYIIC_H
#define __MYIIC_H
#include "ti_msp_dl_config.h"

//设置SDA输出模式
#define SDA_OUT()   {                                                \
                        DL_GPIO_initDigitalOutput(OLED_SDA_IOMUX);    \
                        DL_GPIO_setPins(OLED_PORT, OLED_SDA_PIN);      \
                        DL_GPIO_enableOutput(OLED_PORT, OLED_SDA_PIN); \
                    }
//设置SDA输入模式
#define SDA_IN()    { DL_GPIO_initDigitalInput(OLED_SDA_IOMUX); }

//获取SDA引脚的电平变化
#define SDA_GET()   ( ( ( DL_GPIO_readPins(OLED_PORT,OLED_SDA_PIN) & OLED_SDA_PIN ) > 0 ) ? 1 : 0 )
//SDA与SCL输出
#define IIC_SDA(x)      ( (x) ? (DL_GPIO_setPins(OLED_PORT,OLED_SDA_PIN)) : (DL_GPIO_clearPins(OLED_PORT,OLED_SDA_PIN)) )
#define IIC_SCL(x)      ( (x) ? (DL_GPIO_setPins(OLED_PORT,OLED_SCL_PIN)) : (DL_GPIO_clearPins(OLED_PORT,OLED_SCL_PIN)) )


// IIC所有操作函数
void IIC_Start(void);                // 发送IIC开始信号
void IIC_Stop(void);                 // 发送IIC停止信号
void IIC_Send_Byte(unsigned char txd);          // IIC发送一个字节
unsigned char IIC_Read_Byte(unsigned char ack); // IIC读取一个字节
unsigned char IIC_Wait_Ack(void);               // IIC等待ACK信号
void IIC_Ack(void);                  // IIC发送ACK信号
void IIC_NAck(void);                 // IIC不发送ACK信号

#endif