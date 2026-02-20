#ifndef APP_VOLT_H
#define APP_VOLT_H_

#include <stdint.h>

typedef enum {
    volt_status_ok,
    volt_status_low,
    volt_status_critical
} volt_status_t;

void VOLT_Init(void);

void VOLT_Update100ms(void);

uint16_t VOLT_GetVoltage(void);

volt_status_t VOLT_GetStatus(void);

#endif // APP_VOLT_H_
