#include "common/scheduler.h"
#include "common/swtimer.h"
#include "hal/wdt.h"

#include <stdint.h>

#include "app/feature.h"
#include "app/comm.h"
#include "app/brightness.h"
#include "app/brake.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "bsp/light_control.h"
#include "bsp/line_usart.h"

#include "line_api.h"

void SCH_Task1ms(void) {
    SWTIMER_Update1ms();

    STROBE_Update1ms();

    LINE_USART_Receive();
}

void SCH_Task10ms_A(void) {

    SYSSTATE_Update10ms();

#if FEATURE_BRAKE_USE_EXTERNAL_SIGNAL == 1 || FEATURE_BRAKE_USE_INTERNAL_SIGNAL == 1
    BRAKE_Update10ms();
#endif

    // TODO: enable watchdog
    //WDT_Acknowledge();

    COMM_Update();
    COMM_UpdateSignals();

#if FEATURE_COMM_ENABLE_DEBUG_SIGNALS == 1
    COMM_UpdateDebugSignals();
#endif

    BRIGHTNESS_Update10ms();

    LIGHTCONTROL_Update10ms();
}
