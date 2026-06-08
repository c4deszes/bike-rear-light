#include "app/config.h"
#include "uds_gen.h"

#include "hal/dsu.h"
#include "hal/nvmctrl.h"

#include "app/brightness.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "app/current.h"
#include "app/config_priv.h"

static config_layout_generic_t* CONFIG_Memory = (config_layout_generic_t*)CONFIG_FLASH_ADDRESS;
static config_layout_generic_t CONFIG_Shadow;

static struct {
    bool CrcInvalid : 1;
    bool VersionInvalid : 1;
    uint8_t Reserved : 6;
} CONFIG_ErrorFlags;

static bool CONFIG_Changed;
config_properties_t CONFIG_Props;

uds_StrobeMode_t CONFIG_ConvertStrobeConfigToUds(uint8_t config) {
    /* Warning: this interface should only be modified in a way that's compatible with different
       configuration layout versions */
    switch (config) {
        case CONFIG_STROBE_MODE_DISABLED:
            return uds_StrobeMode_DISABLED;
        case CONFIG_STROBE_MODE_INTERNAL_SINGLE:
            return uds_StrobeMode_INTERNAL_SINGLE;
        case CONFIG_STROBE_MODE_INTERNAL_RAPID:
            return uds_StrobeMode_INTERNAL_RAPID;
    }
    return uds_StrobeMode_DISABLED;
}

static strobe_source_t CONFIG_ToStrobeSource(uds_StrobeMode_t config) {
    if (config == uds_StrobeMode_DISABLED) {
        return strobe_source_disabled;
    }
    else if (config == uds_StrobeMode_INTERNAL_SINGLE) {
        return strobe_source_internal_single;
    }
    else if (config == uds_StrobeMode_INTERNAL_RAPID) {
        return strobe_source_internal_rapid;
    }
    // TODO: support for external negative and positive strobe sources

    /* Safety reaction, strobe is disabled */
    return strobe_source_disabled;
}

static uint8_t CONFIG_ConvertStrobeSourceToConfig(strobe_source_t source) {
    switch (source) {
        case strobe_source_disabled:
            return uds_StrobeMode_DISABLED;
        case strobe_source_internal_single:
            return uds_StrobeMode_INTERNAL_SINGLE;
        case strobe_source_internal_rapid:
            return uds_StrobeMode_INTERNAL_RAPID;
        case strobe_source_external_positive:
            return uds_StrobeMode_EXTERNAL_POS;
        case strobe_source_external_negative:
            return uds_StrobeMode_EXTERNAL_NEG;
    }
    return uds_StrobeMode_DISABLED;
}

void CONFIG_LoadProperties_v1(void) {
    config_layout_v1_t* mem = (config_layout_v1_t*)CONFIG_Memory;
    UDS_RearLight_Properties.AutomaticDiagnostics = mem->properties.AutomaticDiagnostics;
    UDS_RearLight_Properties.BrightnessCurve_Cutoff_X = mem->properties.BrightnessCurve_Cutoff_X;
    UDS_RearLight_Properties.BrightnessCurve_Cutoff_Y = mem->properties.BrightnessCurve_Cutoff_Y;
    UDS_RearLight_Properties.BrightnessCurve_Max_X = mem->properties.BrightnessCurve_Max_X;
    UDS_RearLight_Properties.BrightnessCurve_Max_Y = mem->properties.BrightnessCurve_Max_Y;
    UDS_RearLight_Properties.Brightness_LevelStandard = mem->properties.Brightness_LevelStandard;
    UDS_RearLight_Properties.Brightness_LevelEmergency = mem->properties.Brightness_LevelEmergency;
    UDS_RearLight_Properties.Brightness_LevelSafety = mem->properties.Brightness_LevelSafety;
    UDS_RearLight_Properties.Brightness_BrakeLow = mem->properties.Brightness_BrakeLow;
    UDS_RearLight_Properties.Brightness_BrakeHigh = mem->properties.Brightness_BrakeHigh;
    UDS_RearLight_Properties.Strobe_LevelLow = mem->properties.Strobe_LevelLow;
    UDS_RearLight_Properties.Strobe_LevelHigh = mem->properties.Strobe_LevelHigh;
    UDS_RearLight_Properties.Strobe_LevelEmergency = mem->properties.Strobe_LevelEmergency;
    UDS_RearLight_Properties.Strobe_LevelSafety = mem->properties.Strobe_LevelSafety;
    UDS_RearLight_Properties.Strobe_ModeDefault = CONFIG_ConvertStrobeConfigToUds(mem->properties.Strobe_ModeDefault);
    UDS_RearLight_Properties.Strobe_ModePrimary = CONFIG_ConvertStrobeConfigToUds(mem->properties.Strobe_ModePrimary);
    UDS_RearLight_Properties.Strobe_ModeEmergency = CONFIG_ConvertStrobeConfigToUds(mem->properties.Strobe_ModeEmergency);
    UDS_RearLight_Properties.Strobe_ModeSafety = CONFIG_ConvertStrobeConfigToUds(mem->properties.Strobe_ModeSafety);
    UDS_RearLight_Properties.Strobe_SingleOnTime = mem->properties.Strobe_SingleOnTime;
    UDS_RearLight_Properties.Strobe_SingleOffTime = mem->properties.Strobe_SingleOffTime;
    UDS_RearLight_Properties.Strobe_RapidOnTime = mem->properties.Strobe_RapidOnTime;
    UDS_RearLight_Properties.Strobe_RapidOffTime = mem->properties.Strobe_RapidOffTime;
    UDS_RearLight_Properties.Driver_TailRefCurrent = mem->properties.Driver_TailRefCurrent;
    UDS_RearLight_Properties.Driver_BrakeRefCurrent = mem->properties.Driver_BrakeRefCurrent;
    UDS_RearLight_Properties.Driver_TailRefVoltage = mem->properties.Driver_TailRefVoltage;
    UDS_RearLight_Properties.Driver_BrakeRefVoltage = mem->properties.Driver_BrakeRefVoltage;
    UDS_RearLight_Properties.Driver_TailEfficiency = mem->properties.Driver_TailEfficiency;
    UDS_RearLight_Properties.Driver_BrakeEfficiency = mem->properties.Driver_BrakeEfficiency;
}

