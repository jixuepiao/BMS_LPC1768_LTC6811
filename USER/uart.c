#include "uart.h"

uint8_t uart1_receivebuff[100];
uint16_t uart1_receivenum;
uint16_t uart1_receivednum;
uint8_t *uart1_receivept;
uint8_t uart1_receivestatus;	//0：关闭接收		1：接收中		2：接收完成
uint8_t uart1_received_OT = 0;		//接收超时计数，大于5判断接收超时
uint8_t uart1_sendbuff[100];
uint8_t *uart1_sendpt;
uint16_t uart1_sendnum;

uint8_t uart2_receivebuff[100];
uint16_t uart2_receivenum;
uint16_t uart2_receivednum;
uint8_t *uart2_receivept;
uint8_t uart2_receivestatus;	//0：关闭接收		1：接收中		2：接收完成
uint8_t uart2_received_OT = 0;		//接收超时计数，大于5判断接收超时
uint8_t uart2_sendbuff[100];
uint8_t *uart2_sendpt;
uint16_t uart2_sendnum;

uint8_t uart3_receivebuff[100];
uint16_t uart3_receivenum;			//需要接收数据个数
uint16_t uart3_receivednum;			//已经接收到数据的个数
uint8_t *uart3_receivept;				//接收数据存放数组指针
uint8_t uart3_receivestatus;	//0：关闭接收		1：接收中		2：接收完成
uint8_t uart3_received_OT = 0;		//接收超时计数，大于5判断接收超时
uint8_t uart3_sendbuff[100];
uint8_t *uart3_sendpt;
uint16_t uart3_sendnum;

//////////////////////////////uart1 start/////////////////////////////////
void uart1_init(void){
	LPC_SC->PCLKSEL0 |= 0x00000300;	//SystemCoreClock/8分频
	LPC_SC->PCONP |= 0x00000010;		//使能外设
	//引脚功能设置
	PIN_Configure(2,0,PIN_FUNC_2,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);
	PIN_Configure(2,1,PIN_FUNC_2,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);
	UART1_RT_DIR_OUTPUT;
	UART2_RT_DIR_OUTPUT;
	
	LPC_UART1->FCR = 0x01;
	LPC_UART1->LCR = 0x03;
	LPC_UART1->IER = 0x00000002;
	
	LPC_UART1->LCR = 0x83;	
	LPC_UART1->DLL = 0x04;
	LPC_UART1->DLM = 0x00;
	LPC_UART1->FDR = 0xD9;
	LPC_UART1->LCR = 0x03;
	
	NVIC_EnableIRQ(UART1_IRQn);
}

void uart1_send(uint8_t *p,uint16_t num){
	UART1_Tx;
	uart1_sendpt = p;
	uart1_sendnum = num;
	LPC_UART1->THR = *(uart1_sendpt++);
}

void uart1_receive(uint8_t *p,uint16_t num){
	UART1_Rx;
	uart1_receivept = p;
	uart1_receivenum = num;
	uart1_receivednum = 0;
	uart1_received_OT = 0;
	LPC_UART1->IER |= 0x00000001;
}

void UART1_IRQHandler(void){
	switch((LPC_UART1->IIR&0x0E)>>1){
		case 0x01:
			uart1_sendnum--;
			if(uart1_sendnum>0) LPC_UART1->THR = *(uart1_sendpt++);
			break;		
		case 0x02:
			*uart1_receivept = LPC_UART1->RBR;
			uart1_receivestatus = Receive_Doing;
			uart1_receivept++;
			uart1_receivednum++;
			uart1_receivenum--;
			uart1_received_OT = 0;
			if(uart1_receivenum == 0){
				LPC_UART1->IER &= 0xFFFFFFFE;
				uart1_receivestatus = Receive_Done;
			}
			break;
	}
}
///////////////////////////uart1 end///////////////////////////////////////

