#include "bsp/board.h"
#include "hal/rtc.h"
#include "hal/wdt.h"
#include "hal/nvic.h"
#include "hal/tcc.h"

#include "common/scheduler.h"

#include "bsp/light_control.h"

#include "app/config.h"
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

    // Initializing application services
    SYSSTATE_Init();
    BRIGHTNESS_Init();
    STROBE_Init();
    BRAKE_Init();

    // Initializing communication
    COMM_Initialize();

    CONFIG_LoadFlashProperties();
    CONFIG_ReloadUdsProperties();

    // Setting up scheduler
    // TODO: replace with SysTick
    SCH_Init();
    TCC_Reset(TCC2);
    TCC_SetupTrigger(TCC2, 1000);   // 1000us period
    TCC_Enable(TCC2);

    NVIC_Initialize();
}

void TCC2_Interrupt(void) {
    SCH_Trigger();
}
