#include "app/sys_state.h"
#include "common/swtimer.h"
#include "app/config.h"

#include "app/brightness.h"
#include "app/strobe.h"

#include "line_api.h"

typedef enum {
    sys_state_init,         /* When starting up */
    sys_state_normal,       /* When target signal is valid */
    sys_state_safety,
    sys_state_goto_boot,    /* When boot entry is requested */
    sys_state_goto_reset,   /* When reset is requested */
    sys_state_goto_sleep    /* When sleep is requested */
} sys_state_t;

static sys_state_t sys_state;
static swtimer_t* sys_transition_timer; 

void SYSSTATE_Init(void) {
    sys_state = sys_state_init;
    sys_transition_timer = SWTIMER_Create();
    SWTIMER_Setup(sys_transition_timer, CONFIG_SYSTEM_TIME_INIT);
}

void SYSSTATE_Update10ms(void) {
    if (sys_state == sys_state_init && SWTIMER_Elapsed(sys_transition_timer)) {
        // transition into safety or normal

        // if bus idle then safety
        // else go normal
        sys_state = sys_state_safety;
    }
    else if (sys_state == sys_state_normal) {
        // brightness mode should be set to adaptive/standard/emergency/off
        
        // TODO: set strobe source to primary, or emergency if that's the mode
        STROBE_SetSource(strobe_source_disabled);

        BRIGHTNESS_SetMode(brightness_mode_adaptive);
        // TODO: do the scaling elsewhere
        BRIGHTNESS_SetTarget(LINE_Request_LightSynchronization_data.fields.TargetBrightness * 10u);
    }
    else if (sys_state == sys_state_safety) {
        // TODO: source should be based on STROBE_SAFETY_SOURCE
        STROBE_SetSource(strobe_source_disabled);
        BRIGHTNESS_SetMode(brightness_mode_safety);

        // if not bus idle/ new frame -> go back to normal
    }
}

