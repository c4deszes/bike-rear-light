#ifndef LIGHTSIM_H
#define LIGHTSIM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "bsp/light_control.h"

uint16_t LIGHTSIM_GetTailLightBrightness();

void LIGHTSIM_SetTailLightState(lightcontrol_feature_state_t state);

#if defined(__cplusplus)
}
#endif

#endif // LIGHTSIM_H