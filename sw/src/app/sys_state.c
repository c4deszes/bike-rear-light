#include "app/sys_state.h"

// Hardware Abstraction Layer
#include "common/swtimer.h"
#include "hal/nvic.h"

// Board Support Package
#include "bsp/usart.h"
#include "bsp/light_control.h"

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
#include "app/volt.h"

typedef enum {
    sys_state_init,         /* When starting up */
    sys_state_normal,       /* When target signal is valid */
    sys_state_safety,       /* When communication issues occur */
    sys_state_emergency,    /* When requested by master or the voltage is too low */
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

void SYSSTATE_LoadConfig(void) {
    SYSSTATE_ConfStrobeDefault = CONFIG_Props.Strobe_ModeDefault;
    SYSSTATE_ConfStrobePrimary = CONFIG_Props.Strobe_ModePrimary;
    SYSSTATE_ConfStrobeSafety = CONFIG_Props.Strobe_ModeSafety;
    SYSSTATE_ConfStrobeEmergency = CONFIG_Props.Strobe_ModeEmergency;
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

static bool SYSSTATE_EmergencyCondition(void) {
    /* Emergency mode is entered when requested by the master or when the voltage is too low,
       for this second case a valid voltage calibration must be present and light request must
       have timed out. */
    if (COMM_LightMode() == brightness_mode_emergency) {
        return true;
    }
    else if (COMM_LightRequestTimeout() && VOLT_GetStatus() == volt_status_low) {
        return true;
    }
    return false;
}

static void SYSSTATE_InitTransition(void) {
    if (DIAG_ShutdownRequest()) {
        SYSSTATE_State = sys_state_goto_sleep;
    }
    else if (DIAG_BootRequest()) {
        SYSSTATE_State = sys_state_goto_boot;
    }
    else if (SYSSTATE_EmergencyCondition()) {
        SYSSTATE_State = sys_state_emergency;
    }
    else if (COMM_LightRequestTimeout()) {
        SYSSTATE_State = sys_state_safety;
    }
    else {
        SYSSTATE_State = sys_state_normal;
    }
}

static void SYSSTATE_NormalMode(void) {
    STROBE_SetSource(COMM_LightBehavior(SYSSTATE_ConfStrobeDefault, SYSSTATE_ConfStrobePrimary));
    BRIGHTNESS_SetMode(COMM_LightMode());
    BRIGHTNESS_SetTarget(COMM_GetTargetBrightness());

    if (DIAG_ShutdownRequest()) {
        SYSSTATE_State = sys_state_goto_sleep;
    }
    else if (DIAG_BootRequest()) {
        SYSSTATE_State = sys_state_goto_boot;
    }
    else if (SYSSTATE_EmergencyCondition()) {
        SYSSTATE_State = sys_state_emergency;
    }
    else if (COMM_LightRequestTimeout()) {
        /* If the master's last instruction was emergency mode then we don't transition out  */
        SYSSTATE_State = sys_state_safety;
    }
}

static void SYSSTATE_EmergencyMode(void) {
    STROBE_SetSource(SYSSTATE_ConfStrobeEmergency);
    BRIGHTNESS_SetMode(brightness_mode_emergency);
    BRIGHTNESS_SetTarget(LIGHTCONTROL_BRIGHTNESS_MAX);  /* Not used, set for safety */

    if (DIAG_ShutdownRequest()) {
        SYSSTATE_State = sys_state_goto_sleep;
    }
    else if (DIAG_BootRequest()) {
        SYSSTATE_State = sys_state_goto_boot;
    }
    // TODO: exit conditions
    else if (!SYSSTATE_EmergencyCondition()) {
        SYSSTATE_State = sys_state_normal;
    }
}

static void SYSSTATE_SafetyMode(void) {
    STROBE_SetSource(SYSSTATE_ConfStrobeSafety);
    BRIGHTNESS_SetMode(brightness_mode_safety);
    BRIGHTNESS_SetTarget(LIGHTCONTROL_BRIGHTNESS_MAX);  /* Not used, set for safety */

    if (DIAG_ShutdownRequest()) {
        SYSSTATE_State = sys_state_goto_sleep;
    }
    else if (DIAG_BootRequest()) {
        SYSSTATE_State = sys_state_goto_boot;
    }
    else if (SYSSTATE_EmergencyCondition()) {
        SYSSTATE_State = sys_state_emergency;
    }
    else if (!COMM_LightRequestTimeout()) {
        SYSSTATE_State = sys_state_normal;
    }
}

void SYSSTATE_Update10ms(void) {
    if (SYSSTATE_State == sys_state_init && SWTIMER_Elapsed(SYSSTATE_TransitionTimer)) {
        SYSSTATE_InitTransition();
    }
    else if (SYSSTATE_State == sys_state_normal) {
        SYSSTATE_NormalMode();
    }
    else if (SYSSTATE_State == sys_state_safety) {
        SYSSTATE_SafetyMode();
    }
    else if (SYSSTATE_State == sys_state_emergency) {
        SYSSTATE_EmergencyMode();
    }
    else if (SYSSTATE_State == sys_state_goto_boot) {
        SYSSTATE_BootEntry();

        /* Control should never reach here */
        while(1);
    }
    else if (SYSSTATE_State == sys_state_goto_sleep) {

#if FEATURE_CONFIG_SAVE_AT_SHUTDOWN == 1
        CONFIG_Reload();
        CONFIG_Save();
#endif

        USART_GoToSleep();

        /* Control should never reach here */
        while(1);
    }
}

