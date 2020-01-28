/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/


#include "6811.h"

uint32_t LT68_NTC_SubTable[14][10] = {	{0x2E9E9,0x2BF20,0x29810,0x27100,0x249F0,0x23A21,0x21728,0x1FFB8,0x1E078,0x1CCF0},
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

uint32_t LT68_NTC_UpTable[15] = {0x2E9E9,0x1B738,0x10AA4,0xA68C,0x6AD4,0x463B,0x2F3E,0x2077,0x16C1,0x103E,0x0BCC,0x08B4,0x0684,0x04F3,0x03CE};

uint32_t LTC68_Data_Ready = 0;	//每完成一次LTC68芯片检测数据更新 该变量置1

void (*LTC6811_Send)(uint8_t buffer[],uint16_t byteCount) = SPI_6811_PutArray;
uint8_t *LTC6811_receivebuff = spi_receivebuff;

//void (*LTC6811_Send)(uint8_t buffer[],uint16_t byteCount) = SSP0_PutArray;
//uint8_t *LTC6811_receivebuff = ssp0_receivebuff;

CMD_STRUCT CMD_WRCFGA;
CMD_STRUCT CMD_RDCFGA;
CMD_STRUCT CMD_RDCVA;
CMD_STRUCT CMD_RDCVB;
CMD_STRUCT CMD_RDCVC;
CMD_STRUCT CMD_RDCVD;
CMD_STRUCT CMD_RDAUXA;
CMD_STRUCT CMD_RDAUXB;
CMD_STRUCT CMD_RDSTATA;
CMD_STRUCT CMD_RDSTATB;

CMD_STRUCT CMD_ADCV;
CMD_STRUCT CMD_ADOW_PU;
CMD_STRUCT CMD_ADOW_PD;
CMD_STRUCT CMD_CVST;
CMD_STRUCT CMD_ADOL;
CMD_STRUCT CMD_ADAX;
CMD_STRUCT CMD_ADAXD;
CMD_STRUCT CMD_AXST;
CMD_STRUCT CMD_ADSTAT;
CMD_STRUCT CMD_ADSTATD;
CMD_STRUCT CMD_STATST;
CMD_STRUCT CMD_ADCVAX;
CMD_STRUCT CMD_ADCVSC;
CMD_STRUCT CMD_CLRCELL;
CMD_STRUCT CMD_CLRAUX;
CMD_STRUCT CMD_CLRSTAT;

LTC6811_STRUCT DEVICE[LTC6811_DeviceNUM];                         //设备数据结构体

uint32_t SYSTimer_num=0;                  //同SYSTimer_ms_Counter，为毫秒时钟计数
uint32_t Dec_Time = 0;

uint8_t UartSendData[2000];                 //UART发送数据寄存器
uint8_t SendData[500];                     //SPI发送数据寄存器
uint8_t ReciveData[500];                   //SPI接收数据寄存器
uint16_t pec15Table[256];
uint16_t CRC15_POLY = 0x4599;

uint16_t LTC6811_Mission_StepCounter = 0; //LTC6811的任务计时，0.5ms增加一次
uint8_t LTC6811_Init_Status = 0x00;          //芯片初始化检测状态 0x80：未通过    0x01：通过     0x00:自检中
uint8_t repeat = 0;                       //步骤重复次数
uint16_t pecdata;
uint32_t Measure_Num = 0;									//测量次数，自检完成后(LTC6811_Init_Status == 0x01)时，每完成一次测量加1
uint32_t CUM_Num = 0;											//测量时，本次测量数据存放在累加数据暂存数组的位号
uint32_t CUM_BUFF[15];										//计算测量值平均值时，累加暂存在该处


uint16_t Cell_Select[LTC6811_DeviceNUM];	// 

uint16_t LT68_NTC_Table_Lookup(uint32_t num){	//返回值为温度值 100为0度
	uint16_t i,NTC_Table_num,NTC_SubTable_num;
	if(num > LT68_NTC_UpTable[0]){					//判断温度小于-40度
		return 20;									
	}else if(num < LT68_NTC_UpTable[14]){	//判断温度大于90度
		return 210;									
	}else{														//在温度范围之内
		for(i=0;i<15;i++){
			if((num < LT68_NTC_UpTable[i]) && (num > LT68_NTC_UpTable[i+1])){
				NTC_Table_num = i;
			}
		}
		for(i=0;i<10;i++){
			if((num < LT68_NTC_SubTable[NTC_Table_num][i]) && (num > LT68_NTC_SubTable[NTC_Table_num][i+1])){
				NTC_SubTable_num = i;
			}		
		}
		return NTC_Table_num*10+NTC_SubTable_num+60;
	}
}

void init_PEC15_Table(void){
    uint16_t remainder,bit,i;    
    for (i = 0; i < 256; i++){
        remainder = i << 7;
        for (bit = 8; bit > 0; bit--){
            if (remainder & 0x4000){
                remainder = ((remainder << 1));
                remainder = (remainder ^ CRC15_POLY);
            }else{
                remainder = ((remainder << 1));
            }
        }
        pec15Table[i] = remainder&0xFFFF;
    }
}
uint16_t pec15(uint8_t *data , uint16_t len){
    uint16_t remainder,address;
    uint16_t i;
    remainder = 16;//PEC seed
    for (i = 0; i < len; i++){
        address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
        remainder = (remainder << 8 ) ^ pec15Table[address];
    }
    return (remainder*2);//The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}
void CMD_init(uint16_t cmd,CMD_STRUCT *data){
    uint8_t i[2];
    uint16_t j;
    (*data).STRU.CMD[0] = cmd>>8; 
    (*data).STRU.CMD[1] = cmd&0x00FF;
    i[0] = cmd>>8;
    i[1] = cmd&0x00FF;
    j = pec15(i,2);
    (*data).STRU.CMD_PEC[0] = j>>8;
    (*data).STRU.CMD_PEC[1] = j&0x00FF;
}
void LTC6811_CMD_init(void){
    CMD_init(LTC6811_WRCFGA,&CMD_WRCFGA);
    CMD_init(LTC6811_RDCFGA,&CMD_RDCFGA);
    CMD_init(LTC6811_RDCVA,&CMD_RDCVA);
    CMD_init(LTC6811_RDCVB,&CMD_RDCVB);
    CMD_init(LTC6811_RDCVC,&CMD_RDCVC);
    CMD_init(LTC6811_RDCVD,&CMD_RDCVD);
    CMD_init(LTC6811_RDAUXA,&CMD_RDAUXA);
    CMD_init(LTC6811_RDAUXB,&CMD_RDAUXB);
    CMD_init(LTC6811_RDSTATA,&CMD_RDSTATA);
    CMD_init(LTC6811_RDSTATB,&CMD_RDSTATB);
    CMD_init(LTC6811_ADCV_7kHz_ALLCell,&CMD_ADCV);
    CMD_init(LTC6811_ADOW_7kHz_PU_ALLCell,&CMD_ADOW_PU);
    CMD_init(LTC6811_ADOW_7kHz_PD_ALLCell,&CMD_ADOW_PD);   
    CMD_init(LTC6811_CVST_7kHz_ST10,&CMD_CVST);
    CMD_init(LTC6811_ADOL,&CMD_ADOL);    
    CMD_init(LTC6811_ADOL_7kHz,&CMD_ADAX);
    CMD_init(LTC6811_ADAXD_7kHz_ALLCHG,&CMD_ADAXD);
    CMD_init(LTC6811_AXST_7kHz_ST10,&CMD_AXST);
    CMD_init(LTC6811_ADSTATD_7kHz_ALLCHST,&CMD_ADSTATD);
    CMD_init(LTC6811_STATST_7kHz_ST10,&CMD_STATST);
    CMD_init(LTC6811_ADCVAX_7kHz,&CMD_ADCVAX);
    CMD_init(LTC6811_ADCVSC_7kHz,&CMD_ADCVSC);    
    CMD_init(LTC6811_CLRCELL,&CMD_CLRCELL);
    CMD_init(LTC6811_CLRAUX,&CMD_CLRAUX);
    CMD_init(LTC6811_CLRSTAT,&CMD_CLRSTAT);
}
void LTC6811_Sendcmd(CMD_STRUCT *P){ //发送命令
    SendData[0] = (*P).DATA[0];
    SendData[1] = (*P).DATA[1];
    SendData[2] = (*P).DATA[2];
    SendData[3] = (*P).DATA[3];
    LTC6811_Send(SendData, 4);
}
void LTC6811_Readcmd(CMD_STRUCT *P){
    uint8_t i,j;
    SendData[0] = (*P).DATA[0];
    SendData[1] = (*P).DATA[1];
    SendData[2] = (*P).DATA[2];
    SendData[3] = (*P).DATA[3];
    for(i=0;i<LTC6811_DeviceNUM;i++){
        for(j=0;j<8;j++){
            SendData[i*8+j+4] = 0xFF;
        }
    }   
    LTC6811_Send(SendData, LTC6811_DeviceNUM*8+4);
}
void LTC6811_Writecmd(CMD_STRUCT *P){
    SendData[0] = (*P).DATA[0];
    SendData[1] = (*P).DATA[1];
    SendData[2] = (*P).DATA[2];
    SendData[3] = (*P).DATA[3];   
    LTC6811_Send(SendData, LTC6811_DeviceNUM*8+4);
}
void LTC6811_WeekIdle(void){
	uint8_t i;
	for(i=0;i<LTC6811_DeviceNUM;i++){
		SendData[i*4+0] = 0xAA;
		SendData[i*4+1] = 0xAA;
		SendData[i*4+2] = 0xAA;
		SendData[i*4+3] = 0xAA;	
	}   
	LTC6811_Send(SendData, 4*LTC6811_DeviceNUM);
}
uint8_t LTC6811_MoveData(uint8_t P[],uint8_t DeviceNum){
	uint8_t i[8];
	i[0] = LTC6811_receivebuff[4+(LTC6811_DeviceNUM-1-DeviceNum)*8];
	i[1] = LTC6811_receivebuff[5+(LTC6811_DeviceNUM-1-DeviceNum)*8];
	i[2] = LTC6811_receivebuff[6+(LTC6811_DeviceNUM-1-DeviceNum)*8];
	i[3] = LTC6811_receivebuff[7+(LTC6811_DeviceNUM-1-DeviceNum)*8];
	i[4] = LTC6811_receivebuff[8+(LTC6811_DeviceNUM-1-DeviceNum)*8];
	i[5] = LTC6811_receivebuff[9+(LTC6811_DeviceNUM-1-DeviceNum)*8];		
	pecdata = pec15(i,6);
	i[6] = pecdata>>8;
	i[7] = pecdata&0x00FF;
	if((i[6]==LTC6811_receivebuff[10+(LTC6811_DeviceNUM-1-DeviceNum)*8])&&(i[7]==LTC6811_receivebuff[11+(LTC6811_DeviceNUM-1-DeviceNum)*8])){
		P[0] = i[0];
		P[1] = i[1];
		P[2] = i[2];
		P[3] = i[3];
		P[4] = i[4];
		P[5] = i[5];
		return 1;
	}else return 0;
}
void CellVolt_Max_Min(void){	
	uint8_t i,j;
	uint16_t Max,Min,Delta,Max_num,Min_num;

	Max = DEVICE[0].CellVolt[0];
	Min = DEVICE[0].CellVolt[0];
	Max_num = (LTC6811_DeviceNUM-1)*12;
	Min_num = (LTC6811_DeviceNUM-1)*12;
	for(i=0;i<LTC6811_DeviceNUM;i++){
		for(j=0;j<12;j++){
			if((DEVICE[i].Cell_Select&(0x0001<<j))>0){	//判断该串是否使用 是使用状态
				if(DEVICE[i].CellVolt[j] > Max){
					Max = DEVICE[i].CellVolt[j];
					Max_num = (LTC6811_DeviceNUM - 1 - i)*12 + j;
				}
				if(DEVICE[i].CellVolt[j] < Min){
					Min = DEVICE[i].CellVolt[j];
					Min_num = (LTC6811_DeviceNUM - 1 - i)*12 + j;
				}
			}
		}
	}
	Delta = Max -Min;
	for(i=0;i<LTC6811_DeviceNUM;i++){
		DEVICE[i].CellVolt_Max = Max;
		DEVICE[i].CellVolt_Min = Min;
		DEVICE[i].CellVolt_DELTA = Delta;
		DEVICE[i].MAX_Cell_NUM = Max_num;
		DEVICE[i].MIN_Cell_NUM = Min_num;		
	}
} 
void Balance(void){
	uint16_t i,j,k;
	for(i=0;i<LTC6811_DeviceNUM;i++){
		DEVICE[i].DEC_Flag &= 0xF000;    //将低位清零
	}
	for(i=0;i<LTC6811_DeviceNUM;i++){
		for(j=0;j<12;j++){
			k = 1<<j;
			if((DEVICE[i].Cell_Select&(0x0001<<j))>0){	//判断该串是否使用 是使用状态
				if(DEVICE[i].CellVolt[j] > (DEVICE[i].CellVolt_Min+LTC6811_BalanceThreshold)) DEVICE[i].DEC_Flag |= k;
			}
		}
	}
}
void LTC6811_UartSend(void){
    uint8_t i,j;
    uint16_t k=0;
	
		UartSendData[k] = 'L';
		UartSendData[k+1] = 'T';
		UartSendData[k+2] = 'C';
		UartSendData[k+3] = '6';
		UartSendData[k+4] = '8';
		UartSendData[k+5] = '1';	
		UartSendData[k+6] = '1';	
		UartSendData[k+7] = '\r';
		UartSendData[k+8] = '\n';	
		k = k+9;	
    for(i=0;i<LTC6811_DeviceNUM;i++){
        UartSendData[k] = 'C';
        UartSendData[k+1] = 'V';
        UartSendData[k+2] = ' ';
        k = k+3;
        for(j=0;j<12;j++){
            UartSendData[k] = 0x30+DEVICE[i].CellVolt[j]/10000;
            UartSendData[k+1] = 0x30+(DEVICE[i].CellVolt[j]%10000)/1000;
            UartSendData[k+2] = 0x30+(DEVICE[i].CellVolt[j]%1000)/100;
            UartSendData[k+3] = 0x30+(DEVICE[i].CellVolt[j]%100)/10;
            UartSendData[k+4] = 0x30+(DEVICE[i].CellVolt[j]%10);
            UartSendData[k+5] = ' ';
            UartSendData[k+6] = ' ';
            k = k+7;
        }
        UartSendData[k] = '\r';
        UartSendData[k+1] = '\n';
        k = k+2;
    }
    UartSendData[k] = 'M';
    UartSendData[k+1] = 'A';
    UartSendData[k+2] = 'X';
    UartSendData[k+3] = ' ';    
    k = k+4;
    UartSendData[k] = 0x30+DEVICE[0].CellVolt_Max/10000;
    UartSendData[k+1] = 0x30+(DEVICE[0].CellVolt_Max%10000)/1000;
    UartSendData[k+2] = 0x30+(DEVICE[0].CellVolt_Max%1000)/100;
    UartSendData[k+3] = 0x30+(DEVICE[0].CellVolt_Max%100)/10;
    UartSendData[k+4] = 0x30+(DEVICE[0].CellVolt_Max%10);
    UartSendData[k+5] = '\r';
    UartSendData[k+6] = '\n';
    k = k+7;
    UartSendData[k] = 'M';
    UartSendData[k+1] = 'I';
    UartSendData[k+2] = 'N';
    UartSendData[k+3] = ' ';    
    k = k+4;
    UartSendData[k] = 0x30+DEVICE[0].CellVolt_Min/10000;
    UartSendData[k+1] = 0x30+(DEVICE[0].CellVolt_Min%10000)/1000;
    UartSendData[k+2] = 0x30+(DEVICE[0].CellVolt_Min%1000)/100;
    UartSendData[k+3] = 0x30+(DEVICE[0].CellVolt_Min%100)/10;
    UartSendData[k+4] = 0x30+(DEVICE[0].CellVolt_Min%10);
    UartSendData[k+5] = '\r';
    UartSendData[k+6] = '\n';
    k = k+7;
    UartSendData[k] = 'X';
    UartSendData[k+1] = 'N';
    UartSendData[k+2] = 'M';
    UartSendData[k+3] = ' ';    
    k = k+4;
    UartSendData[k] = 0x30+DEVICE[0].MAX_Cell_NUM/10000;
    UartSendData[k+1] = 0x30+(DEVICE[0].MAX_Cell_NUM%10000)/1000;
    UartSendData[k+2] = 0x30+(DEVICE[0].MAX_Cell_NUM%1000)/100;
    UartSendData[k+3] = 0x30+(DEVICE[0].MAX_Cell_NUM%100)/10;
    UartSendData[k+4] = 0x30+(DEVICE[0].MAX_Cell_NUM%10);
    UartSendData[k+5] = '\r';
    UartSendData[k+6] = '\n';
    k = k+7;		
    UartSendData[k] = 'N';
    UartSendData[k+1] = 'N';
    UartSendData[k+2] = 'M';
    UartSendData[k+3] = ' ';    
    k = k+4;
    UartSendData[k] = 0x30+DEVICE[0].MIN_Cell_NUM/10000;
    UartSendData[k+1] = 0x30+(DEVICE[0].MIN_Cell_NUM%10000)/1000;
    UartSendData[k+2] = 0x30+(DEVICE[0].MIN_Cell_NUM%1000)/100;
    UartSendData[k+3] = 0x30+(DEVICE[0].MIN_Cell_NUM%100)/10;
    UartSendData[k+4] = 0x30+(DEVICE[0].MIN_Cell_NUM%10);
    UartSendData[k+5] = '\r';
    UartSendData[k+6] = '\n';
    k = k+7;		
    UartSendData[k] = 'D';
    UartSendData[k+1] = 'L';
    UartSendData[k+2] = 'T';
    UartSendData[k+3] = ' ';    
    k = k+4;
    UartSendData[k] = 0x30+DEVICE[0].CellVolt_DELTA/10000;
    UartSendData[k+1] = 0x30+(DEVICE[0].CellVolt_DELTA%10000)/1000;
    UartSendData[k+2] = 0x30+(DEVICE[0].CellVolt_DELTA%1000)/100;
    UartSendData[k+3] = 0x30+(DEVICE[0].CellVolt_DELTA%100)/10;
    UartSendData[k+4] = 0x30+(DEVICE[0].CellVolt_DELTA%10);
    UartSendData[k+5] = '\r';
    UartSendData[k+6] = '\n';
    k = k+7;		
		
    for(i=0;i<LTC6811_DeviceNUM;i++){
        UartSendData[k] = 'B';
        UartSendData[k+1] = 'L';
        UartSendData[k+2] = ' ';
        k = k+3;
        UartSendData[k] = 48;
        if(((DEVICE[i].DEC_Flag&0x0F00)>>8)>9)      UartSendData[k+1] = 55+((DEVICE[i].DEC_Flag&0x0F00)>>8);
        else                                        UartSendData[k+1] = 48+((DEVICE[i].DEC_Flag&0x0F00)>>8);
        if(((DEVICE[i].DEC_Flag&0x00F0)>>4)>9)      UartSendData[k+2] = 55+((DEVICE[i].DEC_Flag&0x00F0)>>4);
        else                                        UartSendData[k+2] = 48+((DEVICE[i].DEC_Flag&0x00F0)>>4);
        if((DEVICE[i].DEC_Flag&0x000F)>9)           UartSendData[k+3] = 55+(DEVICE[i].DEC_Flag&0x000F);
        else                                        UartSendData[k+3] = 48+(DEVICE[i].DEC_Flag&0x000F);        
        k = k+4;
        UartSendData[k] = '\r';
        UartSendData[k+1] = '\n';
        k = k+2;
    }    
    for(i=0;i<LTC6811_DeviceNUM;i++){
        UartSendData[k] = 'G';
        UartSendData[k+1] = 'V';
        UartSendData[k+2] = ' ';
        k = k+3;
        for(j=0;j<5;j++){
            UartSendData[k] = 0x30+DEVICE[i].GPIOVolt[j]/10000;
            UartSendData[k+1] = 0x30+(DEVICE[i].GPIOVolt[j]%10000)/1000;
            UartSendData[k+2] = 0x30+(DEVICE[i].GPIOVolt[j]%1000)/100;
            UartSendData[k+3] = 0x30+(DEVICE[i].GPIOVolt[j]%100)/10;
            UartSendData[k+4] = 0x30+(DEVICE[i].GPIOVolt[j]%10);
            UartSendData[k+5] = ' ';
            UartSendData[k+6] = ' ';
            k = k+7;
        }
        UartSendData[k] = '\r';
        UartSendData[k+1] = '\n';
        k = k+2;
    }    
    for(i=0;i<LTC6811_DeviceNUM;i++){
        UartSendData[k] = 'R';
        UartSendData[k+1] = 'F';
        UartSendData[k+2] = ' ';
        k = k+3;
        UartSendData[k] = 0x30+DEVICE[i].REFVolt/10000;
        UartSendData[k+1] = 0x30+(DEVICE[i].REFVolt%10000)/1000;
        UartSendData[k+2] = 0x30+(DEVICE[i].REFVolt%1000)/100;
        UartSendData[k+3] = 0x30+(DEVICE[i].REFVolt%100)/10;
        UartSendData[k+4] = 0x30+(DEVICE[i].REFVolt%10);
        k = k+5;
        UartSendData[k] = '\r';
        UartSendData[k+1] = '\n';
        k = k+2;
    }    
    for(i=0;i<LTC6811_DeviceNUM;i++){
        UartSendData[k] = 'S';
        UartSendData[k+1] = 'C';
        UartSendData[k+2] = ' ';
        k = k+3;
        UartSendData[k] = 0x30+DEVICE[i].SCVolt/10000;
        UartSendData[k+1] = 0x30+(DEVICE[i].SCVolt%10000)/1000;
        UartSendData[k+2] = 0x30+(DEVICE[i].SCVolt%1000)/100;
        UartSendData[k+3] = 0x30+(DEVICE[i].SCVolt%100)/10;
        UartSendData[k+4] = 0x30+(DEVICE[i].SCVolt%10);
        k = k+5;
        UartSendData[k] = '\r';
        UartSendData[k+1] = '\n';
        k = k+2;
    }     
//    for(i=0;i<LTC6811_DeviceNUM;i++){
//        UartSendData[k] = 'I';
//        UartSendData[k+1] = 'T';
//        UartSendData[k+2] = ' ';
//        k = k+3;
//        UartSendData[k] = 0x30+DEVICE[i].ITMPVolt/10000;
//        UartSendData[k+1] = 0x30+(DEVICE[i].ITMPVolt%10000)/1000;
//        UartSendData[k+2] = 0x30+(DEVICE[i].ITMPVolt%1000)/100;
//        UartSendData[k+3] = 0x30+(DEVICE[i].ITMPVolt%100)/10;
//        UartSendData[k+4] = 0x30+(DEVICE[i].ITMPVolt%10);
//        k = k+5;
//        UartSendData[k] = '\r';
//        UartSendData[k+1] = '\n';
//        k = k+2;
//    } 
//    for(i=0;i<LTC6811_DeviceNUM;i++){
//        UartSendData[k] = 'V';
//        UartSendData[k+1] = 'A';
//        UartSendData[k+2] = ' ';
//        k = k+3;
//        UartSendData[k] = 0x30+DEVICE[i].VAVolt/10000;
//        UartSendData[k+1] = 0x30+(DEVICE[i].VAVolt%10000)/1000;
//        UartSendData[k+2] = 0x30+(DEVICE[i].VAVolt%1000)/100;
//        UartSendData[k+3] = 0x30+(DEVICE[i].VAVolt%100)/10;
//        UartSendData[k+4] = 0x30+(DEVICE[i].VAVolt%10);
//        k = k+5;
//        UartSendData[k] = '\r';
//        UartSendData[k+1] = '\n';
//        k = k+2;
//    }
//    for(i=0;i<LTC6811_DeviceNUM;i++){
//        UartSendData[k] = 'V';
//        UartSendData[k+1] = 'D';
//        UartSendData[k+2] = ' ';
//        k = k+3;
//        UartSendData[k] = 0x30+DEVICE[i].VDVolt/10000;
//        UartSendData[k+1] = 0x30+(DEVICE[i].VDVolt%10000)/1000;
//        UartSendData[k+2] = 0x30+(DEVICE[i].VDVolt%1000)/100;
//        UartSendData[k+3] = 0x30+(DEVICE[i].VDVolt%100)/10;
//        UartSendData[k+4] = 0x30+(DEVICE[i].VDVolt%10);
//        k = k+5;
//        UartSendData[k] = '\r';
//        UartSendData[k+1] = '\n';
//        k = k+2;
//    }
//    for(i=0;i<LTC6811_DeviceNUM;i++){
//        UartSendData[k] = 'R';
//        UartSendData[k+1] = 'E';
//        UartSendData[k+2] = 'V';
//        UartSendData[k+3] = ' ';
//        k = k+4;
//        if(((DEVICE[i].REV&0xF0)>>4)>9) UartSendData[k] = 55+((DEVICE[i].REV&0xF0)>>4);
//        else                            UartSendData[k] = 48+((DEVICE[i].REV&0xF0)>>4);
//        if((DEVICE[i].REV&0x0F)>9)      UartSendData[k+1] = 55+(DEVICE[i].REV&0x0F);
//        else                            UartSendData[k+1] = 48+(DEVICE[i].REV&0x0F);
//        k = k+2;
//        UartSendData[k] = '\r';
//        UartSendData[k+1] = '\n';
//        k = k+2;
//    }

		UartSendData[k] = 'R';
		UartSendData[k+1] = 'E';
		UartSendData[k+2] = 'V';
		UartSendData[k+3] = ' ';
		k = k+4;
		if(((DEVICE[i].REV&0xF0)>>4)>9) UartSendData[k] = 55+((DEVICE[0].REV&0xF0)>>4);
		else                            UartSendData[k] = 48+((DEVICE[0].REV&0xF0)>>4);
		if((DEVICE[i].REV&0x0F)>9)      UartSendData[k+1] = 55+(DEVICE[0].REV&0x0F);
		else                            UartSendData[k+1] = 48+(DEVICE[0].REV&0x0F);
		k = k+2;
		UartSendData[k] = '\r';
		UartSendData[k+1] = '\n';
		k = k+2;

		UartSendData[k] = 'M';
    UartSendData[k+1] = 'n';
    UartSendData[k+2] = 'm';
    UartSendData[k+3] = ' ';
		
		if(((Measure_Num&0xF0000000)>>28)<10)	UartSendData[k+4] = 48+((Measure_Num&0xF0000000)>>28);
		else																	UartSendData[k+4] = 55+((Measure_Num&0xF0000000)>>28);
		if(((Measure_Num&0x0F000000)>>24)<10)	UartSendData[k+5] = 48+((Measure_Num&0x0F000000)>>24);
		else																	UartSendData[k+5] = 55+((Measure_Num&0x0F000000)>>24);		
		if(((Measure_Num&0x00F00000)>>20)<10)	UartSendData[k+6] = 48+((Measure_Num&0x00F00000)>>20);
		else																	UartSendData[k+6] = 55+((Measure_Num&0x00F00000)>>20);		
		if(((Measure_Num&0x000F0000)>>16)<10)	UartSendData[k+7] = 48+((Measure_Num&0x000F0000)>>16);
		else																	UartSendData[k+7] = 55+((Measure_Num&0x000F0000)>>16);
		if(((Measure_Num&0x0000F000)>>12)<10)	UartSendData[k+8] = 48+((Measure_Num&0x0000F000)>>12);
		else																	UartSendData[k+8] = 55+((Measure_Num&0x0000F000)>>12);
		if(((Measure_Num&0x00000F00)>>8)<10)	UartSendData[k+9] = 48+((Measure_Num&0x00000F00)>>8);
		else																	UartSendData[k+9] = 55+((Measure_Num&0x00000F00)>>8);
		if(((Measure_Num&0x000000F0)>>4)<10)	UartSendData[k+10] = 48+((Measure_Num&0x000000F0)>>4);
		else																	UartSendData[k+10] = 55+((Measure_Num&0x000000F0)>>4);
		if((Measure_Num&0x0000000F)<10)				UartSendData[k+11] = 48+(Measure_Num&0x0000000F);
		else																	UartSendData[k+11] = 55+(Measure_Num&0x0000000F);
		k = k+12;
    UartSendData[k] = '\r';
    UartSendData[k+1] = '\n';
    k = k+2;
	
    UartSendData[k] = '\r';
    UartSendData[k+1] = '\n';
    k = k+2;		
		
		UartSendData[k] = 'L';
		UartSendData[k+1] = 'T';
		UartSendData[k+2] = 'C';
		UartSendData[k+3] = '6';
		UartSendData[k+4] = '8';
		k = k+5;	
		UartSendData[k] = ' ';
		UartSendData[k+1] = 'T';
		UartSendData[k+2] = 'E';
		UartSendData[k+3] = 'M';
		UartSendData[k+4] = 'P';	
		k = k+5;	
		UartSendData[k] = '\r';
		UartSendData[k+1] = '\n';
		k = k+2;
		
		for(i=0;i<LTC6811_DeviceNUM;i++){
			UartSendData[k] = 'D';
			UartSendData[k+1] = 'E';
			UartSendData[k+2] = 'V';
			UartSendData[k+3] = '0'+i/10;
			UartSendData[k+4] = '0'+i%10;
			UartSendData[k+5] = ' ';		
			k = k+6;			
			for(j=0;j<5;j++){
				UartSendData[k] = 0x30+DEVICE[i].GPIO_NTC_TEMP[j]/100;
				UartSendData[k+1] = 0x30+(DEVICE[i].GPIO_NTC_TEMP[j]%100)/10;
				UartSendData[k+2] = 0x30+(DEVICE[i].GPIO_NTC_TEMP[j]%10);
				UartSendData[k+3] = ' ';
				k = k+4;
			}
			UartSendData[k] = '\r';
			UartSendData[k+1] = '\n';
		k = k+2;		
		}
		UartSendData[k] = 'M';
		UartSendData[k+1] = 'A';
		UartSendData[k+2] = 'X';
		UartSendData[k+3] = ' ';
		k = k+4;	
		UartSendData[k] = 0x30+DEVICE[0].LT68_NTC_Temp_MAX/100;
		UartSendData[k+1] = 0x30+(DEVICE[0].LT68_NTC_Temp_MAX%100)/10;
		UartSendData[k+2] = 0x30+(DEVICE[0].LT68_NTC_Temp_MAX%10);
		k = k+3;
		UartSendData[k] = '\r';
		UartSendData[k+1] = '\n';
		k = k+2;
		UartSendData[k] = 'M';
		UartSendData[k+1] = 'I';
		UartSendData[k+2] = 'N';
		UartSendData[k+3] = ' ';
		k = k+4;	
		UartSendData[k] = 0x30+DEVICE[0].LT68_NTC_Temp_MIN/100;
		UartSendData[k+1] = 0x30+(DEVICE[0].LT68_NTC_Temp_MIN%100)/10;
		UartSendData[k+2] = 0x30+(DEVICE[0].LT68_NTC_Temp_MIN%10);
		k = k+3;
		UartSendData[k] = '\r';
		UartSendData[k+1] = '\n';
		k = k+2;			
		UartSendData[k] = '\r';
		UartSendData[k+1] = '\n';
		k = k+2;		
    uart3_send(UartSendData, k);
		uart1_send(UartSendData, k);
}

void LTC6811_Mission(void){
	uint8_t i,j,k;
	uint16_t MAX,MIN,MAX_NUM,MIN_NUM;
	uint16_t buff;
	if(LTC6811_Init_Status == 0x00){
		switch(LTC6811_Mission_StepCounter){
			case 1:
				LTC6811_WeekIdle();
				break;			
			case 2://写配置寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CFGR[0] = LTC6811_GPIO(0x1F)|LTC6811_REFON(1)|LTC6811_DETN(1)|LTC6811_ADCOPT(0);
					DEVICE[i].CFGR[1] = 0x00FF&LTC6811_VUV;            
					DEVICE[i].CFGR[2] = ((0x000F&LTC6811_VOV)<<4)+((0x0F00&LTC6811_VUV)>>8);            
					DEVICE[i].CFGR[3] = (0x0FF0&LTC6811_VOV)>>4; 
					DEVICE[i].DEC_Flag = 0x0000;
					DEVICE[i].CFGR[4] = 0x00FF&DEVICE[i].DEC_Flag;            
					DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&DEVICE[i].DEC_Flag)>>8);
					pecdata = pec15(DEVICE[i].CFGR,6);
					DEVICE[i].CFGR[6] = pecdata>>8;
					DEVICE[i].CFGR[7] = pecdata&0x00FF;
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<8;j++){
						SendData[i*8+j+4] = DEVICE[i].CFGR[j];
					}
				}
				LTC6811_Writecmd(&CMD_WRCFGA);
				break;               
			case 4://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRCELL); 
				break;                
			case 5://发送CVST命令
				LTC6811_Sendcmd(&CMD_CVST);
				break;
			case 14:
				LTC6811_WeekIdle();
				break;			
			case 15://发送RDCVA命令，读CVAR寄存器
				LTC6811_Readcmd(&CMD_RDCVA);
				break;
			case 17://读取的数据转移到DEVICE1.CVAR，发送RDCVB命令，读CVBR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVAR,i);       //将接收数据移到CVAR寄存器中
				LTC6811_Readcmd(&CMD_RDCVB);
				break; 
			case 19://读取的数据转移到DEVICE1.CVBR，发送RDCVC命令，读CVCR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVBR,i);       //将接收数据移到CVBR寄存器中
				LTC6811_Readcmd(&CMD_RDCVC);            
				break;        
			case 21://读取的数据转移到DEVICE1.CVCR，发送RDCVD命令，读CVDR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVCR,i);       //将接收数据移到CVBR寄存器中
				LTC6811_Readcmd(&CMD_RDCVD);             
				break;        
			case 23://读取的数据转移到DEVICE1.CVDR,判断CVST结果,正确则LTC6811_Init_Status++,错误则LTC6811_Init_Status = 0x80故障模式
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVDR,i);       //将接收数据移到CVBR寄存器中 
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CellVolt_AS[0] = (((uint16_t)DEVICE[i].CVAR[1])<<8)+DEVICE[i].CVAR[0];
					DEVICE[i].CellVolt_AS[1] = (((uint16_t)DEVICE[i].CVAR[3])<<8)+DEVICE[i].CVAR[2];
					DEVICE[i].CellVolt_AS[2] = (((uint16_t)DEVICE[i].CVAR[5])<<8)+DEVICE[i].CVAR[4];
					DEVICE[i].CellVolt_AS[3] = (((uint16_t)DEVICE[i].CVBR[1])<<8)+DEVICE[i].CVBR[0];
					DEVICE[i].CellVolt_AS[4] = (((uint16_t)DEVICE[i].CVBR[3])<<8)+DEVICE[i].CVBR[2];
					DEVICE[i].CellVolt_AS[5] = (((uint16_t)DEVICE[i].CVBR[5])<<8)+DEVICE[i].CVBR[4];
					DEVICE[i].CellVolt_AS[6] = (((uint16_t)DEVICE[i].CVCR[1])<<8)+DEVICE[i].CVCR[0];
					DEVICE[i].CellVolt_AS[7] = (((uint16_t)DEVICE[i].CVCR[3])<<8)+DEVICE[i].CVCR[2];
					DEVICE[i].CellVolt_AS[8] = (((uint16_t)DEVICE[i].CVCR[5])<<8)+DEVICE[i].CVCR[4];
					DEVICE[i].CellVolt_AS[9] = (((uint16_t)DEVICE[i].CVDR[1])<<8)+DEVICE[i].CVDR[0];
					DEVICE[i].CellVolt_AS[10] = (((uint16_t)DEVICE[i].CVDR[3])<<8)+DEVICE[i].CVDR[2];
					DEVICE[i].CellVolt_AS[11] = (((uint16_t)DEVICE[i].CVDR[5])<<8)+DEVICE[i].CVDR[4];
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<12;j++){
						if(DEVICE[i].CellVolt_AS[j] != LTC6811_ADCSelfTestCode)   LTC6811_Init_Status = 0x80;
					}
				}
				if(LTC6811_Init_Status == 0x80){
					if(repeat < MAX_Repeat_Num){
						repeat++;
						LTC6811_Init_Status = 0x00;
						LTC6811_Mission_StepCounter = 2-1;
					}else LTC6811_Init_Status = 0x80;
				}else repeat = 0;
				break;
			case 24://发送CMD_CLRAUX命令
				LTC6811_Sendcmd(&CMD_CLRAUX);  
				break; 
			case 25://发送AXST命令
				LTC6811_Sendcmd(&CMD_AXST);
				break;
			case 34:
				LTC6811_WeekIdle();
				break;			
			case 35://读取RDAUXA
				LTC6811_Readcmd(&CMD_RDAUXA);
				break;
			case 37://移动数据到AVAR 读取RDAUXB
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].AVAR,i);       //将接收数据移到AVAR寄存器中 
				LTC6811_Readcmd(&CMD_RDAUXB);
				break;    
			case 39://移动数据到AVBR，判断结果
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].AVBR,i);       //将接收数据移到AVBR寄存器中
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CellVolt_AS[0] = 0;
					DEVICE[i].CellVolt_AS[1] = 0;
					DEVICE[i].CellVolt_AS[2] = 0;
					DEVICE[i].CellVolt_AS[3] = 0;
					DEVICE[i].CellVolt_AS[4] = 0;
					DEVICE[i].CellVolt_AS[5] = 0;
					DEVICE[i].CellVolt_AS[0] = (((uint16_t)DEVICE[i].AVAR[1])<<8)+DEVICE[i].AVAR[0];
					DEVICE[i].CellVolt_AS[1] = (((uint16_t)DEVICE[i].AVAR[3])<<8)+DEVICE[i].AVAR[2];
					DEVICE[i].CellVolt_AS[2] = (((uint16_t)DEVICE[i].AVAR[5])<<8)+DEVICE[i].AVAR[4];
					DEVICE[i].CellVolt_AS[3] = (((uint16_t)DEVICE[i].AVBR[1])<<8)+DEVICE[i].AVBR[0];
					DEVICE[i].CellVolt_AS[4] = (((uint16_t)DEVICE[i].AVBR[3])<<8)+DEVICE[i].AVBR[2];
					DEVICE[i].CellVolt_AS[5] = (((uint16_t)DEVICE[i].AVBR[5])<<8)+DEVICE[i].AVBR[4];
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<6;j++){
						if(DEVICE[i].CellVolt_AS[j] != LTC6811_ADCSelfTestCode) LTC6811_Init_Status = 0x80;
					}
				}
				if(LTC6811_Init_Status == 0x80){
					if(repeat < MAX_Repeat_Num){
						repeat++;
						LTC6811_Init_Status = 0x00;
						LTC6811_Mission_StepCounter = 13-1;
					}else LTC6811_Init_Status = 0x80;
				}else repeat = 0;
				break;                
			case 40://发送STATST命令
				LTC6811_Sendcmd(&CMD_CLRSTAT);
				break;                 
			case 41://发送STATST命令
				LTC6811_Sendcmd(&CMD_STATST);
				break;
			case 50:
				LTC6811_WeekIdle();
				break;			
			case 51://读取RDAUXA
				LTC6811_Readcmd(&CMD_RDSTATA);
				break;
			case 53://移动数据到AVAR 读取RDAUXB
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].STAR,i);
				LTC6811_Readcmd(&CMD_RDSTATB);
				break;    
			case 55://移动数据到AVBR，判断结果
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].STBR,i);
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CellVolt_AS[0] = 0;
					DEVICE[i].CellVolt_AS[1] = 0;
					DEVICE[i].CellVolt_AS[2] = 0;
					DEVICE[i].CellVolt_AS[3] = 0;
					DEVICE[i].CellVolt_AS[0] = (((uint16_t)DEVICE[i].STAR[1])<<8)+DEVICE[i].STAR[0];
					DEVICE[i].CellVolt_AS[1] = (((uint16_t)DEVICE[i].STAR[3])<<8)+DEVICE[i].STAR[2];
					DEVICE[i].CellVolt_AS[2] = (((uint16_t)DEVICE[i].STAR[5])<<8)+DEVICE[i].STAR[4];
					DEVICE[i].CellVolt_AS[3] = (((uint16_t)DEVICE[i].STBR[1])<<8)+DEVICE[i].STBR[0];
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					if(DEVICE[i].CellVolt_AS[0] != LTC6811_ADCSelfTestCode)   LTC6811_Init_Status = 0x80;
					if(DEVICE[i].CellVolt_AS[1] != LTC6811_ADCSelfTestCode)   LTC6811_Init_Status = 0x80;
					if(DEVICE[i].CellVolt_AS[2] != LTC6811_ADCSelfTestCode)   LTC6811_Init_Status = 0x80;
					if(DEVICE[i].CellVolt_AS[3] != LTC6811_ADCSelfTestCode)   LTC6811_Init_Status = 0x80;
				}
				if(LTC6811_Init_Status == 0x80){
					if(repeat < MAX_Repeat_Num){
						repeat++;
						LTC6811_Init_Status = 0x00;
						LTC6811_Mission_StepCounter = 22-1;
					}else LTC6811_Init_Status = 0x80;
				}else repeat = 0;                
				break;                                     
