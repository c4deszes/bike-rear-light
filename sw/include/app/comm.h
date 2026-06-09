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
void COMM_Init(void);

/**
 * @brief Updates the signals that are published by this peripheral
 */
void COMM_UpdateSignals(void);

/**
 * @brief Updates the debug signals that are published by this peripheral
 *        Only called when FEATURE_COMM_DEBUG_SIGNALS is enabled
 */
void COMM_UpdateDebugSignals(void);

void COMM_Update10ms(void);

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
 * @param default_source The strobe source to return if the behavior is set to default
 * @param primary_source The strobe source to return if the behavior is set to blink
 * 
 * @return strobe_source_t Strobe source
 */
strobe_source_t COMM_LightBehavior(strobe_source_t default_source, strobe_source_t primary_source);

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
 * @brief Returns true if the speed status frame indicates that the speed is valid
 *        (Ok or SlowResponse)
 * 
 * @return true when the speed status is valid
 * @return false otherwise
 */
bool COMM_SpeedValid(void);

/**
 * @brief Returns the current speed as reported by the master device in the SpeedStatus frame
 * 
 * @return uint16_t Speed in 0.1 km/h increments (e.g. 1234 = 123.4 km/h)
 */
uint16_t COMM_GetSpeed(void);

/**
 * @brief Returns true if the speed status frame indicates that the bicycle is braking,
 *        not used under SlowResponse conditions
 * 
 * @return true when the bicycle is braking
 * @return false otherwise
 */
bool COMM_SpeedStatusBraking(void);

bool COMM_BrakeLightEnabled(void);

#endif // APP_COMM_H_
