#include "bsp/light_control.h"
#include "bsp/tld509x_cfg.h"

// Hardware abstraction layer
#include "common/swtimer.h"
#include "hal/tcc.h"
#include "hal/gpio.h"
#include "hal/dac.h"

// Board support package
#include "bsp/pinout.h"

#include "app/feature.h"

#define TLD509x_PWM_PERIOD (1000000u / TLD509x_MODULATION_FREQUENCY)   /* PWM period in microseconds */
#define TLD509x_PWM_SCALE(brightness) ((brightness) * TLD509x_PWM_PERIOD / LIGHTCONTROL_BRIGHTNESS_MAX)

static tcc_channel_setting_t pwm_channels[4];

/* Internal state */
static bool LIGHTCONTROL_FirstSetpointReceived;
static uint16_t LIGHTCONTROL_TailBrightness;
static uint16_t LIGHTCONTROL_BrakeBrightness;
static lightcontrol_feature_state_t LIGHTCONTROL_TailLightState;
static lightcontrol_feature_state_t LIGHTCONTROL_BrakeLightState;
static swtimer_t* LIGHTCONTROL_Timer;
static enum {
    tld509x_state_disabled,
    tld509x_state_enabling,
    tld509x_state_enabled,
    tld509x_state_disabling
} LIGHTCONTROL_State;
static bool LIGHTCONTROL_DriversSetup;
static uint16_t LIGHTCONTROL_DisableTimer;

void LIGHTCONTROL_Init(void) {
    LIGHTCONTROL_FirstSetpointReceived = false;
    LIGHTCONTROL_TailBrightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    LIGHTCONTROL_BrakeBrightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    LIGHTCONTROL_TailLightState = lightcontrol_feature_state_ok;
    LIGHTCONTROL_BrakeLightState = lightcontrol_feature_state_ok;
    LIGHTCONTROL_Timer = SWTIMER_Create();
    LIGHTCONTROL_DriversSetup = false;
    LIGHTCONTROL_DisableTimer = TLD509x_DISABLE_TIMEOUT_MS;
#if TLD509x_HARDWARE_TYPE == TLD509x_HARDWARE_NOINIT
    LIGHTCONTROL_State = tld509x_state_disabling;
    SWTIMER_Setup(LIGHTCONTROL_Timer, TLD509x_TURN_OFF_DELAY_MS);
#elif TLD509x_HARDWARE_TYPE == TLD509x_HARDWARE_MAX_PWM
    /* No hardware setup needed for max PWM mode */
    LIGHTCONTROL_State = tld509x_state_enabling;
    SWTIMER_Setup(LIGHTCONTROL_Timer, TLD509x_TURN_ON_DELAY_MS);
#elif TLD509x_HARDWARE_TYPE == TLD509x_HARDWARE_ANALOG
    LIGHTCONTROL_State = tld509x_state_enabling;
    SWTIMER_Setup(LIGHTCONTROL_Timer, TLD509x_TURN_ON_DELAY_MS);
#else
    #error "Unsupported hardware type for TLD509x LED driver"
#endif
}

uint16_t LIGHTCONTROL_GetCombinedBrightness() {
    if (LIGHTCONTROL_BrakeBrightness > LIGHTCONTROL_TailBrightness) {
        return LIGHTCONTROL_BrakeBrightness;
    }
    return LIGHTCONTROL_TailBrightness;
}

