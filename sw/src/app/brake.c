#include "app/brake.h"

// Board support package
#include "bsp/accel.h"

// Application components
#include "app/feature.h"
#include "app/config.h"
#include "app/brightness.h"
#include "app/comm.h"

// Concept: use MM feature set
// - configure High G detection
// - configure range 2/4G
// - Set high threshold to 0.25g, negative
//
// Potential problems:
// - Axis sensitivity depends on orientation, while in a climb, descent or a turn the
//   the acceleration components will be different than on a flat
// - Road bumps might cause inadvertent brake activation


static bool BRAKE_SensorSetup;
static brake_signal_status_t BRAKE_SignalState;
static accel_data_t BRAKE_Acceleration = { 0 };

void BRAKE_Init(void) {
    BRAKE_SensorSetup = false;
    BRAKE_SignalState = brake_signal_status_na;
}

int16_t BRAKE_GetAccelerationX(void) {
    return BRAKE_Acceleration.x;
}

int16_t BRAKE_GetAccelerationY(void) {
    return BRAKE_Acceleration.y;
}

int16_t BRAKE_GetAccelerationZ(void) {
    return BRAKE_Acceleration.z;
}

void BRAKE_Update10ms(void) {
#if FEATURE_BRAKE_ENABLE_SENSOR == 1
    if (BRAKE_SignalState == brake_signal_status_na) {
        BRAKE_SensorSetup = ACCEL_SetupSensor();

        if (!BRAKE_SensorSetup) {
            BRAKE_SignalState = brake_signal_status_perm_error;
        }
        else {
            BRAKE_SignalState = brake_signal_status_ok;
        }
    }
#endif

    bool internal_brake = false;

#if FEATURE_BRAKE_USE_INTERNAL_SIGNAL == 1
    if (BRAKE_SignalState == brake_signal_status_ok) {
        bool result = ACCEL_ReadData(&BRAKE_Acceleration);

        // TODO: implement brake detection logic based on acceleration data
    }
#else

#endif

    bool external_brake = false;

#if FEATURE_BRAKE_USE_EXTERNAL_SIGNAL == 1
    if (!COMM_SpeedStatusTimeout() && COMM_SpeedStatusBraking()) {
        BRIGHTNESS_SetBraking(true);
    }
    else {
        BRIGHTNESS_SetBraking(false);
    }
#endif
}

bool BRAKE_IsBraking(void) {
    // TODO: implement
    return false;
}

brake_signal_status_t BRAKE_GetBrakeSignalStatus(void) {
    return BRAKE_SignalState;
}
