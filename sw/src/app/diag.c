#include "app/diag.h"

#include "hal/dsu.h"

#include "line_protocol.h"
#include "line_api.h"
#include "flash_line_api.h"
#include "flash_line_diag.h"
#include "uds_diag.h"
#include "uds_gen.h"

#include "bsp/light_control.h"

#include "metainfo.h"
#include "app/brake.h"
#include "app/calib.h"
#include "app/config.h"
#include "app/current.h"
#include "app/feature.h"
#include "app/temp.h"
#include "app/volt.h"
#include "app/router.h"

static LINE_Diag_PowerStatus_t DIAG_PowerStatus;

static const LINE_Diag_SoftwareVersion_t DIAG_SoftwareVersion = {
    .major = APP_SW_MAJOR,
    .minor = APP_SW_MINOR,
    .patch = APP_SW_PATCH
};

static uint8_t DIAG_OperationStatus;
static bool DIAG_BootResetFlag;
static bool DIAG_ShutdownFlag;
static bool DIAG_IdleFlag;

void ld_BicycleNetwork_RearLight_OnWakeup(void) {

}
void ld_BicycleNetwork_RearLight_OnIdle(void) {
    DIAG_IdleFlag = true;
}
void ld_BicycleNetwork_RearLight_OnShutdown(void) {
    DIAG_ShutdownFlag = true;
}
void ld_BicycleNetwork_RearLight_OnConditionalChangeAddress(uint8_t old_address, uint8_t new_address) {

}

uint8_t ld_BicycleNetwork_RearLight_GetOperationStatus(void) {
    return DIAG_OperationStatus;
}

LINE_Diag_PowerStatus_t* ld_BicycleNetwork_RearLight_GetPowerStatus(void) {
    return &DIAG_PowerStatus;
}

uint32_t ld_BicycleNetwork_RearLight_GetSerialNumber(void) {
    return DSU_GetSerialNumber32();
}

LINE_Diag_SoftwareVersion_t* ld_BicycleNetwork_RearLight_GetSoftwareVersion(void) {
    return &DIAG_SoftwareVersion;
}

fl_BootEntryResponse_t FLASH_BL_EnterBoot(void) {
    fl_BootEntryResponse_t response;

#if FEATURE_DIAG_ENABLE_BOOTENTRY == 1
    // TODO: design conditions under which boot entry shouldn't be allowed

    DIAG_BootResetFlag = true;
    response.entry_status = FLASH_LINE_BOOT_ENTRY_SUCCESS;
#else
    response.entry_status = FLASH_LINE_BOOT_ENTRY_OP_UNSAFE;
#endif

    response.serial_number = ld_BicycleNetwork_RearLight_GetSerialNumber();

    return response;
}

void UDS_RearLight_OnPropertyChange(const uds_property_t* property) {
    CONFIG_NotifyPropertyChange(property);
}

void DIAG_Init(void) {
    DIAG_OperationStatus = LINE_DIAG_OP_STATUS_INIT;
    DIAG_BootResetFlag = false;
    DIAG_ShutdownFlag = false;
    DIAG_IdleFlag = false;
}

static bool DIAG_ConvertUdsSegmentToOutput(uds_Segment_t segment, brightness_output_t* output) {
     switch (segment) {
        case uds_Segment_TAIL:
            *output = brightness_output_tail;
            return true;
        case uds_Segment_BRAKE:
            *output = brightness_output_brake;
            return true;
        case uds_Segment_SIGNAL_LEFT:
            *output = brightness_output_turn;
            return true;
        default:
            return false;     // Invalid segment
    }
}

