// Copyright (c) Acconeer AB, 2026
// All rights reserved

#ifndef APP_I2C_SERVICE_H_
#define APP_I2C_SERVICE_H_

#include <stdbool.h>
#include <stdint.h>

uint32_t car_detector_get_app_status(void);

bool car_detector_push_command(uint32_t command);

#endif