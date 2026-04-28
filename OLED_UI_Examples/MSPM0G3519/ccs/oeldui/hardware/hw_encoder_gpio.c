#include "hw_encoder.h"
#include "mid_music.h"

#if !USE_QEI_ENCODER

/*--------------------------------------------------------------
 * 软件 GPIO 中断编码器后端
 * PA30 = A相, PA29 = B相
 * 使用 SysConfig 配置为 GPIO 输入 + 双边沿中断
 *--------------------------------------------------------------*/

#ifndef GPIO_ENCODER_A_PORT
#define GPIO_ENCODER_A_PORT   GPIOA
#endif
#ifndef GPIO_ENCODER_A_PIN
#define GPIO_ENCODER_A_PIN    DL_GPIO_PIN_30
#endif
#ifndef GPIO_ENCODER_B_PORT
#define GPIO_ENCODER_B_PORT   GPIOA
#endif
#ifndef GPIO_ENCODER_B_PIN
#define GPIO_ENCODER_B_PIN    DL_GPIO_PIN_29
#endif

static volatile int32_t encoder_delta = 0;
static volatile uint8_t encoder_enabled = 0;
static volatile uint8_t last_ab = 0;
static int32_t encoder_accumulator = 0;

/* Gray code 状态转移表: [last][new] → 方向 */
static const int8_t enc_table[4][4] = {
    /*          00   01   10   11  */
    /* 00 */ {  0,  -1,  +1,   0 },
    /* 01 */ { +1,   0,   0,  -1 },
    /* 10 */ { -1,   0,   0,  +1 },
    /* 11 */ {  0,  +1,  -1,   0 },
};

static uint8_t read_ab_state(void)
{
    uint8_t a = DL_GPIO_readPins(GPIO_ENCODER_A_PORT, GPIO_ENCODER_A_PIN) ? 1 : 0;
    uint8_t b = DL_GPIO_readPins(GPIO_ENCODER_B_PORT, GPIO_ENCODER_B_PIN) ? 1 : 0;
    return (uint8_t)((a << 1) | b);
}

void HW_Encoder_Init(void)
{
    last_ab = read_ab_state();
    encoder_delta = 0;
    encoder_accumulator = 0;

    DL_GPIO_setUpperPinsPolarity(GPIO_ENCODER_A_PORT,
        DL_GPIO_PIN_29_EDGE_RISE_FALL | DL_GPIO_PIN_30_EDGE_RISE_FALL);
    DL_GPIO_clearInterruptStatus(GPIO_ENCODER_A_PORT,
        GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);
    DL_GPIO_enableInterrupt(GPIO_ENCODER_A_PORT,
        GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);

    NVIC_EnableIRQ(GPIOA_INT_IRQn);
    encoder_enabled = 1;
}

void HW_Encoder_Enable(void)
{
    if (!encoder_enabled) {
        encoder_delta = 0;
        encoder_accumulator = 0;
        last_ab = read_ab_state();
        DL_GPIO_clearInterruptStatus(GPIO_ENCODER_A_PORT,
            GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);
        DL_GPIO_enableInterrupt(GPIO_ENCODER_A_PORT,
            GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);
        encoder_enabled = 1;
    }
}

void HW_Encoder_Disable(void)
{
    if (encoder_enabled) {
        DL_GPIO_disableInterrupt(GPIO_ENCODER_A_PORT,
            GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);
        encoder_enabled = 0;
        encoder_delta = 0;
        encoder_accumulator = 0;
    }
}

int16_t HW_Encoder_GetDelta(void)
{
    if (!encoder_enabled) {
        return 0;
    }

    __disable_irq();
    int32_t raw = encoder_delta;
    encoder_delta = 0;
    __enable_irq();

    encoder_accumulator += raw;
    int16_t result = (int16_t)(encoder_accumulator / 4);
    encoder_accumulator %= 4;

    if (result != 0) {
        Beeper_Perform(BEEPER_KEYPRESS);
    }

    return result;
}

void GROUP1_IRQHandler(void)
{
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(GPIO_ENCODER_A_PORT,
        GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);

    if (status & (GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN)) {
        uint8_t new_ab = read_ab_state();
        if (encoder_enabled) {
            encoder_delta += enc_table[last_ab][new_ab];
        }
        last_ab = new_ab;
        DL_GPIO_clearInterruptStatus(GPIO_ENCODER_A_PORT,
            GPIO_ENCODER_A_PIN | GPIO_ENCODER_B_PIN);
    }
}

#endif /* !USE_QEI_ENCODER */
