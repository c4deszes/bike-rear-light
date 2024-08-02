#include "bsp/pinout.h"

#include "bsp/light_control.h"
#include "hal/tcc.h"
#include "common/swtimer.h"

#define LIGHT_DIAG_ENABLE_DURATION 10
#define LIGHT_DIAG_CHANNEL_DURATION 10
#define LIGHT_DIAG_POST_DURATION 10

static const gpio_pin_input_configuration input = {
    .pull = FLOATING,
    .sample = CONTINUOUS
};

static const gpio_pin_output_configuration output = {
    .drive = NORMAL,
    .input = false
};

static tcc_channel_setting_t pwm_channels[4];

typedef enum {
    lightcontrol_tail_diag_state_stopped,
    lightcontrol_tail_diag_state_enable,
    lightcontrol_tail_diag_state_in1,
    lightcontrol_tail_diag_state_in1_post,
    lightcontrol_tail_diag_state_in2,
    lightcontrol_tail_diag_state_in2_post,
    lightcontrol_tail_diag_state_in3,
    lightcontrol_tail_diag_state_in3_post
} lightcontrol_tail_diag_state_t;

typedef enum {
    lightcontrol_brake_diag_state_stopped,
    lightcontrol_brake_diag_state_enable,
    lightcontrol_brake_diag_state_in1,
    lightcontrol_brake_diag_state_in1_post
} lightcontrol_brake_diag_state_t;

static uint16_t tail_brightness;
static bool tail_diag_complete;
static uint8_t tail_segment_states;
static swtimer_t* taillight_diag_timer;
static lightcontrol_tail_diag_state_t taillight_diag_state;
static lightcontrol_feature_state_t taillight_state;

static uint16_t brake_brightness;
static bool brake_diag_complete;
static swtimer_t* brakelight_diag_timer;
static lightcontrol_brake_diag_state_t brakelight_diag_state;
static lightcontrol_feature_state_t brakelight_state;

static void TAILLIGHT_Init(void) {
    GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, &output);

    GPIO_SetupPinInput(TLD2331_ERROR_PORT, TLD2331_ERROR_PIN, &input);

    /* PWM is Low active, by default enabling the taillight element */
    GPIO_PinWrite(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, LOW);
    GPIO_SetupPinOutput(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, &output);
    GPIO_EnableFunction(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, TLD2331_PWMI_PINMUX);

    GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, &output);

    GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, &output);

    GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, &output);
}

static void BRAKELIGHT_Init(void) {
    GPIO_SetupPinOutput(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, &output);
    GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, LOW);

    GPIO_SetupPinInput(TLD2132_ERROR_PORT, TLD2132_ERROR_PIN, &input);

    /* PWM is low active, by default disabling the brakelight element */
    GPIO_PinWrite(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, &output);
    GPIO_EnableFunction(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, TLD2132_PWMI_PINMUX);

    GPIO_PinWrite(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, &output);
}

static void PWM_TIMER_Setup(void) {
    TCC_Reset(TCC1);

    pwm_channels[TLD2331_PWMI_WO].cc = LIGHTCONTROL_BRIGHTNESS_MAX;
    pwm_channels[TLD2331_PWMI_WO].drv_inv = true;
    pwm_channels[TLD2132_PWMI_WO].cc = LIGHTCONTROL_BRIGHTNESS_MIN;   // TODO: try PWM set to MIN
    pwm_channels[TLD2132_PWMI_WO].drv_inv = true;

    TCC_SetupNormalPwm(TCC1, 999, pwm_channels);
    TCC_Enable(TCC1);
}

static void DIAG_Init(void) {
    tail_diag_complete = false;
    tail_segment_states = 0;
    taillight_diag_timer = SWTIMER_Create();
    taillight_diag_state = lightcontrol_tail_diag_state_stopped;
    taillight_state = lightcontrol_feature_state_ok;

    brake_diag_complete = false;
    brakelight_diag_timer = SWTIMER_Create();
    brakelight_diag_state = lightcontrol_brake_diag_state_stopped;
    brakelight_state = lightcontrol_feature_state_ok;
}

void LIGHTCONTROL_Init() {
    PWM_TIMER_Setup();

    TAILLIGHT_Init();
    BRAKELIGHT_Init();

    DIAG_Init();
}

