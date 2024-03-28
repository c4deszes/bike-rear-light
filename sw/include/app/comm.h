#if !defined(APP_COMM_H_)
#define APP_COMM_H_

#include "common/ringbuffer.h"

DECLARE_RINGBUFFER_8(COMM_UsartBuffer);

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
 * @brief Passes the bytes received from the physical layer to the transport layer
 */
void COMM_UpdatePhy(void);

#endif // APP_COMM_H_
