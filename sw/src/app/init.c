#include "app/init.h"

// Hardware abstraction layer
#include "hal/adc.h"
#include "hal/wdt.h"
#include "hal/nvic.h"
#include "hal/nvmctrl.h"
#include "hal/tcc.h"
#include "hal/systick.h"
#include "common/scheduler.h"

// Board support package
#include "bsp/board.h"
#include "bsp/light_control.h"
#include "bsp/accel.h"
#include "bsp/line_usart.h"
#include "bsp/tt_adc.h"

#include "app/feature.h"
#include "app/brake.h"
#include "app/brightness.h"
#include "app/comm.h"
#include "app/config.h"
#include "app/calib.h"
#include "app/current.h"
#include "app/diag.h"
#include "app/strobe.h"
#include "app/sys_state.h"
#include "app/temp.h"
#include "app/volt.h"
#include "app/wake.h"

void APP_Init() {
    // Low level init
    NVMCTRL_SetAutoPageWrite(false);
    NVMCTRL_SetReadWaitStates(0);

    // TODO: enable watchdog
    //WDT_InitializeNormal(&wdt_config);

    WAKE_Init();

    BSP_ClockInitialize();
    ADC_SetupSingleShot();
    TTADC_Init();

    LIGHTCONTROL_Init();
#if FEATURE_BRAKE_ENABLE_SENSOR == 1
    ACCEL_Init();
#endif

    // Initializing communication
    LINE_USART_Init();
    COMM_Init();
    DIAG_Init();

#if FEATURE_CONFIG_LOAD_AT_STARTUP == 1
    /* This call needs to happen after UDS_Init */
    CONFIG_LoadNvram();
#endif
    CONFIG_Reload();

    CALIB_Init();

    // Initializing application services
    SYSSTATE_Init();

    CURRENT_Init();
    VOLT_Init();
    TEMP_Init();

    BRIGHTNESS_Init();
    STROBE_Init();
    BRAKE_Init();

    // Setting up scheduler
    SCH_Init();
    SYSTICK_Setup(48000000u / 1000u);

    NVIC_Initialize();
}

void SysTick_Handler(void) {
    SCH_Trigger();
}