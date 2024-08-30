#include "bsp/board.h"
#include "hal/rtc.h"
#include "hal/wdt.h"
#include "hal/nvic.h"
#include "hal/tcc.h"

#include "common/scheduler.h"

#include "bsp/light_control.h"

#include "app/comm.h"
#include "app/strobe.h"
#include "app/brightness.h"
#include "app/brake.h"
#include "app/sys_state.h"

#include <stddef.h>

void APP_Initialize() {
    // Low level init
    BSP_ClockInitialize();
    //WDT_InitializeNormal(&wdt_config);
    //EIC_Initialize(NULL);
    LIGHTCONTROL_Init();

    // Initializing communication
    COMM_Initialize();

    // Initializing application services
    SYSSTATE_Init();
    BRIGHTNESS_Init();
    STROBE_Init();
    BRAKE_Init();

    // Setting up scheduler
    SCH_Init();
    TCC_Reset(TCC0);
    TCC_SetupTrigger(TCC0, 1000);   // 1000us period
    TCC_Enable(TCC0);

    NVIC_Initialize();
}

void TCC0_Interrupt(void) {
    SCH_Trigger();
}
