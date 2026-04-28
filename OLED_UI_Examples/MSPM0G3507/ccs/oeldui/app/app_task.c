/**
 * app_task.c
 * App task scheduler — manages app lifecycle (init, tick, fade-in/out, exit)
 * with a frame-interval-based state machine.
 */
#include "app_task.h"
#include "app_key_task.h"
#include "OLED.h"
#include "mid_timer.h"

#define FADE_FRAME_MS 40

static const AppTaskDef *current_app = NULL;
static AppState state = APP_STATE_IDLE;
static int8_t fade_level = 0;
static uint32_t last_tick_ms = 0;

void app_task_start(const AppTaskDef *app)
{
    if (app == NULL || state != APP_STATE_IDLE) return;
    current_app = app;
    current_app->init();
    last_tick_ms = mid_get_sys_tick_ms();

    if (current_app->fade_tick && current_app->fade_steps > 0) {
        state = APP_STATE_FADE_IN;
        fade_level = current_app->fade_steps;
    } else {
        state = APP_STATE_RUNNING;
    }
}

void app_task_stop(void)
{
    if (current_app == NULL) return;
    if (current_app->on_exit) current_app->on_exit();
    key_menu.back = RELEASE;
    current_app = NULL;
    state = APP_STATE_IDLE;
    fade_level = 0;
}

bool app_task_is_active(void)
{
    return state != APP_STATE_IDLE;
}

AppState app_task_get_state(void)
{
    return state;
}

void app_task_tick(void)
{
    if (current_app == NULL) return;

    uint32_t now = mid_get_sys_tick_ms();

    switch (state) {
    case APP_STATE_FADE_IN:
        if (now - last_tick_ms < FADE_FRAME_MS) return;
        last_tick_ms = now;
        current_app->fade_tick(fade_level);
        fade_level--;
        if (fade_level < 1) {
            state = APP_STATE_RUNNING;
        }
        break;

    case APP_STATE_RUNNING:
        if (current_app->sample) {
            current_app->sample();
        }
        if (now - last_tick_ms < current_app->frame_interval_ms) return;
        last_tick_ms = now;

        current_app->tick();

        if (current_app->should_exit()) {
            if (current_app->fade_tick && current_app->fade_steps > 0) {
                state = APP_STATE_FADE_OUT;
                fade_level = 1;
            } else {
                app_task_stop();
            }
        }
        break;

    case APP_STATE_FADE_OUT:
        if (now - last_tick_ms < FADE_FRAME_MS) return;
        last_tick_ms = now;
        current_app->fade_tick(fade_level);
        fade_level++;
        if (fade_level > current_app->fade_steps) {
            app_task_stop();
        }
        break;

    case APP_STATE_IDLE:
    default:
        break;
    }
}
