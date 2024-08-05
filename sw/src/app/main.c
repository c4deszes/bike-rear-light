#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE

#include "app/init.h"
#include "common/scheduler.h"

int main (void) {
    /* Initialize the application */
    APP_Initialize();

    /* Starts the scheduler */
    SCH_Enter();

    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}
