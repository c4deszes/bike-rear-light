#include "app/strobe.h"
#include "app/brightness.h"

#include "common/swtimer.h"

#include "app/config.h"

#include <stdbool.h>

static strobe_source_t strobe_source;
static swtimer_t* strobe_internal_timer;
static bool strobe_internal_flag;
static uint8_t strobe_rapid_cycle;

void STROBE_Init(void) {
    strobe_internal_flag = false;
    strobe_rapid_cycle = 0;
    strobe_source = strobe_source_disabled;
    strobe_internal_timer = SWTIMER_Create();
    SWTIMER_Setup(strobe_internal_timer, CONFIG_BRIGHTNESS_STROBE_SINGLE_OFF_TIME);
}

void STROBE_SetSource(strobe_source_t source) {
    strobe_source = source;
}

void STROBE_Update1ms(void) {
    if (strobe_source == strobe_source_disabled) {
        /* In case the source is disabled blinking should also be disabled */
        BRIGHTNESS_Strobe(true);
    }
    else if (strobe_source == strobe_source_internal_single) {
        if (SWTIMER_Elapsed(strobe_internal_timer)) {
            strobe_internal_flag = !strobe_internal_flag;
            if (strobe_internal_flag) {
                SWTIMER_Setup(strobe_internal_timer, CONFIG_BRIGHTNESS_STROBE_SINGLE_ON_TIME);
            }
            else {
                SWTIMER_Setup(strobe_internal_timer, CONFIG_BRIGHTNESS_STROBE_SINGLE_OFF_TIME);
            }
            BRIGHTNESS_Strobe(strobe_internal_flag);
        }
    }
    else if (strobe_source == strobe_source_internal_rapid) {
        if (SWTIMER_Elapsed(strobe_internal_timer)) {
            /* Rapid cycles  _0_______/¨1¨\_2_/¨3¨\_4_/¨5¨\_ */
            strobe_rapid_cycle++;
            if (strobe_rapid_cycle > 5) {
                strobe_rapid_cycle = 0;
            }

            uint8_t timer = 0;
            if (strobe_rapid_cycle == 0) {
                timer = CONFIG_BRIGHTNESS_STROBE_RAPID_OFF_TIME;
                strobe_internal_flag = false;
            }
            else {
                timer = CONFIG_BRIGHTNESS_STROBE_RAPID_SWITCH_TIME;
                strobe_internal_flag = !strobe_internal_flag;
            }

            SWTIMER_Setup(strobe_internal_timer, timer);
            BRIGHTNESS_Strobe(strobe_internal_flag);
        }
    }
    // TODO: support for external positive and negative sources
    // TODO: in all cases the LEDs shall never be off for more than 1.5seconds
    //       if that's the case the strobe shall self disable and set an error flag
    else {
        /* Same as disabled */
        BRIGHTNESS_Strobe(true);
    }
}