void DIAG_Update10ms(void) {

    /* Configuration reload */
    if (UDS_RearLight_Config_Reload_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_ENABLE_CONFIG_RELOAD == 1
        CONFIG_Reload();
        CONFIG_ReloadComponents();

        UDS_RearLight_Config_Reload_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        #else
        UDS_RearLight_Config_Reload_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Configuration save */
    if (UDS_RearLight_Config_Save_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_ENABLE_CONFIG_SAVE == 1
        CONFIG_Reload();
        CONFIG_Save();

        UDS_RearLight_Config_Save_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        #else
        UDS_RearLight_Config_Save_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Calibrate voltage */
    if (UDS_RearLight_Calib_Voltage_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_CALIB_VOLTAGE_ENABLE == 1
        UDS_Service_RearLight_Calib_Voltage_RequestContext_t* request = UDS_RearLight_Calib_Voltage_GetPendingServiceRequest();
        UDS_Service_RearLight_Calib_Voltage_ResponseContext_t* response = UDS_RearLight_Calib_Voltage_GetPendingServiceResponse();

        bool result = CALIB_SetVoltageCalib(request->Slope, request->Offset);

        if (result) {
            UDS_RearLight_Calib_Voltage_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        }
        else {
            UDS_RearLight_Calib_Voltage_FinishServiceRequest(UDS_SERVICE_CALL_GENERIC_FAILURE);
        }
        #else
        UDS_RearLight_Calib_Voltage_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Calibration save */
    if (UDS_RearLight_Calib_Save_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_CALIB_SAVE_ENABLE == 1
        UDS_Service_RearLight_Calib_Save_RequestContext_t* request = UDS_RearLight_Calib_Save_GetPendingServiceRequest();
        UDS_Service_RearLight_Calib_Save_ResponseContext_t* response = UDS_RearLight_Calib_Save_GetPendingServiceResponse();

        CALIB_Save();

        UDS_RearLight_Calib_Save_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        #else
        UDS_RearLight_Calib_Save_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Driver control */
    if (UDS_RearLight_Driver_Control_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_DRIVER_CONTROL_ENABLE == 1
        UDS_Service_RearLight_Driver_Control_RequestContext_t* request = UDS_RearLight_Driver_Control_GetPendingServiceRequest();
        UDS_Service_RearLight_Driver_Control_ResponseContext_t* response = UDS_RearLight_Driver_Control_GetPendingServiceResponse();

        brightness_output_t output;
        bool result = DIAG_ConvertUdsSegmentToOutput(request->Segment, &output);

        if (result) {

            result = ROUTER_DiagRequest(output, request->Mode, request->Brightness);

            if (result) {
                UDS_RearLight_Driver_Control_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
            }
            else {
                UDS_RearLight_Driver_Control_FinishServiceRequest(UDS_SERVICE_CALL_GENERIC_FAILURE);
            }
        }
        else {
            UDS_RearLight_Driver_Control_FinishServiceRequest(UDS_SERVICE_CALL_GENERIC_FAILURE);
        }
        #else
        UDS_RearLight_Driver_Control_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Driver status */
    // TODO: implement

    /* Adc Read */
    if (UDS_RearLight_Adc_Read_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_ADC_READ_ENABLE == 1
        UDS_Service_RearLight_Adc_Read_RequestContext_t* request = UDS_RearLight_Adc_Read_GetPendingServiceRequest();
        UDS_Service_RearLight_Adc_Read_ResponseContext_t* response = UDS_RearLight_Adc_Read_GetPendingServiceResponse();

        if (request->Channel == uds_AdcChannel_McuTemperature) {
            response->return_value = TEMP_GetMcuTemperatureAdc();

            UDS_RearLight_Adc_Read_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        }
        else if (request->Channel == uds_AdcChannel_DriveTemperature) {
            response->return_value = TEMP_GetDriveTemperatureAdc();

            UDS_RearLight_Adc_Read_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        }
        else if (request->Channel == uds_AdcChannel_BatteryVoltage) {
            response->return_value = VOLT_GetAdcRawValue();

            UDS_RearLight_Adc_Read_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        }
        else if (request->Channel == uds_AdcChannel_TailDriverVoltage) {
            // TODO: implement
            UDS_RearLight_Adc_Read_FinishServiceRequest(UDS_SERVICE_CALL_GENERIC_FAILURE);
        }
        else {
            UDS_RearLight_Adc_Read_FinishServiceRequest(UDS_SERVICE_CALL_GENERIC_FAILURE);
        }
        #else
        UDS_RearLight_Adc_Read_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Imu Read */
    if (UDS_RearLight_Imu_Read_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_IMU_READ_ENABLE == 1
        UDS_Service_RearLight_Imu_Read_RequestContext_t* request = UDS_RearLight_Imu_Read_GetPendingServiceRequest();
        UDS_Service_RearLight_Imu_Read_ResponseContext_t* response = UDS_RearLight_Imu_Read_GetPendingServiceResponse();

        // TODO: if sensor error then return failure response
        int16_t accel_x, accel_y, accel_z;
        BRAKE_GetAcceleration(&accel_x, &accel_y, &accel_z);

        response->return_value.AccelX = accel_x;
        response->return_value.AccelY = accel_y;
        response->return_value.AccelZ = accel_z;

        response->return_value.GyroX = 0;     // TODO: implement
        response->return_value.GyroY = 0;     // TODO: implement
        response->return_value.GyroZ = 0;     // TODO: implement

        UDS_RearLight_Imu_Read_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        #else
        UDS_RearLight_Imu_Read_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }

    /* Imu Calibrate */
    if (UDS_RearLight_Imu_Calibrate_HasPendingServiceRequest()) {
        #if FEATURE_DIAG_CALIB_IMU_ENABLE == 1
        UDS_Service_RearLight_Imu_Calibrate_RequestContext_t* request = UDS_RearLight_Imu_Calibrate_GetPendingServiceRequest();
        UDS_Service_RearLight_Imu_Calibrate_ResponseContext_t* response = UDS_RearLight_Imu_Calibrate_GetPendingServiceResponse();

        // TODO: if sensor error then return failure response
        int16_t accel_x, accel_y, accel_z;
        BRAKE_GetAcceleration(&accel_x, &accel_y, &accel_z);

        bool success = CALIB_SetImuAccelCalib(accel_x, accel_y, accel_z);

        if (success) {
            UDS_RearLight_Imu_Calibrate_FinishServiceRequest(UDS_SERVICE_CALL_SUCCESS);
        } else {
            UDS_RearLight_Imu_Calibrate_FinishServiceRequest(UDS_SERVICE_CALL_GENERIC_FAILURE);
        }
        #else
        UDS_RearLight_Imu_Calibrate_FinishServiceRequest(UDS_SERVICE_CALL_NO_SUCH_SERVICE);
        #endif
    }
}

void DIAG_Update100ms(void) {
    /* Update operation status */
    lightcontrol_feature_state_t tail_state = LIGHTCONTROL_GetDiagnosticState(lightcontrol_segment_tail);
    lightcontrol_feature_state_t brake_state = LIGHTCONTROL_GetDiagnosticState(lightcontrol_segment_brake);

    if (tail_state == lightcontrol_feature_state_ok && brake_state == lightcontrol_feature_state_ok) {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_OK;
    }
    else if (tail_state == lightcontrol_feature_state_partial_error || brake_state == lightcontrol_feature_state_partial_error) {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_WARN;
    }
    else if (tail_state == lightcontrol_feature_state_error || brake_state == lightcontrol_feature_state_error) {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_ERROR;
    }
    else {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_ERROR;
    }

    /* Update power status */
    DIAG_PowerStatus.U_measured = VOLT_GetVoltage();
    DIAG_PowerStatus.I_operating = CURRENT_GetCurrent();
    DIAG_PowerStatus.I_sleep = LINE_DIAG_POWER_STATUS_SLEEP_CURRENT(100);       // TODO: update based on HW measurement
}

bool DIAG_BootRequest(void) {
    bool flag = DIAG_BootResetFlag;
    DIAG_BootResetFlag = false;
    return flag;
}

bool DIAG_ShutdownRequest(void) {
    bool flag = DIAG_ShutdownFlag;
    DIAG_ShutdownFlag = false;
    return flag;
}

bool DIAG_IdleRequest(void) {
    bool flag = DIAG_IdleFlag;
    DIAG_IdleFlag = false;
    return flag;
}
