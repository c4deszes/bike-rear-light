#include "app/brake.h"
#include "app/brightness.h"

#include "bma456mm.h"

#include "bsp/spi.h"

#include "hal/gpio.h"
#include "bsp/pinout.h"

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
static struct bma4_accel sens_data = { 0 };
static struct bma4_accel_config accel_conf = { 0 };
static int8_t sensor_init_code;
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

  /*
   *  The following loop:
   *
   *    for (; ul; ul--) {
   *      __asm__ volatile("");
   *    }
   *
   *  produce the following assembly code:
   *
   *    loop:
   *      subs r3, #1        // 1 Core cycle
   *      bne.n loop         // 1 Core cycle + 1 if branch is taken
   */

  // VARIANT_MCK / 1000000 == cycles needed to delay 1uS
  //                     3 == cycles used in a loop
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
    accel_conf.range = BMA4_ACCEL_RANGE_2G;

    /* The bandwidth parameter is used to configure the number of sensor samples that are averaged
     * if it is set to 2, then 2^(bandwidth parameter) samples
     * are averaged, resulting in 4 averaged samples
     * Note1 : For more information, refer the datasheet.
     * Note2 : A higher number of averaged samples will result in a less noisier signal, but
     * this has an adverse effect on the power consumed.
     */
    accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    accel_conf.perf_mode = BMA4_CIC_AVG_MODE;

    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, HIGH);
    GPIO_SetupPinOutput(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, &output);

    GPIO_EnableFunction(BMA456_SPI_MISO_PORT, BMA456_SPI_MISO_PIN, BMA456_SPI_MISO_PINMUX);
    GPIO_EnableFunction(BMA456_SPI_SCK_PORT, BMA456_SPI_SCK_PIN, BMA456_SPI_SCK_PINMUX);
    GPIO_EnableFunction(BMA456_SPI_MOSI_PORT, BMA456_SPI_MOSI_PIN, BMA456_SPI_MOSI_PINMUX);

    SERCOM_SPI_SetupMaster(SERCOM1, 8000000UL, 1000000UL, 0, 0);
    SERCOM_SPI_Enable(SERCOM1);
}

// (int8_t)coines_write(COINES_SENSOR_INTF_SPI, dev_addr, 0, reg_addr, reg_data, count);

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

static uint8_t counter = 0;

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

    if (brake_signal_state == brake_signal_status_ok) {
        int8_t result = bma4_read_accel_xyz(&sens_data, &bma456);
    }

    // else if (brake_signal_state == brake_signal_status_ok) {

    // }
    // else if (brake_signal_state == brake_signal_status_perm_error) {

    // }
    // else {

    // }


    // rslt = bma4_read_accel_xyz(&sens_data, &bma);

    // 1. if we trust the brake signal from the speed sensor then use that

    // 2. else if we trust the accelerometer

    // otherwise turn off the brake light and set error?

}