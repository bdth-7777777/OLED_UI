/**
 * app_robot_face.c
 * Robot face expression display — renders animated facial expressions
 * using OLED primitives (eyes, brows, mouth) with smooth transitions
 * and character-based random eye blinking.
 */
#include "app_robot_face.h"
#include "app_key_task.h"
#include "OLED.h"
#include "stdlib.h"

#define CENTER_X  64
#define CENTER_Y  28

#define FP_SHIFT  8
#define TO_FP(x)  ((int32_t)(x) << FP_SHIFT)
#define FROM_FP(x) ((int16_t)(((x) + (1<<(FP_SHIFT-1))) >> FP_SHIFT))

static const int8_t sin8[32] = {
    0,3,6,9,12,15,18,20,22,24,26,27,28,29,30,30,
    30,30,30,29,28,27,26,24,22,20,18,15,12,9,6,3
};
static int8_t fast_sin8(uint8_t p) {
    return (p & 0x20) ? -sin8[p & 0x1F] : sin8[p & 0x1F];
}

static bool exit_requested;
static ExpressionType current_expr;
static uint16_t frame_count;
static uint8_t breath_phase;

typedef struct {
    int16_t eye_width, eye_height, eye_spacing, eye_offset_y;
    int16_t brow_offset_y, brow_angle_l, brow_angle_r;
    int16_t mouth_width, mouth_curve;
    int16_t l_eye_h_pct, r_eye_h_pct;
} FPState;

static FPState cur, tgt;
static int16_t blink_h_pct;
static uint8_t blink_timer, blink_wait;
static int8_t  blink_phase;

static const char *expr_names[] = {
    "Normal",  "Happy",   "Sad",     "Angry",
    "Surprise","Sleepy",  "Suspect", "Love",
    "Cautious","Crying",  "Smirk",   "Awkward",
    "Shy",     "Dizzy",   "Proud",   "Crazy",
    "Helpless",
    "Shocked",
    "Sleeping",
    "Bored",
    "Expect",
    "Cozy",
    "Sigh",
    "Disdain",
    "Chewing",
    "Disgust",
    "Hesitant"
};

//                                         ew  eh  er  sp  oy brow bal bar mw  mh  mc  L%   R%
static const RobotExpression presets[EXPR_COUNT] = {
    [EXPR_NORMAL]     = { 22, 22, 6, 50,  0,   0,  0,  0, 12, 0,  0, 100, 100 },
    [EXPR_HAPPY]      = { 24, 10, 8, 50,  2,   0,  0,  0, 16, 0,  4, 100, 100 },
    [EXPR_SAD]        = { 24, 18, 4, 52,  0,  -6,  5, -5, 14, 0, -2, 100, 100 },
    [EXPR_ANGRY]      = { 20, 18, 5, 46,  4,  -8, -3,  3, 10, 0, -3, 100, 100 },
    [EXPR_SURPRISED]  = { 28, 28,12, 48, -2,   0,  0,  0,  0, 0,  0, 100, 100 },
    [EXPR_SLEEPY]     = { 24, 18, 4, 48,  4,   0,  0,  0, 10, 0,  0, 100, 100 },
    [EXPR_SUSPICIOUS] = { 22, 22, 5, 50,  0,  -7,  0, -4, 10, 0, -1,  45, 100 },
    [EXPR_LOVE]       = { 22, 22, 8, 50,  0,   0,  0,  0, 12, 0,  3, 100, 100 },
    [EXPR_CAUTIOUS]   = { 18, 18, 5, 50,  0,   0,  0,  0,  8, 0, -1, 100, 100 },
    [EXPR_CRYING]     = { 22, 22, 6, 50,  0,  -6,  3, -3, 14, 0, -4, 100, 100 },
    [EXPR_SMIRK]      = { 22, 22, 6, 50,  0,   0,  0,  0, 14, 0,  3,  60, 100 },
    [EXPR_AWKWARD]    = { 20, 16, 5, 50,  2,  -6,  2, -2, 14, 0,  0, 100, 100 },
    [EXPR_SHY]        = { 18,  8, 6, 46,  4,   0,  0,  0, 10, 0,  2, 100, 100 },
    [EXPR_DIZZY]      = { 22, 22, 6, 50,  0,   0,  0,  0,  8, 0,  0, 100, 100 },
    [EXPR_PROUD]      = { 24, 12, 6, 52,  0,   0,  0,  0, 16, 0,  4, 100, 100 },
    [EXPR_CRAZY]      = { 24, 24, 6, 50,  0,  -6, -4,  4, 16, 0, -3, 100, 100 },
    [EXPR_HELPLESS]   = { 22, 10, 5, 50, -2,  -7,  3, -3, 12, 0,  0, 100, 100 },
    [EXPR_SHOCKED]    = { 26, 26, 8, 48, -2,   0,  0,  0,  0, 0,  0,  60, 110 },
    [EXPR_SLEEPING]   = { 22,  2, 2, 50,  2,   0,  0,  0, 10, 0,  0, 100, 100 },
    [EXPR_BORED]      = { 22, 22, 6, 50,  0,   0,  0,  0,  0, 0,  0, 100, 100 },
    [EXPR_EXPECT]     = { 26, 24, 8, 46, -3,   0,  0,  0,  0, 0,  0, 100, 100 },
    [EXPR_COZY]       = { 24,  8, 6, 48,  3,   0,  0,  0,  0, 0,  0, 100, 100 },
    [EXPR_SIGH]       = { 22, 16, 5, 50,  3,  -7,  3, -3,  0, 0,  0, 100, 100 },
    [EXPR_DISDAIN]    = { 24,  9, 6, 50,  2,   0,  0,  0, 14, 0,  0, 100, 100 },
    [EXPR_CHEWING]    = { 24, 10, 6, 50,  2,   0,  0,  0, 12, 0,  0, 100, 100 },
    [EXPR_DISGUST]    = { 20, 14, 5, 52,  0,  -6, -3,  3, 12, 0, -3,  70, 100 },
    [EXPR_HESITANT]   = { 20, 18, 5, 50,  0,  -7,  2, -2, 10, 0,  0,  80, 100 },
};

