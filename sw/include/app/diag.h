#ifndef APP_DIAG_H_
#define APP_DIAG_H_

#include <stdint.h>
#include <stdbool.h>

void DIAG_Init(void);

/**
 * @brief Update function for critical diagnostics, called every 10ms
 * 
 * Contains:
 * - UDS Service call handling
 */
void DIAG_Update10ms(void);

/**
 * @brief Update function for non-critical diagnostics, called every 100ms
 * 
 * Contains:
 * - Operation status updates
 * - Power status updates
 */
void DIAG_Update100ms(void);

/**
 * @brief Returns true if boot entry was requested via LINE Flash protocol, the flag is cleared after reading
 * 
 * @return true When boot entry is requested
 * @return false Otherwise
 */
bool DIAG_BootRequest(void);

/**
 * @brief Returns true if shutdown was requested via LINE protocol, the flag is cleared after reading
 * 
 * @return true When shutdown is requested
 * @return false Otherwise
 */
bool DIAG_ShutdownRequest(void);

/**
 * @brief Returns true if idle mode was requested via LINE protocol, the flag is cleared after reading
 * 
 * @return true When idle mode is requested
 * @return false Otherwise
 */
bool DIAG_IdleRequest(void);

#endif // APP_DIAG_H_
