#ifndef XM125_DEV_H
#define XM125_DEV_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t major;
    uint8_t minor;
    uint8_t patch;
} xm125_version_t;

typedef struct
{
    uint8_t protocol_state_error : 1;
    uint8_t packet_length_error : 1;
    uint8_t address_error : 1;
    uint8_t write_failed : 1;
    uint8_t write_to_read_only : 1;
    uint32_t reserved : 27;
} xm125_protocol_status_t;

typedef struct
{
    uint8_t rss_register_ok : 1;
    uint8_t config_create_ok : 1;
    uint8_t processing_create_ok : 1;
    uint8_t buffer_ok : 1;
    uint8_t sensor_create_ok : 1;
    uint8_t sensor_calibrate_ok : 1;
    uint8_t sensor_prepare_ok : 1;
    uint8_t running : 1;
    uint32_t reserved : 23;
} xm125_app_status_t;

typedef struct
{
    uint32_t frame_counter;
    uint16_t range_bins;
    uint16_t speed_bins;
    uint16_t map_chunk_count;
    uint16_t point_stride;
    int32_t range_start_mm;
    uint32_t range_step_mm;
    int32_t speed_start_mmps;
    uint32_t speed_step_mmps;
} xm125_frame_info_t;

typedef struct
{
    uint32_t frame_counter;
    uint32_t valid;
    int32_t distance_mm;
    int32_t speed_mmps;
    uint32_t confidence;
} xm125_target_info_t;

typedef struct
{
    xm125_version_t version;
    xm125_protocol_status_t protocol_status;
    xm125_app_status_t app_status;
} xm125_dev_status_t;

typedef struct
{
    uint8_t sercom_i2c_instance;
    uint8_t i2c_address;

    xm125_dev_status_t status;
} xm125_dev_t;

bool xm125_dev_get_version(xm125_dev_t *dev, xm125_version_t *version);

bool xm125_dev_get_protocol_status(xm125_dev_t *dev, xm125_protocol_status_t *protocol_status);

bool xm125_dev_get_app_status(xm125_dev_t *dev, xm125_app_status_t *app_status);

bool xm125_dev_apply_configuration(xm125_dev_t *dev);

bool xm125_dev_start(xm125_dev_t *dev);

bool xm125_dev_stop(xm125_dev_t *dev);

bool xm125_dev_get_next_frame(xm125_dev_t *dev);

bool xm125_dev_get_frame_info(xm125_dev_t *dev, xm125_frame_info_t *frame_info);

bool xm125_dev_get_target_info(xm125_dev_t *dev, xm125_target_info_t *target_info);

bool xm125_dev_set_map_chunk_index(xm125_dev_t *dev, uint16_t chunk_index);

bool xm125_dev_read_map_chunk(xm125_dev_t *dev, uint8_t *chunk_data, uint16_t len);

#endif