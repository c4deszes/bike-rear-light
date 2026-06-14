#include "bsp/board.h"

#include "hal/sysctrl.h"
#include "hal/gclk.h"
#include "hal/pm.h"

// TODO: remove direct register access and replace with HAL calls
#include "sam.h"

void BSP_ClockInitialize (void) {
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

    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_TCC2_TC3_Val, GCLK_GEN4);          // Scheduler
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_TCC0_TCC1_Val, GCLK_GEN4);         // Light PWM control
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_ADC_Val, GCLK_GEN4);               // ADC for analog measurements
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOMX_SLOW_Val, GCLK_GEN3);      // SERCOM slow clock for I2C and USART
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOM0_CORE_Val, GCLK_GEN3);      // LINE
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val, GCLK_GEN3);      // Radar I2C
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOM2_CORE_Val, GCLK_GEN3);      // Radar USART
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_SERCOM3_CORE_Val, GCLK_GEN3);      // Acc. SPI
    GCLK_SelectGenerator(GCLK_CLKCTRL_ID_DAC_Val, GCLK_GEN3);               // DAC for Light control

    PM_SelectCpuDiv(PM_CPUSEL_CPUDIV_DIV1);
    PM_SelectBusDiv(PM_APBASEL_APBADIV_DIV1,
                    PM_APBBSEL_APBBDIV_DIV1,
                    PM_APBCSEL_APBCDIV_DIV1);

    // TODO: replace with HAL calls
    /* Configure the APBC Bridge Clocks */
    PM_REGS->PM_APBCMASK = PM_APBCMASK_EVSYS_Msk |
                           PM_APBCMASK_ADC_Msk |
                           PM_APBCMASK_DAC_Msk |
                           PM_APBCMASK_TCC0_Msk |
                           PM_APBCMASK_TCC1_Msk |
                           PM_APBCMASK_TCC2_Msk |
                           PM_APBCMASK_SERCOM0_Msk |
                           PM_APBCMASK_SERCOM1_Msk |
                           PM_APBCMASK_SERCOM2_Msk |
                           PM_APBCMASK_SERCOM3_Msk;
}