void CONFIG_CopyPropsToShadow_v1(void) {
    config_layout_v1_t* mem = (config_layout_v1_t*)(&CONFIG_Shadow);
    mem->version = 1;
    mem->properties.AutomaticDiagnostics = CONFIG_Props.AutomaticDiagnostics;
    mem->properties.BrightnessCurve_Cutoff_X = CONFIG_Props.BrightnessCurve_Cutoff_X;
    mem->properties.BrightnessCurve_Cutoff_Y = CONFIG_Props.BrightnessCurve_Cutoff_Y;
    mem->properties.BrightnessCurve_Max_X = CONFIG_Props.BrightnessCurve_Max_X;
    mem->properties.BrightnessCurve_Max_Y = CONFIG_Props.BrightnessCurve_Max_Y;
    mem->properties.Brightness_LevelStandard = CONFIG_Props.Brightness_LevelStandard;
    mem->properties.Brightness_LevelEmergency = CONFIG_Props.Brightness_LevelEmergency;
    mem->properties.Brightness_LevelSafety = CONFIG_Props.Brightness_LevelSafety;
    mem->properties.Brightness_BrakeLow = CONFIG_Props.Brightness_BrakeLow;
    mem->properties.Brightness_BrakeHigh = CONFIG_Props.Brightness_BrakeHigh;
    mem->properties.Strobe_LevelLow = CONFIG_Props.Strobe_LevelLow;
    mem->properties.Strobe_LevelHigh = CONFIG_Props.Strobe_LevelHigh;
    mem->properties.Strobe_LevelEmergency = CONFIG_Props.Strobe_LevelEmergency;
    mem->properties.Strobe_LevelSafety = CONFIG_Props.Strobe_LevelSafety;
    mem->properties.Strobe_ModeDefault = CONFIG_ConvertStrobeSourceToConfig(CONFIG_Props.Strobe_ModeDefault);
    mem->properties.Strobe_ModePrimary = CONFIG_ConvertStrobeSourceToConfig(CONFIG_Props.Strobe_ModePrimary);
    mem->properties.Strobe_ModeEmergency = CONFIG_ConvertStrobeSourceToConfig(CONFIG_Props.Strobe_ModeEmergency);
    mem->properties.Strobe_ModeSafety = CONFIG_ConvertStrobeSourceToConfig(CONFIG_Props.Strobe_ModeSafety);
    mem->properties.Strobe_SingleOnTime = CONFIG_Props.Strobe_SingleOnTime;
    mem->properties.Strobe_SingleOffTime = CONFIG_Props.Strobe_SingleOffTime;
    mem->properties.Strobe_RapidOnTime = CONFIG_Props.Strobe_RapidOnTime;
    mem->properties.Strobe_RapidOffTime = CONFIG_Props.Strobe_RapidOffTime;
    mem->properties.Driver_TailRefCurrent = CONFIG_Props.Driver_TailRefCurrent;
    mem->properties.Driver_BrakeRefCurrent = CONFIG_Props.Driver_BrakeRefCurrent;
    mem->properties.Driver_TailRefVoltage = CONFIG_Props.Driver_TailRefVoltage;
    mem->properties.Driver_BrakeRefVoltage = CONFIG_Props.Driver_BrakeRefVoltage;
    mem->properties.Driver_TailEfficiency = CONFIG_Props.Driver_TailEfficiency;
    mem->properties.Driver_BrakeEfficiency = CONFIG_Props.Driver_BrakeEfficiency;

    for (uint8_t i = 0; i < sizeof(mem->padding); i++) {
        mem->padding[i] = 0xFF;
    }
    mem->crc32 = DSU_SoftwareCRC32(DSU_CRC32_INITIAL, (void*)(&CONFIG_Shadow), CONFIG_FLASH_SIZE-4u);
}

