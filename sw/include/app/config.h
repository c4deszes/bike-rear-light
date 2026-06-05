#if !defined(APP_CONFIG_H_)
#define APP_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#include "strobe.h"
#include "uds_api.h"

typedef struct {
    bool AutomaticDiagnostics;

    uint16_t BrightnessCurve_Cutoff_X;
    uint16_t BrightnessCurve_Cutoff_Y;
    uint16_t BrightnessCurve_Max_X;
    uint16_t BrightnessCurve_Max_Y;

    uint16_t Brightness_LevelStandard;
    uint16_t Brightness_LevelEmergency;
    uint16_t Brightness_LevelSafety;

    uint16_t Brightness_BrakeLow;
    uint16_t Brightness_BrakeHigh;

    uint16_t Strobe_LevelLow;
    uint16_t Strobe_LevelHigh;
    uint16_t Strobe_LevelEmergency;
    uint16_t Strobe_LevelSafety;

    strobe_source_t Strobe_ModeDefault;
    strobe_source_t Strobe_ModePrimary;
    strobe_source_t Strobe_ModeEmergency;
    strobe_source_t Strobe_ModeSafety;

    uint16_t Strobe_SingleOnTime;
    uint16_t Strobe_SingleOffTime;
    uint16_t Strobe_RapidOnTime;
    uint16_t Strobe_RapidOffTime;

    uint16_t Driver_TailRefCurrent;
    uint16_t Driver_BrakeRefCurrent;
    uint8_t Driver_TailRefVoltage;
    uint8_t Driver_BrakeRefVoltage;
    uint8_t Driver_TailEfficiency;
    uint8_t Driver_BrakeEfficiency;
} config_properties_t;

extern config_properties_t CONFIG_Props;

/**
 * @brief Loads properties from flash memory into the UDS container,
 *        should only be called once at startup, but only after UDS Init
 */
void CONFIG_LoadNvram(void);

/**
 * @brief Loads the current UDS properties into the active configuration
 */
void CONFIG_Reload(void);

/**
 * @brief Notifies application components about configuration changes. Should be called after CONFIG_Reload if the new configuration should be applied.
 */
void CONFIG_ReloadComponents(void);

void CONFIG_Save(void);

void CONFIG_NotifyPropertyChange(const uds_property_t* property);

#endif // APP_CONFIG_H_
