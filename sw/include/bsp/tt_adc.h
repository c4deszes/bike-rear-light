#ifndef BSP_TT_ADC_H
#define BSP_TT_ADC_H

#include <stdint.h>
#include <stdbool.h>

#include "hal/adc.h"

#define TTADC_CHANNEL_UNUSED 0xFF

typedef enum {
    ttadc_result_status_na,
    ttadc_result_status_ok,
    ttadc_result_status_error
} ttadc_result_status_t;

typedef struct {
    uint16_t result;
    bool result_ready;
    ttadc_result_status_t status;
} ttadc_channel_t;

typedef struct {
    uint8_t channel;
    adc_read_job_t job;
} ttadc_timeslot_t;

void TTADC_Init(void);

void TTADC_Trigger(void);

bool TTADC_ResultReady(uint8_t channel);

uint16_t TTADC_GetResult(uint8_t channel);

#endif