void CONFIG_LoadNvram(void) {
    uint32_t crc = DSU_CalculateCRC32(DSU_CRC32_INITIAL,
                                      (void*)CONFIG_FLASH_ADDRESS,
                                      CONFIG_FLASH_SIZE-4u);

    if (crc == CONFIG_Memory->crc32) {
        if (CONFIG_Memory->version == 1) {
            CONFIG_LoadProperties_v1();
        }
        else {
            CONFIG_ErrorFlags.VersionInvalid = 1;
        }
    }
    else {
        CONFIG_ErrorFlags.CrcInvalid = 1;
    }
}

void CONFIG_Reload(void) {
    CONFIG_Props.AutomaticDiagnostics = UDS_RearLight_Properties.AutomaticDiagnostics;
    CONFIG_Props.BrightnessCurve_Cutoff_X = UDS_RearLight_Properties.BrightnessCurve_Cutoff_X;
    CONFIG_Props.BrightnessCurve_Cutoff_Y = UDS_RearLight_Properties.BrightnessCurve_Cutoff_Y;
    CONFIG_Props.BrightnessCurve_Max_X = UDS_RearLight_Properties.BrightnessCurve_Max_X;
    CONFIG_Props.BrightnessCurve_Max_Y = UDS_RearLight_Properties.BrightnessCurve_Max_Y;
    CONFIG_Props.Brightness_LevelStandard = UDS_RearLight_Properties.Brightness_LevelStandard;
    CONFIG_Props.Brightness_LevelEmergency = UDS_RearLight_Properties.Brightness_LevelEmergency;
    CONFIG_Props.Brightness_LevelSafety = UDS_RearLight_Properties.Brightness_LevelSafety;
    CONFIG_Props.Brightness_BrakeLow = UDS_RearLight_Properties.Brightness_BrakeLow;
    CONFIG_Props.Brightness_BrakeHigh = UDS_RearLight_Properties.Brightness_BrakeHigh;
    CONFIG_Props.Strobe_LevelLow = UDS_RearLight_Properties.Strobe_LevelLow;
    CONFIG_Props.Strobe_LevelHigh = UDS_RearLight_Properties.Strobe_LevelHigh;
    CONFIG_Props.Strobe_LevelEmergency = UDS_RearLight_Properties.Strobe_LevelEmergency;
    CONFIG_Props.Strobe_LevelSafety = UDS_RearLight_Properties.Strobe_LevelSafety;
    CONFIG_Props.Strobe_ModeDefault = CONFIG_ToStrobeSource(UDS_RearLight_Properties.Strobe_ModeDefault);
    CONFIG_Props.Strobe_ModePrimary = CONFIG_ToStrobeSource(UDS_RearLight_Properties.Strobe_ModePrimary);
    CONFIG_Props.Strobe_ModeEmergency = CONFIG_ToStrobeSource(UDS_RearLight_Properties.Strobe_ModeEmergency);
    CONFIG_Props.Strobe_ModeSafety = CONFIG_ToStrobeSource(UDS_RearLight_Properties.Strobe_ModeSafety);
    CONFIG_Props.Strobe_SingleOnTime = UDS_RearLight_Properties.Strobe_SingleOnTime;
    CONFIG_Props.Strobe_SingleOffTime = UDS_RearLight_Properties.Strobe_SingleOffTime;
    CONFIG_Props.Strobe_RapidOnTime = UDS_RearLight_Properties.Strobe_RapidOnTime;
    CONFIG_Props.Strobe_RapidOffTime = UDS_RearLight_Properties.Strobe_RapidOffTime;
    CONFIG_Props.Driver_TailRefCurrent = UDS_RearLight_Properties.Driver_TailRefCurrent;
    CONFIG_Props.Driver_BrakeRefCurrent = UDS_RearLight_Properties.Driver_BrakeRefCurrent;
    CONFIG_Props.Driver_TailRefVoltage = UDS_RearLight_Properties.Driver_TailRefVoltage;
    CONFIG_Props.Driver_BrakeRefVoltage = UDS_RearLight_Properties.Driver_BrakeRefVoltage;
    CONFIG_Props.Driver_TailEfficiency = UDS_RearLight_Properties.Driver_TailEfficiency;
    CONFIG_Props.Driver_BrakeEfficiency = UDS_RearLight_Properties.Driver_BrakeEfficiency;
}

void CONFIG_ReloadComponents(void) {
    SYSSTATE_LoadConfig();
    BRIGHTNESS_LoadConfig();
    STROBE_LoadConfig();
    CURRENT_LoadConfig();
}

void CONFIG_Save(void) {
    if (CONFIG_Changed) {
        CONFIG_CopyPropsToShadow_v1();
        NVMCTRL_EraseRow(CONFIG_FLASH_ADDRESS);
        NVMCTRL_WritePages(CONFIG_FLASH_ADDRESS, (uint8_t*)(&CONFIG_Shadow), CONFIG_FLASH_SIZE / 64u);

        CONFIG_Changed = false;
    }
}

void CONFIG_NotifyPropertyChange(const uds_property_t* property)
{
    if (property->storage_class == uds_storage_class_persistent) {
        CONFIG_Changed = true;
    }
}
