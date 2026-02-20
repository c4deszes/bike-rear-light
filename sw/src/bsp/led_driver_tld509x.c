#include "bsp/light_control.h"

// Hardware abstraction layer
#include "common/swtimer.h"
#include "hal/tcc.h"
#include "hal/gpio.h"
#include "hal/dac.h"

// Board support package
#include "bsp/pinout.h"

#include "app/feature.h"

static tcc_channel_setting_t pwm_channels[4];

/* Internal state */
static uint16_t LIGHTCONTROL_TargetBrightness;
static swtimer_t* LIGHTCONTROL_Timer;
static enum {
    tld509x_state_startup,
    tld509x_state_enabled,
    tld509x_state_drive
} LIGHTCONTROL_State;

static lightcontrol_feature_state_t LIGHTCONTROL_TailLightState;

void LIGHTCONTROL_Init(void) {
    /* Pin setup */
    GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, LOW);
    GPIO_SetupPinOutput(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    GPIO_PinWrite(TLD509x_ISET_PORT, TLD509x_ISET_PIN, LOW);
    GPIO_SetupPinOutput(TLD509x_ISET_PORT, TLD509x_ISET_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    /* Timer setup */
    TCC_Reset(TCC2);
    pwm_channels[TLD509x_PWMI_WO].cc = LIGHTCONTROL_BRIGHTNESS_MAX;
    pwm_channels[TLD509x_PWMI_WO].drv_inv = false;
    TCC_SetupNormalPwm(TCC2, FEATURE_LED_DRIVER_PWM_FREQUENCY - 1, pwm_channels);
    TCC_Enable(TCC2);

    /* DAC setup */
    DAC_Setup();

    LIGHTCONTROL_State = tld509x_state_startup;
    LIGHTCONTROL_Timer = SWTIMER_Create();
    SWTIMER_Setup(LIGHTCONTROL_Timer, FEATURE_LED_DRIVER_STARTUP_DELAY);
    LIGHTCONTROL_TargetBrightness = LIGHTCONTROL_BRIGHTNESS_MAX;
}

void LIGHTCONTROL_Update10ms(void) {
    if (SWTIMER_Elapsed(LIGHTCONTROL_Timer)) {
        if (LIGHTCONTROL_State == tld509x_state_startup) {
            LIGHTCONTROL_State = tld509x_state_enabled;
            SWTIMER_Setup(LIGHTCONTROL_Timer, FEATURE_LED_DRIVER_ENABLE_DELAY);
        }
        else if (LIGHTCONTROL_State == tld509x_state_enabled) {
            LIGHTCONTROL_State = tld509x_state_drive;
        }
    }

    if (LIGHTCONTROL_State == tld509x_state_startup) {
        /* Disable driver */
        GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, LOW);
    }
    else if (LIGHTCONTROL_State == tld509x_state_enabled) {
        /* Enable driver */
        GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, HIGH);
    }
    else if (LIGHTCONTROL_State == tld509x_state_drive) {
        
        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            DAC_SetValue(LIGHTCONTROL_TargetBrightness / 2);
            GPIO_EnableFunction(TLD509x_ISET_PORT, TLD509x_ISET_PIN, TLD509x_ISET_PINMUX);
        }
        else {
            GPIO_PinWrite(TLD509x_ISET_PORT, TLD509x_ISET_PIN, LOW);
            GPIO_DisableFunction(TLD509x_ISET_PORT, TLD509x_ISET_PIN);
        }
    }
}

void LIGHTCONTROL_SetBrightness(uint16_t brightness) {
    if (brightness > LIGHTCONTROL_BRIGHTNESS_MAX) {
        brightness = LIGHTCONTROL_BRIGHTNESS_MAX;
    }

    LIGHTCONTROL_TargetBrightness = brightness;
}

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState(void) {
    return LIGHTCONTROL_TailLightState;
}
