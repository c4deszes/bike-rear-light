#include "app/config_priv.h"

#include "bsp/light_control.h"

CONFIG_ATTR const config_layout_v1_t config_default = {
    .version = 1,
    .properties = {
        .AutomaticDiagnostics = true,

        .BrightnessCurve_Cutoff_X = 50,
        .BrightnessCurve_Cutoff_Y = 50,
        .BrightnessCurve_Max_X = 900,
        .BrightnessCurve_Max_Y = 800,
        .Brightness_LevelStandard = 100,
        .Brightness_LevelEmergency = 200,
        .Brightness_LevelSafety = 300,

        .Brightness_BrakeLow = 200,
        .Brightness_BrakeHigh = LIGHTCONTROL_BRIGHTNESS_MAX,

        .Strobe_LevelLow = 0,
        .Strobe_LevelHigh = 0,
        .Strobe_LevelEmergency = 0,
        .Strobe_LevelSafety = 0,

        .Strobe_ModeDefault = CONFIG_STROBE_MODE_DISABLED,
        .Strobe_ModePrimary = CONFIG_STROBE_MODE_INTERNAL_SINGLE,
        .Strobe_ModeEmergency = CONFIG_STROBE_MODE_INTERNAL_SINGLE,
        .Strobe_ModeSafety = CONFIG_STROBE_MODE_INTERNAL_SINGLE,

        .Strobe_SingleOnTime = 160,
        .Strobe_SingleOffTime = 160,

        .Strobe_RapidOnTime = 120,
        .Strobe_RapidOffTime = 700,

        .Driver_TailRefCurrent = 700,   // 700mA
        .Driver_BrakeRefCurrent = 0,
        .Driver_TailRefVoltage = 19,    // 1.9V
        .Driver_BrakeRefVoltage = 19,   // 1.9V
        .Driver_TailEfficiency = 85,
        .Driver_BrakeEfficiency = 100
    },
    .padding = {[0 ... sizeof(config_default.padding) - 1] = 0xFF},
    .crc32 = 0 // Will be calculated and filled in by the post-build step
};

