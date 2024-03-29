#include "common/scheduler.h"
#include "hal/wdt.h"

#include <stdint.h>

#include "app/comm.h"
#include "common/swtimer.h"

#include "app/brightness.h"
#include "app/brake.h"
#include "bsp/light_control.h"

void SCH_Task1ms(void) {
    SWTIMER_Update1ms();
}

uint8_t counter = 0;
bool state = false;

void SCH_Task10ms_A(void) {

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

    //BRIGHTNESS_Update10ms();
    //WDT_Acknowledge();

    //IET_Update();

    //ITPMS_Update();

    //COMM_UpdateSignals();
}
