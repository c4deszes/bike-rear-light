#include "app/calib.h"
#include "app/calib_priv.h"

#include "app/feature.h"

#include "bsp/volt_cfg.h"
#include "app/volt.h"

#include "hal/dsu.h"
#include "hal/nvmctrl.h"

static calib_layout_generic_t* CALIB_Memory = (calib_layout_generic_t*)CALIB_FLASH_ADDRESS;
uint32_t CALIB_CalculatedCrc;

static struct {
    bool CrcInvalid : 1;
    bool VersionInvalid : 1;
    bool VoltageCalibInvalid : 1;
    bool ImuAccelCalibInvalid : 1;
    uint8_t Reserved : 3;
} CALIB_ErrorFlags;

static uint16_t CALIB_VoltageSlopeCalib;
static int16_t CALIB_VoltageOffsetCalib;
static int16_t CALIB_ImuAccelXCalib;
static int16_t CALIB_ImuAccelYCalib;
static int16_t CALIB_ImuAccelZCalib;

static void CALIB_InitSafeDefaults(void) {
    CALIB_VoltageSlopeCalib = VOLT_DEFAULT_CALIB_SLOPE;
    CALIB_VoltageOffsetCalib = 0;
    CALIB_ImuAccelXCalib = 0;
    CALIB_ImuAccelYCalib = 0;
    CALIB_ImuAccelZCalib = 0;
}

static void CALIB_LoadCalibration_v1(void) {
    calib_layout_v1_t* mem = (calib_layout_v1_t*)CALIB_Memory;

    // Voltage calibration validation
    uint16_t temp_voltage_slope_calib = mem->Properties.Voltage_Slope_Calib;
    int16_t temp_voltage_offset_calib = mem->Properties.Voltage_Slope_Offset;
    
    // TODO: validate offset
    if (temp_voltage_slope_calib >= VOLT_CALIB_SLOPE_MIN && temp_voltage_slope_calib <= VOLT_CALIB_SLOPE_MAX) {
        CALIB_VoltageSlopeCalib = temp_voltage_slope_calib;
        CALIB_VoltageOffsetCalib = temp_voltage_offset_calib;
        CALIB_ErrorFlags.VoltageCalibInvalid = 0;
    }
    else {
        CALIB_ErrorFlags.VoltageCalibInvalid = 1;
    }

    // IMU accel calibration - no defined valid range, so just load it
    CALIB_ImuAccelXCalib = mem->Properties.Imu_Accel_Calib_X;
    CALIB_ImuAccelYCalib = mem->Properties.Imu_Accel_Calib_Y;
    CALIB_ImuAccelZCalib = mem->Properties.Imu_Accel_Calib_Z;
    CALIB_ErrorFlags.ImuAccelCalibInvalid = 0; // No validation for IMU accel calib in this version
}

static void CALIB_LoadNvram(void) {
    CALIB_CalculatedCrc = DSU_CalculateCRC32(DSU_CRC32_INITIAL,
                                              (void*)CALIB_FLASH_ADDRESS,
                                              CALIB_FLASH_SIZE-sizeof(uint32_t));

    if (CALIB_CalculatedCrc == CALIB_Memory->Crc32) {
        // TODO: load properties based on version
        if (CALIB_Memory->Version == 1) {
            CALIB_LoadCalibration_v1();
        }
        else {
            /* Unknown version - could implement fallback or default loading behavior here */
            CALIB_ErrorFlags.VersionInvalid = 1;
        }
    }
    else {
        /* CRC is invalid */
        CALIB_ErrorFlags.CrcInvalid = 1;
    }
}

void CALIB_Init(void) {
    CALIB_InitSafeDefaults();

#if FEATURE_CALIB_LOAD_AT_STARTUP == 1
    CALIB_LoadNvram();
#endif
}

bool CALIB_Save(void) {
    // TODO: implement saving to flash with wear leveling and CRC calculation
    return false;
}

bool CALIB_GetVoltageCalib(uint16_t* slope_calib, int16_t* offset_calib)
{
    // TODO: validate pointers
    *slope_calib = CALIB_VoltageSlopeCalib;
    *offset_calib = CALIB_VoltageOffsetCalib;
    return true;
}

bool CALIB_SetVoltageCalib(uint16_t slope_calib, int16_t offset_calib) {
    // TODO: validate calib values
    CALIB_VoltageSlopeCalib = slope_calib;
    CALIB_VoltageOffsetCalib = offset_calib;
    return true;
}

bool CALIB_GetImuAccelCalib(int16_t* x_calib, int16_t* y_calib, int16_t* z_calib)
{
#if FEATURE_CALIB_IMU_ENABLE == 1
    // TODO: validate pointers
    *x_calib = CALIB_ImuAccelXCalib;
    *y_calib = CALIB_ImuAccelYCalib;
    *z_calib = CALIB_ImuAccelZCalib;
    return true;
#else
    return false;
#endif
}

bool CALIB_SetImuAccelCalib(int16_t x_calib, int16_t y_calib, int16_t z_calib)
{
#if FEATURE_CALIB_IMU_ENABLE == 1
    // TODO: validate calib values
    CALIB_ImuAccelXCalib = x_calib;
    CALIB_ImuAccelYCalib = y_calib;
    CALIB_ImuAccelZCalib = z_calib;
    return true;
#else
    return false;
#endif
}

