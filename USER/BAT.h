#ifndef __BAT_H__
#define __BAT_H__
#include "LPC17xx.h"
#include "uart.h"
#include "6811.h"
#include "I2C.h"
#include "ad.h"
#include "BATConfig.h"
#include "IO.h" 
#include "AD7124.h"
#include "CAN.h"

#define CAN_Addr_Base	0x1909F000


#define Port_voltage_CHN 					CH_Volt[1]				//AD7124 通道0电压	（AIN4）
#define Current_CHN 							CH_Volt[0]				//AD7124 通道1电压	（AIN6）
#define Heat_Current_CHN 					1250							//CH_Volt[1]				//AD7124 通道1电压	（AIN6）
#define Current_offset 						1279							//小于为放电，大于为充电
//#define Current_offset 						CH_OffSet[0]							//小于为放电，大于为充电
#define Heat_Current_offset 			1250							//
//#define Heat_Current_offset 			CH_OffSet[1]							//
#define CellVolt_Max_Data 				(DEVICE[0].CellVolt_Max)			
#define CellVolt_Min_Data 				(DEVICE[0].CellVolt_Min)			
#define TEMP_Max_Data 						(DEVICE[0].LT68_NTC_Temp_MAX)						//最高温度值	100作为0度点
#define TEMP_Min_Data 						(DEVICE[0].LT68_NTC_Temp_MIN)						//最低温度值	100作为0度点

#define Cell_Volt_Ave							((DEVICE[0].CellVolt_Max + DEVICE[0].CellVolt_Min)/2)		//单体电压最大值和最小值的平均值作为初始化时根据电压粗判容量的依据
#define Temp_Ave									((TEMP_Max_Data+TEMP_Min_Data)/2)		//温度最大和最小值的平均值，作为获得容量根据温度衰减的系数，以获得实时容量

//DOC和COC判断通过运放对电压的比较，GPIO读取比较输出的逻辑值，XXX_Status为读取到的逻辑值，
//XXX_Pass为判定逻辑的生效值(1:表示XXX_Status为1时发生过流，0：表示XXX_Status为0时发生过流)
#define SC_Status									SC_Read
#define DOC0_Status								DOC0_Read
#define DOC1_Status								DOC1_Read
#define COC_Status								COC_Read
#define SC_Pass										0
#define DOC_Pass									0
#define COC_Pass									0

#define Current_Lowest_threshold	300													//(mA) 检测到的电流低于该值时不计算入容量

#define SC_bit						(1<<0)
#define DOC1_bit					(1<<1)
#define DOC0_bit					(1<<2)
#define COC_bit						(1<<3)
#define CeOV_bit					(1<<4)
#define CeUV_bit					(1<<5)
#define PkOV_bit					(1<<6)
#define PkUV_bit					(1<<7)
#define OT_bit						(1<<8)
#define UT_bit						(1<<9)
#define TUB_bit						(1<<10)
#define CapLow_bit				(1<<16)
#define Freeze_bit				(1<<17)
#define HOC_bit						(1<<18)
#define HOT_bit						(1<<19)
#define MTA_bit						(1<<20)


#define FCCF_bit					(1<<24)
#define CHG_Complete_bit	(1<<17)
#define CHG_Plugged_bit		(1<<16)
#define Heating_bit				(1<<10)
#define CHGing_bit				(1<<9)
#define DSGing_bit				(1<<8)
#define SelfPWR_mos_bit		(1<<3)
#define Heat_mos_bit			(1<<2)
#define CHG_mos_bit				(1<<1)
#define DSG_mos_bit				(1<<0)

#define BAT_Work_Status_DSG_mos_ON		BAT_Work_Status |= DSG_mos_bit
#define BAT_Work_Status_DSG_mos_OFF		BAT_Work_Status &= ~DSG_mos_bit
#define BAT_Work_Status_CHG_mos_ON		BAT_Work_Status |= CHG_mos_bit
#define BAT_Work_Status_CHG_mos_OFF		BAT_Work_Status &= ~CHG_mos_bit
#define BAT_Work_Status_Heat_mos_ON		BAT_Work_Status |= Heat_mos_bit
#define BAT_Work_Status_Heat_mos_OFF	BAT_Work_Status &= ~Heat_mos_bit
#define BAT_Work_Status_SelfPWR_mos_ON		BAT_Work_Status |= SelfPWR_mos_bit
#define BAT_Work_Status_SelfPWR_mos_OFF		BAT_Work_Status &= ~SelfPWR_mos_bit



#define EE_Flag_def	0x5AA5

extern uint8_t EE_DATE_Send[50];

