#include "bsp/pinout.h"

#include "bsp/light_control.h"
#include "hal/tcc.h"
#include "hal/gpio.h"
#include "common/swtimer.h"

#include "bsp/config.h"

#if LED_DRIVER_CONTROL_MODE != CONTROL_MODE_PWM
#warning "ILD8150 only supports PWM control."
#endif

/* HAL configuration */
static const gpio_pin_output_configuration output = {
    .drive = NORMAL,
    .input = false
};
static tcc_channel_setting_t pwm_channels[4];

/* Internal state */
static uint16_t set_brightness;
static swtimer_t* transition_timer;
static enum {
    ild8150_state_startup,
    ild8150_state_enabled,
    ild8150_state_drive
} driver_state;

static uint16_t vmon_voltage_raw;
static uint16_t vmon_error_counter;
static lightcontrol_feature_state_t main_beam_state;

static void LIGHTCONTROL_IO_Init() {
    /* Shutdown is Low active, by default disabling the element */
    GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, LOW);
    GPIO_SetupPinOutput(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, &output);

    /* PWM is High active, by default disabling the element */
    GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, LOW);
    GPIO_SetupPinOutput(ILD8150_DIM_PORT, ILD8150_DIM_PIN, &output);

    /* VMON is floating when ILD8150 is disabled */
    GPIO_EnableFunction(ILD8150_VMON_PORT, ILD8150_VMON_PIN, ILD8150_VMON_PINMUX);
}

static void LIGHTCONTROL_Timer_Init() {
    /* Timer setup */
    TCC_Reset(TCC2);

    pwm_channels[ILD8150_DIM_WO].cc = LIGHTCONTROL_BRIGHTNESS_MIN;
    pwm_channels[ILD8150_DIM_WO].drv_inv = false;

    pwm_channels[1].cc = 50;
    pwm_channels[1].drv_inv = false;

    TCC_SetupNormalPwm(TCC2, LED_DRIVER_PWM_FREQUENCY - 1, pwm_channels);
    TCC_Enable(TCC2);
}

static void LIGHTCONTROL_Adc_Init() {
    /* Write the calibration data. */
    uint8_t bias_comp = (OTP5_FUSES_REGS->FUSES_OTP5_WORD_0 & FUSES_OTP5_WORD_0_ADC1_BIASCOMP_Msk) >> FUSES_OTP5_WORD_0_ADC1_BIASCOMP_Pos;
    uint8_t bias_ref = (OTP5_FUSES_REGS->FUSES_OTP5_WORD_0 & FUSES_OTP5_WORD_0_ADC1_BIASREFBUF_Msk) >> FUSES_OTP5_WORD_0_ADC1_BIASREFBUF_Pos;

    ADC1_REGS->ADC_CALIB = ADC_CALIB_BIASCOMP(bias_comp) | ADC_CALIB_BIASREFBUF(bias_ref);

    ADC1_REGS->ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTVCC2;
    ADC1_REGS->ADC_AVGCTRL = ADC_AVGCTRL_SAMPLENUM_1;
    ADC1_REGS->ADC_CTRLB = ADC_CTRLB_PRESCALER_DIV4;
    ADC1_REGS->ADC_CTRLC = ADC_CTRLC_RESSEL_12BIT | ADC_CTRLC_FREERUN_Msk;

    ADC1_REGS->ADC_INPUTCTRL = ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_AIN11;

    ADC1_REGS->ADC_CTRLA = ADC_CTRLA_ENABLE_Msk;

    while ((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_ENABLE_Msk) != 0);
}

void LIGHTCONTROL_Init(void) {
    set_brightness = LIGHTCONTROL_BRIGHTNESS_MIN;
    driver_state = ild8150_state_startup;
    transition_timer = SWTIMER_Create();
    SWTIMER_Setup(transition_timer, LED_DRIVER_STARTUP_DELAY);

    vmon_error_counter = 0;
    vmon_voltage_raw = 0;
    main_beam_state = lightcontrol_feature_state_off;

    LIGHTCONTROL_IO_Init();
    LIGHTCONTROL_Adc_Init();
    LIGHTCONTROL_Timer_Init();
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

    if (SWTIMER_Elapsed(transition_timer)) {
        if (driver_state == ild8150_state_startup) {
            driver_state = ild8150_state_enabled;
            SWTIMER_Setup(transition_timer, LED_DRIVER_ENABLE_DELAY);
        }
        else if (driver_state == ild8150_state_enabled) {
            driver_state = ild8150_state_drive;
        }
    }

    if (driver_state == ild8150_state_startup) {
        /* Disable driver */
        GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, LOW);
        GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, LOW);
        GPIO_DisableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN);
    }
    else if (driver_state == ild8150_state_enabled) {
        GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, HIGH);
        GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, LOW);
        GPIO_DisableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN);
    }
    else if (driver_state == ild8150_state_drive) {
        if (set_brightness == LIGHTCONTROL_BRIGHTNESS_MIN) {
            GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, LOW);
            GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, LOW);
            GPIO_DisableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN);

            main_beam_state = lightcontrol_feature_state_off;
        }
        else if (set_brightness >= LIGHTCONTROL_BRIGHTNESS_MAX) {
            GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, HIGH);
            GPIO_PinWrite(ILD8150_DIM_PORT, ILD8150_DIM_PIN, HIGH);
            GPIO_DisableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN);
        }
        else {
            TCC_SetCompareCapture(TCC2, ILD8150_DIM_WO, set_brightness);
            GPIO_PinWrite(ILD8150_SHUTDOWN_PORT, ILD8150_SHUTDOWN_PIN, HIGH);
            GPIO_EnableFunction(ILD8150_DIM_PORT, ILD8150_DIM_PIN, ILD8150_DIM_PINMUX);
        }

        if (set_brightness > LIGHTCONTROL_BRIGHTNESS_MIN) {
            if ((ADC1_REGS->ADC_INTFLAG & ADC_INTFLAG_RESRDY_Msk) != 0) {
                ADC1_REGS->ADC_INTFLAG = ADC_INTFLAG_RESRDY_Msk;
                vmon_voltage_raw = ADC1_REGS->ADC_RESULT;
            
                if (vmon_voltage_raw >= LED_DRIVER_VMON_WINDOW_HIGH && vmon_error_counter < LED_DRIVER_VMON_SAMPLE_COUNT) {
                    vmon_error_counter++;
                }
                else if (vmon_voltage_raw <= LED_DRIVER_VMON_WINDOW_LOW && vmon_error_counter < LED_DRIVER_VMON_SAMPLE_COUNT) {
                    vmon_error_counter++;
                }
                else if (vmon_error_counter > 0) {
                    vmon_error_counter--;
                }

                if (vmon_error_counter >= LED_DRIVER_VMON_SAMPLE_COUNT) {
                    main_beam_state = lightcontrol_feature_state_error;
                }
                else {
                    main_beam_state = lightcontrol_feature_state_ok;
                }
            }
        }
    }
}