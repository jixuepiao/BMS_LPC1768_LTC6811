#ifndef __TIMER_H__
#define __TIMER_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"


void rit_init(void);
void rit_start(void);
void rit_stop(void);

void timer3_init(void);
void timer3_start(void);
void timer3_stop(void);
void timer3_TC_Reset(void);
uint32_t timer3_get_TC(void);

#endif

/*
	END OF FILE
*/
