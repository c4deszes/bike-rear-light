#include "app/sys_state.h"

// Hardware Abstraction Layer
#include "common/swtimer.h"
#include "hal/nvic.h"

// Board Support Package
#include "bsp/usart.h"

// Communication protocols and external libs
#include "line_api.h"
#include "uds_gen.h"
#include "bl/api.h"

// Application modules
#include "app/config.h"
#include "app/feature.h"
#include "app/brightness.h"
#include "app/strobe.h"
#include "app/comm.h"
#include "app/diag.h"

typedef enum {
    sys_state_init,         /* When starting up */
    sys_state_normal,       /* When target signal is valid */
    sys_state_safety,       /* When communication issues occur */
    sys_state_goto_boot,    /* When boot entry is requested */
    sys_state_goto_reset,   /* When reset is requested */
    sys_state_goto_sleep    /* When sleep is requested */
} sys_state_t;

static sys_state_t SYSSTATE_State;
static swtimer_t* SYSSTATE_TransitionTimer; 

strobe_source_t SYSSTATE_ConfStrobeDefault = strobe_source_disabled;
strobe_source_t SYSSTATE_ConfStrobePrimary = strobe_source_disabled;
strobe_source_t SYSSTATE_ConfStrobeSafety = strobe_source_disabled;
strobe_source_t SYSSTATE_ConfStrobeEmergency = strobe_source_disabled;

static strobe_source_t ConvertOperationalStrobeSource(uint8_t value) {
    switch(value) {
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_DISABLED:
            return strobe_source_disabled;
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_SINGLE:
            return strobe_source_internal_single;
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_RAPID:
            return strobe_source_internal_rapid;
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_EXTERNAL_POS:
            return strobe_source_external_positive;
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_EXTERNAL_NEG:
            return strobe_source_external_negative;
        default:
            return strobe_source_disabled;
    }
}

static strobe_source_t ConvertReserveStrobeSource(uint8_t value) {
    switch(value) {
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_DISABLED:
            return strobe_source_disabled;
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_SINGLE:
            return strobe_source_internal_single;
        case UDS_APP_PROPERTY_RearLight_Strobe_ModeDefault_VALUE_INTERNAL_RAPID:
            return strobe_source_internal_rapid;
        default:
            return strobe_source_disabled;
    }
}

void SYSSTATE_LoadConfig(void) {
    SYSSTATE_ConfStrobeDefault = ConvertOperationalStrobeSource(UDS_Properties_RearLight.Strobe_ModeDefault);
    SYSSTATE_ConfStrobePrimary = ConvertOperationalStrobeSource(UDS_Properties_RearLight.Strobe_ModePrimary);
    SYSSTATE_ConfStrobeSafety = ConvertReserveStrobeSource(UDS_Properties_RearLight.Strobe_ModeSafety);
    SYSSTATE_ConfStrobeEmergency = ConvertReserveStrobeSource(UDS_Properties_RearLight.Strobe_ModeEmergency);
}

void SYSSTATE_Init(void) {
    SYSSTATE_State = sys_state_init;
    SYSSTATE_TransitionTimer = SWTIMER_Create();
    SWTIMER_Setup(SYSSTATE_TransitionTimer, FEATURE_SYSTEM_TIME_INIT);

    SYSSTATE_LoadConfig();
}

// TODO: move attribute to bootloader api
uint64_t boot_entry_key __attribute__((section(".bl_shared_ram")));
static void SYSSTATE_BootEntry(void) {
    boot_entry_key = BL_BOOT_ENTRY_MAGIC;

    NVIC_Reset();
}

void SYSSTATE_Update10ms(void) {
    if (SYSSTATE_State == sys_state_init && SWTIMER_Elapsed(SYSSTATE_TransitionTimer)) {
        if (DIAG_ShutdownRequest()) {
            SYSSTATE_State = sys_state_goto_sleep;
        }
        else if (DIAG_BootRequest()) {
            SYSSTATE_State = sys_state_goto_boot;
        }
        else if (COMM_LightRequestTimeout()) {
            SYSSTATE_State = sys_state_safety;
        }
        else {
            SYSSTATE_State = sys_state_normal;
        }
    }
    else if (SYSSTATE_State == sys_state_normal) {
        STROBE_SetSource(COMM_LightBehavior(SYSSTATE_ConfStrobeDefault, SYSSTATE_ConfStrobePrimary));
        BRIGHTNESS_SetMode(COMM_LightMode());
        BRIGHTNESS_SetTarget(COMM_GetTargetBrightness());

        if (DIAG_ShutdownRequest()) {
            SYSSTATE_State = sys_state_goto_sleep;
        }
        else if (DIAG_BootRequest()) {
            SYSSTATE_State = sys_state_goto_boot;
        }
        else if (COMM_LightRequestTimeout() && COMM_LightMode() != brightness_mode_emergency) {
            /* If the master's last instruction was emergency mode then we don't transition out  */
            SYSSTATE_State = sys_state_safety;
        }
    }
    else if (SYSSTATE_State == sys_state_safety) {
        STROBE_SetSource(SYSSTATE_ConfStrobeSafety);
        BRIGHTNESS_SetMode(brightness_mode_safety);

        if (DIAG_ShutdownRequest()) {
            SYSSTATE_State = sys_state_goto_sleep;
        }
        else if (DIAG_BootRequest()) {
            SYSSTATE_State = sys_state_goto_boot;
        }
        else if (!COMM_LightRequestTimeout()) {
            SYSSTATE_State = sys_state_normal;
        }
    }
    else if (SYSSTATE_State == sys_state_goto_boot) {
        SYSSTATE_BootEntry();

        while(1);
    }
    else if (SYSSTATE_State == sys_state_goto_sleep) {

#if FEATURE_CONFIG_SAVE_AT_SHUTDOWN == 1
        CONFIG_Save();
#endif

        USART_GoToSleep();

        while(1);
    }
}

