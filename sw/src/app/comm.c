#include "app/comm.h"
#include "bsp/usart.h"

#include "line_protocol.h"
#include "line_api.h"
#include "flash_line_api.h"
#include "flash_line_diag.h"
#include "app/feature.h"
#include "uds_gen.h"
#include "hal/dsu.h"
#include "app/brake.h"
#include "bsp/light_control.h"

#include "app/config.h"
#include "metainfo.h"

#include "common/swtimer.h"
#include "common/ringbuffer.h"

RINGBUFFER_8(COMM_UsartBufferTx, 128);
RINGBUFFER_8(COMM_UsartBufferRx, 128);

// TODO: data should be dynamic based on actual current figures 
static LINE_Diag_PowerStatus_t power_status = {
    .U_measured = 120,
    .I_operating = 100u,
    .I_sleep = LINE_DIAG_POWER_STATUS_SLEEP_CURRENT(100)
};

static LINE_Diag_SoftwareVersion_t sw_version = {
    .major = APP_SW_MAJOR,
    .minor = APP_SW_MINOR,
    .patch = APP_SW_PATCH
};

static swtimer_t* comm_lightrequest_timer;
static swtimer_t* comm_speedstatus_timer;

static bool comm_bootrequest = false;
static bool comm_shutdown_request = false;
static bool comm_idle_request = false;

void LINE_Diag_BicycleNetwork_RearLight_OnWakeup(void) {

}
void LINE_Diag_BicycleNetwork_RearLight_OnIdle(void) {
    comm_idle_request = true;
}
void LINE_Diag_BicycleNetwork_RearLight_OnShutdown(void) {
    comm_shutdown_request = true;
}
void LINE_Diag_BicycleNetwork_RearLight_OnConditionalChangeAddress(uint8_t old_address, uint8_t new_address) {

}

// TODO: op status should be dynamic based on device state
uint8_t LINE_Diag_BicycleNetwork_RearLight_GetOperationStatus(void) {
    return LINE_DIAG_OP_STATUS_OK;
}
LINE_Diag_PowerStatus_t* LINE_Diag_BicycleNetwork_RearLight_GetPowerStatus(void) {
    return &power_status;
}
uint32_t LINE_Diag_BicycleNetwork_RearLight_GetSerialNumber(void) {
    return DSU_GetSerialNumber32();
}
LINE_Diag_SoftwareVersion_t* LINE_Diag_BicycleNetwork_RearLight_GetSoftwareVersion(void) {
    return &sw_version;
}

void COMM_Initialize(void) {
    USART_Initialize(LINE_NETWORK_BicycleNetwork_BAUDRATE, &COMM_UsartBufferTx, &COMM_UsartBufferRx);
    USART_Enable();

    LINE_App_Init();
    UDS_Init();

    // TODO: change channel number
    FLASH_LINE_Init(0, FLASH_LINE_APPLICATION_MODE);

    comm_lightrequest_timer = SWTIMER_Create();
    comm_speedstatus_timer = SWTIMER_Create();
}

void COMM_UpdatePhy(void) {
    uint8_t length = USART_Available();
    while (length > 0) {
        uint8_t data = USART_Read();
        LINE_Transport_Receive(LINE_CHANNEL_BicycleNetwork, data);
        length--;
    }

    LINE_Transport_Update(LINE_CHANNEL_BicycleNetwork, 1);

    // TODO: also timeout if setpoint is invalid for a long time
    if (LINE_Request_LightSynchronization_flag() || LINE_Request_RearLightSetting_flag()) {
        SWTIMER_Setup(comm_lightrequest_timer, FEATURE_COMM_LIGHTREQUEST_TIMEOUT);
    }

    if (LINE_Request_SpeedStatus_flag()) {
        SWTIMER_Setup(comm_speedstatus_timer, FEATURE_COMM_SPEEDSTATUS_TIMEOUT);
    }
}

void LINE_Transport_WriteResponse(uint8_t channel, uint8_t size, uint8_t* payload, uint8_t checksum) {
    uint8_t fix = 69;
    USART_WriteData(&size, sizeof(uint8_t));
    // TODO: fix for skipped 3rd byte
    USART_WriteData(payload, 1);
    USART_WriteData(&fix, 1);
    USART_WriteData(payload+1, size-1);
    USART_WriteData(&checksum, sizeof(uint8_t));
    USART_FlushOutput();
}

fl_BootEntryResponse_t FLASH_BL_EnterBoot(void) {
    fl_BootEntryResponse_t response;
    
    // TODO: when do we reject boot entry requests?
    comm_bootrequest = true;

    response.entry_status = FLASH_LINE_BOOT_ENTRY_SUCCESS;
    response.serial_number = LINE_Diag_BicycleNetwork_RearLight_GetSerialNumber();

    return response;
}

bool COMM_BootRequest(void) {
    // TODO: clear flag before returning
    return comm_bootrequest;
}

bool COMM_ShutdownRequest(void) {
    // TODO: clear flag before returning
    return comm_shutdown_request;
}

bool COMM_IdleRequest(void) {
    // TODO: clear flag before returning
    return comm_idle_request;
}

