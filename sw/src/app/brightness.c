#include "app/brightness.h"

// Board support package
#include "bsp/light_control.h"

// Application components
#include "app/config.h"
#include "app/calib.h"
#include "app/feature.h"
#include "app/current.h"

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

static uint16_t BRIGHTNESS_TailOutput;
static uint16_t BRIGHTNESS_BrakeOutput;
static uint16_t BRIGHTNESS_TurnOutput;

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
    if (target <= BRIGHTNESS_ConfCutoffX) {
        return BRIGHTNESS_ConfStrobeLow;
    }
    else if (target >= BRIGHTNESS_ConfMaxX) {
        return BRIGHTNESS_ConfStrobeHigh;
    }
    else {
        return (target - BRIGHTNESS_ConfCutoffX) * (BRIGHTNESS_ConfStrobeHigh - BRIGHTNESS_ConfStrobeLow) /
               (BRIGHTNESS_ConfMaxX - BRIGHTNESS_ConfCutoffX) + BRIGHTNESS_ConfStrobeLow;
    }
}

static uint16_t BRIGHTNESS_MapBrake(uint16_t target) {
    if (target <= BRIGHTNESS_ConfCutoffX) {
        return BRIGHTNESS_ConfBrakeLow;
    }
    else if (target >= BRIGHTNESS_ConfMaxX) {
        return BRIGHTNESS_ConfBrakeHigh;
    }
    else {
        return (target - BRIGHTNESS_ConfCutoffX) * (BRIGHTNESS_ConfBrakeHigh - BRIGHTNESS_ConfBrakeLow) /
               (BRIGHTNESS_ConfMaxX - BRIGHTNESS_ConfCutoffX) + BRIGHTNESS_ConfBrakeLow;
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

void BRIGHTNESS_OffMode(uint16_t* tail_target, uint16_t* brake_target, uint16_t* turn_target) {
    uint16_t target = LIGHTCONTROL_BRIGHTNESS_MIN;

    #if FEATURE_BRIGHTNESS_BRAKE_IN_OFF_MODE == 1
    if (BRIGHTNESS_Brake) {
        // TODO: in off mode target is ignored completely
        target = BRIGHTNESS_MapBrake(LIGHTCONTROL_BRIGHTNESS_MIN);
    }
    #endif

    *tail_target = target;
    *brake_target = target;
    *turn_target = LIGHTCONTROL_BRIGHTNESS_MIN;
}

void BRIGHTNESS_NormalMode(uint16_t* tail_target, uint16_t* brake_target, uint16_t* turn_target) {
    uint16_t temp_tail = BRIGHTNESS_MapTargetAdaptive(BRIGHTNESS_Target);
    uint16_t temp_brake = BRIGHTNESS_MapBrake(BRIGHTNESS_Target);
    uint16_t temp_strobe = BRIGHTNESS_MapStrobe(BRIGHTNESS_Target);

    /* In standard mode the rear light is in daylight running mode */
    if ((BRIGHTNESS_Mode == brightness_mode_standard || BRIGHTNESS_Brake) && temp_tail < BRIGHTNESS_ConfLevelStandard) {
        temp_tail = BRIGHTNESS_ConfLevelStandard;
    }

    if (BRIGHTNESS_Brake) {
        temp_tail = temp_brake;
        temp_brake = temp_brake;
    }
    else {
        temp_brake = LIGHTCONTROL_BRIGHTNESS_MIN;
        uint16_t temp_strobe = BRIGHTNESS_MapStrobe(BRIGHTNESS_Target);
        // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
        if (!BRIGHTNESS_Strobe) {
            temp_tail = temp_strobe;
        }
    }
    *tail_target = temp_tail;
    *brake_target = temp_brake;
    *turn_target = LIGHTCONTROL_BRIGHTNESS_MIN;
}

void BRIGHTNESS_EmergencyMode(uint16_t* tail_target, uint16_t* brake_target, uint16_t* turn_target) {
    uint16_t temp_tail = BRIGHTNESS_ConfLevelEmergency;
    uint16_t temp_strobe = BRIGHTNESS_ConfStrobeEmergency;

    // TODO: support for braking in emergency mode
    if (!BRIGHTNESS_Strobe) {
        temp_tail = temp_strobe;
    }

    *tail_target = temp_tail;
    *brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
    *turn_target = LIGHTCONTROL_BRIGHTNESS_MIN;
}

void BRIGHTNESS_SafetyMode(uint16_t* tail_target, uint16_t* brake_target, uint16_t* turn_target) {
    uint16_t temp_tail = BRIGHTNESS_ConfLevelSafety;
    uint16_t temp_brake = BRIGHTNESS_MapBrake(BRIGHTNESS_Target);
    uint16_t temp_strobe = BRIGHTNESS_MapStrobe(BRIGHTNESS_Target);

    // TODO: feature toggle FEATURE_BRIGHTNESS_BRAKE_IN_SAFETY_MODE
    if (BRIGHTNESS_Brake) {
        temp_tail = temp_brake;
        temp_brake = temp_brake;
    }
    else {
        temp_brake = LIGHTCONTROL_BRIGHTNESS_MIN;
        // TODO: when blinking the output should be coordinated so that the blinking resumes only well after braking stopped
        if (!BRIGHTNESS_Strobe) {
            temp_tail = temp_strobe;
        }
    }

    *tail_target = temp_tail;
    *brake_target = temp_brake;
    *turn_target = LIGHTCONTROL_BRIGHTNESS_MIN;
}

void BRIGHTNESS_MaxMode(uint16_t* tail_target, uint16_t* brake_target, uint16_t* turn_target) {
    *tail_target = LIGHTCONTROL_BRIGHTNESS_MAX;
    *brake_target = LIGHTCONTROL_BRIGHTNESS_MIN;
    *turn_target = LIGHTCONTROL_BRIGHTNESS_MIN;
}

void BRIGHTNESS_Update10ms(void) {

    uint16_t temp_tail, temp_brake, temp_turn;

    if (BRIGHTNESS_Mode == brightness_mode_off) {
        BRIGHTNESS_OffMode(&temp_tail, &temp_brake, &temp_turn);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_standard || BRIGHTNESS_Mode == brightness_mode_adaptive) {
        BRIGHTNESS_NormalMode(&temp_tail, &temp_brake, &temp_turn);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_emergency) {
        BRIGHTNESS_EmergencyMode(&temp_tail, &temp_brake, &temp_turn);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_safety) {
        BRIGHTNESS_SafetyMode(&temp_tail, &temp_brake, &temp_turn);
    }
    else if (BRIGHTNESS_Mode == brightness_mode_max) {
        /* In max mode tail segments is set to maximum brightness, brake light is disabled */
        BRIGHTNESS_MaxMode(&temp_tail, &temp_brake, &temp_turn);
    }
    else {
        // Control should never reach this scenario
        BRIGHTNESS_MaxMode(&temp_tail, &temp_brake, &temp_turn);
    }

    // Calibrate
    // TODO: implement

    // Derate final output
    uint16_t derating_factor = CURRENT_GetDeratingFactor();

    BRIGHTNESS_BrakeOutput = ((uint32_t)temp_brake * derating_factor) / CURRENT_DERATING_NONE;
    BRIGHTNESS_TailOutput = ((uint32_t)temp_tail * derating_factor) / CURRENT_DERATING_NONE;
    BRIGHTNESS_TurnOutput = ((uint32_t)temp_turn * derating_factor) / CURRENT_DERATING_NONE;
}

void BRIGHTNESS_SetMode(brightness_mode_t mode) {
    BRIGHTNESS_Mode = mode;
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

uint16_t BRIGHTNESS_GetOutput(brightness_output_t output)
{
    switch (output) {
        case brightness_output_tail:
            return BRIGHTNESS_TailOutput;
        case brightness_output_brake:
            return BRIGHTNESS_BrakeOutput;
        case brightness_output_turn:
            return BRIGHTNESS_TurnOutput;
        default:
            return LIGHTCONTROL_BRIGHTNESS_MIN;
    }
}