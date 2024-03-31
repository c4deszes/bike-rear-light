#include "app/brightness.h"
#include "app/config.h"

#include "bsp/light_control.h"

static brightness_mode_t brightness_mode;
static uint16_t brightness_target;
static bool brightness_brake;
static bool brightness_blinking;
static bool brightness_strobe;

void BRIGHTNESS_Init(void) {
    brightness_mode = brightness_mode_max;
    brightness_target = 1000;
    brightness_brake = false;
    brightness_blinking = false;
    brightness_strobe = true;
}

static uint16_t BRIGHTNESS_MapTargetAdaptive(uint16_t target) {
    if (target < CONFIG_BRIGHTNESS_CURVE_CUTOFF_X) {
        return LIGHTCONTROL_BRIGHTNESS_MIN;
    }
    else if (target >= CONFIG_BRIGHTNESS_CURVE_MAX_X) {
        return CONFIG_BRIGHTNESS_CURVE_MAX_Y;
    }
    else {
        return (target - CONFIG_BRIGHTNESS_CURVE_CUTOFF_X) * (CONFIG_BRIGHTNESS_CURVE_MAX_Y - CONFIG_BRIGHTNESS_CURVE_CUTOFF_Y) /
               (CONFIG_BRIGHTNESS_CURVE_MAX_X - CONFIG_BRIGHTNESS_CURVE_CUTOFF_X) + CONFIG_BRIGHTNESS_CURVE_CUTOFF_Y;
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
        uint16_t brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;

        /* In standard mode the rear light is in daylight running mode */
        if (brightness_mode == brightness_mode_standard && tail_target < CONFIG_BRIGHTNESS_STANDARD_MIN_LEVEL) {
            tail_target = CONFIG_BRIGHTNESS_STANDARD_MIN_LEVEL;
        }

        if (brightness_brake) {
            // TODO: in adaptive mode during braking the tail target should be at minimum CONFIG_BRIGHTNESS_STANDARD_MIN_LEVEL
            tail_target += CONFIG_BRIGHTNESS_BRAKE_OFFSET;
            brake_target = tail_target;
        }
        else {
            if (brightness_blinking) {
                // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
                if (brightness_strobe) {
                    tail_target += CONFIG_BRIGHTNESS_STROBE_HIGH_OFFSET;
                }
                else {
                    tail_target -= CONFIG_BRIGHTNESS_STROBE_LOW_OFFSET;
                }
            }
            else {
            }
        }

        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, brake_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, tail_target);
    }
    else if (brightness_mode == brightness_mode_emergency) {
        /* In emergency mode the brake light is disabled */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, LIGHTCONTROL_BRIGHTNESS_MIN);

        /* And tail light is set to emergency brightness */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, CONFIG_BRIGHTNESS_EMERGENCY_LEVEL);
    }
    else if (brightness_mode == brightness_mode_safety) {
        /* In emergency mode the brake light reacts to accelerometer data */
        // TODO: use strobe signal
        // TODO: use brake signal
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, LIGHTCONTROL_BRIGHTNESS_MIN);

        /* And tail light is set to safety brightness */
        LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, CONFIG_BRIGHTNESS_EMERGENCY_LEVEL);
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

void BRIGHTNESS_SetTarget(uint16_t target) {
    brightness_target = target;
}

void BRIGHTNESS_SetBraking(bool brake) {
    brightness_brake = brake;
}

void BRIGHTNESS_SetBlinking(bool blink) {
    brightness_blinking = blink;
}

void BRIGHTNESS_Strobe(bool strobe) {
    brightness_strobe = strobe;
}
