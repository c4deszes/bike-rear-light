// Copyright (c) Acconeer AB, 2026
// All rights reserved

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "i2c_raw_iq_service.h"
#include "raw_iq_reg_protocol.h"

#define SENSOR_ID         (1U)
#define SENSOR_TIMEOUT_MS (2000U)

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

static uint32_t readout_sweep_index = 0U;
static uint32_t readout_point_index = 0U;

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

// static bool app_status_test_bits(uint32_t mask)
// {
// 	acc_integration_critical_section_enter();
// 	bool test = (app_status & mask) == mask;
// 	acc_integration_critical_section_exit();
// 	return test;
// }

static uint32_t pull_command(void)
{
	acc_integration_critical_section_enter();
	uint32_t command = pending_command;
	pending_command  = 0U;
	acc_integration_critical_section_exit();
	return command;
}

static void set_default_config(acc_config_t *config)
{
	/*
	 * Replace these defaults with your own subsweep layout.
	 * The exposed frame is 2D: [sweeps_per_frame][points_per_sweep].
	 */
	acc_config_sweeps_per_frame_set(config, 16U);
	acc_config_num_subsweeps_set(config, 1U);

	acc_config_subsweep_start_point_set(config, 80, 0U);
	acc_config_subsweep_num_points_set(config, 160U, 0U);
	acc_config_subsweep_step_length_set(config, 1U, 0U);
	acc_config_subsweep_profile_set(config, ACC_CONFIG_PROFILE_3, 0U);
	acc_config_subsweep_hwaas_set(config, 16U, 0U);
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

	app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_SENSOR_CALIBRATE_OK_MASK);

	acc_hal_integration_sensor_disable(SENSOR_ID);
	acc_hal_integration_sensor_enable(SENSOR_ID);

	status = acc_sensor_prepare(resources.sensor, resources.config, &resources.cal_result, buffer, buffer_size);
	if (status)
	{
		app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_SENSOR_PREPARE_OK_MASK);
	}

	return status;
}

static bool apply_configuration(void)
{
	if (configured)
	{
		return true;
	}

	resources.config = acc_config_create();
	if (resources.config == NULL)
	{
		return false;
	}

	app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_CONFIG_CREATE_OK_MASK);

	set_default_config(resources.config);

	resources.processing = acc_processing_create(resources.config, &resources.proc_meta);
	if (resources.processing == NULL)
	{
		return false;
	}

	app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_PROCESSING_CREATE_OK_MASK);

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

	app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_BUFFER_OK_MASK);

	acc_hal_integration_sensor_supply_on(SENSOR_ID);
	acc_hal_integration_sensor_enable(SENSOR_ID);

	resources.sensor = acc_sensor_create(SENSOR_ID);
	if (resources.sensor == NULL)
	{
		return false;
	}

	app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_SENSOR_CREATE_OK_MASK);

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
	if (resources.proc_result.data_saturated)
	{
		frame_flags |= RAW_IQ_REG_FRAME_FLAGS_FIELD_DATA_SATURATED_MASK;
	}
	if (resources.proc_result.frame_delayed)
	{
		frame_flags |= RAW_IQ_REG_FRAME_FLAGS_FIELD_FRAME_DELAYED_MASK;
	}
	if (resources.proc_result.calibration_needed)
	{
		frame_flags |= RAW_IQ_REG_FRAME_FLAGS_FIELD_CALIBRATION_NEEDED_MASK;
	}

	memcpy(resources.frame_snapshot,
	       resources.proc_result.frame,
	       resources.proc_meta.frame_data_length * sizeof(acc_int16_complex_t));

	frame_flags |= RAW_IQ_REG_FRAME_FLAGS_FIELD_FRAME_READY_MASK;
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
		case RAW_IQ_REG_COMMAND_ENUM_APPLY_CONFIGURATION:
			if (!apply_configuration())
			{
				app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_ERROR_MASK);
			}
			break;

		case RAW_IQ_REG_COMMAND_ENUM_START:
			if (apply_configuration())
			{
				running = true;
				app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_RUNNING_MASK);
			}
			else
			{
				app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_ERROR_MASK);
			}
			break;

		case RAW_IQ_REG_COMMAND_ENUM_STOP:
			running = false;
			app_status_clr_bits(RAW_IQ_REG_APP_STATUS_FIELD_RUNNING_MASK);
			break;

		case RAW_IQ_REG_COMMAND_ENUM_GET_NEXT_FRAME:
			if (!capture_next_frame())
			{
				app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_ERROR_MASK);
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

static uint32_t get_linear_index(uint32_t sweep_idx, uint32_t point_idx)
{
	return sweep_idx * resources.proc_meta.sweep_data_length + point_idx;
}

