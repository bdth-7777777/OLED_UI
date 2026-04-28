/* 中层 IMU 姿态解算 — 基于 x-io Technologies Fusion AHRS */
#include "mid_imu.h"
#include "mid_timer.h"
#include "FusionAhrs.h"
#include "FusionOffset.h"
#include <string.h>
#include <math.h>

Angle angle_new;

static uint32_t last_tick_ms = 0;

static FusionOffset fusionOffset;
static FusionAhrs   fusionAhrs;
static uint8_t      yaw_zeroed = 0;
static unsigned int post_init_count = 0;

static float acc[3] = {0, 0, 0};
static float gyr[3] = {0, 0, 0};

static void reverse(char *str, int len)
{
    int i;
    for (i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

static void int_to_str(int num, char *str)
{
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
    } else {
        while (num > 0) {
            str[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    str[i] = '\0';
    reverse(str, i);
}

void mid_imu_float_to_string(float num, char *str)
{
    char int_str[20];
    char dec_str[7];

    int str_index = 0;

    if (num < 0) {
        str[str_index++] = '-';
        num = -num;
    }

    float num_abs = num;
    int int_part = (int)num_abs;
    float decimal_part = num_abs - (float)int_part;

    int_to_str(int_part, int_str);
    int int_len = strlen(int_str);
    memcpy(str + str_index, int_str, int_len);
    str_index += int_len;

    int i;
    for (i = 0; i < 2; i++) {
        decimal_part *= 10.0f;
        int digit = (int)decimal_part;
        dec_str[i] = '0' + digit;
        decimal_part -= (float)digit;
    }
    dec_str[2] = '\0';

    int decimal_len = 2;
    while (decimal_len > 0 && dec_str[decimal_len - 1] == '0') {
        decimal_len--;
    }

    if (decimal_len > 0) {
        str[str_index++] = '.';
        memcpy(str + str_index, dec_str, decimal_len);
        str_index += decimal_len;
    }

    str[str_index] = '\0';
}

void mid_imu_get_angle(Angle *angle)
{
    int i;
    uint8_t status;
    float deltaTime;
    uint32_t now_ms;
    FusionVector gyroscope;
    FusionVector accelerometer;
    FusionEuler euler;

    status = lsm6ds3_get_status();
    if (!((status & LSM6DS3TRC_STATUS_ACCELEROMETER) &&
          (status & LSM6DS3TRC_STATUS_GYROSCOPE))) {
        return;
    }

    now_ms = mid_get_sys_tick_ms();
    deltaTime = (float)(now_ms - last_tick_ms) / 1000.0f;
    if (deltaTime <= 0.0f || deltaTime > 0.5f) deltaTime = 0.02f;
    last_tick_ms = now_ms;

    lsm6ds3_get_acceleration(LSM6DS3TRC_ACC_FSXL_2G, acc);
    for (i = 0; i < 3; i++) acc[i] /= 1000.0f;

    lsm6ds3_get_gyroscope(LSM6DS3TRC_GYR_FSG_2000, gyr);
    for (i = 0; i < 3; i++) gyr[i] /= 1000.0f;

    gyroscope.axis.x = gyr[0];
    gyroscope.axis.y = gyr[1];
    gyroscope.axis.z = gyr[2];
    gyroscope = FusionOffsetUpdate(&fusionOffset, gyroscope);

    accelerometer.axis.x = acc[0];
    accelerometer.axis.y = acc[1];
    accelerometer.axis.z = acc[2];

    FusionAhrsUpdateNoMagnetometer(&fusionAhrs, gyroscope, accelerometer, deltaTime);

    euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&fusionAhrs));
    angle->x = euler.angle.roll;
    angle->y = -euler.angle.pitch;
    angle->z = -euler.angle.yaw;

    if (!fusionAhrs.initialising && !yaw_zeroed) {
        post_init_count++;
        if (post_init_count >= 70) {
            yaw_zeroed = 1;
            FusionAhrsSetHeading(&fusionAhrs, 0.0f);
        }
    }
}

void mid_imu_angle_return_zero(void)
{
    angle_new.x = 0;
    angle_new.y = 0;
    angle_new.z = 0;
    FusionAhrsReset(&fusionAhrs);
    yaw_zeroed = 0;
    post_init_count = 0;
    lsm6ds3_reset();
}

unsigned char mid_imu_init(void)
{
    if (lsm6ds3_hw_init() != 0) return 1;

    FusionOffsetInitialise(&fusionOffset, 16);
    FusionAhrsInitialise(&fusionAhrs);
    yaw_zeroed = 0;
    post_init_count = 0;

    const FusionAhrsSettings settings = {
        .convention = FusionConventionNwu,
        .gain = 0.5f,
        .gyroscopeRange = 2000.0f,
        .accelerationRejection = 10.0f,
        .magneticRejection = 0.0f,
        .recoveryTriggerPeriod = 5 * 16,
    };
    FusionAhrsSetSettings(&fusionAhrs, &settings);

    last_tick_ms = mid_get_sys_tick_ms();
    return 0;
}
