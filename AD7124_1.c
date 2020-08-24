#include "AD7124.h"



uint32_t AD7124_COUNT = 0;
uint8_t AD7124_DATA_BUFF[20];	//SPI口发送数据

uint8_t uartsendbuff[500];
uint32_t buffdata;

uint32_t AD1_CH_Volt[10];									//1mV		电压值
uint32_t AD1_CH_DATA[10];									//AD转换值
uint32_t AD1_CUM_DATA[10][20];						//取20组数据的平均值
uint8_t AD1_HEX_BUFF[20][4];							//AD转换原值
uint8_t AD1_CUM_Count = 0;								//测量累加计数，记20组
uint8_t	AD1_DataReady_Flag = 0;		//累计到20组数据后置该标志位

uint32_t AD0_CH_Volt[10];									//1mV		电压值
uint32_t AD0_CH_DATA[10];									//AD转换值
uint32_t AD0_CUM_DATA[10][20];						//取20组数据的平均值
uint8_t AD0_HEX_BUFF[20][4];							//AD转换原值
uint8_t AD0_CUM_Count = 0;								//测量累加计数，记20组
uint8_t	AD0_DataReady_Flag = 0;		//累计到20组数据后置该标志位


void AD7124_Send(uint8_t dev,uint16_t num){
	switch(dev){
		case 0:
			SSP0_PutArray(AD7124_DATA_BUFF,num);	//此函数必须为while循环等待发送完成方式，不能为中断方式
			break;
		case 1:
			SSP1_PutArray(AD7124_DATA_BUFF,num);	//此函数必须为while循环等待发送完成方式，不能为中断方式
			break;		
	}
}
uint32_t AD7124_CH_VoltGet(uint8_t ch,uint32_t data){
	uint32_t  buff;
	uint32_t i;
	if(ch == AD7124_DEV0){
		buff = (data>>4)*DEV0_ADC_REF;
		i = buff/0x000FFFFF;	
	}
	if(ch == AD7124_DEV1){
		buff = (data>>4)*DEV1_ADC_REF;
		i = buff/0x000FFFFF;
	}
	return i;	
}
uint16_t AD7124_Write_CHANNEL(uint8_t ch,uint8_t ainp,uint8_t ainm,uint8_t setup,uint8_t enable){
	uint16_t buff16;
	buff16 = 0;
	buff16 = 	CHANNEL_int|
						Enable(enable)|
						Setup(setup)|
						AINP(ainp)|
						AINM(ainm);
	AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Write)|RS(ch);
	AD7124_DATA_BUFF[1] = buff16>>8;	
	AD7124_DATA_BUFF[2] = buff16&0x00FF;	
	return 3;
//	SSP1_PutArray(AD7124_DATA_BUFF,3);
}

uint16_t AD7124_Write_CONFIGURATION(	uint8_t ch,
																	uint8_t bipolar,
																	uint8_t burnout,
																	uint8_t ref_bufp,
																	uint8_t ref_bufm,
																	uint8_t ain_bufp,
																	uint8_t ain_bufm,
																	uint8_t ref_sel,
																	uint8_t pga																
																){
	uint16_t buff16;
	buff16 = 0;
	buff16 = 	CONFIGURATION_int|
						Bipolar(bipolar)|
						Burnout(burnout)|
						REF_BUFP(ref_bufp)|
						REF_BUFM(ref_bufm)|
						AIN_BUFP(ain_bufp)|
						AIN_BUFM(ain_bufm)|						
						REF_SEL(ref_sel)|
						PGA(pga);
	AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Write)|RS(ch);
	AD7124_DATA_BUFF[1] = buff16>>8;	
	AD7124_DATA_BUFF[2] = buff16&0x00FF;
	return 3;
//	SSP1_PutArray(AD7124_DATA_BUFF,3);
}
																
uint16_t AD7124_Write_FILTER(	uint8_t ch,
													uint8_t filter_sel,
													uint8_t rej,
													uint8_t post,
													uint8_t single,
													uint16_t fs														
												){
	uint32_t buff32;
	buff32 = 0;
	buff32 = 	FILTER_int|
						FILTER_SEL(filter_sel)|
						REJ60(rej)|
						POST_FILTER(post)|
						SINGLE_CYCLE(single)|
						FS(fs);					
	AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Write)|RS(ch);
	AD7124_DATA_BUFF[1] = (buff32&0x00FF0000)>>16;	
	AD7124_DATA_BUFF[2] = (buff32&0x0000FF00)>>8;
	AD7124_DATA_BUFF[3] = buff32&0x000000FF;
	return 4;													
