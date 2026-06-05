#include "app/router.h"
#include "app/diag.h"
#include "app/brightness.h"
#include "app/feature.h"

#include "app/comm.h"

#include "bsp/light_control.h"

#define ROUTER_MAX_DIAG_CHANNELS 4

static struct {
    bool active;
    uint16_t activeTime;
    uint8_t mode;
    uint16_t brightness;
} ROUTER_DiagRequests[ROUTER_MAX_DIAG_CHANNELS];
static bool ROUTER_DiagRequestActive;

void ROUTER_Init(void) {
    // Initialization code for the router
}

void ROUTER_Update10ms(void) {
    // Update code for the router, called every 10ms

    if (!ROUTER_DiagRequestActive) {
        uint16_t tail_target = BRIGHTNESS_GetOutput(brightness_output_tail);
        uint16_t brake_target = BRIGHTNESS_GetOutput(brightness_output_brake);

        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, tail_target);
        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, brake_target);
    }
    else {
        bool any_active = false;

        for (int i = 0; i < ROUTER_MAX_DIAG_CHANNELS; i++) {
            if (ROUTER_DiagRequests[i].active) {
                ROUTER_DiagRequests[i].activeTime += 10;

                if (ROUTER_DiagRequests[i].activeTime >= FEATURE_DIAG_DRIVER_CONTROL_TIMEOUT) {
                    ROUTER_DiagRequests[i].active = false;
                }
                else {
                    any_active = true;
                    // TODO: set control according to mode and brightness, for now just set brightness

                    if (i == brightness_output_tail) {
                        LIGHTCONTROL_SetBrightness(lightcontrol_segment_tail, ROUTER_DiagRequests[i].brightness);
                    }
                    else if (i == brightness_output_brake) {
                        LIGHTCONTROL_SetBrightness(lightcontrol_segment_brake, ROUTER_DiagRequests[i].brightness);
                    }
                }
            }
        }

        ROUTER_DiagRequestActive = any_active;
    }
}

bool ROUTER_DiagRequest(brightness_output_t output, uint8_t mode, uint16_t brightness)
{

    if (output >= ROUTER_MAX_DIAG_CHANNELS) {
        return false;
    }

    if (
        // Below 2.0 km/h diagnostics are allowed
        (!COMM_SpeedStatusTimeout() && COMM_SpeedValid() && COMM_GetSpeed() < 20) ||
        // When there's no speed data they're allowed but only if the requested light mode is off
        (COMM_SpeedStatusTimeout() && !COMM_LightRequestTimeout() && COMM_LightMode() == brightness_mode_off)
    ) {

        ROUTER_DiagRequests[output].active = true;
        ROUTER_DiagRequests[output].activeTime = 0;
        ROUTER_DiagRequests[output].mode = mode;
        ROUTER_DiagRequests[output].brightness = brightness;

        ROUTER_DiagRequestActive = true;

        return true;
    }
    
    return false;
}
