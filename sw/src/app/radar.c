#include "app/radar.h"

#include "hal/sercom_i2c.h"
#include "bsp/pinout.h"

#include "xm125/xm125_dev.h"
#include "car_detector_i2c_protocol.h"

xm125_dev_t xm125_dev = {
    .sercom_i2c_instance = XM125_I2C_INSTANCE,
    .i2c_address         = XM125_I2C_ADDRESS_FLOATING,
};

enum {
    radar_state_not_initialized = 0,
    radar_state_configuring,
    radar_state_configured,
    radar_state_running,
} radar_state = radar_state_not_initialized;

void RADAR_Init(void)
{

}

void RADAR_Update100ms(void)
{
    if (radar_state == radar_state_not_initialized) {
        xm125_dev_get_version(&xm125_dev, &xm125_dev.status.version);
        xm125_dev_get_protocol_status(&xm125_dev, &xm125_dev.status.protocol_status);
        xm125_dev_get_app_status(&xm125_dev, &xm125_dev.status.app_status);

        xm125_dev_apply_configuration(&xm125_dev);
        radar_state = radar_state_configuring;
    }
    else if (radar_state == radar_state_configuring) {
        xm125_dev_get_app_status(&xm125_dev, &xm125_dev.status.app_status);
        if (xm125_dev.status.app_status.config_create_ok &&
            xm125_dev.status.app_status.processing_create_ok &&
            xm125_dev.status.app_status.buffer_ok &&
            xm125_dev.status.app_status.sensor_create_ok) {
            radar_state = radar_state_configured;
        }
    }
    else if (radar_state == radar_state_configured) {
        xm125_dev_start(&xm125_dev);
        radar_state = radar_state_running;
    }
    else if (radar_state == radar_state_running) {
        //xm125_dev_get_next_frame(&xm125_dev);
    }

}
