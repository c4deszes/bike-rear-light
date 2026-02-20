#include "app/diag.h"

#include "hal/dsu.h"

#include "line_protocol.h"
#include "line_api.h"
#include "flash_line_api.h"
#include "flash_line_diag.h"
#include "uds_gen.h"

#include "bsp/light_control.h"

#include "app/config.h"
#include "app/feature.h"
#include "metainfo.h"
#include "app/current.h"
#include "app/volt.h"

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
    DIAG_BootResetFlag = true;
    response.entry_status = FLASH_LINE_BOOT_ENTRY_SUCCESS;
#else
    response.entry_status = FLASH_LINE_BOOT_ENTRY_OP_UNSAFE;
#endif

    response.serial_number = ld_BicycleNetwork_RearLight_GetSerialNumber();

    return response;
}

void DIAG_Init(void) {
    DIAG_OperationStatus = LINE_DIAG_OP_STATUS_INIT;
    DIAG_BootResetFlag = false;
    DIAG_ShutdownFlag = false;
    DIAG_IdleFlag = false;
}

void DIAG_Update10ms(void) {
    // TODO: handle pending UDS service calls
}

void DIAG_Update100ms(void) {
    /* Update operation status */
    if (LIGHTCONTROL_GetDiagnosticState() == lightcontrol_feature_state_ok) {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_OK;
    }
    else if (LIGHTCONTROL_GetDiagnosticState() == lightcontrol_feature_state_partial_error) {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_WARN;
    }
    else if (LIGHTCONTROL_GetDiagnosticState() == lightcontrol_feature_state_error) {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_ERROR;
    }
    else {
        DIAG_OperationStatus = LINE_DIAG_OP_STATUS_ERROR;
    }

    /* Update power status */
    DIAG_PowerStatus.U_measured = VOLT_GetVoltage();            // TODO: convert data
    DIAG_PowerStatus.I_operating = CURRENT_GetCurrent();        // TODO: convert data
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
