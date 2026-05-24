#ifndef APP_WAKE_H
#define APP_WAKE_H

typedef enum {
    WAKE_REASON_OTHER,
    WAKE_REASON_POR,
    WAKE_REASON_CYCLIC,
    WAKE_REASON_COMMUNICATION,
    WAKE_REASON_IGNITION,
    WAKE_REASON_MOVEMENT
} wake_reason_t;

void WAKE_Init(void);

wake_reason_t WAKE_GetWakeReason(void);

#endif
