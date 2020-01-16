#include "spi.h"

uint8_t *spi_sendpt;							//需要发送的数据块指针
uint16_t spi_sendnum;							//需要发送的数据个数
uint8_t spi_receivebuff[500];			//接收数据缓存区
uint16_t spi_receivenum;					//接收到有效数据的个数
uint16_t spi_readstep;						//下一次读取时的spi_receivebuff序号

void SPI_init(void){
	LPC_SC->PCLKSEL0 |= 0x00020000;		//SystemCoreClock/2分频
	LPC_SC->PCONP |= 0x00000100;			//使能外设供电
	//引脚功能设置
	PIN_Configure (0,15,PIN_FUNC_3 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );
//	PIN_Configure (0,16,PIN_FUNC_3 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	PIN_Configure (0,17,PIN_FUNC_3 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	PIN_Configure (0,18,PIN_FUNC_3 ,PIN_PINMODE_PULLUP ,PIN_PINMODE_NORMAL );	
	
	SSEL_DIR_OUTPUT ;
	SSEL_SET_1 ;
	//SPI配置
	LPC_SPI ->SPCR = 0x00B8;	//8位数据，CPHA=1，CPOL=1，主机模式，MSB在先，使能中断
	LPC_SPI ->SPCCR = 50;
	//使能NVIC
	NVIC_EnableIRQ (SPI_IRQn);
}

uint16_t SPI_6811_GetRxBufferSize(void){
	return spi_receivenum ;
}
void SPI_6811_PutArray(uint8_t buffer[],uint16_t byteCount){
	spi_sendpt = buffer;
	spi_sendnum = byteCount;
	spi_receivenum = 0;
	spi_readstep = 0;
	SSEL_SET_0 ;
	LPC_SPI ->SPDR = *spi_sendpt;
	spi_sendpt++;
	spi_sendnum--;
}

uint8_t SPI_6811_ReadRxData(void){
	if(spi_readstep < spi_receivenum){
		spi_readstep ++;
		if(spi_readstep == (spi_receivenum-1)){
			spi_readstep = 0;
			return spi_receivebuff[spi_receivenum-1];
		}else {
			return spi_receivebuff[spi_readstep];
		}
	}else return 0;
}

void SPI_IRQHandler(void){
	LPC_SPI ->SPINT = 0x01;
	if((LPC_SPI->SPSR & 0x80) == 0x80){
		spi_receivebuff[spi_receivenum] = LPC_SPI ->SPDR;		//缓存接收数据
		spi_receivenum++;
		if(spi_sendnum > 0){
			LPC_SPI ->SPDR = *spi_sendpt;
			spi_sendpt++;
			spi_sendnum--;
		}else{
			SSEL_SET_1;
			spi_readstep = 0;
		}
	}		
}






