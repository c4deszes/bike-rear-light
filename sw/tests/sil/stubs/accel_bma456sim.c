#include "bsp/accel.h"

// Sensor driver
#include "bma456mm.h"

struct bma4_dev ACCEL_BMA456_Device = {
    .intf = BMA4_SPI_INTF,
    .variant = BMA45X_VARIANT,
    //bma456.intf_ptr = &dev_addr;
    .perf_mode_status = BMA4_DISABLE,
};

void ACCEL_Initialize(void) {

}

int8_t bma456mm_init(struct bma4_dev *dev)
{
    return BMA4_OK;
}

int8_t bma456mm_write_config_file(struct bma4_dev *dev)
{
    return BMA4_OK;
}

int8_t bma4_set_accel_config(const struct bma4_accel_config *accel, struct bma4_dev *dev)
{
    return BMA4_OK;
}

int8_t bma4_set_accel_enable(uint8_t enable, struct bma4_dev *dev)
{
    return BMA4_OK;
}
