#ifndef BSP_ACCEL_H
#define BSP_ACCEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_data_t;

void ACCEL_Init(void);

bool ACCEL_SetupSensor(void);

bool ACCEL_ReadData(accel_data_t* data);

#endif // BSP_ACCEL_H
