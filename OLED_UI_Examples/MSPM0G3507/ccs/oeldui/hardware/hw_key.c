/**
 * hw_key.c
 * Key GPIO hardware driver — scans key levels and returns raw debounced state.
 */
#include "hw_key.h"

KEY_STATUS key_scan(void)
{
    KEY_STATUS states;
    states.enter = DL_GPIO_readPins(KEY_ENTER_PORT, KEY_ENTER_PIN) ? 1 : 0;
    states.back = DL_GPIO_readPins(KEY_BACK_PORT, KEY_BACK_PIN) ? 0 : 1;
    return states;
}