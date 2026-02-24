#include "bsp/accel.h"

void ACCEL_Init(void)
{
    // No initialization needed for the "none" implementation
}

bool ACCEL_SetupSensor(void)
{
    // No sensor to set up in the "none" implementation
    return false;
}

bool ACCEL_ReadData(accel_data_t* data)
{
    // No data to read in the "none" implementation
    return false;
}