static void TLD509x_Enable(void) {
    GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, HIGH);
    GPIO_SetupPinOutput(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void TLD509x_Disable(void) {
    GPIO_PinWrite(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, LOW);
    GPIO_SetupPinOutput(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void TLD509x_StartPwm(void) {
    TCC_Reset(TCC2);
    pwm_channels[TLD509x_PWMI_WO].cc = TLD509x_PWM_SCALE(LIGHTCONTROL_BRIGHTNESS_MIN);     // TODO: May need to be inverted
    pwm_channels[TLD509x_PWMI_WO].drv_inv = false;
    TCC_SetupNormalPwm(TCC2, TLD509x_PWM_PERIOD - 1u, pwm_channels);
    TCC_Enable(TCC2);

    /* In PWM mode analog dimming is disabled by setting the ISET pin above 1.6V */
    GPIO_PinWrite(TLD509x_ISET_PORT, TLD509x_ISET_PIN, HIGH);
    GPIO_SetupPinOutput(TLD509x_ISET_PORT, TLD509x_ISET_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void TLD509x_StartDac(void) {
    DAC_Setup();
}

static void TLD509x_UpdateBrightness(uint16_t brightness) {

    if (brightness > LIGHTCONTROL_BRIGHTNESS_MAX) {
        brightness = LIGHTCONTROL_BRIGHTNESS_MAX;
    }

#if TLD509x_MODULATION_TYPE == TLD509x_MODULATION_TYPE_PWM
    /* In PWM mode the brightness is controlled by the duty cycle, here we scale the input 
       to the period of the timer.*/
    if (brightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
        TCC_SetCompareCapture(TCC2, TLD509x_PWMI_WO, TLD509x_PWM_SCALE(brightness));
    }
    else {
        /* In PWM mode the enable pin needs to be on for a minimum time so that the driver doesn't
           power down */
        TCC_SetCompareCapture(TCC2, TLD509x_PWMI_WO, TLD509x_PWM_MINIMUM_ON_US);
    }
#elif TLD509x_MODULATION_TYPE == TLD509x_MODULATION_TYPE_ANALOG
    /* In analog mode the brightness is scaled so that at 1000 increment the output
       voltage is ~1.6V, with a 10bit DAC using a reference of 3.3V that's achieved
       by 512 increment as the DAC input */
    if (brightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
        DAC_SetValue(brightness / 2u);
        GPIO_EnableFunction(TLD509x_ISET_PORT, TLD509x_ISET_PIN, TLD509x_ISET_PINMUX);
    }
    else {
        /* In analog mode the LED is turned off by setting the ISET pin low, as the DAC
           output would be higher than the switch off threshold */
        GPIO_PinWrite(TLD509x_ISET_PORT, TLD509x_ISET_PIN, LOW);
        GPIO_SetupPinOutput(TLD509x_ISET_PORT, TLD509x_ISET_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);      // TODO: this shouldn't be called very often
    }
#endif  
}

static void TLD509x_FirstTimeSetup(uint16_t brightness) {
    if (!LIGHTCONTROL_DriversSetup) {
        #if TLD509x_MODULATION_TYPE == TLD509x_MODULATION_TYPE_PWM
        TLD509x_StartPwm();
        #elif TLD509x_MODULATION_TYPE == TLD509x_MODULATION_TYPE_ANALOG
        TLD509x_StartDac();
        #endif
        LIGHTCONTROL_DriversSetup = true;
    }
    TLD509x_UpdateBrightness(brightness);
}

void LIGHTCONTROL_Update10ms(void) {

    uint16_t LIGHTCONTROL_TargetBrightness = LIGHTCONTROL_GetCombinedBrightness();

    if (LIGHTCONTROL_State == tld509x_state_disabled) {
        /* Transition to enabling */
        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            /* Enable the driver */
            TLD509x_Enable();

            /* Performs first time setup, the DAC and TCC2 are only started if they haven't been
               setup before. This call is mostly important for analog mode, in that case the DAC
               output will be set immediately */
            TLD509x_FirstTimeSetup(LIGHTCONTROL_TargetBrightness);

            LIGHTCONTROL_State = tld509x_state_enabling;
            SWTIMER_Setup(LIGHTCONTROL_Timer, TLD509x_TURN_ON_DELAY_MS);
        }
    }
    else if (LIGHTCONTROL_State == tld509x_state_enabling) {
        if (SWTIMER_Elapsed(LIGHTCONTROL_Timer) && LIGHTCONTROL_FirstSetpointReceived) {

            /* First time setup, this call is only relevant for cases where the hardware automatically
               enables the driver. */
            TLD509x_FirstTimeSetup(LIGHTCONTROL_TargetBrightness);

            #if TLD509x_MODULATION_TYPE == TLD509x_MODULATION_TYPE_PWM
            /* In PWM mode the pin function needs to be modified once the driver has been enabled */
            GPIO_EnableFunction(TLD509x_PWMI_PORT, TLD509x_PWMI_PIN, TLD509x_PWMI_PINMUX);
            #endif

            LIGHTCONTROL_State = tld509x_state_enabled;
        }
    }
    else if (LIGHTCONTROL_State == tld509x_state_enabled) {

        TLD509x_UpdateBrightness(LIGHTCONTROL_TargetBrightness);

        // TODO: monitor PWMO

        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            /* Reset disable timer */
            LIGHTCONTROL_DisableTimer = TLD509x_DISABLE_TIMEOUT_MS;
        }
        else if (LIGHTCONTROL_DisableTimer > 0) {
            /* Countdown to disable */
            LIGHTCONTROL_DisableTimer -= 10; /* This function is called every 10ms */
        }

        /* Transition to disabling */
#if TLD509x_DISABLE_AT_ZERO == 1
        if (LIGHTCONTROL_DisableTimer == 0) {
            /* Disable the driver */
            TLD509x_Disable();
            LIGHTCONTROL_State = tld509x_state_disabling;
            SWTIMER_Setup(LIGHTCONTROL_Timer, TLD509x_TURN_OFF_DELAY_MS);
        }
#endif
    }
    else if (LIGHTCONTROL_State == tld509x_state_disabling) {
        if (SWTIMER_Elapsed(LIGHTCONTROL_Timer)) {
            LIGHTCONTROL_State = tld509x_state_disabled;
        }
    }
}

void LIGHTCONTROL_SetBrightness(lightcontrol_segment_t segment, uint16_t brightness) {
    if (brightness > LIGHTCONTROL_BRIGHTNESS_MAX) {
        brightness = LIGHTCONTROL_BRIGHTNESS_MAX;
    }

    if (segment == lightcontrol_segment_tail) {
        LIGHTCONTROL_TailBrightness = brightness;
    } else if (segment == lightcontrol_segment_brake) {
        LIGHTCONTROL_BrakeBrightness = brightness;
    }
    LIGHTCONTROL_FirstSetpointReceived = true;
}

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState(lightcontrol_segment_t segment) {
    if (segment == lightcontrol_segment_tail) {
        return LIGHTCONTROL_TailLightState;
    } else if (segment == lightcontrol_segment_brake) {
        return LIGHTCONTROL_BrakeLightState;
    }
    return lightcontrol_feature_state_error; /* Invalid segment */
}
