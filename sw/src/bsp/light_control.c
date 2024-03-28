#include "bsp/pinout.h"

// initial version:
//    - pulls enable high
//    - polls error pin
//    - brightness adjustment done via PWMI pin
//    - inset pins are left as is

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

    //GPIO_EnableFunction(TLD2331_PWMI_PORT, TLD2331_PWMI_PIN, TLD2331_PWMI_PINMUX);
}

static void BRAKELIGHT_Init(void) {
    GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, LOW);
    GPIO_SetupPinOutput(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, &output);

    GPIO_SetupPinInput(TLD2132_ERROR_PORT, TLD2132_ERROR_PIN, &input);
    // TODO: setup interrupt, pinmux

    //GPIO_EnableFunction(TLD2132_PWMI_PORT, TLD2132_PWMI_PIN, TLD2132_PWMI_PINMUX);
}

void LIGHTCONTROL_Init() {
    TAILLIGHT_Init();
    BRAKELIGHT_Init();
}

void LIGHTCONTROL_SetTailState(bool enabled) {
    if (enabled) {
        GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, HIGH);
    }
    else {
        GPIO_PinWrite(TLD2331_ENABLE_PORT, TLD2331_ENABLE_PIN, LOW);
    }
}

void LIGHTCONTROL_SetBrakeState(bool enabled) {
    if (enabled) {
        GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, HIGH);
    }
    else {
        GPIO_PinWrite(TLD2132_ENABLE_PORT, TLD2132_ENABLE_PIN, LOW);
    }
}
