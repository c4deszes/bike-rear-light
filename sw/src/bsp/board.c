#include "bsp/board.h"

#include "hal/sysctrl.h"
#include "hal/gclk.h"
#include "hal/pm.h"

#include "atsamd21e18a.h"

void BSP_ClockInitialize (void) {
    NVMCTRL_REGS->NVMCTRL_CTRLB |= NVMCTRL_CTRLB_RWS_HALF_Val ;

    SYSCTRL_EnableInternalOSC32K();

    GCLK_Reset();

    GCLK_ConfigureGenerator(GCLK_GEN1, GCLK_GENCTRL_SRC_OSC32K_Val, 0u);    // GCLK1 -> DFLL source
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_DFLL48_Val, GCLK_GEN1);
    SYSCTRL_InitializeDFLL(32768u, 48000000u);                              // DFLL -> ~48MHz
    GCLK_ConfigureGenerator(GCLK_GEN0, GCLK_GENCTRL_SRC_DFLL48M_Val, 0u);   // DFLL -> MCLK

    SYSCTRL_ConfigureOSC8M();

    GCLK_ConfigureGenerator(GCLK_GEN2, GCLK_GENCTRL_SRC_OSCULP32K_Val, 0u); // GCLK2 -> 32kHz (low power)
    GCLK_ConfigureGenerator(GCLK_GEN3, GCLK_GENCTRL_SRC_OSC8M_Val, 0u);     // GCLK3 -> 8MHz
    GCLK_ConfigureGenerator(GCLK_GEN4, GCLK_GENCTRL_SRC_OSC8M_Val, 8u);     // GCLK4 -> 1MHz
 
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_EIC_Val, GCLK_GEN3);
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_WDT_Val, GCLK_GEN3);

    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_TCC2_TC3_Val, GCLK_GEN4);      // Scheduler
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_TCC0_TCC1_Val, GCLK_GEN4);     // Light PWM control
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOM3_CORE_Val, GCLK_GEN3);  // LIN Trans.
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val, GCLK_GEN3);  // Acc. SPI

    PM_SelectCpuDiv(PM_CPUSEL_CPUDIV_DIV1);
    PM_SelectBusDiv(PM_APBASEL_APBADIV_DIV1,
                    PM_APBBSEL_APBBDIV_DIV1,
                    PM_APBCSEL_APBCDIV_DIV1);

    /* Configure the APBC Bridge Clocks */
        
    // APBA peripherals (eic, rtc, wdt, gclk, sysctrl, pm, pac0) are enabled on reset
    // PM_REGS->PM_APBAMASK = PM_APBAMASK_Msk;
    
    // APBB peripherals (usb, dmac, port, nvmctrl, dsu, pac1) are enabled on reset
    // TODO: maybe disable usb?
    // PM_REGS->PM_APBBMASK = PM_APBBMASK_Msk;

    PM_REGS->PM_APBCMASK = PM_APBCMASK_EVSYS_Msk |
                           //PM_APBCMASK_TCC0_Msk |
                           PM_APBCMASK_TCC1_Msk |
                           PM_APBCMASK_TCC2_Msk |
                           PM_APBCMASK_SERCOM3_Msk |
                           PM_APBCMASK_SERCOM1_Msk;
}
