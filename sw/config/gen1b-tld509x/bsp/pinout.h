#if !defined(BSP_PINOUT_H_)
#define BSP_PINOUT_H_

#include "hal/gpio.h"
#include "hal/sercom_usart.h"

// TODO: isn't great that we include it, since it could transitively end up in the app
#include "sam.h"

/** TLD509x LED driver IC pins */
#define TLD509x_PWMI_PORT PORT_GROUP_A
#define TLD509x_PWMI_PIN 16
#define TLD509x_PWMI_PINMUX MUX_PA16E_TCC2_WO0 MUX_PA16
#define TLD509x_PWMI_WO 0

#define TLD509x_ISET_PORT PORT_GROUP_A
#define TLD509x_ISET_PIN 2
#define TLD509x_ISET_PINMUX MUX_PA02B_DAC_VOUT

#define TLD509x_FAULT_PORT PORT_GROUP_A
#define TLD509x_FAULT_PIN 19
#define TLD509x_FAULT_PINMUX MUX_PA19A_EIC_EXTINT3

#define TLD509x_FREQ_PORT PORT_GROUP_A
#define TLD509x_FREQ_PIN 17
#define TLD509x_FREQ_PINMUX MUX_PA17E_TCC2_WO1
#define TLD509x_FREQ_WO 1

/** Board NTC (Negative Temperature Coefficient) thermistor pins */
#define BOARD_NTC_PORT PORT_GROUP_A
#define BOARD_NTC_PIN 5
#define BOARD_NTC_PINMUX MUX_PA05B_ADC0_AIN5
#define BOARD_NTC_ADC_INPUT ADC_INPUTCTRL_MUXPOS_PIN5_Val

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
