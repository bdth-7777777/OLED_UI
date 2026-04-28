/**
 * OLED_UI_Driver.c
 * Platform hardware abstraction layer — timer, key, encoder stubs, and delay functions.
 * Porting guide: implement these functions according to your target MCU. The OLED display
 * must work correctly (ShowString, etc.) before porting the input drivers.
 */

#include "OLED_UI_Driver.h"

/* delay_us macro (replaces hw_delay.h; uses delay_cycles from ti_msp_dl_config.h) */
#define delay_us(X) delay_cycles((80*(X)))


/**
 * @brief 定时器中断服务函数的初始化函数，用于产生20ms的定时器中断
 * @param 无
 * @return 无
 */
void Timer_Init(void)
{
  //TI无需代码配置
  //已于 sysconfig 图形化工具配置
}

/**
 * @brief 按键初始化函数，用于初始化按键GPIO
 * @param 无
 * @note GPIO被初始化为上拉输入模式（虽然在我的开发板上已经加上了上拉电阻，但是以防万一）
 * @return 无
 */
void Key_Init(void)
{
  //TI无需代码配置
  //已于 sysconfig 图形化工具配置
}



/**
 * @brief 编码器初始化函数，将定时器1配置为编码器模式
 * @param 无
 * @return 无
 */
void Encoder_Init(void)
{
	//该项目未使用
}


/**
 * @brief 编码器使能函数
 * @param 无
 * @return 无
 */
void Encoder_Enable(void)
{
  //该项目未使用
}

/**
 * @brief 编码器失能函数
 * @param 无
 * @return 无
 */
void Encoder_Disable(void)
{
  //该项目未使用
}

/**
 * @brief 获取编码器的增量计数值（四倍频解码）
 * 
 * @details 该函数通过读取定时器TIM1的计数值，对编码器信号进行四倍频解码处理。
 *          使用静态变量累积计数，并通过除法和取模运算去除多余的增量部分，
 *          确保返回精确的增量值。主要用于电机控制、位置检测等应用场景。
 * 
 * @note   函数内部会自动清零定时器计数值，确保下次读取的准确性
 * 
 * @return int16_t 返回解码后的编码器增量值
 */
int16_t Encoder_Get(void)
{
//    // 静态变量，用于在函数调用间保存未被4整除的余数
//    static int32_t encoderAccumulator = 0;
//    
//    // 读取当前定时器计数值
//    int16_t temp = TIM_GetCounter(TIM1);
//    
//    // 清零定时器计数器，为下次读取做准备
//    TIM_SetCounter(TIM1, 0);
//    
//    // 将当前读取值累加到累加器中
//    encoderAccumulator += temp;
//    
//    // 计算四倍频解码后的增量值（去除未完成的部分）
//    int16_t result = encoderAccumulator / 4;
//    
//    // 保存未被4整除的余数，保证精度
//    encoderAccumulator %= 4;
//    
//    // 返回解码后的增量值
//    return result;
	return 0;
}



/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{	
	delay_us( xus );
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 


