/**
 * hw_i2c_soft.c
 * Software bit-banged I2C bus driver — GPIO bit-banging for OLED and IMU communication.
 */
#include "hw_i2c_soft.h"
#include "hw_delay.h"

void iic_start(void)
{
    SDA_OUT();
    IIC_SDA(1);
    IIC_SCL(1);
    delay_us(4);
    IIC_SDA(0);
    delay_us(4);
    IIC_SCL(0);
}

void iic_stop(void)
{
    SDA_OUT();
    IIC_SCL(0);
    IIC_SDA(0);
    delay_us(4);
    IIC_SCL(1);
    IIC_SDA(1);
    delay_us(4);
}

unsigned char iic_wait_ack(void)
{
    unsigned char ucErrTime = 0;
    SDA_IN();
    IIC_SDA(1);
    delay_us(1);
    IIC_SCL(1);
    delay_us(1);
    while (SDA_GET())
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            iic_stop();
            return 1;
        }
    }
    IIC_SCL(0);
    return 0;
}

void iic_ack(void)
{
    IIC_SCL(0);
    SDA_OUT();
    IIC_SDA(0);
    delay_us(2);
    IIC_SCL(1);
    delay_us(2);
    IIC_SCL(0);
}

void iic_nack(void)
{
    IIC_SCL(0);
    SDA_OUT();
    IIC_SDA(1);
    delay_us(2);
    IIC_SCL(1);
    delay_us(2);
    IIC_SCL(0);
}

void iic_send_byte(unsigned char txd)
{
    unsigned char t;
    SDA_OUT();
    IIC_SCL(0);
    for (t = 0; t < 8; t++)
    {
        IIC_SDA( (txd & 0x80) >> 7 );
        txd <<= 1;
        delay_us(2);
        IIC_SCL(1);
        delay_us(2);
        IIC_SCL(0);
        delay_us(2);
    }
}

unsigned char iic_read_byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();
    for (i = 0; i < 8; i++)
    {
        IIC_SCL(0);
        delay_us(2);
        IIC_SCL(1);
        receive <<= 1;
        if (SDA_GET())
            receive++;
        delay_us(1);
    }
    if (!ack)
        iic_nack();
    else
        iic_ack();
    return receive;
}
