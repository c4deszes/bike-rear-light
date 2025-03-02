#include "app/config.h"
#include "uds_gen.h"

#include "hal/dsu.h"

#define CONFIG_FLASH_ADDRESS 0x3F000
#define CONFIG_FLASH_SIZE (64 * 4)

typedef struct {
    UDS_Properties_RearLight_t properties;
    uint8_t padding[CONFIG_FLASH_SIZE - sizeof(UDS_Properties_RearLight_t) - sizeof(uint32_t)];
    uint32_t crc32;
} config_memlayout_t;

static config_memlayout_t* config = (config_memlayout_t*)CONFIG_FLASH_ADDRESS;

void CONFIG_LoadFlashProperties(void) {

    // uint32_t calculatedCrc = DSU_CalculateCRC32(0xFFFFFFFFUL,
    //                                             (void*)CONFIG_FLASH_ADDRESS,
    //                                             CONFIG_FLASH_SIZE);

    // if (calculatedCrc == config->crc32) {
    //     UDS_Properties_RearLight = config->properties;
    // }
    // else {

    // }
}

config_properties_t CONFIG_Props;

static strobe_source_t STROBE_ConvertSource(uint8_t config) {
    if (config == UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_DISABLED) {
        return strobe_source_disabled;
    }
    else if (config == UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_SINGLE) {
        return strobe_source_internal_single;
    }
    else if (config == UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_RAPID) {
        return strobe_source_internal_rapid;
    }
    // TODO: support for negative and positive
    return strobe_source_disabled;
}

void CONFIG_ReloadUdsProperties(void) {
    // Takes the UDS container
    // for every property checks whether it's valid
    // if it's not valid, it loads the default value
    // if it's valid, it loads the value from the container
    // it also does typecasting
    CONFIG_Props.AutomaticDiagnostics = UDS_Properties_RearLight.AutomaticDiagnostics;
    CONFIG_Props.BrightnessCurve_Cutoff_X = UDS_App_GetValidProperty_RearLight_BrightnessCurve_Cutoff_X();
    CONFIG_Props.BrightnessCurve_Cutoff_Y = UDS_App_GetValidProperty_RearLight_BrightnessCurve_Cutoff_Y();
    CONFIG_Props.BrightnessCurve_Max_X = UDS_App_GetValidProperty_RearLight_BrightnessCurve_Max_X();
    CONFIG_Props.BrightnessCurve_Max_Y = UDS_App_GetValidProperty_RearLight_BrightnessCurve_Max_Y();
    CONFIG_Props.Brightness_LevelStandard = UDS_App_GetValidProperty_RearLight_Brightness_LevelStandard();
    CONFIG_Props.Brightness_LevelEmergency = UDS_App_GetValidProperty_RearLight_Brightness_LevelEmergency();
    CONFIG_Props.Brightness_LevelSafety = UDS_App_GetValidProperty_RearLight_Brightness_LevelSafety();
    CONFIG_Props.Brightness_BrakeLow = UDS_App_GetValidProperty_RearLight_Brightness_BrakeLow();
    CONFIG_Props.Brightness_BrakeHigh = UDS_App_GetValidProperty_RearLight_Brightness_BrakeHigh();
    CONFIG_Props.Strobe_LevelLow = UDS_App_GetValidProperty_RearLight_Strobe_LevelLow();
    CONFIG_Props.Strobe_LevelHigh = UDS_App_GetValidProperty_RearLight_Strobe_LevelHigh();
    CONFIG_Props.Strobe_LevelEmergency = UDS_App_GetValidProperty_RearLight_Strobe_LevelEmergency();
    CONFIG_Props.Strobe_LevelSafety = UDS_App_GetValidProperty_RearLight_Strobe_LevelSafety();
    CONFIG_Props.Strobe_ModeDefault = STROBE_ConvertSource(UDS_Properties_RearLight.Strobe_ModeDefault);
    CONFIG_Props.Strobe_ModePrimary = STROBE_ConvertSource(UDS_Properties_RearLight.Strobe_ModePrimary);
    CONFIG_Props.Strobe_ModeEmergency = STROBE_ConvertSource(UDS_Properties_RearLight.Strobe_ModeEmergency);
    CONFIG_Props.Strobe_ModeSafety = STROBE_ConvertSource(UDS_Properties_RearLight.Strobe_ModeSafety);
    CONFIG_Props.Strobe_SingleOnTime = UDS_App_GetValidProperty_RearLight_Strobe_SingleOnTime();
    CONFIG_Props.Strobe_SingleOffTime = UDS_App_GetValidProperty_RearLight_Strobe_SingleOffTime();
    CONFIG_Props.Strobe_RapidOnTime = UDS_App_GetValidProperty_RearLight_Strobe_RapidOnTime();
    CONFIG_Props.Strobe_RapidOffTime = UDS_App_GetValidProperty_RearLight_Strobe_RapidOffTime();
}

void CONFIG_Save(void) {
    // copy uds properties
    // calculate crc
    // write to flash

    // config->properties = UDS_Properties_RearLight;
    // config->crc32 = DSU_CalculateCRC32(0xFFFFFFFFUL,
    //                                    (void*)CONFIG_FLASH_ADDRESS,
    //                                    CONFIG_FLASH_SIZE);
}
