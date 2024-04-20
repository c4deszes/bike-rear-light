#include "common/scheduler.h"
#include "common/swtimer.h"
#include "hal/wdt.h"

#include <stdint.h>

#include "app/comm.h"
#include "app/brightness.h"
#include "app/brake.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "bsp/light_control.h"

#include "line_api.h"

void SCH_Task1ms(void) {
    SWTIMER_Update1ms();

    STROBE_Update1ms();

    // TODO: comm update
    COMM_UpdatePhy();
}

uint16_t counter = 0;
bool state = false;

void SCH_Task10ms_A(void) {

    SYSSTATE_Update10ms();

    BRIGHTNESS_Update10ms();

    BRAKE_Update10ms();

    //WDT_Acknowledge();

    COMM_UpdateSignals();

    // TODO: only do if debugging is enabled
    COMM_UpdateDebugSignals();

    if (counter > 1000) {   // every 10 seconds
        //LIGHTCONTROL_RunDiagnostics(lightcontrol_feature_tail_segment);
        LIGHTCONTROL_RunDiagnostics(lightcontrol_feature_brake_segment);
        counter = 0;
    }
    counter++;

    LIGHTCONTROL_Update10ms();
}
