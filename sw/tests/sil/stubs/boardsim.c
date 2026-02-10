#include "bsp/board.h"
#include "bsp/line_usart.h"

#include "line_transport.h"

void BSP_ClockInitialize (void) {
    // Do nothing, clock is not used in simulation
}

void LINE_USART_Init(void) {
    
}

void LINE_USART_Receive(void) {

}

void LINE_Transport_Init(uint8_t channel, LINE_Transport_Inst_t* inst)
{
    // Do nothing, transport is not used in simulation
}

void USART_GoToSleep(void) {
    // Do nothing, sleep is not simulated
}
