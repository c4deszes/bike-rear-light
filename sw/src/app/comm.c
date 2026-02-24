#include "app/comm.h"

// Hardware abstraction layer
#include "common/swtimer.h"

// Board support package
#include "bsp/light_control.h"

// Communication protocol
#include "line_protocol.h"
#include "line_api.h"
#include "flash_line_api.h"
#include "flash_line_diag.h"
#include "uds_gen.h"

// Application components
#include "app/feature.h"
#include "app/brake.h"
#include "app/temp.h"

static swtimer_t* COMM_LightRequestTimer;
static swtimer_t* COMM_SpeedStatusTimer;

void COMM_Init(void) {
    LINE_App_Init();
    UDS_Init();
    FLASH_LINE_Init(LD_RearLight_CHANNEL, FLASH_LINE_APPLICATION_MODE);

    COMM_LightRequestTimer = SWTIMER_Create();
    COMM_SpeedStatusTimer = SWTIMER_Create();
}

void COMM_Update10ms(void) {
    if (l_flg_tst_LightSynchronization()) {
        l_flg_clr_LightSynchronization();
        SWTIMER_Setup(COMM_LightRequestTimer, FEATURE_COMM_LIGHTREQUEST_TIMEOUT);
    }

    if (l_flg_tst_RearLightSetting()) {
        l_flg_clr_RearLightSetting();
        SWTIMER_Setup(COMM_LightRequestTimer, FEATURE_COMM_LIGHTREQUEST_TIMEOUT);
    }

    if (l_flg_tst_SpeedStatus()) {
        l_flg_clr_SpeedStatus();
        SWTIMER_Setup(COMM_SpeedStatusTimer, FEATURE_COMM_SPEEDSTATUS_TIMEOUT);
    }
}

uint16_t COMM_GetTargetBrightness(void) {
    uint16_t target = l_rd_LightSynchronization_TargetBrightness() * 10U;

    /* Limit the target brightness */
    if (target >= LIGHTCONTROL_BRIGHTNESS_MAX) {
        target = LIGHTCONTROL_BRIGHTNESS_MAX;
    }

    return target;
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

strobe_source_t COMM_LightBehavior(strobe_source_t default_source, strobe_source_t primary_source) {
    uint8_t behavior = l_rd_RearLightSetting_Behavior();
    if (behavior == L_LightBehaviorEncoder_Default) {
        return default_source;
    }
    else if (behavior == L_LightBehaviorEncoder_Blink) {
        return primary_source;
    }
    return strobe_source_disabled;
}

bool COMM_LightRequestTimeout(void) {
    return SWTIMER_Elapsed(COMM_LightRequestTimer);
}

bool COMM_SpeedStatusTimeout(void) {
    return SWTIMER_Elapsed(COMM_SpeedStatusTimer);
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
    return L_LightStatusEncoder_Error;
}

void COMM_UpdateSignals(void) {
    /* Tail light state equals the diagnostic state if there were errors */
    lightcontrol_feature_state_t tail_state = LIGHTCONTROL_GetDiagnosticState();
    uint8_t tail_status = COMM_EncodeLightStatus(tail_state);
    l_wr_RearLightStatus_TailLightStatus(tail_status);
    l_wr_RearLightStatus_BrakeLightStatus(tail_status);     /* No separate brakelight on Gen1.0b */

    /* Turn Signal light is not present in Gen1.0 */
    l_wr_RearLightStatus_TurnSignalLightStatus(L_LightStatusEncoder_Off);

    // TODO: measure MCU temp. and return accordingly
    l_wr_RearLightStatus_ThermalStatus(L_ThermalStatusEncoder_NotMeasured);
}

uint8_t COMM_EncodeBrakeStatus(bool braking, brake_signal_status_t brake_signal_status) {
    if(brake_signal_status == brake_signal_status_perm_error || brake_signal_status == brake_signal_status_na) {
        return L_BrakeStateEncoder_Disabled;
    }
    else if (braking) {
        return L_BrakeStateEncoder_Braking;
    }
    return L_BrakeStateEncoder_NotBraking;
}

void COMM_UpdateDebugSignals(void) {
    // TODO: use brightness from driver
    l_wr_RearLightBrightnessDebug_Brightness(0);

    uint8_t drive_temp = L_TemperatureEncoder_Encode(TEMP_GetDriveTemperature());
    uint8_t mcu_temp = L_TemperatureEncoder_Encode(TEMP_GetMcuTemperature());
    l_wr_RearLightTemperatureDebug_EcuTemperature(mcu_temp);
    l_wr_RearLightTemperatureDebug_DriveTemperature(drive_temp);

    int16_t accel_x = BRAKE_GetAccelerationX();
    int16_t accel_y = BRAKE_GetAccelerationY();
    int16_t accel_z = BRAKE_GetAccelerationZ();

    l_wr_RearLightMotionDebug_aX(accel_x);
    l_wr_RearLightMotionDebug_aY(accel_y);
    l_wr_RearLightMotionDebug_aZ(accel_z);

    uint8_t brake_status = COMM_EncodeBrakeStatus(BRAKE_IsBraking(), BRAKE_GetBrakeSignalStatus());
    l_wr_RearLightMotionDebug_Braking(brake_status);
}
