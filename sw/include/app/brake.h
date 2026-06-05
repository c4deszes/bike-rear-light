#if !defined(APP_BRAKE_H_)
#define APP_BRAKE_H_

/**
 * @file brake.h
 * @brief Brake detection and control module
 *
 * This module handles brake signal detection using an accelerometer sensor and/or external signals.
 */
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    brake_signal_status_na,
    brake_signal_status_ok,
    brake_signal_status_perm_error
} brake_signal_status_t;

void BRAKE_Init(void);

void BRAKE_Update10ms(void);

// Diagnostics

brake_signal_status_t BRAKE_GetInternalStatus(void);

bool BRAKE_GetInternalBraking(void);

void BRAKE_GetAcceleration(int16_t* x, int16_t* y, int16_t* z);

#endif // APP_BRAKE_H_
