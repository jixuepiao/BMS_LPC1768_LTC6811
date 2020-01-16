#ifndef __SSP0_H__
#define __SSP0_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#define SSEL0_DIR_OUTPUT GPIO_SetDir(0,16,GPIO_DIR_OUTPUT)
#define SSEL0_SET_1 GPIO_PinWrite(0,16,1)
#define SSEL0_SET_0 GPIO_PinWrite(0,16,0)

extern uint8_t ssp0_receivebuff[100];

void SSP0_init(void);
uint16_t SSP0_GetRxBufferSize(void);
void SSP0_PutArray(uint8_t buffer[],uint16_t byteCount);
uint8_t SSP0_ReadRxData(void);

#endif


