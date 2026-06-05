#ifndef APP_VOLT_H
#define APP_VOLT_H_

#include <stdint.h>

#define VOLT_CALIB_SLOPE_MAX 1200
#define VOLT_CALIB_SLOPE_MIN 300

typedef enum {
    volt_status_not_measured,   /**< Voltage has not been measured yet */
    volt_status_ok,             /**< Voltage is within the normal range */
    volt_status_low,            /**< Voltage is below the low threshold */
    volt_status_error           /**< Voltage measurement error */
} volt_status_t;

void VOLT_Init(void);

void VOLT_Update100ms(void);

/**
 * @brief Get the latest measured voltage value
 * @return Voltage in 100mV steps (e.g. 120 = 12.0V)
 */
uint16_t VOLT_GetVoltage(void);

uint16_t VOLT_GetAdcRawValue(void);

/**
 * @brief Get the current voltage measurement status
 * @return Voltage status
 */
volt_status_t VOLT_GetStatus(void);

#endif // APP_VOLT_H_
