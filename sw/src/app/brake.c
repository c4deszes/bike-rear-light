#include "app/brake.h"
#include "app/brightness.h"

#include "hal/gpio.h"
#include "bsp/pinout.h"

#include "app/config.h"
#include "app/comm.h"

void BRAKE_Init(void) {

}

void BRAKE_Update10ms(void) {
    if (!COMM_SpeedStatusTimeout() && COMM_SpeedStatusBraking()) {
        BRIGHTNESS_SetBraking(true);
    }
    else {
        BRIGHTNESS_SetBraking(false);
    }
}