static void set_target(ExpressionType expr) {
    const RobotExpression *p = &presets[expr];
    tgt.eye_width    = TO_FP(p->eye_width);
    tgt.eye_height   = TO_FP(p->eye_height);
    tgt.eye_spacing  = TO_FP(p->eye_spacing);
    tgt.eye_offset_y = TO_FP(p->eye_offset_y);
    tgt.brow_offset_y = TO_FP(p->brow_offset_y);
    tgt.brow_angle_l = TO_FP(p->brow_angle_l);
    tgt.brow_angle_r = TO_FP(p->brow_angle_r);
    tgt.mouth_width  = TO_FP(p->mouth_width);
    tgt.mouth_curve  = TO_FP(p->mouth_curve);
    tgt.l_eye_h_pct  = TO_FP(p->left_eye_h_pct);
    tgt.r_eye_h_pct  = TO_FP(p->right_eye_h_pct);
}

static void snap_to_target(void) { cur = tgt; }

static int16_t fp_lerp(int16_t a, int16_t b) {
    return a + (((int32_t)(b - a) * 50) >> 8);
}

static void interpolate_step(void) {
    cur.eye_width    = fp_lerp(cur.eye_width,    tgt.eye_width);
    cur.eye_height   = fp_lerp(cur.eye_height,   tgt.eye_height);
    cur.eye_spacing  = fp_lerp(cur.eye_spacing,  tgt.eye_spacing);
    cur.eye_offset_y = fp_lerp(cur.eye_offset_y, tgt.eye_offset_y);
    cur.brow_offset_y = fp_lerp(cur.brow_offset_y, tgt.brow_offset_y);
    cur.brow_angle_l = fp_lerp(cur.brow_angle_l, tgt.brow_angle_l);
    cur.brow_angle_r = fp_lerp(cur.brow_angle_r, tgt.brow_angle_r);
    cur.mouth_width  = fp_lerp(cur.mouth_width,  tgt.mouth_width);
    cur.mouth_curve  = fp_lerp(cur.mouth_curve,  tgt.mouth_curve);
    cur.l_eye_h_pct  = fp_lerp(cur.l_eye_h_pct,  tgt.l_eye_h_pct);
    cur.r_eye_h_pct  = fp_lerp(cur.r_eye_h_pct,  tgt.r_eye_h_pct);
}

static void blink_reset(void) {
    blink_wait = 40 + (rand() & 0x3F);
    blink_timer = 0;
    blink_phase = 0;
    blink_h_pct = 100;
}

