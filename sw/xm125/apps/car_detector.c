// Copyright (c) Acconeer AB, 2026
// All rights reserved

#include <stdbool.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "acc_algorithm.h"
#include "acc_config.h"
#include "acc_config_subsweep.h"
#include "acc_definitions_a121.h"
#include "acc_hal_definitions_a121.h"
#include "acc_hal_integration_a121.h"
#include "acc_integration.h"
#include "acc_processing.h"
#include "acc_rss_a121.h"
#include "acc_sensor.h"
#include "acc_version.h"

#include "i2c_application_system.h"

#include "car_detector_i2c_protocol.h"
#include "car_detector_i2c.h"
#include "car_detector.h"

#define SENSOR_ID         (1U)
#define SENSOR_TIMEOUT_MS (2000U)

#define MAP_SPEED_BINS     (32U)
#define MAP_SPEED_BINS_LOG2 (5U)
#define MAP_MAX_RANGE_BINS (64U)

#define RADAR_WAVELENGTH_M (0.005f)

typedef struct
{
    acc_config_t             *config;
    acc_processing_t         *processing;
    acc_sensor_t             *sensor;
    void                     *buffer;
    acc_int16_complex_t      *frame_snapshot;
    uint32_t                  buffer_size;
    acc_processing_metadata_t proc_meta;
    acc_processing_result_t   proc_result;
    acc_cal_result_t          cal_result;
} raw_iq_resources_t;

static raw_iq_resources_t resources = {0};

static uint32_t app_status    = 0U;
static uint32_t frame_counter = 0U;
static uint32_t frame_flags   = 0U;

static uint32_t pending_command = 0U;
static bool     running         = false;
static bool     configured      = false;

static uint16_t map_readout_chunk = 0U;
static uint16_t map_range_bins    = 0U;
static uint16_t map_speed_bins    = MAP_SPEED_BINS;
static uint16_t map_point_stride  = 1U;
static uint32_t map_data_bytes    = 0U;

static int32_t  map_range_start_mm  = 0;
static uint32_t map_range_step_mm   = 0U;
static int32_t  map_speed_start_mmps = 0;
static uint32_t map_speed_step_mmps = 0U;

static uint16_t range_speed_map[MAP_MAX_RANGE_BINS * MAP_SPEED_BINS] = {0U};
static uint32_t range_speed_map_raw[MAP_MAX_RANGE_BINS * MAP_SPEED_BINS] = {0U};

static uint8_t  target_valid      = 0U;
static int32_t  target_distance_mm = 0;
static int32_t  target_speed_mmps = 0;
static uint32_t target_power      = 0U;

static void app_status_set_bits(uint32_t mask)
{
    acc_integration_critical_section_enter();
    app_status |= mask;
    acc_integration_critical_section_exit();
}

static void app_status_clr_bits(uint32_t mask)
{
    acc_integration_critical_section_enter();
    app_status &= ~mask;
    acc_integration_critical_section_exit();
}

static uint32_t pull_command(void)
{
    acc_integration_critical_section_enter();
    uint32_t command = pending_command;
    pending_command  = 0U;
    acc_integration_critical_section_exit();
    return command;
}

static bool write_u16_be(uint8_t *data, size_t length, size_t offset, uint16_t value)
{
    if ((offset + 2U) > length)
    {
        return false;
    }

    data[offset + 0U] = (uint8_t)(value >> 8);
    data[offset + 1U] = (uint8_t)(value >> 0);

    return true;
}

static bool write_u32_be(uint8_t *data, size_t length, size_t offset, uint32_t value)
{
    if ((offset + 4U) > length)
    {
        return false;
    }

    data[offset + 0U] = (uint8_t)(value >> 24);
    data[offset + 1U] = (uint8_t)(value >> 16);
    data[offset + 2U] = (uint8_t)(value >> 8);
    data[offset + 3U] = (uint8_t)(value >> 0);

    return true;
}

