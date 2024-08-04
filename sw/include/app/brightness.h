#if !defined(APP_BRIGHTNESS_H_)
#define APP_BRIGHTNESS_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    brightness_mode_off,
    brightness_mode_standard,
    brightness_mode_adaptive,
    brightness_mode_emergency,
    brightness_mode_safety,
    brightness_mode_max
} brightness_mode_t;

void BRIGHTNESS_Init(void);

void BRIGHTNESS_SetMode(brightness_mode_t mode);

brightness_mode_t BRIGHTNESS_GetMode(void);

void BRIGHTNESS_SetTarget(uint16_t target);

void BRIGHTNESS_SetBraking(bool brake);

void BRIGHTNESS_Strobe(bool strobe);

void BRIGHTNESS_Update10ms(void);

#endif // APP_BRIGHTNESS_H_