static void blink_update(void) {
    blink_timer++;
    if (blink_phase == 0) {
        if (blink_timer >= blink_wait) { blink_phase = 1; blink_timer = 0; }
        blink_h_pct = 100;
    } else if (blink_phase == 1) {
        blink_h_pct = 100 - blink_timer * 45;
        if (blink_h_pct < 5) blink_h_pct = 5;
        if (blink_timer >= 2) { blink_phase = 2; blink_timer = 0; }
    } else {
        blink_h_pct = 5 + blink_timer * 45;
        if (blink_h_pct > 100) blink_h_pct = 100;
        if (blink_timer >= 2) blink_reset();
    }
}

static void draw_filled_round_rect(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (w < 6 || h < 6) {
        OLED_DrawRectangle(x, y, w, h, OLED_FILLED);
        return;
    }
    int16_t r = 3;
    OLED_DrawRectangle(x + r, y, w - r * 2, h, OLED_FILLED);
    OLED_DrawRectangle(x, y + r, r, h - r * 2, OLED_FILLED);
    OLED_DrawRectangle(x + w - r, y + r, r, h - r * 2, OLED_FILLED);
    OLED_DrawCircle(x + r, y + r, r, OLED_FILLED);
    OLED_DrawCircle(x + w - r - 1, y + r, r, OLED_FILLED);
    OLED_DrawCircle(x + r, y + h - r - 1, r, OLED_FILLED);
    OLED_DrawCircle(x + w - r - 1, y + h - r - 1, r, OLED_FILLED);
}

static void draw_heart(int16_t cx, int16_t cy, int16_t size) {
    int16_t r = size >> 2;
    if (r < 2) r = 2;
    OLED_DrawCircle(cx - r, cy - r, r, OLED_FILLED);
    OLED_DrawCircle(cx + r, cy - r, r, OLED_FILLED);
    int16_t bot = cy + (size >> 1);
    int16_t span = bot - cy;
    if (span <= 0) return;
    int16_t half = r << 1;
    for (int16_t yy = cy; yy <= bot; yy++) {
        int16_t hw = (int16_t)((int32_t)half * (bot - yy) / span);
        if (hw > 0) OLED_DrawLine(cx - hw, yy, cx + hw, yy);
    }
}

static void draw_x_eye(int16_t cx, int16_t cy, int16_t size) {
    int16_t s = size / 2;
    OLED_DrawLine(cx - s, cy - s, cx + s, cy + s);
    OLED_DrawLine(cx - s + 1, cy - s, cx + s + 1, cy + s);
    OLED_DrawLine(cx + s, cy - s, cx - s, cy + s);
    OLED_DrawLine(cx + s - 1, cy - s, cx - s - 1, cy + s);
}

static void draw_spiral_eye(int16_t cx, int16_t cy, int16_t r, uint16_t fc) {
    OLED_DrawCircle(cx, cy, r, OLED_UNFILLED);
    if (r > 4) OLED_DrawCircle(cx, cy, r - 3, OLED_UNFILLED);
    if (r > 7) OLED_DrawCircle(cx, cy, r - 6, OLED_UNFILLED);
    int8_t ox = fast_sin8((uint8_t)(fc * 6)) >> 3;
    int8_t oy = fast_sin8((uint8_t)(fc * 6 + 16)) >> 3;
    OLED_DrawCircle(cx + ox, cy + oy, 2, OLED_FILLED);
}

static void draw_tear(int16_t x, int16_t y, int16_t len) {
    OLED_DrawLine(x, y, x, y + len);
    OLED_DrawLine(x + 1, y, x + 1, y + len);
}

static void draw_blush(int16_t cx, int16_t cy) {
    OLED_DrawLine(cx - 4, cy, cx - 2, cy);
    OLED_DrawLine(cx + 2, cy, cx + 4, cy);
    OLED_DrawLine(cx - 3, cy + 1, cx - 1, cy + 1);
    OLED_DrawLine(cx + 1, cy + 1, cx + 3, cy + 1);
}

