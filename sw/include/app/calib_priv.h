#ifndef APP_CALIB_PRIV_H
#define APP_CALIB_PRIV_H

#include "app/calib.h"

#include <stdint.h>

#define CALIB_ATTR __attribute__((section(".calib_data")))
#define CALIB_FLASH_ADDRESS 0x3F200u
#define CALIB_FLASH_SIZE (64u * 4u)

typedef struct {
    uint8_t brightness_curve[CALIB_BRIGHTNESS_RANGE];
    uint16_t voltage_calib;
    uint8_t padding[CALIB_FLASH_SIZE - CALIB_BRIGHTNESS_RANGE - sizeof(uint16_t) - sizeof(uint32_t)];
    uint32_t crc32;
} calib_memlayout_t;

#endif // APP_CALIB_PRIV_H
