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

static bool BRAKE_InternalBrakeActive;
static bool BRAKE_ExternalBrakeActive;

void BRAKE_Init(void) {
    BRAKE_IsSensorSetup = false;
    BRAKE_SignalState = brake_signal_status_na;

    BRAKE_Acceleration.x = 0;
    BRAKE_Acceleration.y = 0;
    BRAKE_Acceleration.z = 0;

    BRAKE_AccelerationCalib.x = 0;
    BRAKE_AccelerationCalib.y = 0;
    BRAKE_AccelerationCalib.z = 0;

    BRAKE_InternalBrakeActive = false;
    BRAKE_ExternalBrakeActive = false;

    // TODO: handle when calibration is not available
    CALIB_GetImuAccelCalib(&BRAKE_AccelerationCalib.x, &BRAKE_AccelerationCalib.y, &BRAKE_AccelerationCalib.z);
}

static void BRAKE_DetermineExternalBrakeActive(void) {
    BRAKE_ExternalBrakeActive = false;

    /* External brake signal is only used when SpeedStatus is up to date,
       controlled by FEATURE_COMM_SPEEDSTATUS_TIMEOUT */
    if (!COMM_SpeedStatusTimeout() && COMM_SpeedStatusBraking()) {
        BRAKE_ExternalBrakeActive = true;
    }
}

static void BRAKE_DetermineInternalBrakeActive(void) {
    BRAKE_InternalBrakeActive = false;
}

static bool BRAKE_CombineBrakeSignals(void) {
    bool combined_brake = false;
#if FEATURE_BRAKE_USE_EXTERNAL_SIGNAL == 1
    if (BRAKE_ExternalBrakeActive) {
        combined_brake = true;
    }
#endif
#if FEATURE_BRAKE_USE_INTERNAL_SIGNAL == 1
    if (BRAKE_InternalBrakeActive) {
        combined_brake = true;
    }
#endif

    /* Brake light control is disabled when requested, even under timeout conditions.
       it's also only enabled once the rear light setting has been received at least once
    */
    if (!COMM_BrakeLightEnabled()) {
        combined_brake = false;
    }

    return combined_brake;
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

/* Internal brake signal */

#if FEATURE_BRAKE_ENABLE_SENSOR == 1
    if (BRAKE_SignalState == brake_signal_status_ok) {
        bool result = ACCEL_ReadData(&BRAKE_Acceleration);

        // TODO: implement sensor error handling
        // TODO: implement brake detection logic based on acceleration data
        #if FEATURE_BRAKE_ENABLE_ALGORITHM
        BRAKE_DetermineInternalBrakeActive();
        #endif
    }
#endif

/* External brake signal */
    BRAKE_DetermineExternalBrakeActive();

/* Final brake signal determination */
    bool combined_brake = BRAKE_CombineBrakeSignals();

    BRIGHTNESS_SetBraking(combined_brake);
}

brake_signal_status_t BRAKE_GetInternalStatus(void) {
    return BRAKE_SignalState;
}

bool BRAKE_GetInternalBraking(void) {
    return BRAKE_InternalBrakeActive;
}

void BRAKE_GetAcceleration(int16_t* x, int16_t* y, int16_t* z) {
    // TODO: validate pointers
    *x = BRAKE_Acceleration.x;
    *y = BRAKE_Acceleration.y;
    *z = BRAKE_Acceleration.z;
}
