#include "ssp1.h"

uint8_t *ssp1_sendpt;								//需要发送的数据块指针
uint16_t ssp1_sendnum;							//需要发送的数据个数
uint8_t ssp1_receivebuff[100];			//接收数据缓存区
uint16_t ssp1_receivenum;						//接收到有效数据的个数
uint16_t ssp1_readstep;							//下一次读取时的spi_receivebuff序号

void SSP1_init(void){
	LPC_SC->PCLKSEL0 |= 0x00200000;		//SystemCoreClock/2分频
	LPC_SC->PCONP |= 0x00000400;			//使能外设供电
	//引脚功能设置
	PIN_Configure (0,7,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );
//	PIN_Configure (0,6,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	PIN_Configure (0,8,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	PIN_Configure (0,9,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );
	
	SSEL1_DIR_OUTPUT;
	SSEL1_SET_1;
	//SPI配置
	LPC_SSP1 ->CR0 = 0x0AC7;	//8位数据，SPI,CPHA=1，CPOL=1，1MHz
	LPC_SSP1 ->CPSR = 5;			//1MHz
}

uint16_t SSP1_GetRxBufferSize(void){
	return ssp1_receivenum;
}
void SSP1_PutArray(uint8_t buffer[],uint16_t byteCount){
	ssp1_sendpt = buffer;
	ssp1_sendnum = byteCount;
	ssp1_receivenum = 0;
	ssp1_readstep = 0;
	LPC_SSP1 ->CR1 = 0x02;		//使能SSP1	
	SSEL1_SET_0;		
	while(ssp1_sendnum > 0){
		while((LPC_SSP1->SR & 0x02) == 0){};	// Wait Tx FIFO empty 
			LPC_SSP1 ->DR = *ssp1_sendpt;
			ssp1_sendpt++;
			ssp1_sendnum--;
		while((LPC_SSP1->SR& 0x01) == 0){}; 	// Send out all data in the FIFO		
		while((LPC_SSP1->SR& 0x04) == 0){}; 	// Wait for the Rx data
			ssp1_receivebuff[ssp1_receivenum] = LPC_SSP1 ->DR;		//缓存接收数据
			ssp1_receivenum++;		
	}
	SSEL1_SET_1;
	LPC_SSP1 ->CR1 = 0x00;		//关闭SSP1
}

uint8_t SSP1_ReadRxData(void){
	if(ssp1_readstep < ssp1_receivenum){
		ssp1_readstep ++;
		if(ssp1_readstep == (ssp1_receivenum-1)){
			ssp1_readstep = 0;
			return ssp1_receivebuff[ssp1_receivenum-1];
		}else {
			return ssp1_receivebuff[ssp1_readstep];
		}
	}else return 0;
}









