#ifndef APP_ROUTER_H
#define APP_ROUTER_H_

#include "app/brightness.h"

#define ROUTER_DIAG_MODE_ALL_OFF 0u
#define ROUTER_DIAG_MODE_ALL_ON 1u

void ROUTER_Init(void);

void ROUTER_Update10ms(void);

bool ROUTER_DiagRequest(brightness_output_t output, uint8_t mode, uint16_t brightness);


#endif // APP_ROUTER_H_
