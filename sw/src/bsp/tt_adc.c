#include "bsp/tt_adc.h"
#include "hal/adc.h"

#include <stdlib.h>

#include "bsp/tt_adc_cfg.h"

typedef struct {
    uint16_t result;
    bool result_ready;
    ttadc_result_status_t status;
} ttadc_channel_t;

typedef struct {
    uint8_t channel;
    adc_read_job_t job;
} ttadc_timeslot_t;

static uint8_t TTADC_TimeslotIndex;
static ttadc_channel_t TTADC_Channels[TTADC_NUM_CHANNELS]; // Assuming 3 channels for example
static ttadc_timeslot_t TTADC_Timeslots[TTADC_NUM_TIMESLOTS]; // Assuming 8 timeslots

void TTADC_AdcResultHandler(adc_read_job_t* job, adc_read_result_t* result) {
    // Find the corresponding channel for the completed job
    for (uint8_t i = 0; i < TTADC_NUM_TIMESLOTS; i++) {
        if (&(TTADC_Timeslots[i].job) == job) {
            TTADC_Channels[TTADC_Timeslots[i].channel].result = result->result;
            TTADC_Channels[TTADC_Timeslots[i].channel].result_ready = true;
            TTADC_Channels[TTADC_Timeslots[i].channel].status = ttadc_result_status_ok;
        }
    }
}

void TTADC_Init(void) {
    TTADC_TimeslotIndex = 0;

    for (uint8_t i = 0; i < TTADC_NUM_CHANNELS; i++) {
        TTADC_Channels[i].result = 0;
        TTADC_Channels[i].result_ready = false;
        TTADC_Channels[i].status = ttadc_result_status_na;
    }

    for (uint8_t i = 0; i < TTADC_NUM_TIMESLOTS; i++) {
        TTADC_Timeslots[i].channel = TTADC_CHANNEL_UNUSED;
        TTADC_Timeslots[i].job.muxpos = 0;
        TTADC_Timeslots[i].job.muxneg = 0;
        TTADC_Timeslots[i].job.callback = NULL;
    }

#ifdef TTADC_TIMESLOT_0_CHANNEL
    TTADC_Timeslots[0].channel = TTADC_TIMESLOT_0_CHANNEL;
    TTADC_Timeslots[0].job.muxpos = TTADC_TIMESLOT_0_MUXPOS;
    TTADC_Timeslots[0].job.muxneg = TTADC_TIMESLOT_0_MUXNEG;
    TTADC_Timeslots[0].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_1_CHANNEL
    TTADC_Timeslots[1].channel = TTADC_TIMESLOT_1_CHANNEL;
    TTADC_Timeslots[1].job.muxpos = TTADC_TIMESLOT_1_MUXPOS;
    TTADC_Timeslots[1].job.muxneg = TTADC_TIMESLOT_1_MUXNEG;
    TTADC_Timeslots[1].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_2_CHANNEL
    TTADC_Timeslots[2].channel = TTADC_TIMESLOT_2_CHANNEL;
    TTADC_Timeslots[2].job.muxpos = TTADC_TIMESLOT_2_MUXPOS;
    TTADC_Timeslots[2].job.muxneg = TTADC_TIMESLOT_2_MUXNEG;
    TTADC_Timeslots[2].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_3_CHANNEL
    TTADC_Timeslots[3].channel = TTADC_TIMESLOT_3_CHANNEL;
    TTADC_Timeslots[3].job.muxpos = TTADC_TIMESLOT_3_MUXPOS;
    TTADC_Timeslots[3].job.muxneg = TTADC_TIMESLOT_3_MUXNEG;
    TTADC_Timeslots[3].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_4_CHANNEL
    TTADC_Timeslots[4].channel = TTADC_TIMESLOT_4_CHANNEL;
    TTADC_Timeslots[4].job.muxpos = TTADC_TIMESLOT_4_MUXPOS;
    TTADC_Timeslots[4].job.muxneg = TTADC_TIMESLOT_4_MUXNEG;
    TTADC_Timeslots[4].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_5_CHANNEL
    TTADC_Timeslots[5].channel = TTADC_TIMESLOT_5_CHANNEL;
    TTADC_Timeslots[5].job.muxpos = TTADC_TIMESLOT_5_MUXPOS;
    TTADC_Timeslots[5].job.muxneg = TTADC_TIMESLOT_5_MUXNEG;
    TTADC_Timeslots[5].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_6_CHANNEL
    TTADC_Timeslots[6].channel = TTADC_TIMESLOT_6_CHANNEL;
    TTADC_Timeslots[6].job.muxpos = TTADC_TIMESLOT_6_MUXPOS;
    TTADC_Timeslots[6].job.muxneg = TTADC_TIMESLOT_6_MUXNEG;
    TTADC_Timeslots[6].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_7_CHANNEL
    TTADC_Timeslots[7].channel = TTADC_TIMESLOT_7_CHANNEL;
    TTADC_Timeslots[7].job.muxpos = TTADC_TIMESLOT_7_MUXPOS;
    TTADC_Timeslots[7].job.muxneg = TTADC_TIMESLOT_7_MUXNEG;
    TTADC_Timeslots[7].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_8_CHANNEL
    TTADC_Timeslots[8].channel = TTADC_TIMESLOT_8_CHANNEL;
    TTADC_Timeslots[8].job.muxpos = TTADC_TIMESLOT_8_MUXPOS;
    TTADC_Timeslots[8].job.muxneg = TTADC_TIMESLOT_8_MUXNEG;
    TTADC_Timeslots[8].job.callback = TTADC_AdcResultHandler;
#endif

#ifdef TTADC_TIMESLOT_9_CHANNEL
    TTADC_Timeslots[9].channel = TTADC_TIMESLOT_9_CHANNEL;
    TTADC_Timeslots[9].job.muxpos = TTADC_TIMESLOT_9_MUXPOS;
    TTADC_Timeslots[9].job.muxneg = TTADC_TIMESLOT_9_MUXNEG;
    TTADC_Timeslots[9].job.callback = TTADC_AdcResultHandler;
#endif
}

void TTADC_Trigger(void) {
    ttadc_timeslot_t* current_slot = &TTADC_Timeslots[TTADC_TimeslotIndex];
    if (current_slot->channel != TTADC_CHANNEL_UNUSED) {
        bool job_queued = ADC_ReadAsync(&(current_slot->job));

        if (!job_queued) {
            TTADC_Channels[current_slot->channel].status = ttadc_result_status_error;
        }
        else {
            TTADC_Channels[current_slot->channel].result_ready = false;
        }
    }

    TTADC_TimeslotIndex++;
    if (TTADC_TimeslotIndex >= TTADC_NUM_TIMESLOTS) {
        TTADC_TimeslotIndex = 0; // Wrap around to the first timeslot
    }
}

uint16_t TTADC_GetResult(uint8_t channel) {
    if (channel < TTADC_NUM_CHANNELS) {
        return TTADC_Channels[channel].result;
    }
    return 0; // Invalid channel, return 0 or some error code
}

bool TTADC_ResultReady(uint8_t channel) {
    if (channel < TTADC_NUM_CHANNELS) {
        return TTADC_Channels[channel].result_ready;
    }
    return false; // Invalid channel
}
