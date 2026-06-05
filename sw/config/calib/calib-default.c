#include "app/calib_priv.h"

//#include "bsp/volt_cfg.h"

CALIB_ATTR const calib_layout_v1_t calib_default = {
    .Version = 1,
    .Properties = {
        .Voltage_Slope_Calib = 0, // Invalid calib value will trigger fallback to default behavior in voltage mapping
        .Voltage_Slope_Offset = 0,
        .Imu_Accel_Calib_X = 0,
        .Imu_Accel_Calib_Y = 0,
        .Imu_Accel_Calib_Z = 0
    },
    .Padding = {[0 ... sizeof(calib_default.Padding) - 1] = 0xFF},
    .Crc32 = 0 // Will be calculated and filled in by the post-build step
};

