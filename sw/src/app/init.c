#include "app/init.h"

// Hardware abstraction layer
#include "hal/wdt.h"
#include "hal/nvic.h"
#include "hal/nvmctrl.h"
#include "hal/tcc.h"
#include "common/scheduler.h"

// Board support package
#include "bsp/board.h"
#include "bsp/light_control.h"
#include "bsp/accel.h"
#include "bsp/line_usart.h"

#include "app/feature.h"
#include "app/brake.h"
#include "app/brightness.h"
#include "app/comm.h"
#include "app/config.h"
#include "app/current.h"
#include "app/diag.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "app/temp.h"
#include "app/volt.h"

void APP_Init() {
    // Low level init
    NVMCTRL_SetAutoPageWrite(false);
    NVMCTRL_SetReadWaitStates(0);

    // TODO: enable watchdog
    //WDT_InitializeNormal(&wdt_config);

    BSP_ClockInitialize();

    LIGHTCONTROL_Init();
#if FEATURE_BRAKE_ENABLE_SENSOR == 1
    ACCEL_Init();
#endif

    // Initializing communication
    LINE_USART_Init();
    COMM_Init();
    DIAG_Init();

#if FEATURE_CONFIG_LOAD_AT_STARTUP == 1
    CONFIG_LoadNvram();
#endif
    CONFIG_Reload();

    // Initializing application services
    SYSSTATE_Init();

    CURRENT_Init();
    VOLT_Init();
    TEMP_Init();

    BRIGHTNESS_Init();
    STROBE_Init();
    BRAKE_Init();

    // Setting up scheduler
    // TODO: replace with SysTick
    SCH_Init();
    TCC_Reset(TCC0);
    TCC_SetupTrigger(TCC0, 1000);   // 1000us period
    TCC_Enable(TCC0);

    NVIC_Initialize();
}

void TCC0_Interrupt(void) {
    SCH_Trigger();
}
