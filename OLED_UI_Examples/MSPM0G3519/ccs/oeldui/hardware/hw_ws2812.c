#include "hw_ws2812.h"
#include "hw_delay.h"
#include "hw_ws2812_effects.h"

/*单个灯珠的需要传输的数据对应的比较值数组*/
uint32_t Single_WS2812B_Buffer[DATA_SIZE * WS2812B_NUM + 50] = {0};
volatile bool gChannel0InterruptTaken           = false;

/* RGB控制参数 */
bool     ws2812_enable  = true;
int16_t  ws2812_r       = 0;
int16_t  ws2812_g       = 0;
int16_t  ws2812_b       = 0;
int16_t  ws2812_led_num = 4;
int16_t  ws2812_brightness = 100;

/**
 * @brief TIM5_PWM_CH2&DMA1&PA1初始化
 * @param  arr重装载值（105 - 1）
 * @retval 无
 */
void PWM_WS2812B_Init(void)
{
	int i;
	DL_SYSCTL_disableSleepOnExit();
    NVIC_EnableIRQ(DMA_INT_IRQn);

    /* 事件织网：TIMA1 零事件 → 事件通道1 → DMA订阅者0 */
    DL_Timer_enableEvent(WS2812_INST, DL_TIMER_EVENT_ROUTE_1, DL_TIMER_EVENT_ZERO_EVENT);
    DL_Timer_setPublisherChanID(WS2812_INST, DL_TIMER_PUBLISHER_INDEX_0, 1);
    DL_DMA_setSubscriberChanID(DMA, DL_DMA_SUBSCRIBER_INDEX_0, 1);

    /* 关键修复：CC更新模式改为TIMER_LOAD，确保DMA写入的新CC值
     * 在下一个PWM周期零点才生效，防止mid-cycle写入导致bit错误 */
    DL_TimerA_setCaptCompUpdateMethod(WS2812_INST,
        DL_TIMER_CC_UPDATE_METHOD_ZERO_EVT,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    //设置DMA搬运的目的地址
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &WS2812_INST->COUNTERREGS.CC_01[GPIO_WS2812_C0_IDX]);
    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);

    /* 整个buffer填充T0H，确保所有PWM周期都是有效的WS2812 "0" bit */
    for (i = 0; i < DATA_SIZE * WS2812B_NUM + 50; i++) {
        Single_WS2812B_Buffer[i] = T0H;
    }

    /* CC=1空跑，输出仅12.5ns HIGH（低于WS2812识别阈值220ns），等效于持续LOW */
    WS2812_INST->COUNTERREGS.CC_01[GPIO_WS2812_C0_IDX] = 1;
    DL_Timer_startCounter(WS2812_INST);
}

void WS2812B_Write_24Bits(uint16_t num, uint32_t GRB_Data)
{
	uint8_t i, j;
	if (num > WS2812B_NUM) num = WS2812B_NUM;
	for (j = 0; j < num; j++)
	{
		for (i = 0; i < DATA_SIZE; i++)
		{
			Single_WS2812B_Buffer[i + j * DATA_SIZE] = ((GRB_Data << i) & 0x800000) ? T1H : T0H;
		}
	}
}

void WS2812B_Write_24Bits_independence(uint16_t num, uint32_t *GRB_Data)
{
	uint8_t i, j;
	if (num > WS2812B_NUM) num = WS2812B_NUM;
	for (j = 0; j < num; j++)
	{
		for (i = 0; i < DATA_SIZE; i++)
		{
			Single_WS2812B_Buffer[i + j * DATA_SIZE] = ((GRB_Data[j] << i) & 0x800000) ? T1H : T0H;
		}
	}
}

static void WS2812B_Send(void)
{
	DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);

	DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)Single_WS2812B_Buffer);
	DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, DATA_SIZE * WS2812B_NUM + 50);

	DL_Timer_disableEvent(WS2812_INST, DL_TIMER_EVENT_ROUTE_1, DL_TIMER_EVENT_ZERO_EVENT);
	DL_Timer_enableEvent(WS2812_INST, DL_TIMER_EVENT_ROUTE_1, DL_TIMER_EVENT_ZERO_EVENT);

	gChannel0InterruptTaken = false;
	DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);

	volatile uint32_t timeout = 200000;
	while (gChannel0InterruptTaken == false) {
		if (--timeout == 0) break;
	}

	DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
	WS2812_INST->COUNTERREGS.CC_01[GPIO_WS2812_C0_IDX] = 1;
}

