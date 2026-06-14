// Copyright (c) Acconeer AB, 2026
// All rights reserved

#ifndef RAW_IQ_REG_PROTOCOL_H_
#define RAW_IQ_REG_PROTOCOL_H_

#include <stdbool.h>
#include <stdint.h>

#define RAW_IQ_REG_REGISTER_COUNT 15U

#define RAW_IQ_REG_VERSION_ADDRESS 0U
#define RAW_IQ_REG_PROTOCOL_STATUS_ADDRESS 1U
#define RAW_IQ_REG_APP_STATUS_ADDRESS 2U
#define RAW_IQ_REG_FRAME_COUNTER_ADDRESS 3U
#define RAW_IQ_REG_FRAME_FLAGS_ADDRESS 4U
#define RAW_IQ_REG_SWEEPS_PER_FRAME_ADDRESS 5U
#define RAW_IQ_REG_POINTS_PER_SWEEP_ADDRESS 6U
#define RAW_IQ_REG_FRAME_DATA_LENGTH_ADDRESS 7U
#define RAW_IQ_REG_READOUT_SWEEP_INDEX_ADDRESS 8U
#define RAW_IQ_REG_READOUT_POINT_INDEX_ADDRESS 9U
#define RAW_IQ_REG_IQ_POINT_REAL_ADDRESS 10U
#define RAW_IQ_REG_IQ_POINT_IMAG_ADDRESS 11U
#define RAW_IQ_REG_IQ_POINT_PACKED_ADDRESS 12U
#define RAW_IQ_REG_COMMAND_ADDRESS 256U
#define RAW_IQ_REG_APPLICATION_ID_ADDRESS 65535U

#define RAW_IQ_REG_APP_STATUS_FIELD_RSS_REGISTER_OK_MASK 0x00000001U
#define RAW_IQ_REG_APP_STATUS_FIELD_CONFIG_CREATE_OK_MASK 0x00000002U
#define RAW_IQ_REG_APP_STATUS_FIELD_PROCESSING_CREATE_OK_MASK 0x00000004U
#define RAW_IQ_REG_APP_STATUS_FIELD_BUFFER_OK_MASK 0x00000008U
#define RAW_IQ_REG_APP_STATUS_FIELD_SENSOR_CREATE_OK_MASK 0x00000010U
#define RAW_IQ_REG_APP_STATUS_FIELD_SENSOR_CALIBRATE_OK_MASK 0x00000020U
#define RAW_IQ_REG_APP_STATUS_FIELD_SENSOR_PREPARE_OK_MASK 0x00000040U
#define RAW_IQ_REG_APP_STATUS_FIELD_RUNNING_MASK 0x00000100U
#define RAW_IQ_REG_APP_STATUS_FIELD_ERROR_MASK 0x10000000U
#define RAW_IQ_REG_APP_STATUS_FIELD_BUSY_MASK 0x80000000U

#define RAW_IQ_REG_FRAME_FLAGS_FIELD_FRAME_READY_MASK 0x00000001U
#define RAW_IQ_REG_FRAME_FLAGS_FIELD_DATA_SATURATED_MASK 0x00000002U
#define RAW_IQ_REG_FRAME_FLAGS_FIELD_FRAME_DELAYED_MASK 0x00000004U
#define RAW_IQ_REG_FRAME_FLAGS_FIELD_CALIBRATION_NEEDED_MASK 0x00000008U

#define RAW_IQ_REG_COMMAND_ENUM_APPLY_CONFIGURATION 1U
#define RAW_IQ_REG_COMMAND_ENUM_START 2U
#define RAW_IQ_REG_COMMAND_ENUM_STOP 3U
#define RAW_IQ_REG_COMMAND_ENUM_GET_NEXT_FRAME 4U
#define RAW_IQ_REG_COMMAND_ENUM_RESET_MODULE 0x52535421U

#define RAW_IQ_REG_APPLICATION_ID_ENUM_RAW_IQ_SERVICE 100U

void raw_iq_reg_protocol_setup(void);

void raw_iq_reg_read_application_id(uint32_t *value);
void raw_iq_reg_read_version(uint32_t *value);
void raw_iq_reg_read_protocol_status(uint32_t *value);
void raw_iq_reg_read_app_status(uint32_t *value);
void raw_iq_reg_read_frame_counter(uint32_t *value);
void raw_iq_reg_read_frame_flags(uint32_t *value);
void raw_iq_reg_read_sweeps_per_frame(uint32_t *value);
void raw_iq_reg_read_points_per_sweep(uint32_t *value);
void raw_iq_reg_read_frame_data_length(uint32_t *value);
void raw_iq_reg_read_readout_sweep_index(uint32_t *value);
void raw_iq_reg_read_readout_point_index(uint32_t *value);
void raw_iq_reg_read_iq_point_real(uint32_t *value);
void raw_iq_reg_read_iq_point_imag(uint32_t *value);
void raw_iq_reg_read_iq_point_packed(uint32_t *value);

bool raw_iq_reg_write_readout_sweep_index(uint32_t value);
bool raw_iq_reg_write_readout_point_index(uint32_t value);
bool raw_iq_reg_write_command(uint32_t value);

#endif