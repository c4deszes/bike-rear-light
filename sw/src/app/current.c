#include "app/current.h"

#include "bsp/light_control.h"
#include "bsp/tt_adc_cfg.h"

#include "app/config.h"
#include "app/feature.h"
#include "app/volt.h"
#include "app/temp.h"

static const uint8_t Current_VoltDerate_X[] = {FEATURE_CURRENT_VOLT_DERATE_X};
static const uint16_t Current_VoltDerate_Y[] = {FEATURE_CURRENT_VOLT_DERATE_Y};

static const int8_t Current_TempDerate_X[] = {FEATURE_CURRENT_TEMP_DERATE_X};
static const uint16_t Current_TempDerate_Y[] = {FEATURE_CURRENT_TEMP_DERATE_Y};

static uint16_t CURRENT_Estimate;
static current_estimate_state_t CURRENT_EstimateState;
static uint16_t CURRENT_VoltageDerating;
static uint16_t CURRENT_TemperatureDerating;
static uint16_t CURRENT_DeratingFactor;

static uint16_t CURRENT_ConfTailReferenceCurrent;
static uint16_t CURRENT_ConfTailReferenceVoltage;
static uint8_t CURRENT_ConfTailEfficiency;
static uint16_t CURRENT_ConfBrakeReferenceCurrent;
static uint16_t CURRENT_ConfBrakeReferenceVoltage;
static uint8_t CURRENT_ConfBrakeEfficiency;

void CURRENT_Init(void) {
    CURRENT_Estimate = 0;
    CURRENT_EstimateState = current_estimate_state_na;
    CURRENT_VoltageDerating = CURRENT_DERATING_NONE;
    CURRENT_TemperatureDerating = CURRENT_DERATING_NONE;
    CURRENT_DeratingFactor = CURRENT_DERATING_NONE;

    CURRENT_LoadConfig();
}

void CURRENT_LoadConfig(void) {
    CURRENT_ConfTailReferenceCurrent = CONFIG_Props.Driver_TailRefCurrent;
    CURRENT_ConfTailReferenceVoltage = CONFIG_Props.Driver_TailRefVoltage;
    CURRENT_ConfTailEfficiency = CONFIG_Props.Driver_TailEfficiency;
    CURRENT_ConfBrakeReferenceCurrent = CONFIG_Props.Driver_BrakeRefCurrent;
    CURRENT_ConfBrakeReferenceVoltage = CONFIG_Props.Driver_BrakeRefVoltage;
    CURRENT_ConfBrakeEfficiency = CONFIG_Props.Driver_BrakeEfficiency;
}

/**
 * @brief Calculate the current for a linear driver
 * 
 * @param brightness The target brightness (0-1000)
 * @param reference_current The reference current at maximum brightness (in mA)
 * @param efficiency The efficiency of the driver (in %)
 * @return The calculated current (in mA)
 */
static uint16_t CURRENT_CalculateLinearCurrent(uint16_t brightness, uint16_t reference_current,
                                               uint8_t efficiency) {
    uint16_t current = (reference_current * brightness) / LIGHTCONTROL_BRIGHTNESS_MAX;
    current = (current * 100) / efficiency;

    return current;
}

/**
 * @brief Calculate the current for a buck driver
 * 
 * @param brightness The target brightness (0-1000)
 * @param voltage The supply voltage (in 100mV/inc)
 * @param reference_current The reference current at maximum brightness and reference voltage (in mA)
 * @param reference_voltage The reference voltage for the reference current (in 100mV/inc)
 * @param efficiency The efficiency of the driver (in %)
 * @return The calculated current (in mA)
 */
static uint16_t CURRENT_CalculateBuckCurrent(uint16_t brightness, uint16_t voltage,
                                             uint16_t reference_current, uint16_t reference_voltage,
                                             uint8_t efficiency) {
                                            
    // TODO: limit the voltage, otherwise zero division is possible, limit efficiency as well
    uint16_t current = (reference_current * reference_voltage * brightness)
                       / (voltage * LIGHTCONTROL_BRIGHTNESS_MAX);
    current = (current * 100) / efficiency;

    return current;
}

static void CURRENT_UpdateEstimate(void) {
    // TODO: add mcu current consumption
    // TODO: implement, difficult because of strobing, also settings might mean that the strobe
    //       is not on/off but 50-100%, so the strobe target needs to be considered as well
}

