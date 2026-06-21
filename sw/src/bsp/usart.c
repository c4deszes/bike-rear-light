#include "hal/sercom_usart.h"
#include "bsp/pinout.h"

#include "bsp/usart.h"

void USART_Init(uint32_t baudrate, ringbuffer8_t* tx_buffer, ringbuffer8_t* rx_buffer) {
    GPIO_EnableFunction(UART_TX_PORT, UART_TX_PIN, UART_TX_PINMUX);
    GPIO_EnableFunction(UART_RX_PORT, UART_RX_PIN, UART_RX_PINMUX);

    GPIO_SetupPinOutput(UART_CS_PORT, UART_CS_PIN, &GPIO_OUTPUT_DEFAULT_CONFIG);
    GPIO_PinWrite(UART_CS_PORT, UART_CS_PIN, HIGH);

    SERCOM_USART_SetupAsync(
        UART_SERCOM_INSTANCE,
        8000000u,
        baudrate,
        UART_TX_PAD,
        UART_RX_PAD,
        tx_buffer,
        rx_buffer
    );
}

void USART_Enable(void) {
    SERCOM_USART_Enable(UART_SERCOM_INSTANCE);
}

void USART_WriteData(uint8_t* data, const uint8_t size) {
    SERCOM_USART_WriteData(UART_SERCOM_INSTANCE, data, size);
}

uint16_t USART_Available(void) {
    return SERCOM_USART_Available(UART_SERCOM_INSTANCE);
}

uint8_t USART_Read(void) {
    return SERCOM_USART_Read(UART_SERCOM_INSTANCE);
}

void USART_FlushOutput(void) {
    SERCOM_USART_FlushOutput(UART_SERCOM_INSTANCE);
}

void USART_GoToSleep(void) {
    GPIO_PinWrite(UART_CS_PORT, UART_CS_PIN, LOW);
}

#if UART_SERCOM_INSTANCE == SERCOM0
void SERCOM0_Interrupt(void) {
    SERCOM_USART_InterruptHandler(UART_SERCOM_INSTANCE);
}
#elif UART_SERCOM_INSTANCE == SERCOM3
void SERCOM3_Interrupt(void) {
    SERCOM_USART_InterruptHandler(UART_SERCOM_INSTANCE);
}
#else
#error "Unhandled SERCOM instance for USART"
#endif
