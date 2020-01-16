#ifndef __SPI_H__
#define __SPI_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#define SSEL_DIR_OUTPUT GPIO_SetDir(0,16,GPIO_DIR_OUTPUT)
#define SSEL_SET_1 GPIO_PinWrite(0,16,1)
#define SSEL_SET_0 GPIO_PinWrite(0,16,0)

extern uint8_t spi_receivebuff[500];
void SPI_init(void);
uint16_t SPI_6811_GetRxBufferSize(void);
void SPI_6811_PutArray(uint8_t buffer[],uint16_t byteCount);
uint8_t SPI_6811_ReadRxData(void);


#endif


