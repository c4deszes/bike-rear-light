#include "common/scheduler.h"
#include "common/swtimer.h"
#include "hal/wdt.h"

#include <stdint.h>

#include "app/comm.h"
#include "app/brightness.h"
#include "app/brake.h"
#include "app/strobe.h"
#include "app/sys_state.h"

void SCH_Task1ms(void) {
    SWTIMER_Update1ms();

    STROBE_Update1ms();

    // TODO: comm update
    COMM_UpdatePhy();
}

uint8_t counter = 0;
bool state = false;

void SCH_Task10ms_A(void) {

    //SYSSTATE_Update10ms();

    //BRIGHTNESS_Update10ms();

        counter++;

    if (counter > 100) {
        LIGHTCONTROL_SetTailState(true);
        LIGHTCONTROL_SetBrakeState(true);
        if (state) {
            LIGHTCONTROL_SetTailBrightness(100);
            LIGHTCONTROL_SetBrakeBrightness(900);
        }
        else {
            LIGHTCONTROL_SetTailBrightness(900);
            LIGHTCONTROL_SetBrakeBrightness(100);
        }

        state = !state;
        counter = 0;
    }

    // TODO: brake update

    //WDT_Acknowledge();

    //COMM_UpdateSignals();
}
