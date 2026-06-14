#include "bsp/xm125.h"

#include "bsp/pinout.h"
#include "hal/gpio.h"
#include "hal/sercom_i2c.h"

static void XM125_SetResetHigh(void)
{
    GPIO_PinWrite(XM125_RESET_PORT, XM125_RESET_PIN, HIGH);
}

static void XM125_SetResetLow(void)
{
    GPIO_PinWrite(XM125_RESET_PORT, XM125_RESET_PIN, LOW);
}

void XM125_Init(void)
{
    /* Reset pin setup */
    XM125_SetResetHigh();
    GPIO_SetupPinOutput(XM125_RESET_PORT, XM125_RESET_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    /* Wake pin setup */
    XM125_Wake();
    GPIO_SetupPinOutput(XM125_WAKE_PORT, XM125_WAKE_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    /* Boot pin setup */
    XM125_SetBootLow();
    GPIO_SetupPinOutput(XM125_BOOT_PORT, XM125_BOOT_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);

    // TODO: INT pin
    // TODO: GPIO0 pin

    /* USART pins setup */
    GPIO_EnableFunction(XM125_TX_PORT, XM125_TX_PIN, XM125_TX_PINMUX);
    GPIO_EnableFunction(XM125_RX_PORT, XM125_RX_PIN, XM125_RX_PINMUX);

    /* I2C pins setup */
    GPIO_EnableFunction(XM125_SDA_PORT, XM125_SDA_PIN, XM125_SDA_PINMUX);
    GPIO_EnableFunction(XM125_SCL_PORT, XM125_SCL_PIN, XM125_SCL_PINMUX);

    SERCOM_I2C_SetupMaster(
        XM125_I2C_INSTANCE,
        8000000u,
        100000u
    );
    SERCOM_I2C_Enable(XM125_I2C_INSTANCE);
}

void XM125_Reset(void)
{
    XM125_SetResetLow();
    // TODO: add delay
    XM125_SetResetHigh();
}

void XM125_Wake(void)
{
    GPIO_PinWrite(XM125_WAKE_PORT, XM125_WAKE_PIN, HIGH);
}

void XM125_GoToSleep(void)
{
    GPIO_PinWrite(XM125_WAKE_PORT, XM125_WAKE_PIN, LOW);
}

void XM125_SetBootHigh(void)
{
    GPIO_PinWrite(XM125_BOOT_PORT, XM125_BOOT_PIN, HIGH);
}

void XM125_SetBootLow(void)
{
    GPIO_PinWrite(XM125_BOOT_PORT, XM125_BOOT_PIN, LOW);
}
