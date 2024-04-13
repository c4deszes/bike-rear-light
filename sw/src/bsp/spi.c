#include "bsp/spi.h"

#include "atsamd21e18a.h"
#include <stdlib.h>

static inline sercom_registers_t* get_peripheral(uint8_t sercom) {
    switch (sercom) {
        case SERCOM0: return SERCOM0_REGS;
        case SERCOM1: return SERCOM1_REGS;
        case SERCOM2: return SERCOM2_REGS;
        case SERCOM3: return SERCOM3_REGS;
    }
    // TODO: handle?
    return NULL;
}

void SERCOM_SPI_SetupMaster(uint8_t sercom, uint32_t clock_in, uint32_t datarate,
                            uint8_t mosi_pad, uint8_t miso_pad) {

    // steps
    // 1. setup ctrla values
        // ctrla.mode = SPI MASTER
        // dipo = receive pin (miso)
        // dopo = transmit pin (mosi)
    // 2. ctrlb values
        // chsize = 8bit

    // 3. set baud
    // 4. set address register

    sercom_registers_t* peripheral = get_peripheral(sercom);
    peripheral->SPIM.SERCOM_CTRLA =	SERCOM_SPIM_CTRLA_MODE_SPI_MASTER |
                                    SERCOM_SPIM_CTRLA_DOPO_PAD0 |
                                    SERCOM_SPIM_CTRLA_DIPO_PAD3 |
                                    SERCOM_SPIM_CTRLA_DORD_MSB;

    //Setting the CTRLB register
    peripheral->SPIM.SERCOM_CTRLB = SERCOM_SPIM_CTRLB_CHSIZE_8_BIT |
                                    SERCOM_SPIM_CTRLB_RXEN_Msk;

    peripheral->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_CPHA_TRAILING_EDGE |
                                    SERCOM_SPIM_CTRLA_CPOL_IDLE_HIGH;

    // 8MHz -> 1MHz
    // 8000000 / 2000000 = 4 - 1 -> 3
    peripheral->SPIM.SERCOM_BAUD = clock_in / (2 * datarate) - 1;
}

void SERCOM_SPI_Enable(uint8_t sercom) {
    sercom_registers_t* peripheral = get_peripheral(sercom);
    peripheral->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_ENABLE_Msk;

    while((peripheral->SPIM.SERCOM_SYNCBUSY & SERCOM_SPIM_SYNCBUSY_ENABLE_Msk) != 0)
    {
        //Waiting then enable bit from SYNCBUSY is equal to 0;
    }
}

uint8_t SERCOM_SPI_TransferByte(uint8_t sercom, uint8_t data) {
    sercom_registers_t* peripheral = get_peripheral(sercom);
    peripheral->SPIM.SERCOM_DATA = data;

    while((peripheral->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == 0)
    {
        // Waiting Complete Reception
    }

    return peripheral->SPIM.SERCOM_DATA;
}