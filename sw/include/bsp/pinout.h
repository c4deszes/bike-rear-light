#if !defined(BSP_PINOUT_H_)
#define BSP_PINOUT_H_

#include "hal/gpio.h"
#include "hal/sercom_usart.h"

// TODO: isn't great that we include it, since it could transitively end up in the app
#include "sam.h"

/** Tail light LED driver IC pins */
#define TLD509x_PWMI_PORT PORT_GROUP_A
#define TLD509x_PWMI_PIN 16
#define TLD509x_PWMI_PINMUX MUX_PA16E_TCC2_WO0 MUX_PA16
#define TLD509x_PWMI_WO 0

#define TLD509x_ISET_PORT PORT_GROUP_A
#define TLD509x_ISET_PIN 2
#define TLD509x_ISET_PINMUX MUX_PA02B_DAC_VOUT

// TODO: support, either interrupt or polling
#define TLD509x_FAULT_PORT PORT_GROUP_A
#define TLD509x_FAULT_PIN 19
#define TLD509x_FAULT_PINMUX MUX_PA19A_EIC_EXTINT3

// TODO: disabled for now, frequency set by resistor 4.7k -> Spread spectrum 188kHz
// if TCC CC[1] is used to trigger ADC sampling then this cannot be used, TCC0 WO7 is available but
// that's used by the scheduler (could be TCC1)
#define TLD509x_FREQ_PORT PORT_GROUP_A
#define TLD509x_FREQ_PIN 17
#define TLD509x_FREQ_PINMUX MUX_PA17E_TCC2_WO1
#define TLD509x_FREQ_WO 1

#define BOARD_NTC_PORT PORT_GROUP_A
#define BOARD_NTC_PIN 5
#define BOARD_NTC_PINMUX MUX_PA05B_ADC0_AIN5

/** BMA456 3-axis accelerometer pins */
#define BMA456_ACC1_INT_PORT PORT_GROUP_A
#define BMA456_ACC1_INT_PIN 14

#define BMA456_ACC2_INT_PORT PORT_GROUP_A
#define BMA456_ACC2_INT_PIN 15

#define BMA456_SPI_MISO_PORT PORT_GROUP_A
#define BMA456_SPI_MISO_PIN 19
#define BMA456_SPI_MISO_PINMUX MUX_PA19C_SERCOM1_PAD3

#define BMA456_SPI_SCK_PORT PORT_GROUP_A
#define BMA456_SPI_SCK_PIN 17
#define BMA456_SPI_SCK_PINMUX MUX_PA17C_SERCOM1_PAD1

#define BMA456_SPI_CS_PORT PORT_GROUP_A
#define BMA456_SPI_CS_PIN 18

#define BMA456_SPI_MOSI_PORT PORT_GROUP_A
#define BMA456_SPI_MOSI_PIN 16
#define BMA456_SPI_MOSI_PINMUX MUX_PA16C_SERCOM1_PAD0

/** UART pins */
#define UART_RX_PORT PORT_GROUP_A
#define UART_RX_PIN 25
#define UART_RX_PINMUX MUX_PA25C_SERCOM3_PAD3

#define UART_TX_PORT PORT_GROUP_A
#define UART_TX_PIN 24
#define UART_TX_PINMUX MUX_PA24C_SERCOM3_PAD2

#define UART_CS_PORT PORT_GROUP_A
#define UART_CS_PIN 22

#define UART_TXE_PORT PORT_GROUP_A
#define UART_TXE_PIN 23

#define UART_TX_PAD SERCOM_USART_TX_PAD2
#define UART_RX_PAD SERCOM_USART_RX_PAD3

#endif // BSP_PINOUT_H_
