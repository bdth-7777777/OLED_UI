#include "hw_buzzer.h"

void buzzer_init(void)
{
    //TI的CCS中已通过图形化配置PWM定时器为1MHz
}

//开启蜂鸣器
void buzzer_on(void)
{
    DL_Timer_startCounter(BUZZER_INST);
}

//关闭蜂鸣器
void buzzer_off(void)
{
    DL_Timer_stopCounter(BUZZER_INST);
}

//设置蜂鸣器PWM占空比
//用于音量调节
void buzzer_set_duty(uint16_t value)
{
    DL_TimerA_setCaptureCompareValue(BUZZER_INST, value, GPIO_BUZZER_C1_IDX);
}

//设置蜂鸣器PWM定时器重装载值
//用于音调调节
void buzzer_set_reload_value(uint16_t value)
{
    DL_Timer_setLoadValue(BUZZER_INST, value);
}