static void draw_face(void) {
    int8_t bv = fast_sin8(breath_phase);
    int16_t ew = FROM_FP(cur.eye_width);
    int16_t eh = FROM_FP(cur.eye_height);
    ew += (int16_t)((int32_t)ew * bv >> 11);
    eh += (int16_t)((int32_t)eh * bv >> 11);

    int16_t l_pct = FROM_FP(cur.l_eye_h_pct);
    int16_t r_pct = FROM_FP(cur.r_eye_h_pct);
    int16_t leh = (int16_t)((int32_t)eh * l_pct / 100);
    int16_t reh = (int16_t)((int32_t)eh * r_pct / 100);
    leh = (int16_t)((int32_t)leh * blink_h_pct / 100);
    reh = (int16_t)((int32_t)reh * blink_h_pct / 100);

    if (current_expr == EXPR_SLEEPY) {
        // 80 frames slow close, 3 frames snap wide open, 7 frames settle = 90 frame cycle
        uint16_t cyc = frame_count % 90;
        int16_t sleepy_pct;
        if (cyc < 80) {
            sleepy_pct = 100 - cyc;  // 100 -> 20 slowly
        } else if (cyc < 83) {
            sleepy_pct = 150;
        } else {
            sleepy_pct = 150 - (int16_t)((cyc - 83) * 7);
        }
        if (sleepy_pct < 20) sleepy_pct = 20;
        if (sleepy_pct > 150) sleepy_pct = 150;
        leh = (int16_t)((int32_t)leh * sleepy_pct / 100);
        reh = (int16_t)((int32_t)reh * sleepy_pct / 100);
        if (leh < 2) leh = 2;
        if (reh < 2) reh = 2;
    }

    static int16_t bored_mouth_r = 0;
    static int16_t bored_tear = 0;
    if (current_expr == EXPR_BORED) {
        // 70 frames yawn open, 10 hold, 20 close = 100 frame cycle
        int16_t cyc = frame_count % 100;
        int16_t yawn_pct;
        if (cyc < 70) {
            yawn_pct = 100 - (int16_t)(cyc * 100 / 70);
        } else if (cyc < 80) {
            yawn_pct = 0;
        } else {
            yawn_pct = (int16_t)((cyc - 80) * 5);
        }
        if (yawn_pct < 15) yawn_pct = 15;
        if (yawn_pct > 100) yawn_pct = 100;
        leh = (int16_t)((int32_t)leh * yawn_pct / 100);
        reh = (int16_t)((int32_t)reh * yawn_pct / 100);
        if (leh < 2) leh = 2;
        if (reh < 2) reh = 2;
        bored_mouth_r = (int16_t)(7 - (int32_t)7 * yawn_pct / 100);
        if (bored_mouth_r < 1) bored_mouth_r = 1;
        if (cyc >= 65 && cyc < 85) {
            bored_tear = (int16_t)(cyc - 65);
        } else {
            bored_tear = 0;
        }
    } else {
        bored_mouth_r = 0;
        bored_tear = 0;
    }

    int16_t sp = FROM_FP(cur.eye_spacing);
    int16_t oy = FROM_FP(cur.eye_offset_y);
    if (leh < 2) leh = 2;
    if (reh < 2) reh = 2;
    if (ew < 4) ew = 4;

    int16_t shake_x = 0, shake_y = 0;
    if (current_expr == EXPR_CRAZY) {
        shake_x = (frame_count & 1) ? 2 : -2;
        shake_y = (frame_count & 2) ? 1 : -1;
    }

    int16_t lx = CENTER_X - sp / 2 - ew + shake_x;
    int16_t rx = CENTER_X + sp / 2 + shake_x;
    int16_t ley = CENTER_Y + oy - leh / 2 + shake_y;
    int16_t rey = CENTER_Y + oy - reh / 2 + shake_y;

    int16_t eye_ox = 0;
    if (current_expr == EXPR_CAUTIOUS) {
        eye_ox = fast_sin8((uint8_t)(frame_count * 3)) >> 2;
    }
    if (current_expr == EXPR_HESITANT) {
        eye_ox = fast_sin8((uint8_t)(frame_count * 2)) >> 2;
    }

    if (current_expr == EXPR_LOVE && blink_h_pct > 80) {
        int16_t hs = (ew < leh) ? ew : leh;
        draw_heart(lx + ew / 2, ley + leh / 2, hs);
        draw_heart(rx + ew / 2, rey + reh / 2, hs);
    } else if (current_expr == EXPR_DIZZY) {
        draw_spiral_eye(lx + ew / 2, CENTER_Y + oy + shake_y, ew / 2, frame_count);
        draw_spiral_eye(rx + ew / 2, CENTER_Y + oy + shake_y, ew / 2, frame_count);
    } else {
        draw_filled_round_rect(lx + eye_ox, ley, ew, leh);
        draw_filled_round_rect(rx + eye_ox, rey, ew, reh);
        if (current_expr == EXPR_CAUTIOUS && leh > 6 && reh > 6) {
            int16_t pr = 3;
            int16_t plx = lx + eye_ox + ew / 2 + (eye_ox >> 1);
            int16_t prx = rx + eye_ox + ew / 2 + (eye_ox >> 1);
            OLED_ClearArea(plx - pr, ley + leh / 2 - pr, pr * 2 + 1, pr * 2 + 1);
            OLED_ClearArea(prx - pr, rey + reh / 2 - pr, pr * 2 + 1, pr * 2 + 1);
            OLED_DrawCircle(plx, ley + leh / 2, pr, OLED_FILLED);
            OLED_DrawCircle(prx, rey + reh / 2, pr, OLED_FILLED);
        }
    }

    int16_t brow_oy = FROM_FP(cur.brow_offset_y);
    if (brow_oy > 1 || brow_oy < -1) {
        int16_t lby = ley + brow_oy;
        int16_t rby = rey + brow_oy;
        int16_t bw = ew + 4;
        int16_t bal = FROM_FP(cur.brow_angle_l);
        int16_t bar = FROM_FP(cur.brow_angle_r);
        OLED_DrawLine(lx - 2, lby + bal, lx + bw - 2, lby - bal);
        OLED_DrawLine(lx - 2, lby + bal + 1, lx + bw - 2, lby - bal + 1);
        OLED_DrawLine(rx, rby + bar, rx + bw, rby - bar);
        OLED_DrawLine(rx, rby + bar + 1, rx + bw, rby - bar + 1);
    }

    if (current_expr == EXPR_CRYING) {
        int16_t tear_off = (int16_t)(frame_count % 12);
        int16_t tl_x = lx + ew / 2 - 2;
        int16_t tr_x = rx + ew / 2 + 2;
        int16_t t_y  = ley + leh + 1;
        draw_tear(tl_x, t_y + tear_off, 4);
        draw_tear(tr_x, t_y + tear_off, 4);
        if (tear_off > 5) {
            draw_tear(tl_x + 3, t_y + tear_off - 5, 3);
            draw_tear(tr_x - 3, t_y + tear_off - 5, 3);
        }
    }

    if (current_expr == EXPR_SHY) {
        draw_blush(lx + ew / 2, ley + leh + 3);
        draw_blush(rx + ew / 2, rey + reh + 3);
    }

    if (current_expr == EXPR_AWKWARD) {
        int16_t sw_x = rx + ew + 4;
        int16_t sw_y0 = rey - 2;
        int16_t drop = (int16_t)(frame_count % 16);
        OLED_DrawCircle(sw_x, sw_y0, 2, OLED_FILLED);
        draw_tear(sw_x, sw_y0 + 2 + drop, 3);
    }

    if (current_expr == EXPR_CRAZY) {
        // anger symbol 井 at top-right of forehead
        int16_t ax = rx + ew + 2;
        int16_t ay = rey - 6;
        // two horizontal lines
        OLED_DrawLine(ax, ay,     ax + 8, ay);
        OLED_DrawLine(ax, ay + 1, ax + 8, ay + 1);
        OLED_DrawLine(ax, ay + 4, ax + 8, ay + 4);
        OLED_DrawLine(ax, ay + 5, ax + 8, ay + 5);
        // two vertical lines
        OLED_DrawLine(ax + 2, ay - 2, ax + 2, ay + 7);
        OLED_DrawLine(ax + 3, ay - 2, ax + 3, ay + 7);
        OLED_DrawLine(ax + 6, ay - 2, ax + 6, ay + 7);
        OLED_DrawLine(ax + 7, ay - 2, ax + 7, ay + 7);
    }

    int16_t mw = FROM_FP(cur.mouth_width);
    int16_t mc = FROM_FP(cur.mouth_curve);
    int16_t my = CENTER_Y + 18 + shake_y;
    int16_t mx = CENTER_X - mw / 2 + shake_x;

    if (current_expr == EXPR_SURPRISED && mw == 0) {
        OLED_DrawCircle(CENTER_X, my, 5, OLED_UNFILLED);
    } else if (current_expr == EXPR_BORED && bored_mouth_r > 0) {
        OLED_DrawCircle(CENTER_X + shake_x, my + shake_y, bored_mouth_r, OLED_UNFILLED);
    } else if (current_expr == EXPR_SHOCKED && mw == 0) {
        int16_t tw = 10, th = 8;
        OLED_DrawLine(CENTER_X, my, CENTER_X - tw / 2, my + th);
        OLED_DrawLine(CENTER_X, my, CENTER_X + tw / 2, my + th);
        OLED_DrawLine(CENTER_X - tw / 2, my + th, CENTER_X + tw / 2, my + th);
    } else if (current_expr == EXPR_EXPECT && mw == 0) {
        int16_t tw = 10, th = 7;
        OLED_DrawLine(CENTER_X - tw / 2, my, CENTER_X + tw / 2, my);
        OLED_DrawLine(CENTER_X - tw / 2, my, CENTER_X, my + th);
        OLED_DrawLine(CENTER_X + tw / 2, my, CENTER_X, my + th);
    } else if (current_expr == EXPR_COZY && mw == 0) {
        int16_t tw = 8, th = 5;
        OLED_DrawLine(CENTER_X - tw / 2, my, CENTER_X + tw / 2, my);
        OLED_DrawLine(CENTER_X - tw / 2, my, CENTER_X, my + th);
        OLED_DrawLine(CENTER_X + tw / 2, my, CENTER_X, my + th);
    } else if (current_expr == EXPR_SIGH && mw == 0) {
        OLED_DrawCircle(CENTER_X, my + 2, 4, OLED_UNFILLED);
    } else if (current_expr == EXPR_DISDAIN) {
        int16_t amp = 2 + (fast_sin8((uint8_t)(frame_count * 8)) > 0 ? 1 : -1);
        int16_t ww = mw;
        int16_t wx = CENTER_X - ww / 2;
        OLED_DrawLine(wx, my, wx + ww / 4, my - amp);
        OLED_DrawLine(wx + ww / 4, my - amp, wx + ww / 2, my + amp);
        OLED_DrawLine(wx + ww / 2, my + amp, wx + ww * 3 / 4, my - amp);
        OLED_DrawLine(wx + ww * 3 / 4, my - amp, wx + ww, my);
    } else if (current_expr == EXPR_CHEWING) {
        int16_t chew_phase = fast_sin8((uint8_t)(frame_count * 5));
        int16_t open = 2 + (chew_phase > 0 ? chew_phase >> 3 : 0);
        OLED_DrawLine(mx, my, mx + mw, my);
        OLED_DrawLine(mx, my + open, mx + mw, my + open);
        OLED_DrawLine(mx, my, mx, my + open);
        OLED_DrawLine(mx + mw, my, mx + mw, my + open);
        // food crumbs inside mouth when closed
        if (open <= 3) {
            OLED_DrawPoint(mx + 3, my + 1);
            OLED_DrawPoint(mx + mw - 3, my + 1);
            OLED_DrawPoint(mx + mw / 2, my + 1);
        }
        // food piece approaching from right side
        uint16_t fcyc = frame_count % 30;
        int16_t food_x = mx + mw + 12 - (int16_t)(fcyc * 12 / 30);
        int16_t food_y = my + 1;
        if (fcyc < 25) {
            OLED_DrawRectangle(food_x, food_y, 3, 3, OLED_FILLED);
        }
    } else if (current_expr == EXPR_HELPLESS) {
        int16_t hmy = my - 4;
        int16_t tw = 8, th = 5;
        OLED_DrawLine(CENTER_X - tw / 2, hmy, CENTER_X + tw / 2, hmy);
        OLED_DrawLine(CENTER_X - tw / 2, hmy, CENTER_X, hmy + th);
        OLED_DrawLine(CENTER_X + tw / 2, hmy, CENTER_X, hmy + th);
    } else if (current_expr == EXPR_AWKWARD) {
        OLED_DrawLine(mx, my, mx + mw, my);
        int16_t seg = mw / 4;
        for (int16_t i = 1; i < 4; i++) {
            int16_t sx = mx + seg * i;
            int16_t dy = (i & 1) ? -2 : 2;
            OLED_DrawLine(sx, my, sx, my + dy);
        }
    } else if (current_expr == EXPR_HESITANT) {
        int16_t mox = fast_sin8((uint8_t)(frame_count * 2)) >> 3;
        OLED_DrawLine(mx + mox, my, mx + mw / 3 + mox, my + 2);
        OLED_DrawLine(mx + mw / 3 + mox, my + 2, mx + mw * 2 / 3 + mox, my - 2);
        OLED_DrawLine(mx + mw * 2 / 3 + mox, my - 2, mx + mw + mox, my);
    } else if (current_expr == EXPR_DIZZY) {
        int16_t ww = 14;
        int16_t wx = CENTER_X - ww / 2;
        int16_t amp = 3;
        OLED_DrawLine(wx, my, wx + ww / 4, my - amp);
        OLED_DrawLine(wx + ww / 4, my - amp, wx + ww / 2, my + amp);
        OLED_DrawLine(wx + ww / 2, my + amp, wx + ww * 3 / 4, my - amp);
        OLED_DrawLine(wx + ww * 3 / 4, my - amp, wx + ww, my);
    } else if (current_expr == EXPR_PROUD) {
        OLED_DrawLine(mx, my, mx + mw / 3, my + mc);
        OLED_DrawLine(mx + mw / 3, my + mc, mx + mw * 2 / 3, my + mc);
        OLED_DrawLine(mx + mw * 2 / 3, my + mc, mx + mw, my);
        OLED_DrawPoint(mx + mw + 2, my - 1);
        OLED_DrawPoint(mx + mw + 3, my - 2);
    } else if (mc != 0) {
        OLED_DrawLine(mx, my, mx + mw / 3, my + mc);
        OLED_DrawLine(mx + mw / 3, my + mc, mx + mw * 2 / 3, my + mc);
        OLED_DrawLine(mx + mw * 2 / 3, my + mc, mx + mw, my);
    } else {
        OLED_DrawLine(mx, my, mx + mw, my);
    }

    if (current_expr == EXPR_SMIRK) {
        int16_t smx = mx + mw;
        OLED_DrawLine(smx, my, smx + 2, my - 2);
    }

    if (current_expr == EXPR_BORED && bored_tear > 0) {
        int16_t tx = rx + ew - 2;
        int16_t ty = rey + reh + 1;
        draw_tear(tx, ty, bored_tear / 2);
        OLED_DrawCircle(tx, ty + bored_tear / 2 + 1, 1, OLED_FILLED);
    }

    if (current_expr == EXPR_SIGH) {
        // breath puff drifting down-right from mouth
        uint16_t scyc = frame_count % 40;
        if (scyc < 30) {
            int16_t px = CENTER_X + 6 + scyc / 2;
            int16_t py = my + 4 + scyc / 3;
            int16_t pr = 2 + scyc / 6;
            OLED_DrawCircle(px, py, pr, OLED_UNFILLED);
            if (scyc > 8) {
                int16_t px2 = CENTER_X + 4 + (scyc - 8) / 2;
                int16_t py2 = my + 3 + (scyc - 8) / 3;
                OLED_DrawCircle(px2, py2, 1 + (scyc - 8) / 8, OLED_UNFILLED);
            }
        }
    }

    if (current_expr == EXPR_SLEEPY) {
        // drool drops only at the wake-up snap moment (frame 80-89 of 90-cycle)
        uint16_t dcyc = frame_count % 90;
        if (dcyc >= 80) {
            int16_t dx = mx + mw + 2;
            int16_t dy = my + 1;
            int16_t drip = (int16_t)(dcyc - 80);
            OLED_DrawCircle(dx, dy, 1, OLED_FILLED);
            draw_tear(dx, dy + 2, drip);
            OLED_DrawCircle(dx, dy + 2 + drip, 1, OLED_FILLED);
        }
    }

    if (current_expr == EXPR_SLEEPING) {
        // nose bubble: inflates and deflates
        int16_t bx = mx + mw + 3;
        int16_t by = my + 1;
        uint16_t bcyc = frame_count % 40;
        int16_t br;
        if (bcyc < 30) {
            br = 2 + bcyc / 5;  // inflate: 2 -> 8
        } else {
            br = 8 - (bcyc - 30); // deflate: 8 -> 0
        }
        if (br > 0) OLED_DrawCircle(bx + br, by, br, OLED_UNFILLED);

        // floating ZZZ: 3 Z's at different sizes drifting up-right
        uint16_t zcyc = frame_count % 60;
        int16_t zx = rx + ew + 4;
        int16_t zy = rey - 4;
        // Z1: small, lowest
        int16_t z1y = zy - (int16_t)(zcyc / 4);
        int16_t z1x = zx + zcyc / 6;
        if (zcyc < 50) {
            OLED_ShowString(z1x, z1y, "z", OLED_6X8_HALF);
        }
        // Z2: medium, middle
        int16_t z2phase = (zcyc + 20) % 60;
        int16_t z2y = zy - 6 - (int16_t)(z2phase / 4);
        int16_t z2x = zx + 4 + z2phase / 6;
        if (z2phase < 50) {
            OLED_ShowString(z2x, z2y, "Z", OLED_6X8_HALF);
        }
        // Z3: large, highest
        int16_t z3phase = (zcyc + 40) % 60;
        int16_t z3y = zy - 12 - (int16_t)(z3phase / 4);
        int16_t z3x = zx + 8 + z3phase / 6;
        if (z3phase < 50) {
            OLED_ShowString(z3x, z3y, "Z", OLED_7X12_HALF);
        }
    }

    if (current_expr == EXPR_HELPLESS) {
        int16_t sy = my + 4;
        // left: shrug arm  ¯\_(ツ)_/¯
        OLED_DrawLine(CENTER_X - 14, sy, CENTER_X - 24, sy + 6);
        OLED_DrawLine(CENTER_X - 14, sy + 1, CENTER_X - 24, sy + 7);
        OLED_DrawLine(CENTER_X - 24, sy + 6, CENTER_X - 32, sy);
        OLED_DrawLine(CENTER_X - 24, sy + 7, CENTER_X - 32, sy + 1);
        OLED_DrawLine(CENTER_X - 36, sy - 1, CENTER_X - 30, sy - 1);
        OLED_DrawLine(CENTER_X - 36, sy, CENTER_X - 30, sy);
        // right: mirror
        OLED_DrawLine(CENTER_X + 14, sy, CENTER_X + 24, sy + 6);
        OLED_DrawLine(CENTER_X + 14, sy + 1, CENTER_X + 24, sy + 7);
        OLED_DrawLine(CENTER_X + 24, sy + 6, CENTER_X + 32, sy);
        OLED_DrawLine(CENTER_X + 24, sy + 7, CENTER_X + 32, sy + 1);
        OLED_DrawLine(CENTER_X + 30, sy - 1, CENTER_X + 36, sy - 1);
        OLED_DrawLine(CENTER_X + 30, sy, CENTER_X + 36, sy);
    }
    // 取消显示图片时的表情名称展示
    //OLED_ShowString(CENTER_X - 21, 56, (char *)expr_names[current_expr], OLED_6X8_HALF);
}