static bool write_s32_be(uint8_t *data, size_t length, size_t offset, int32_t value)
{
    return write_u32_be(data, length, offset, (uint32_t)value);
}

static uint16_t get_map_chunk_count(void)
{
    if (map_data_bytes == 0U)
    {
        return 0U;
    }

    return (uint16_t)((map_data_bytes + CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN - 1U) /
                      CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN);
}

static void set_default_config(acc_config_t *config)
{
    /*
     * Replace these defaults with your own subsweep layout.
     * The exposed frame is 2D: [sweeps_per_frame][points_per_sweep].
     */
    acc_config_sweeps_per_frame_set(config, MAP_SPEED_BINS);
    acc_config_sweep_rate_set(config, 0.0f);
    acc_config_frame_rate_set(config, 0.0f);
    acc_config_inter_sweep_idle_state_set(config, ACC_CONFIG_IDLE_STATE_READY);
    acc_config_inter_frame_idle_state_set(config, ACC_CONFIG_IDLE_STATE_READY);
    acc_config_continuous_sweep_mode_set(config, false);

    acc_config_num_subsweeps_set(config, 1U);

    acc_config_subsweep_start_point_set(config, 16, 0U);
    acc_config_subsweep_num_points_set(config, 84U, 0U);
    acc_config_subsweep_step_length_set(config, 96U, 0U);
    acc_config_subsweep_profile_set(config, ACC_CONFIG_PROFILE_4, 0U);
    acc_config_subsweep_hwaas_set(config, 4U, 0U);
    acc_config_subsweep_prf_set(config, ACC_CONFIG_PRF_5_2_MHZ, 0U);
}

