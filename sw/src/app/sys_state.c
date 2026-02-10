#include "app/sys_state.h"

#include "common/swtimer.h"
#include "hal/nvic.h"

#include "bsp/usart.h"

#include "app/config.h"
#include "app/feature.h"
#include "app/brightness.h"
#include "app/strobe.h"
#include "app/comm.h"

#include "line_api.h"
#include "uds_gen.h"
#include "bl/api.h"

typedef enum {
    sys_state_init,         /* When starting up */
    sys_state_normal,       /* When target signal is valid */
    sys_state_safety,       /* When communication issues occur */
    sys_state_goto_boot,    /* When boot entry is requested */
    sys_state_goto_reset,   /* When reset is requested */
    sys_state_goto_sleep    /* When sleep is requested */
} sys_state_t;

static sys_state_t sys_state;
static swtimer_t* sys_transition_timer; 

strobe_source_t sys_config_default_strobe = strobe_source_disabled;
strobe_source_t sys_config_primary_strobe = strobe_source_disabled;
strobe_source_t sys_config_safety_strobe = strobe_source_disabled;
strobe_source_t sys_config_emergency_strobe = strobe_source_disabled;

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
    sys_config_default_strobe = ConvertOperationalStrobeSource(UDS_Properties_RearLight.Strobe_ModeDefault);
    sys_config_primary_strobe = ConvertOperationalStrobeSource(UDS_Properties_RearLight.Strobe_ModePrimary);
    sys_config_safety_strobe = ConvertReserveStrobeSource(UDS_Properties_RearLight.Strobe_ModeSafety);
    sys_config_emergency_strobe = ConvertReserveStrobeSource(UDS_Properties_RearLight.Strobe_ModeEmergency);
}

void SYSSTATE_Init(void) {
    sys_state = sys_state_init;
    sys_transition_timer = SWTIMER_Create();
    SWTIMER_Setup(sys_transition_timer, FEATURE_SYSTEM_TIME_INIT);

    SYSSTATE_LoadConfig();
}

uint64_t boot_entry_key __attribute__((section(".bl_shared_ram")));
static void SYSSTATE_BootEntry(void) {
    boot_entry_key = BL_BOOT_ENTRY_MAGIC;

    NVIC_Reset();
}

void SYSSTATE_Update10ms(void) {
    if (sys_state == sys_state_init && SWTIMER_Elapsed(sys_transition_timer)) {
        if (COMM_LightRequestTimeout()) {
            sys_state = sys_state_safety;
        }
        else if (COMM_BootRequest()) {
            sys_state = sys_state_goto_boot;
        }
        else {
            sys_state = sys_state_normal;
        }
    }
    else if (sys_state == sys_state_normal) {
        STROBE_SetSource(COMM_LightBehavior());
        BRIGHTNESS_SetMode(COMM_LightMode());
        BRIGHTNESS_SetTarget(COMM_GetTargetBrightness());

        // Critical state transitions
        if (COMM_ShutdownRequest()) {
            sys_state = sys_state_goto_sleep;
        }
        else if (COMM_BootRequest()) {
            sys_state = sys_state_goto_boot;
        }
        else if (COMM_IdleRequest()) {
            // TODO: either safety mode or emergency mode
        }

        // Transition to safety state
        if (COMM_LightRequestTimeout() && COMM_LightMode() != brightness_mode_emergency) {
            /* If the master's last instruction was emergency mode then we don't transition out  */
            sys_state = sys_state_safety;
        }
    }
    else if (sys_state == sys_state_safety) {
        STROBE_SetSource(sys_config_safety_strobe);
        BRIGHTNESS_SetMode(brightness_mode_safety);

        // Critical state transitions
        if (COMM_ShutdownRequest()) {
            sys_state = sys_state_goto_sleep;
        }
        else if (COMM_BootRequest()) {
            // sys_state = sys_state_goto_boot;
        }
        else if (COMM_IdleRequest()) {
            // TODO: either safety mode or emergency mode
        }
        else if (!COMM_LightRequestTimeout()) {
            sys_state = sys_state_normal;
        }
    }
    else if (sys_state == sys_state_goto_boot) {
        // TODO: reenable once bootloader is implemented
        SYSSTATE_BootEntry();

        while(1);
    }
    else if (sys_state == sys_state_goto_sleep) {
        // TODO: optional save config call
        //CONFIG_Save();

        USART_GoToSleep();

        while(1);
    }
}

