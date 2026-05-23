#include "app/config.h"
#include "uds_gen.h"

#include "hal/dsu.h"
#include "hal/nvmctrl.h"

#include "app/brightness.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "app/current.h"
#include "app/config_priv.h"

static config_memlayout_t* CONFIG_Memory = (config_memlayout_t*)CONFIG_FLASH_ADDRESS;
config_memlayout_t CONFIG_Shadow __attribute__((aligned(4)));
uint32_t CONFIG_CalculatedCrc;

void CONFIG_LoadNvram(void) {
    CONFIG_CalculatedCrc = DSU_CalculateCRC32(0xFFFFFFFFUL,
                                                (void*)CONFIG_FLASH_ADDRESS,
                                                256u-4u);

    if (CONFIG_CalculatedCrc == CONFIG_Memory->crc32) {
        UDS_Properties_RearLight = CONFIG_Memory->properties;
    }
    else {
        // TODO: handle config memory corruption
    }
}

config_properties_t CONFIG_Props;

static strobe_source_t CONFIG_ToStrobeSource(uint8_t config) {
    if (config == UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_DISABLED) {
        return strobe_source_disabled;
    }
    else if (config == UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_SINGLE) {
        return strobe_source_internal_single;
    }
    else if (config == UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_RAPID) {
        return strobe_source_internal_rapid;
    }
    // TODO: support for external negative and positive strobe sources

    /* Safety reaction, strobe is disabled */
    return strobe_source_disabled;
}

void CONFIG_Reload(void) {
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
    CONFIG_Props.Strobe_ModeDefault = CONFIG_ToStrobeSource(UDS_Properties_RearLight.Strobe_ModeDefault);
    CONFIG_Props.Strobe_ModePrimary = CONFIG_ToStrobeSource(UDS_Properties_RearLight.Strobe_ModePrimary);
    CONFIG_Props.Strobe_ModeEmergency = CONFIG_ToStrobeSource(UDS_Properties_RearLight.Strobe_ModeEmergency);
    CONFIG_Props.Strobe_ModeSafety = CONFIG_ToStrobeSource(UDS_Properties_RearLight.Strobe_ModeSafety);
    CONFIG_Props.Strobe_SingleOnTime = UDS_App_GetValidProperty_RearLight_Strobe_SingleOnTime();
    CONFIG_Props.Strobe_SingleOffTime = UDS_App_GetValidProperty_RearLight_Strobe_SingleOffTime();
    CONFIG_Props.Strobe_RapidOnTime = UDS_App_GetValidProperty_RearLight_Strobe_RapidOnTime();
    CONFIG_Props.Strobe_RapidOffTime = UDS_App_GetValidProperty_RearLight_Strobe_RapidOffTime();
    CONFIG_Props.Driver_TailRefCurrent = UDS_App_GetValidProperty_RearLight_Driver_TailRefCurrent();
    CONFIG_Props.Driver_BrakeRefCurrent = UDS_App_GetValidProperty_RearLight_Driver_BrakeRefCurrent();
    CONFIG_Props.Driver_TailRefVoltage = UDS_App_GetValidProperty_RearLight_Driver_TailRefVoltage();
    CONFIG_Props.Driver_BrakeRefVoltage = UDS_App_GetValidProperty_RearLight_Driver_BrakeRefVoltage();
    CONFIG_Props.Driver_TailEfficiency = UDS_App_GetValidProperty_RearLight_Driver_TailEfficiency();
    CONFIG_Props.Driver_BrakeEfficiency = UDS_App_GetValidProperty_RearLight_Driver_BrakeEfficiency();
}

void CONFIG_ReloadComponents(void) {
    SYSSTATE_LoadConfig();
    BRIGHTNESS_LoadConfig();
    STROBE_LoadConfig();
    CURRENT_LoadConfig();
}

// TODO: replace with DSU_SoftwareCRC32
static uint32_t crc32(const uint8_t *data, uint32_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return crc ^ 0xFFFFFFFF;
}

// TODO: need callback for when properties are changed
// only save when changed
void CONFIG_Save(void) {
    CONFIG_Shadow.properties = UDS_Properties_RearLight;
    for (uint16_t i = 0; i < sizeof(CONFIG_Shadow.padding); i++) {
        CONFIG_Shadow.padding[i] = 0xFF;
    }
    CONFIG_Shadow.crc32 = crc32((uint8_t*)(&CONFIG_Shadow), 256u-4u);

    NVMCTRL_EraseRow(CONFIG_FLASH_ADDRESS);

    // // TODO: don't hardcode page size
    // TODO: replace with HAL call
    for (uint16_t i = 0; i < sizeof(config_memlayout_t) / 64u; i += 1) {
        NVMCTRL_PageBufferClear();
        for (uint16_t j = 0; j < 64u; j += 4) {
            *((uint32_t*)(CONFIG_FLASH_ADDRESS + i * 64u + j)) = *((uint32_t*)(((uint8_t*)&CONFIG_Shadow) + i * 64u + j));
        }
        NVMCTRL_WritePage(CONFIG_FLASH_ADDRESS + i * 64u);
    }
}
