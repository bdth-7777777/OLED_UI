/**
 * app_gyroscope.c
 * 3D cube display driven by LSM6DS3 gyroscope angles
 */

#include "app_gyroscope.h"
#include "OLED.h"
#include "OLED_UI.h"
#include "mid_imu.h"
#include "hw_delay.h"
#include "mid_timer.h"
#include <math.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define CUBE_CX  88   // cube center X on screen (right half)
#define CUBE_CY  32   // cube center Y on screen
#define CUBE_SIZE 18.0f  // half-edge length in pixels
#define DEG2RAD  0.017453f

// 8 vertices of a unit cube, scaled by CUBE_SIZE
static const float cube_v[8][3] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
};

// 12 edges (vertex index pairs)
static const uint8_t cube_e[12][2] = {
    {0,1},{1,2},{2,3},{3,0},  // back face
    {4,5},{5,6},{6,7},{7,4},  // front face
    {0,4},{1,5},{2,6},{3,7}   // connecting edges
};

static bool exit_requested = false;
static Angle gyro_angle;
static char  gyro_buf[16];

static void rotate_project(float vx, float vy, float vz,
                           float sx, float cx, float sy, float cy, float sz, float cz,
                           int16_t *ox, int16_t *oy)
{
    // Rz * Ry * Rx
    float x1 = vx;
    float y1 = vy * cx - vz * sx;
    float z1 = vy * sx + vz * cx;

    float x2 = x1 * cy + z1 * sy;
    float y2 = y1;
    float z2 = -x1 * sy + z1 * cy;

    float x3 = x2 * cz - y2 * sz;
    float y3 = x2 * sz + y2 * cz;
    float z3 = z2;

    // simple perspective: distance = 80
    float d = 80.0f;
    float scale = d / (d + z3);
    *ox = (int16_t)(CUBE_CX + x3 * scale);
    *oy = (int16_t)(CUBE_CY + y3 * scale);
}

static void draw_cube(float ax, float ay, float az)
{
    float sx = sinf(ax * DEG2RAD);
    float cx = cosf(ax * DEG2RAD);
    float sy = sinf(ay * DEG2RAD);
    float cy = cosf(ay * DEG2RAD);
    float sz = sinf(az * DEG2RAD);
    float cz = cosf(az * DEG2RAD);

    int16_t px[8], py[8];
    int i;

    for (i = 0; i < 8; i++) {
        rotate_project(cube_v[i][0] * CUBE_SIZE,
                       cube_v[i][1] * CUBE_SIZE,
                       cube_v[i][2] * CUBE_SIZE,
                       sx, cx, sy, cy, sz, cz,
                       &px[i], &py[i]);
    }

    for (i = 0; i < 12; i++) {
        OLED_DrawLine(px[cube_e[i][0]], py[cube_e[i][0]],
                      px[cube_e[i][1]], py[cube_e[i][1]]);
    }
}

void app_gyroscope_request_exit(void)
{
    exit_requested = true;
}

bool app_gyroscope_should_exit(void)
{
    return exit_requested;
}

static void render_frame(Angle *angle, char *buf)
{
    OLED_Clear();
    draw_cube(angle->x, angle->y, angle->z);

    mid_imu_float_to_string(angle->x, buf);
    OLED_ShowString(0, 4, "X:", OLED_6X8_HALF);
    OLED_ShowString(12, 4, buf, OLED_6X8_HALF);

    mid_imu_float_to_string(angle->y, buf);
    OLED_ShowString(0, 24, "Y:", OLED_6X8_HALF);
    OLED_ShowString(12, 24, buf, OLED_6X8_HALF);

    mid_imu_float_to_string(angle->z, buf);
    OLED_ShowString(0, 44, "Z:", OLED_6X8_HALF);
    OLED_ShowString(12, 44, buf, OLED_6X8_HALF);
}

void app_gyroscope_init(void)
{
    exit_requested = false;
    mid_imu_init();
    angle_new.x = 0;
    angle_new.y = 0;
    angle_new.z = 0;
    gyro_angle.x = 0;
    gyro_angle.y = 0;
    gyro_angle.z = 0;
}

void app_gyroscope_sample(void)
{
    mid_imu_get_angle(&angle_new);
}

void app_gyroscope_tick(void)
{
    gyro_angle = angle_new;
    render_frame(&gyro_angle, gyro_buf);
    OLED_Update();
}

void app_gyroscope_fade_tick(int8_t level)
{
    mid_imu_get_angle(&angle_new);
    gyro_angle = angle_new;
    render_frame(&gyro_angle, gyro_buf);
    OLED_UI_FadeOut_Masking(0, 0, 128, 64, level);
    OLED_Update();
}

void app_gyroscope_on_exit(void)
{
    OLED_Clear();
    OLED_Update();
}

void app_gyroscope_loop(void)
{
    Angle angle;
    char buf[16];
    int8_t i;
    exit_requested = false;

    // init sensor and zero angles
    mid_imu_init();
    angle_new.x = 0;
    angle_new.y = 0;
    angle_new.z = 0;

    // fade-in: masking level 5 -> 1 (heavy mask to clear)
    for (i = 5; i >= 1; i--) {
        mid_imu_get_angle(&angle_new);
        angle = angle_new;
        render_frame(&angle, buf);
        OLED_UI_FadeOut_Masking(0, 0, 128, 64, i);
        OLED_Update();
        delay_ms(FADEOUT_TIME);
    }

    while (1) {
        // 在渲染间隙多次采样 IMU，提高姿态积分精度
        {
            uint32_t frame_start = mid_get_sys_tick_ms();
            while ((mid_get_sys_tick_ms() - frame_start) < 20) {
                mid_imu_get_angle(&angle_new);
            }
        }
        angle = angle_new;

        render_frame(&angle, buf);
        OLED_Update();

        if (app_gyroscope_should_exit()) {
            // fade-out: masking level 1 -> 5 (clear to heavy mask)
            for (i = 1; i <= 5; i++) {
                mid_imu_get_angle(&angle_new);
                angle = angle_new;
                render_frame(&angle, buf);
                OLED_UI_FadeOut_Masking(0, 0, 128, 64, i);
                OLED_Update();
                delay_ms(FADEOUT_TIME);
            }
            OLED_Clear();
            OLED_Update();
            break;
        }
    }
}