/*			case 56://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRCELL);  
				break; 
			case 57://发送ADOW_PU
				LTC6811_Sendcmd(&CMD_ADOW_PU);
				break;
			case 66:
				LTC6811_WeekIdle();
				break;			
			case 67://发送ADOW_PU
				LTC6811_Sendcmd(&CMD_ADOW_PU);
				break;
			case 76:
				LTC6811_WeekIdle();
				break;			
			case 77://发送RDCVA命令，读CVAR寄存器
				LTC6811_Readcmd(&CMD_RDCVA);
				break;
			case 79://读取的数据转移到DEVICE1.CVAR，发送RDCVB命令，读CVBR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVAR,i);
				LTC6811_Readcmd(&CMD_RDCVB);
				break; 
			case 81://读取的数据转移到DEVICE1.CVBR，发送RDCVC命令，读CVCR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVBR,i);
				LTC6811_Readcmd(&CMD_RDCVC);            
				break;        
			case 83://读取的数据转移到DEVICE1.CVCR，发送RDCVD命令，读CVDR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVCR,i);
				LTC6811_Readcmd(&CMD_RDCVD);             
				break;        
			case 85://读取的数据转移到DEVICE1.CVDR,判断CVST结果,正确则LTC6811_Init_Status++,错误则LTC6811_Init_Status = 0x80故障模式
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVDR,i);  
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CellVolt_AS[0] = (((uint16_t)DEVICE[i].CVAR[1])<<8)+DEVICE[i].CVAR[0];
					DEVICE[i].CellVolt_AS[1] = (((uint16_t)DEVICE[i].CVAR[3])<<8)+DEVICE[i].CVAR[2];
					DEVICE[i].CellVolt_AS[2] = (((uint16_t)DEVICE[i].CVAR[5])<<8)+DEVICE[i].CVAR[4];
					DEVICE[i].CellVolt_AS[3] = (((uint16_t)DEVICE[i].CVBR[1])<<8)+DEVICE[i].CVBR[0];
					DEVICE[i].CellVolt_AS[4] = (((uint16_t)DEVICE[i].CVBR[3])<<8)+DEVICE[i].CVBR[2];
					DEVICE[i].CellVolt_AS[5] = (((uint16_t)DEVICE[i].CVBR[5])<<8)+DEVICE[i].CVBR[4];
					DEVICE[i].CellVolt_AS[6] = (((uint16_t)DEVICE[i].CVCR[1])<<8)+DEVICE[i].CVCR[0];
					DEVICE[i].CellVolt_AS[7] = (((uint16_t)DEVICE[i].CVCR[3])<<8)+DEVICE[i].CVCR[2];
					DEVICE[i].CellVolt_AS[8] = (((uint16_t)DEVICE[i].CVCR[5])<<8)+DEVICE[i].CVCR[4];
					DEVICE[i].CellVolt_AS[9] = (((uint16_t)DEVICE[i].CVDR[1])<<8)+DEVICE[i].CVDR[0];
					DEVICE[i].CellVolt_AS[10] = (((uint16_t)DEVICE[i].CVDR[3])<<8)+DEVICE[i].CVDR[2];
					DEVICE[i].CellVolt_AS[11] = (((uint16_t)DEVICE[i].CVDR[5])<<8)+DEVICE[i].CVDR[4];
					if(DEVICE[i].CellVolt_AS[0] == 0)   DEVICE[i].OPENWires |= 0x0001;
					else                                DEVICE[i].OPENWires &= 0xFFFE;
				}
				break;
			case 86://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRCELL);  
				break;                
			case 87://发送ADOW_PD
				LTC6811_Sendcmd(&CMD_ADOW_PD);
				break;
			case 96:
				LTC6811_WeekIdle();
				break;			
			case 97://发送ADOW_PD
				LTC6811_Sendcmd(&CMD_ADOW_PD);
				break;
			case 106:
				LTC6811_WeekIdle();
				break;			
			case 107://发送RDCVA命令，读CVAR寄存器
				LTC6811_Readcmd(&CMD_RDCVA);
				break;
			case 109://读取的数据转移到DEVICE1.CVAR，发送RDCVB命令，读CVBR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVAR,i);
				LTC6811_Readcmd(&CMD_RDCVB);
				break; 
			case 111://读取的数据转移到DEVICE1.CVBR，发送RDCVC命令，读CVCR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVBR,i);
				LTC6811_Readcmd(&CMD_RDCVC);            
				break;        
			case 113://读取的数据转移到DEVICE1.CVCR，发送RDCVD命令，读CVDR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVCR,i);
				LTC6811_Readcmd(&CMD_RDCVD);             
				break;        
			case 115://读取的数据转移到DEVICE1.CVDR,判断CVST结果,正确则LTC6811_Init_Status++,错误则LTC6811_Init_Status = 0x80故障模式
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVDR,i); 
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CellVolt_AS[0] = (((uint16_t)DEVICE[i].CVAR[1])<<8)+DEVICE[i].CVAR[0];
					DEVICE[i].CellVolt_AS[1] = (((uint16_t)DEVICE[i].CVAR[3])<<8)+DEVICE[i].CVAR[2];
					DEVICE[i].CellVolt_AS[2] = (((uint16_t)DEVICE[i].CVAR[5])<<8)+DEVICE[i].CVAR[4];
					DEVICE[i].CellVolt_AS[3] = (((uint16_t)DEVICE[i].CVBR[1])<<8)+DEVICE[i].CVBR[0];
					DEVICE[i].CellVolt_AS[4] = (((uint16_t)DEVICE[i].CVBR[3])<<8)+DEVICE[i].CVBR[2];
					DEVICE[i].CellVolt_AS[5] = (((uint16_t)DEVICE[i].CVBR[5])<<8)+DEVICE[i].CVBR[4];
					DEVICE[i].CellVolt_AS[6] = (((uint16_t)DEVICE[i].CVCR[1])<<8)+DEVICE[i].CVCR[0];
					DEVICE[i].CellVolt_AS[7] = (((uint16_t)DEVICE[i].CVCR[3])<<8)+DEVICE[i].CVCR[2];
					DEVICE[i].CellVolt_AS[8] = (((uint16_t)DEVICE[i].CVCR[5])<<8)+DEVICE[i].CVCR[4];
					DEVICE[i].CellVolt_AS[9] = (((uint16_t)DEVICE[i].CVDR[1])<<8)+DEVICE[i].CVDR[0];
					DEVICE[i].CellVolt_AS[10] = (((uint16_t)DEVICE[i].CVDR[3])<<8)+DEVICE[i].CVDR[2];
					DEVICE[i].CellVolt_AS[11] = (((uint16_t)DEVICE[i].CVDR[5])<<8)+DEVICE[i].CVDR[4];
					if(DEVICE[i].CellVolt[11] == 0)     DEVICE[i].OPENWires |= 0x1000;
					else                                DEVICE[i].OPENWires &= 0x7FFF;
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					if(((DEVICE[i].CellVolt_AS[1]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[1]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0002; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[2]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[2]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0004; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[3]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[3]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0008; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[4]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[4]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0010; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[5]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[5]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0020; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[6]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[6]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0040; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[7]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[7]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0080; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[8]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[8]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0100; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[9]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[9]+400) < 0xFE70))    { DEVICE[i].OPENWires |= 0x0200; LTC6811_Init_Status = 0x80;}
					if(((DEVICE[i].CellVolt_AS[10]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[10]+400) < 0xFE70))  { DEVICE[i].OPENWires |= 0x0400; LTC6811_Init_Status = 0x80;}
//                if(((DEVICE[i].CellVolt_AS[11]+400) > 0xAFC8)&&((DEVICE[i].CellVolt_AS[11]+400) < 0xFE70))  { DEVICE[i].OPENWires |= 0x0800; LTC6811_Init_Status = 0x80;}
				}
				if(LTC6811_Init_Status == 0x80){
					if(repeat < MAX_Repeat_Num){
						repeat++;
						LTC6811_Init_Status = 0x00;
						LTC6811_Mission_StepCounter = 31-1;
					}else LTC6811_Init_Status = 0x80;
				}else repeat = 0;
				break; 
*/
			case 116://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRCELL);  
				break; 
			case 117://发送ADCV命令
				LTC6811_Sendcmd(&CMD_ADCV);
				break;
			case 126:
				LTC6811_WeekIdle();
				break;			
			case 127://发送RDCVA命令，读CVAR寄存器
				LTC6811_Readcmd(&CMD_RDCVA);
				break;
			case 129://读取的数据转移到DEVICE1.CVAR，发送RDCVB命令，读CVBR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVAR,i); 
				LTC6811_Readcmd(&CMD_RDCVB);
				break; 
			case 131://读取的数据转移到DEVICE1.CVBR，发送RDCVC命令，读CVCR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVBR,i); 
				LTC6811_Readcmd(&CMD_RDCVC);            
				break;        
			case 133://读取的数据转移到DEVICE1.CVCR，发送RDCVD命令，读CVDR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVCR,i); 
				LTC6811_Readcmd(&CMD_RDCVD);             
				break;        
			case 135://读取的数据转移到DEVICE1.CVDR,判断CVST结果,正确则LTC6811_Init_Status++,错误则LTC6811_Init_Status = 0x80故障模式
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVDR,i);  
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].CellVolt[0] = (((uint16_t)DEVICE[i].CVAR[1])<<8)+DEVICE[i].CVAR[0];
					DEVICE[i].CellVolt[1] = (((uint16_t)DEVICE[i].CVAR[3])<<8)+DEVICE[i].CVAR[2];
					DEVICE[i].CellVolt[2] = (((uint16_t)DEVICE[i].CVAR[5])<<8)+DEVICE[i].CVAR[4];
					DEVICE[i].CellVolt[3] = (((uint16_t)DEVICE[i].CVBR[1])<<8)+DEVICE[i].CVBR[0];
					DEVICE[i].CellVolt[4] = (((uint16_t)DEVICE[i].CVBR[3])<<8)+DEVICE[i].CVBR[2];
					DEVICE[i].CellVolt[5] = (((uint16_t)DEVICE[i].CVBR[5])<<8)+DEVICE[i].CVBR[4];
					DEVICE[i].CellVolt[6] = (((uint16_t)DEVICE[i].CVCR[1])<<8)+DEVICE[i].CVCR[0];
					DEVICE[i].CellVolt[7] = (((uint16_t)DEVICE[i].CVCR[3])<<8)+DEVICE[i].CVCR[2];
					DEVICE[i].CellVolt[8] = (((uint16_t)DEVICE[i].CVCR[5])<<8)+DEVICE[i].CVCR[4];
					DEVICE[i].CellVolt[9] = (((uint16_t)DEVICE[i].CVDR[1])<<8)+DEVICE[i].CVDR[0];
					DEVICE[i].CellVolt[10] = (((uint16_t)DEVICE[i].CVDR[3])<<8)+DEVICE[i].CVDR[2];
					DEVICE[i].CellVolt[11] = (((uint16_t)DEVICE[i].CVDR[5])<<8)+DEVICE[i].CVDR[4];
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					//判断电压在最大和最小值之间，认为该串使用
					for(j=0;j<12;j++){
						if((DEVICE[i].CellVolt[j]>BatteryVolt_MIN)||(DEVICE[i].CellVolt[j]<BatteryVolt_MAX))    DEVICE[i].Cell_Select |= (0x0001<<j);
					}
					//判断小于100mV，认为该串未使用
					for(j=0;j<12;j++){
						if(DEVICE[i].CellVolt[j]<=CHN_Unuse_THD)    DEVICE[i].Cell_Select &= ~(0x0001<<j);
					}									
				}
				if(LTC6811_Init_Status == 0x80){
					if(repeat < MAX_Repeat_Num){
						repeat++;
						LTC6811_Init_Status = 0x00;
						LTC6811_Mission_StepCounter = 63-1;
					}else LTC6811_Init_Status = 0x80;
				}else repeat = 0;
				break;                
			case 136://写配置寄存器 打开奇数均衡
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].DEC_Flag = 0x0555;
					DEVICE[i].CFGR[4] = 0x00FF&DEVICE[i].DEC_Flag;            
					DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&DEVICE[i].DEC_Flag)>>8);
					pecdata = pec15(DEVICE[i].CFGR,6);
					DEVICE[i].CFGR[6] = pecdata>>8;
					DEVICE[i].CFGR[7] = pecdata&0x00FF;
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<8;j++){
						SendData[i*8+j+4] = DEVICE[i].CFGR[j];
					}
				}
				LTC6811_Writecmd(&CMD_WRCFGA);
				break;               
			case 285:
					LTC6811_WeekIdle();
					break;			
			case 286://写配置寄存器 打开偶数均衡
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].DEC_Flag = 0x0AAA;
					DEVICE[i].CFGR[4] = 0x00FF&DEVICE[i].DEC_Flag;            
					DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&DEVICE[i].DEC_Flag)>>8);
					pecdata = pec15(DEVICE[i].CFGR,6);
					DEVICE[i].CFGR[6] = pecdata>>8;
					DEVICE[i].CFGR[7] = pecdata&0x00FF;
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<8;j++){
						SendData[i*8+j+4] = DEVICE[i].CFGR[j];
					}
				}
				LTC6811_Writecmd(&CMD_WRCFGA);
				break;
			case 435:
				LTC6811_WeekIdle();
				break;
			case 436://写配置寄存器 关闭均衡
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].DEC_Flag &= 0xF000;
					DEVICE[i].CFGR[4] = 0x00FF&DEVICE[i].DEC_Flag;            
					DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&DEVICE[i].DEC_Flag)>>8);
					pecdata = pec15(DEVICE[i].CFGR,6);
					DEVICE[i].CFGR[6] = pecdata>>8;
					DEVICE[i].CFGR[7] = pecdata&0x00FF;
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<8;j++){
						SendData[i*8+j+4] = DEVICE[i].CFGR[j];
					}
				}
				LTC6811_Writecmd(&CMD_WRCFGA);
				break;                                               
			case 438://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRCELL);  
				break; 
			case 439://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRAUX);  
				break; 
			case 440://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRSTAT);  
				break; 
			case 441:
				LTC6811_Init_Status = 0x01;
				
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<12;j++){
						DEVICE[i].CellVolt[j] = 0;
					}
				}				
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<5;j++){
						DEVICE[i].GPIOVolt[j] = 0;
					}
				}				
				
				
				LTC6811_Mission_StepCounter = 0;
				break;                 
		}
	}else if(LTC6811_Init_Status == 0x80){	//判断为自检错误
		LED1_SET_0;
	}else if(LTC6811_Init_Status == 0x01){	//判断为自检通过
		LED1_SET_1;
//		DEVICE[0].Cell_Select = 0x01CF;
//		DEVICE[1].Cell_Select = 0x03CF;
			
		switch(LTC6811_Mission_StepCounter){
			case 1:
				LTC6811_WeekIdle();
				break;
			case 2://发送CMD_CLRCELL命令
				LTC6811_Sendcmd(&CMD_CLRCELL);  
				break; 
			case 3://发送ADCV命令
				LTC6811_Sendcmd(&CMD_ADCV);
				break;
			case 12:
				LTC6811_WeekIdle();
				break;
			case 13://发送RDCVA命令，读CVAR寄存器
				LTC6811_Readcmd(&CMD_RDCVA);
				break;
			case 15://读取的数据转移到DEVICE1.CVAR，发送RDCVB命令，读CVBR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVAR,i);
				LTC6811_Readcmd(&CMD_RDCVB);
				break; 
			case 17://读取的数据转移到DEVICE1.CVBR，发送RDCVC命令，读CVCR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVBR,i);
				LTC6811_Readcmd(&CMD_RDCVC);            
				break;        
			case 19://读取的数据转移到DEVICE1.CVCR，发送RDCVD命令，读CVDR寄存器
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVCR,i);
				LTC6811_Readcmd(&CMD_RDCVD);             
				break;        
			case 21://读取的数据转移到DEVICE1.CVDR,判断CVST结果
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].CVDR,i);
				for(i=0;i<LTC6811_DeviceNUM;i++){	//将电压缓存在电压辅助数组里
					DEVICE[i].CellVolt_AS[0] = (((uint16_t)DEVICE[i].CVAR[1])<<8)+DEVICE[i].CVAR[0];
					DEVICE[i].CellVolt_AS[1] = (((uint16_t)DEVICE[i].CVAR[3])<<8)+DEVICE[i].CVAR[2];
					DEVICE[i].CellVolt_AS[2] = (((uint16_t)DEVICE[i].CVAR[5])<<8)+DEVICE[i].CVAR[4];
					DEVICE[i].CellVolt_AS[3] = (((uint16_t)DEVICE[i].CVBR[1])<<8)+DEVICE[i].CVBR[0];
					DEVICE[i].CellVolt_AS[4] = (((uint16_t)DEVICE[i].CVBR[3])<<8)+DEVICE[i].CVBR[2];
					DEVICE[i].CellVolt_AS[5] = (((uint16_t)DEVICE[i].CVBR[5])<<8)+DEVICE[i].CVBR[4];
					DEVICE[i].CellVolt_AS[6] = (((uint16_t)DEVICE[i].CVCR[1])<<8)+DEVICE[i].CVCR[0];
					DEVICE[i].CellVolt_AS[7] = (((uint16_t)DEVICE[i].CVCR[3])<<8)+DEVICE[i].CVCR[2];
					DEVICE[i].CellVolt_AS[8] = (((uint16_t)DEVICE[i].CVCR[5])<<8)+DEVICE[i].CVCR[4];
					DEVICE[i].CellVolt_AS[9] = (((uint16_t)DEVICE[i].CVDR[1])<<8)+DEVICE[i].CVDR[0];
					DEVICE[i].CellVolt_AS[10] = (((uint16_t)DEVICE[i].CVDR[3])<<8)+DEVICE[i].CVDR[2];
					DEVICE[i].CellVolt_AS[11] = (((uint16_t)DEVICE[i].CVDR[5])<<8)+DEVICE[i].CVDR[4];
				}

				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<12;j++){
						if((DEVICE[i].Cell_Select&(0x0001<<j))>0){
							if((DEVICE[i].CellVolt_AS[j]>BatteryVolt_MIN)&&(DEVICE[i].CellVolt_AS[j]<BatteryVolt_MAX)){
//								DEVICE[i].CellVolt[j] = DEVICE[i].CellVolt_AS[j];
								DEVICE[i].CellVolt_CUM[j][CUM_Num] = DEVICE[i].CellVolt_AS[j];	//数据存到累加数组
							}
						}
					}
				}
				
