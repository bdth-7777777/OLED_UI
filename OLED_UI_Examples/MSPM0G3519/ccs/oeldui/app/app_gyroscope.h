#ifndef APP_GYROSCOPE_H
#define APP_GYROSCOPE_H

#include "stdbool.h"
#include "stdint.h"

void gyroscope_loop(void);
void gyroscope_request_exit(void);
bool gyroscope_should_exit(void);

void gyroscope_init(void);
void gyroscope_tick(void);
void gyroscope_sample(void);
void gyroscope_fade_tick(int8_t level);
void gyroscope_on_exit(void);

#endif
