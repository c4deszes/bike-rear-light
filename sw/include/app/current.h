#ifndef APP_CURRENT_H_
#define APP_CURRENT_H_

#include <stdint.h>
#include <stdbool.h>

#define CURRENT_DERATING_NONE 1000u
#define CURRENT_DERATING_MAX 0u

typedef enum {
    current_estimate_state_na,       /**< Current estimate has not been measured yet */
    current_estimate_state_ok,       /**< Current estimate is within acceptable range */
    current_estimate_state_error     /**< Current estimate is faulty */
} current_estimate_state_t;

void CURRENT_Init(void);

void CURRENT_Update100ms(void);

uint16_t CURRENT_GetCurrent(void);

current_estimate_state_t CURRENT_GetEstimateState(void);

bool CURRENT_ThermalDeratingActive(void);

bool CURRENT_ThermalShutdownActive(void);

/**
 * @brief Gets the derating factor based on the temperature and voltage, this ranges from 0 (full derating)
 *        to 1000 (no derating)
 * 
 * @return The current derating factor in percentage (0-100)
 */
uint16_t CURRENT_GetDeratingFactor(void);

void CURRENT_LoadConfig(void);

#endif /* APP_CURRENT_H_ */
