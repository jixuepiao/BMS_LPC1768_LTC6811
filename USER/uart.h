#ifndef __UART_H__
#define __UART_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"

#define Receive_OT_Threshold	10		//ms 接收超时时间
#define Receive_Stop					0		
#define Receive_Doing					1
#define Receive_Done					2

#define UART1_RT_DIR_OUTPUT		GPIO_SetDir(2,2,GPIO_DIR_OUTPUT)
#define UART2_RT_DIR_OUTPUT		GPIO_SetDir(2,13,GPIO_DIR_OUTPUT)
#define UART1_RT_H						GPIO_PinWrite(2,2,1)
#define UART1_RT_L						GPIO_PinWrite(2,2,0)
#define UART2_RT_H						GPIO_PinWrite(2,13,1)
#define UART2_RT_L						GPIO_PinWrite(2,13,0)

#define UART1_Rx						UART1_RT_L
#define UART1_Tx						UART1_RT_H
#define UART2_Rx						UART2_RT_L
#define UART2_Tx						UART2_RT_H

extern uint8_t uart1_receivebuff[100];
extern uint16_t uart1_receivenum;
extern uint16_t uart1_receivednum;
extern uint8_t *uart1_receivept;
extern uint8_t uart1_receivestatus;	//0：关闭接收		1：接收中		2：接收完成
extern uint8_t uart1_received_OT;
extern uint8_t uart1_sendbuff[100];
extern uint8_t *uart1_sendpt;
extern uint16_t uart1_sendnum;

extern uint8_t uart2_receivebuff[100];
extern uint16_t uart2_receivenum;
extern uint16_t uart2_receivednum;
extern uint8_t *uart2_receivept;
extern uint8_t uart2_receivestatus;	//0：关闭接收		1：接收中		2：接收完成
extern uint8_t uart2_received_OT;
extern uint8_t uart2_sendbuff[100];
extern uint8_t *uart2_sendpt;
extern uint16_t uart2_sendnum;

extern uint8_t uart3_receivebuff[100];
extern uint16_t uart3_receivenum;
extern uint16_t uart3_receivednum;
extern uint8_t *uart3_receivept;
extern uint8_t uart3_receivestatus;	//0：关闭接收		1：接收中		2：接收完成
extern uint8_t uart3_received_OT;
extern uint8_t uart3_sendbuff[100];
extern uint8_t *uart3_sendpt;
extern uint16_t uart3_sendnum;

void uart1_init(void);
void uart1_send(uint8_t *p,uint16_t num);
void uart1_receive(uint8_t *p,uint16_t num);

void uart2_init(void);
void uart2_send(uint8_t *p,uint16_t num);
void uart2_receive(uint8_t *p,uint16_t num);

void uart3_init(void);
void uart3_send(uint8_t *p,uint16_t num);
void uart3_receive(uint8_t *p,uint16_t num);

#endif



