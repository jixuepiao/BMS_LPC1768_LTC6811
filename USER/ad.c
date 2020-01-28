#include "ad.h"

uint32_t NTC_SubTable[14][10] = {	{0x2E9E9,0x2BF20,0x29810,0x27100,0x249F0,0x23A21,0x21728,0x1FFB8,0x1E078,0x1CCF0},
																	{0x1B738,0x1A5E0,0x19640,0x17700,0x16760,0x154F5,0x14820,0x14050,0x130B0,0x11940},
																	{0x10AA4,0x101F8,0xF5A0,0xE9F2,0xDEEE,0xD202,0xCA94,0xC12A,0xB84C,0xAFDC},
																	{0xA68C,0xA050,0x9920,0x924A,0x8BCE,0x84F6,0x7FD0,0x7A44,0x74F4,0x6FF4},
																	{0x6AD4,0x669E,0x6248,0x5E2E,0x5A46,0x565C,0x52F8,0x4F92,0x4C54,0x493E},
																	{0x463B,0x4380,0x40CE,0x3E3A,0x3BCE,0x3972,0x373C,0x3516,0x330E,0x311A},
																	{0x2F3E,0x2D78,0x2BC0,0x2A1C,0x288C,0x2710,0x259E,0x2440,0x22EC,0x21AC},
																	{0x2077,0x1F4A,0x1E28,0x1D24,0x1C16,0x1B1D,0x1A2C,0x1946,0x186A,0x178E},
																	{0x16C1,0x15FE,0x1540,0x148C,0x13D8,0x132F,0x128E,0x11F8,0x1162,0x10CC},
																	{0x103E,0x0FBE,0x0F3C,0x0EC4,0x0E4C,0x0DDD,0x0D66,0x0CF8,0x0C94,0x0C30},
																	{0x0BCC,0x0B72,0x0B18,0x0AC8,0x0A6E,0x0A1D,0x09CE,0x0988,0x0942,0x08FC},
																	{0x08B4,0x087A,0x0834,0x07F8,0x07BC,0x0785,0x074E,0x071C,0x06EA,0x06B8},
																	{0x0684,0x0654,0x062C,0x05FA,0x05D2,0x05AB,0x0582,0x055A,0x053C,0x0514},
																	{0x04F3,0x04CE,0x04B0,0x0492,0x0474,0x0455,0x0438,0x041A,0x03FC,0x03E8}	};

uint32_t NTC_UpTable[15] = {0x2E9E9,0x1B738,0x10AA4,0xA68C,0x6AD4,0x463B,0x2F3E,0x2077,0x16C1,0x103E,0x0BCC,0x08B4,0x0684,0x04F3,0x03CE};

uint32_t databuff[8];				//AD测得的数值缓存
uint32_t Ave_databuff[8];		//AD测得的累加值及平均值
uint32_t NTC_Reg[7];				//计算得的NTC电阻值
uint16_t NTC_Temp[7];				//温度值 分度值为1度，负温度用补码
uint16_t ad_step = 0;

uint16_t NTC_Temp_MAX = 0;		//主板NTC温度值最高值
uint16_t NTC_Temp_MIN = 0;		//主板NTC温度值最低值

uint16_t LT68_NTC_Temp_MAX = 0;		//主板NTC温度值最高值
uint16_t LT68_NTC_Temp_MIN = 0;		//主板NTC温度值最低值

uint8_t ADUartSendData[100];                 //UART发送数据寄存器

void ad_init(void){
	LPC_SC->PCLKSEL0 |= 0x01000000;		//SystemCoreClock不分频
	LPC_SC->PCONP |= 0x00001000;			//使能外设供电
	//引脚功能设置
	PIN_Configure (0,2,PIN_FUNC_2,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);		//AD0.7
	PIN_Configure (0,3,PIN_FUNC_2,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);		//AD0.6
	PIN_Configure (0,23,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);	//AD0.0
	PIN_Configure (0,24,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);	//AD0.1
	PIN_Configure (0,25,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);	//AD0.2
	PIN_Configure (0,26,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);	//AD0.3
	PIN_Configure (1,30,PIN_FUNC_3,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);	//AD0.4
	PIN_Configure (1,31,PIN_FUNC_3,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);	//AD0.5	
	//模式设置
	LPC_ADC->ADCR = 0x002109FF;
	LPC_ADC->ADINTEN = 0;
}

void ad_start(void){
	ADon_SET_0;
	LPC_ADC->ADCR |= 0x00000000;
}

