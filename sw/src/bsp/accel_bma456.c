#include "bsp/accel.h"

// Hardware abstraction layer
#include "hal/sercom_spi.h"
#include "hal/gpio.h"

// Board support package
#include "bsp/pinout.h"

// Sensor driver
#include "bma456mm.h"

static int8_t BMA456_ReadAdapter(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
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

static int8_t BMA456_WriteAdapter(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
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

struct bma4_dev ACCEL_BMA456_Device = {
    .intf = BMA4_SPI_INTF,
    .bus_read = BMA456_ReadAdapter,
    .bus_write = BMA456_WriteAdapter,
    .variant = BMA45X_VARIANT,
    //bma456.intf_ptr = &dev_addr;
    .delay_us = delayMicroseconds,
    .read_write_len = 46u,
    .perf_mode_status = BMA4_DISABLE,
};

void ACCEL_Init(void) {
    GPIO_PinWrite(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, HIGH);
    GPIO_SetupPinOutput(BMA456_SPI_CS_PORT, BMA456_SPI_CS_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

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
