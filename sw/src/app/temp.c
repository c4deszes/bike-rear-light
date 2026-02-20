#include "app/temp.h"

static int8_t TEMP_DriveTemperature;
static int8_t TEMP_McuTemperature;
static temp_status_t TEMP_Status;

void TEMP_Init(void) {
    TEMP_DriveTemperature = 0;
    TEMP_McuTemperature = 0;
    TEMP_Status = temp_status_not_measured;
}

void TEMP_Update100ms(void) {
    // TODO: implement
}

int8_t TEMP_GetDriveTemperature(void) {
    return TEMP_DriveTemperature;
}

int8_t TEMP_GetMcuTemperature(void) {
    return TEMP_McuTemperature;
}

temp_status_t TEMP_GetStatus(void) {
    return TEMP_Status;
}
