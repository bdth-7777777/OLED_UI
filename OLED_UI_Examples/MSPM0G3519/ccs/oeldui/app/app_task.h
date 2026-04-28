#ifndef APP_TASK_H
#define APP_TASK_H

#include "stdbool.h"
#include "stdint.h"

typedef enum {
    APP_STATE_IDLE,
    APP_STATE_FADE_IN,
    APP_STATE_RUNNING,
    APP_STATE_FADE_OUT,
} AppState;

typedef struct {
    void (*init)(void);
    void (*tick)(void);
    void (*sample)(void);
    bool (*should_exit)(void);
    void (*on_exit)(void);
    void (*fade_tick)(int8_t level);
    int8_t fade_steps;
    uint32_t frame_interval_ms;
} AppTaskDef;

void     app_task_start(const AppTaskDef *app);
void     app_task_stop(void);
void     app_task_tick(void);
bool     app_task_is_active(void);
AppState app_task_get_state(void);

#endif
