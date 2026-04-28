#include "hw_key.h"

KEY_STATUS key_scan(void)
{
    KEY_STATUS states;
    states.enter = DL_GPIO_readPins(KEY_ENTER_PORT, KEY_ENTER_PIN) ? 1 : 0;
    states.back = DL_GPIO_readPins(KEY_BACK_PORT, KEY_BACK_PIN) ? 0 : 1;
    states.encoder_sw = DL_GPIO_readPins(KEY_ENCODER_SW_PORT, KEY_ENCODER_SW_PIN) ? 1 : 0;
    // states.up = DL_GPIO_readPins(KEY_UP_PORT, KEY_UP_PIN) ? 1 : 0;
    // states.down = DL_GPIO_readPins(KEY_DOWN_PORT, KEY_DOWN_PIN) ? 1 : 0;

    return states;
}