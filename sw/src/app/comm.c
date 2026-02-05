#include "app/comm.h"
#include "bsp/usart.h"

#include "line_protocol.h"
#include "line_api.h"
#include "flash_line_api.h"
#include "flash_line_diag.h"
#include "uds_api.h"
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
    .U_status = LINE_DIAG_POWER_STATUS_VOLTAGE_OK,
    .BOD_status = LINE_DIAG_POWER_STATUS_BOD_NONE,
    .I_operating = LINE_DIAG_POWER_STATUS_OP_CURRENT(100),
    .I_sleep = LINE_DIAG_POWER_STATUS_SLEEP_CURRENT(100)
};

static LINE_Diag_SoftwareVersion_t sw_version = {
    .major = APP_SW_MAJOR,
    .minor = APP_SW_MINOR,
    .patch = APP_SW_PATCH
};

static swtimer_t* comm_lightrequest_timer;
static swtimer_t* comm_speedstatus_timer;

// TODO: support op. mode (error in case all drivers fail, warning if IMU or single segment fails)
uint8_t LINE_Diag_GetOperationStatus(void) {
    return LINE_DIAG_OP_STATUS_OK;
}

LINE_Diag_PowerStatus_t* LINE_Diag_GetPowerStatus(void) {
    return &power_status;
}

uint32_t LINE_Diag_GetSerialNumber(void) {
    //return DSU_GetSerialNumber32();
    return 0x69696969;
}

LINE_Diag_SoftwareVersion_t* LINE_Diag_GetSoftwareVersion(void) {
    return &sw_version;
}

// TODO: remove later
void LINE_Transport_WriteRequest(uint16_t request) {

}

void COMM_Initialize(void) {
    USART_Initialize(LINE_NETWORK_BicycleNetwork1_BAUDRATE, &COMM_UsartBufferTx, &COMM_UsartBufferRx);
    USART_Enable();

    LINE_Transport_Init(true);
    LINE_App_Init();
    LINE_Diag_SetAddress(LINE_NODE_RearLight_DIAG_ADDRESS);
    FLASH_LINE_Init(FLASH_LINE_APPLICATION_MODE);

    UDS_Init();
    UDS_LINE_Init();

    // TODO: remove later
    UDS_AppContainer.Brightness_Safety_Level = 300;

    comm_lightrequest_timer = SWTIMER_Create();
    comm_speedstatus_timer = SWTIMER_Create();
}

void COMM_UpdatePhy(void) {
    uint8_t length = USART_Available();
    while (length > 0) {
        uint8_t data = USART_Read();
        LINE_Transport_Receive(data);
        length--;
    }

    LINE_Transport_Update(1);

    if (LINE_Request_LightSynchronization_flag() || LINE_Request_RearLightSetting_flag()) {
        SWTIMER_Setup(comm_lightrequest_timer, FEATURE_COMM_LIGHTREQUEST_TIMEOUT);
    }

    if (LINE_Request_SpeedStatus_flag()) {
        SWTIMER_Setup(comm_speedstatus_timer, FEATURE_COMM_SPEEDSTATUS_TIMEOUT);
    }
}

void LINE_Transport_WriteResponse(uint8_t size, uint8_t* payload, uint8_t checksum) {
    uint8_t fix = 69;
    USART_WriteData(&size, sizeof(uint8_t));
    // TODO: fix for skipped 3rd byte
    USART_WriteData(payload, 1);
    USART_WriteData(&fix, 1);
    USART_WriteData(payload+1, size-1);
    USART_WriteData(&checksum, sizeof(uint8_t));
    USART_FlushOutput();
}

static bool comm_bootrequest = false;
static bool comm_shutdown_request = false;
static bool comm_idle_request = false;

uint8_t FLASH_BL_EnterBoot(void) {

    // TODO: when do we reject boot entry requests?
    comm_bootrequest = false;

    return FLASH_LINE_BOOT_ENTRY_NO_BL_PRESENT;
}

bool COMM_BootRequest(void) {
    return comm_bootrequest;
}

bool COMM_ShutdownRequest(void) {
    return comm_shutdown_request;
}

bool COMM_IdleRequest(void) {
    comm_idle_request = true;
}

void COMM_ClearPendingRequests(void) {
    comm_bootrequest = false;
    comm_shutdown_request = false;
    comm_idle_request = false;
}

void LINE_Diag_OnIdle(void) {
    comm_idle_request = true;
}

void LINE_Diag_OnShutdown(void) {
    comm_shutdown_request = true;
}

uint16_t COMM_GetTargetBrightness(void) {
    return LINE_Request_LightSynchronization_data.fields.TargetBrightness * 10U;
}

bool COMM_LightRequestTimeout(void) {
    return SWTIMER_Elapsed(comm_lightrequest_timer);
}

uint8_t COMM_LightMode(void) {
    return LINE_Request_LightSynchronization_data.fields.LightMode;
}

uint8_t COMM_LightBehavior(void) {
    return LINE_Request_RearLightSetting_data.fields.Behavior;
}

bool COMM_SpeedStatusTimeout(void) {
    return SWTIMER_Elapsed(comm_speedstatus_timer);
}

bool COMM_SpeedStatusBraking(void) {
    if (LINE_Request_SpeedStatus_data.fields.GlobalSpeedState == LINE_ENCODER_GlobalSpeedStateEncoder_Ok) {
        if (LINE_Request_SpeedStatus_data.fields.BrakeState == LINE_ENCODER_BrakeStateEncoder_Braking) {
            return true;
        }
        return false;
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
    /* Tail light state equals the diagnostic state if there were errors, otherwise it's ok when off, and off when brightness is 0 */
    // lightcontrol_feature_state_t tail_state = LIGHTCONTROL_GetDiagnosticState();
    LINE_Request_RearLightStatus_data.fields.TailLightStatus = LINE_ENCODER_LightStatusEncoder_Ok;
    LINE_Request_RearLightStatus_data.fields.BrakeLightStatus = LINE_ENCODER_LightStatusEncoder_Ok;

    /* Turn Signal light is not present in Gen1.0 */
    LINE_Request_RearLightStatus_data.fields.TurnSignalLightStatus = LINE_ENCODER_LightStatusEncoder_Off;

    // TODO: measure MCU temp. and return accordingly
    LINE_Request_RearLightStatus_data.fields.ThermalStatus = LINE_ENCODER_ThermalStatusEncoder_NotMeasured;
    UDS_AppContainer.Brightness_Emergency_Level = 100;
}

void COMM_UpdateDebugSignals(void) {
    // LINE_Request_RearLightDebug_data.fields.SensorErrorCode = (uint8_t) BRAKE_GetAccelerometerErrorCode();

    // TODO: use actual temperature measurement
    // LINE_Request_RearLightDebug_data.fields.EcuTemperature = LINE_ENCODER_TemperatureEncoder_Encode(25);
}
