#ifndef __SSP1_H__
#define __SSP1_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#define SSEL1_DIR_OUTPUT GPIO_SetDir(0,6,GPIO_DIR_OUTPUT)
#define SSEL1_SET_1 GPIO_PinWrite(0,6,1)
#define SSEL1_SET_0 GPIO_PinWrite(0,6,0)

extern uint8_t ssp1_receivebuff[100];			//接收数据缓存区
extern uint16_t ssp1_receivenum;						//接收到有效数据的个数
void SSP1_init(void);
uint16_t SSP1_GetRxBufferSize(void);
void SSP1_PutArray(uint8_t buffer[],uint16_t byteCount);
uint8_t SSP1_ReadRxData(void);




#endif


