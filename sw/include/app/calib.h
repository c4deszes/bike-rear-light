#ifndef APP_CALIB_H
#define APP_CALIB_H

#include <stdint.h>
#include <stdbool.h>

#define CALIB_BRIGHTNESS_RANGE 100
#define CALIB_BRIGHTNESS_MAXDEV 20

void CALIB_Init(void);

void CALIB_Save(void);

bool CALIB_GetVoltageCalib(uint16_t* slope_calib, int16_t* offset_calib);

bool CALIB_SetVoltageCalib(uint16_t slope_calib, int16_t offset_calib);

bool CALIB_GetImuAccelCalib(int16_t* x_calib, int16_t* y_calib, int16_t* z_calib);

bool CALIB_SetImuAccelCalib(int16_t x_calib, int16_t y_calib, int16_t z_calib);

#endif
