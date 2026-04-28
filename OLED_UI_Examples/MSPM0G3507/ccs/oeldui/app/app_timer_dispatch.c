/**
 * app_timer_dispatch.c
 * Timer callback dispatch — registers 5/10/20ms tick handlers for button scan,
 * LED effects, OLED UI interrupt, and debug LED toggling.
 */
#include "app_timer_dispatch.h"
#include "mid_timer.h"
#include "mid_button.h"
#include "mid_debug_led.h"
#include "mid_music.h"
#include "OLED_UI.h"
#include "hw_ws2812_effects.h"

static void on_5ms_tick(void)
{
    mid_button_ticks();
    ws2812_effect_tick();
}

static void on_20ms_tick(void)
{
    OLED_UI_InterruptHandler();
    set_debug_led_toggle();
}

void app_timer_dispatch_init(void)
{
    mid_timer_register_cb_5ms(on_5ms_tick);
    mid_timer_register_cb_10ms(mid_beeper_proc);
    mid_timer_register_cb_20ms(on_20ms_tick);
}