static bool update_range_speed_map(void)
{
    if (resources.frame_snapshot == NULL || resources.config == NULL)
    {
        return false;
    }

    const uint16_t points_per_sweep = resources.proc_meta.sweep_data_length;
    if (points_per_sweep == 0U)
    {
        return false;
    }

    const uint16_t sweeps_per_frame = (uint16_t)(resources.proc_meta.frame_data_length / points_per_sweep);
    if (sweeps_per_frame == 0U)
    {
        return false;
    }

    const uint16_t sweeps_used = (sweeps_per_frame > MAP_SPEED_BINS) ? MAP_SPEED_BINS : sweeps_per_frame;
    if (sweeps_used < 8U)
    {
        return false;
    }

    uint16_t point_stride = 1U;
    uint16_t range_bins   = points_per_sweep;
    if (range_bins > MAP_MAX_RANGE_BINS)
    {
        point_stride = (uint16_t)((points_per_sweep + MAP_MAX_RANGE_BINS - 1U) / MAP_MAX_RANGE_BINS);
        range_bins   = (uint16_t)(points_per_sweep / point_stride);
    }

    if (range_bins == 0U)
    {
        range_bins = 1U;
    }

    if (range_bins > MAP_MAX_RANGE_BINS)
    {
        range_bins = MAP_MAX_RANGE_BINS;
    }

    float window[MAP_SPEED_BINS] = {0.0f};
    acc_algorithm_hann(MAP_SPEED_BINS, window);

    uint32_t max_power = 0U;
    float configured_sweep_rate_hz = acc_config_sweep_rate_get(resources.config);
    float sweep_rate_hz = (configured_sweep_rate_hz > 0.0f) ? configured_sweep_rate_hz : resources.proc_meta.max_sweep_rate;
    if (sweep_rate_hz <= 0.0f)
    {
        sweep_rate_hz = 1.0f;
    }

    for (uint16_t range_bin = 0U; range_bin < range_bins; range_bin++)
    {
        const uint16_t point_index = (uint16_t)(range_bin * point_stride);

        float complex time_data[MAP_SPEED_BINS] = {0.0f};
        float complex spectrum[MAP_SPEED_BINS]  = {0.0f};
        float complex mean_value                = 0.0f;

        for (uint16_t sweep_idx = 0U; sweep_idx < sweeps_used; sweep_idx++)
        {
            const size_t raw_index = ((size_t)sweep_idx * points_per_sweep) + point_index;
            const float complex sample = (float)resources.frame_snapshot[raw_index].real +
                                         (float)resources.frame_snapshot[raw_index].imag * I;
            mean_value += sample;
        }

        mean_value /= (float)sweeps_used;

        for (uint16_t sweep_idx = 0U; sweep_idx < sweeps_used; sweep_idx++)
        {
            const size_t raw_index = ((size_t)sweep_idx * points_per_sweep) + point_index;
            const float complex sample = (float)resources.frame_snapshot[raw_index].real +
                                         (float)resources.frame_snapshot[raw_index].imag * I;
            time_data[sweep_idx] = (sample - mean_value) * window[sweep_idx];
        }

        acc_algorithm_fft(time_data, sweeps_used, MAP_SPEED_BINS_LOG2, spectrum);

        for (uint16_t speed_bin = 0U; speed_bin < MAP_SPEED_BINS; speed_bin++)
        {
            const uint16_t shifted_index = (uint16_t)((speed_bin + (MAP_SPEED_BINS / 2U)) % MAP_SPEED_BINS);
            const float re = crealf(spectrum[shifted_index]);
            const float im = cimagf(spectrum[shifted_index]);
            const float power = (re * re) + (im * im);
            const uint32_t power_u32 = (power > 0.0f) ? (uint32_t)power : 0U;

            const size_t map_index = ((size_t)range_bin * MAP_SPEED_BINS) + speed_bin;
            range_speed_map_raw[map_index] = power_u32;

            if (power_u32 > max_power)
            {
                max_power = power_u32;
            }
        }
    }

    if (max_power == 0U)
    {
        memset(range_speed_map, 0, sizeof(range_speed_map));
    }
    else
    {
        for (size_t i = 0U; i < ((size_t)range_bins * MAP_SPEED_BINS); i++)
        {
            range_speed_map[i] = (uint16_t)(((uint64_t)range_speed_map_raw[i] * UINT16_MAX) / max_power);
        }
    }

    int32_t start_point = acc_config_subsweep_start_point_get(resources.config, 0U);
    uint16_t step_length = acc_config_subsweep_step_length_get(resources.config, 0U);

    map_range_start_mm = (int32_t)(acc_processing_points_to_meter(start_point) * 1000.0f);
    map_range_step_mm = (uint32_t)(acc_processing_points_to_meter((int32_t)step_length * (int32_t)point_stride) * 1000.0f);

    const float speed_step_mps = (RADAR_WAVELENGTH_M * sweep_rate_hz) / (2.0f * (float)MAP_SPEED_BINS);
    const float speed_span_mps = speed_step_mps * (float)(MAP_SPEED_BINS / 2U);

    map_speed_start_mmps = (int32_t)(-speed_span_mps * 1000.0f);
    map_speed_step_mmps = (uint32_t)(speed_step_mps * 1000.0f);

    uint32_t best_power = 0U;
    uint16_t best_range_bin = 0U;
    int16_t best_speed_bin_centered = 0;

    for (uint16_t range_bin = 0U; range_bin < range_bins; range_bin++)
    {
        for (uint16_t speed_bin = 0U; speed_bin < MAP_SPEED_BINS; speed_bin++)
        {
            int16_t speed_bin_centered = (int16_t)speed_bin - (int16_t)(MAP_SPEED_BINS / 2U);
            if (speed_bin_centered >= -1 && speed_bin_centered <= 1)
            {
                continue;
            }

            const size_t map_index = ((size_t)range_bin * MAP_SPEED_BINS) + speed_bin;
            const uint32_t power = range_speed_map_raw[map_index];

            if (power > best_power)
            {
                best_power = power;
                best_range_bin = range_bin;
                best_speed_bin_centered = speed_bin_centered;
            }
        }
    }

    if (best_power > 0U)
    {
        target_valid = 1U;
        target_distance_mm = map_range_start_mm + ((int32_t)best_range_bin * (int32_t)map_range_step_mm);
        target_speed_mmps = best_speed_bin_centered * (int32_t)map_speed_step_mmps;
        target_power = best_power;
    }
    else
    {
        target_valid = 0U;
        target_distance_mm = 0;
        target_speed_mmps = 0;
        target_power = 0U;
    }

    map_range_bins = range_bins;
    map_speed_bins = MAP_SPEED_BINS;
    map_point_stride = point_stride;
    map_data_bytes = (uint32_t)range_bins * MAP_SPEED_BINS * sizeof(uint16_t);

    if (map_readout_chunk >= get_map_chunk_count())
    {
        map_readout_chunk = 0U;
    }

    return true;
}

