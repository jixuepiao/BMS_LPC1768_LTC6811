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

#ifndef __6811_H__
#define __6811_H__

#include "BATConfig.h"
#include "LPC17xx.h"
#include "spi.h"
#include "uart.h"
#include "ssp0.h"



#define LTC6811_DeviceNUM 2 

#define LTC6811_BalanceStartVolt 33000		//当最低单体电压值超过此值时开启均衡，或当最高电压值达到此值加1000时开启均衡
#define LTC6811_BalanceThreshold 500			//(0.1mV)	均衡开启的压差阈值
#define LTC6811_ADCSelfTestCode 0x6AAA		//ADC自测试的标准值
#define MAX_Repeat_Num 5									//自检阶段发生错误时的重复检查次数，重复数超过该值则认为真的故障，自检判定不合格
#define BatteryVolt_MAX 43000 						//电压数据有效判断的上限值(0.1mV)
#define BatteryVolt_MIN 25000							//电压数据有效判断的下限值(0.1mV)
#define CHN_Unuse_THD 10000								//测得通道电压低于此值认为该通道未使用

#define LED1_DIR_OUTPUT GPIO_SetDir(1,22,GPIO_DIR_OUTPUT)
#define LED2_DIR_OUTPUT GPIO_SetDir(1,23,GPIO_DIR_OUTPUT)

#define LED1_SET_1 GPIO_PinWrite(1,22,1)
#define LED1_SET_0 GPIO_PinWrite(1,22,0)
#define LED2_SET_1 GPIO_PinWrite(1,23,1)
#define LED2_SET_0 GPIO_PinWrite(1,23,0)


//命令
#define LTC6811_WRCFGA 0x0001		    //写配置寄存器组命令
#define LTC6811_WRCFGB 0x0024		    //写配置寄存器组命令
#define LTC6811_RDCFGA 0x0002		    //读配置寄存器组命令
#define LTC6811_RDCFGB 0x0026		    //读配置寄存器组命令
#define LTC6811_RDCVA 0x0004			//读电池电压寄存器组A命令
#define LTC6811_RDCVB 0x0006			//读电池电压寄存器组B命令
#define LTC6811_RDCVC 0x0008			//读电池电压寄存器组C命令
#define LTC6811_RDCVD 0x000A			//读电池电压寄存器组D命令
#define LTC6811_RDCVE 0x0009			//读电池电压寄存器组E命令
#define LTC6811_RDCVF 0x000B			//读电池电压寄存器组F命令
#define LTC6811_RDAUXA 0x000C		    //读辅助寄存器组A命令
#define LTC6811_RDAUXB 0x000E		    //读辅助寄存器组B命令
#define LTC6811_RDAUXC 0x000D		    //读辅助寄存器组C命令
#define LTC6811_RDAUXD 0x000F		    //读辅助寄存器组D命令
#define LTC6811_RDSTATA 0x0010		    //读状态寄存器组A命令
#define LTC6811_RDSTATB 0x0012		    //读状态寄存器组B命令
#define LTC6811_WRSCTRL 0x0014		    //写S控制寄存器组命令
#define LTC6811_WRPWM 0x0020				//写PWM寄存器组命令
#define LTC6811_WRPSB 0x001C				//写PWM/S控制寄存器组B命令
#define LTC6811_RDSCTRL 0x0016		  //读S控制寄存器组命令
#define LTC6811_RDPWM 0x0022				//读PWM寄存器组命令
#define LTC6811_RDPSB 0x001E				//读PWM/S控制寄存器组B命令
#define LTC6811_STSCTRL 0x0019		  //开始S引脚的脉冲控制，轮询状态
#define LTC6811_CLRSCTRL 0x0018	    //清除S控住寄存器组命令
#define LTC6811_ADCV 0x0260			    //启动电池电压ADC转换，轮询状态
#define LTC6811_ADOW 0x0228			    //启动导线开路ADC转换，轮询状态
#define LTC6811_CVST 0x0207			    //启动自测试电池电压转换，轮询状态
#define LTC6811_ADOL 0x0201			    //启动CELL7电压重复测量命令
#define LTC6811_ADAX 0x0460			    //启动GPIO ADC转换，轮询状态
#define LTC6811_ADAXD 0x0400				//启动带数字冗余的GPIO ADC转换，轮询状态
#define LTC6811_AXST 0x0407			    //启动自测试GPIO转换，轮询状态
#define LTC6811_ADSTAT 0x0468		    //启动状态组(SC总电压 ITMP芯片温度 VD数字电压 VA模拟电压)ADC转换，轮询状态
#define LTC6811_ADSTATD 0x0408		  //启动带数字冗余的状态组(CellsV ITMP ASP DSP)ADC转换，轮询状态
#define LTC6811_STATST 0x040f		    //启动状态组自测试转换，轮询状态
#define LTC6811_ADCVAX 0x046f		    //启动组合电池电压以及GPIO1、GPIO2转换，轮询状态
#define LTC6811_ADCVSC 0x0467		    //启动组合电池电压以及SC转换，轮询状态
#define LTC6811_CLRCELL 0x0711		  //清除电池电压寄存器组命令
#define LTC6811_CLRAUX 0x0712		    //清除辅助寄存器组命令
#define LTC6811_CLRSTAT 0x0713		  //清除状态寄存器组命令
#define LTC6811_PLADC 0x0714				//轮询ADC转换状态命令
#define LTC6811_DIAGN 0x0715				//诊断MUX，轮询状态
#define LTC6811_WRCOMM 0x0721		    //写COMM寄存器组命令
#define LTC6811_RDCOMM 0x0722		    //读COMM寄存器组命令
#define LTC6811_STCOMM 0x0723		    //启动I2C/SPI通信命令
//带参数命令
#define LTC6811_ADCV_7kHz_ALLCell       LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH000
#define LTC6811_ADCV_7kHz_Cell1and7     LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH001
#define LTC6811_ADCV_7kHz_Cell2and8     LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH010
#define LTC6811_ADCV_7kHz_Cell3and9     LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH011
#define LTC6811_ADCV_7kHz_Cell4and10    LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH100
#define LTC6811_ADCV_7kHz_Cell5and11    LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH101
#define LTC6811_ADCV_7kHz_Cell6and12    LTC6811_ADCV|LTC6811_MD10|LTC6811_DCP0|LTC6811_CH110

