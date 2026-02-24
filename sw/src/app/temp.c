#include "app/temp.h"

#include "bsp/tt_adc.h"
#include "bsp/tt_adc_cfg.h"

static int8_t TEMP_DriveTemperature;
static int8_t TEMP_McuTemperature;
static temp_status_t TEMP_Status;

struct {
    uint16_t adc_values[190 / 5];
    int8_t temperatures[190 / 5];
} NTC_LookupTable = {
    .adc_values = {
                    12, // 150°C 126.50Ω 0.04V
                    14, // 145°C 144.50Ω 0.05V
                    16, // 140°C 165.40Ω 0.05V
                    19, // 135°C 189.80Ω 0.06V
                    21, // 130°C 218.40Ω 0.07V
                    25, // 125°C 251.90Ω 0.08V
                    28, // 120°C 291.30Ω 0.09V
                    33, // 115°C 337.80Ω 0.11V
                    38, // 110°C 392.90Ω 0.12V
                    44, // 105°C 458.50Ω 0.14V
                    52, // 100°C 537.10Ω 0.17V
                    60, // 95°C 631.60Ω 0.20V
                    70, // 90°C 745.70Ω 0.23V
                    83, // 85°C 884.30Ω 0.27V
                    97, // 80°C 1054.00Ω 0.31V
                    114, // 75°C 1261.00Ω 0.37V
                    134, // 70°C 1516.00Ω 0.43V
                    158, // 65°C 1831.00Ω 0.51V
                    185, // 60°C 2222.00Ω 0.60V
                    218, // 55°C 2709.00Ω 0.70V
                    254, // 50°C 3320.00Ω 0.82V
                    296, // 45°C 4090.00Ω 0.96V
                    344, // 40°C 5069.00Ω 1.11V
                    396, // 35°C 6318.00Ω 1.28V
                    452, // 30°C 7923.00Ω 1.46V
                    511, // 25°C 10000.00Ω 1.65V
                    572, // 20°C 12710.00Ω 1.85V
                    633, // 15°C 16270.00Ω 2.04V
                    692, // 10°C 20980.00Ω 2.23V
                    748, // 5°C 27250.00Ω 2.41V
                    798, // 0°C 35650.00Ω 2.58V
                    843, // -5°C 46990.00Ω 2.72V
                    881, // -10°C 62440.00Ω 2.84V
                    913, // -15°C 83700.00Ω 2.95V
                    940, // -20°C 113300.00Ω 3.03V
                    961, // -25°C 155000.00Ω 3.10V
                    977, // -30°C 214200.00Ω 3.15V
                    989, // -35°C 299600.00Ω 3.19V
                    999, // -40°C 424000.00Ω 3.22V
    },
    .temperatures = {
        150, 145, 140, 135, 130, 125, 120, 115, 110, 105,
        100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
        50, 45, 40, 35, 30, 25, 20, 15, 10, 5,
        0, -5, -10, -15, -20, -25, -30, -35, -40
    }
};

void TEMP_Init(void) {
    TEMP_DriveTemperature = 0;
    TEMP_McuTemperature = 0;
    TEMP_Status = temp_status_not_measured;
}

int8_t TEMP_NtcLookup(uint16_t adc_value) {
    // Simple linear search through the lookup table
    for (uint8_t i = 0; i < sizeof(NTC_LookupTable.adc_values) / sizeof(NTC_LookupTable.adc_values[0]); i++) {
        if (adc_value <= NTC_LookupTable.adc_values[i]) {
            return NTC_LookupTable.temperatures[i];
        }
    }
    // If the ADC value is higher than any in the table, return the lowest temperature
    return NTC_LookupTable.temperatures[sizeof(NTC_LookupTable.temperatures) / sizeof(NTC_LookupTable.temperatures[0]) - 1];
}

void TEMP_Update100ms(void) {
    if (TTADC_ResultReady(TTADC_CHANNEL_TDRV)) {
        TEMP_DriveTemperature = TEMP_NtcLookup(TTADC_GetResult(TTADC_CHANNEL_TDRV));
    } else {
        TEMP_Status = temp_status_error;
        return;
    }
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
