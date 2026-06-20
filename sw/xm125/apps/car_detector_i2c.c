
// Copyright (c) Acconeer AB, 2026
// All rights reserved

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "acc_integration.h"
#include "acc_reg_protocol.h"
#include "acc_version.h"

#include "car_detector_i2c_protocol.h"
#include "car_detector_i2c.h"
#include "car_detector.h"

const acc_reg_protocol_t app_reg_protocol[CAR_DETECTOR_REG_REGISTER_COUNT] = {
    {
        .address = CAR_DETECTOR_READ_VERSION_REG,
        .length  = CAR_DETECTOR_READ_VERSION_LEN,
        .read    = car_detector_reg_read_version,
        .write   = NULL,
    },
    {
        .address = CAR_DETECTOR_READ_PROTOCOL_STATUS_REG,
        .length  = CAR_DETECTOR_READ_PROTOCOL_STATUS_LEN,
        .read    = car_detector_reg_read_protocol_status,
        .write   = NULL,
    },
    {
        .address = CAR_DETECTOR_READ_APP_STATUS_REG,
        .length  = CAR_DETECTOR_READ_APP_STATUS_LEN,
        .read    = car_detector_reg_read_app_status,
        .write   = NULL,
    },
    {
        .address = CAR_DETECTOR_READ_FRAME_INFO_REG,
        .length  = CAR_DETECTOR_READ_FRAME_INFO_LEN,
        .read    = car_detector_reg_read_frame_info,
        .write   = NULL,
    },
    {
        .address = CAR_DETECTOR_READ_TARGET_INFO_REG,
        .length  = CAR_DETECTOR_READ_TARGET_INFO_LEN,
        .read    = car_detector_reg_read_target_info,
        .write   = NULL,
    },
    {
        .address = CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_REG,
        .length  = CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN,
        .read    = car_detector_reg_read_range_speed_map_chunk,
        .write   = NULL,
    },
    {
        .address = CAR_DETECTOR_COMMAND_APPLY_CONFIGURATION,
        .length  = 4,
        .read    = NULL,
        .write   = car_detector_reg_apply_configuration,
    },
    {
        .address = CAR_DETECTOR_COMMAND_START,
        .length  = 4,
        .read    = NULL,
        .write   = car_detector_reg_start,
    },
    {
        .address = CAR_DETECTOR_COMMAND_STOP,
        .length  = 4,
        .read    = NULL,
        .write   = car_detector_reg_stop,
    },
    {
        .address = CAR_DETECTOR_COMMAND_GET_NEXT_FRAME,
        .length  = 4,
        .read    = NULL,
        .write   = car_detector_reg_get_next_frame,
    },
    {
        .address = CAR_DETECTOR_WRITE_MAP_READOUT_CONTROL_REG,
        .length  = CAR_DETECTOR_WRITE_MAP_READOUT_CONTROL_LEN,
        .read    = NULL,
        .write   = car_detector_reg_write_map_readout_control,
    },
};


void car_detector_reg_protocol_setup(void)
{
    acc_reg_protocol_setup(app_reg_protocol, CAR_DETECTOR_REG_REGISTER_COUNT);
}

static uint32_t get_protocol_error_flags(void)
{
    uint32_t error_flags;

    acc_integration_critical_section_enter();
    error_flags = acc_reg_protocol_get_error_flags();
    acc_integration_critical_section_exit();

    return error_flags;
}


static bool write_u32_be(uint8_t *data, size_t length, uint32_t value)
{
    if (length != 4u)
    {
        return false;
    }

    data[0] = (uint8_t)(value >> 24);
    data[1] = (uint8_t)(value >> 16);
    data[2] = (uint8_t)(value >> 8);
    data[3] = (uint8_t)(value >> 0);

    return true;
}

bool car_detector_reg_read_version(uint8_t *data, size_t length)
{
    return write_u32_be(data, length, acc_version_get_hex());
}

bool car_detector_reg_read_protocol_status(uint8_t *data, size_t length)
{
    uint32_t error_flags = get_protocol_error_flags();

    return write_u32_be(data, length, error_flags);
}

bool car_detector_reg_read_app_status(uint8_t *data, size_t length)
{
    return write_u32_be(data, length, car_detector_get_app_status());
}

bool car_detector_reg_read_frame_info(uint8_t *data, size_t length)
{
    return car_detector_get_frame_info(data, length);
}

bool car_detector_reg_read_target_info(uint8_t *data, size_t length)
{
    return car_detector_get_target_info(data, length);
}

bool car_detector_reg_read_range_speed_map_chunk(uint8_t *data, size_t length)
{
    return car_detector_get_map_chunk(data, length);
}

bool car_detector_reg_apply_configuration(uint8_t *data, size_t length)
{
    return car_detector_push_command(CAR_DETECTOR_COMMAND_APPLY_CONFIGURATION);
}
bool car_detector_reg_start(uint8_t *data, size_t length)
{
    return car_detector_push_command(CAR_DETECTOR_COMMAND_START);
}
bool car_detector_reg_stop(uint8_t *data, size_t length)
{
    return car_detector_push_command(CAR_DETECTOR_COMMAND_STOP);
}
bool car_detector_reg_get_next_frame(uint8_t *data, size_t length)
{
    return car_detector_push_command(CAR_DETECTOR_COMMAND_GET_NEXT_FRAME);
}

bool car_detector_reg_write_map_readout_control(uint8_t *data, size_t length)
{
    if (length != CAR_DETECTOR_WRITE_MAP_READOUT_CONTROL_LEN)
    {
        return false;
    }

    uint16_t chunk_index = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
    car_detector_set_map_readout_chunk(chunk_index);

    return true;
}
