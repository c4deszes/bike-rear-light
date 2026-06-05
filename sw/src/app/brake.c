#include "app/brake.h"

// Board support package
#include "bsp/accel.h"

// Application components
#include "app/feature.h"
#include "app/config.h"
#include "app/calib.h"
#include "app/brightness.h"
#include "app/comm.h"

static bool BRAKE_IsSensorSetup;
static brake_signal_status_t BRAKE_SignalState;
static accel_data_t BRAKE_Acceleration;
static accel_data_t BRAKE_AccelerationCalib;

void BRAKE_Init(void) {
    BRAKE_IsSensorSetup = false;
    BRAKE_SignalState = brake_signal_status_na;

    BRAKE_Acceleration.x = 0;
    BRAKE_Acceleration.y = 0;
    BRAKE_Acceleration.z = 0;

    BRAKE_AccelerationCalib.x = 0;
    BRAKE_AccelerationCalib.y = 0;
    BRAKE_AccelerationCalib.z = 0;

    // TODO: handle when calibration is not available
    CALIB_GetImuAccelCalib(&BRAKE_AccelerationCalib.x, &BRAKE_AccelerationCalib.y, &BRAKE_AccelerationCalib.z);
}

void BRAKE_Update10ms(void) {
#if FEATURE_BRAKE_ENABLE_SENSOR == 1
    if (BRAKE_SignalState == brake_signal_status_na) {
        BRAKE_IsSensorSetup = ACCEL_SetupSensor();

        if (!BRAKE_IsSensorSetup) {
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

brake_signal_status_t BRAKE_GetInternalStatus(void) {
    return BRAKE_SignalState;
}

bool BRAKE_GetInternalBraking(void) {
    // TODO: implement
    return false;
}

void BRAKE_GetAcceleration(int16_t* x, int16_t* y, int16_t* z) {
    // TODO: validate pointers
    *x = BRAKE_Acceleration.x;
    *y = BRAKE_Acceleration.y;
    *z = BRAKE_Acceleration.z;
}
