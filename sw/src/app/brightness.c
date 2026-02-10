#include "app/brightness.h"

// Board support package
#include "bsp/light_control.h"

// Application components
#include "app/config.h"
#include "uds_gen.h"

static brightness_mode_t brightness_mode;
static uint16_t brightness_target;
static bool brightness_brake;
static bool brightness_strobe;

static uint16_t brightness_config_cutoff_x;
static uint16_t brightness_config_cutoff_y;
static uint16_t brightness_config_max_x;
static uint16_t brightness_config_max_y;
static uint16_t brightness_config_standard_level;
static uint16_t brightness_config_emergency_level;
static uint16_t brightness_config_safety_level;
static uint16_t brightness_config_brake_low;
static uint16_t brightness_config_brake_high;

static uint16_t brightness_config_strobe_low;
static uint16_t brightness_config_strobe_high;

static uint16_t brightness_config_strobe_emergency;
static uint16_t brightness_config_strobe_safety;

void BRIGHTNESS_LoadConfig(void) {
    brightness_config_cutoff_x = UDS_Properties_RearLight.BrightnessCurve_Cutoff_X;
    brightness_config_cutoff_y = UDS_Properties_RearLight.BrightnessCurve_Cutoff_Y;
    brightness_config_max_x = UDS_Properties_RearLight.BrightnessCurve_Max_X;
    brightness_config_max_y = UDS_Properties_RearLight.BrightnessCurve_Max_Y;
    brightness_config_standard_level = UDS_Properties_RearLight.Brightness_LevelStandard;
    brightness_config_emergency_level = UDS_Properties_RearLight.Brightness_LevelEmergency;
    brightness_config_safety_level = UDS_Properties_RearLight.Brightness_LevelSafety;
    brightness_config_brake_low = UDS_Properties_RearLight.Brightness_BrakeLow;
    brightness_config_brake_high = UDS_Properties_RearLight.Brightness_BrakeHigh;
    brightness_config_strobe_low = UDS_Properties_RearLight.Strobe_LevelLow;
    brightness_config_strobe_high = UDS_Properties_RearLight.Strobe_LevelHigh;
    brightness_config_strobe_emergency = UDS_Properties_RearLight.Strobe_LevelEmergency;
    brightness_config_strobe_safety = UDS_Properties_RearLight.Strobe_LevelSafety;
}

void BRIGHTNESS_Init(void) {
    brightness_mode = brightness_mode_max;
    brightness_target = 1000;
    brightness_brake = false;
    brightness_strobe = true;

    BRIGHTNESS_LoadConfig();
}

static uint16_t BRIGHTNESS_MapStrobe(uint16_t target) {
    if (target <= LIGHTCONTROL_BRIGHTNESS_MIN) {
        return brightness_config_strobe_low;
    }
    else if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        return brightness_config_strobe_high;
    }
    else {
        return (target - LIGHTCONTROL_BRIGHTNESS_MIN) * (brightness_config_strobe_high - brightness_config_strobe_low) /
               (LIGHTCONTROL_BRIGHTNESS_MAX - LIGHTCONTROL_BRIGHTNESS_MIN) + brightness_config_strobe_low;
    }
}

static uint16_t BRIGHTNESS_MapBrake(uint16_t target) {
    if (target <= LIGHTCONTROL_BRIGHTNESS_MIN) {
        return brightness_config_brake_low;
    }
    else if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        return brightness_config_brake_high;
    }
    else {
        return (target - LIGHTCONTROL_BRIGHTNESS_MIN) * (brightness_config_brake_high - brightness_config_brake_low) /
               (LIGHTCONTROL_BRIGHTNESS_MAX - LIGHTCONTROL_BRIGHTNESS_MIN) + brightness_config_brake_low;
    }
}

static uint16_t BRIGHTNESS_MapTargetAdaptive(uint16_t target) {
    if (target < CONFIG_Props.BrightnessCurve_Cutoff_X) {
        return LIGHTCONTROL_BRIGHTNESS_MIN;
    }
    else if (target >= CONFIG_Props.BrightnessCurve_Max_X) {
        return CONFIG_Props.BrightnessCurve_Max_Y;
    }
    else {
        return (target - CONFIG_Props.BrightnessCurve_Cutoff_X) * (CONFIG_Props.BrightnessCurve_Max_Y - CONFIG_Props.BrightnessCurve_Cutoff_Y) /
               (CONFIG_Props.BrightnessCurve_Max_X - CONFIG_Props.BrightnessCurve_Cutoff_X) + CONFIG_Props.BrightnessCurve_Cutoff_Y;
    }
}

void BRIGHTNESS_Update10ms(void) {
    if (brightness_mode == brightness_mode_off) {
        /* And tail light is disabled as well */
        LIGHTCONTROL_SetBrightness(LIGHTCONTROL_BRIGHTNESS_MIN);

        // TODO: brake light can actually come on
    }
    else if (brightness_mode == brightness_mode_standard || brightness_mode == brightness_mode_adaptive) {
        uint16_t tail_target = BRIGHTNESS_MapTargetAdaptive(brightness_target);
        uint16_t brake_target = BRIGHTNESS_MapBrake(brightness_target);
        uint16_t strobe_target = BRIGHTNESS_MapStrobe(brightness_target);

        /* In standard mode the rear light is in daylight running mode */
        if ((brightness_mode == brightness_mode_standard || brightness_brake) && tail_target < CONFIG_Props.Brightness_LevelStandard) {
            tail_target = CONFIG_Props.Brightness_LevelStandard;
        }

        if (brightness_brake) {
            tail_target = brake_target;
            brake_target = brake_target;
        }
        else {
            brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
            // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
            if (!brightness_strobe) {
                tail_target = strobe_target;
            }
        }
        LIGHTCONTROL_SetBrightness(tail_target);
    }
    else if (brightness_mode == brightness_mode_emergency) {
        /* And tail light is set to emergency brightness */
        LIGHTCONTROL_SetBrightness(CONFIG_Props.Brightness_LevelEmergency);
    }
    else if (brightness_mode == brightness_mode_safety) {
        uint16_t tail_target = CONFIG_Props.Brightness_LevelSafety;
        uint16_t brake_target = BRIGHTNESS_MapBrake(brightness_target);
        uint16_t strobe_target = BRIGHTNESS_MapStrobe(brightness_target);

        if (brightness_brake) {
            tail_target = brake_target;
            brake_target = brake_target;
        }
        else {
            brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
            // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
            if (!brightness_strobe) {
                tail_target = strobe_target;
            }
        }

        LIGHTCONTROL_SetBrightness(tail_target);
    }
    else if (brightness_mode == brightness_mode_max) {
        /* In max mode all segments are set to their hardware default level */
        LIGHTCONTROL_SetBrightness(LIGHTCONTROL_BRIGHTNESS_MAX);
    }
    else {
        // Control should never reach this scenario
        LIGHTCONTROL_SetBrightness(LIGHTCONTROL_BRIGHTNESS_MAX);
    }
}

void BRIGHTNESS_SetMode(brightness_mode_t mode) {
    brightness_mode = mode;
}

brightness_mode_t BRIGHTNESS_GetMode(void) {
    return brightness_mode;
}

void BRIGHTNESS_SetTarget(uint16_t target) {
    brightness_target = target;
}

void BRIGHTNESS_SetBraking(bool brake) {
    brightness_brake = brake;
}

void BRIGHTNESS_Strobe(bool strobe) {
    brightness_strobe = strobe;
}
