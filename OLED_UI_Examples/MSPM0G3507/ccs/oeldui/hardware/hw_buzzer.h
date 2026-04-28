/**
 * hw_buzzer.h
 * Buzzer hardware interface — PWM frequency control and buzzer pin macros.
 */
#ifndef __HW_BUZZER_H__
#define __HW_BUZZER_H__

#include "ti_msp_dl_config.h"

void buzzer_on(void);
void buzzer_off(void);
void buzzer_set_duty(uint16_t value);
void buzzer_set_reload_value(uint16_t value);

#endif