static void handle_input(void) {
    if (key_menu.back == PRESS) {
        key_menu.back = RELEASE;
        exit_requested = true;
        return;
    }
    if (key_menu.up == PRESS) {
        key_menu.up = RELEASE;
        current_expr = (current_expr == 0) ? EXPR_COUNT - 1 : current_expr - 1;
        set_target(current_expr);
    }
    if (key_menu.down == PRESS) {
        key_menu.down = RELEASE;
        current_expr = (current_expr + 1 >= EXPR_COUNT) ? 0 : current_expr + 1;
        set_target(current_expr);
    }
    if (key_menu.enter == PRESS) {
        key_menu.enter = RELEASE;
        current_expr = (current_expr + 1 >= EXPR_COUNT) ? 0 : current_expr + 1;
        set_target(current_expr);
    }
}

void app_robot_face_init(void) {
    exit_requested = false;
    current_expr = EXPR_NORMAL;
    frame_count = 0;
    breath_phase = 0;
    set_target(EXPR_NORMAL);
    snap_to_target();
    blink_reset();
    srand(12345);
    OLED_Clear();
    OLED_Update();
}

void app_robot_face_tick(void) {
    handle_input();
    if (exit_requested) return;

    frame_count++;
    breath_phase += 4;

    interpolate_step();
    blink_update();

    OLED_Clear();
    draw_face();
    OLED_Update();
}

bool app_robot_face_should_exit(void) {
    return exit_requested;
}

void app_robot_face_on_exit(void) {
    OLED_Clear();
    OLED_Update();
}
