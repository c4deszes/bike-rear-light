
// Hardware abstraction layer
#include "common/scheduler.h"
#include "common/swtimer.h"
#include "hal/wdt.h"

// Board support package
#include "bsp/light_control.h"
#include "bsp/line_usart.h"
#include "bsp/tt_adc.h"

#include "app/feature.h"
#include "app/brake.h"
#include "app/brightness.h"
#include "app/comm.h"
#include "app/current.h"
#include "app/diag.h"
#include "app/router.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "app/temp.h"
#include "app/volt.h"

void SCH_Task1ms(void) {
    SWTIMER_Update1ms();

    STROBE_Update1ms();

    LINE_USART_Receive();
}

void SCH_Task10ms_A(void) {
    // TODO: enable watchdog
    //WDT_Acknowledge();

    TTADC_Trigger();

    SYSSTATE_Update10ms();

#if FEATURE_BRAKE_USE_EXTERNAL_SIGNAL == 1 || FEATURE_BRAKE_USE_INTERNAL_SIGNAL == 1
    BRAKE_Update10ms();
#endif

    BRIGHTNESS_Update10ms();
    ROUTER_Update10ms();


    LIGHTCONTROL_Update10ms();

    COMM_Update10ms();
    COMM_UpdateSignals();

#if FEATURE_COMM_ENABLE_DEBUG_SIGNALS == 1
    COMM_UpdateDebugSignals();
#endif

    DIAG_Update10ms();
}

void SCH_Task100ms(void) {

    STROBE_Update100ms();

    VOLT_Update100ms();

    TEMP_Update100ms();

    CURRENT_Update100ms();

    DIAG_Update100ms();
}
