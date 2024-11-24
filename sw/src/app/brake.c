#include "app/brake.h"
#include "app/brightness.h"

#include "bma456mm.h"

#include "hal/sercom_spi.h"

#include "hal/gpio.h"
#include "bsp/pinout.h"

#include "app/config.h"
#include "app/comm.h"

// Concept: use MM feature set
// - configure High G detection
// - configure range 2/4G
// - Set high threshold to 0.25g, negative
//
// Potential problems:
// - Axis sensitivity depends on orientation, while in a climb, descent or a turn the
//   the acceleration components will be different than on a flat
// - Road bumps might cause inadvertent brake activation

static struct bma4_dev bma456 = { 0 };
static struct bma4_accel_config accel_conf = { 0 };
static int8_t sensor_init_code;

static const float alpha = 0.1; // Low pass filter coefficient

static struct bma4_accel previous_data = { 0 };
static struct bma4_accel current_data = { 0 };
static struct bma4_accel gravity_vector = { 0 };
static brake_signal_status_t brake_signal_state;

static const gpio_pin_output_configuration output = {
    .drive = NORMAL,
    .input = false
};

int8_t BMA456_ReadAdapter(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, LOW);
    SERCOM_SPI_TransferByte(SERCOM1, reg_addr);  // Read register address (driver sets the R/W bit)
    
    // Dummy byte is included by the driver
    for (uint32_t i=0;i<len;i++) {
        uint8_t data = SERCOM_SPI_TransferByte(SERCOM1, 0xFF); // Read from slave

        reg_data[i] = data;
    }

    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, HIGH);

    return BMA4_INTF_RET_SUCCESS;
}

int8_t BMA456_WriteAdapter(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, LOW);
    SERCOM_SPI_TransferByte(SERCOM1, reg_addr);  // Read register address (driver sets the R/W bit)
    
    // Dummy byte is included by the driver
    for (uint32_t i=0;i<len;i++) {
        SERCOM_SPI_TransferByte(SERCOM1, reg_data[i]); // Read from slave
    }

    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, HIGH);

    return BMA4_INTF_RET_SUCCESS;
}

static void delayMicroseconds( uint32_t usec, void* ptr )
{
  if ( usec == 0 )
  {
    return ;
  }

  uint32_t n = usec * (48000000 / 1000000) / 3;
  __asm__ __volatile__(
    "1:              \n"
    "   sub %0, #1   \n" // substract 1 from %0 (n)
    "   bne 1b       \n" // if result is not 0 jump to 1
    : "+r" (n)           // '%0' is n variable with RW constraints
    :                    // no input
    :                    // no clobber
  );
  // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
  // https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Volatile
}

void BRAKE_Init(void) {
    brake_signal_state = brake_signal_status_na;

    accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    accel_conf.range = BMA4_ACCEL_RANGE_4G;
    accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    accel_conf.perf_mode = BMA4_CIC_AVG_MODE;

    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, HIGH);
    GPIO_SetupPinOutput(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, &output);

    // TODO: consider pullup/pulldown on MISO, there's capacitive discharge at the end of transcations
    GPIO_EnableFunction(BMA456_SPI_MISO_PORT, BMA456_SPI_MISO_PIN, BMA456_SPI_MISO_PINMUX);
    GPIO_EnableFunction(BMA456_SPI_SCK_PORT, BMA456_SPI_SCK_PIN, BMA456_SPI_SCK_PINMUX);
    GPIO_EnableFunction(BMA456_SPI_MOSI_PORT, BMA456_SPI_MOSI_PIN, BMA456_SPI_MOSI_PINMUX);

    SERCOM_SPI_SetupMaster(SERCOM1, 8000000UL, 1000000UL,
                           sercom_spi_dataorder_msb,
                           sercom_spi_cpha_trailing, sercom_spi_cpol_idle_high,
                           SERCOM_SPI_MOSI_PAD0, SERCOM_SPI_MISO_PAD3);
    SERCOM_SPI_Enable(SERCOM1);
}

static int8_t BRAKE_SetupSensor(void) {
    int8_t rslt;
    bma456.intf = BMA4_SPI_INTF;
    bma456.bus_read = BMA456_ReadAdapter;
    bma456.bus_write = BMA456_WriteAdapter;
    bma456.variant = BMA45X_VARIANT;
    //bma456.intf_ptr = &dev_addr;
    bma456.delay_us = delayMicroseconds;
    bma456.read_write_len = 46u;
    bma456.perf_mode_status = BMA4_DISABLE;

    /* Sensor initialization */
    rslt = bma456mm_init(&bma456);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    /* Upload the configuration file to enable the features of the sensor. */
    rslt = bma456mm_write_config_file(&bma456);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    /* Set the accel configurations */
    rslt = bma4_set_accel_config(&accel_conf, &bma456);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    /* NOTE : Enable accel after set of configurations */
    rslt = bma4_set_accel_enable(BMA4_ENABLE, &bma456);

    if (rslt != BMA4_OK) {
        return rslt;
    }

    return rslt;
}

int8_t BRAKE_GetAccelerometerErrorCode(void) {
    return sensor_init_code;
}

int16_t BRAKE_GetAccelerationX(void) {
    return current_data.x;
}

int16_t BRAKE_GetAccelerationY(void) {
    return current_data.y;
}

int16_t BRAKE_GetAccelerationZ(void) {
    return current_data.z;
}

void BRAKE_Update10ms(void) {
    if (brake_signal_state == brake_signal_status_na) {
        sensor_init_code = BRAKE_SetupSensor();

        if (sensor_init_code != BMA4_OK) {
            brake_signal_state = brake_signal_status_perm_error;
        }
        else {
            brake_signal_state = brake_signal_status_ok;
        }
    }

    bool internal_brake = false;

#if FEATURE_BRAKE_USE_INTERNAL_SIGNAL == 1
    if (brake_signal_state == brake_signal_status_ok) {
        int8_t result = bma4_read_accel_xyz(&current_data, &bma456);

        // TODO: determine braking
        // TODO: if error result then put signal status to error
        if (result == BMA4_OK) {

            gravity_vector.x = (int16_t)(alpha * current_data.x + (1 - alpha) * previous_data.x);
            gravity_vector.y = (int16_t)(alpha * current_data.y + (1 - alpha) * previous_data.y);
            gravity_vector.z = (int16_t)(alpha * current_data.z + (1 - alpha) * previous_data.z);

            previous_data.x = current_data.x;
            previous_data.y = current_data.y;
            previous_data.z = current_data.z;

            struct bma4_accel difference = { 0 };
            difference.x = current_data.x - gravity_vector.x;
            difference.y = current_data.y - gravity_vector.y;
            difference.z = current_data.z - gravity_vector.z;

            if (difference.z < -1000) {
                internal_brake = true;
            }
            else {
                internal_brake = false;
            }

        } else {
            brake_signal_state = brake_signal_status_perm_error;
        }
    }
#else

#endif

    bool external_brake = false;

// TODO: in safety mode we should ignore the external signal (assume that comms are bad)
#if FEATURE_BRAKE_USE_EXTERNAL_SIGNAL == 1
    if (!COMM_SpeedStatusTimeout() && COMM_SpeedStatusBraking()) {
        external_brake = true;
    }
#else

#endif

    if (internal_brake || external_brake) {
        BRIGHTNESS_SetBraking(true);
    }
    else {
        BRIGHTNESS_SetBraking(false);
    }
}