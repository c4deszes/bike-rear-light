#include "app/wake.h"

#include "hal/pm.h"

void WAKE_Init(void) {
    pm_rcause cause = PM_ResetCause();

    // TODO: implement logic to determine wakereason
}

wake_reason_t WAKE_GetWakeReason(void) {
    return WAKE_REASON_OTHER;
}
