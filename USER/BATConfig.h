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
#include "LPC17xx.h"

#define COC_Delay_Recovery	//COC延时恢复功能开启	注释掉则关闭
#define DOC0_Delay_Recovery	//DOC0延时恢复功能开启	注释掉则关闭
#define CeUV_Delay_Recovery	//CeUV延时恢复功能开启	注释掉则关闭
#define CeOV_Delay_Recovery	//CeUV延时恢复功能开启	注释掉则关闭
#define PkUV_Delay_Recovery	//PkUV延时恢复功能开启	注释掉则关闭
#define PkOV_Delay_Recovery	//PkUV延时恢复功能开启	注释掉则关闭

#define COC_Delay_Recovery_Time		(30)			//COC延时恢复 (s)
#define DOC0_Delay_Recovery_Time	(30)			//DOC0延时恢复 (s)
#define CeUV_Delay_Recovery_Time	(30)			//CeUV延时恢复 (s)
#define CeOV_Delay_Recovery_Time	(30)			//CeOV延时恢复 (s)
#define PkUV_Delay_Recovery_Time	(30)			//PkUV延时恢复 (s)
#define PkOV_Delay_Recovery_Time	(30)			//PkOV延时恢复 (s)

#define BMS_Cur 									(150)			//BMS消耗电流 (mA)

//#define SC_Threshold 							(10000)		//short current (mA)	通过硬件比较电路实现，通过电位器调节

//#define DOC1_Threshold 						(4000)		//Discharge over current 1 Threshold (mA)	通过硬件比较电路实现，通过电位器调节
//#define DOC1_Delay_Time 					(500)			//Discharge over current 1 Delay Time (us)

#define DOC0_Threshold 						(10000)		//Discharge over current 0 Threshold (mA)	通过硬件比较电路实现，通过电位器调节
#define DOC0_Delay_Time 					(2000)		//Discharge over current 0 Delay Time (us)

#define COC_Threshold 						(10000)		//Charge over current Threshold (mA)	通过硬件比较电路实现，通过电位器调节
#define COC_Delay_Time 						(3000)		//Charge over current Delay Time (us)

#define HOC_Threshold 						(15000)		//Heat over current Threshold (mA)	通过测量得到加热电流值

#define CeOV_Threshold 						(36500)		//Cell over Voltage Threshold (0.1mV)
#define CeOV_Delay_Time 					(55*10)		//Cell over Voltage Delay Time (ms) 电压数值每55ms更新一次
#define CeOV_Recovery_Threshold 	(34000)		//Cell over Voltage Recovery Threshold (0.1mV)

#define CeUV_Threshold 						(25500)		//Cell under Voltage Threshold (0.1mV)
#define CeUV_Delay_Time 					(55*10)		//Cell under Voltage Delay Time (ms) 电压数值每55ms更新一次
#define CeUV_Recovery_Threshold 	(30000)		//Cell under Voltage Recovery Threshold (0.1mV)

#define PkOV_Threshold 						(3650*15)	//Pack over Voltage Threshold (mV)
#define PkOV_Delay_Time 					(55*10)		//Pack over Voltage Delay Time (ms) 电压数值每55ms更新一次
#define PkOV_Recovery_Threshold 	(3400*15)	//Pack over Voltage Recovery Threshold (mV)

#define PkUV_Threshold 						(2550*15)	//Pack under Voltage Threshold (mV)
#define PkUV_Delay_Time 					(50*10)		//Pack under Voltage Delay Time (ms) 电压数值每50ms更新一次
#define PkUV_Recovery_Threshold 	(3000*15)	//Pack under Voltage Recovery Threshold (mV)

#define OT_Threshold 							(60+100)	//Over Temperature Threshold (℃)				100作为0度点
#define OT_Delay_Time 						(10)				//Over Temperature Delay Time (s)
#define OT_Recovery_Threshold 		(50+100)	//Over Temperature Recovery Threshold (℃)			100作为0度点
#define OT_Recovery_Time 					(10)				//Over Temperature Recovery Delay Time (s)

#define UT_Threshold 							(100-40)	//Over Temperature Threshold (℃)			100作为0度点
#define UT_Delay_Time 						(10)				//Over Temperature Delay Time (s)
#define UT_Recovery_Threshold 		(100-35)	//Over Temperature Recovery Threshold (℃)		100作为0度点
#define UT_Recovery_Time 					(10)				//Over Temperature Recovery Delay Time (s)

#define CapLow_Threshold 					(10)				//Capacity Low Threshold (%)		剩余电量百分比低于此值时告警				
#define Freeze_Threshold 					(0+100)		//Freeze Point Threshold (℃)		温度达到5度以下时告警
#define Freeze_Recovery_Threshold (2+100)		//Freeze Protect Recovery Threshold (℃)		零度以下保护的恢复阈值
#define HOT_Threshold 						(50+100)	//Heat Over Temperature Threshold (℃)		加热过程中，最高温度超温阈值

#define TUB_Threshold 						(15)				//Temperature Unbalance (℃)		非加热状态下，多个测温探头，检测温度最大值与最小值之差大于此值时告警

#define DFC 											(20000)		//design full capacity (mAh)
#define MAX_DFC 									(21000)		//design full capacity (mAh)
#define MTA_Alarm_days						(180)			//维护告警天数，从上次关机到这次开机间隔时间大于该天数置维护告警位，关机清零

#define Full_CAP_Cali_Current_Threshold (3000)	//(mA)满容量校准充电电流阈值，在满容量校准标志置位时，充电电流降低到该值时，满容量开始等于剩余容量直至超压保护充电停止

#define CHG_Stop_Current_Threshold (1000)		//mA	充电电流小于此值超过1min认为充电完成
#define CHG_Stop_Timer_Threshold 	(60)				//s		充电电流小于此值超过1min认为充电完成
/*
	END OF FILE
*/
