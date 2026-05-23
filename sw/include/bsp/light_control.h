#if !defined(BSP_LIGHT_CONTROL_H_)
#define BSP_LIGHT_CONTROL_H_

#include <stdint.h>

#define LIGHTCONTROL_BRIGHTNESS_MIN 0
#define LIGHTCONTROL_BRIGHTNESS_MAX 1000

typedef enum {
    lightcontrol_feature_state_ok,
    lightcontrol_feature_state_partial_error,
    lightcontrol_feature_state_error
} lightcontrol_feature_state_t;

typedef enum {
    lightcontrol_segment_tail,
    lightcontrol_segment_brake,
    lightcontrol_segment_turn_signal_left,
    lightcontrol_segment_turn_signal_right
} lightcontrol_segment_t;

typedef enum {
    lightcontrol_drive_mode_linear,
    lightcontrol_drive_mode_buck
} lightcontrol_drive_mode_t;

void LIGHTCONTROL_Init(void);

void LIGHTCONTROL_Update10ms(void);

/**
 * @brief Set the brightness of a light segment.
 * @param segment The light segment to set the brightness for.
 * @param brightness The brightness value to set (0 to 1000).
 * @note The brightness value is scaled to the range supported by the hardware.
 */
void LIGHTCONTROL_SetBrightness(lightcontrol_segment_t segment, uint16_t brightness);

lightcontrol_drive_mode_t LIGHTCONTROL_GetDriveMode(lightcontrol_segment_t segment);

lightcontrol_feature_state_t LIGHTCONTROL_GetDiagnosticState(lightcontrol_segment_t segment);

#endif // BSP_LIGHT_CONTROL_H_
