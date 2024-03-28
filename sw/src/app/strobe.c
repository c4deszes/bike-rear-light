#include "app/strobe.h"
#include "app/brightness.h"

#include "common/swtimer.h"

#include <stdbool.h>

static strobe_source_t strobe_source;
static swtimer_t* strobe_internal_timer;
static bool strobe_internal_flag;

void STROBE_Init(void) {
    strobe_source = strobe_source_disabled;
    strobe_internal_timer = SWTIMER_Create();
    SWTIMER_Setup(strobe_internal_timer, 1000);
}

void STROBE_SetSource(strobe_source_t source) {
    strobe_source = source;
}

void STROBE_Update10ms(void) {
    if (strobe_source == strobe_source_disabled) {
        /* In case the source is disabled blinking should also be disabled */
        BRIGHTNESS_SetBlinking(false);
        BRIGHTNESS_Strobe(true);
    }
    else if (strobe_source == strobe_source_internal_1Hz) {
        BRIGHTNESS_SetBlinking(true);
        BRIGHTNESS_Strobe(strobe_internal_flag);
    }
    // TODO: support for external positive and negative sources
    // TODO: in all cases the LEDs shall never be off for more than 1.5seconds
    //       if that's the case the strobe shall self disable
    else {
        /* Same as disabled */
        BRIGHTNESS_SetBlinking(false);
        BRIGHTNESS_Strobe(true);
    }

    /* Updating internal flag based on the timer value */
    if (SWTIMER_Elapsed(strobe_internal_timer)) {
        strobe_internal_flag = !strobe_internal_flag;
        SWTIMER_Setup(strobe_internal_timer, 1000);
    }
}
