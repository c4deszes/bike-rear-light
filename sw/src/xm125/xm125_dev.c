#include "xm125/xm125_dev.h"
#include "car_detector_i2c_protocol.h"

#include <stdlib.h>

#include "hal/sercom_i2c.h"

bool xm125_dev_read_register(xm125_dev_t *dev, uint16_t reg_addr, uint8_t *reg_value, uint8_t len)
{
    if (dev == NULL || reg_value == NULL) {
        return false;
    }

    uint8_t reg_addr_bytes[2] = { (uint8_t)(reg_addr >> 8), (uint8_t)(reg_addr & 0xFF) };

    sercom_i2c_result_t result = SERCOM_I2C_WriteRead(
        dev->sercom_i2c_instance,
        dev->i2c_address,
        reg_addr_bytes,
        sizeof(reg_addr_bytes),
        (uint8_t*)reg_value,
        len
    );

    return result == sercom_i2c_ok;
}

bool xm125_dev_run_command(xm125_dev_t *dev, uint16_t command)
{
    if (dev == NULL) {
        return false;
    }

    uint8_t command_bytes[6] = { (uint8_t)(command >> 8), (uint8_t)(command & 0xFF), 0x00, 0x00, 0x00, 0x00 };

    sercom_i2c_result_t result = SERCOM_I2C_Write(
        dev->sercom_i2c_instance,
        dev->i2c_address,
        command_bytes,
        sizeof(command_bytes)
    );

    return result == sercom_i2c_ok;
}

bool xm125_dev_get_version(xm125_dev_t *dev, xm125_version_t *version)
{
    if (dev == NULL || version == NULL) {
        return false;
    }

    uint8_t reg_value[4] = {0};
    bool result = xm125_dev_read_register(dev, CAR_DETECTOR_READ_VERSION_REG, reg_value, sizeof(xm125_version_t));

    if (!result) {
        return false;
    }

    version->major = (reg_value[0] << 8) | reg_value[1];
    version->minor = reg_value[2];
    version->patch = reg_value[3];

    dev->status.version = *version;

    return true;
}

bool xm125_dev_get_protocol_status(xm125_dev_t *dev, xm125_protocol_status_t *protocol_status)
{
    if (dev == NULL || protocol_status == NULL) {
        return false;
    }

    uint8_t reg_value[4] = {0};
    bool result = xm125_dev_read_register(dev, CAR_DETECTOR_READ_PROTOCOL_STATUS_REG, reg_value, sizeof(xm125_protocol_status_t));

    if (!result) {
        return false;
    }

    uint32_t status_value = (reg_value[0] << 24) | (reg_value[1] << 16) | (reg_value[2] << 8) | reg_value[3];
    *protocol_status = *(xm125_protocol_status_t*)&status_value;

    dev->status.protocol_status = *protocol_status;

    return true;
}

bool xm125_dev_get_app_status(xm125_dev_t *dev, xm125_app_status_t *app_status)
{
    if (dev == NULL || app_status == NULL) {
        return false;
    }

    uint8_t reg_value[4] = {0};
    bool result = xm125_dev_read_register(dev, CAR_DETECTOR_READ_APP_STATUS_REG, reg_value, sizeof(xm125_app_status_t));

    if (!result) {
        return false;
    }

    uint32_t status_value = (reg_value[0] << 24) | (reg_value[1] << 16) | (reg_value[2] << 8) | reg_value[3];
    *app_status = *(xm125_app_status_t*)&status_value;

    dev->status.app_status = *app_status;

    return true;
}

bool xm125_dev_apply_configuration(xm125_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    return xm125_dev_run_command(dev, CAR_DETECTOR_COMMAND_APPLY_CONFIGURATION);
}

bool xm125_dev_start(xm125_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    return xm125_dev_run_command(dev, CAR_DETECTOR_COMMAND_START);
}

bool xm125_dev_stop(xm125_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    return xm125_dev_run_command(dev, CAR_DETECTOR_COMMAND_STOP);
}

bool xm125_dev_get_next_frame(xm125_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    return xm125_dev_run_command(dev, CAR_DETECTOR_COMMAND_GET_NEXT_FRAME);
}
