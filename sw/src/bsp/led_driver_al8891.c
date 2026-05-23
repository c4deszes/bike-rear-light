#include "bsp/light_control.h"
#include "bsp/al8891_cfg.h"

// Hardware abstraction layer
#include "common/swtimer.h"
#include "hal/tcc.h"
#include "hal/gpio.h"
#include "hal/dac.h"

// Board support package
#include "bsp/pinout.h"

#include "app/feature.h"

// TODO: frequency should be taken from board config
#define AL8891_PWM_PERIOD (1000000u / AL8891_PWM_FREQUENCY)   /* PWM period in microseconds */
#define AL8891_PWM_SCALE(brightness) ((brightness) * AL8891_PWM_PERIOD / LIGHTCONTROL_BRIGHTNESS_MAX)

static tcc_channel_setting_t pwm_channels[4];

/* Internal state */
static bool LIGHTCONTROL_FirstSetpointReceived;
static uint16_t LIGHTCONTROL_TailBrightness;
static uint16_t LIGHTCONTROL_BrakeBrightness;
static lightcontrol_feature_state_t LIGHTCONTROL_TailLightState;
static lightcontrol_feature_state_t LIGHTCONTROL_BrakeLightState;
static swtimer_t* LIGHTCONTROL_Timer;
static enum {
    al8891_state_disabled,
    al8891_state_enabling,
    al8891_state_enabled,
    al8891_state_disabling
} LIGHTCONTROL_State;
static bool LIGHTCONTROL_DriversSetup;
static uint16_t LIGHTCONTROL_DisableTimer;

static uint16_t LIGHTCONTROL_AverageDutyData[10];
static uint16_t LIGHTCONTROL_AverageDuty;

void LIGHTCONTROL_Init(void) {
    LIGHTCONTROL_FirstSetpointReceived = false;
    LIGHTCONTROL_TailBrightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    LIGHTCONTROL_BrakeBrightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    LIGHTCONTROL_TailLightState = lightcontrol_feature_state_ok;
    LIGHTCONTROL_BrakeLightState = lightcontrol_feature_state_ok;
    LIGHTCONTROL_Timer = SWTIMER_Create();
    LIGHTCONTROL_DriversSetup = false;
    LIGHTCONTROL_DisableTimer = AL8891_DISABLE_TIMEOUT_MS;

#if AL8891_HARDWARE_TYPE == AL8891_HARDWARE_NOINIT
    LIGHTCONTROL_State = al8891_state_disabling;
    SWTIMER_Setup(LIGHTCONTROL_Timer, AL8891_TURN_OFF_DELAY_MS);
#elif AL8891_HARDWARE_TYPE == AL8891_HARDWARE_MAX_PWM
    /* No hardware setup needed for max PWM mode */
    LIGHTCONTROL_State = al8891_state_enabling;
    SWTIMER_Setup(LIGHTCONTROL_Timer, AL8891_TURN_ON_DELAY_MS);
#elif AL8891_HARDWARE_TYPE == AL8891_HARDWARE_ANALOG
    LIGHTCONTROL_State = al8891_state_enabling;
    SWTIMER_Setup(LIGHTCONTROL_Timer, AL8891_TURN_ON_DELAY_MS);
#else
    #error "Unsupported hardware type for AL8891 LED driver"
#endif
}

uint16_t LIGHTCONTROL_GetCombinedBrightness() {
    if (LIGHTCONTROL_BrakeBrightness > LIGHTCONTROL_TailBrightness) {
        return LIGHTCONTROL_BrakeBrightness;
    }
    return LIGHTCONTROL_TailBrightness;
}

