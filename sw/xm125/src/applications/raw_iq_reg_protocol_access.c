// Copyright (c) Acconeer AB, 2026
// All rights reserved

#include <stdbool.h>
#include <stdint.h>

#include "acc_integration.h"
#include "acc_reg_protocol.h"
#include "acc_version.h"
#include "i2c_raw_iq_service.h"
#include "raw_iq_reg_protocol.h"

static uint32_t get_protocol_error_flags(void)
{
	uint32_t error_flags;

	acc_integration_critical_section_enter();
	error_flags = acc_reg_protocol_get_error_flags();
	acc_integration_critical_section_exit();

	return error_flags;
}

void raw_iq_reg_read_application_id(uint32_t *value)
{
	*value = RAW_IQ_REG_APPLICATION_ID_ENUM_RAW_IQ_SERVICE;
}

void raw_iq_reg_read_version(uint32_t *value)
{
	*value = acc_version_get_hex();
}

void raw_iq_reg_read_protocol_status(uint32_t *value)
{
	uint32_t error_flags = get_protocol_error_flags();

	*value = error_flags;
}

void raw_iq_reg_read_app_status(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_status();
}

void raw_iq_reg_read_frame_counter(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_frame_counter();
}

void raw_iq_reg_read_frame_flags(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_frame_flags();
}

void raw_iq_reg_read_sweeps_per_frame(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_sweeps_per_frame();
}

void raw_iq_reg_read_points_per_sweep(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_points_per_sweep();
}

void raw_iq_reg_read_frame_data_length(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_frame_data_length();
}

void raw_iq_reg_read_readout_sweep_index(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_readout_sweep_index();
}

void raw_iq_reg_read_readout_point_index(uint32_t *value)
{
	*value = i2c_raw_iq_service_get_readout_point_index();
}

void raw_iq_reg_read_iq_point_real(uint32_t *value)
{
	*value = (uint32_t)(int32_t)i2c_raw_iq_service_get_point_real();
}

void raw_iq_reg_read_iq_point_imag(uint32_t *value)
{
	*value = (uint32_t)(int32_t)i2c_raw_iq_service_get_point_imag();
}

void raw_iq_reg_read_iq_point_packed(uint32_t *value)
{
	uint16_t real_u16 = (uint16_t)i2c_raw_iq_service_get_point_real();
	uint16_t imag_u16 = (uint16_t)i2c_raw_iq_service_get_point_imag();

	*value = ((uint32_t)real_u16 << 16) | imag_u16;
}

bool raw_iq_reg_write_readout_sweep_index(uint32_t value)
{
	return i2c_raw_iq_service_set_readout_sweep_index(value);
}

bool raw_iq_reg_write_readout_point_index(uint32_t value)
{
	return i2c_raw_iq_service_set_readout_point_index(value);
}

bool raw_iq_reg_write_command(uint32_t value)
{
	return i2c_raw_iq_service_command(value);
}