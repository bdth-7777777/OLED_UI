/**
 * mid_imu.h
 * IMU sensor fusion interface — AHRS-based attitude estimation using LSM6DS3TR-C.
 */
#ifndef __MID_IMU_H__
#define __MID_IMU_H__

#include "hw_lsm6ds3.h"

extern Angle angle_new;

unsigned char mid_imu_init(void);
void mid_imu_get_angle(Angle *angle);
void mid_imu_angle_return_zero(void);
void mid_imu_float_to_string(float num, char *str);

#endif
