#ifndef APP_CALIB_H
#define APP_CALIB_H

#include <stdint.h>

#define CALIB_BRIGHTNESS_RANGE 100
#define CALIB_BRIGHTNESS_MAXDEV 20

#define CALIB_VOLTAGE_MAX 3300

void CALIB_Init(void);

uint8_t CALIB_MapBrightnessValue(uint8_t value);

uint16_t CALIB_GetVoltageCalib(void);

#endif