//	SSP1_PutArray(AD7124_DATA_BUFF,4);
}
void AD7124_Read_DATA(uint8_t dev,uint8_t ch){
	uint8_t i;
	uint32_t buf;
	if(dev == AD7124_DEV0){
		AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Read)|RS(Data);
		AD7124_DATA_BUFF[1] = 0xFF;	
		AD7124_DATA_BUFF[2] = 0xFF;
		AD7124_DATA_BUFF[3] = 0xFF;
		AD7124_DATA_BUFF[4] = 0xFF;
		AD7124_Send(AD7124_DEV0,5);	
		AD0_HEX_BUFF[ch][0] = ssp0_receivebuff[1];
		AD0_HEX_BUFF[ch][1] = ssp0_receivebuff[2];	
		AD0_HEX_BUFF[ch][2] = ssp0_receivebuff[3];
		AD0_HEX_BUFF[ch][3] = ssp0_receivebuff[4];
		buf = ssp0_receivebuff[1];
		buf <<= 8;
		buf += ssp0_receivebuff[2];
		buf <<= 8;
		buf += ssp0_receivebuff[3];

//		AD0_CH_DATA[ch] = buf;
		
		AD0_CUM_DATA[ch][AD0_CUM_Count] = buf;
		if(ch == CH_7){
			if(AD0_CUM_Count == 4){
				AD0_CUM_Count = 0;
				AD0_DataReady_Flag = 1;
			}else AD0_CUM_Count ++;
		}
		if(AD0_DataReady_Flag == 1){
			buf = 0;
			for(i=0;i<5;i++){
				buf +=  AD0_CUM_DATA[ch][i];
			}
			AD0_CH_DATA[ch] = buf/5;
		}	
	}
}

void AD7124_mission(void){
	uint16_t i;
	uint16_t buff16;
	switch(AD7124_COUNT){
		case 500:
			buff16 = 0;
			buff16 = 	ADC_Ctrl_int|
								DOUT_RDY_DEL(0)|
								CONT_READ(0)|
								DATA_STATUS(1)|
								CS_EN(1)|
								REF_EN(1)|
								POWER_MODE(MID_POWER)|
								MODE(MODE8)|
								CLK_SEL(SEL1);
			AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Write)|RS(ADC_Ctrl);
			AD7124_DATA_BUFF[1] = buff16>>8;	
			AD7124_DATA_BUFF[2] = buff16&0x00FF;
			AD7124_Send(AD7124_DEV0,3);
//			SSP1_PutArray(AD7124_DATA_BUFF,3);			
			break;		
		case 700:
			buff16 = 0;
			buff16 = 	ADC_Ctrl_int|
								DOUT_RDY_DEL(0)|
								CONT_READ(0)|
								DATA_STATUS(1)|
								CS_EN(1)|
								REF_EN(1)|
								POWER_MODE(MID_POWER)|
								MODE(MODE7)|
								CLK_SEL(SEL1);
			AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Write)|RS(ADC_Ctrl);
			AD7124_DATA_BUFF[1] = buff16>>8;	
			AD7124_DATA_BUFF[2] = buff16&0x00FF;	
			AD7124_Send(AD7124_DEV0,3);
//			SSP1_PutArray(AD7124_DATA_BUFF,3);			
			break;		
		case 900:
			buff16 = 0;
			buff16 = 	ADC_Ctrl_int|
								DOUT_RDY_DEL(0)|
								CONT_READ(0)|
								DATA_STATUS(1)|
								CS_EN(1)|
								REF_EN(1)|
								POWER_MODE(FULL_POWER)|
								MODE(CONTINUOUS)|
								CLK_SEL(SEL1);
			AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Write)|RS(ADC_Ctrl);
			AD7124_DATA_BUFF[1] = buff16>>8;	
			AD7124_DATA_BUFF[2] = buff16&0x00FF;
			AD7124_Send(AD7124_DEV0,3);	
