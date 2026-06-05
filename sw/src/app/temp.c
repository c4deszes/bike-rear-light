#include "app/temp.h"

#include "bsp/tt_adc.h"
#include "bsp/tt_adc_cfg.h"

#include "app/feature.h"

struct {
    uint16_t adc_value;
    int8_t temperature;
} const TEMP_NtcLookupTable[] = {
    {23, -40},      // -40°C 424000.00Ω 0.08V
    {33, -35},      // -35°C 299600.00Ω 0.11V
    {45, -30},      // -30°C 214200.00Ω 0.15V
    {62, -25},      // -25°C 155000.00Ω 0.20V
    {82, -20},      // -20°C 113300.00Ω 0.27V
    {109, -15},     // -15°C 83700.00Ω 0.35V
    {141, -10},     // -10°C 62440.00Ω 0.46V
    {179, -5},      // -5°C 46990.00Ω 0.58V
    {224, 0},       // 0°C 35650.00Ω 0.72V
    {274, 5},       // 5°C 27250.00Ω 0.89V
    {330, 10},      // 10°C 20980.00Ω 1.07V
    {389, 15},      // 15°C 16270.00Ω 1.26V
    {450, 20},      // 20°C 12710.00Ω 1.45V
    {511, 25},      // 25°C 10000.00Ω 1.65V
    {570, 30},      // 30°C 7923.00Ω 1.84V
    {626, 35},      // 35°C 6318.00Ω 2.02V
    {678, 40},      // 40°C 5069.00Ω 2.19V
    {726, 45},      // 45°C 4090.00Ω 2.34V
    {768, 50},      // 50°C 3320.00Ω 2.48V
    {804, 55},      // 55°C 2709.00Ω 2.60V
    {837, 60},      // 60°C 2222.00Ω 2.70V
    {864, 65},      // 65°C 1831.00Ω 2.79V
    {888, 70},      // 70°C 1516.00Ω 2.87V
    {908, 75},      // 75°C 1261.00Ω 2.93V
    {925, 80},      // 80°C 1054.00Ω 2.99V
    {939, 85},      // 85°C 884.30Ω 3.03V
    {952, 90},      // 90°C 745.70Ω 3.07V
    {962, 95},      // 95°C 631.60Ω 3.10V
    {970, 100},     // 100°C 537.10Ω 3.13V
    {978, 105},     // 105°C 458.50Ω 3.16V
    {984, 110},     // 110°C 392.90Ω 3.18V
    {989, 115},     // 115°C 337.80Ω 3.19V
    {994, 120},     // 120°C 291.30Ω 3.21V
    {997, 125},     // 125°C 251.90Ω 3.22V
    {1001, 130},    // 130°C 218.40Ω 3.23V
    {1003, 135},    // 135°C 189.80Ω 3.24V
    {1006, 140},    // 140°C 165.40Ω 3.25V
    {1008, 145},    // 145°C 144.50Ω 3.25V
    {1010, 150},    // 150°C 126.50Ω 3.26V
};

static uint16_t TEMP_DriveAdcValue;
static int8_t TEMP_DriveTemperature;
static uint8_t TEMP_DriveTempTimeout;
static temp_status_t TEMP_DriveStatus;

static uint16_t TEMP_McuAdcValue;
static int8_t TEMP_McuTemperature;
static uint8_t TEMP_McuTempTimeout;
static temp_status_t TEMP_McuStatus;

static int8_t TEMP_Temperature;
static temp_status_t TEMP_Status;

void TEMP_Init(void) {
    TEMP_DriveTemperature = 0;
    TEMP_McuTemperature = 0;
    TEMP_DriveTempTimeout = 0;
    TEMP_McuTempTimeout = 0;
    TEMP_DriveStatus = temp_status_not_measured;
    TEMP_McuStatus = temp_status_not_measured;
    TEMP_Temperature = 0;
    TEMP_Status = temp_status_not_measured;
}

