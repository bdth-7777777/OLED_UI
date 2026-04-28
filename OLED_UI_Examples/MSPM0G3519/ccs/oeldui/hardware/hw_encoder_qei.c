#include "hw_encoder.h"
#include "mid_music.h"

#if USE_QEI_ENCODER

#define QEI_MIDPOINT  32768U

static int32_t encoder_accumulator = 0;
static volatile uint8_t encoder_enabled = 0;

void HW_Encoder_Init(void)
{
    DL_TimerG_setTimerCount(QEI_ENCODER_INST, QEI_MIDPOINT);
    DL_TimerG_startCounter(QEI_ENCODER_INST);
    encoder_enabled = 1;
    encoder_accumulator = 0;
}

void HW_Encoder_Enable(void)
{
    if (!encoder_enabled) {
        encoder_accumulator = 0;
        DL_TimerG_setTimerCount(QEI_ENCODER_INST, QEI_MIDPOINT);
        DL_TimerG_startCounter(QEI_ENCODER_INST);
        encoder_enabled = 1;
    }
}

void HW_Encoder_Disable(void)
{
    if (encoder_enabled) {
        DL_TimerG_stopCounter(QEI_ENCODER_INST);
        DL_TimerG_setTimerCount(QEI_ENCODER_INST, QEI_MIDPOINT);
        encoder_enabled = 0;
        encoder_accumulator = 0;
    }
}

int16_t HW_Encoder_GetDelta(void)
{
    if (!encoder_enabled) {
        return 0;
    }

    uint16_t count = DL_TimerG_getTimerCount(QEI_ENCODER_INST);
    DL_TimerG_setTimerCount(QEI_ENCODER_INST, QEI_MIDPOINT);

    int32_t raw_delta = (int32_t)count - (int32_t)QEI_MIDPOINT;

    encoder_accumulator += raw_delta;
    int16_t result = (int16_t)(encoder_accumulator / 4);
    encoder_accumulator %= 4;

    if (result != 0) {
        Beeper_Perform(BEEPER_KEYPRESS);
    }

    return result;
}

#endif /* USE_QEI_ENCODER */
