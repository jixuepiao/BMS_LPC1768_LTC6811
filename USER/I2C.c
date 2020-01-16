#include "I2C.h"

uint8_t *I2C0_pt;	//待发送的数据块指针
uint8_t I2C0_num;	//待发送的数据总数量
uint8_t I2C0_cnt;	//目前需要发送的数据序号
uint8_t I2C0_mode;	//模式 0x00：主发送		0x01：主接收
uint8_t I2C0_Done = 0;	//完成标志。0x00:发送或接收完成；0x01：未完成。
uint8_t I2C0_SA_num = ADDRESS_Byte;

void i2c0_init(void){
	LPC_SC->PCLKSEL0 |= 0x00008000;		//SystemCoreClock/2分频
	LPC_SC->PCONP |= 0x00000080;			//使能外设供电
	//引脚功能设置
	PIN_Configure (0,28,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_OPENDRAIN);
	PIN_Configure (0,27,PIN_FUNC_1,PIN_PINMODE_TRISTATE,PIN_PINMODE_OPENDRAIN);	
	PIN_ConfigureI2C0Pins (PIN_I2C_Normal_Mode, 1);
	//模式设置
	LPC_I2C0->I2SCLH = 250;		//设置波特率100kHz
	LPC_I2C0->I2SCLL = 250;		//设置波特率100kHz
	LPC_I2C0->I2CONSET = 0x00000040;	//使能I2C 
	//使能NVIC
	NVIC_EnableIRQ(I2C0_IRQn);
	
	WP_DIR_OUTPUT;
	WP_SET_0;
}

//	*pt:指向所要发送的数据数组；num：需要发送的数据个数
//注：发送数组中序号2和序号0的数不要相同 否则容易判断为读取
void i2c0_start(uint8_t *pt,uint8_t num){
	I2C0_pt = pt;
	I2C0_num = num;
	I2C0_cnt = 0;
	I2C0_Done = 1;	
	I2C0_SA_num = ADDRESS_Byte;
	if(((*(I2C0_pt+3) & 0x01) == 0x01) &&(*(I2C0_pt) == (*(I2C0_pt+3) & 0xFE)))		I2C0_mode = 0x01;	//判断为主接收模式
	else																																					I2C0_mode = 0x00;	//判断为主发送模式
	LPC_I2C0->I2CONSET = 0x00000020;	//置STA
}

uint8_t Get_I2C0_Done(void){
	return I2C0_Done;
}

void I2C0_IRQHandler(void){
//	if((LPC_I2C0->I2CONSET & 0x00000008) == 0x00000008){
  uint8_t StatValue;
  StatValue = LPC_I2C0->I2STAT & 0xF8;
		switch(StatValue){				
			case 0x08:	//已发送起始条件	发送地址和读写位
				LPC_I2C0->I2DAT = *I2C0_pt;
				LPC_I2C0->I2CONSET = 0x00000004;	//置AA
				LPC_I2C0->I2CONCLR = 0x00000020;	//清起始标志
				I2C0_cnt++;
				break;
			case 0x10:	//已发送重复起始条件
				LPC_I2C0->I2DAT = *(I2C0_pt++);
				LPC_I2C0->I2CONSET = 0x00000004;	//置AA
				I2C0_cnt++;
				break;
			case 0x18:	//之前状态为8 或10 表示已发送从机地址和写操作位，并接收了应答。即将发送第一个数据字节和接收ACK 位。
				LPC_I2C0->I2DAT = *(I2C0_pt++);
				LPC_I2C0->I2CONSET = 0x00000004;	//置AA
				I2C0_SA_num--;
				I2C0_cnt++;
				break;
			case 0x20:	//已发送从机地址和写操作位并接收了非应答。即将发送停止条件。
				LPC_I2C0->I2CONSET = 0x00000014;	//置STO和AA
				break;
			case 0x28:	//已发送数据并接收了ACK。如果发送的数据是最后一个数据字节则发送一个停止条件，否则发送下一个数据字节。
				if(I2C0_mode == 0x00){	//主发送模式
					if(I2C0_cnt > I2C0_num){
						LPC_I2C0->I2CONSET = 0x00000014;	//置STO和AA
						I2C0_Done = 0;
					}else{
						LPC_I2C0->I2DAT = *(I2C0_pt++);
						I2C0_cnt++;				
						LPC_I2C0->I2CONSET = 0x00000004;	//置AA
					}
				}else{	//主接收模式 发送重复起始条件
					if(I2C0_SA_num > 0){	//发送剩余地址位
						I2C0_SA_num --;
						LPC_I2C0->I2DAT = *(I2C0_pt++);
						I2C0_cnt++;				
						LPC_I2C0->I2CONSET = 0x00000004;	//置AA						
					}else{
						LPC_I2C0->I2CONSET = 0x00000020;	//置STA
					}
				}
				break;						
			case 0x30:	//已发送数据并接收到非应答。即将发送停止条件
				LPC_I2C0->I2CONSET = 0x00000014;	//置STO和AA
				break;			
			case 0x38:	//发生错误，重新发送
				LPC_I2C0->I2CONSET = 0x00000024;	//置STA和AA
				break;			
			case 0x40:	//前面的状态是08 或10 表示已发送从机地址和读操作位，并接收到ACK。将接收数据和返回ACK。
				LPC_I2C0->I2CONSET = 0x00000004;	//置AA
				break;			
			case 0x48:	//已发送从机地址和读操作位，并接收到非应答。将发送停止条件。
				LPC_I2C0->I2CONSET = 0x00000014;	//置STO和AA
				break;			
			case 0x50:	//已接收到数据，并返回ACK。将从I2DAT 读取数据。将接收其它的数据。如果这是最后一个数据字节，则返回非应答，否则返回ACK。
				if(I2C0_cnt > I2C0_num){
					LPC_I2C0->I2CONCLR = 0x00000004;	//清AA
					I2C0_Done = 0;
				}else{
					*(I2C0_pt++) = LPC_I2C0->I2DAT;
					I2C0_cnt++;					
					LPC_I2C0->I2CONSET = 0x00000004;	//置AA
				}
				break;			
			case 0x58:	//已接收到数据，已返回非应答。将从I2DAT 中读取数据和发送停止条件。
				*(I2C0_pt++) = LPC_I2C0->I2DAT;
				LPC_I2C0->I2CONSET = 0x00000014;	//置STO和AA
				break;			
		}
		LPC_I2C0->I2CONCLR = 0x00000008;	//清中断标志
}








