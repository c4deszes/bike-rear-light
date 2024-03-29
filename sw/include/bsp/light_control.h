#if !defined(BSP_LIGHT_CONTROL_H_)
#define BSP_LIGHT_CONTROL_H_

#include <stdint.h>

void LIGHTCONTROL_Init(void);

void LIGHTCONTROL_SetTailState(bool enabled);

void LIGHTCONTROL_SetBrakeState(bool enabled);

void LIGHTCONTROL_SetTailBrightness(uint16_t brightness);

void LIGHTCONTROL_SetBrakeBrightness(uint16_t brightness);

#endif // BSP_LIGHT_CONTROL_H_