//////////////////////////////uart2 start/////////////////////////////////
void uart2_init(void){
	LPC_SC->PCLKSEL1 |= 0x00030000;	//SystemCoreClock/8分频
	LPC_SC->PCONP |= 0x01000000;		//使能外设
	//引脚功能设置
	PIN_Configure(0,10,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);
	PIN_Configure(0,11,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);
	
	LPC_UART2->FCR = 0x01;
	LPC_UART2->LCR = 0x03;
	LPC_UART2->IER = 0x00000002;
	
	LPC_UART2->LCR = 0x83;	
	LPC_UART2->DLL = 0x04;
	LPC_UART2->DLM = 0x00;
	LPC_UART2->FDR = 0xD9;
	LPC_UART2->LCR = 0x03;
	
	NVIC_EnableIRQ(UART2_IRQn);
}

void uart2_send(uint8_t *p,uint16_t num){
	UART2_Tx;
	uart2_sendpt = p;
	uart2_sendnum = num;
	LPC_UART2->THR = *(uart2_sendpt++);
}

void uart2_receive(uint8_t *p,uint16_t num){
	UART2_Rx;
	uart2_receivept = p;
	uart2_receivenum = num;
	uart2_receivednum = 0;
	uart2_received_OT = 0;
	LPC_UART2->IER |= 0x00000001;
}

void UART2_IRQHandler(void){
	switch((LPC_UART2->IIR&0x0E)>>1){
		case 0x01:
			uart2_sendnum--;
			if(uart2_sendnum>0) LPC_UART2->THR = *(uart2_sendpt++);
			break;		
		case 0x02:
			*uart2_receivept = LPC_UART2->RBR;
			uart2_receivestatus = Receive_Doing;
			uart2_receivept++;
			uart2_receivednum++;
			uart2_receivenum--;
			uart2_received_OT = 0;
			if(uart2_receivenum == 0){
				LPC_UART2->IER &= 0xFFFFFFFE;
				uart2_receivestatus = Receive_Done;
			}
			break;
	}
}
///////////////////////////uart2 end///////////////////////////////////////

//////////////////////////////uart3 start/////////////////////////////////
void uart3_init(void){
	LPC_SC->PCLKSEL1 |= 0x000C0000;	//SystemCoreClock/8分频
	LPC_SC->PCONP |= 0x02000000;		//使能外设
	//引脚功能设置
	PIN_Configure(4,28,PIN_FUNC_3,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);
	PIN_Configure(4,29,PIN_FUNC_3,PIN_PINMODE_TRISTATE,PIN_PINMODE_NORMAL);
	
	LPC_UART3->FCR = 0x01;
	LPC_UART3->LCR = 0x03;
	LPC_UART3->IER = 0x00000002;
	
	LPC_UART3->LCR = 0x83;	
	LPC_UART3->DLL = 0x04;
	LPC_UART3->DLM = 0x00;
	LPC_UART3->FDR = 0xD9;
	LPC_UART3->LCR = 0x03;
	
	NVIC_EnableIRQ(UART3_IRQn);
}

void uart3_send(uint8_t *p,uint16_t num){
	uart3_sendpt = p;
	uart3_sendnum = num;
	LPC_UART3->THR = *(uart3_sendpt++);
}

void uart3_receive(uint8_t *p,uint16_t num){
	uart3_receivept = p;
	uart3_receivenum = num;
	uart3_receivednum = 0;
	uart3_received_OT = 0;
	LPC_UART3->IER |= 0x00000001;
}

void UART3_IRQHandler(void){
	switch((LPC_UART3->IIR&0x0E)>>1){
		case 0x01:
			uart3_sendnum--;
			if(uart3_sendnum>0) LPC_UART3->THR = *(uart3_sendpt++);
			break;		
		case 0x02:
			*uart3_receivept = LPC_UART3->RBR;
			uart3_receivestatus = Receive_Doing;
			uart3_receivept++;
			uart3_receivednum++;
			uart3_receivenum--;
			uart3_received_OT = 0;
			if(uart3_receivenum == 0){
				LPC_UART3->IER &= 0xFFFFFFFE;
				uart3_receivestatus = Receive_Done;
			}
			break;
	}
}
///////////////////////////uart3 end///////////////////////////////////////
/*
	END OF FILE
*/

