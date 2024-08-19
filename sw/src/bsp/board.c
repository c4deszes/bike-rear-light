#include "bsp/board.h"

#include "hal/sysctrl.h"
#include "hal/gclk.h"
#include "hal/pm.h"

#include "sam.h"

static void OSC48M_Init() {
    uint32_t calibValue = (uint32_t)(((*(uint64_t*)0x00806020UL) >> 19 ) & 0x3fffffUL);
    OSCCTRL_REGS->OSCCTRL_CAL48M = calibValue;


    /* Selection of the Division Value */
    OSCCTRL_REGS->OSCCTRL_OSC48MDIV = (uint8_t)OSCCTRL_OSC48MDIV_DIV(0UL);

    while((OSCCTRL_REGS->OSCCTRL_OSC48MSYNCBUSY & OSCCTRL_OSC48MSYNCBUSY_OSC48MDIV_Msk) == OSCCTRL_OSC48MSYNCBUSY_OSC48MDIV_Msk)
    {
        /* Waiting for the synchronization */
    }

    while((OSCCTRL_REGS->OSCCTRL_STATUS & OSCCTRL_STATUS_OSC48MRDY_Msk) != OSCCTRL_STATUS_OSC48MRDY_Msk)
    {
        /* Waiting for the OSC48M Ready state */
    }
    OSCCTRL_REGS->OSCCTRL_OSC48MCTRL |= OSCCTRL_OSC48MCTRL_ONDEMAND_Msk;
}

static void OSC32K_Init() {
    OSC32KCTRL_REGS->OSC32KCTRL_OSC32K = 0x0UL;

    OSC32KCTRL_REGS->OSC32KCTRL_RTCCTRL = OSC32KCTRL_RTCCTRL_RTCSEL(0UL);
}

static void GCLK_Select(uint8_t peripheral, uint8_t generator) {
    GCLK_REGS->GCLK_PCHCTRL[peripheral] = GCLK_PCHCTRL_GEN(generator)  | GCLK_PCHCTRL_CHEN_Msk;

    while ((GCLK_REGS->GCLK_PCHCTRL[peripheral] & GCLK_PCHCTRL_CHEN_Msk) != GCLK_PCHCTRL_CHEN_Msk)
    {
        /* Wait for synchronization */
    }
}

void BSP_ClockInitialize (void) {
    NVMCTRL_REGS->NVMCTRL_CTRLB |= NVMCTRL_CTRLB_RWS_HALF_Val ;

    //SYSCTRL_EnableInternalOSC32K();

    OSC48M_Init();

    OSC32K_Init();

    //GCLK_Reset();

    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1) | GCLK_GENCTRL_SRC(GCLK_SOURCE_OSC48M) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* wait for the Generator 0 synchronization */
    }

    GCLK_REGS->GCLK_GENCTRL[1] = GCLK_GENCTRL_DIV(48) | GCLK_GENCTRL_SRC(GCLK_SOURCE_OSC48M) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL1_Msk) == GCLK_SYNCBUSY_GENCTRL1_Msk)
    {
        /* wait for the Generator 1 synchronization */
    }

    GCLK_REGS->GCLK_GENCTRL[2] = GCLK_GENCTRL_DIV(6) | GCLK_GENCTRL_SRC(GCLK_SOURCE_OSC48M) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL2_Msk) == GCLK_SYNCBUSY_GENCTRL2_Msk)
    {
        /* wait for the Generator 2 synchronization */
    }

    GCLK_Select(SERCOM3_GCLK_ID_CORE, 2);
    GCLK_Select(TCC0_GCLK_ID, 1);
    GCLK_Select(TCC2_GCLK_ID, 1);
    GCLK_Select(ADC0_GCLK_ID, 2);
    GCLK_Select(ADC1_GCLK_ID, 2);
    GCLK_Select(DAC_GCLK_ID, 2);
    GCLK_Select(EVSYS_GCLK_ID_0, 2);

    MCLK_REGS->MCLK_APBCMASK =  MCLK_APBCMASK_ADC0_Msk |
                                MCLK_APBCMASK_ADC1_Msk |
                                MCLK_APBCMASK_DAC_Msk |
                                MCLK_APBCMASK_SERCOM3_Msk |
                                MCLK_APBCMASK_TCC0_Msk |
                                MCLK_APBCMASK_TCC2_Msk;
                                MCLK_APBCMASK_EVSYS_Msk;
}