static bool calibrate_and_prepare(void)
{
    bool   status       = false;
    bool   cal_complete = false;
    void  *buffer       = resources.buffer;
    uint32_t buffer_size = resources.buffer_size;

    if (resources.sensor == NULL || resources.config == NULL || buffer == NULL)
    {
        return false;
    }

    printf("Calibrating and preparing sensor\n");

    for (uint16_t retry = 0U; retry < 2U && !status; retry++)
    {
        acc_hal_integration_sensor_disable(SENSOR_ID);
        acc_hal_integration_sensor_enable(SENSOR_ID);

        do
        {
            status = acc_sensor_calibrate(resources.sensor, &cal_complete, &resources.cal_result, buffer, buffer_size);
            if (status && !cal_complete)
            {
                status = acc_hal_integration_wait_for_sensor_interrupt(SENSOR_ID, SENSOR_TIMEOUT_MS);
            }
        } while (status && !cal_complete);
    }

    if (!status)
    {
        return false;
    }

    app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_SENSOR_CALIBRATE_OK_MASK);
    printf("Sensor calibration complete\n");

    acc_hal_integration_sensor_disable(SENSOR_ID);
    acc_hal_integration_sensor_enable(SENSOR_ID);

    status = acc_sensor_prepare(resources.sensor, resources.config, &resources.cal_result, buffer, buffer_size);
    if (status)
    {
        app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_SENSOR_PREPARE_OK_MASK);
        printf("Sensor preparation complete\n");
    }

    return status;
}

static bool apply_configuration(void)
{
    if (configured)
    {
        return true;
    }

    printf("Applying configuration\n");

    resources.config = acc_config_create();
    if (resources.config == NULL)
    {
        return false;
    }

    app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_CONFIG_CREATE_OK_MASK);
    printf("Created configuration\n");

    set_default_config(resources.config);

    resources.processing = acc_processing_create(resources.config, &resources.proc_meta);
    if (resources.processing == NULL)
    {
        return false;
    }

    app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_PROCESSING_CREATE_OK_MASK);
    printf("Created processing instance.\n");

    if (!acc_rss_get_buffer_size(resources.config, &resources.buffer_size))
    {
        return false;
    }

    resources.buffer = acc_integration_mem_alloc(resources.buffer_size);
    if (resources.buffer == NULL)
    {
        return false;
    }

    resources.frame_snapshot = acc_integration_mem_alloc(resources.proc_meta.frame_data_length * sizeof(acc_int16_complex_t));
    if (resources.frame_snapshot == NULL)
    {
        return false;
    }

    app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_BUFFER_OK_MASK);
    printf("Allocated frame snapshot buffer.\n");

    acc_hal_integration_sensor_supply_on(SENSOR_ID);
    acc_hal_integration_sensor_enable(SENSOR_ID);

    resources.sensor = acc_sensor_create(SENSOR_ID);
    if (resources.sensor == NULL)
    {
        return false;
    }

    app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_SENSOR_CREATE_OK_MASK);

    if (!calibrate_and_prepare())
    {
        return false;
    }

    configured = true;
    return true;
}

