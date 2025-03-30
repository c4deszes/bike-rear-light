#include "app/init.h"

#include "hal/rtc.h"
#include "hal/wdt.h"
#include "hal/nvic.h"
#include "hal/nvmctrl.h"
#include "hal/tcc.h"
#include "common/scheduler.h"

#include "bsp/board.h"
#include "bsp/light_control.h"

#include "app/brake.h"
#include "app/brightness.h"
#include "app/comm.h"
#include "app/config.h"
#include "app/strobe.h"
#include "app/sys_state.h"

void APP_Initialize() {
    // Low level init
    NVMCTRL_SetAutoPageWrite(false);
    BSP_ClockInitialize();
    // TODO: enable watchdog
    //WDT_InitializeNormal(&wdt_config);
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
