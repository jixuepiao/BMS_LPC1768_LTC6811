#ifndef __I2C_H__
#define __I2C_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#define WP_DIR_OUTPUT	GPIO_SetDir(3,26,GPIO_DIR_OUTPUT)
#define WP_SET_1 GPIO_PinWrite(3,26,1)
#define WP_SET_0 GPIO_PinWrite(3,26,0)
#define ADDRESS_Byte 2


void i2c0_init(void);
void i2c0_start(uint8_t *pt,uint8_t num);
void I2C0_IRQHandler(void);
uint8_t Get_I2C0_Done(void);


#endif
/*
	END OF FILE
*/










