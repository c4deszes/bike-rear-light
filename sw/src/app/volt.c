#include "app/volt.h"

#include "bsp/tt_adc.h"
#include "bsp/tt_adc_cfg.h"

#include "app/calib.h"

#include "app/feature.h"

#define VOLT_MEAS_REF 33 // Reference voltage
#define VOLT_MEAS_RES 1024 // ADC resolution

static uint16_t VOLT_VbatAdcValue;
static uint16_t VOLT_Voltage;
static volt_status_t VOLT_Status;
static uint16_t VOLT_VbatTempTimeout;

static uint16_t VOLT_CalibSlope;
static int16_t VOLT_CalibOffset;

void VOLT_Init(void) {
    VOLT_VbatAdcValue = 0;
    VOLT_Voltage = 0;
    VOLT_Status = volt_status_not_measured;
    VOLT_VbatTempTimeout = 0;

    CALIB_GetVoltageCalib(&VOLT_CalibSlope, &VOLT_CalibOffset);
}

void VOLT_Update100ms(void) {
#if defined(TTADC_CHANNEL_VBAT)
    if (TTADC_ResultReady(TTADC_CHANNEL_VBAT)) {
        VOLT_VbatAdcValue = TTADC_GetResult(TTADC_CHANNEL_VBAT);
        VOLT_Voltage = (VOLT_VbatAdcValue * VOLT_MEAS_REF * VOLT_CalibSlope) / (VOLT_MEAS_RES * 100) + VOLT_CalibOffset;

        if (VOLT_Voltage < FEATURE_VOLT_VBAT_ERROR_LOW || VOLT_Voltage > FEATURE_VOLT_VBAT_ERROR_HIGH) {
            VOLT_Status = volt_status_error;
        }
        else if (VOLT_Voltage < FEATURE_VOLT_VBAT_LOW_THRESHOLD) {
            VOLT_Status = volt_status_low;
        }
        else {
            VOLT_Status = volt_status_ok;
        }

        VOLT_VbatTempTimeout = 0;
    }
    else if(VOLT_VbatTempTimeout < FEATURE_VOLT_VBAT_TIMEOUT) {
        VOLT_VbatTempTimeout += 100;
    }
    else {
        VOLT_Status = volt_status_error;
    }
#endif
}

uint16_t VOLT_GetVoltage(void) {
    return VOLT_Voltage;
}

uint16_t VOLT_GetAdcRawValue(void) {
    return VOLT_VbatAdcValue;
}

volt_status_t VOLT_GetStatus(void) {
    return VOLT_Status;
}
