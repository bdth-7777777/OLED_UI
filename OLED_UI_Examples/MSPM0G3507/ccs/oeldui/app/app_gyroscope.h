/**
 * app_gyroscope.h
 * 3D gyroscope cube display interface — init, tick, sample, fade, and exit callbacks.
 */
#ifndef __APP_GYROSCOPE_H__
#define __APP_GYROSCOPE_H__

#include "stdbool.h"
#include "stdint.h"

void app_gyroscope_loop(void);
void app_gyroscope_request_exit(void);
bool app_gyroscope_should_exit(void);

void app_gyroscope_init(void);
void app_gyroscope_tick(void);
void app_gyroscope_sample(void);
void app_gyroscope_fade_tick(int8_t level);
void app_gyroscope_on_exit(void);

#endif