static uint16_t CURRENT_CalculateVoltageDerating(uint8_t voltage) {
    // Find the appropriate derating factor based on the voltage using linear interpolation
    for (uint8_t i = 0; i < sizeof(Current_VoltDerate_X) / sizeof(Current_VoltDerate_X[0]) - 1; i++) {
        if (voltage < Current_VoltDerate_X[i]) {
            return Current_VoltDerate_Y[i];
        } else if (voltage < Current_VoltDerate_X[i + 1]) {
            // Linear interpolation
            uint8_t x1 = Current_VoltDerate_X[i];
            uint16_t y1 = Current_VoltDerate_Y[i];
            uint8_t x2 = Current_VoltDerate_X[i + 1];
            uint16_t y2 = Current_VoltDerate_Y[i + 1];

            return y1 + (y2 - y1) * (voltage - x1) / (x2 - x1);
        }
    }

    return Current_VoltDerate_Y[sizeof(Current_VoltDerate_Y) / sizeof(Current_VoltDerate_Y[0]) - 1];
}

static uint16_t CURRENT_CalculateTemperatureDerating(int8_t temperature) {
    // Find the appropriate derating factor based on the temperature using linear interpolation
    for (uint8_t i = 0; i < sizeof(Current_TempDerate_X) / sizeof(Current_TempDerate_X[0]) - 1; i++) {
        if (temperature < Current_TempDerate_X[i]) {
            return Current_TempDerate_Y[i];
        } else if (temperature < Current_TempDerate_X[i + 1]) {
            // Linear interpolation
            int8_t x1 = Current_TempDerate_X[i];
            uint16_t y1 = Current_TempDerate_Y[i];
            int8_t x2 = Current_TempDerate_X[i + 1];
            uint16_t y2 = Current_TempDerate_Y[i + 1];

            return y1 + (y2 - y1) * (temperature - x1) / (x2 - x1);
        }
    }

    return Current_TempDerate_Y[sizeof(Current_TempDerate_Y) / sizeof(Current_TempDerate_Y[0]) - 1];
}

void CURRENT_UpdateDeratingFactor(void) {
    /**
     * Determining the derating factor based on the voltage and temperature derating curves,
     * the lowest derating factor is used to limit the current as much as necessary.
     */
    CURRENT_VoltageDerating = CURRENT_DERATING_MAX;

    #if defined(TTADC_CHANNEL_VBAT)
    /* If voltage measurement is available then determine the derating factor based on the voltage.
     * In case of an error in the voltage measurement, use maximum derating to be safe
     */
    volt_status_t volt_status = VOLT_GetStatus();
    if (volt_status == volt_status_ok || volt_status == volt_status_low) {
        CURRENT_VoltageDerating = CURRENT_CalculateVoltageDerating(VOLT_GetVoltage()); // Convert to V
    }
    else {
        CURRENT_VoltageDerating = CURRENT_DERATING_MAX;
    }
    #else
    /* In case the voltage measurement is not available, use no derating */
    CURRENT_VoltageDerating = CURRENT_DERATING_NONE;
    #endif

    CURRENT_TemperatureDerating = CURRENT_DERATING_MAX;
    #if defined(TTADC_CHANNEL_TMCU) || defined(TTADC_CHANNEL_TDRV)
    /* If temperature measurement is available then determine the derating factor based on the temperature.
     * In case of an error in the temperature measurement, use maximum derating to be safe
     */
    temp_status_t temp_status = TEMP_GetStatus();
    if (temp_status == temp_status_ok) {
        CURRENT_TemperatureDerating = CURRENT_CalculateTemperatureDerating(TEMP_GetTemperature()); // Convert to °C
    }
    else {
        CURRENT_TemperatureDerating = CURRENT_DERATING_MAX;
    }
    #else
    /* In case the temperature measurement is not available, use no derating */
    CURRENT_TemperatureDerating = CURRENT_DERATING_NONE;
    #endif

    if (CURRENT_VoltageDerating < CURRENT_TemperatureDerating) {
        CURRENT_DeratingFactor = CURRENT_VoltageDerating;
    }
    else {
        CURRENT_DeratingFactor = CURRENT_TemperatureDerating;
    }
}

void CURRENT_Update100ms(void) {

    CURRENT_UpdateEstimate();

    CURRENT_UpdateDeratingFactor();
}

uint16_t CURRENT_GetCurrent(void) {
    return CURRENT_Estimate;
}

current_estimate_state_t CURRENT_GetEstimateState(void) {
    return CURRENT_EstimateState;
}

bool CURRENT_ThermalDeratingActive(void) {
    return CURRENT_TemperatureDerating < CURRENT_DERATING_NONE;
}

uint16_t CURRENT_GetDeratingFactor(void) {
    return CURRENT_DeratingFactor;
}
