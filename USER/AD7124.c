#include "AD7124.h"



uint32_t AD7124_COUNT = 0;
uint8_t AD7124_DATA_BUFF[20];

uint32_t CH_OffSet[20];	//1mV 初始化期间测量各模拟输入的零点电位基准
uint32_t CH_OffSet_MeasTimes = 0;

uint8_t uartsendbuff[500];
uint32_t buffdata;
uint32_t CH_Volt[20];	//1mV
uint32_t CH_DATA[10];
uint32_t CUM_DATA[10][20];
uint8_t HEX_BUFF[20][4];
uint8_t CUM_Count = 0;		//测量累加计数，记20组
uint8_t	AD7124_DataReady_Flag = 0;	//累计到20组数据后置该标志位


void AD7124_UartSend(void){
	uint8_t i;
  uint16_t k=0;

	uartsendbuff[k] = 'A';
	uartsendbuff[k+1] = 'D';
	uartsendbuff[k+2] = '7';
	uartsendbuff[k+3] = '1';
	uartsendbuff[k+4] = '2';
	uartsendbuff[k+5] = '4';	
	uartsendbuff[k+6] = '\r';	
	uartsendbuff[k+7] = '\n';	
	k = k+8;	
	for(i=0;i<=1;i++){
		uartsendbuff[k] = 'C';
		uartsendbuff[k+1] = 'H';
		uartsendbuff[k+2] = '0'+i/10;
		uartsendbuff[k+3] = '0'+i%10;
		uartsendbuff[k+4] = ' ';		
		k = k+5;		
		uartsendbuff[k] = 0x30+CH_Volt[i]/10000;
		uartsendbuff[k+1] = 0x30+(CH_Volt[i]%10000)/1000;
		uartsendbuff[k+2] = 0x30+(CH_Volt[i]%1000)/100;
		uartsendbuff[k+3] = 0x30+(CH_Volt[i]%100)/10;
		uartsendbuff[k+4] = 0x30+(CH_Volt[i]%10);
		k = k+5;			
		uartsendbuff[k] = '\r';
		uartsendbuff[k+1] = '\n';
		k = k+2;
	}
	uartsendbuff[k] = '\r';
	uartsendbuff[k+1] = '\n';
	k = k+2;	
	uart3_send(uartsendbuff, k);
	uart1_send(uartsendbuff, k);
}

uint32_t AD7124_CH_VoltGet(uint32_t data){
	uint32_t  buff;
	uint32_t i;
	buff = (data>>4)*ADC_REF;
	i = buff/0x000FFFFF;
	return i;
}
void AD7124_Write_CHANNEL(uint8_t ch,uint8_t ainp,uint8_t ainm,uint8_t setup,uint8_t enable){
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
	SSP1_PutArray(AD7124_DATA_BUFF,3);
}

void AD7124_Write_CONFIGURATION(	uint8_t ch,
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
	SSP1_PutArray(AD7124_DATA_BUFF,3);
}
																
void AD7124_Write_FILTER(	uint8_t ch,
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
	SSP1_PutArray(AD7124_DATA_BUFF,4);
}
void AD7124_Read_DATA(uint8_t ch){
	uint8_t i;
	uint32_t buf;
	AD7124_DATA_BUFF[0] = COMMS_int|WEN(WEN_L)|RW(RW_Read)|RS(Data);
	AD7124_DATA_BUFF[1] = 0xFF;	
	AD7124_DATA_BUFF[2] = 0xFF;
	AD7124_DATA_BUFF[3] = 0xFF;
	AD7124_DATA_BUFF[4] = 0xFF;
	SSP1_PutArray(AD7124_DATA_BUFF,5);	
	HEX_BUFF[ch][0] = ssp1_receivebuff[1];
	HEX_BUFF[ch][1] = ssp1_receivebuff[2];	
	HEX_BUFF[ch][2] = ssp1_receivebuff[3];
	HEX_BUFF[ch][3] = ssp1_receivebuff[4];
	buffdata = ssp1_receivebuff[1];
	buffdata <<= 8;
	buffdata += ssp1_receivebuff[2];
	buffdata <<= 8;
	buffdata += ssp1_receivebuff[3];		
	CUM_DATA[ch][CUM_Count] = buffdata;
	if(ch == CH_0){
		if(CUM_Count == 19){
			CUM_Count = 0;
			AD7124_DataReady_Flag = 1;
		}else								CUM_Count ++;
	}
	if(AD7124_DataReady_Flag == 1){
		buf = 0;
		for(i=0;i<20;i++){
			buf +=  CUM_DATA[ch][i];
		}
		CH_DATA[ch] = buf/20;
	}
//	uart3_send(HEX_BUFF[ch],4);
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
			SSP1_PutArray(AD7124_DATA_BUFF,3);			
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
			SSP1_PutArray(AD7124_DATA_BUFF,3);			
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
			SSP1_PutArray(AD7124_DATA_BUFF,3);
			AD7124_Write_CONFIGURATION(CONFIG_0,0,OFF,0,0,1,1,int_ref,G_1);							//配置CONFIG_0寄存器
			AD7124_Write_FILTER(FILTER_0,SINC4,0,POST2,0,60);														//配置FILTER_0寄存器
			AD7124_Write_CHANNEL(CHANNEL_0,		AIN0,							AVss,							0,0);	//配置CHANNEL0寄存器	
			break;		
		case 1000:
			AD7124_Write_CHANNEL(CHANNEL_0,		AIN0,							AVss,							0,1);	//配置CHANNEL0寄存器			
			break;
		case 1025:
			AD7124_Read_DATA(CH_0);
			AD7124_Write_CHANNEL(CHANNEL_0,		AIN0,							AVss,							0,0);	//配置CHANNEL0寄存器
			for(i=0;i<=7;i++){
				CH_Volt[i] = AD7124_CH_VoltGet(CH_DATA[i]);				
				if(CH_OffSet_MeasTimes<15) CH_OffSet[i] += AD7124_CH_VoltGet(CH_DATA[i]);
				CH_OffSet_MeasTimes++;
				if(CH_OffSet_MeasTimes==16) CH_OffSet[i] = CH_OffSet[i]/15;
			}
			
			AD7124_COUNT = 999;
			break;
	}
	AD7124_COUNT ++;
}


/*
	END OF FILE
*/


