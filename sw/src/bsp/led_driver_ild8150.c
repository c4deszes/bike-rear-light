#include "bsp/pinout.h"
#include "bsp/ild8150_cfg.h"

#include "bsp/light_control.h"
#include "hal/tcc.h"
#include "hal/gpio.h"
#include "common/swtimer.h"

#include "app/feature.h"

#define ILD8150_PWM_PERIOD (1000000u / ILD8150_PWM_FREQUENCY)   /* PWM period in microseconds */
#define ILD8150_PWM_SCALE(brightness) ((brightness) * ILD8150_PWM_PERIOD / LIGHTCONTROL_BRIGHTNESS_MAX)

/* HAL configuration */
static tcc_channel_setting_t pwm_channels[4];

/* Internal state */
static bool LIGHTCONTROL_FirstSetpointReceived;
static uint16_t LIGHTCONTROL_TailBrightness;
static uint16_t LIGHTCONTROL_BrakeBrightness;
static swtimer_t* LIGHTCONTROL_Timer;
static enum {
    ild8150_state_disabled,
    ild8150_state_enabling,
    ild8150_state_enabled,
    ild8150_state_disabling
} LIGHTCONTROL_State;
static bool LIGHTCONTROL_DriversSetup;
static uint16_t LIGHTCONTROL_DisableTimer;

static lightcontrol_feature_state_t LIGHTCONTROL_TailLightState;
static uint16_t vmon_voltage_raw;
static uint16_t vmon_error_counter;

static void LIGHTCONTROL_IO_Init() {
    /* Shutdown is Low active, by default disabling the element */
    GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, LOW);
    GPIO_SetupPinOutput(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    /* PWM is High active, by default disabling the element */
    GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, LOW);
    GPIO_SetupPinOutput(ILD8150_DIM_PORT, ILD8150_DIM_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    /* VMON is floating when ILD8150 is disabled */
    GPIO_EnableFunction(ILD8150_VMON_PORT, ILD8150_VMON_PIN, ILD8150_VMON_PINMUX);
}

static uint16_t LIGHTCONTROL_GetCombinedBrightness() {
    if (LIGHTCONTROL_BrakeBrightness > LIGHTCONTROL_TailBrightness) {
        return LIGHTCONTROL_BrakeBrightness;
    }
    return LIGHTCONTROL_TailBrightness;
}

static void ILD8150_StartPwm() {
    /* Timer setup */
    TCC_Reset(TCC2);

    pwm_channels[ILD8150_DIM_WO].cc = ILD8150_PWM_SCALE(LIGHTCONTROL_BRIGHTNESS_MIN);
    pwm_channels[ILD8150_DIM_WO].drv_inv = false;

    pwm_channels[1].cc = 50;
    pwm_channels[1].drv_inv = false;

    TCC_SetupNormalPwm(TCC2, ILD8150_PWM_PERIOD - 1, pwm_channels);
    TCC_Enable(TCC2);
}

static void ILD8150_Enable(void) {
    GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, HIGH);
    GPIO_SetupPinOutput(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void ILD8150_Disable(void) {
    GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, LOW);
    GPIO_SetupPinOutput(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
}

static void ILD8150_UpdateBrightness(uint16_t brightness) {

    if (brightness > LIGHTCONTROL_BRIGHTNESS_MAX) {
        brightness = LIGHTCONTROL_BRIGHTNESS_MAX;
    }

    /* In PWM mode the brightness is controlled by the duty cycle, here we scale the input 
       to the period of the timer.*/
    if (brightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
        GPIO_EnableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN, ILD8150_DIM_PINMUX);
        TCC_SetCompareCapture(TCC2, ILD8150_DIM_WO, ILD8150_PWM_SCALE(brightness));
    }
    else {
        GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, LOW);
        GPIO_DisableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN);
    }
}