#define LTC6811_ADOW_7kHz_PD_ALLCell    LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH000
#define LTC6811_ADOW_7kHz_PD_Cell1and7  LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH001
#define LTC6811_ADOW_7kHz_PD_Cell2and8  LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH010
#define LTC6811_ADOW_7kHz_PD_Cell3and9  LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH011
#define LTC6811_ADOW_7kHz_PD_Cell4and10 LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH100
#define LTC6811_ADOW_7kHz_PD_Cell5and11 LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH101
#define LTC6811_ADOW_7kHz_PD_Cell6and12 LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP0|LTC6811_CH110

#define LTC6811_ADOW_7kHz_PU_ALLCell    LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH000
#define LTC6811_ADOW_7kHz_PU_Cell1and7  LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH001
#define LTC6811_ADOW_7kHz_PU_Cell2and8  LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH010
#define LTC6811_ADOW_7kHz_PU_Cell3and9  LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH011
#define LTC6811_ADOW_7kHz_PU_Cell4and10 LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH100
#define LTC6811_ADOW_7kHz_PU_Cell5and11 LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH101
#define LTC6811_ADOW_7kHz_PU_Cell6and12 LTC6811_ADOW|LTC6811_MD10|LTC6811_PUP1|LTC6811_CH110

#define LTC6811_CVST_7kHz_ST10 LTC6811_CVST|LTC6811_MD10|LTC6811_ST10  //7kHz 0x6AAA

#define LTC6811_ADOL_7kHz LTC6811_ADOL|LTC6811_MD10|LTC6811_DCP0  

