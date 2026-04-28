/**
 * mid_app_task.h
 * App task framework types — AppState, AppTaskDef struct, and task lifecycle function declarations.
 */
#ifndef __MID_APP_TASK_H__
#define __MID_APP_TASK_H__

#include <stdint.h>
#include <stdbool.h>

/* AppTaskDef 类型定义（供菜单创建 AppTaskDef 字面量） */
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

/* App 任务管理函数 */
void app_task_start(const AppTaskDef *app);
void app_task_stop(void);
void app_task_tick(void);
bool app_task_is_active(void);
AppState app_task_get_state(void);

#endif