//			SSP1_PutArray(AD7124_DATA_BUFF,3);
			break;
		case 905:
			AD7124_Send(AD7124_DEV0,AD7124_Write_CONFIGURATION(CONFIG_0,0,OFF,0,0,1,1,int_ref,G_1));	//配置CONFIG_0寄存器
			break;
		case 910:
			AD7124_Send(AD7124_DEV0,AD7124_Write_FILTER(FILTER_0,SINC4,0,POST2,0,120));	//配置FILTER_0寄存器
			break;
		case 915:
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_0,AIN0,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_1,AIN1,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_2,AIN2,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_3,AIN3,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_4,AIN4,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_5,AIN5,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_6,AIN6,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_7,AIN7,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			break;		
		case 1000:
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_0,AIN0,AVss,SEL_Setup0,AD7124_CHN_Enable));
			break;
		case 1100:
			AD7124_Read_DATA(AD7124_DEV0,CH_0);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_0,AIN0,AVss,SEL_Setup0,AD7124_CHN_Disenable));	
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_1,AIN1,AVss,SEL_Setup0,AD7124_CHN_Enable));				
			break;
		case 1200:
			AD7124_Read_DATA(AD7124_DEV0,CH_1);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_1,AIN1,AVss,SEL_Setup0,AD7124_CHN_Disenable));	
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_2,AIN2,AVss,SEL_Setup0,AD7124_CHN_Enable));			
			break;
		case 1300:
			AD7124_Read_DATA(AD7124_DEV0,CH_2);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_2,AIN2,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_3,AIN3,AVss,SEL_Setup0,AD7124_CHN_Enable));		
			break;
		case 1400:
			AD7124_Read_DATA(AD7124_DEV0,CH_3);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_3,AIN3,AVss,SEL_Setup0,AD7124_CHN_Disenable));
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_4,AIN4,AVss,SEL_Setup0,AD7124_CHN_Enable));	
			break;
		case 1500:
			AD7124_Read_DATA(AD7124_DEV0,CH_4);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_4,AIN4,AVss,SEL_Setup0,AD7124_CHN_Disenable));	
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_5,AIN5,AVss,SEL_Setup0,AD7124_CHN_Enable));		
			break;
		case 1600:
			AD7124_Read_DATA(AD7124_DEV0,CH_5);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_5,AIN5,AVss,SEL_Setup0,AD7124_CHN_Disenable));	
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_6,AIN6,AVss,SEL_Setup0,AD7124_CHN_Enable));		
			break;
		case 1700:
			AD7124_Read_DATA(AD7124_DEV0,CH_6);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_6,AIN6,AVss,SEL_Setup0,AD7124_CHN_Disenable));		
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_7,AIN7,AVss,SEL_Setup0,AD7124_CHN_Enable));		
			break;
		case 1800:
			AD7124_Read_DATA(AD7124_DEV0,CH_7);
			AD7124_Send(AD7124_DEV0,AD7124_Write_CHANNEL(CHANNEL_7,AIN7,AVss,SEL_Setup0,AD7124_CHN_Disenable));		
			break;
		case 1900:	
			for(i=0;i<=7;i++){
				AD0_CH_Volt[i] = AD7124_CH_VoltGet(AD7124_DEV0,AD0_CH_DATA[i]);
			}
			break;

		case 2000:	
			AD7124_COUNT = 950;
			break;
	}
}

void AD7124_UartSend(void){
	uint8_t i;
  uint16_t k=0;

	uartsendbuff[k] = 'A';
	uartsendbuff[k+1] = 'D';
	uartsendbuff[k+2] = '7';
	uartsendbuff[k+3] = '1';
	uartsendbuff[k+4] = '2';
	uartsendbuff[k+5] = '4';
	uartsendbuff[k+6] = '-';	
	uartsendbuff[k+7] = '0';	
	uartsendbuff[k+8] = '\r';	
	uartsendbuff[k+9] = '\n';	
	k = k+10;	
	for(i=0;i<=1;i++){
		uartsendbuff[k] = 'C';
		uartsendbuff[k+1] = 'H';
		uartsendbuff[k+2] = '0'+i/10;
		uartsendbuff[k+3] = '0'+i%10;
		uartsendbuff[k+4] = ' ';		
		k = k+5;		
		uartsendbuff[k] = 0x30+AD0_CH_Volt[i]/10000;
		uartsendbuff[k+1] = 0x30+(AD0_CH_Volt[i]%10000)/1000;
		uartsendbuff[k+2] = 0x30+(AD0_CH_Volt[i]%1000)/100;
		uartsendbuff[k+3] = 0x30+(AD0_CH_Volt[i]%100)/10;
		uartsendbuff[k+4] = 0x30+(AD0_CH_Volt[i]%10);
		k = k+5;			
		uartsendbuff[k] = '\r';
		uartsendbuff[k+1] = '\n';
		k = k+2;
	}	
	uart3_send(uartsendbuff, k);
}

/*
	END OF FILE
*/


