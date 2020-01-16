#include "IO.h" 

uint32_t LED_Timer = 0;	//LED电量时钟计数 （ms）
uint8_t Light_Num = 0;	//需要点亮的LED数量

uint8_t PRESSED = 0;		//按键确认按下标识	bit0：KEY1
uint8_t PRESS_FLAG = 0;			//按键按下标识	bit0～bit4：KEY1～KEY5	bit8:有按键按下标识，

void IO_Init(void){
	SelfPWR_Ctrl_OUTPUT;
	DSG_DIR_OUTPUT;
	CHG_DIR_OUTPUT;
	HEAT_Ctrl_DIR_OUTPUT;
	PRE_Ctrl_DIR_OUTPUT;
	
	KEY_DIR_INPUT;
	SC_DIR_INPUT;
	DOC0_DIR_INPUT;
	DOC1_DIR_INPUT;	
	COC_DIR_INPUT;
	
	AlarmLed_DIR_OUTPUT;
	CapLed1_DIR_OUTPUT;
	CapLed2_DIR_OUTPUT;
	CapLed3_DIR_OUTPUT;
	CapLed4_DIR_OUTPUT;
	CapLed5_DIR_OUTPUT;
	
	SelfPWR_Ctrl_SET_1;
	DSG_SET_0;
	CHG_SET_0;
	HEAT_Ctrl_SET_0;
	
	AlarmLed_SET_1;
	CapLed1_SET_1;
	CapLed2_SET_1;
	CapLed3_SET_1;
	CapLed4_SET_1;
	CapLed5_SET_1;	

}
void KEYSCAN(void){
	if(PRESSED == 0){			
		if(KEY_Read == 0){
			PRESS_FLAG |= 0x01;
		}
		if((PRESS_FLAG>0)&&(KEY_Read==1)){
			PRESSED = PRESS_FLAG;
			PRESS_FLAG = 0;					
		}
	}
}

void LED_light(uint8_t num){
	switch(num){
		case 0:
			CapLed1_SET_1;
			CapLed2_SET_1;
			CapLed3_SET_1;
			CapLed4_SET_1;
			CapLed5_SET_1;
			break;
		case 1:
			CapLed1_SET_0;
			CapLed2_SET_1;
			CapLed3_SET_1;
			CapLed4_SET_1;
			CapLed5_SET_1;
			break;		
		case 2:
			CapLed1_SET_0;
			CapLed2_SET_0;
			CapLed3_SET_1;
			CapLed4_SET_1;
			CapLed5_SET_1;
			break;
		case 3:
			CapLed1_SET_0;
			CapLed2_SET_0;
			CapLed3_SET_0;
			CapLed4_SET_1;
			CapLed5_SET_1;
			break;
		case 4:
			CapLed1_SET_0;
			CapLed2_SET_0;
			CapLed3_SET_0;
			CapLed4_SET_0;
			CapLed5_SET_1;
			break;	
		case 5:
			CapLed1_SET_0;
			CapLed2_SET_0;
			CapLed3_SET_0;
			CapLed4_SET_0;
			CapLed5_SET_0;
			break;		
	}
}

void LED(void){
	if(Remain_CAP_Percent < LED1_Threshold){
		Light_Num = 0;
	}else if((Remain_CAP_Percent >= LED1_Threshold)&&(Remain_CAP_Percent < LED2_Threshold)){
		Light_Num = 1;
	}else if((Remain_CAP_Percent >= LED2_Threshold)&&(Remain_CAP_Percent < LED3_Threshold)){
		Light_Num = 2;
	}else if((Remain_CAP_Percent >= LED3_Threshold)&&(Remain_CAP_Percent < LED4_Threshold)){
		Light_Num = 3;
	}else if((Remain_CAP_Percent >= LED4_Threshold)&&(Remain_CAP_Percent < LED5_Threshold)){
		Light_Num = 4;
	}else if(Remain_CAP_Percent > LED5_Threshold){
		Light_Num = 5;
	}	
	
	if(((BAT_Work_Status&DSGing_bit)>0)&&((BAT_Work_Status&CHGing_bit)==0)&&((BAT_Work_Status&Heating_bit)==0)){	//放电中
		switch(LED_Timer){
			case 1:
				LED_light(5);
				break;
			case 150:
				LED_light(4);
				break;				
			case 300:
				LED_light(3);
				break;		
			case 450:
				LED_light(2);
				break;
			case 600:
				LED_light(1);
				break;				
			case 750:
				LED_light(0);
				break;			
			case 1000:
				LED_light(Light_Num);
				break;				
			case 2000:
				LED_light(0);
				break;
			case 5000:
				LED_Timer = 0;
				break;			
		}
	}
	if(((BAT_Work_Status&DSGing_bit)==0)&&((BAT_Work_Status&CHGing_bit)>0)&&((BAT_Work_Status&Heating_bit)==0)){	//充电中
		switch(LED_Timer){
			case 1:
				LED_light(1);
				break;
			case 150:
				LED_light(2);
				break;				
			case 300:
				LED_light(3);
				break;		
			case 450:
				LED_light(4);
				break;
			case 600:
				LED_light(5);
				break;				
			case 750:
				LED_light(0);
				break;			
			case 1000:
				LED_light(Light_Num);
				break;				
			case 2000:
				LED_light(0);
				break;
			case 5000:
				LED_Timer = 0;
				break;			
		}
	}
//	if(((BAT_Work_Status&DSGing_bit)==0)&&((BAT_Work_Status&CHGing_bit)>0)&&((BAT_Work_Status&Heating_bit)>0)){	//加热中
//		switch(LED_Timer){
//			case 0:
//				LED_light(Light_Num);
//				break;
//			case 1000:
//				LED_light(0);
//				break;				
//			case 2000:
//				LED_Timer = 0;
//				break;
//		}
//	}
	if(((BAT_Work_Status&DSGing_bit)==0)&&((BAT_Work_Status&CHGing_bit)==0)&&((BAT_Work_Status&Heating_bit)==0)){	//静置中
		LED_light(Light_Num);
		LED_Timer = 0;
	}
}

void SHUTDOWN(void){
	SelfPWR_Ctrl_SET_0;
}




/*
	END OF FILE
*/













