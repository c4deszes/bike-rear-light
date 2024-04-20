#if !defined(APP_COMM_H_)
#define APP_COMM_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initializes the communication stack
 * - Application protocol
 * - Transport layer
 * - Physical layer
 */
void COMM_Initialize(void);

/**
 * @brief Updates the signals that are published by this peripheral
 */
void COMM_UpdateSignals(void);

void COMM_UpdateDebugSignals(void);

/**
 * @brief Passes the bytes received from the physical layer to the transport layer
 */
void COMM_UpdatePhy(void);

uint16_t COMM_GetTargetBrightness(void);

/**
 * @brief Returns true if the time since the last LINE frames has exceeded
 *        FEATURE_COMM_LIGHTREQUEST_TIMEOUT, specifically for the following frames:
 * 
 *          - LightSynchronization
 *          - RearLightSetting
 * 
 * @return true 
 * @return false 
 */
bool COMM_LightRequestTimeout(void);

uint8_t COMM_LightMode(void);

uint8_t COMM_LightBehavior(void);

bool COMM_SpeedStatusTimeout(void);

bool COMM_SpeedStatusBraking(void);

/**
 * @brief Returns true if boot entry was requested via LINE Flash protocol
 * 
 * @return true When boot entry is requested
 * @return false Otherwise
 */
bool COMM_BootRequest(void);

#endif // APP_COMM_H_
