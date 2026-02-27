#include "app/brightness.h"

// Board support package
#include "bsp/light_control.h"

// Application components
#include "app/config.h"
#include "uds_gen.h"

static brightness_mode_t BRIGHTNESS_Mode;
static uint16_t BRIGHTNESS_Target;
static bool BRIGHTNESS_Brake;
static bool BRIGHTNESS_Strobe;

static uint16_t BRIGHTNESS_ConfCutoffX;
static uint16_t BRIGHTNESS_ConfCutoffY;
static uint16_t BRIGHTNESS_ConfMaxX;
static uint16_t BRIGHTNESS_ConfMaxY;
static uint16_t BRIGHTNESS_ConfLevelStandard;
static uint16_t BRIGHTNESS_ConfLevelEmergency;
static uint16_t BRIGHTNESS_ConfLevelSafety;
static uint16_t BRIGHTNESS_ConfBrakeLow;
static uint16_t BRIGHTNESS_ConfBrakeHigh;
static uint16_t BRIGHTNESS_ConfStrobeLow;
static uint16_t BRIGHTNESS_ConfStrobeHigh;
static uint16_t BRIGHTNESS_ConfStrobeEmergency;
static uint16_t BRIGHTNESS_ConfStrobeSafety;

void BRIGHTNESS_LoadConfig(void) {
    BRIGHTNESS_ConfCutoffX = CONFIG_Props.BrightnessCurve_Cutoff_X;
    BRIGHTNESS_ConfCutoffY = CONFIG_Props.BrightnessCurve_Cutoff_Y;
    BRIGHTNESS_ConfMaxX = CONFIG_Props.BrightnessCurve_Max_X;
    BRIGHTNESS_ConfMaxY = CONFIG_Props.BrightnessCurve_Max_Y;
    BRIGHTNESS_ConfLevelStandard = CONFIG_Props.Brightness_LevelStandard;
    BRIGHTNESS_ConfLevelEmergency = CONFIG_Props.Brightness_LevelEmergency;
    BRIGHTNESS_ConfLevelSafety = CONFIG_Props.Brightness_LevelSafety;
    BRIGHTNESS_ConfBrakeLow = CONFIG_Props.Brightness_BrakeLow;
    BRIGHTNESS_ConfBrakeHigh = CONFIG_Props.Brightness_BrakeHigh;
    BRIGHTNESS_ConfStrobeLow = CONFIG_Props.Strobe_LevelLow;
    BRIGHTNESS_ConfStrobeHigh = CONFIG_Props.Strobe_LevelHigh;
    BRIGHTNESS_ConfStrobeEmergency = CONFIG_Props.Strobe_LevelEmergency;
    BRIGHTNESS_ConfStrobeSafety = CONFIG_Props.Strobe_LevelSafety;
}

void BRIGHTNESS_Init(void) {
    BRIGHTNESS_Mode = brightness_mode_max;
    BRIGHTNESS_Target = LIGHTCONTROL_BRIGHTNESS_MAX;
    BRIGHTNESS_Brake = false;
    BRIGHTNESS_Strobe = true;

    BRIGHTNESS_LoadConfig();
}

static uint16_t BRIGHTNESS_MapStrobe(uint16_t target) {
    if (target <= LIGHTCONTROL_BRIGHTNESS_MIN) {
        return BRIGHTNESS_ConfStrobeLow;
    }
    else if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        return BRIGHTNESS_ConfStrobeHigh;
    }
    else {
        return (target - LIGHTCONTROL_BRIGHTNESS_MIN) * (BRIGHTNESS_ConfStrobeHigh - BRIGHTNESS_ConfStrobeLow) /
               (LIGHTCONTROL_BRIGHTNESS_MAX - LIGHTCONTROL_BRIGHTNESS_MIN) + BRIGHTNESS_ConfStrobeLow;
    }
}

static uint16_t BRIGHTNESS_MapBrake(uint16_t target) {
    if (target <= LIGHTCONTROL_BRIGHTNESS_MIN) {
        return BRIGHTNESS_ConfBrakeLow;
    }
    else if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        return BRIGHTNESS_ConfBrakeHigh;
    }
    else {
        return (target - LIGHTCONTROL_BRIGHTNESS_MIN) * (BRIGHTNESS_ConfBrakeHigh - BRIGHTNESS_ConfBrakeLow) /
               (LIGHTCONTROL_BRIGHTNESS_MAX - LIGHTCONTROL_BRIGHTNESS_MIN) + BRIGHTNESS_ConfBrakeLow;
    }
}

