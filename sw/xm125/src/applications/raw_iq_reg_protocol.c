// Copyright (c) Acconeer AB, 2026
// All rights reserved

#include <stddef.h>

#include "acc_reg_protocol.h"
#include "raw_iq_reg_protocol.h"

const acc_reg_protocol_t raw_iq_reg_protocol[RAW_IQ_REG_REGISTER_COUNT] = {
    {
        RAW_IQ_REG_VERSION_ADDRESS,
        raw_iq_reg_read_version,
        NULL,
    },
    {
        RAW_IQ_REG_PROTOCOL_STATUS_ADDRESS,
        raw_iq_reg_read_protocol_status,
        NULL,
    },
    {
        RAW_IQ_REG_APP_STATUS_ADDRESS,
        raw_iq_reg_read_app_status,
        NULL,
    },
    {
        RAW_IQ_REG_FRAME_COUNTER_ADDRESS,
        raw_iq_reg_read_frame_counter,
        NULL,
    },
    {
        RAW_IQ_REG_FRAME_FLAGS_ADDRESS,
        raw_iq_reg_read_frame_flags,
        NULL,
    },
    {
        RAW_IQ_REG_SWEEPS_PER_FRAME_ADDRESS,
        raw_iq_reg_read_sweeps_per_frame,
        NULL,
    },
    {
        RAW_IQ_REG_POINTS_PER_SWEEP_ADDRESS,
        raw_iq_reg_read_points_per_sweep,
        NULL,
    },
    {
        RAW_IQ_REG_FRAME_DATA_LENGTH_ADDRESS,
        raw_iq_reg_read_frame_data_length,
        NULL,
    },
    {
        RAW_IQ_REG_READOUT_SWEEP_INDEX_ADDRESS,
        raw_iq_reg_read_readout_sweep_index,
        raw_iq_reg_write_readout_sweep_index,
    },
    {
        RAW_IQ_REG_READOUT_POINT_INDEX_ADDRESS,
        raw_iq_reg_read_readout_point_index,
        raw_iq_reg_write_readout_point_index,
    },
    {
        RAW_IQ_REG_IQ_POINT_REAL_ADDRESS,
        raw_iq_reg_read_iq_point_real,
        NULL,
    },
    {
        RAW_IQ_REG_IQ_POINT_IMAG_ADDRESS,
        raw_iq_reg_read_iq_point_imag,
        NULL,
    },
    {
        RAW_IQ_REG_IQ_POINT_PACKED_ADDRESS,
        raw_iq_reg_read_iq_point_packed,
        NULL,
    },
    {
        RAW_IQ_REG_COMMAND_ADDRESS,
        NULL,
        raw_iq_reg_write_command,
    },
    {
        RAW_IQ_REG_APPLICATION_ID_ADDRESS,
        raw_iq_reg_read_application_id,
        NULL,
    },
};


void raw_iq_reg_protocol_setup(void)
{
    acc_reg_protocol_setup(raw_iq_reg_protocol, RAW_IQ_REG_REGISTER_COUNT);
}