void LIGHTCONTROL_Init(void) {
    LIGHTCONTROL_TailBrightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    LIGHTCONTROL_BrakeBrightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    LIGHTCONTROL_TailLightState = lightcontrol_feature_state_ok;
    LIGHTCONTROL_Timer = SWTIMER_Create();

    #if ILD8150_HARDWARE_TYPE == ILD8150_HARDWARE_NOINIT
        LIGHTCONTROL_State = ild8150_state_disabling;
        SWTIMER_Setup(LIGHTCONTROL_Timer, ILD8150_TURN_OFF_DELAY_MS);
    #elif ILD8150_HARDWARE_TYPE == ILD8150_HARDWARE_OFF
        /* No hardware setup needed for max PWM mode */
        LIGHTCONTROL_State = ild8150_state_disabling;
        SWTIMER_Setup(LIGHTCONTROL_Timer, ILD8150_TURN_OFF_DELAY_MS);
    #elif ILD8150_HARDWARE_TYPE == ILD8150_HARDWARE_MAX_PWM
        LIGHTCONTROL_State = ild8150_state_enabling;
        SWTIMER_Setup(LIGHTCONTROL_Timer, ILD8150_TURN_ON_DELAY_MS);
    #else
        #error "Unsupported hardware type for ILD8150 LED driver"
    #endif
}

void LIGHTCONTROL_Update10ms(void) {

        uint16_t LIGHTCONTROL_TargetBrightness = LIGHTCONTROL_GetCombinedBrightness();

    if (LIGHTCONTROL_State == ild8150_state_disabled) {
        /* Transition to enabling */
        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            /* Enable the driver */
            ILD8150_Enable();

            /* Performs first time setup, the DAC and TCC2 are only started if they haven't been
               setup before. This call is mostly important for analog mode, in that case the DAC
               output will be set immediately */
            if (!LIGHTCONTROL_DriversSetup) {
                ILD8150_StartPwm();
                LIGHTCONTROL_DriversSetup = true;
            }

            LIGHTCONTROL_State = ild8150_state_enabling;
            SWTIMER_Setup(LIGHTCONTROL_Timer, ILD8150_TURN_ON_DELAY_MS);
        }
    }
    else if (LIGHTCONTROL_State == ild8150_state_enabling) {
        if (SWTIMER_Elapsed(LIGHTCONTROL_Timer) && LIGHTCONTROL_FirstSetpointReceived) {

            /* First time setup, this call is only relevant for cases where the hardware automatically
               enables the driver. */
            if (!LIGHTCONTROL_DriversSetup) {
                ILD8150_StartPwm();
                LIGHTCONTROL_DriversSetup = true;
            }

            GPIO_EnableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN, ILD8150_DIM_PINMUX);

            LIGHTCONTROL_State = ild8150_state_enabled;
        }
    }
    else if (LIGHTCONTROL_State == ild8150_state_enabled) {

        ILD8150_UpdateBrightness(LIGHTCONTROL_TargetBrightness);

        // TODO: monitor

        if (LIGHTCONTROL_TargetBrightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            /* Reset disable timer */
            LIGHTCONTROL_DisableTimer = ILD8150_DISABLE_TIMEOUT_MS;
        }
        else if (LIGHTCONTROL_DisableTimer > 0) {
            /* Countdown to disable */
            LIGHTCONTROL_DisableTimer -= 10; /* This function is called every 10ms */
        }

        /* Transition to disabling */
#if ILD8150_DISABLE_AT_ZERO == 1
        if (LIGHTCONTROL_DisableTimer == 0) {
            /* Disable the driver */
            ILD8150_Disable();
            LIGHTCONTROL_State = ild8150_state_disabling;
            SWTIMER_Setup(LIGHTCONTROL_Timer, ILD8150_TURN_OFF_DELAY_MS);
        }
#endif
    }
    else if (LIGHTCONTROL_State == ild8150_state_disabling) {
        if (SWTIMER_Elapsed(LIGHTCONTROL_Timer)) {
            LIGHTCONTROL_State = ild8150_state_disabled;
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
        return LIGHTCONTROL_TailLightState;
    }
    return lightcontrol_feature_state_error; /* Invalid segment */
}