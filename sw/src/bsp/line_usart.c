#include "common/ringbuffer.h"

// Board support package
#include "bsp/usart.h"

#include "line_protocol.h"
#include "line_api.h"

RINGBUFFER_8(COMM_UsartBufferTx, 128);
RINGBUFFER_8(COMM_UsartBufferRx, 128);

void LINE_USART_Init(void) {
    USART_Initialize(LT_BicycleNetwork_BAUDRATE, &COMM_UsartBufferTx, &COMM_UsartBufferRx);
    USART_Enable();
}

void LINE_USART_Receive(void) {
    uint8_t length = USART_Available();
    while (length > 0) {
        uint8_t data = USART_Read();
        LINE_Transport_Receive(LT_BicycleNetwork_CHANNEL, data);
        length--;
    }

    LINE_Transport_Update(LT_BicycleNetwork_CHANNEL, 1);
}

void LINE_Transport_WriteResponse(uint8_t channel, uint8_t size, uint8_t* payload, uint8_t checksum) {
    uint8_t fix = 69;
    USART_WriteData(&size, sizeof(uint8_t));
    // TODO: fix for skipped 3rd byte
    USART_WriteData(payload, 1);
    USART_WriteData(&fix, 1);
    USART_WriteData(payload+1, size-1);
    USART_WriteData(&checksum, sizeof(uint8_t));
    USART_FlushOutput();
}
