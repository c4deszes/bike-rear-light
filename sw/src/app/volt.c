#include "app/volt.h"

static uint16_t VOLT_Voltage;
static volt_status_t VOLT_Status;

void VOLT_Init(void) {
    VOLT_Voltage = 0;
    VOLT_Status = volt_status_ok;
}

void VOLT_Update100ms(void) {
    // TODO: measure voltage and update status accordingly
}

uint16_t VOLT_GetVoltage(void) {
    return VOLT_Voltage;
}

volt_status_t VOLT_GetStatus(void) {
    return VOLT_Status;
}