static void AL8891_Enable(void) {
    GPIO_PinWrite(AL8891_ENABLE_PORT, AL8891_ENABLE_PIN, HIGH);
    GPIO_SetupPinOutput(AL8891_ENABLE_PORT, AL8891_ENABLE_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    GPIO_PinWrite(AL8891_PDIM_PORT, AL8891_PDIM_PIN, HIGH);
    GPIO_SetupPinOutput(AL8891_PDIM_PORT, AL8891_PDIM_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void AL8891_Disable(void) {
    GPIO_PinWrite(AL8891_ENABLE_PORT, AL8891_ENABLE_PIN, LOW);
    GPIO_SetupPinOutput(AL8891_ENABLE_PORT, AL8891_ENABLE_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    GPIO_PinWrite(AL8891_PDIM_PORT, AL8891_PDIM_PIN, LOW);
    GPIO_SetupPinOutput(AL8891_PDIM_PORT, AL8891_PDIM_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void AL8891_StartPwm(void) {
    // TCC_Reset(TCC3);
    // pwm_channels[AL8891_PDIM_WO].cc = AL8891_PWM_SCALE(LIGHTCONTROL_BRIGHTNESS_MIN);     // TODO: May need to be inverted
    // pwm_channels[AL8891_PDIM_WO].drv_inv = false;
    // TCC_SetupNormalPwm(TCC3, AL8891_PWM_PERIOD - 1u, pwm_channels);
    // TCC_Enable(TCC3);

    // /* In PWM mode analog dimming is disabled by setting the ISET pin above 1.6V */
    // GPIO_PinWrite(AL8891_ADIM_PORT, AL8891_ADIM_PIN, HIGH);
    // GPIO_SetupPinOutput(AL8891_ADIM_PORT, AL8891_ADIM_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void AL8891_StartDac(void) {
    DAC_Setup();
}

static void AL8891_UpdateBrightness(uint16_t brightness) {

    if (brightness > LIGHTCONTROL_BRIGHTNESS_MAX) {
        brightness = LIGHTCONTROL_BRIGHTNESS_MAX;
    }

#if AL8891_MODULATION_TYPE == AL8891_MODULATION_TYPE_PWM
    /* In PWM mode the brightness is controlled by the duty cycle, here we scale the input 
       to the period of the timer.*/
    if (brightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
        TCC_SetCompareCapture(TCC2, AL8891_PWMI_WO, AL8891_PWM_SCALE(brightness));
    }
    else {
        /* In PWM mode the enable pin needs to be on for a minimum time so that the driver doesn't
           power down */
        // TODO: this does some funky things, there's still quite a lot of current passing through the device
        TCC_SetCompareCapture(TCC2, AL8891_PWMI_WO, AL8891_PWM_MINIMUM_ON_US);
    }
#elif AL8891_MODULATION_TYPE == AL8891_MODULATION_TYPE_ANALOG
    /* In analog mode the brightness is scaled so that at 1000 increment the output
       voltage is ~1.6V, with a 10bit DAC using a reference of 3.3V that's achieved
       by 512 increment as the DAC input */
    DAC_SetValue(brightness / 2u);
    // TODO: LED current is not true zero at 0 brightness, the workaround of keeping the driver enabled
    //       and setting the pin to output low doesn't seem to work. analog modulation is disabled
#endif  
}

static void AL8891_FirstTimeSetup(uint16_t brightness) {
    if (!LIGHTCONTROL_DriversSetup) {
        #if AL8891_MODULATION_TYPE == AL8891_MODULATION_TYPE_PWM
        AL8891_StartPwm();
        #elif AL8891_MODULATION_TYPE == AL8891_MODULATION_TYPE_ANALOG
        AL8891_StartDac();
        #endif
        LIGHTCONTROL_DriversSetup = true;
    }
    AL8891_UpdateBrightness(brightness);

    #if AL8891_MODULATION_TYPE == AL8891_MODULATION_TYPE_ANALOG
    GPIO_EnableFunction(AL8891_ADIM_PORT, AL8891_ADIM_PIN, AL8891_ADIM_PINMUX);
    #endif
}

void LIGHTCONTROL_Update10ms(void) {

    uint16_t LIGHTCONTROL_TargetBrightness = LIGHTCONTROL_GetCombinedBrightness();

    if (LIGHTCONTROL_State == al8891_state_disabled) {
        /* Transition to enabling */
        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            /* Enable the driver */
            AL8891_Enable();

            /* Performs first time setup, the DAC and TCC2 are only started if they haven't been
               setup before. This call is mostly important for analog mode, in that case the DAC
               output will be set immediately */
            AL8891_FirstTimeSetup(LIGHTCONTROL_TargetBrightness);

            LIGHTCONTROL_State = al8891_state_enabling;
            SWTIMER_Setup(LIGHTCONTROL_Timer, AL8891_TURN_ON_DELAY_MS);
        }
    }
    else if (LIGHTCONTROL_State == al8891_state_enabling) {
        if (SWTIMER_Elapsed(LIGHTCONTROL_Timer) && LIGHTCONTROL_FirstSetpointReceived) {

            /* First time setup, this call is only relevant for cases where the hardware automatically
               enables the driver. */
            AL8891_FirstTimeSetup(LIGHTCONTROL_TargetBrightness);

            #if AL8891_MODULATION_TYPE == AL8891_MODULATION_TYPE_PWM
            /* In PWM mode the pin function needs to be modified once the driver has been enabled */
            GPIO_EnableFunction(AL8891_PWMI_PORT, AL8891_PWMI_PIN, AL8891_PWMI_PINMUX);
            #endif

            LIGHTCONTROL_State = al8891_state_enabled;
        }
    }
    else if (LIGHTCONTROL_State == al8891_state_enabled) {

        AL8891_UpdateBrightness(LIGHTCONTROL_TargetBrightness);

        // TODO: monitor PWMO

        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            /* Reset disable timer */
            LIGHTCONTROL_DisableTimer = AL8891_DISABLE_TIMEOUT_MS;
        }
        else if (LIGHTCONTROL_DisableTimer > 0) {
            /* Countdown to disable */
            if (LIGHTCONTROL_DisableTimer > 10) {
                LIGHTCONTROL_DisableTimer -= 10; /* This function is called every 10ms */
            }
            else {
                LIGHTCONTROL_DisableTimer = 0;
            }
        }

        /* Transition to disabling */
#if AL8891_DISABLE_AT_ZERO == 1
        if (LIGHTCONTROL_DisableTimer == 0) {
            /* Disable the driver */
            AL8891_Disable();
            LIGHTCONTROL_State = al8891_state_disabling;
            SWTIMER_Setup(LIGHTCONTROL_Timer, AL8891_TURN_OFF_DELAY_MS);
        }
#endif
    }
    else if (LIGHTCONTROL_State == al8891_state_disabling) {
        if (SWTIMER_Elapsed(LIGHTCONTROL_Timer)) {
            LIGHTCONTROL_State = al8891_state_disabled;
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



lightcontrol_drive_mode_t LIGHTCONTROL_GetDriveMode(lightcontrol_segment_t segment) {
    /* Both tail and rear light use a switching regulator */
    return lightcontrol_drive_mode_buck;
}

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState(lightcontrol_segment_t segment) {
    if (segment == lightcontrol_segment_tail) {
        return LIGHTCONTROL_TailLightState;
    } else if (segment == lightcontrol_segment_brake) {
        return LIGHTCONTROL_BrakeLightState;
    }
    return lightcontrol_feature_state_error; /* Invalid segment */
}

