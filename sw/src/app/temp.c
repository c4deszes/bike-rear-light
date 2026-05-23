#include "app/temp.h"

#include "bsp/tt_adc.h"
#include "bsp/tt_adc_cfg.h"

#include "app/feature.h"

struct {
    uint16_t adc_value;
    int8_t temperature;
} const TEMP_NtcLookupTable[] = {
    {12,    150},   // 150°C 126.50Ω 0.04V
    {14,    145},   // 145°C 144.50Ω 0.05V
    {16,    140},   // 140°C 165.40Ω 0.05V
    {19,    135},   // 135°C 189.80Ω 0.06V
    {21,    130},   // 130°C 218.40Ω 0.07V
    {25,    125},   // 125°C 251.90Ω 0.08V
    {28,    120},   // 120°C 291.30Ω 0.09V
    {33,    115},   // 115°C 337.80Ω 0.11V
    {38,    110},   // 110°C 392.90Ω 0.12V
    {44,    105},   // 105°C 458.50Ω 0.14V
    {52,    100},   // 100°C 537.10Ω 0.17V
    {60,    95},    // 95°C 631.60Ω 0.20V
    {70,    90},    // 90°C 745.70Ω 0.23V
    {83,    85},    // 85°C 884.30Ω 0.27V
    {97,    80},    // 80°C 1054.00Ω 0.31V
    {114,   75},    // 75°C 1261.00Ω 0.37V
    {134,   70},    // 70°C 1516.00Ω 0.43V
    {158,   65},    // 65°C 1831.00Ω 0.51V
    {185,   60},    // 60°C 2222.00Ω 0.60V
    {218,   55},    // 55°C 2709.00Ω 0.70V
    {254,   50},    // 50°C 3320.00Ω 0.82V
    {296,   45},    // 45°C 4090.00Ω 0.96V
    {344,   40},    // 40°C 5069.00Ω 1.11V
    {396,   35},    // 35°C 6318.00Ω 1.28V
    {452,   30},    // 30°C 7923.00Ω 1.46V
    {511,   25},    // 25°C 10000.00Ω 1.65V
    {572,   20},    // 20°C 12710.00Ω 1.85V
    {633,   15},    // 15°C 16270.00Ω 2.04V
    {692,   10},    // 10°C 20980.00Ω 2.23V
    {748,   5},     // 5°C 27250.00Ω 2.41V
    {798,   0},     // 0°C 35650.00Ω 2.58V
    {843,   -5},    // -5°C 46990.00Ω 2.72V
    {881,   -10},   // -10°C 62440.00Ω 2.84V
    {913,   -15},   // -15°C 83700.00Ω 2.95V
    {940,   -20},   // -20°C 113300.00Ω 3.03V
    {961,   -25},   // -25°C 155000.00Ω 3.10V
    {977,   -30},   // -30°C 214200.00Ω 3.15V
    {989,   -35},   // -35°C 299600.00Ω 3.19V
    {999,   -40},   // -40°C 424000.00Ω 3.22V
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
