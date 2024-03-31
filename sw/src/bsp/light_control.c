#include "bsp/pinout.h"

#include "bsp/light_control.h"

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

static void PWM_TIMER_Setup(void) {
    // TODO: move this to the HAL library
    TCC1_REGS->TCC_CTRLA = TCC_CTRLA_SWRST_Msk;
    while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_SWRST_Msk) != 0);

    TCC1_REGS->TCC_CTRLA = TCC_CTRLA_PRESCALER_DIV1 |
                           TCC_CTRLA_PRESCSYNC_PRESC ;
    TCC1_REGS->TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0x00);
    // TODO: externally adjustable frequency
    TCC1_REGS->TCC_DRVCTRL = TCC_DRVCTRL_INVEN0_Msk | TCC_DRVCTRL_INVEN1_Msk;
    TCC1_REGS->TCC_PER = TCC_PER_PER(999);   // GCLK 1MHz div 1 -> 1kHz
    TCC1_REGS->TCC_WAVE = TCC_WAVE_WAVEGEN_NPWM;

    TCC1_REGS->TCC_CC[TLD2331_PWMI_WO] = TCC_CC_CC(LIGHTCONTROL_BRIGHTNESS_MAX);
    TCC1_REGS->TCC_CC[TLD2132_PWMI_WO] = TCC_CC_CC(LIGHTCONTROL_BRIGHTNESS_MAX);    //TODO: why dummy handler when set to 100?
    TCC1_REGS->TCC_INTFLAG = TCC_INTFLAG_Msk;

    TCC1_REGS->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk) != 0);
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
    if (feature == lightcontrol_feature_tail_segment) {
        TCC1_REGS->TCC_CCB[TLD2331_PWMI_WO] = TCC_CCB_CCB(brightness);
        while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_CCB_Msk) != 0);
    }
    else if(feature == lightcontrol_feature_brake_segment) {
        TCC1_REGS->TCC_CCB[TLD2132_PWMI_WO] = TCC_CCB_CCB(brightness);
        while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_CCB_Msk) != 0);
    }
}
