#include "app/radar.h"

#include "hal/sercom_i2c.h"
#include "bsp/pinout.h"

#include "xm125/xm125_dev.h"
#include "car_detector_i2c_protocol.h"

#if 0

#define RADAR_MAP_CHUNK_BYTES CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN
#define RADAR_MAP_MAX_CHUNKS  32U
#define RADAR_MAP_MAX_BYTES   (RADAR_MAP_CHUNK_BYTES * RADAR_MAP_MAX_CHUNKS)

xm125_dev_t xm125_dev = {
    .sercom_i2c_instance = XM125_I2C_INSTANCE,
    .i2c_address         = XM125_I2C_ADDRESS_FLOATING,
};

xm125_frame_info_t radar_frame_info = {0};
xm125_target_info_t radar_target_info = {0};
uint8_t radar_range_speed_map[RADAR_MAP_MAX_BYTES] = {0};
uint16_t radar_map_chunks_expected = 0U;
uint16_t radar_map_chunks_read = 0U;
uint16_t radar_map_bytes_valid = 0U;

enum {
    radar_state_not_initialized = 0,
    radar_state_configuring,
    radar_state_configured,
    radar_state_running,
    radar_state_error
} radar_state = radar_state_not_initialized;

#endif

void RADAR_Init(void)
{

}

uint8_t RADAR_SampleCnt = 0;

void RADAR_Update100ms(void)
{
#if 0
    if (radar_state == radar_state_not_initialized) {
        bool version_ok = xm125_dev_get_version(&xm125_dev, &xm125_dev.status.version);

        if (!version_ok) {
            radar_state = radar_state_error;
        }
        else {
            xm125_dev_get_protocol_status(&xm125_dev, &xm125_dev.status.protocol_status);
            xm125_dev_get_app_status(&xm125_dev, &xm125_dev.status.app_status);

            xm125_dev_apply_configuration(&xm125_dev);
            radar_state = radar_state_configuring;
        }
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

        if (RADAR_SampleCnt > 10) {
            xm125_dev_get_next_frame(&xm125_dev);
            xm125_dev_get_frame_info(&xm125_dev, &radar_frame_info);
            xm125_dev_get_target_info(&xm125_dev, &radar_target_info);

            radar_map_chunks_expected = radar_frame_info.map_chunk_count;
            if (radar_map_chunks_expected > RADAR_MAP_MAX_CHUNKS) {
                radar_map_chunks_expected = RADAR_MAP_MAX_CHUNKS;
            }

            radar_map_chunks_read = 0U;
            radar_map_bytes_valid = 0U;

            if (radar_map_chunks_expected == 0U) {
                radar_map_bytes_valid = 0U;
            }

            RADAR_SampleCnt = 0;
        }

        if (radar_map_chunks_read < radar_map_chunks_expected) {
            uint16_t chunk_index = radar_map_chunks_read;
            uint16_t map_offset = chunk_index * RADAR_MAP_CHUNK_BYTES;

            if ((map_offset + RADAR_MAP_CHUNK_BYTES) <= RADAR_MAP_MAX_BYTES) {
                bool set_ok = xm125_dev_set_map_chunk_index(&xm125_dev, chunk_index);
                bool read_ok = false;

                if (set_ok) {
                    read_ok = xm125_dev_read_map_chunk(&xm125_dev,
                                                       &radar_range_speed_map[map_offset],
                                                       RADAR_MAP_CHUNK_BYTES);
                }

                if (read_ok) {
                    radar_map_chunks_read++;

                    if (radar_map_chunks_read >= radar_map_chunks_expected) {
                        uint32_t expected_bytes = (uint32_t)radar_frame_info.range_bins *
                                                  (uint32_t)radar_frame_info.speed_bins * 2U;

                        if (expected_bytes > RADAR_MAP_MAX_BYTES) {
                            expected_bytes = RADAR_MAP_MAX_BYTES;
                        }

                        radar_map_bytes_valid = (uint16_t)expected_bytes;
                    }
                }
            }
        }

        RADAR_SampleCnt++;
    }
#endif
}