uint16_t COMM_GetTargetBrightness(void) {
    return LINE_Request_LightSynchronization_data.fields.TargetBrightness * 10U;
}

brightness_mode_t COMM_LightMode(void) {
    if (LINE_Request_LightSynchronization_data.fields.LightMode == LINE_ENCODER_LightModeEncoder_Adaptive) {
        return brightness_mode_adaptive;
    }
    else if (LINE_Request_LightSynchronization_data.fields.LightMode == LINE_ENCODER_LightModeEncoder_Standard) {
        return brightness_mode_standard;
    }
    else if (LINE_Request_LightSynchronization_data.fields.LightMode == LINE_ENCODER_LightModeEncoder_Emergency) {
        return brightness_mode_emergency;
    }
    else if (LINE_Request_LightSynchronization_data.fields.LightMode == LINE_ENCODER_LightModeEncoder_Off) {
        return brightness_mode_off;
    }
    return brightness_mode_safety;
}

strobe_source_t COMM_LightBehavior(void) {
    if (LINE_Request_RearLightSetting_data.fields.Behavior == LINE_ENCODER_LightBehaviorEncoder_Default) {
        return CONFIG_Props.Strobe_ModeDefault;
    }
    else if (LINE_Request_RearLightSetting_data.fields.Behavior == LINE_ENCODER_LightBehaviorEncoder_Blink) {
        return CONFIG_Props.Strobe_ModePrimary;
    }
    return strobe_source_disabled;
}

bool COMM_LightRequestTimeout(void) {
    return SWTIMER_Elapsed(comm_lightrequest_timer);
}

bool COMM_SpeedStatusTimeout(void) {
    return SWTIMER_Elapsed(comm_speedstatus_timer);
}

bool COMM_SpeedStatusBraking(void) {
    if (   LINE_Request_SpeedStatus_data.fields.SpeedState == LINE_ENCODER_SpeedStateEncoder_Ok
        && LINE_Request_SpeedStatus_data.fields.BrakeState == LINE_ENCODER_BrakeStateEncoder_Braking) {

        return true;
    }
    return false;
}

static uint8_t COMM_EncodeLightStatus(lightcontrol_feature_state_t state) {
    if (state == lightcontrol_feature_state_ok) {
        return LINE_ENCODER_LightStatusEncoder_Ok;
    }
    else if(state == lightcontrol_feature_state_partial_error) {
        return LINE_ENCODER_LightStatusEncoder_PartialError;
    }
    else if(state == lightcontrol_feature_state_error) {
        return LINE_ENCODER_LightStatusEncoder_Error;
    }
    else {
        return LINE_ENCODER_LightStatusEncoder_Error;
    }
}

void COMM_UpdateSignals(void) {
    // TODO: fix brake light status flag
    /* Since the Brake light is rarely on we report ok status in most cases */
    LINE_Request_RearLightStatus_data.fields.BrakeLightStatus = LINE_ENCODER_LightStatusEncoder_Ok;
    
    /* Tail light state equals the diagnostic state if there were errors, otherwise it's ok when off, and off when brightness is 0 */
    lightcontrol_feature_state_t tail_state = LIGHTCONTROL_GetDiagnosticState(lightcontrol_feature_tail_segment);
    if (tail_state != lightcontrol_feature_state_ok) {
        LINE_Request_RearLightStatus_data.fields.TailLightStatus = COMM_EncodeLightStatus(tail_state);
    } else if (LIGHTCONTROL_GetBrightness(lightcontrol_feature_tail_segment) > LIGHTCONTROL_BRIGHTNESS_MIN) {
        LINE_Request_RearLightStatus_data.fields.TailLightStatus = LINE_ENCODER_LightStatusEncoder_Ok;
    }
    else {
        LINE_Request_RearLightStatus_data.fields.TailLightStatus = LINE_ENCODER_LightStatusEncoder_Off;
    }

    /* Turn Signal light is not present in Gen1.0 */
    LINE_Request_RearLightStatus_data.fields.TurnSignalLightStatus = LINE_ENCODER_LightStatusEncoder_Off;

    // TODO: measure MCU temp. and return accordingly
    LINE_Request_RearLightStatus_data.fields.ThermalStatus = LINE_ENCODER_ThermalStatusEncoder_NotMeasured;
}

void COMM_UpdateDebugSignals(void) {
    // TODO: use brightness from driver
    LINE_Request_RearLightBrightnessDebug_data.fields.Brightness = 0;

    // TODO: use actual temperature measurement
    LINE_Request_RearLightTemperatureDebug_data.fields.EcuTemperature = LINE_ENCODER_TemperatureEncoder_Encode(25);
    LINE_Request_RearLightTemperatureDebug_data.fields.DriveTemperature = LINE_ENCODER_TemperatureEncoder_Encode(25);

    // TODO: update with data from accelerometer
    LINE_Request_RearLightMotionDebug_data.fields.aX = 0;
    LINE_Request_RearLightMotionDebug_data.fields.aY = 0;
    LINE_Request_RearLightMotionDebug_data.fields.aZ = 0;
    LINE_Request_RearLightMotionDebug_data.fields.Braking = 0;
}
