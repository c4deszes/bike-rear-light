#include "app/brake.h"

#include "bma456mm.h"
#include "bsp/accel.h"

// Application components
#include "app/config.h"
#include "app/feature.h"
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


static struct bma4_accel_config accel_conf = { 0 };
static int8_t sensor_init_code;

static const float alpha = 0.1; // Low pass filter coefficient

static struct bma4_accel previous_data = { 0 };
static struct bma4_accel current_data = { 0 };
static struct bma4_accel gravity_vector = { 0 };
static brake_signal_status_t brake_signal_state;

void BRAKE_Init(void) {
    accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    accel_conf.range = BMA4_ACCEL_RANGE_4G;
    accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    accel_conf.perf_mode = BMA4_CIC_AVG_MODE;
}

static int8_t BRAKE_SetupSensor(void) {
    int8_t rslt;

    /* Sensor initialization */
    rslt = bma456mm_init(&ACCEL_BMA456_Device);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    /* Upload the configuration file to enable the features of the sensor. */
    rslt = bma456mm_write_config_file(&ACCEL_BMA456_Device);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    /* Set the accel configurations */
    rslt = bma4_set_accel_config(&accel_conf, &ACCEL_BMA456_Device);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    /* NOTE : Enable accel after set of configurations */
    rslt = bma4_set_accel_enable(BMA4_ENABLE, &ACCEL_BMA456_Device);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    return rslt;
}

int8_t BRAKE_GetAccelerometerErrorCode(void) {
    return sensor_init_code;
}

int16_t BRAKE_GetAccelerationX(void) {
    return current_data.x;
}

int16_t BRAKE_GetAccelerationY(void) {
    return current_data.y;
}

int16_t BRAKE_GetAccelerationZ(void) {
    return current_data.z;
}

void BRAKE_Update10ms(void) {
#if FEATURE_BRAKE_ENABLE_SENSOR == 1
    if (brake_signal_state == brake_signal_status_na) {
        sensor_init_code = BRAKE_SetupSensor();

        if (sensor_init_code != BMA4_OK) {
            brake_signal_state = brake_signal_status_perm_error;
        }
        else {
            brake_signal_state = brake_signal_status_ok;
        }
    }
#endif

    bool internal_brake = false;

#if FEATURE_BRAKE_USE_INTERNAL_SIGNAL == 1
    if (brake_signal_state == brake_signal_status_ok) {
        int8_t result = bma4_read_accel_xyz(&current_data, &ACCEL_BMA456_Device);

        // TODO: determine braking
        // TODO: if error result then put signal status to error
        if (result == BMA4_OK) {

            gravity_vector.x = (int16_t)(alpha * current_data.x + (1 - alpha) * previous_data.x);
            gravity_vector.y = (int16_t)(alpha * current_data.y + (1 - alpha) * previous_data.y);
            gravity_vector.z = (int16_t)(alpha * current_data.z + (1 - alpha) * previous_data.z);

            previous_data.x = current_data.x;
            previous_data.y = current_data.y;
            previous_data.z = current_data.z;

            struct bma4_accel difference = { 0 };
            difference.x = current_data.x - gravity_vector.x;
            difference.y = current_data.y - gravity_vector.y;
            difference.z = current_data.z - gravity_vector.z;

            if (difference.z < -1000) {
                internal_brake = true;
            }
            else {
                internal_brake = false;
            }

        } else {
            brake_signal_state = brake_signal_status_perm_error;
        }
    }
#else

#endif

    bool external_brake = false;

// TODO: in safety mode we should ignore the external signal (assume that comms are bad)
#if FEATURE_BRAKE_USE_EXTERNAL_SIGNAL == 1
    if (!COMM_SpeedStatusTimeout() && COMM_SpeedStatusBraking()) {
        BRIGHTNESS_SetBraking(true);
    }
    else {
        BRIGHTNESS_SetBraking(false);
    }
#endif
}