bool i2c_raw_iq_service_command(uint32_t command)
{
	bool accepted = false;

	acc_integration_critical_section_enter();
	if (pending_command == 0U)
	{
		pending_command = command;
		app_status |= RAW_IQ_REG_APP_STATUS_FIELD_BUSY_MASK;
		i2c_application_system_set_ready_pin(false);
		accepted = true;
	}
	acc_integration_critical_section_exit();

	return accepted;
}

uint32_t i2c_raw_iq_service_get_status(void)
{
	acc_integration_critical_section_enter();
	uint32_t status = app_status;
	acc_integration_critical_section_exit();
	return status;
}

uint32_t i2c_raw_iq_service_get_frame_counter(void)
{
	acc_integration_critical_section_enter();
	uint32_t counter = frame_counter;
	acc_integration_critical_section_exit();
	return counter;
}

uint32_t i2c_raw_iq_service_get_frame_flags(void)
{
	acc_integration_critical_section_enter();
	uint32_t flags = frame_flags;
	acc_integration_critical_section_exit();
	return flags;
}

uint32_t i2c_raw_iq_service_get_sweeps_per_frame(void)
{
	if (!configured)
	{
		return 0U;
	}

	return (uint32_t)acc_config_sweeps_per_frame_get(resources.config);
}

uint32_t i2c_raw_iq_service_get_points_per_sweep(void)
{
	if (!configured)
	{
		return 0U;
	}

	return (uint32_t)resources.proc_meta.sweep_data_length;
}

uint32_t i2c_raw_iq_service_get_frame_data_length(void)
{
	if (!configured)
	{
		return 0U;
	}

	return (uint32_t)resources.proc_meta.frame_data_length;
}

uint32_t i2c_raw_iq_service_get_readout_sweep_index(void)
{
	acc_integration_critical_section_enter();
	uint32_t idx = readout_sweep_index;
	acc_integration_critical_section_exit();
	return idx;
}

uint32_t i2c_raw_iq_service_get_readout_point_index(void)
{
	acc_integration_critical_section_enter();
	uint32_t idx = readout_point_index;
	acc_integration_critical_section_exit();
	return idx;
}

bool i2c_raw_iq_service_set_readout_sweep_index(uint32_t sweep_idx)
{
	if (!configured)
	{
		return false;
	}

	if (sweep_idx >= acc_config_sweeps_per_frame_get(resources.config))
	{
		return false;
	}

	acc_integration_critical_section_enter();
	readout_sweep_index = sweep_idx;
	acc_integration_critical_section_exit();
	return true;
}

bool i2c_raw_iq_service_set_readout_point_index(uint32_t point_idx)
{
	if (!configured)
	{
		return false;
	}

	if (point_idx >= resources.proc_meta.sweep_data_length)
	{
		return false;
	}

	acc_integration_critical_section_enter();
	readout_point_index = point_idx;
	acc_integration_critical_section_exit();
	return true;
}

int16_t i2c_raw_iq_service_get_point_real(void)
{
	if (!configured || (frame_flags & RAW_IQ_REG_FRAME_FLAGS_FIELD_FRAME_READY_MASK) == 0U)
	{
		return 0;
	}

	acc_integration_critical_section_enter();
	uint32_t linear_idx = get_linear_index(readout_sweep_index, readout_point_index);
	int16_t  value      = resources.frame_snapshot[linear_idx].real;
	acc_integration_critical_section_exit();
	return value;
}

int16_t i2c_raw_iq_service_get_point_imag(void)
{
	if (!configured || (frame_flags & RAW_IQ_REG_FRAME_FLAGS_FIELD_FRAME_READY_MASK) == 0U)
	{
		return 0;
	}

	acc_integration_critical_section_enter();
	uint32_t linear_idx = get_linear_index(readout_sweep_index, readout_point_index);
	int16_t  value      = resources.frame_snapshot[linear_idx].imag;
	acc_integration_critical_section_exit();
	return value;
}

int acconeer_main(int argc, char *argv[]);

int acconeer_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	printf("I2C Raw IQ Service\n");
	printf("Acconeer software version %s\n", acc_version_get());

	const acc_hal_a121_t *hal = acc_hal_rss_integration_get_implementation();
	if (acc_rss_hal_register(hal))
	{
		app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_RSS_REGISTER_OK_MASK);
	}
	else
	{
		app_status_set_bits(RAW_IQ_REG_APP_STATUS_FIELD_ERROR_MASK);
	}

	i2c_application_system_init();
	raw_iq_reg_protocol_setup();

	while (true)
	{
		uint32_t command = pull_command();

		if (command == 0U)
		{
			module_low_power();
			continue;
		}

		if (command == RAW_IQ_REG_COMMAND_ENUM_RESET_MODULE)
		{
			i2c_application_system_reset();
			continue;
		}

		handle_command(command);

		app_status_clr_bits(RAW_IQ_REG_APP_STATUS_FIELD_BUSY_MASK);
		i2c_application_system_set_ready_pin(true);
	}
}