static bool capture_next_frame(void)
{
    if (!configured || !running)
    {
        return false;
    }

    if (!acc_sensor_measure(resources.sensor))
    {
        return false;
    }

    if (!acc_hal_integration_wait_for_sensor_interrupt(SENSOR_ID, SENSOR_TIMEOUT_MS))
    {
        return false;
    }

    if (!acc_sensor_read(resources.sensor, resources.buffer, resources.buffer_size))
    {
        return false;
    }

    acc_processing_execute(resources.processing, resources.buffer, &resources.proc_result);

    acc_integration_critical_section_enter();
    frame_flags = 0U;
    // if (resources.proc_result.data_saturated)
    // {
    //     frame_flags |= APP_REG_FRAME_FLAGS_FIELD_DATA_SATURATED_MASK;
    // }
    // if (resources.proc_result.frame_delayed)
    // {
    //     frame_flags |= XM125_APP_STATUS_FIELD_FRAME_DELAYED_MASK;
    // }
    // if (resources.proc_result.calibration_needed)
    // {
    //     frame_flags |= XM125_APP_STATUS_FIELD_CALIBRATION_NEEDED_MASK;
    // }

    memcpy(resources.frame_snapshot,
           resources.proc_result.frame,
           resources.proc_meta.frame_data_length * sizeof(acc_int16_complex_t));

    if (!update_range_speed_map())
    {
        memset(range_speed_map, 0, sizeof(range_speed_map));
        map_range_bins = 0U;
        map_speed_bins = MAP_SPEED_BINS;
        map_point_stride = 1U;
        map_data_bytes = 0U;
        target_valid = 0U;
        target_distance_mm = 0;
        target_speed_mmps = 0;
        target_power = 0U;
    }

    // frame_flags |= XM125_APP_STATUS_FIELD_FRAME_READY_MASK;
    frame_counter++;
    acc_integration_critical_section_exit();

    if (resources.proc_result.calibration_needed)
    {
        if (!calibrate_and_prepare())
        {
            return false;
        }
    }

    return true;
}

static void handle_command(uint32_t command)
{
    switch (command)
    {
        case CAR_DETECTOR_COMMAND_APPLY_CONFIGURATION:
            if (!apply_configuration())
            {
                app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_ERROR_MASK);
            }
            break;

        case CAR_DETECTOR_COMMAND_START:
            if (apply_configuration())
            {
                running = true;
                app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_RUNNING_MASK);
            }
            else
            {
                app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_ERROR_MASK);
            }
            break;

        case CAR_DETECTOR_COMMAND_STOP:
            running = false;
            app_status_clr_bits(CAR_DETECTOR_STATUS_FIELD_RUNNING_MASK);
            break;

        case CAR_DETECTOR_COMMAND_GET_NEXT_FRAME:
            if (!capture_next_frame())
            {
                app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_ERROR_MASK);
            }
            break;

        default:
            break;
    }
}

static void module_low_power(void)
{
    if (i2c_application_system_test_wakeup_pin())
    {
        i2c_application_system_wait_for_interrupt();
    }
    else
    {
        i2c_application_system_set_ready_pin(false);
        i2c_application_enter_low_power_state();
    }

    if (i2c_application_system_test_wakeup_pin())
    {
        i2c_application_system_set_ready_pin(true);
    }
}

uint32_t car_detector_get_app_status(void)
{
    acc_integration_critical_section_enter();
    uint32_t status = app_status;
    acc_integration_critical_section_exit();
    return status;
}

bool car_detector_push_command(uint32_t command)
{
    bool accepted = false;

    acc_integration_critical_section_enter();
    if (pending_command == 0U)
    {
        pending_command = command;
        app_status |= CAR_DETECTOR_STATUS_FIELD_BUSY_MASK;
        i2c_application_system_set_ready_pin(false);
        accepted = true;
    }
    acc_integration_critical_section_exit();

    return accepted;
}

void car_detector_set_map_readout_chunk(uint16_t chunk_index)
{
    acc_integration_critical_section_enter();

    uint16_t chunk_count = get_map_chunk_count();
    if (chunk_count == 0U)
    {
        map_readout_chunk = 0U;
    }
    else if (chunk_index < chunk_count)
    {
        map_readout_chunk = chunk_index;
    }
    else
    {
        map_readout_chunk = (uint16_t)(chunk_count - 1U);
    }

    acc_integration_critical_section_exit();
}

