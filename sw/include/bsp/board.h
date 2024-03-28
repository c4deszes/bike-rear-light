#if !defined(BSP_BOARD_H_)
#define BSP_BOARD_H_

#include "hal/eic.h"

//extern eic_configuration bsp_eic_config;

void BSP_Initialize(void);

void BSP_ClockInitialize(void);

#endif // BSP_BOARD_H_
