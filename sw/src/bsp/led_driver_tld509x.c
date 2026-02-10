#include "bsp/pinout.h"

#include "bsp/light_control.h"
#include "hal/tcc.h"
#include "hal/gpio.h"
#include "common/swtimer.h"
#include "app/config.h"
#include "app/feature.h"

#include "sam.h"

static const gpio_pin_output_configuration output = {
    .drive = NORMAL,
    .input = false
};

static tcc_channel_setting_t pwm_channels[4];

/* Internal state */
static uint16_t set_brightness;
static swtimer_t* transition_timer;
static enum {
    tld509x_state_startup,
    tld509x_state_enabled,
    tld509x_state_drive
} driver_state;

static lightcontrol_feature_state_t main_beam_state;

void LIGHTCONTROL_Init(void) {
    // GPIO setup
    // TCC setup
    // DAC setup

    GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, LOW);
    GPIO_SetupPinOutput(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, &output);

    GPIO_PinWrite(TLD509x_ISET_PORT, TLD509x_ISET_PIN, LOW);
    GPIO_SetupPinOutput(TLD509x_ISET_PORT, TLD509x_ISET_PIN, &output);

    /* Timer setup */
    TCC_Reset(TCC2);
    pwm_channels[TLD509x_PWMI_WO].cc = LIGHTCONTROL_BRIGHTNESS_MAX;
    pwm_channels[TLD509x_PWMI_WO].drv_inv = false;
    TCC_SetupNormalPwm(TCC2, FEATURE_LED_DRIVER_PWM_FREQUENCY - 1, pwm_channels);
    TCC_Enable(TCC2);

    /* DAC setup */
    DAC_REGS->DAC_CTRLA = DAC_CTRLA_SWRST_Msk;
    while((DAC_REGS->DAC_STATUS & DAC_STATUS_SYNCBUSY_Msk) != 0);

    DAC_REGS->DAC_CTRLB = DAC_CTRLB_REFSEL_AVCC | DAC_CTRLB_EOEN_Msk | DAC_CTRLB_LEFTADJ(0);

    DAC_REGS->DAC_CTRLA = DAC_CTRLA_ENABLE_Msk;
    while((DAC_REGS->DAC_STATUS & DAC_STATUS_SYNCBUSY_Msk) != 0);

    driver_state = tld509x_state_startup;
    transition_timer = SWTIMER_Create();
    SWTIMER_Setup(transition_timer, FEATURE_LED_DRIVER_STARTUP_DELAY);
    set_brightness = LIGHTCONTROL_BRIGHTNESS_MAX;
}

void LIGHTCONTROL_SetBrightness(uint16_t brightness) {
    // TODO: clamp brightness, if needed disable PWM function and use high/low for 100% / 0%
    // TODO: check if PWM 0 and PWM 100% are achievable

    set_brightness = brightness;
}

lightcontrol_feature_state_t LIGHTCONTROL_GetMainBeamState(void) {
    return main_beam_state;
}

void LIGHTCONTROL_Update10ms(void) {
    // 1. Set PWM duty
    // 2. Set Shutdown if PWM is low

    // 3. Poll VMON
        // if vmon is outside boundaries
        // too low -> short circuit
        // too high -> open circuit

    // 4. In case of short or open circuit do shutdown
        // retry every 4 seconds
        // dim pin can be high

    if (SWTIMER_Elapsed(transition_timer)) {
        if (driver_state == tld509x_state_startup) {
            driver_state = tld509x_state_enabled;
            SWTIMER_Setup(transition_timer, FEATURE_LED_DRIVER_ENABLE_DELAY);
        }
        else if (driver_state == tld509x_state_enabled) {
            driver_state = tld509x_state_drive;
        }
    }

    if (driver_state == tld509x_state_startup) {
        /* Disable driver */
        GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, LOW);
    }
    else if (driver_state == tld509x_state_enabled) {
        GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, HIGH);
    }
    else if (driver_state == tld509x_state_drive) {
        // TODO: drive SET pin using DAC or use PWM
        DAC_REGS->DAC_DATA = set_brightness / 2;

        // TODO: rail to rail operation so that the minimum brightness is 0

        GPIO_EnableFunction(TLD509x_ISET_PORT, TLD509x_ISET_PIN, TLD509x_ISET_PINMUX);

        //GPIO_PinWrite(TLD509x_ISET_PORT, TLD509x_ISET_PIN, HIGH);
    }
}