bool car_detector_get_frame_info(uint8_t *data, size_t length)
{
    if (data == NULL || length != CAR_DETECTOR_READ_FRAME_INFO_LEN)
    {
        return false;
    }

    acc_integration_critical_section_enter();
    const uint16_t chunk_count = get_map_chunk_count();

    bool ok = true;
    ok = ok && write_u32_be(data, length, 0U, frame_counter);
    ok = ok && write_u16_be(data, length, 4U, map_range_bins);
    ok = ok && write_u16_be(data, length, 6U, map_speed_bins);
    ok = ok && write_u16_be(data, length, 8U, chunk_count);
    ok = ok && write_u16_be(data, length, 10U, map_point_stride);
    ok = ok && write_s32_be(data, length, 12U, map_range_start_mm);
    ok = ok && write_u32_be(data, length, 16U, map_range_step_mm);
    ok = ok && write_s32_be(data, length, 20U, map_speed_start_mmps);
    ok = ok && write_u32_be(data, length, 24U, map_speed_step_mmps);

    acc_integration_critical_section_exit();

    return ok;
}

bool car_detector_get_target_info(uint8_t *data, size_t length)
{
    if (data == NULL || length != CAR_DETECTOR_READ_TARGET_INFO_LEN)
    {
        return false;
    }

    acc_integration_critical_section_enter();

    bool ok = true;
    ok = ok && write_u32_be(data, length, 0U, frame_counter);
    ok = ok && write_u32_be(data, length, 4U, target_valid);
    ok = ok && write_s32_be(data, length, 8U, target_distance_mm);
    ok = ok && write_s32_be(data, length, 12U, target_speed_mmps);
    ok = ok && write_u32_be(data, length, 16U, target_power);

    acc_integration_critical_section_exit();

    return ok;
}

bool car_detector_get_map_chunk(uint8_t *data, size_t length)
{
    if (data == NULL || length != CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN)
    {
        return false;
    }

    memset(data, 0, length);

    acc_integration_critical_section_enter();

    const uint32_t base_byte = (uint32_t)map_readout_chunk * CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN;

    for (uint32_t i = 0U; i < CAR_DETECTOR_READ_RANGE_SPEED_MAP_CHUNK_LEN; i++)
    {
        const uint32_t global_byte = base_byte + i;
        if (global_byte >= map_data_bytes)
        {
            break;
        }

        const uint32_t sample_index = global_byte / 2U;
        const uint16_t sample_value = range_speed_map[sample_index];

        if ((global_byte % 2U) == 0U)
        {
            data[i] = (uint8_t)(sample_value >> 8);
        }
        else
        {
            data[i] = (uint8_t)(sample_value >> 0);
        }
    }

    acc_integration_critical_section_exit();

    return true;
}

int acconeer_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("Car detector service\n");
    printf("Acconeer software version %s\n", acc_version_get());

    const acc_hal_a121_t *hal = acc_hal_rss_integration_get_implementation();
    if (acc_rss_hal_register(hal))
    {
        app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_RSS_REGISTER_OK_MASK);
    }
    else
    {
        app_status_set_bits(CAR_DETECTOR_STATUS_FIELD_ERROR_MASK);
    }

    i2c_application_system_init();
    car_detector_reg_protocol_setup();

    while (true)
    {
        uint32_t command = pull_command();

        if (command == 0U)
        {
            module_low_power();
            continue;
        }

        // if (command == XM125_APP_COMMAND_ENUM_RESET_MODULE)
        // {
        // 	i2c_application_system_reset();
        // 	continue;
        // }

        handle_command(command);

        app_status_clr_bits(CAR_DETECTOR_STATUS_FIELD_BUSY_MASK);
        i2c_application_system_set_ready_pin(true);
    }
}