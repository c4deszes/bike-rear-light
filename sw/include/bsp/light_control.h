#if !defined(BSP_LIGHT_CONTROL_H_)
#define BSP_LIGHT_CONTROL_H_

#include <stdint.h>

#define LIGHTCONTROL_BRIGHTNESS_MIN 0
#define LIGHTCONTROL_BRIGHTNESS_MAX 1000

typedef enum {
    lightcontrol_feature_tail_segment,
    lightcontrol_feature_brake_segment
} lightcontrol_feature_t;

typedef enum {
    lightcontrol_feature_state_off,
    lightcontrol_feature_state_ok,
    lightcontrol_feature_state_partial_error,
    lightcontrol_feature_state_error
} lightcontrol_feature_state_t;

void LIGHTCONTROL_Init(void);

void LIGHTCONTROL_Update10ms(void);

/**
 * @brief Sets the brightness of the given feature, the brightness `LIGHTCONTROL_BRIGHTNESS_MIN` is
 *        equivalent to the feature being off, the value `LIGHTCONTROL_BRIGHTNESS_MAX` being the
 *        maximum brightness and anything in between is mapped 
 * 
 * @param feature 
 * @param brightness 
 */
void LIGHTCONTROL_SetBrightness(lightcontrol_feature_t feature, uint16_t brightness);

void LIGHTCONTROL_RunDiagnostics(lightcontrol_feature_t feature);

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState(lightcontrol_feature_t feature);

#endif // BSP_LIGHT_CONTROL_H_