void WS2812B_Show(void)
{
	NVIC_DisableIRQ(TIMER_TICK_INST_INT_IRQN);
	delay_us(80);
	WS2812B_Send();
	NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);
}

// N个灯珠发红光
void PWM_WS2812B_Red(uint16_t num)
{
	WS2812B_Write_24Bits(num, 0x00ff00); 
	WS2812B_Show();
}
// N个灯珠发绿光
void PWM_WS2812B_Green(uint16_t num)
{
	WS2812B_Write_24Bits(num, 0xff0000);
	WS2812B_Show();
}
// N个灯珠发蓝光
void PWM_WS2812B_Blue(uint16_t num)
{
	WS2812B_Write_24Bits(num, 0x0000ff);
	WS2812B_Show();
}

/*呼吸灯*/
void set_ws2812_breathing(uint8_t index)
{
	int i = 0;
	switch (index)
	{
	case 0: /* red */
		for (i = 0; i < 254; i += 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | i << 8 | 0x00)); 
			WS2812B_Show();
			delay_ms(10);
		}
		for (i = 254; i > 0; i -= 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | i << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		break;
	case 1: /* green */
		for (i = 0; i < 254; i += 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(i << 16 | 0x00 << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		for (i = 254; i > 0; i -= 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(i << 16 | 0x00 << 8 | 0x00));
			WS2812B_Show();
			delay_ms(10);
		}
		break;
	case 2: /* blue */
		for (i = 0; i < 254; i += 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | 0x00 << 8 | i));
			WS2812B_Show();
			delay_ms(10);
		}
		for (i = 254; i > 0; i -= 2)
		{
			WS2812B_Write_24Bits(64, (uint32_t)(0x00 << 16 | 0x00 << 8 | i));
			WS2812B_Show();
			delay_ms(10);
		}
		break;
	}
}

/*根据当前参数刷新LED（仅参数变化时才实际发送）*/
void ws2812_update(void)
{
	static bool     prev_enable  = 0xFF;  /* 初始值故意不同，确保首次一定刷新 */
	static int16_t  prev_r       = -1;
	static int16_t  prev_g       = -1;
	static int16_t  prev_b       = -1;
	static int16_t  prev_num     = -1;
	static int16_t  prev_bright  = -1;

	/* 参数没变就跳过 */
	if (ws2812_enable == prev_enable &&
	    ws2812_r == prev_r && ws2812_g == prev_g && ws2812_b == prev_b &&
	    ws2812_led_num == prev_num && ws2812_brightness == prev_bright) {
		return;
	}

	prev_enable = ws2812_enable;
	prev_r = ws2812_r;
	prev_g = ws2812_g;
	prev_b = ws2812_b;
	prev_num = ws2812_led_num;
	prev_bright = ws2812_brightness;

	if (!ws2812_enable || ws2812_led_num == 0) {
		WS2812B_Write_24Bits(WS2812B_NUM, 0x000000);
		WS2812B_Show();
		return;
	}

	uint32_t grb = ((uint32_t)(ws2812_g & 0xFF) << 16)
	             | ((uint32_t)(ws2812_r & 0xFF) << 8)
	             | ((uint32_t)(ws2812_b & 0xFF));

	/* 应用亮度 */
	uint16_t br = (uint16_t)ws2812_brightness;
	if (br > 100) br = 100;
	if (br < 100) {
		uint8_t g = (uint8_t)(((grb >> 16) & 0xFF) * br / 100);
		uint8_t r = (uint8_t)(((grb >>  8) & 0xFF) * br / 100);
		uint8_t b = (uint8_t)(( grb        & 0xFF) * br / 100);
		grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
	}
	uint16_t num = (uint16_t)ws2812_led_num;
	if (num > WS2812B_NUM) num = WS2812B_NUM;

	WS2812B_Write_24Bits(WS2812B_NUM, 0x000000);
	WS2812B_Write_24Bits(num, grb);
	WS2812B_Show();
}

void DMA_IRQHandler(void)
{
    switch (DL_DMA_getPendingInterrupt(DMA)) {
        case DL_DMA_EVENT_IIDX_DMACH0:
            /* 立刻禁用DMA，防止定时器零事件再次触发多余搬运 */
            DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
            gChannel0InterruptTaken = true;
            break;
        default:
            break;
    }
}