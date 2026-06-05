#ifndef APP_CONFIG_PRIV_H_
#define APP_CONFIG_PRIV_H_

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_ATTR __attribute__((section(".config_data")))
#define CONFIG_FLASH_ADDRESS 0x3F000u
#define CONFIG_FLASH_SIZE (64u * 4u)

#define CONFIG_STROBE_MODE_DISABLED 0
#define CONFIG_STROBE_MODE_INTERNAL_SINGLE 1
#define CONFIG_STROBE_MODE_INTERNAL_RAPID 2

typedef struct __attribute__((packed)) {
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

    uint8_t Strobe_ModeDefault;
    uint8_t Strobe_ModePrimary;
    uint8_t Strobe_ModeEmergency;
    uint8_t Strobe_ModeSafety;

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
} config_properties_v1_t;

typedef struct __attribute__((packed)) {
    uint8_t version;
    config_properties_v1_t properties;
    uint8_t padding[CONFIG_FLASH_SIZE - sizeof(uint8_t) - sizeof(config_properties_v1_t) - sizeof(uint32_t)];
    uint32_t crc32;
} config_layout_v1_t;

typedef struct __attribute__((packed)) {
    uint8_t version;
    uint8_t padding[CONFIG_FLASH_SIZE - sizeof(uint8_t) - sizeof(uint32_t)];
    uint32_t crc32;
} config_layout_generic_t;

#endif /* APP_CONFIG_PRIV_H_ */