int8_t TEMP_NtcLookup(uint16_t adc_value) {
    int table_size = sizeof(TEMP_NtcLookupTable) / sizeof(TEMP_NtcLookupTable[0]);

    // Handle out of range - return first or last element
    if (adc_value <= TEMP_NtcLookupTable[0].adc_value) {
        return TEMP_NtcLookupTable[0].temperature;
    }
    if (adc_value >= TEMP_NtcLookupTable[table_size - 1].adc_value) {
        return TEMP_NtcLookupTable[table_size - 1].temperature;
    }

    // Find the two points to interpolate between
    for (int i = 0; i < table_size - 1; i++) {
        if (adc_value >= TEMP_NtcLookupTable[i].adc_value && 
            adc_value <= TEMP_NtcLookupTable[i + 1].adc_value) {

            uint16_t adc1 = TEMP_NtcLookupTable[i].adc_value;
            uint16_t adc2 = TEMP_NtcLookupTable[i + 1].adc_value;
            int8_t temp1 = TEMP_NtcLookupTable[i].temperature;
            int8_t temp2 = TEMP_NtcLookupTable[i + 1].temperature;

            // Linear interpolation
            int16_t temp = temp1 + (int16_t)(temp2 - temp1) * (adc_value - adc1) / (adc2 - adc1);
            return (int8_t)temp;
        }
    }
    
    return TEMP_NtcLookupTable[table_size - 1].temperature;
}

int8_t TEMP_McuTempLookup(uint16_t adc_value) {
    // TODO: implement according SAMD21 datasheet and internal temperature sensor characteristics
    return 0;
}

void TEMP_Update100ms(void) {

#if defined(TTADC_CHANNEL_TDRV)
    if (TTADC_ResultReady(TTADC_CHANNEL_TDRV)) {
        TEMP_DriveAdcValue = TTADC_GetResult(TTADC_CHANNEL_TDRV);
        TEMP_DriveTemperature = TEMP_NtcLookup(TEMP_DriveAdcValue);
        TEMP_DriveTempTimeout = 0;

        if (TEMP_DriveTemperature > FEATURE_TEMP_DRIVE_MAX_TEMPERATURE || TEMP_DriveTemperature < FEATURE_TEMP_DRIVE_MIN_TEMPERATURE) {
            TEMP_DriveStatus = temp_status_error;
        }
        else {
            TEMP_DriveStatus = temp_status_ok;
        }
    }
    else if(TEMP_DriveTempTimeout < FEATURE_TEMP_DRIVE_TEMP_TIMEOUT) {
        TEMP_DriveTempTimeout += 100;
    }
    else {
        TEMP_DriveStatus = temp_status_error;
    }
#endif

#if defined(TTADC_CHANNEL_TMCU)
    if (TTADC_ResultReady(TTADC_CHANNEL_TMCU)) {
        TEMP_McuAdcValue = TTADC_GetResult(TTADC_CHANNEL_TMCU);
        TEMP_McuTemperature = TEMP_McuTempLookup(TEMP_McuAdcValue);
        TEMP_McuTempTimeout = 0;
        TEMP_McuStatus = temp_status_ok;
    }
    else if(TEMP_McuTempTimeout < FEATURE_TEMP_MCU_TEMP_TIMEOUT) {
        TEMP_McuTempTimeout += 100;
    }
    else {
        TEMP_McuStatus = temp_status_error;
    }
#endif

    if (TEMP_DriveStatus == temp_status_ok && TEMP_McuStatus == temp_status_ok) {
        TEMP_Temperature = (FEATURE_TEMP_DRIVE_FACTOR * TEMP_DriveTemperature +
                            FEATURE_TEMP_MCU_FACTOR * TEMP_McuTemperature)
                           / (FEATURE_TEMP_DRIVE_FACTOR + FEATURE_TEMP_MCU_FACTOR);
        TEMP_Status = temp_status_ok;
    }
    else if (TEMP_DriveStatus == temp_status_ok) {
        TEMP_Temperature = TEMP_DriveTemperature;
        TEMP_Status = temp_status_ok;
    }
    else if (TEMP_McuStatus == temp_status_ok) {
        // TODO: estimate drive temperature based on MCU temperature and target brightness
        TEMP_Temperature = TEMP_McuTemperature;
        TEMP_Status = temp_status_ok;
    }
    else if (TEMP_DriveStatus == temp_status_not_measured && TEMP_McuStatus == temp_status_not_measured) {
        TEMP_Temperature = 0;
        TEMP_Status = temp_status_not_measured;
    }
    else {
        TEMP_Temperature = 0;
        TEMP_Status = temp_status_error;
    }
}

int8_t TEMP_GetDriveTemperature(void) {
    return TEMP_DriveTemperature;
}

int8_t TEMP_GetMcuTemperature(void) {
    return TEMP_McuTemperature;
}

int8_t TEMP_GetTemperature(void) {
    return TEMP_Temperature;
}

temp_status_t TEMP_GetStatus(void) {
    return TEMP_Status;
}

uint16_t TEMP_GetDriveTemperatureAdc(void) {
    return TEMP_DriveAdcValue;
}

uint16_t TEMP_GetMcuTemperatureAdc(void) {
    return TEMP_McuAdcValue;
}