void LIGHTCONTROL_SetBrightness(lightcontrol_feature_t feature, uint16_t brightness) {
    // TODO: clamp brightness, if needed disable PWM function and use high/low for 100% / 0%
    // TODO: check if PWM 0 and PWM 100% are achievable

    // TODO: might prefer this just being a setter function and doing the actual work in Update10ms
    if (feature == lightcontrol_feature_tail_segment) {
        tail_brightness = brightness;
    }
    else if(feature == lightcontrol_feature_brake_segment) {
        brake_brightness = brightness;
    }
}

static void LIGHTCONTROL_Taillight_DiagRun(void) {
    if (SWTIMER_Elapsed(taillight_diag_timer)) {
        if (taillight_diag_state == lightcontrol_tail_diag_state_stopped) {

        }
        else if (taillight_diag_state == lightcontrol_tail_diag_state_enable) {
            tail_segment_states = 0;
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
            GPIO_PinWrite(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, LOW);
            GPIO_DisableFunction(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, LOW);

            taillight_diag_state = lightcontrol_tail_diag_state_in1;
            SWTIMER_Setup(taillight_diag_timer, LIGHT_DIAG_ENABLE_DURATION);
        }
        else if (taillight_diag_state == lightcontrol_tail_diag_state_in1) {
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, LOW);

            taillight_diag_state = lightcontrol_tail_diag_state_in1_post;
            SWTIMER_Setup(taillight_diag_timer, LIGHT_DIAG_CHANNEL_DURATION);
        }
        else if (taillight_diag_state == lightcontrol_tail_diag_state_in1_post) {
            if (GPIO_PinRead(TLD2331_ERROR_PORT, TLD2331_ERROR_PIN) == LOW) {
                tail_segment_states |= (1 << 0);
            }

            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, LOW);

            taillight_diag_state = lightcontrol_tail_diag_state_in2;
            SWTIMER_Setup(taillight_diag_timer, LIGHT_DIAG_POST_DURATION);
        }
        else if (taillight_diag_state == lightcontrol_tail_diag_state_in2) {
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, LOW);

            taillight_diag_state = lightcontrol_tail_diag_state_in2_post;
            SWTIMER_Setup(taillight_diag_timer, LIGHT_DIAG_CHANNEL_DURATION);
        }
        else if (taillight_diag_state == lightcontrol_tail_diag_state_in2_post) {
            if (GPIO_PinRead(TLD2331_ERROR_PORT, TLD2331_ERROR_PIN) == LOW) {
                tail_segment_states |= (1 << 1);
            }

            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, LOW);

            taillight_diag_state = lightcontrol_tail_diag_state_in3;
            SWTIMER_Setup(taillight_diag_timer, LIGHT_DIAG_POST_DURATION);
        }
        else if (taillight_diag_state == lightcontrol_tail_diag_state_in3) {
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, LOW);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, HIGH);

            taillight_diag_state = lightcontrol_tail_diag_state_in3_post;
            SWTIMER_Setup(taillight_diag_timer, LIGHT_DIAG_CHANNEL_DURATION);
        } else if (taillight_diag_state == lightcontrol_tail_diag_state_in3_post) {
            if (GPIO_PinRead(TLD2331_ERROR_PORT, TLD2331_ERROR_PIN) == LOW) {
                tail_segment_states |= (1 << 2);
            }

            if (tail_segment_states == 0U) {
                taillight_state = lightcontrol_feature_state_ok;
            }
            else if (tail_segment_states == 0b111U) {
                taillight_state = lightcontrol_feature_state_error;
            }
            else {
                taillight_state = lightcontrol_feature_state_partial_error;
            }
            tail_diag_complete = true;
            taillight_diag_state = lightcontrol_tail_diag_state_stopped;

            /* Restoring original state */
            LIGHTCONTROL_SetBrightness(lightcontrol_feature_tail_segment, tail_brightness);

            GPIO_PinWrite(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, LOW);
            GPIO_EnableFunction(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, TLD2331_PWMI_PINMUX);
            GPIO_PinWrite(TLD2331_INSET1_PORT, TLD2331_INSET1_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET2_PORT, TLD2331_INSET2_PIN, HIGH);
            GPIO_PinWrite(TLD2331_INSET3_PORT, TLD2331_INSET3_PIN, HIGH);
        }
        else {
            // Handle unknown state
        }
    }
}

