#include "bsp/pinout.h"

#include "bsp/light_control.h"
#include "hal/tcc.h"

static const gpio_pin_input_configuration input = {
    .pull = FLOATING,
    .sample = CONTINUOUS
};

static const gpio_pin_output_configuration output = {
    .drive = NORMAL,
    .input = false
};

static void TAILLIGHT_Init(void) {
    GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, &output);

    GPIO_SetupPinInput(TLD2331_ERROR_PORT, TLD2331_ERROR_PIN, &input);
    // TODO: setup interrupt, pinmux

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
    // TODO: setup interrupt, pinmux

    GPIO_SetupPinOutput(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, &output);
    GPIO_EnableFunction(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, TLD2132_PWMI_PINMUX);

    GPIO_PinWrite(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, HIGH);
    GPIO_SetupPinOutput(TLD2132_INSET1_PORT, TLD2132_INSET1_PIN, &output);
}

tcc_channel_setting_t pwm_channels[4];

static void PWM_TIMER_Setup(void) {
    TCC_Reset(TCC1);

    pwm_channels[TLD2331_PWMI_WO].cc = LIGHTCONTROL_BRIGHTNESS_MAX;
    pwm_channels[TLD2331_PWMI_WO].drv_inv = true;
    pwm_channels[TLD2132_PWMI_WO].cc = LIGHTCONTROL_BRIGHTNESS_MAX;   // TODO: try PWM set to MIN
    pwm_channels[TLD2132_PWMI_WO].drv_inv = true;

    // TODO: for some reason dummy handler is reached when we write one
    pwm_channels[2].cc = LIGHTCONTROL_BRIGHTNESS_MAX;
    pwm_channels[3].cc = LIGHTCONTROL_BRIGHTNESS_MAX;

    TCC_SetupNormalPwm(TCC1, 999, pwm_channels);
    TCC_Enable(TCC1);
}

void LIGHTCONTROL_Init() {
    PWM_TIMER_Setup();

    TAILLIGHT_Init();
    BRAKELIGHT_Init();
}

void LIGHTCONTROL_SetState(lightcontrol_feature_t feature, bool enabled) {
    if (feature == lightcontrol_feature_tail_segment) {
        if (enabled) {
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
        }
        else {
            GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, LOW);
        }
    }
    else if (feature == lightcontrol_feature_brake_segment) {
        if (enabled) {
            GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, HIGH);
        }
        else {
            GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, LOW);
        }
    }
}

void LIGHTCONTROL_SetBrightness(lightcontrol_feature_t feature, uint16_t brightness) {
    // TODO: clamp brightness, if needed disable PWM function and use high/low for 100% / 0%
    // TODO: check if PWM 0 and PWM 100% are achievable
    if (feature == lightcontrol_feature_tail_segment) {
        TCC_SetCompareCapture(TCC1, TLD2331_PWMI_WO, brightness);
    }
    else if(feature == lightcontrol_feature_brake_segment) {
        TCC_SetCompareCapture(TCC1, TLD2132_PWMI_WO, brightness);
    }

    if (brightness == LIGHTCONTROL_BRIGHTNESS_MIN) {
        LIGHTCONTROL_SetState(feature, false);
    }
    else {
        LIGHTCONTROL_SetState(feature, true);
    }
}
