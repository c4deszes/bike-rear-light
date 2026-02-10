#include "app/comm.h"

// Hardware abstraction layer
#include "hal/dsu.h"
#include "common/swtimer.h"

#include "line_protocol.h"
#include "line_api.h"
#include "flash_line_api.h"
#include "flash_line_diag.h"
#include "uds_gen.h"

#include "app/brake.h"
#include "bsp/light_control.h"

#include "app/config.h"
#include "app/feature.h"
#include "metainfo.h"



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

void ld_BicycleNetwork_RearLight_OnWakeup(void) {

}
void ld_BicycleNetwork_RearLight_OnIdle(void) {
    comm_idle_request = true;
}
void ld_BicycleNetwork_RearLight_OnShutdown(void) {
    comm_shutdown_request = true;
}
void ld_BicycleNetwork_RearLight_OnConditionalChangeAddress(uint8_t old_address, uint8_t new_address) {

}

// TODO: op status should be dynamic based on device state
uint8_t ld_BicycleNetwork_RearLight_GetOperationStatus(void) {
    return LINE_DIAG_OP_STATUS_OK;
}
LINE_Diag_PowerStatus_t* ld_BicycleNetwork_RearLight_GetPowerStatus(void) {
    return &power_status;
}
uint32_t ld_BicycleNetwork_RearLight_GetSerialNumber(void) {
    return DSU_GetSerialNumber32();
}
LINE_Diag_SoftwareVersion_t* ld_BicycleNetwork_RearLight_GetSoftwareVersion(void) {
    return &sw_version;
}

void COMM_Initialize(void) {
    LINE_App_Init();
    UDS_Init();
    FLASH_LINE_Init(LD_RearLight_CHANNEL, FLASH_LINE_APPLICATION_MODE);

    comm_lightrequest_timer = SWTIMER_Create();
    comm_speedstatus_timer = SWTIMER_Create();
}

void COMM_Update(void) {
    // TODO: also timeout if setpoint is invalid for a long time
    if (l_flg_tst_LightSynchronization()) {
        l_flg_clr_LightSynchronization();
        SWTIMER_Setup(comm_lightrequest_timer, FEATURE_COMM_LIGHTREQUEST_TIMEOUT);
    }

    if (l_flg_tst_RearLightSetting()) {
        l_flg_clr_RearLightSetting();
        SWTIMER_Setup(comm_lightrequest_timer, FEATURE_COMM_LIGHTREQUEST_TIMEOUT);
    }

    if (l_flg_tst_SpeedStatus()) {
        l_flg_clr_SpeedStatus();
        SWTIMER_Setup(comm_speedstatus_timer, FEATURE_COMM_SPEEDSTATUS_TIMEOUT);
    }
}

fl_BootEntryResponse_t FLASH_BL_EnterBoot(void) {
    fl_BootEntryResponse_t response;
    
    // TODO: when do we reject boot entry requests?
    comm_bootrequest = false;

    response.entry_status = FLASH_LINE_BOOT_ENTRY_SUCCESS;
    response.serial_number = ld_BicycleNetwork_RearLight_GetSerialNumber();

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
    return l_rd_LightSynchronization_TargetBrightness() * 10U;
}

brightness_mode_t COMM_LightMode(void) {
    uint8_t light_mode = l_rd_LightSynchronization_LightMode();
    if (light_mode == L_LightModeEncoder_Adaptive) {
        return brightness_mode_adaptive;
    }
    else if (light_mode == L_LightModeEncoder_Standard) {
        return brightness_mode_standard;
    }
    else if (light_mode == L_LightModeEncoder_Emergency) {
        return brightness_mode_emergency;
    }
    else if (light_mode == L_LightModeEncoder_Off) {
        return brightness_mode_off;
    }
    return brightness_mode_safety;
}

strobe_source_t COMM_LightBehavior(void) {
    uint8_t behavior = l_rd_RearLightSetting_Behavior();
    if (behavior == L_LightBehaviorEncoder_Default) {
        return CONFIG_Props.Strobe_ModeDefault;
    }
    else if (behavior == L_LightBehaviorEncoder_Blink) {
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
    if (   l_rd_SpeedStatus_SpeedState() == L_SpeedStateEncoder_Ok
        && l_rd_SpeedStatus_BrakeState() == L_BrakeStateEncoder_Braking) {

        return true;
    }
    return false;
}

static uint8_t COMM_EncodeLightStatus(lightcontrol_feature_state_t state) {
    if (state == lightcontrol_feature_state_ok) {
        return L_LightStatusEncoder_Ok;
    }
    else if(state == lightcontrol_feature_state_partial_error) {
        return L_LightStatusEncoder_PartialError;
    }
    else if(state == lightcontrol_feature_state_error) {
        return L_LightStatusEncoder_Error;
    }
    else {
        return L_LightStatusEncoder_Error;
    }
}

void COMM_UpdateSignals(void) {
    /* Tail light state equals the diagnostic state if there were errors, otherwise it's ok when off, and off when brightness is 0 */
    // lightcontrol_feature_state_t tail_state = LIGHTCONTROL_GetDiagnosticState();
    l_wr_RearLightStatus_TailLightStatus(L_LightStatusEncoder_Ok);
    l_wr_RearLightStatus_BrakeLightStatus(L_LightStatusEncoder_Ok);

    /* Turn Signal light is not present in Gen1.0 */
    l_wr_RearLightStatus_TurnSignalLightStatus(L_LightStatusEncoder_Off);
    // TODO: measure MCU temp. and return accordingly
    l_wr_RearLightStatus_ThermalStatus(L_ThermalStatusEncoder_NotMeasured);
}

void COMM_UpdateDebugSignals(void) {
    // TODO: use brightness from driver
    l_wr_RearLightBrightnessDebug_Brightness(0);

    // TODO: use actual temperature measurement
    l_wr_RearLightTemperatureDebug_EcuTemperature(L_TemperatureEncoder_Encode(25));
    l_wr_RearLightTemperatureDebug_DriveTemperature(L_TemperatureEncoder_Encode(25));

    // TODO: update with data from accelerometer
    l_wr_RearLightMotionDebug_aX(0);
    l_wr_RearLightMotionDebug_aY(0);
    l_wr_RearLightMotionDebug_aZ(0);
    l_wr_RearLightMotionDebug_Braking(0);
}
