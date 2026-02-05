#include "bsp/board.h"

#include "hal/sysctrl.h"
#include "hal/gclk.h"
#include "hal/pm.h"

#include "sam.h"

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
