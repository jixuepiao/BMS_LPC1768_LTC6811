#include "ssp0.h"

uint8_t *ssp0_sendpt;								//需要发送的数据块指针
uint16_t ssp0_sendnum;							//需要发送的数据个数
uint8_t ssp0_receivebuff[100];			//接收数据缓存区
uint16_t ssp0_receivenum;						//接收到有效数据的个数
uint16_t ssp0_readstep;							//下一次读取时的spi_receivebuff序号

void SSP0_init(void){
	LPC_SC->PCLKSEL1 |= 0x00000800;		//SystemCoreClock/2分频
	LPC_SC->PCONP |= 0x00200000;			//使能外设供电
	//引脚功能设置
	PIN_Configure (0,15,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );
//	PIN_Configure (0,16,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	PIN_Configure (0,17,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	PIN_Configure (0,18,PIN_FUNC_2 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	
	SSEL0_DIR_OUTPUT;
	SSEL0_SET_1;
	//SPI配置
	LPC_SSP0 ->CR0 = 0x0AC7;	//8位数据，SPI,CPHA=1，CPOL=1，1MHz
	LPC_SSP0 ->CPSR = 5;			//1MHz
}

uint16_t SSP0_GetRxBufferSize(void){
	return ssp0_receivenum;
}
void SSP0_PutArray(uint8_t buffer[],uint16_t byteCount){
	ssp0_sendpt = buffer;
	ssp0_sendnum = byteCount;
	ssp0_receivenum = 0;
	ssp0_readstep = 0;
	LPC_SSP0 ->CR1 = 0x02;		//使能SSP1	
	SSEL0_SET_0;		
	while(ssp0_sendnum > 0){
		while((LPC_SSP0->SR & 0x02) == 0){};	// Wait Tx FIFO empty 
			LPC_SSP0 ->DR = *ssp0_sendpt;
			ssp0_sendpt++;
			ssp0_sendnum--;
		while((LPC_SSP0->SR& 0x01) == 0){}; 	// Send out all data in the FIFO		
		while((LPC_SSP0->SR& 0x04) == 0){}; 	// Wait for the Rx data
			ssp0_receivebuff[ssp0_receivenum] = LPC_SSP0 ->DR;		//缓存接收数据
			ssp0_receivenum++;		
	}
	SSEL0_SET_1;
	LPC_SSP0 ->CR1 = 0x00;		//关闭SSP0
}

uint8_t SSP0_ReadRxData(void){
	if(ssp0_readstep < ssp0_receivenum){
		ssp0_readstep ++;
		if(ssp0_readstep == (ssp0_receivenum-1)){
			ssp0_readstep = 0;
			return ssp0_receivebuff[ssp0_receivenum-1];
		}else {
			return ssp0_receivebuff[ssp0_readstep];
		}
	}else return 0;
}









