// Copyright (c) Acconeer AB, 2026
// All rights reserved

#ifndef I2C_RAW_IQ_SERVICE_H_
#define I2C_RAW_IQ_SERVICE_H_

#include <stdbool.h>
#include <stdint.h>

bool i2c_raw_iq_service_command(uint32_t command);

uint32_t i2c_raw_iq_service_get_status(void);
uint32_t i2c_raw_iq_service_get_frame_counter(void);
uint32_t i2c_raw_iq_service_get_frame_flags(void);

uint32_t i2c_raw_iq_service_get_sweeps_per_frame(void);
uint32_t i2c_raw_iq_service_get_points_per_sweep(void);
uint32_t i2c_raw_iq_service_get_frame_data_length(void);

uint32_t i2c_raw_iq_service_get_readout_sweep_index(void);
uint32_t i2c_raw_iq_service_get_readout_point_index(void);
bool i2c_raw_iq_service_set_readout_sweep_index(uint32_t sweep_idx);
bool i2c_raw_iq_service_set_readout_point_index(uint32_t point_idx);

int16_t i2c_raw_iq_service_get_point_real(void);
int16_t i2c_raw_iq_service_get_point_imag(void);

#endif