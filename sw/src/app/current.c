#include "app/current.h"

static uint16_t CURRENT_Estimate;

void CURRENT_Init(void) {
    CURRENT_Estimate = 0;
}

void CURRENT_Update100ms(void) {
    // TODO: update current estimate based on brightness and other factors
}

uint16_t CURRENT_GetCurrent(void) {
    return CURRENT_Estimate;
}
