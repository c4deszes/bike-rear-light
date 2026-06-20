// Copyright (c) Acconeer AB, 2026
// All rights reserved

#ifndef APP_I2C_SERVICE_H_
#define APP_I2C_SERVICE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t car_detector_get_app_status(void);

bool car_detector_push_command(uint32_t command);

void car_detector_set_map_readout_chunk(uint16_t chunk_index);

bool car_detector_get_frame_info(uint8_t *data, size_t length);

bool car_detector_get_target_info(uint8_t *data, size_t length);

bool car_detector_get_map_chunk(uint8_t *data, size_t length);

#endif