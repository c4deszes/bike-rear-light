#if !defined(BSP_LIGHT_CONTROL_H_)
#define BSP_LIGHT_CONTROL_H_

#include <stdint.h>

#define LIGHTCONTROL_BRIGHTNESS_MIN 0
#define LIGHTCONTROL_BRIGHTNESS_MAX 1000

typedef enum {
    lightcontrol_feature_tail_segment,
    lightcontrol_feature_brake_segment
} lightcontrol_feature_t;

void LIGHTCONTROL_Init(void);

/**
 * @brief Sets the state of the given feature, a state of `false` is equivalent to the feature being
 *        off and `true` being on.
 * 
 * @param feature Feature specification
 * @param enabled Feature's state
 */
void LIGHTCONTROL_SetState(lightcontrol_feature_t feature, bool enabled);

/**
 * @brief Sets the brightness of the given feature, the brightness `LIGHTCONTROL_BRIGHTNESS_MIN` is
 *        equivalent to the feature being off, the value `LIGHTCONTROL_BRIGHTNESS_MAX` being the
 *        maximum brightness and anything in between is mapped 
 * 
 * @param feature 
 * @param brightness 
 */
void LIGHTCONTROL_SetBrightness(lightcontrol_feature_t feature, uint16_t brightness);

#endif // BSP_LIGHT_CONTROL_H_
