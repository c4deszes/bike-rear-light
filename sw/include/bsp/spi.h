#if !defined(BSP_SPI_H_)
#define BSP_SPI_H_

#include <stdint.h>

#define SERCOM0 0
#define SERCOM1 1
#define SERCOM2 2
#define SERCOM3 3
#define SERCOM4 4
#define SERCOM5 5

#define SERCOM_USART_TX_PAD0 0
#define SERCOM_USART_TX_PAD2 1

#define SERCOM_USART_RX_PAD0 0
#define SERCOM_USART_RX_PAD1 1
#define SERCOM_USART_RX_PAD2 2
#define SERCOM_USART_RX_PAD3 3

void SERCOM_SPI_SetupMaster(uint8_t sercom, uint32_t clock_in, uint32_t datarate,
                            uint8_t mosi_pad, uint8_t miso_pad);

void SERCOM_SPI_Enable(uint8_t sercom);

uint8_t SERCOM_SPI_TransferByte(uint8_t sercom, uint8_t data);

#endif // BSP_SPI_H_
