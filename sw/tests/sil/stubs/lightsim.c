#include "bsp/light_control.h"
#include "lightsim.h"

static lightcontrol_feature_state_t current_state = lightcontrol_feature_state_ok;
static uint16_t current_brightness = LIGHTCONTROL_BRIGHTNESS_MAX;

void LIGHTCONTROL_Init(void)
{
    // Simulate initialization.
}

void LIGHTCONTROL_Update10ms(void)
{
    // Simulate updating the light control state.
}

void LIGHTCONTROL_SetBrightness(uint16_t brightness)
{
    current_brightness = brightness;
}

uint16_t LIGHTSIM_GetTailLightBrightness() {
    return current_brightness;
}

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState()
{
    return current_state;
}

void LIGHTSIM_SetTailLightState(lightcontrol_feature_state_t state) {
    current_state = state;
}