static void LIGHTCONTROL_Brakelight_DiagRun(void) {
    if (SWTIMER_Elapsed(brakelight_diag_timer)) {
        if (brakelight_diag_state == lightcontrol_brake_diag_state_stopped) {

        }
        else if (brakelight_diag_state == lightcontrol_brake_diag_state_enable) {
            GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, HIGH);
            GPIO_PinWrite(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, LOW);
            GPIO_DisableFunction(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN);
            GPIO_PinWrite(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, LOW);

            brakelight_diag_state = lightcontrol_brake_diag_state_in1;
            SWTIMER_Setup(brakelight_diag_timer, LIGHT_DIAG_ENABLE_DURATION);
        }
        else if (brakelight_diag_state == lightcontrol_brake_diag_state_in1) {
            GPIO_PinWrite(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, HIGH);

            brakelight_diag_state = lightcontrol_brake_diag_state_in1_post;
            SWTIMER_Setup(brakelight_diag_timer, LIGHT_DIAG_CHANNEL_DURATION);
        }
        else if (brakelight_diag_state == lightcontrol_brake_diag_state_in1_post) {
            if (GPIO_PinRead(TLD2132_ERROR_PORT, TLD2132_ERROR_PIN) == LOW) {
                brakelight_state = lightcontrol_feature_state_error;
            }
            brake_diag_complete = true;
            brakelight_diag_state = lightcontrol_brake_diag_state_stopped;

            /* Restoring original state */
            LIGHTCONTROL_SetBrightness(lightcontrol_feature_brake_segment, brake_brightness);

            GPIO_PinWrite(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, HIGH);
            GPIO_EnableFunction(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, TLD2132_PWMI_PINMUX);
            GPIO_PinWrite(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, HIGH);
        }
    }
}

void LIGHTCONTROL_Update10ms(void) {
    // If diagnostics was not run yet or diagnostics indicate that 
    // this logic makes sure that errors are detected but for example in the case of a partial failure (single led short or single channel short/open)
    // the partial error state stays until diagnostics indicate otherwise
    if ((!tail_diag_complete || taillight_state == lightcontrol_feature_state_ok) && GPIO_PinRead(TLD2331_ERROR_PORT, TLD2331_ERROR_PIN) == LOW) {
        taillight_state = lightcontrol_feature_state_error;
    }

    if ((!brake_diag_complete || brakelight_state  == lightcontrol_feature_state_ok) && GPIO_PinRead(TLD2132_ERROR_PORT, TLD2132_ERROR_PIN) == LOW) {
        brakelight_state = lightcontrol_feature_state_error;
    }

    LIGHTCONTROL_Taillight_DiagRun();
    LIGHTCONTROL_Brakelight_DiagRun();

    if (taillight_diag_state == lightcontrol_tail_diag_state_stopped) {
        TCC_SetCompareCapture(TCC1, TLD2331_PWMI_WO, tail_brightness);

        if (tail_brightness == LIGHTCONTROL_BRIGHTNESS_MIN) {
            // TODO: when disable we should disable the PWM output to minimize current
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, LOW);
        }
        else {
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
        }
    }

    if (brakelight_diag_state == lightcontrol_brake_diag_state_stopped) {
        TCC_SetCompareCapture(TCC1, TLD2132_PWMI_WO, brake_brightness);

        if (brake_brightness == LIGHTCONTROL_BRIGHTNESS_MIN) {
            // TODO: when disable we should disable the PWM output to minimize current
            GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, LOW);
        }
        else {
            GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, HIGH);
        }
    }
}

void LIGHTCONTROL_RunDiagnostics(lightcontrol_feature_t feature) {
    if (feature == lightcontrol_feature_tail_segment && taillight_diag_state == lightcontrol_tail_diag_state_stopped) {
        taillight_diag_state = lightcontrol_tail_diag_state_enable;
        SWTIMER_Setup(taillight_diag_timer, 0);
    }
    else if (feature == lightcontrol_feature_brake_segment && brakelight_diag_state == lightcontrol_brake_diag_state_stopped) {
        brakelight_diag_state = lightcontrol_brake_diag_state_enable;
        SWTIMER_Setup(brakelight_diag_timer, 0);
    }
}

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState(lightcontrol_feature_t feature) {
    // TODO: return off when channels are off
    if (feature == lightcontrol_feature_tail_segment) {
        return taillight_state;
    }
    else if (feature == lightcontrol_feature_brake_segment) {
        return brakelight_state;
    }
    else {
        return lightcontrol_feature_state_error;
    }
}
