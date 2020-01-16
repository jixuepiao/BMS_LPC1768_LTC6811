#ifndef __IO_H__
#define __IO_H__
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "BAT.h" 

#define KEY_DIR_INPUT 						GPIO_SetDir(0,5,GPIO_DIR_INPUT)
#define KEY_Read 									GPIO_PinRead (0,5)

#define SelfPWR_Ctrl_OUTPUT				GPIO_SetDir(1,15,GPIO_DIR_OUTPUT)		//selfpwr
#define DSG_DIR_OUTPUT						GPIO_SetDir(1,16,GPIO_DIR_OUTPUT)
#define CHG_DIR_OUTPUT						GPIO_SetDir(1,17,GPIO_DIR_OUTPUT)
#define HEAT_Ctrl_DIR_OUTPUT			GPIO_SetDir(1,14,GPIO_DIR_OUTPUT)
#define PRE_Ctrl_DIR_OUTPUT				GPIO_SetDir(1,10,GPIO_DIR_OUTPUT)

#define SelfPWR_Ctrl_SET_1 				GPIO_PinWrite(1,15,1)
#define DSG_SET_1 								GPIO_PinWrite(1,16,1)
#define CHG_SET_1 								GPIO_PinWrite(1,17,1)
#define HEAT_Ctrl_SET_1 					GPIO_PinWrite(1,14,1)
#define PRE_Ctrl_SET_1 						GPIO_PinWrite(1,10,1)

#define SelfPWR_Ctrl_SET_0 				GPIO_PinWrite(1,15,0)
#define DSG_SET_0 								GPIO_PinWrite(1,16,0)
#define CHG_SET_0 								GPIO_PinWrite(1,17,0)
#define HEAT_Ctrl_SET_0 					GPIO_PinWrite(1,14,0)
#define PRE_Ctrl_SET_0 						GPIO_PinWrite(1,10,0)

#define SC_DIR_INPUT							GPIO_SetDir(2,5,GPIO_DIR_INPUT)
#define DOC0_DIR_INPUT						GPIO_SetDir(2,3,GPIO_DIR_INPUT)
#define DOC1_DIR_INPUT						GPIO_SetDir(2,4,GPIO_DIR_INPUT)
#define COC_DIR_INPUT							GPIO_SetDir(2,2,GPIO_DIR_INPUT)
#define SC_Read 									GPIO_PinRead (2,5)
#define DOC0_Read 								GPIO_PinRead (2,3)
#define DOC1_Read 								GPIO_PinRead (2,4)
#define COC_Read 									GPIO_PinRead (2,2)


#define AlarmLed_DIR_OUTPUT				GPIO_SetDir(1,24,GPIO_DIR_OUTPUT)
#define CapLed1_DIR_OUTPUT				GPIO_SetDir(1,25,GPIO_DIR_OUTPUT)
#define CapLed2_DIR_OUTPUT				GPIO_SetDir(1,26,GPIO_DIR_OUTPUT)
#define CapLed3_DIR_OUTPUT				GPIO_SetDir(1,27,GPIO_DIR_OUTPUT)
#define CapLed4_DIR_OUTPUT				GPIO_SetDir(1,28,GPIO_DIR_OUTPUT)
#define CapLed5_DIR_OUTPUT				GPIO_SetDir(1,29,GPIO_DIR_OUTPUT)

#define AlarmLed_SET_1						GPIO_PinWrite(1,24,1)
#define CapLed1_SET_1							GPIO_PinWrite(1,25,1)
#define CapLed2_SET_1							GPIO_PinWrite(1,26,1)
#define CapLed3_SET_1							GPIO_PinWrite(1,27,1)
#define CapLed4_SET_1							GPIO_PinWrite(1,28,1)
#define CapLed5_SET_1							GPIO_PinWrite(1,29,1)

#define AlarmLed_SET_0						GPIO_PinWrite(1,24,0)
#define CapLed1_SET_0							GPIO_PinWrite(1,25,0)
#define CapLed2_SET_0							GPIO_PinWrite(1,26,0)
#define CapLed3_SET_0							GPIO_PinWrite(1,27,0)
#define CapLed4_SET_0							GPIO_PinWrite(1,28,0)
#define CapLed5_SET_0							GPIO_PinWrite(1,29,0)

#define LED1_Threshold						10	//容量百分比大于此值点亮，小于此值熄灭
#define LED2_Threshold						30	//容量百分比大于此值点亮，小于此值熄灭
#define LED3_Threshold						50	//容量百分比大于此值点亮，小于此值熄灭
#define LED4_Threshold						70	//容量百分比大于此值点亮，小于此值熄灭
#define LED5_Threshold						90	//容量百分比大于此值点亮，小于此值熄灭

#define Light_AlarmLED						AlarmLed_SET_0
#define UnLight_AlarmLED					AlarmLed_SET_1

#define Heat_ON						HEAT_Ctrl_SET_1
#define Heat_OFF					HEAT_Ctrl_SET_0

#define DSG_ON						DSG_SET_1
#define DSG_OFF						DSG_SET_0

#define CHG_ON						CHG_SET_1
#define CHG_OFF						CHG_SET_0

extern uint32_t LED_Timer;
extern uint8_t PRESSED;		
extern uint8_t PRESS_FLAG;

void IO_Init(void);
void LED_light(uint8_t num);
void SHUTDOWN(void);
void LED(void);
void KEYSCAN(void);
#endif


/*
	END OF FILE
*/







