#include "app/radar.h"

#include "hal/sercom_i2c.h"
#include "bsp/pinout.h"

uint32_t version = 0;

void RADAR_Init(void)
{

}

void RADAR_Update100ms(void)
{
    uint16_t reg_addr = 0x0000;
    uint32_t reg_value = 0;
    sercom_i2c_result_t result = SERCOM_I2C_WriteRead(
        XM125_I2C_INSTANCE,
        0x52,
        (uint8_t*)&reg_addr,
        sizeof(reg_addr),
        (uint8_t*)&reg_value,
        sizeof(reg_value)
    );

    if (result == sercom_i2c_ok) {
        version = reg_value;
    }

}
