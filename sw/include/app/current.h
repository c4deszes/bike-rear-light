#ifndef APP_CURRENT_H_
#define APP_CURRENT_H_
#include <stdint.h>

void CURRENT_Init(void);

void CURRENT_Update100ms(void);

uint16_t CURRENT_GetCurrent(void);

#endif /* APP_CURRENT_H_ */