#define LTC6811_ADAX_7kHz_ALLCHG    LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG000
#define LTC6811_ADAX_7kHz_GPIO1     LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG001
#define LTC6811_ADAX_7kHz_GPIO2     LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG010
#define LTC6811_ADAX_7kHz_GPIO3     LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG011
#define LTC6811_ADAX_7kHz_GPIO4     LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG100
#define LTC6811_ADAX_7kHz_GPIO5     LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG101
#define LTC6811_ADAX_7kHz_2ndRef    LTC6811_ADAX|LTC6811_MD10|LTC6811_CHG110

#define LTC6811_ADAXD_7kHz_ALLCHG   LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG000
#define LTC6811_ADAXD_7kHz_GPIO1    LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG001
#define LTC6811_ADAXD_7kHz_GPIO2    LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG010
#define LTC6811_ADAXD_7kHz_GPIO3    LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG011
#define LTC6811_ADAXD_7kHz_GPIO4    LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG100
#define LTC6811_ADAXD_7kHz_GPIO5    LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG101
#define LTC6811_ADADX_7kHz_2ndRef   LTC6811_ADAXD|LTC6811_MD10|LTC6811_CHG110

#define LTC6811_AXST_7kHz_ST10 LTC6811_AXST|LTC6811_MD10|LTC6811_ST10  //7kHz 0x6AAA

#define LTC6811_ADSTAT_7kHz_ALLCHST LTC6811_ADSTAT|LTC6811_MD10|LTC6811_CHST000
#define LTC6811_ADSTAT_7kHz_SC      LTC6811_ADSTAT|LTC6811_MD10|LTC6811_CHST001
#define LTC6811_ADSTAT_7kHz_ITMP    LTC6811_ADSTAT|LTC6811_MD10|LTC6811_CHST010
#define LTC6811_ADSTAT_7kHz_VA      LTC6811_ADSTAT|LTC6811_MD10|LTC6811_CHST011
#define LTC6811_ADSTAT_7kHz_VD      LTC6811_ADSTAT|LTC6811_MD10|LTC6811_CHST100

#define LTC6811_ADSTATD_7kHz_ALLCHST    LTC6811_ADSTATD|LTC6811_MD10|LTC6811_CHST000
#define LTC6811_ADSTATD_7kHz_SC         LTC6811_ADSTATD|LTC6811_MD10|LTC6811_CHST001
#define LTC6811_ADSTATD_7kHz_ITMP       LTC6811_ADSTATD|LTC6811_MD10|LTC6811_CHST010
#define LTC6811_ADSTATD_7kHz_VA         LTC6811_ADSTATD|LTC6811_MD10|LTC6811_CHST011
#define LTC6811_ADSTATD_7kHz_VD         LTC6811_ADSTATD|LTC6811_MD10|LTC6811_CHST100

#define LTC6811_STATST_7kHz_ST10 LTC6811_STATST|LTC6811_MD10|LTC6811_ST10  //7kHz 0x6AAA

#define LTC6811_ADCVAX_7kHz LTC6811_ADCVAX|LTC6811_MD10|LTC6811_DCP0  //7kHz
#define LTC6811_ADCVSC_7kHz LTC6811_ADCVSC|LTC6811_MD10|LTC6811_DCP0  //7kHz


//命令参数
#define LTC6811_MD00 0x0000			    //转换速率参数：422Hz（ADCOPT：0）；1KHz （ADCOPT：1）
#define LTC6811_MD01 0x0080			    //转换速率参数：27KHz（ADCOPT：0）；14KHz（ADCOPT：1）
#define LTC6811_MD10 0x0100			    //转换速率参数：7KHz （ADCOPT：0）；3KHz （ADCOPT：1）
#define LTC6811_MD11 0x0180			    //转换速率参数：26Hz （ADCOPT：0）；2KHz （ADCOPT：1）

#define LTC6811_DCP1 0x0010			    //测量时放电允许参数
#define LTC6811_DCP0 0x0000			    //测量时放电不允许参数

