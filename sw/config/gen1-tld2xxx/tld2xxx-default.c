#include "app/config_priv.h"

#include "bsp/light_control.h"

CONFIG_ATTR const config_layout_v1_t config_default = {
    .properties = {
        .AutomaticDiagnostics = true,

        .BrightnessCurve_Cutoff_X = 50,
        .BrightnessCurve_Cutoff_Y = 200,
        .BrightnessCurve_Max_X = 800,
        .BrightnessCurve_Max_Y = 1000,
        .Brightness_LevelStandard = 250,
        .Brightness_LevelEmergency = 500,
        .Brightness_LevelSafety = 700,

        .Brightness_BrakeLow = 400,
        .Brightness_BrakeHigh = LIGHTCONTROL_BRIGHTNESS_MAX,

        .Strobe_LevelLow = 0,
        .Strobe_LevelHigh = 0,

        .Strobe_ModeDefault = CONFIG_STROBE_MODE_DISABLED,
        .Strobe_ModePrimary = CONFIG_STROBE_MODE_INTERNAL_SINGLE,
        .Strobe_ModeEmergency = CONFIG_STROBE_MODE_DISABLED,
        .Strobe_ModeSafety = CONFIG_STROBE_MODE_INTERNAL_RAPID,

        .Strobe_SingleOnTime = 160,
        .Strobe_SingleOffTime = 160,

        .Strobe_RapidOnTime = 80,
        .Strobe_RapidOffTime = 300,

        .Driver_TailRefCurrent = 60,    // 3x 20mA
        .Driver_BrakeRefCurrent = 20,   // 1x 20mA
        .Driver_TailRefVoltage = 60,    // 6.0V
        .Driver_BrakeRefVoltage = 60,   // 6.0V
        .Driver_TailEfficiency = 100,   // Uses linear drivers, efficiency is not applicable
        .Driver_BrakeEfficiency = 100   // Uses linear drivers, efficiency is not applicable
    },
    .padding = {[0 ... sizeof(config_default.padding) - 1] = 0xFF},
    .crc32 = 0 // Will be calculated and filled in by the post-build step
};