extern uint8_t EE_DATE_Receive[50];
extern uint8_t EE_DATE_Receive_Check[50];
extern uint32_t EE_BAT_Voltage;
extern uint32_t EE_Rem_CAP_mAh;
extern uint8_t EE_Remain_CAP_Percent;
extern uint32_t EE_Full_CAP;
extern uint32_t EE_DSG_CAP_Lifetime_mAh;	
extern uint32_t EE_CHG_CAP_Lifetime_mAh;
extern uint16_t EE_Cycle_Times;
extern uint16_t EE_Flag;

extern uint8_t _Remain_CAP_Percent;

extern uint8_t AlarmLED_Flash_Flag;
extern uint32_t BAT_Timer;
extern uint8_t Remain_CAP_Percent;

extern uint32_t BAT_Work_Status;								//锂电池工作状态
extern uint32_t BAT_Protect_Status;							//锂电池保护状态
extern uint32_t BAT_Protect_Alarm;


extern uint32_t Full_CAP;											//满容量			(mAh)
extern uint32_t Full_CAP_RT;
extern uint32_t Remain_CAP_mAms;							//剩余容量		(mAms)	容量在Remain_CAP_mAms上累加和累减，累加后大于1mAh时向Rem_CAP_mAh进1，累减不够时向Rem_CAP_mAh借1
extern uint32_t Rem_CAP_mAh;									//剩余容量		(mAh)
extern uint32_t Designed_CAP;									//设计容量		(mAh)
extern uint8_t Remain_CAP_Percent;						//剩余容量		(%)		(剩余容量/设计容量)*100%
extern uint32_t Current;											//电流				(mA)	放电时为负，充电时为正，最高位为1表示为负，最高位为0表示为正
extern uint32_t Heat_Current;									//加热电流		(mA)	加热时电流为正
extern uint32_t BAT_Voltage;									//电池组电压	(mV)
extern uint32_t Port_Voltage;									//输出口电压	(mV)
extern uint32_t BAT_Timer;										//同SYSTimer_ms_Counter，为毫秒时钟计数
extern uint32_t CAP_Timer;										//上次完成容量计算时的毫秒时钟数
extern uint32_t CANSend_Timer;
extern uint16_t Cycle_Times;
extern uint32_t DSG_CAP_Lifetime_mAh;
extern uint32_t CHG_CAP_Lifetime_mAh;

extern uint32_t CHGStop_Timer;
extern uint32_t DOC1_Timer;							//放电过流1触发延时和恢复延时计时
extern uint32_t DOC0_Timer;							//放电过流0触发延时和恢复延时计时
extern uint32_t COC_Timer;								//充电过流触发延时和恢复延时计时
extern uint32_t CeOV_Timer;								//单体超压触发延时和恢复延时计时
extern uint32_t CeUV_Timer;								//单体欠压触发延时和恢复延时计时
extern uint32_t PkOV_Timer;								//总电压超压触发延时和恢复延时计时
extern uint32_t PkUV_Timer;								//总电压欠压触发延时和恢复延时计时
extern uint32_t OT_Timer;									//超温触发延时和恢复延时计时
extern uint32_t UT_Timer;									//低温触发延时和恢复延时计时

extern uint32_t DOC_Delay_Timer;					//DOC延时恢复计时
extern uint32_t COC_Delay_Timer;					//COC延时恢复计时
extern uint32_t CeUV_Delay_Timer;					//CeUV延时恢复计时
extern uint32_t CeOV_Delay_Timer;					//CeOV延时恢复计时
extern uint32_t PkUV_Delay_Timer;					//PkUV延时恢复计时
extern uint32_t PkOV_Delay_Timer;					//PkOV延时恢复计时

extern uint8_t CAP_Temp_para;			//容量对温度的衰减系数，百分制，当发生变化时修正满容量和剩余容量
extern uint8_t CAP_Temp_para_old;	//旧值 容量对温度的衰减系数，百分制

uint8_t Array_Check(uint8_t a[],uint8_t b[],uint8_t num);
uint8_t EE_Date_Read_Check(void);
void EE_Date_Save(void);
void EE_Date_Read(void);
void CHG_MOS(uint8_t data);
void DSG_MOS(uint8_t data);
void Heat_MOS(uint8_t data);
void From_Temp_Get_RTCap(void);
void From_CellVolt_Get_RemCapPer(void);
void BAT_Voltage_get(void);
void Port_Voltage_get(void);
void Current_get(void);
void CHG_Plugged_get(void);
void Heat_Current_get(void);
void CAP(void);
void Remain_CAP_Percent_get(void);
void Protection_deal(void);
void BAT_Protect_Status_get_slow(void);
void BAT(void);
void Heat(void);
void BAT_CANSend(void);
void BAT_UartSend(void);












#endif


/*
	END OF FILE
*/