#define LTC6811_CH000 0x0000			//转换电池选择参数：所有电池
#define LTC6811_CH001 0x0001			//转换电池选择参数：电池1和7
#define LTC6811_CH010 0x0002			//转换电池选择参数：电池2和8
#define LTC6811_CH011 0x0003			//转换电池选择参数：电池3和9
#define LTC6811_CH100 0x0004			//转换电池选择参数：电池4和10
#define LTC6811_CH101 0x0005			//转换电池选择参数：电池5和11
#define LTC6811_CH110 0x0006			//转换电池选择参数：电池6和12

#define LTC6811_PUP1 0x0040			    //导线开路转电流设定参数：上拉电流
#define LTC6811_PUP0 0x0000			    //导线开路转电流设定参数：下拉电流

#define LTC6811_ST01 0x0020			    //自测模式设定参数：自测模式1
#define LTC6811_ST10 0x0040			    //自测模式设定参数：自测模式2

#define LTC6811_CHG000 0x0000		    //GPIO转换脚选择参数：GPIO1-5，第二基准
#define LTC6811_CHG001 0x0001		    //GPIO转换脚选择参数：GPIO1
#define LTC6811_CHG010 0x0002		    //GPIO转换脚选择参数：GPIO2
#define LTC6811_CHG011 0x0003		    //GPIO转换脚选择参数：GPIO3
#define LTC6811_CHG100 0x0004		    //GPIO转换脚选择参数：GPIO4
#define LTC6811_CHG101 0x0005		    //GPIO转换脚选择参数：GPIO5
#define LTC6811_CHG110 0x0006		    //GPIO转换脚选择参数：第二基准

#define LTC6811_CHST000 0x0000		    //状态组选择参数：SC，ITMP，VA，VD
#define LTC6811_CHST001 0x0001		    //状态组选择参数：SC
#define LTC6811_CHST010 0x0002		    //状态组选择参数：ITMP
#define LTC6811_CHST011 0x0003		    //状态组选择参数：VA
#define LTC6811_CHST100 0x0004		    //状态组选择参数：VD

//寄存器参数
#define LTC6811_GPIO(n) (n<<3)			//GPIO控制 bit0～4有效  写0：下拉开  写1：下拉关  读:GPIO逻辑状态
#define LTC6811_REFON(n) (n<<2)         //bit0有效 1：参考电压保持到看门狗超时 0：转换完成之后关闭
#define LTC6811_DETN(n) (n<<1)          //bit0有效 1：使能放电定时器 0：关闭放电定时器
#define LTC6811_ADCOPT(n) (n<<1)        //bit0有效 AD转换频率选择//
#define LTC6811_VUV (((CeUV_Threshold*10)/16)-1)   //
#define LTC6811_VOV (((CeOV_Threshold*10)/16)-1)   //
#define LTC6811_DCT_30s 0x10            //
	

typedef union{
    uint8_t DATA[4];
    struct{
        uint8_t CMD[2];
        uint8_t CMD_PEC[2];
    }STRU;
}CMD_STRUCT; 


