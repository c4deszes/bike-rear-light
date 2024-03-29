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
    // TODO: disable
    .input = true
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

void LIGHTCONTROL_Init() {
    // Setup TCC1

    // Enable W0 (tail) and W1 (brake)
    
    // 1. prescaler
    TCC1_REGS->TCC_CTRLA = TCC_CTRLA_SWRST_Msk;
    while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_SWRST_Msk) != 0);

    TCC1_REGS->TCC_CTRLA = TCC_CTRLA_PRESCALER_DIV1 |
                           TCC_CTRLA_PRESCSYNC_PRESC ;

                           // WAVEGEN DSBOTTOM
                           // WEXCTRL ?
                           //
    TCC1_REGS->TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0x00);

    // Period
    TCC1_REGS->TCC_PER = TCC_PER_PER(999);   // GCLK 1MHz div 1 -> 1kHz
    //while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_PERB_Msk) != 0);

    // 2. Wavegen

    // 3. Wavegen polarity and drive control invert
    //TCC1_REGS->TCC_DRVCTRL = TCC_DRVCTRL_INVEN0_Msk;    // Enable output inversion for tail light as it's enabled by default with pullup
    TCC1_REGS->TCC_WAVE = TCC_WAVE_WAVEGEN_NPWM;
    //while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_WAVE_Msk) != 0);

    TCC1_REGS->TCC_CC[0] = TCC_CC_CC(900);
    TCC1_REGS->TCC_CC[1] = TCC_CC_CC(900);

    //TCC1_REGS->TCC_INTENSET = TCC_INTENSET_OVF_Msk;
    TCC1_REGS->TCC_INTFLAG = TCC_INTFLAG_Msk;

    // while (TCC1_REGS->TCC_SYNCBUSY != 0U)
    // {
    //     /* Wait for sync */
    // }

        // Enable
    TCC1_REGS->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk) != 0);

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

void LIGHTCONTROL_SetTailBrightness(uint16_t brightness) {
    TCC1_REGS->TCC_CCB[0] = TCC_CCB_CCB(brightness);
    while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_CCB_Msk) != 0);
}

void LIGHTCONTROL_SetBrakeBrightness(uint16_t brightness) {
    TCC1_REGS->TCC_CCB[1] = TCC_CCB_CCB(brightness);
    while((TCC1_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_CCB_Msk) != 0);
}
