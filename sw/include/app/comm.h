#if !defined(APP_COMM_H_)
#define APP_COMM_H_

#include <stdbool.h>
#include <stdint.h>
#include "app/strobe.h"
#include "app/brightness.h"

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

/**
 * @brief Updates the debug signals that are published by this peripheral
 *        Only called when FEATURE_COMM_DEBUG_SIGNALS is enabled
 */
void COMM_UpdateDebugSignals(void);

/**
 * @brief Passes the bytes received from the physical layer to the transport layer
 */
void COMM_UpdatePhy(void);

/**
 * @brief Returns the target brightness scaled into the brightness range of the light control
 * 
 * @return uint16_t Target brightness
 */
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

/**
 * @brief Returns the light mode requested by the master device
 * 
 * @return brightness_mode_t Light mode
 */
brightness_mode_t COMM_LightMode(void);

/**
 * @brief Returns the strobe source requested by the master device
 * 
 * @return strobe_source_t Strobe source
 */
strobe_source_t COMM_LightBehavior(void);

/**
 * @brief Returns true if the time since the last LINE frame has exceeded
 *        FEATURE_COMM_SPEEDSTATUS_TIMEOUT, specifically for the following frame:
 * 
 *          - SpeedStatus
 * 
 * @return true 
 * @return false 
 */
bool COMM_SpeedStatusTimeout(void);

/**
 * @brief Returns true if the speed status frame indicates that the bicycle is braking
 * 
 * @return true 
 * @return false 
 */
bool COMM_SpeedStatusBraking(void);

/**
 * @brief Returns true if boot entry was requested via LINE Flash protocol, the flag is cleared after reading
 * 
 * @return true When boot entry is requested
 * @return false Otherwise
 */
bool COMM_BootRequest(void);

/**
 * @brief Returns true if shutdown was requested via LINE protocol, the flag is cleared after reading
 * 
 * @return true When shutdown is requested
 * @return false Otherwise
 */
bool COMM_ShutdownRequest(void);

/**
 * @brief Returns true if idle mode was requested via LINE protocol, the flag is cleared after reading
 * 
 * @return true When idle mode is requested
 * @return false Otherwise
 */
bool COMM_IdleRequest(void);

#endif // APP_COMM_H_
