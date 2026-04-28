#ifndef _MID_DEBUG_LED_H_
#define _MID_DEBUG_LED_H_

#include "ti_msp_dl_config.h"

typedef enum {
    LED_OFF,
    LED_ON
} LED_STATE_ENUM;

typedef struct {
    LED_STATE_ENUM state;  
} DEBUG_LED_STRUCT;

void set_debug_led_on(void);
void set_debug_led_off(void);
void set_debug_led_toggle(void);
LED_STATE_ENUM get_debug_led_state(void);

#endif