static uint16_t BRIGHTNESS_MapTargetAdaptive(uint16_t target) {
    if (target < BRIGHTNESS_ConfCutoffX) {
        return LIGHTCONTROL_BRIGHTNESS_MIN;
    }
    else if (target >= BRIGHTNESS_ConfMaxX) {
        return BRIGHTNESS_ConfMaxY;
    }
    else {
        return (target - BRIGHTNESS_ConfCutoffX) * (BRIGHTNESS_ConfMaxY - BRIGHTNESS_ConfCutoffY) /
               (BRIGHTNESS_ConfMaxX - BRIGHTNESS_ConfCutoffX) + BRIGHTNESS_ConfCutoffY;
    }
}

void BRIGHTNESS_Update10ms(void) {
    if (BRIGHTNESS_Mode == brightness_mode_off) {
        uint16_t brake_target = BRIGHTNESS_MapBrake(LIGHTCONTROL_BRIGHTNESS_MIN);
        
        if (!BRIGHTNESS_Brake) {
            brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
        }
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, brake_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, brake_target);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_standard || BRIGHTNESS_Mode == brightness_mode_adaptive) {
        uint16_t tail_target = BRIGHTNESS_MapTargetAdaptive(BRIGHTNESS_Target);
        uint16_t brake_target = BRIGHTNESS_MapBrake(BRIGHTNESS_Target);
        uint16_t strobe_target = BRIGHTNESS_MapStrobe(BRIGHTNESS_Target);

        /* In standard mode the rear light is in daylight running mode */
        if ((BRIGHTNESS_Mode == brightness_mode_standard || BRIGHTNESS_Brake) && tail_target < BRIGHTNESS_ConfLevelStandard) {
            tail_target = BRIGHTNESS_ConfLevelStandard;
        }

        if (BRIGHTNESS_Brake) {
            tail_target = brake_target;
            brake_target = brake_target;
        }
        else {
            brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
            // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
            if (!BRIGHTNESS_Strobe) {
                tail_target = strobe_target;
            }
        }
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, tail_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, brake_target);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_emergency) {
        /* And tail light is set to emergency brightness */
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, BRIGHTNESS_ConfLevelEmergency);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, LIGHTCONTROL_BRIGHTNESS_MIN);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_safety) {
        uint16_t tail_target = BRIGHTNESS_ConfLevelSafety;
        uint16_t brake_target = BRIGHTNESS_MapBrake(BRIGHTNESS_Target);
        uint16_t strobe_target = BRIGHTNESS_MapStrobe(BRIGHTNESS_Target);

        if (BRIGHTNESS_Brake) {
            tail_target = brake_target;
            brake_target = brake_target;
        }
        else {
            brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
            // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
            if (!BRIGHTNESS_Strobe) {
                tail_target = strobe_target;
            }
        }

        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, tail_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, brake_target);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_max) {
        /* In max mode tail segments is set to maximum brightness, brake light is disabled */
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, LIGHTCONTROL_BRIGHTNESS_MAX);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, LIGHTCONTROL_BRIGHTNESS_MIN);
    }
    else {
        // Control should never reach this scenario
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, LIGHTCONTROL_BRIGHTNESS_MAX);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, LIGHTCONTROL_BRIGHTNESS_MIN);
    }
}

void BRIGHTNESS_SetMode(brightness_mode_t mode) {
    BRIGHTNESS_Mode = mode;
}

brightness_mode_t BRIGHTNESS_GetMode(void) {
    return BRIGHTNESS_Mode;
}

void BRIGHTNESS_SetTarget(uint16_t target) {
    BRIGHTNESS_Target = target;
}

void BRIGHTNESS_SetBraking(bool brake) {
    BRIGHTNESS_Brake = brake;
}

void BRIGHTNESS_SetStrobe(bool strobe) {
    BRIGHTNESS_Strobe = strobe;
}