void ad_max_min(void){
	uint8_t i;
	NTC_Temp_MAX = NTC_Temp[0];
	NTC_Temp_MIN = NTC_Temp[0];
	for(i=1;i<7;i++){
		if((NTC_Temp[i]+100) > (NTC_Temp_MAX+100))	NTC_Temp_MAX = NTC_Temp[i];
		if((NTC_Temp[i]+100) < (NTC_Temp_MIN+100))	NTC_Temp_MIN = NTC_Temp[i];
	}
}

void ad_read(void){
	databuff[0] = (LPC_ADC->ADDR0 & 0x0000FFF0) >>4;
	databuff[1] = (LPC_ADC->ADDR1 & 0x0000FFF0) >>4;
	databuff[2] = (LPC_ADC->ADDR2 & 0x0000FFF0) >>4;
	databuff[3] = (LPC_ADC->ADDR3 & 0x0000FFF0) >>4;
	databuff[4] = (LPC_ADC->ADDR4 & 0x0000FFF0) >>4;
	databuff[5] = (LPC_ADC->ADDR5 & 0x0000FFF0) >>4;
	databuff[6] = (LPC_ADC->ADDR6 & 0x0000FFF0) >>4;
	databuff[7] = (LPC_ADC->ADDR7 & 0x0000FFF0) >>4;
}
uint16_t NTC_Table_Lookup(uint32_t num){	//返回值为温度值 100为0度
	uint16_t i,NTC_Table_num,NTC_SubTable_num;
	if(num > NTC_UpTable[0]){					//判断温度小于-40度
		return 20;									
	}else if(num < NTC_UpTable[14]){	//判断温度大于90度
		return 210;									
	}else{														//在温度范围之内
		for(i=0;i<15;i++){
			if((num < NTC_UpTable[i]) && (num > NTC_UpTable[i+1])){
				NTC_Table_num = i;
			}
		}
		for(i=0;i<10;i++){
			if((num < NTC_SubTable[NTC_Table_num][i]) && (num > NTC_SubTable[NTC_Table_num][i+1])){
				NTC_SubTable_num = i;
			}		
		}
		return NTC_Table_num*10+NTC_SubTable_num+60;
	}
}

void ad_datamath(void){
	uint8_t i;
	//计算MCU的AD测温值
	for(i=0;i<7;i++){
		NTC_Reg[i] = (10000*Ave_databuff[i+1])/(2*Ave_databuff[0]-Ave_databuff[i+1]);
	}
	for(i=0;i<7;i++){
		NTC_Temp[i] = NTC_Table_Lookup(NTC_Reg[i]);
	}
}


