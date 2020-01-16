#ifndef __AD_H__
#define __AD_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "uart.h"
#include "6811.h"

#define ADon_DIR_OUTPUT	GPIO_SetDir(3,25,GPIO_DIR_OUTPUT)

#define ADon_SET_1 GPIO_PinWrite(3,25,1)
#define ADon_SET_0 GPIO_PinWrite(3,25,0)

extern uint16_t LT68_NTC_Temp_MAX;		//主板NTC温度值最高值
extern uint16_t LT68_NTC_Temp_MIN;		//主板NTC温度值最低值
extern uint16_t NTC_Temp_MAX;		//温度值最高值
extern uint16_t NTC_Temp_MIN;		//温度值最低值
extern uint16_t ad_step;



void ad_init(void);
void ad_start(void);
void ad_read(void);
void ad_UartSend(void);
uint16_t NTC_Table_Lookup(uint32_t num);
void ad_datamath(void);
void ad(void);



#endif

/*
	END OF FILE
*/
