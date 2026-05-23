#ifndef APP_TEMP_H
#define APP_TEMP_H

#include <stdint.h>

typedef enum {
    temp_status_not_measured,       /**< Temperature has not been measured yet */
    temp_status_ok,                 /**< Temperature is within acceptable range */
    temp_status_error               /**< Temperature measurement is faulty */
} temp_status_t;

/**
 * @brief Initializes the temperature monitoring module, this should be called once at startup
 */
void TEMP_Init(void);

/**
 * @brief Updates the temperature measurements and their status. Should be called every 100ms.
 */
void TEMP_Update100ms(void);

/**
 * @brief Gets the current drive temperature, this is based on the NTC thermistor placed near
 *        the LED. The measured ADC value is converted to a temperature in degree Celsius using a
 *        lookup table and linear interpolation.
 * 
 * @return The current drive temperature in Celsius
 */
int8_t TEMP_GetDriveTemperature(void);

/**
 * @brief Gets the current MCU temperature, this is based on the internal temperature sensor of the
 *        MCU. The measured ADC value is converted to a temperature in degree Celsius using a linear
 *        formula based on calibration values.
 * 
 * @return The current MCU temperature in Celsius
 */
int8_t TEMP_GetMcuTemperature(void);

/**
 * @brief Gets the current temperature, this is based on the drive temperature and the MCU temperature.
 *        Normally it is calculated by including both measurements with different factors.
 *        If either of the measurements is considered faulty then there are fallbacks the system can use.
 * 
 * @return The current temperature in Celsius
 */
int8_t TEMP_GetTemperature(void);

/**
 * @brief Gets the current temperature status, this is based on the value and status of both the drive
 *        and MCU temperature measurements. Generally this will only return an error if both are faulty.
 * 
 * @return The current temperature status
 */
temp_status_t TEMP_GetStatus(void);

#endif // APP_TEMP_H