void ad(void){
	uint8_t i;
	switch(ad_step){
		case 1:
		case 2:
		case 3:
		case 4:			
		case 5:
		case 6:
		case 7:
		case 8:			
		case 9:
		case 10:
			ad_read();
			for(i=0;i<8;i++){
				Ave_databuff[i] += databuff[i];
			}
			break;			
		case 11:
			for(i=0;i<8;i++){
				Ave_databuff[i] = Ave_databuff[i]/10;
			}
			ad_datamath();
			ad_max_min();
			break;	
		case 100:
			
			
			ad_step = 0;
			break;	
	}
	ad_step++;
}
void ad_UartSend(void){
	uint8_t i,j;
	uint16_t k=0;
	
	ADUartSendData[k] = 'N';
	ADUartSendData[k+1] = 'T';
	ADUartSendData[k+2] = 'C';
	ADUartSendData[k+3] = '\r';
	ADUartSendData[k+4] = '\n';	
	k = k+5;	
	ADUartSendData[k] = 'T';
	ADUartSendData[k+1] = 'E';
	ADUartSendData[k+2] = 'M';
	ADUartSendData[k+3] = 'P';
	ADUartSendData[k+4] = ' ';	
	k = k+5;
	for(j=0;j<7;j++){
		if(NTC_Temp[j] == 0xA0){
			ADUartSendData[k] = 'O';
			ADUartSendData[k+1] = 'T';
			ADUartSendData[k+2] = ' ';
			k = k+3;
		}else if(NTC_Temp[j] == 0xB0){
			ADUartSendData[k] = 'U';
			ADUartSendData[k+1] = 'T';
			ADUartSendData[k+2] = ' ';
			k = k+3;
		}else if(NTC_Temp[j] < 0xA0){
			ADUartSendData[k] = 0x30+NTC_Temp[j]/100;
			ADUartSendData[k+1] = 0x30+(NTC_Temp[j]%100)/10;
			ADUartSendData[k+2] = 0x30+(NTC_Temp[j]%10);
			ADUartSendData[k+3] = ' ';
			k = k+4;
		}else if((NTC_Temp[j]>0xB0)&&(NTC_Temp[j] <= 0xFF)){
			i = 0xFF-NTC_Temp[j]+1;
			ADUartSendData[k] = '-';
			k = k+1;
			ADUartSendData[k] = 0x30+i/100;
			ADUartSendData[k+1] = 0x30+(i%100)/10;
			ADUartSendData[k+2] = 0x30+(i%10);
			ADUartSendData[k+3] = ' ';
			k = k+4;
		}
	}
	ADUartSendData[k] = '\r';
	ADUartSendData[k+1] = '\n';
	k = k+2;
	
	ADUartSendData[k] = 'M';
	ADUartSendData[k+1] = 'A';
	ADUartSendData[k+2] = 'X';
	ADUartSendData[k+3] = ' ';
	k = k+4;
	if(NTC_Temp_MAX == 0xA0){
		ADUartSendData[k] = 'O';
		ADUartSendData[k+1] = 'T';
		ADUartSendData[k+2] = ' ';
		k = k+3;
	}else if(NTC_Temp_MAX == 0xB0){
		ADUartSendData[k] = 'U';
		ADUartSendData[k+1] = 'T';
		ADUartSendData[k+2] = ' ';
		k = k+3;
	}else if(NTC_Temp_MAX < 0xA0){
		ADUartSendData[k] = 0x30+NTC_Temp_MAX/100;
		ADUartSendData[k+1] = 0x30+(NTC_Temp_MAX%100)/10;
		ADUartSendData[k+2] = 0x30+(NTC_Temp_MAX%10);
		ADUartSendData[k+3] = ' ';
		k = k+4;
	}else if((NTC_Temp_MAX>0xB0)&&(NTC_Temp_MAX <= 0xFF)){
		i = 0xFF-NTC_Temp_MAX+1;
		ADUartSendData[k] = '-';
		k = k+1;
		ADUartSendData[k] = 0x30+i/100;
		ADUartSendData[k+1] = 0x30+(i%100)/10;
		ADUartSendData[k+2] = 0x30+(i%10);
		ADUartSendData[k+3] = ' ';
		k = k+4;
	}
	ADUartSendData[k] = '\r';
	ADUartSendData[k+1] = '\n';	
	k = k+2;	
	
	ADUartSendData[k] = 'M';
	ADUartSendData[k+1] = 'I';
	ADUartSendData[k+2] = 'N';
	ADUartSendData[k+3] = ' ';
	k = k+4;
	if(NTC_Temp_MIN == 0xA0){
		ADUartSendData[k] = 'O';
		ADUartSendData[k+1] = 'T';
		ADUartSendData[k+2] = ' ';
		k = k+3;
	}else if(NTC_Temp_MIN == 0xB0){
		ADUartSendData[k] = 'U';
		ADUartSendData[k+1] = 'T';
		ADUartSendData[k+2] = ' ';
		k = k+3;
	}else if(NTC_Temp_MIN < 0xA0){
		ADUartSendData[k] = 0x30+NTC_Temp_MIN/100;
		ADUartSendData[k+1] = 0x30+(NTC_Temp_MIN%100)/10;
		ADUartSendData[k+2] = 0x30+(NTC_Temp_MIN%10);
		ADUartSendData[k+3] = ' ';
		k = k+4;
	}else if((NTC_Temp_MIN>0xB0)&&(NTC_Temp_MIN <= 0xFF)){
		i = 0xFF-NTC_Temp_MIN+1;
		ADUartSendData[k] = '-';
		k = k+1;
		ADUartSendData[k] = 0x30+i/100;
		ADUartSendData[k+1] = 0x30+(i%100)/10;
		ADUartSendData[k+2] = 0x30+(i%10);
		ADUartSendData[k+3] = ' ';
		k = k+4;
	}
	ADUartSendData[k] = '\r';
	ADUartSendData[k+1] = '\n';	
	k = k+2;
	ADUartSendData[k] = '\r';
	ADUartSendData[k+1] = '\n';	
	k = k+2;
	
	ADUartSendData[k] = '\r';
	ADUartSendData[k+1] = '\n';
	k = k+2;

	uart3_send(ADUartSendData, k);
}

/*
	END OF FILE
*/

