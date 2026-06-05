#include "app/strobe.h"

#include <stdint.h>
#include <stdbool.h>

#include "common/swtimer.h"

#include "app/feature.h"
#include "app/config.h"
#include "app/brightness.h"

static strobe_source_t STROBE_Source;

/* 1ms data */
static bool STROBE_Flag;
static swtimer_t* STROBE_Timer;
static uint8_t STROBE_RapidCycle;

/* 100ms data */
static uint8_t STROBE_DutyCycle;

/* Configuration data */
static uint16_t STROBE_ConfSingleOnTime;
static uint16_t STROBE_ConfSingleOffTime;
static uint16_t STROBE_ConfRapidOnTime;
static uint16_t STROBE_ConfRapidOffTime;

void STROBE_Init(void) {
    STROBE_Source = strobe_source_disabled;
    STROBE_Flag = false;
    STROBE_Timer = SWTIMER_Create();
    STROBE_RapidCycle = 0;

    STROBE_DutyCycle = 0;

    STROBE_LoadConfig();

    SWTIMER_Setup(STROBE_Timer, STROBE_ConfSingleOffTime);
}

void STROBE_LoadConfig(void) {
    STROBE_ConfSingleOnTime = CONFIG_Props.Strobe_SingleOnTime;
    STROBE_ConfSingleOffTime = CONFIG_Props.Strobe_SingleOffTime;
    STROBE_ConfRapidOnTime = CONFIG_Props.Strobe_RapidOnTime;
    STROBE_ConfRapidOffTime = CONFIG_Props.Strobe_RapidOffTime;
}

void STROBE_SetSource(strobe_source_t source) {
    STROBE_Source = source;
}

void STROBE_Update1ms(void) {
    if (STROBE_Source == strobe_source_disabled) {
        /* In case the source is disabled blinking should also be disabled */
        BRIGHTNESS_SetStrobe(true);
    }
    else if (STROBE_Source == strobe_source_internal_single) {
        if (SWTIMER_Elapsed(STROBE_Timer)) {
            STROBE_Flag = !STROBE_Flag;
            if (STROBE_Flag) {
                SWTIMER_Setup(STROBE_Timer, STROBE_ConfSingleOnTime);
            }
            else {
                SWTIMER_Setup(STROBE_Timer, STROBE_ConfSingleOffTime);
            }
            BRIGHTNESS_SetStrobe(STROBE_Flag);
        }
    }
    else if (STROBE_Source == strobe_source_internal_rapid) {
        if (SWTIMER_Elapsed(STROBE_Timer)) {
            /* Rapid cycles  _0_______/¨1¨\_2_/¨3¨\_4_/¨5¨\_ */
            STROBE_RapidCycle++;
            if (STROBE_RapidCycle > FEATURE_STROBE_RAPID_CYCLES) {
                STROBE_RapidCycle = 0;
            }

            uint16_t timer = 0;
            if (STROBE_RapidCycle == 0) {
                timer = STROBE_ConfRapidOffTime;
                STROBE_Flag = false;
            }
            else {
                timer = STROBE_ConfRapidOnTime;
                STROBE_Flag = !STROBE_Flag;
            }

            SWTIMER_Setup(STROBE_Timer, timer);
            BRIGHTNESS_SetStrobe(STROBE_Flag);
        }
    }
    // TODO: support for external positive and negative sources
    // TODO: in all cases the LEDs shall never be off for more than 1.5seconds
    //       if that's the case the strobe shall self disable and set an error flag
    else {
        /* Same as disabled */
        BRIGHTNESS_SetStrobe(true);
    }
}

uint8_t STROBE_CalculateDutyCycle(strobe_source_t source) {
    if (source == strobe_source_internal_single) {
        uint16_t on_time = STROBE_ConfSingleOnTime;
        uint16_t off_time = STROBE_ConfSingleOffTime;
        return (on_time * 100) / (on_time + off_time);
    }
    else if (source == strobe_source_internal_rapid) {
        uint16_t on_time = STROBE_ConfRapidOnTime * FEATURE_STROBE_RAPID_CYCLES;
        uint16_t off_time = STROBE_ConfRapidOffTime + STROBE_ConfRapidOnTime * (FEATURE_STROBE_RAPID_CYCLES - 1);
        return (on_time * 100) / (on_time + off_time);
    }
    else {
        return 100;
    }
}

void STROBE_Update100ms(void) {
    STROBE_DutyCycle = STROBE_CalculateDutyCycle(STROBE_Source);
}
