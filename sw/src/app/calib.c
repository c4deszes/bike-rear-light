#include "app/calib.h"
#include "app/calib_priv.h"

#include "app/feature.h"

#include "hal/dsu.h"
#include "hal/nvmctrl.h"

static calib_memlayout_t* CALIB_Memory = (calib_memlayout_t*)CALIB_FLASH_ADDRESS;
uint32_t CALIB_CalculatedCrc;

static uint8_t CALIB_BrightnessCurve[CALIB_BRIGHTNESS_RANGE] = {0};

static void CALIB_InitBrightnessCurve(void) {
    for (uint8_t i = 0; i < CALIB_BRIGHTNESS_RANGE; i++) {
        CALIB_BrightnessCurve[i] = i;
    }
}

static bool CALIB_ValidateBrightnessCurve(void) {
    for (uint8_t i = 0; i < CALIB_BRIGHTNESS_RANGE; i++) {
        uint8_t point = CALIB_BrightnessCurve[i];
        if (point > i + CALIB_BRIGHTNESS_MAXDEV || point < i - CALIB_BRIGHTNESS_MAXDEV) {
            return false;
        }
    }
    return true;
}

static void CALIB_CopyBrightnessCurveToMemory(void) {
    for (uint8_t i = 0; i < CALIB_BRIGHTNESS_RANGE; i++) {
        CALIB_Memory->brightness_curve[i] = CALIB_BrightnessCurve[i];
    }
}

static void CALIB_LoadNvram(void) {
    CALIB_CalculatedCrc = DSU_CalculateCRC32(0xFFFFFFFFUL,
                                              (void*)CALIB_FLASH_ADDRESS,
                                              CALIB_FLASH_SIZE-sizeof(uint32_t));

    if (CALIB_CalculatedCrc == CALIB_Memory->crc32) {
        if (CALIB_ValidateBrightnessCurve()) {
            CALIB_CopyBrightnessCurveToMemory();
        }
    }
    else {
        /* CRC is invalid */
    }
}

void CALIB_Init(void) {
    // Initialize calibration parameters if needed
    CALIB_InitBrightnessCurve();
#if FEATURE_CALIB_LOAD_AT_STARTUP == 1
    (void)CALIB_LoadNvram();
#endif
}

uint8_t CALIB_MapBrightnessValue(uint8_t value) {
#if FEATURE_CALIB_ENABLE == 1
    // Map the input brightness value to a calibrated value using the brightness curve
    if (value < CALIB_BRIGHTNESS_RANGE) {
        return CALIB_BrightnessCurve[value];
    }
    else {
        return CALIB_BrightnessCurve[CALIB_BRIGHTNESS_RANGE - 1];
    }
#else
    // If calibration is disabled, return the input value directly
    return value;
#endif
}

uint16_t CALIB_GetVoltageCalib(void) {
    // Return a calibrated voltage value, for example, read from a sensor or stored calibration data
    return CALIB_VOLTAGE_MAX; // Placeholder value
}
