#ifndef APP_CALIB_PRIV_H
#define APP_CALIB_PRIV_H

#include "app/calib.h"

#include <stdint.h>

#define CALIB_ATTR __attribute__((section(".calib_data")))
#define CALIB_FLASH_ADDRESS 0x3F200u
#define CALIB_FLASH_SIZE (64u * 4u)

typedef struct __attribute__((packed)){
    uint16_t Voltage_Slope_Calib;
    int16_t Voltage_Slope_Offset;
    int16_t Imu_Accel_Calib_X;
    int16_t Imu_Accel_Calib_Y;
    int16_t Imu_Accel_Calib_Z;
} calib_properties_v1_t;

typedef struct __attribute__((packed)){
    uint8_t Version;
    calib_properties_v1_t Properties;
    uint8_t Padding[CALIB_FLASH_SIZE - sizeof(uint8_t) - sizeof(calib_properties_v1_t) - sizeof(uint32_t)];
    uint32_t Crc32;
} calib_layout_v1_t;

typedef struct __attribute__((packed)){
    uint8_t Version;
    uint8_t Padding[CALIB_FLASH_SIZE - sizeof(uint8_t) - sizeof(uint32_t)];
    uint32_t Crc32;
} calib_layout_generic_t;

#endif // APP_CALIB_PRIV_H