//				CellVolt_Max_Min();	//找出最大值和最小值
				
				//均衡开启判断条件	最小值大于LTC6811_BalanceStartVolt(0.1mV) 或最大值大于(LTC6811_BalanceStartVolt+1000)(0.1mV) 
				if((DEVICE[0].CellVolt_Min>LTC6811_BalanceStartVolt) || (DEVICE[0].CellVolt_Max>(LTC6811_BalanceStartVolt+1000))){	
					if((DEVICE[0].CellVolt_Max - DEVICE[0].CellVolt_Min)>LTC6811_BalanceThreshold){    //并且最大值和最小值差值大于LTC6811_BalanceThreshold开启均衡
						Balance();	//标记需要均衡的串
					}else{					//最大值和最小值差值小于LTC6811_BalanceThreshold 清零均衡标志 关闭均衡
						for(i=0;i<LTC6811_DeviceNUM;i++){
							DEVICE[i].DEC_Flag &= 0xF000;
						}
					}
				}else{
					for(i=0;i<LTC6811_DeviceNUM;i++){
						DEVICE[i].DEC_Flag &= 0xF000;
					}										
				}
				break;
			case 22:       
				//奇数串和偶数串分开 每隔1s切换均衡
				if(Dec_Time>SYSTimer_num){													//Dec_Time均衡放电计时，均衡偶数和奇数的切换计时
					for(i=0;i<LTC6811_DeviceNUM;i++){
						DEVICE[i].CFGR[0] = LTC6811_GPIO(0x1F)|LTC6811_REFON(1)|LTC6811_DETN(1)|LTC6811_ADCOPT(0);
						DEVICE[i].CFGR[1] = 0x00FF&LTC6811_VUV;            
						DEVICE[i].CFGR[2] = ((0x000F&LTC6811_VOV)<<4)+((0x0F00&LTC6811_VUV)>>8);            
						DEVICE[i].CFGR[3] = (0x0FF0&LTC6811_VOV)>>4;
						switch(DEVICE[i].DEC_Flag&0xF000){
							case 0x1000:
								DEVICE[i].CFGR[4] = 0x00FF&(DEVICE[i].DEC_Flag&0x0111);            
								DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&(DEVICE[i].DEC_Flag&0x0111))>>8);								
								break;
							case 0x2000:
								DEVICE[i].CFGR[4] = 0x00FF&(DEVICE[i].DEC_Flag&0x0222);            
								DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&(DEVICE[i].DEC_Flag&0x0222))>>8);								
								break;
							case 0x4000:
								DEVICE[i].CFGR[4] = 0x00FF&(DEVICE[i].DEC_Flag&0x0444);            
								DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&(DEVICE[i].DEC_Flag&0x0444))>>8);								
								break;
							case 0x8000:
								DEVICE[i].CFGR[4] = 0x00FF&(DEVICE[i].DEC_Flag&0x0888);            
								DEVICE[i].CFGR[5] = LTC6811_DCT_30s|((0x0F00&(DEVICE[i].DEC_Flag&0x0888))>>8);								
								break;
						}
						pecdata = pec15(DEVICE[i].CFGR,6);
						DEVICE[i].CFGR[6] = pecdata>>8;
						DEVICE[i].CFGR[7] = pecdata&0x00FF;
					}
					for(i=0;i<LTC6811_DeviceNUM;i++){					//组织发送数据
						for(j=0;j<8;j++){
							SendData[i*8+j+4] = DEVICE[i].CFGR[j];
						}
					}
					LTC6811_Writecmd(&CMD_WRCFGA);						//发送数据
				}else{																				//均衡计时时间到，切换奇数串和偶数串
					for(i=0;i<LTC6811_DeviceNUM;i++){
						switch(DEVICE[i].DEC_Flag&0xF000){
							case 0x0000:
								DEVICE[i].DEC_Flag &= 0x0FFF;
								DEVICE[i].DEC_Flag |= 0x1000;								
								break;							
							case 0x1000:
								DEVICE[i].DEC_Flag &= 0x0FFF;
								DEVICE[i].DEC_Flag |= 0x2000;								
								break;
							case 0x2000:
								DEVICE[i].DEC_Flag &= 0x0FFF;
								DEVICE[i].DEC_Flag |= 0x4000;								
								break;
							case 0x4000:
								DEVICE[i].DEC_Flag &= 0x0FFF;
								DEVICE[i].DEC_Flag |= 0x8000;								
								break;
							case 0x8000:
								DEVICE[i].DEC_Flag &= 0x0FFF;
								DEVICE[i].DEC_Flag |= 0x1000;								
								break;
						}
					}
					Dec_Time = SYSTimer_num+1000;																											//均衡放电计时设为比系统时间大1000ms
				} 
				break;
			case 24://发送CMD_CLRAUX命令
				LTC6811_Sendcmd(&CMD_CLRAUX);  
				break; 
			case 25://发送CMD_ADAXD命令
				LTC6811_Sendcmd(&CMD_ADAXD);
				break;
			case 34:
				LTC6811_WeekIdle();
				break;
			case 35://读取RDAUXA
				LTC6811_Readcmd(&CMD_RDAUXA);
				break;
			case 37://移动数据到AVAR 读取RDAUXB
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].AVAR,i);       //将接收数据移到AVAR寄存器中 
				LTC6811_Readcmd(&CMD_RDAUXB);
				break;    
			case 39://移动数据到AVBR，判断结果
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].AVBR,i);       //将接收数据移到AVBR寄存器中
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].GPIOVolt_AS[0] = (((uint16_t)DEVICE[i].AVAR[1])<<8)+DEVICE[i].AVAR[0];
					DEVICE[i].GPIOVolt_AS[1] = (((uint16_t)DEVICE[i].AVAR[3])<<8)+DEVICE[i].AVAR[2];
					DEVICE[i].GPIOVolt_AS[2] = (((uint16_t)DEVICE[i].AVAR[5])<<8)+DEVICE[i].AVAR[4];
					DEVICE[i].GPIOVolt_AS[3] = (((uint16_t)DEVICE[i].AVBR[1])<<8)+DEVICE[i].AVBR[0];
					DEVICE[i].GPIOVolt_AS[4] = (((uint16_t)DEVICE[i].AVBR[3])<<8)+DEVICE[i].AVBR[2];
					DEVICE[i].REFVolt_AS = (((uint16_t)DEVICE[i].AVBR[5])<<8)+DEVICE[i].AVBR[4];
				}
				for(i=0;i<LTC6811_DeviceNUM;i++){
					for(j=0;j<5;j++){
						if((DEVICE[i].GPIOVolt_AS[j] > 200) && (DEVICE[i].GPIOVolt_AS[j] < 31000)){
							DEVICE[i].GPIOVolt_CUM[j][CUM_Num] = DEVICE[i].GPIOVolt_AS[j];	//数据存到累加数组
//							DEVICE[i].GPIOVolt[j] = DEVICE[i].GPIOVolt_AS[j];
						}
					}
					if(DEVICE[i].REFVolt_AS != 0xFFFF) DEVICE[i].REFVolt = DEVICE[i].REFVolt_AS;
				}
				break;
			case 40:
				LTC6811_Sendcmd(&CMD_CLRSTAT);
				break;                 
			case 41:
				LTC6811_Sendcmd(&CMD_ADSTATD);
				break;
			case 50:
				LTC6811_WeekIdle();
				break;			
			case 51:
				LTC6811_Readcmd(&CMD_RDSTATA);
				break;
			case 53:
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].STAR,i);
				LTC6811_Readcmd(&CMD_RDSTATB);
				break;    
			case 55:
				for(i=0;i<LTC6811_DeviceNUM;i++) LTC6811_MoveData(DEVICE[i].STBR,i);
				for(i=0;i<LTC6811_DeviceNUM;i++){
					DEVICE[i].SCVolt_AS 	= (((uint16_t)DEVICE[i].STAR[1])<<8)+DEVICE[i].STAR[0];
					DEVICE[i].ITMPVolt_AS = (((uint16_t)DEVICE[i].STAR[3])<<8)+DEVICE[i].STAR[2];
					DEVICE[i].VAVolt_AS 	= (((uint16_t)DEVICE[i].STAR[5])<<8)+DEVICE[i].STAR[4];
					DEVICE[i].VDVolt_AS 	= (((uint16_t)DEVICE[i].STBR[1])<<8)+DEVICE[i].STBR[0];                 
					DEVICE[i].REV_AS 			= DEVICE[i].STBR[5];
				}              
				for(i=0;i<LTC6811_DeviceNUM;i++){
					if(DEVICE[i].SCVolt_AS != 0xFFFF) DEVICE[i].SCVolt = DEVICE[i].SCVolt_AS;
					if(DEVICE[i].ITMPVolt_AS != 0xFFFF) DEVICE[i].ITMPVolt = DEVICE[i].ITMPVolt_AS;
					if(DEVICE[i].VAVolt_AS != 0xFFFF) DEVICE[i].VAVolt = DEVICE[i].VAVolt_AS;
					if(DEVICE[i].VDVolt_AS != 0xFFFF) DEVICE[i].VDVolt = DEVICE[i].VDVolt_AS;                 
					if(DEVICE[i].REV_AS != 0xFF) DEVICE[i].REV = DEVICE[i].REV_AS;
				}
				
				Measure_Num++;
				
				if(CUM_Num < 10){
					CUM_Num++;
				}
				if(CUM_Num == 10){
					CUM_Num = 0;
				}
				
				if(Measure_Num > 22){	//测量次数大于22次后累加数组内数据填充满 计算累加平均值
					for(i=0;i<LTC6811_DeviceNUM;i++){	//取电压的累加平均值赋值给有效值
						for(j=0;j<12;j++){
							for(k=0;k<12;k++){
								CUM_BUFF[k] = 0;
							}
							for(k=0;k<10;k++){
								CUM_BUFF[j] += DEVICE[i].CellVolt_CUM[j][k];
							}
							DEVICE[i].CellVolt[j] = CUM_BUFF[j]/10;
						}
					}
					
					CellVolt_Max_Min();	//找出最大值和最小值
					
					for(i=0;i<LTC6811_DeviceNUM;i++){	//取GPIO的累加平均值赋值给有效值
						for(j=0;j<5;j++){
							for(k=0;k<5;k++){
								CUM_BUFF[k] = 0;
							}
							for(k=0;k<10;k++){
								CUM_BUFF[j] += DEVICE[i].GPIOVolt_CUM[j][k];
							}
							DEVICE[i].GPIOVolt[j] = CUM_BUFF[j]/10;
						}
					}

					for(i=0;i<LTC6811_DeviceNUM;i++){
						for(j=0;j<5;j++){
							DEVICE[i].GPIO_NTCReg[j] =	(10000*DEVICE[i].GPIOVolt[j])/(DEVICE[i].REFVolt - DEVICE[i].GPIOVolt[j]);	
//							DEVICE[i].GPIO_NTCReg[j] =	(10000*DEVICE[i].GPIOVolt[j])/(30000 - DEVICE[i].GPIOVolt[j]);							
						}
					}
				
					//计算LTC68的GPIO测温值	
					for(i=0;i<LTC6811_DeviceNUM;i++){
						for(j=0;j<5;j++){
							buff = LT68_NTC_Table_Lookup(DEVICE[i].GPIO_NTCReg[j]);
							if(buff < 210){
								DEVICE[i].GPIO_NTC_TEMP[j] = buff;
							}
						}
					}				
				
					MAX = DEVICE[0].GPIO_NTC_TEMP[0];
					MIN = DEVICE[0].GPIO_NTC_TEMP[0];
					MAX_NUM = (LTC6811_DeviceNUM-1)*5;
					MIN_NUM = (LTC6811_DeviceNUM-1)*5;
					
					for(i=0;i<LTC6811_DeviceNUM;i++){
						for(j=0;j<5;j++){
							if(DEVICE[i].GPIO_NTC_TEMP[j] > MAX){
								MAX = DEVICE[i].GPIO_NTC_TEMP[j];
								MAX_NUM = (LTC6811_DeviceNUM - 1 - i)*5 + j;
							}
							if(DEVICE[i].GPIO_NTC_TEMP[j] < MIN){
								MIN = DEVICE[i].GPIO_NTC_TEMP[j];	
								MIN_NUM = (LTC6811_DeviceNUM - 1 - i)*5 + j;
							}
						}
					}

//	//大连48V电池只是用DEVICE[0]的GPIO_NTC_TEMP[0]和[4]
//					if(DEVICE[0].GPIO_NTC_TEMP[0] >= DEVICE[0].GPIO_NTC_TEMP[4]){
//						MAX = DEVICE[0].GPIO_NTC_TEMP[0];
//						MAX_NUM = 0;
//						MIN = DEVICE[0].GPIO_NTC_TEMP[4];
//						MIN_NUM = 4;
//					}else{
//						MAX = DEVICE[0].GPIO_NTC_TEMP[4];
//						MAX_NUM = 4;
//						MIN = DEVICE[0].GPIO_NTC_TEMP[0];	
//						MIN_NUM = 0;
//					}
//					
//	//				MAX = 120;
//	//				MAX_NUM = 0;
//	//				MIN = 115;
//	//				MIN_NUM = 4;
//	//大连48V电池只是用DEVICE[0]的GPIO_NTC_TEMP[0]和[4]
					
					for(i=0;i<LTC6811_DeviceNUM;i++){
						DEVICE[i].LT68_NTC_Temp_MAX = MAX;
						DEVICE[i].LT68_NTC_Temp_MIN = MIN;
						DEVICE[i].LT68_NTC_Temp_DELTA = MAX-MIN;
						DEVICE[i].LT68_NTC_Temp_MAX_NUM = MAX_NUM;
						DEVICE[i].LT68_NTC_Temp_MIN_NUM = MIN_NUM;		
					}

					LTC68_Data_Ready = 1;
				}
				LTC6811_Mission_StepCounter = 0;  
				break;   
		}        
	}
}

/*
	END OF FILE
*/
