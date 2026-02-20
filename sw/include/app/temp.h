#ifndef APP_TEMP_H
#define APP_TEMP_H

#include <stdint.h>

typedef enum {
    temp_status_not_measured = 0,
    temp_status_ok = 1,
    temp_status_error = 2
} temp_status_t;

void TEMP_Init(void);

void TEMP_Update100ms(void);

int8_t TEMP_GetDriveTemperature(void);

int8_t TEMP_GetMcuTemperature(void);

temp_status_t TEMP_GetStatus(void);

#endif // APP_TEMP_H
