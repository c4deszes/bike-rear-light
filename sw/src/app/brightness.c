#include "app/brightness.h"
#include "app/config.h"

#include "bsp/light_control.h"
#include "uds_gen.h"

static brightness_mode_t brightness_mode;
static uint16_t brightness_target;
static bool brightness_brake;
static bool brightness_strobe;

void BRIGHTNESS_Init(void) {
    brightness_mode = brightness_mode_max;
    brightness_target = 1000;
    brightness_brake = false;
    brightness_strobe = true;
}

static uint16_t BRIGHTNESS_MapStrobe(uint16_t target) {
    if (target <= LIGHTCONTROL_BRIGHTNESS_MIN) {
        return CONFIG_Props.Strobe_LevelLow;
    }
    else if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        return CONFIG_Props.Strobe_LevelHigh;
    }
    else {
        return (target - LIGHTCONTROL_BRIGHTNESS_MIN) * (CONFIG_Props.Strobe_LevelHigh - CONFIG_Props.Strobe_LevelLow) /
               (LIGHTCONTROL_BRIGHTNESS_MAX - LIGHTCONTROL_BRIGHTNESS_MIN) + CONFIG_Props.Strobe_LevelLow;
    }
}

static uint16_t BRIGHTNESS_MapBrake(uint16_t target) {
    if (target <= LIGHTCONTROL_BRIGHTNESS_MIN) {
        return CONFIG_Props.Brightness_BrakeLow;
    }
    else if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        return CONFIG_Props.Brightness_BrakeHigh;
    }
    else {
        return (target - LIGHTCONTROL_BRIGHTNESS_MIN) * (CONFIG_Props.Brightness_BrakeHigh - CONFIG_Props.Brightness_BrakeLow) /
               (LIGHTCONTROL_BRIGHTNESS_MAX - LIGHTCONTROL_BRIGHTNESS_MIN) + CONFIG_Props.Brightness_BrakeLow;
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
        /* In off mode the brake light is disabled*/
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, LIGHTCONTROL_BRIGHTNESS_MIN);

        /* And tail light is disabled as well */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, LIGHTCONTROL_BRIGHTNESS_MIN);
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

        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, brake_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, tail_target);
    }
    else if (brightness_mode == brightness_mode_emergency) {
        /* In emergency mode the brake light is disabled */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, LIGHTCONTROL_BRIGHTNESS_MIN);

        /* And tail light is set to emergency brightness */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, CONFIG_Props.Brightness_LevelEmergency);
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

        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, brake_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, tail_target);
    }
    else if (brightness_mode == brightness_mode_max) {
        /* In max mode all segments are set to their hardware default level */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, LIGHTCONTROL_BRIGHTNESS_MIN);
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, LIGHTCONTROL_BRIGHTNESS_MAX);
    }
    else {
        // Control should never reach this scenario
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, LIGHTCONTROL_BRIGHTNESS_MIN);
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, LIGHTCONTROL_BRIGHTNESS_MAX);
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
