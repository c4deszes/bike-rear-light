#include "xm125/xm125_dev.h"
#include "car_detector_i2c_protocol.h"

#include <stdlib.h>
#include <string.h>

#include "hal/sercom_i2c.h"

static uint16_t be16_to_u16(const uint8_t *data)
{
    return ((uint16_t)data[0] << 8) | (uint16_t)data[1];
}

static uint32_t be32_to_u32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

static int32_t be32_to_s32(const uint8_t *data)
{
    return (int32_t)be32_to_u32(data);
}

bool xm125_dev_read_register(xm125_dev_t *dev, uint16_t reg_addr, uint8_t *reg_value, uint16_t len)
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

static bool xm125_dev_write_register(xm125_dev_t *dev, uint16_t reg_addr, const uint8_t *reg_value, uint16_t len)
{
    if (dev == NULL || reg_value == NULL) {
        return false;
    }

    if ((len + 2U) > 260U) {
        return false;
    }

    uint8_t tx_data[260];
    tx_data[0] = (uint8_t)(reg_addr >> 8);
    tx_data[1] = (uint8_t)(reg_addr & 0xFF);
    memcpy(&tx_data[2], reg_value, len);

    sercom_i2c_result_t result = SERCOM_I2C_Write(
        dev->sercom_i2c_instance,
        dev->i2c_address,
        tx_data,
        (uint16_t)(len + 2U)
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

    uint32_t status_value = be32_to_u32(reg_value);
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

    uint32_t status_value = be32_to_u32(reg_value);
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

bool xm125_dev_get_frame_info(xm125_dev_t *dev, xm125_frame_info_t *frame_info)
{
    if (dev == NULL || frame_info == NULL) {
        return false;
    }

    uint8_t reg_value[CAR_DETECTOR_READ_FRAME_INFO_LEN] = {0};
    if (!xm125_dev_read_register(dev, CAR_DETECTOR_READ_FRAME_INFO_REG, reg_value, CAR_DETECTOR_READ_FRAME_INFO_LEN)) {
        return false;
    }

    frame_info->frame_counter = be32_to_u32(&reg_value[0]);
    frame_info->range_bins = be16_to_u16(&reg_value[4]);
    frame_info->speed_bins = be16_to_u16(&reg_value[6]);
    frame_info->map_chunk_count = be16_to_u16(&reg_value[8]);
    frame_info->point_stride = be16_to_u16(&reg_value[10]);
    frame_info->range_start_mm = be32_to_s32(&reg_value[12]);
    frame_info->range_step_mm = be32_to_u32(&reg_value[16]);
    frame_info->speed_start_mmps = be32_to_s32(&reg_value[20]);
    frame_info->speed_step_mmps = be32_to_u32(&reg_value[24]);

    return true;
}

bool xm125_dev_get_target_info(xm125_dev_t *dev, xm125_target_info_t *target_info)
{
    if (dev == NULL || target_info == NULL) {
        return false;
    }

    uint8_t reg_value[CAR_DETECTOR_READ_TARGET_INFO_LEN] = {0};
    if (!xm125_dev_read_register(dev, CAR_DETECTOR_READ_TARGET_INFO_REG, reg_value, CAR_DETECTOR_READ_TARGET_INFO_LEN)) {
        return false;
    }

    target_info->frame_counter = be32_to_u32(&reg_value[0]);
    target_info->valid = be32_to_u32(&reg_value[4]);
    target_info->distance_mm = be32_to_s32(&reg_value[8]);
    target_info->speed_mmps = be32_to_s32(&reg_value[12]);
    target_info->confidence = be32_to_u32(&reg_value[16]);

    return true;
}

bool xm125_dev_set_map_chunk_index(xm125_dev_t *dev, uint16_t chunk_index)
{
    if (dev == NULL) {
        return false;
    }

    uint8_t payload[CAR_DETECTOR_WRITE_MAP_READOUT_CONTROL_LEN] = {
        (uint8_t)(chunk_index >> 8),
        (uint8_t)(chunk_index & 0xFF),
        0x00,
        0x00,
    };

    return xm125_dev_write_register(dev, CAR_DETECTOR_WRITE_MAP_READOUT_CONTROL_REG, payload, sizeof(payload));
}

bool xm125_dev_read_map_chunk(xm125_dev_t *dev, uint8_t *chunk_data, uint16_t len)
{
    if (dev == NULL || chunk_data == NULL) {
        return false;
    }

    if (len == 0U || len > CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN) {
        return false;
    }

    return xm125_dev_read_register(dev,
                                   CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_REG,
                                   chunk_data,
                                   len);
}
