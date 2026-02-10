#ifndef BSP_ACCEL_H
#define BSP_ACCEL_H

#include "bma456mm.h"

extern struct bma4_dev ACCEL_BMA456_Device;

void ACCEL_Initialize(void);

#endif // BSP_ACCEL_H