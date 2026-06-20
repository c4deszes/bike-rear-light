// Copyright (c) Acconeer AB, 2026
// All rights reserved

#ifndef CAR_DETECTOR_I2C_H_
#define CAR_DETECTOR_I2C_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CAR_DETECTOR_REG_REGISTER_COUNT 11U

bool car_detector_reg_read_version(uint8_t *data, size_t length);
bool car_detector_reg_read_protocol_status(uint8_t *data, size_t length);
bool car_detector_reg_read_app_status(uint8_t *data, size_t length);
bool car_detector_reg_read_frame_info(uint8_t *data, size_t length);
bool car_detector_reg_read_target_info(uint8_t *data, size_t length);
bool car_detector_reg_read_range_speed_map_chunk(uint8_t *data, size_t length);

bool car_detector_reg_apply_configuration(uint8_t *data, size_t length);
bool car_detector_reg_start(uint8_t *data, size_t length);
bool car_detector_reg_stop(uint8_t *data, size_t length);
bool car_detector_reg_get_next_frame(uint8_t *data, size_t length);
bool car_detector_reg_write_map_readout_control(uint8_t *data, size_t length);

void car_detector_reg_protocol_setup(void);

#endif