typedef struct{
    uint8_t CFGR[8];
    uint8_t RDCFGR[6];    
    uint16_t DEC_Flag;       	//放电标志位  bit0～bit11：为1时表示相应的n+1串需要均衡；
															//bit12～bit15； 要均衡的组数序号。
    uint8_t CVAR[6];
    uint8_t CVBR[6];
    uint8_t CVCR[6];
    uint8_t CVDR[6];
    uint8_t AVAR[6];
    uint8_t AVBR[6];
    uint8_t STAR[6];
    uint8_t STBR[6];
		uint16_t Cell_Select;
    uint16_t CellVolt[12];
    uint16_t CellVolt_AS[12];				//辅助缓存
		uint16_t CellVolt_CUM[12][10];	//累加数据暂存数组 取近10次测量值，有效值为累加后的平均值
    uint16_t CellVolt_Max;    //所有串数中的最大值
    uint16_t CellVolt_Min;    //所有串数中的最小值
    uint16_t CellVolt_DELTA;  //单体电压最大值和最小值差值
    uint16_t MAX_Cell_NUM;    //最高电压的串数	
    uint16_t MIN_Cell_NUM;    //最低电压的串数	
    uint16_t OPENWires;       //bit0～bit12共13位，为1表示相应的C(n)断线
    uint16_t GPIOVolt[5];
		uint16_t GPIOVolt_AS[5];			//辅助缓存
		uint16_t GPIOVolt_CUM[5][10];	//累加数据暂存数组 取近10次测量值，有效值为累加后的平均值
		uint32_t GPIO_NTCReg[5];			//计算得NTC电阻值
		uint16_t GPIO_NTC_TEMP[5];		//通过GPIO接NTC测得温度值
//		uint16_t GPIO_NTC_TEMP_Delta[5];	//通过GPIO接NTC测得温度值		
		uint16_t LT68_NTC_Temp_MAX;
		uint16_t LT68_NTC_Temp_MIN;
		uint16_t LT68_NTC_Temp_DELTA;
		uint16_t LT68_NTC_Temp_MAX_NUM;	//NTC位号
		uint16_t LT68_NTC_Temp_MIN_NUM;				
    uint16_t REFVolt;
    uint16_t REFVolt_AS;	
    uint16_t SCVolt;
		uint16_t SCVolt_AS;
    uint16_t ITMPVolt;
    uint16_t ITMPVolt_AS;		
    uint16_t VAVolt;
    uint16_t VAVolt_AS;		
    uint16_t VDVolt;
    uint16_t VDVolt_AS;		
    uint8_t REV;  //设备版本号,多路选择器状态,热关机状态
    uint8_t REV_AS;		
    uint16_t COV_Flag;
    uint16_t CUV_Flag;
}LTC6811_STRUCT;

extern uint8_t LTC6811_Init_Status;
extern LTC6811_STRUCT DEVICE[LTC6811_DeviceNUM];
extern CMD_STRUCT CMD_WRCFGA;
extern CMD_STRUCT CMD_RDCFGA;
extern CMD_STRUCT CMD_RDCVA;
extern CMD_STRUCT CMD_RDCVB;
extern CMD_STRUCT CMD_RDCVC;
extern CMD_STRUCT CMD_RDCVD;
extern CMD_STRUCT CMD_RDAUXA;
extern CMD_STRUCT CMD_RDAUXB;
extern CMD_STRUCT CMD_RDSTATA;
extern CMD_STRUCT CMD_RDSTATB;
extern CMD_STRUCT CMD_ADCV;
extern CMD_STRUCT CMD_ADOW;
extern CMD_STRUCT CMD_CVST;
extern CMD_STRUCT CMD_ADOL;
extern CMD_STRUCT CMD_ADAX;
extern CMD_STRUCT CMD_ADAXD;
extern CMD_STRUCT CMD_AXST;
extern CMD_STRUCT CMD_ADSTAT;
extern CMD_STRUCT CMD_ADSTATD;
extern CMD_STRUCT CMD_STATST;
extern CMD_STRUCT CMD_ADCVAX;
extern CMD_STRUCT CMD_ADCVSC;
extern CMD_STRUCT CMD_CLRCELL;
extern CMD_STRUCT CMD_CLRAUX;
extern CMD_STRUCT CMD_CLRSTAT;

extern uint32_t Measure_Num;
extern uint16_t LTC6811_Mission_StepCounter;
extern uint32_t SYSTimer_num;
extern uint8_t spi_receivebuff[500];
extern uint32_t LTC68_Data_Ready;

void init_PEC15_Table(void);
uint16_t pec15(uint8_t *data , uint16_t len);
void CMD_init(uint16_t cmd,CMD_STRUCT *data);
void LTC6811_CMD_init(void);
void LTC6811_Sendcmd(CMD_STRUCT *P);
void LTC6811_Readcmd(CMD_STRUCT *P);
void LTC6811_Writecmd(CMD_STRUCT *P);
void LTC6811_WeekIdle(void);
uint8_t LTC6811_MoveData(uint8_t P[],uint8_t DeviceNum);
void CellVolt_Max_Min(void);
void Balance(void);
void LTC6811_UartSend(void);
void LTC6811_Mission(void);


#endif
/*
	END OF FILE
*/
