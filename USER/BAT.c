#include "BAT.h"
/*	BAT_Work_Status各位定义描述
//bit24:FCCF					(Full_CAP_Cali_Flag)满容量校准标志
//bit17:CHG_Complete	充电完成标志
//bit16:CHG_Plugged		充电器接入
//bit10:Heating				工作状态 加热中
//bit9:CHGing					工作状态 充电中		待电流小于CHG_Stop_Current_Threshold超过1min，认为充电完成，容量不再增加。
//bit8:DSGing					工作状态 放电中
//bit3:SelfPWR_mos	  自供电开关状态	1接通 0断开
//bit2:Heat_mos				加热开关状态	1接通 0断开
//bit1:CHG_mos				充电开关状态	1接通 0断开	
//bit0:DSG_mos				放电开关状态	1接通 0断开					*/
uint32_t BAT_Work_Status = 0;									//锂电池工作状态

/*	BAT_Protect_Status各位定义描述
//bit20:MTA						(Maintain Alarm)维护警告
//bit19:HOT						加热超温
//bit18:HOC						加热过流
//bit17:Freeze				0℃以下告警
//bit16:CapLow				电量低告警
//bit10:TUB						温度不均衡告警
//bit9:UT							低温	低于产品的使用温度范围
//bit8:OT							高温	高于产品的使用温度范围
//bit7:PkUV	  				电池组总电压欠压
//bit6:PkOV						电池组总电压超压
//bit5:CeUV	  				单体欠压
//bit4:CeOV						单体超压
//bit3:COC						充电过流
//bit2:DOC0						放电过流0级
//bit1:DOC1						放电过流1级
//bit0:SC							短路					*/
uint32_t BAT_Protect_Status = 0;						//锂电池保护状态
uint32_t BAT_Protect_Alarm = 0;							//锂电池保护告警	在达到触发值但为达到延时值时置位该变量相应位，在达到延时值时置零该变量，置位BAT_Protect_Status，
																						//在BAT_Protect_Status为1时，BAT_Protect_Alarm也为1表示恢复条件达到，延时值未达到
//mos_Status：在Protection_deal(void)函数中应用，为1表示需要打开mos，为0表示需要关闭mos
uint8_t CHG_mos_Status = 0;
uint8_t DSG_mos_Status = 0;
uint8_t Heat_mos_Status = 0;

//根据电压判断电量值时的单体电压参考(0.1mV)
uint16_t 	BatVol_Ref[20] = {27000,29000,30000,30300,30700,31000,31200,31400,31600,31800,32000,32100,32150,32200,32300,32450,32600,32800,33000,33500};
//各电压参考值对应的剩余电量值(%)
uint8_t BatCapPer_Ref[20] = {0,0,0,3,10,17,27,35,43,50,55,60,65,70,75,80,85,90,95,100};
uint8_t _Remain_CAP_Percent;		//根据单体电压估算出的剩余容量百分比
	
uint8_t BatTemp_Ref[20] = 						{60,65,70,75,80,85,90,95,100,	105,110,115,120,125,130,135,140,145,150,155};	//100对应0度
uint8_t BatTemp_CapAttenuation[20] = 	{60,63,67,72,80,84,87,90,92,	95,	98,	100,100,100,100,99,	98,	98,	98,	98};
uint8_t CAP_Temp_para = 100;			//容量对温度的衰减系数，百分制，当发生变化时修正满容量和剩余容量
uint32_t CAP_Temp_para_times = 0;	//
uint8_t CAP_Temp_para_old = 100;	//旧值 容量对温度的衰减系数，百分制
	
//开机后从EEPROM中读取的数值
uint32_t EE_BAT_Voltage;
uint32_t EE_Rem_CAP_mAh;
uint8_t EE_Remain_CAP_Percent;
uint32_t EE_Full_CAP;
uint32_t EE_DSG_CAP_Lifetime_mAh;	
uint32_t EE_CHG_CAP_Lifetime_mAh;	
uint16_t EE_Cycle_Times;
uint16_t EE_Flag;		//EE存储结尾标志 EE_Flag_def
	
uint32_t CHGStop_Timer = 0;						//充电电流小于CHG_Stop_Current_Threshold时开始计时，当计时大于	
uint32_t DOC1_Timer = 0;							//放电过流1触发延时和恢复延时计时
uint32_t DOC0_Timer = 0;							//放电过流0触发延时和恢复延时计时
uint32_t COC_Timer = 0;								//充电过流触发延时和恢复延时计时																						
uint32_t CeOV_Timer = 0;							//单体超压触发延时和恢复延时计时
uint32_t CeUV_Timer = 0;							//单体欠压触发延时和恢复延时计时
uint32_t PkOV_Timer = 0;							//总电压超压触发延时和恢复延时计时
uint32_t PkUV_Timer = 0;							//总电压欠压触发延时和恢复延时计时
uint32_t OT_Timer = 0;								//超温触发延时和恢复延时计时
uint32_t UT_Timer = 0;								//低温触发延时和恢复延时计时
uint32_t BAT_Timer = 0;								//同SYSTimer_ms_Counter，为毫秒时钟计数
uint32_t CAP_Timer = 0;								//上次完成容量计算时的毫秒时钟数
uint32_t CANSend_Timer = 0;						//同SYSTimer_ms_Counter，CAN发送计时
uint32_t DOC_Delay_Timer = 0;					//DOC延时恢复计时
uint32_t COC_Delay_Timer = 0;					//COC延时恢复计时
uint32_t CeUV_Delay_Timer = 0;				//CeUV延时恢复计时
uint32_t CeOV_Delay_Timer = 0;				//CeOV延时恢复计时
uint32_t PkUV_Delay_Timer = 0;				//PkUV延时恢复计时
uint32_t PkOV_Delay_Timer = 0;				//PkOV延时恢复计时

uint32_t CeUV_Counter = 0;							
uint32_t CeOV_Counter = 0;
uint32_t PkUV_Counter = 0;							
uint32_t PkOV_Counter = 0;
uint32_t OT_Counter = 0;							
uint32_t UT_Counter = 0;							
uint32_t Freeze_Counter = 0;
uint32_t TUB_Counter = 0;

uint32_t Full_CAP = 20000;										//满容量			(mAh)
uint32_t Full_CAP_RT = 20000;									//实时满容量	(mAh)	RT：RealTime
uint32_t Remain_CAP_mAms;						//剩余容量		(mAms)	容量在Remain_CAP_mAms上累加和累减，累加后大于1mAh时向Rem_CAP_mAh进1，累减不够时向Rem_CAP_mAh借1
uint32_t Rem_CAP_mAh = 20000;									//剩余容量		(mAh)
//uint32_t Remain_CAP_RT_mAh;						//剩余容量		(mAh)	RT：RealTime
uint32_t Designed_CAP = DFC;					//设计容量		(mAh)
uint8_t Remain_CAP_Percent;						//剩余容量		(%)		(剩余容量/设计容量)*100%
uint32_t Current = 0;									//电流				(mA)	放电时为负，充电时为正，最高位为1表示为负，最高位为0表示为正
uint16_t DSG_Rate = 0;								//放电倍率		以与100的比值作为倍率值，100对应1C、50对应0.5C、200对应2C
uint16_t CHG_Rate = 0;								//充电倍率		以与100的比值作为倍率值，100对应1C、50对应0.5C、200对应2C
uint32_t Heat_Current = 0;						//加热电流		(mA)	加热时电流为正
uint32_t BAT_Voltage = 0;							//电池组电压	(mV)
uint32_t Port_Voltage = 0;						//输出口电压	(mV)

uint32_t DSG_CAP_Lifetime_mAh;				//电池累计放电量
uint32_t DSG_CAP_Lifetime_mAms;				//电池累计放电量	满1mAh向DSG_CAP_Lifetime_mAh进1
uint32_t CHG_CAP_Lifetime_mAh;				//电池累计充电量
uint32_t CHG_CAP_Lifetime_mAms;				//电池累计充电量	满1mAh向CHG_CAP_Lifetime_mAh进1
uint16_t Cycle_Times;									//循环次数		DSG_CAP_Lifetime_mAh/DFC 总放电量除以设计容量

uint8_t Full_CAP_Cali_Flag = 0;				//满容量校准标志，校准方式：在常温下进行，先放电至欠压，在放电欠压后Full_CAP_Cali_Flag置1，此时清零剩余容量
																			//然后不关机直接进行充电，充电至电流降到Full_CAP_Cali_Current_Threshold时满容量开始始终等于剩余容量
																			//待电流小于CHG_Stop_Current_Threshold超过1min，认为充电完成，容量不再增加。
uint8_t AlarmLED_Flash = 0;															
uint8_t AlarmLED_Flash_Flag = 0;			//报警灯闪烁标志，为1时开始闪烁，为0时停止
uint8_t AlarmLED_Light_Flag = 0;			//报警灯常亮标志
uint8_t BATUartSendData[500];

uint8_t BATUartSendbuff[30];

uint8_t EE_DATE_Send[50] = {0xA0,0x00,0x01};
uint8_t EE_DATE_Receive[50] = {0xA0,0x00,0x01,0xA1};
uint8_t EE_DATE_Receive_Check[50] = {0xA0,0x00,0x01,0xA1};

//// CAP函数用
uint32_t buff_mAh = 0,buff_mAms = 0;
//uint32_t buff = 0,buff1 = 0;

uint8_t Array_Check(uint8_t a[],uint8_t b[],uint8_t num){
	uint8_t i,c;
	c = 1;
	for(i=0;i<num;i++){
		if(a[i] != b[i])	c = 0;
	}
	return c; 
}
void EE_Date_Save(void){
	EE_DATE_Send[0] = 0xA0;
	EE_DATE_Send[1] = 0x01;
	EE_DATE_Send[2] = 0x01;	
	EE_DATE_Send[3] = (BAT_Voltage&0xFF000000)>>24;
	EE_DATE_Send[4] = (BAT_Voltage&0x00FF0000)>>16;	
	EE_DATE_Send[5] = (BAT_Voltage&0x0000FF00)>>8;	
	EE_DATE_Send[6] = (BAT_Voltage&0x000000FF)>>0;	
	EE_DATE_Send[7] = (Rem_CAP_mAh&0xFF000000)>>24;
	EE_DATE_Send[8] = (Rem_CAP_mAh&0x00FF0000)>>16;	
	EE_DATE_Send[9] = (Rem_CAP_mAh&0x0000FF00)>>8;	
	EE_DATE_Send[10] = (Rem_CAP_mAh&0x000000FF)>>0;	
	EE_DATE_Send[11] = Remain_CAP_Percent;		
	EE_DATE_Send[12] = (Full_CAP&0xFF000000)>>24;
	EE_DATE_Send[13] = (Full_CAP&0x00FF0000)>>16;	
	EE_DATE_Send[14] = (Full_CAP&0x0000FF00)>>8;	
	EE_DATE_Send[15] = (Full_CAP&0x000000FF)>>0;
	EE_DATE_Send[16] = (DSG_CAP_Lifetime_mAh&0xFF000000)>>24;
	EE_DATE_Send[17] = (DSG_CAP_Lifetime_mAh&0x00FF0000)>>16;	
	EE_DATE_Send[18] = (DSG_CAP_Lifetime_mAh&0x0000FF00)>>8;	
	EE_DATE_Send[19] = (DSG_CAP_Lifetime_mAh&0x000000FF)>>0;	
	EE_DATE_Send[20] = (CHG_CAP_Lifetime_mAh&0xFF000000)>>24;
	EE_DATE_Send[21] = (CHG_CAP_Lifetime_mAh&0x00FF0000)>>16;	
	EE_DATE_Send[22] = (CHG_CAP_Lifetime_mAh&0x0000FF00)>>8;	
	EE_DATE_Send[23] = (CHG_CAP_Lifetime_mAh&0x000000FF)>>0;	
	EE_DATE_Send[24] = (Cycle_Times&0xFF00)>>8;	
	EE_DATE_Send[25] = (Cycle_Times&0x00FF)>>0;	
	
	EE_DATE_Send[26] = EE_Flag_def>>8;	
	EE_DATE_Send[27] = EE_Flag_def&0x00FF;	
}

uint8_t EE_Date_Read_Check(void){
	uint8_t i;
	EE_DATE_Receive[0] = 0xA0;
	EE_DATE_Receive[1] = 0x00;
	EE_DATE_Receive[2] = 0x01;
	EE_DATE_Receive[3] = 0xA1;
	EE_BAT_Voltage = ((uint32_t)EE_DATE_Receive[4]<<24)+((uint32_t)EE_DATE_Receive[5]<<16)+((uint32_t)EE_DATE_Receive[6]<<8)+((uint32_t)EE_DATE_Receive[7]<<0);
	EE_Rem_CAP_mAh = ((uint32_t)EE_DATE_Receive[8]<<24)+((uint32_t)EE_DATE_Receive[9]<<16)+((uint32_t)EE_DATE_Receive[10]<<8)+((uint32_t)EE_DATE_Receive[11]<<0);
	EE_Remain_CAP_Percent = EE_DATE_Receive[12];
	EE_Full_CAP = ((uint32_t)EE_DATE_Receive[13]<<24)+((uint32_t)EE_DATE_Receive[14]<<16)+((uint32_t)EE_DATE_Receive[15]<<8)+((uint32_t)EE_DATE_Receive[16]<<0);
	EE_DSG_CAP_Lifetime_mAh = ((uint32_t)EE_DATE_Receive[17]<<24)+((uint32_t)EE_DATE_Receive[18]<<16)+((uint32_t)EE_DATE_Receive[19]<<8)+((uint32_t)EE_DATE_Receive[20]<<0);
	EE_CHG_CAP_Lifetime_mAh = ((uint32_t)EE_DATE_Receive[21]<<24)+((uint32_t)EE_DATE_Receive[22]<<16)+((uint32_t)EE_DATE_Receive[23]<<8)+((uint32_t)EE_DATE_Receive[24]<<0);
	EE_Cycle_Times = ((uint16_t)EE_DATE_Receive[25]<<8)+((uint16_t)EE_DATE_Receive[26]<<0);
	EE_Flag = ((uint16_t)EE_DATE_Receive[27]<<8)+((uint16_t)EE_DATE_Receive[28]<<0);

	i = 1;
	if(EE_BAT_Voltage != BAT_Voltage) i = 0;
	if(EE_Rem_CAP_mAh != Rem_CAP_mAh) i = 0;
	if(EE_Remain_CAP_Percent != Remain_CAP_Percent) i = 0;
	if(EE_Full_CAP != Full_CAP) i = 0;	
	if(EE_DSG_CAP_Lifetime_mAh != DSG_CAP_Lifetime_mAh) i = 0;
	if(EE_CHG_CAP_Lifetime_mAh != CHG_CAP_Lifetime_mAh) i = 0;	
	if(EE_Cycle_Times != Cycle_Times) i = 0;
	if(EE_Flag != EE_Flag_def) i = 0;
	return i;
}

void EE_Date_Read(void){
	EE_DATE_Receive[0] = 0xA0;
	EE_DATE_Receive[1] = 0x01;
	EE_DATE_Receive[2] = 0x01;
	EE_DATE_Receive[3] = 0xA1;
	EE_BAT_Voltage = ((uint32_t)EE_DATE_Receive[4]<<24)+((uint32_t)EE_DATE_Receive[5]<<16)+((uint32_t)EE_DATE_Receive[6]<<8)+((uint32_t)EE_DATE_Receive[7]<<0);
	EE_Rem_CAP_mAh = ((uint32_t)EE_DATE_Receive[8]<<24)+((uint32_t)EE_DATE_Receive[9]<<16)+((uint32_t)EE_DATE_Receive[10]<<8)+((uint32_t)EE_DATE_Receive[11]<<0);
	EE_Remain_CAP_Percent = EE_DATE_Receive[12];
	EE_Full_CAP = ((uint32_t)EE_DATE_Receive[13]<<24)+((uint32_t)EE_DATE_Receive[14]<<16)+((uint32_t)EE_DATE_Receive[15]<<8)+((uint32_t)EE_DATE_Receive[16]<<0);
	EE_DSG_CAP_Lifetime_mAh = ((uint32_t)EE_DATE_Receive[17]<<24)+((uint32_t)EE_DATE_Receive[18]<<16)+((uint32_t)EE_DATE_Receive[19]<<8)+((uint32_t)EE_DATE_Receive[20]<<0);
	EE_CHG_CAP_Lifetime_mAh = ((uint32_t)EE_DATE_Receive[21]<<24)+((uint32_t)EE_DATE_Receive[22]<<16)+((uint32_t)EE_DATE_Receive[23]<<8)+((uint32_t)EE_DATE_Receive[24]<<0);
	EE_Cycle_Times = ((uint16_t)EE_DATE_Receive[25]<<8)+((uint16_t)EE_DATE_Receive[26]<<0);
	EE_Flag = ((uint16_t)EE_DATE_Receive[27]<<8)+((uint16_t)EE_DATE_Receive[28]<<0);
}

void CHG_MOS(uint8_t data){
	if(data == 1){
		CHG_ON;
		BAT_Work_Status_CHG_mos_ON;
	}else{
		CHG_OFF;
		BAT_Work_Status_CHG_mos_OFF;
	}
}
void DSG_MOS(uint8_t data){
	if(data == 1){
		DSG_ON;
		BAT_Work_Status_DSG_mos_ON;
	}else{
		DSG_OFF;
		BAT_Work_Status_DSG_mos_OFF;
	}
}
void Heat_MOS(uint8_t data){
	if(data == 1){
		Heat_ON;
		BAT_Work_Status_Heat_mos_ON;
	}else{
		Heat_OFF;
		BAT_Work_Status_Heat_mos_OFF;
	}
}

void From_Temp_Get_RTCap(void){	//对容量增加温度系数，通过温度查找容量衰减系数，获得实时容量
	uint8_t	i;
	uint16_t buff;
	buff = TEMP_Min_Data;
	if(buff < BatTemp_Ref[0])		CAP_Temp_para = BatTemp_CapAttenuation[0];
	if(buff > BatTemp_Ref[19])	CAP_Temp_para = BatTemp_CapAttenuation[19];
	for(i=0;i<19;i++){
		if((buff > BatTemp_Ref[i]) && (buff < BatTemp_Ref[i+1]))	CAP_Temp_para = BatTemp_CapAttenuation[i];
	}
}

void From_CellVolt_Get_RemCapPer(void){	//根据单体电压估算剩余容量百分比 估算结果为_Remain_CAP_Percent
	uint8_t	i;
	if(Cell_Volt_Ave < BatVol_Ref[0])	_Remain_CAP_Percent = 0;
	if(Cell_Volt_Ave > BatVol_Ref[19])	_Remain_CAP_Percent = 100;
	for(i=0;i<19;i++){
		if((Cell_Volt_Ave > BatVol_Ref[i]) && (Cell_Volt_Ave < BatVol_Ref[i+1]))	_Remain_CAP_Percent = BatCapPer_Ref[i];
	}
}

void BAT_Voltage_get(void){
	uint8_t i;
	uint32_t buff = 0;
	for(i=0;i<LTC6811_DeviceNUM;i++){
		buff += (DEVICE[i].SCVolt * 2);
	}
	BAT_Voltage = buff;
}

void Port_Voltage_get(void){
	Port_Voltage = 200*Port_voltage_CHN;
}

void Current_get(void){
	uint32_t cur_buff = 0;
	if(Current_CHN <= Current_offset){											//放电
		cur_buff = (20*(Current_offset - Current_CHN));	//buff为电流值，单位为mA
		DSG_Rate = (100*cur_buff)/Designed_CAP;										//计算放电倍率
		if(cur_buff > Current_Lowest_threshold){									//电流大于最低阈值，计入充放电
			Current = 0x80000000|cur_buff;
			BAT_Work_Status |= DSGing_bit;											//置位 放电中 标志
			BAT_Work_Status &= ~CHGing_bit;											//清位 充电中 标志
		}else{																								//电流小于最低阈值，不计入充放电
			Current = 0;
			BAT_Work_Status &= ~CHGing_bit;											//清位 充电中 标志
			BAT_Work_Status &= ~DSGing_bit;											//清位 放电中 标志
		}
	}else{																									//充电
		cur_buff = (20*(Current_CHN - Current_offset));	//buff为电流值，单位为mA
		CHG_Rate = (100*cur_buff)/Designed_CAP;										//计算充电倍率
		if(cur_buff > Current_Lowest_threshold){
			Current = cur_buff;
			BAT_Work_Status |= CHGing_bit;											//置位 充电中 标志
			BAT_Work_Status &= ~DSGing_bit;											//清位 放电中 标志
		}else{																								//电流小于最低阈值，不计入充放电
			Current = 0;
			BAT_Work_Status &= ~CHGing_bit;											//清位 充电中 标志
			BAT_Work_Status &= ~DSGing_bit;											//清位 放电中 标志
		}
	}
}

void CHG_Plugged_get(void){
	if((BAT_Work_Status&CHGing_bit) > 0)	BAT_Work_Status |= CHG_Plugged_bit;	//充电器接入标志置位
	else																	BAT_Work_Status &= ~CHG_Plugged_bit;
	if((Port_Voltage-10000) > BAT_Voltage)				BAT_Work_Status |= CHG_Plugged_bit;	//充电器接入标志置位
	else																	BAT_Work_Status &= ~CHG_Plugged_bit;	
}
void Heat_Current_get(void){
	uint32_t buff = 0;
	if(Heat_Current_CHN > Heat_Current_offset){	//加热
		buff = 0;
		if(buff > Current_Lowest_threshold){
			Heat_Current = buff;
			BAT_Work_Status |= Heating_bit;	//置加热位
		}else{
			BAT_Work_Status &= ~Heating_bit;	//清加热位
		}
	}
}

void CAP(void){

	//放电
	if((BAT_Work_Status&DSGing_bit) == DSGing_bit){				//判断为放电中
		//计算历史放电量
		DSG_CAP_Lifetime_mAms += (Current&0x7FFFFFFF);
		if(DSG_CAP_Lifetime_mAms > (3600*1000)){
			DSG_CAP_Lifetime_mAh += DSG_CAP_Lifetime_mAms / (3600*1000);
			DSG_CAP_Lifetime_mAms = DSG_CAP_Lifetime_mAms % (3600*1000);
		}
		
		//计算剩余容量 减少
		if(Remain_CAP_mAms > (Current&0x7FFFFFFF)){
			Remain_CAP_mAms -= (Current&0x7FFFFFFF);
		}else{
			if(Rem_CAP_mAh > 0){
				Rem_CAP_mAh--;		
				Remain_CAP_mAms += 3600*1000;
				Remain_CAP_mAms -= (Current&0x7FFFFFFF);
			}else{
				Rem_CAP_mAh = 0;
				Remain_CAP_mAms = 0;			
			}
		}
//		BAT_Work_Status &= ~CHG_Complete_bit;	//清充电完成标志
//		BAT_Work_Status &= ~FCCF_bit;					//清满容量校准标志
	}
	
	//充电
	if((BAT_Work_Status&CHGing_bit) == CHGing_bit){
		if((BAT_Work_Status&CHG_Complete_bit) == 0){	//充电未完成
			
			//计算历史充电量
			CHG_CAP_Lifetime_mAms += (Current&0x7FFFFFFF);						//加mAms
			if(CHG_CAP_Lifetime_mAms > (3600*1000)){									//判断加mAms后，剩余mAms是否大于1mAh，是则加到剩余mAh中
				CHG_CAP_Lifetime_mAh += CHG_CAP_Lifetime_mAms/(3600*1000);
				CHG_CAP_Lifetime_mAms = CHG_CAP_Lifetime_mAms%(3600*1000);
			}				
			
			//计算剩余容量 增加
			Remain_CAP_mAms += (Current&0x7FFFFFFF);						//加mAms
			if(Remain_CAP_mAms > (3600*1000)){									//判断加mAms后，剩余mAms是否大于1mAh，是则加到剩余mAh中
				Rem_CAP_mAh++;
				Remain_CAP_mAms = Remain_CAP_mAms - (3600*1000);
				if(Rem_CAP_mAh >= MAX_DFC){
					Rem_CAP_mAh = MAX_DFC;
					Remain_CAP_mAms = 0;
				}
			}
			
			if(CellVolt_Max_Data>35500){
				Full_CAP = Full_CAP_RT = Rem_CAP_mAh;
			}
			
//			if((BAT_Work_Status&FCCF_bit) == FCCF_bit){
//				if(CellVolt_Max_Data>35500){
//					Full_CAP = Full_CAP_RT = Rem_CAP_mAh;
//				}
//			}else{
//				if(Rem_CAP_mAh > Full_CAP_RT) Full_CAP_RT = Rem_CAP_mAh;	//剩余容量大于满容量时更新满容量
//				if(Full_CAP_RT > Full_CAP) Full_CAP = Full_CAP_RT;
//			}
		}
	}

	//计算自耗电
	DSG_CAP_Lifetime_mAms += BMS_Cur;											//将自耗电加入到历史放电容量中
	if(Remain_CAP_mAms > BMS_Cur){
		Remain_CAP_mAms -= BMS_Cur;
	}else{
		if(Rem_CAP_mAh > 0){
			Rem_CAP_mAh--;
			Remain_CAP_mAms = Remain_CAP_mAms+3600*1000-BMS_Cur;
		}else{
			Rem_CAP_mAh = 0;
			Remain_CAP_mAms = 0;			
		}
	}
	
	Cycle_Times = DSG_CAP_Lifetime_mAh/((DFC*80)/100);					//计算循环次数	历史放电量除以设计容量的80%
}

void Remain_CAP_Percent_get(void){	
	Remain_CAP_Percent = (100*Rem_CAP_mAh)/Full_CAP;
	if(((100*Rem_CAP_mAh)%Full_CAP)>(Full_CAP/2))	Remain_CAP_Percent += 1;
	if(Remain_CAP_Percent > 100){
		Remain_CAP_Percent = (100*Rem_CAP_mAh)/Full_CAP;
		if(((100*Rem_CAP_mAh)%Full_CAP)>(Full_CAP/2))	Remain_CAP_Percent += 1;
		if(Remain_CAP_Percent > 100){
			Remain_CAP_Percent = (100*Rem_CAP_mAh)/Full_CAP;
			if(((100*Rem_CAP_mAh)%Full_CAP)>(Full_CAP/2))	Remain_CAP_Percent += 1;
			if(Remain_CAP_Percent > 100) Remain_CAP_Percent = 100;
		}
	}	
	
//	Remain_CAP_Percent = (100*Rem_CAP_mAh)/Full_CAP_RT;
//	if(((100*Rem_CAP_mAh)%Full_CAP_RT)>(Full_CAP_RT/2))	Remain_CAP_Percent += 1;
//	if(Remain_CAP_Percent > 100){
//		Remain_CAP_Percent = (100*Rem_CAP_mAh)/Full_CAP_RT;
//		if(((100*Rem_CAP_mAh)%Full_CAP_RT)>(Full_CAP_RT/2))	Remain_CAP_Percent += 1;
//		if(Remain_CAP_Percent > 100){
//			Remain_CAP_Percent = (100*Rem_CAP_mAh)/Full_CAP_RT;
//			if(((100*Rem_CAP_mAh)%Full_CAP_RT)>(Full_CAP_RT/2))	Remain_CAP_Percent += 1;
//			if(Remain_CAP_Percent > 100) Remain_CAP_Percent = 100;
//		}
//	}
	
}


//加热过流		告警灯常亮，关闭加热，重新开机恢复
//无保护状态 	充放电允许
//电量低告警	闪烁告警灯
//零度以下		在放电状态下允许充放电，在充电状态下禁止充电、允许放电，在没有电流的情况下允许充放电，达到零度以上时恢复
//单体超压		在放电状态下允许充放电，在充电状态下禁止充电、允许放电，在没有电流的情况下允许充放电，达到恢复条件时恢复
//总体超压		在放电状态下允许充放电，在充电状态下禁止充电、允许放电，在没有电流的情况下允许充放电，达到恢复条件时恢复
//单体欠压		在放电状态下禁止放电、允许充电，在充电状态下允许充放电，在没有电流的情况下允许充放电，达到恢复条件时恢复
//总体欠压		在放电状态下禁止放电、允许充电，在充电状态下允许充放电，在没有电流的情况下允许充放电，达到恢复条件时恢复
//高温和低温	禁止放电，禁止充电，达到恢复条件时恢复
void Protection_deal(void){
	AlarmLED_Light_Flag = 0;
	AlarmLED_Flash = 0;
	CHG_mos_Status = 1;
	DSG_mos_Status = 1;
//////维护告警//////////////////////////////////////////////////////////////////////////////////////////////////
	if((BAT_Protect_Status&MTA_bit)==MTA_bit){		//维护告警
		AlarmLED_Flash = 1;								//告警灯闪烁标志置位
	}		
//////低电量告警//////////////////////////////////////////////////////////////////////////////////////////////////
	if((BAT_Protect_Status&CapLow_bit)==CapLow_bit){		//低电量告警
		AlarmLED_Flash = 1;								//告警灯闪烁标志置位
	}
//////温度不均衡//////////////////////////////////////////////////////////////////////////////////////////////////
	if((BAT_Protect_Status&TUB_bit)==TUB_bit){				//温度不均衡
		AlarmLED_Flash = 1;								//告警灯闪烁标志置位
	}		
//////零度以下////////////////////////////////////////////////////////////////////////////////////////////////////
	if(((BAT_Protect_Status&Freeze_bit)==Freeze_bit)){		//零度以下
		CHG_mos_Status = 0;
		BAT_Work_Status &= ~FCCF_bit;
	}
//////加热超温、加热过流//////////////////////////////////////////////////////////////////////////////////////////
		if(((BAT_Protect_Status&HOT_bit)>0)||((BAT_Protect_Status&HOC_bit)>0)){				//加热超温、加热过流
//			CHG_mos_Status = 0;
//			DSG_mos_Status = 0;
//			Heat_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯
		}		
//////单体超压、总体超压//////////////////////////////////////////////////////////////////////////////////////////		
		if((BAT_Protect_Status&CeOV_bit)==CeOV_bit){		//单体超压、总体超压
			CHG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯
		}
		if((BAT_Protect_Status&PkOV_bit)==PkOV_bit){		//单体超压、总体超压
//			CHG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯
		}		
		
//////单体欠压、总体欠压//////////////////////////////////////////////////////////////////////////////////////////			
		if((BAT_Protect_Status&CeUV_bit)==CeUV_bit){//单体欠压
			BAT_Work_Status |= FCCF_bit;	//置满容量校准标志
			Rem_CAP_mAh = 0;
			Remain_CAP_mAms = 0;
			Remain_CAP_Percent = 0;
			DSG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯
		}
		if((BAT_Protect_Status&PkUV_bit)==PkUV_bit){//总体欠压
//			DSG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯
		}		
		
///////高温、低温/////////////////////////////////////////////////////////////////////////////////////////////////		
		if(((BAT_Protect_Status&OT_bit)==OT_bit)||((BAT_Protect_Status&UT_bit)==UT_bit)){//高温、低温
			CHG_mos_Status = 0;
			DSG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯		
		}	
///////充电过流/////////////////////////////////////////////////////////////////////////////////////////////////		
		if((BAT_Protect_Status&COC_bit)==COC_bit){
			CHG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯		
		}
///////放电过流/////////////////////////////////////////////////////////////////////////////////////////////////		
		if((BAT_Protect_Status&DOC0_bit)==DOC0_bit){
			DSG_mos_Status = 0;
			AlarmLED_Light_Flag = 1;	//点亮报警灯		
		}
///////////////////////////////////////////////////////////////////////////////////////////////
			
	if(CHG_mos_Status > 0)	CHG_MOS(1);
	else{
		CHG_MOS(0);		
	}
	if(DSG_mos_Status > 0)	DSG_MOS(1);
	else{										
		DSG_MOS(0);	
	}
	if(Heat_mos_Status > 0)	Heat_MOS(1);
	else										Heat_MOS(0);
		
	if(AlarmLED_Light_Flag > 0){
		Light_AlarmLED;
		AlarmLED_Flash_Flag = 0;
	}else{
		if(AlarmLED_Flash >0){
			AlarmLED_Flash_Flag = 1;
		}else{
			UnLight_AlarmLED;
			AlarmLED_Flash_Flag = 0;		
		}
	}	
}

void BAT_Protect_Status_get_slow(void){
//COC get
	if((BAT_Work_Status&CHGing_bit) > 0){								//判断电流为充电电流
//		BAT_Protect_Status &= ~COC_bit;						//清保护位
		if((Current&0x7FFFFFFF) > COC_Threshold){
			BAT_Protect_Status |= COC_bit;						//置告警位
		}
	}
	
//DOC get
	if((BAT_Work_Status&DSGing_bit) > 0){								//判断电流为放电电流
//		BAT_Protect_Status &= ~DOC0_bit;						//清保护位
		if((Current&0x7FFFFFFF) > DOC0_Threshold){
			BAT_Protect_Status |= DOC0_bit;						//置告警位
		}
	}

//CapLow get
	if(Remain_CAP_Percent < CapLow_Threshold){
		BAT_Protect_Status |= CapLow_bit;
	}else{
		BAT_Protect_Status &= ~CapLow_bit;
	}


	if(LTC68_Data_Ready == 1){
		LTC68_Data_Ready = 0;
		//CeUV get
		if(CellVolt_Min_Data < CeUV_Threshold){
			CeUV_Counter++;
			if(CeUV_Counter > 60)
				BAT_Protect_Status |= CeUV_bit;						//置告警位
		}else CeUV_Counter = 0;
		
	//CeOV get
		if(CellVolt_Max_Data > CeOV_Threshold){
			CeOV_Counter++;
			if(CeOV_Counter > 40) BAT_Protect_Status |= CeOV_bit;						//置告警位
		}else CeOV_Counter = 0;
		
	//PkUV get
		if(BAT_Voltage < PkUV_Threshold){
			PkUV_Counter++;
			if(PkUV_Counter > 40)	BAT_Protect_Status |= PkUV_bit;						//置告警位
		}else	PkUV_Counter = 0;
	//PkOV get
		if(BAT_Voltage > PkOV_Threshold){
			PkOV_Counter++;
			if(PkOV_Counter > 40)	BAT_Protect_Status |= PkOV_bit;						//置告警位
		}else	PkOV_Counter = 0;
	//OT get
		if(TEMP_Max_Data > OT_Threshold){
			OT_Counter++;
			if(OT_Counter > 100) BAT_Protect_Status |= OT_bit;	//置保护位
		}
		if(TEMP_Max_Data < OT_Recovery_Threshold){
			OT_Counter = 0;
			BAT_Protect_Status &= ~OT_bit;	//清保护位
		}	
	//UT get
		if(TEMP_Min_Data < UT_Threshold){
			UT_Counter++;
			if(UT_Counter > 100) BAT_Protect_Status |= UT_bit;	//置保护位
		}
		if(TEMP_Min_Data > UT_Recovery_Threshold){
			UT_Counter = 0;
			BAT_Protect_Status &= ~UT_bit;	//清保护位
		}
		
	//Freeze get
		if(TEMP_Min_Data < Freeze_Threshold){
			Freeze_Counter++;
			if(Freeze_Counter > 100) BAT_Protect_Status |= Freeze_bit;
		}
		if(TEMP_Min_Data > Freeze_Recovery_Threshold){
			Freeze_Counter = 0;
			BAT_Protect_Status &= ~Freeze_bit;	
		}
		
	//TUB get	多个测温探头，检测温度最大值与最小值之差大于此值时告警
		if((TEMP_Max_Data-TEMP_Min_Data) > TUB_Threshold){
			TUB_Counter++;
			if(TUB_Counter > 100) BAT_Protect_Status |= TUB_bit;	
		}else{
			TUB_Counter = 0;
			BAT_Protect_Status &= ~TUB_bit;
		}
	}
/*	
////CeOV get
//	if(CellVolt_Max_Data < CeOV_Threshold){	//未达到触发条件 清零相应数据
//		BAT_Protect_Status &= ~CeOV_bit;	//清保护位
//		BAT_Protect_Alarm &= ~CeOV_bit;		//清告警位
//		CeOV_Timer = 0;
//	}
//	//检测值大于CeOV_Threshold和CeOV_Recovery_Threshold，且保护位和告警位未置位，则触发告警开始延时计时
//	if((CellVolt_Max_Data > CeOV_Threshold) && (CellVolt_Max_Data > CeOV_Recovery_Threshold) && ((BAT_Protect_Status&CeOV_bit)==0) && ((BAT_Protect_Alarm&CeOV_bit)==0)){	
//		CeOV_Timer = 0;												//清零计时
//		BAT_Protect_Status &= ~CeOV_bit;			//清保护位
//		BAT_Protect_Alarm |= CeOV_bit;				//置告警位
//	}
//	//检测值大于CeOV_Threshold和CeOV_Recovery_Threshold，且保护位未置位，告警位置位, 表示正在触发的延时计时
//	if((CellVolt_Max_Data > CeOV_Threshold) && (CellVolt_Max_Data > CeOV_Recovery_Threshold) && ((BAT_Protect_Status&CeOV_bit)==0) && ((BAT_Protect_Alarm&CeOV_bit)>0)){		
//		if(CeOV_Timer > CeOV_Delay_Time){	//如果延时计时大于设定值 则置保护状态位	清告警位
//			CeOV_Timer = 0;												//清零计时
//			BAT_Protect_Status |= CeOV_bit;				//置保护位
//			BAT_Protect_Alarm &= ~CeOV_bit;				//清告警位
//		}
//	}
////	//保护位置位，告警位未置位, 表示已经触发保护
////	if(((BAT_Protect_Status&CeOV_bit)>0) && ((BAT_Protect_Alarm&CeOV_bit)==0)){		
////			BAT_Protect_Status |= CeOV_bit;
////			BAT_Protect_Alarm &= ~CeOV_bit;
////	}		
//	//检测值小于CeOV_Threshold和CeOV_Recovery_Threshold，且保护位置位，告警位未置位, 则达到恢复条件 开始恢复计时
//	if((CellVolt_Max_Data < CeOV_Threshold) && (CellVolt_Max_Data < CeOV_Recovery_Threshold) && ((BAT_Protect_Status&CeOV_bit)>0) && ((BAT_Protect_Alarm&CeOV_bit)==0)){
//		CeOV_Timer = 0;													//清零计时
//		BAT_Protect_Status |= CeOV_bit;					//置保护位
//		BAT_Protect_Alarm |= CeOV_bit;					//置告警位
//	}	
//	//检测值小于CeOV_Threshold和CeOV_Recovery_Threshold，且保护位置位，告警位置位, 表示正在恢复的延时计时
//	if((CellVolt_Max_Data < CeOV_Threshold) && (CellVolt_Max_Data < CeOV_Recovery_Threshold) && ((BAT_Protect_Status&CeOV_bit)>0) && ((BAT_Protect_Alarm&CeOV_bit)>0)){		
//		if(CeOV_Timer > CeOV_Recovery_Time){	//如果延时计时大于设定值 则清保护位	清告警位
//			CeOV_Timer = 0;												//清零计时
//			BAT_Protect_Status &= ~CeOV_bit;			//清保护位
//			BAT_Protect_Alarm &= ~CeOV_bit;				//清告警位
//		}
//	}
	
////CeUV get
//	if(CellVolt_Min_Data > CeUV_Threshold){	//未达到触发条件 清零相应数据
//		BAT_Protect_Status &= ~CeUV_bit;	//清保护位
//		BAT_Protect_Alarm &= ~CeUV_bit;		//清告警位
//		CeUV_Timer = 0;
//	}
//	//检测值小于于CeUV_Threshold和CeUV_Recovery_Threshold，且保护位和告警位未置位，则触发告警开始延时计时
//	if((CellVolt_Min_Data < CeUV_Threshold) && (CellVolt_Min_Data < CeUV_Recovery_Threshold) && ((BAT_Protect_Status&CeUV_bit)==0) && ((BAT_Protect_Alarm&CeUV_bit)==0)){	
//		CeUV_Timer = 0;												//清零计时
//		BAT_Protect_Status &= ~CeUV_bit;			//清保护位
//		BAT_Protect_Alarm |= CeUV_bit;				//置告警位	
//	}
//	//检测值大于CeUV_Threshold和CeUV_Recovery_Threshold，且保护位未置位，告警位置位, 表示正在触发的延时计时
//	if((CellVolt_Min_Data < CeUV_Threshold) && (CellVolt_Min_Data < CeUV_Recovery_Threshold) && ((BAT_Protect_Status&CeUV_bit)==0) && ((BAT_Protect_Alarm&CeUV_bit)>0)){		
//		if(CeUV_Timer > CeUV_Delay_Time){	//如果延时计时大于设定值 则置保护状态位	清告警位
//			CeUV_Timer = 0;												//清零计时
//			BAT_Protect_Status |= CeUV_bit;				//置保护位
//			BAT_Protect_Alarm &= ~CeUV_bit;				//清告警位
//		}
//	}
//	//保护位置位，告警位未置位, 表示已经触发保护
//	if(((BAT_Protect_Status&CeUV_bit)>0) && ((BAT_Protect_Alarm&CeUV_bit)==0)){		
//			BAT_Protect_Status |= CeUV_bit;
//			BAT_Protect_Alarm &= ~CeUV_bit;
//	}		
//	//检测值大于CeUV_Threshold和CeUV_Recovery_Threshold，且保护位置位，告警位未置位, 则达到恢复条件 开始恢复计时
//	if((CellVolt_Min_Data > CeUV_Threshold) && (CellVolt_Min_Data > CeUV_Recovery_Threshold) && ((BAT_Protect_Status&CeUV_bit)>0) && ((BAT_Protect_Alarm&CeUV_bit)==0)){
//		if(CeUV_Timer > CeUV_Recovery_Time){
//			CeUV_Timer = 0;													//清零计时
//			BAT_Protect_Status |= CeUV_bit;					//置保护位
//			BAT_Protect_Alarm |= CeUV_bit;					//置告警位
//		}
//	}	
//	//检测值小于CeUV_Threshold和CeUV_Recovery_Threshold，且保护位置位，告警位置位, 表示正在恢复的延时计时
//	if((CellVolt_Min_Data > CeUV_Threshold) && (CellVolt_Min_Data > CeUV_Recovery_Threshold) && ((BAT_Protect_Status&CeUV_bit)>0) && ((BAT_Protect_Alarm&CeUV_bit)>0)){		
//		if(CeUV_Timer > CeUV_Recovery_Time){	//如果延时计时大于设定值 则清保护位	清告警位
//			CeUV_Timer = 0;												//清零计时
//			BAT_Protect_Status &= ~CeUV_bit;			//清保护位
//			BAT_Protect_Alarm &= ~CeUV_bit;				//清告警位
//		}
//	}

////PkOV get
//	if(BAT_Voltage < PkOV_Threshold){	//未达到触发条件 清零相应数据
//		BAT_Protect_Status &= ~PkOV_bit;	//清保护位
//		BAT_Protect_Alarm &= ~PkOV_bit;		//清告警位
//		PkOV_Timer = 0;
//	}
//	//检测值大于PkOV_Threshold和PkOV_Recovery_Threshold，且保护位和告警位未置位，则触发告警开始延时计时
//	if((BAT_Voltage > PkOV_Threshold) && (BAT_Voltage > PkOV_Recovery_Threshold) && ((BAT_Protect_Status&PkOV_bit)==0) && ((BAT_Protect_Alarm&PkOV_bit)==0)){	
//		PkOV_Timer = 0;												//清零计时
//		BAT_Protect_Status &= ~PkOV_bit;			//清保护位
//		BAT_Protect_Alarm |= PkOV_bit;				//置告警位	
//	}
//	//检测值大于PkOV_Threshold和PkOV_Recovery_Threshold，且保护位未置位，告警位置位, 表示正在触发的延时计时
//	if((BAT_Voltage > PkOV_Threshold) && (BAT_Voltage > PkOV_Recovery_Threshold) && ((BAT_Protect_Status&PkOV_bit)==0) && ((BAT_Protect_Alarm&PkOV_bit)>0)){		
//		if(PkOV_Timer > PkOV_Delay_Time){	//如果延时计时大于设定值 则置保护状态位	清告警位
//			PkOV_Timer = 0;												//清零计时
//			BAT_Protect_Status |= PkOV_bit;				//置保护位
//			BAT_Protect_Alarm &= ~PkOV_bit;				//清告警位
//		}
//	}
//	//保护位置位，告警位未置位, 表示已经触发保护
//	if(((BAT_Protect_Status&PkOV_bit)>0) && ((BAT_Protect_Alarm&PkOV_bit)==0)){		
//			BAT_Protect_Status |= PkOV_bit;
//			BAT_Protect_Alarm &= ~PkOV_bit;
//	}		
//	//检测值小于PkOV_Threshold和PkOV_Recovery_Threshold，且保护位置位，告警位未置位, 则达到恢复条件 开始恢复计时
//	if((BAT_Voltage < PkOV_Threshold) && (BAT_Voltage < PkOV_Recovery_Threshold) && ((BAT_Protect_Status&PkOV_bit)>0) && ((BAT_Protect_Alarm&PkOV_bit)==0)){
//		PkOV_Timer = 0;													//清零计时
//		BAT_Protect_Status |= PkOV_bit;					//置保护位
//		BAT_Protect_Alarm |= PkOV_bit;					//置告警位
//	}	
//	//检测值小于CeOV_Threshold和CeOV_Recovery_Threshold，且保护位置位，告警位置位, 表示正在恢复的延时计时
//	if((BAT_Voltage < PkOV_Threshold) && (BAT_Voltage < PkOV_Recovery_Threshold) && ((BAT_Protect_Status&PkOV_bit)>0) && ((BAT_Protect_Alarm&PkOV_bit)>0)){		
//		if(PkOV_Timer > PkOV_Recovery_Time){	//如果延时计时大于设定值 则清保护位	清告警位
//			PkOV_Timer = 0;									//清零计时
//			BAT_Protect_Status &= ~PkOV_bit;			//清保护位
//			BAT_Protect_Alarm &= ~PkOV_bit;				//清告警位
//		}
//	}
	
////PkUV get
//	if(BAT_Voltage > PkUV_Threshold){	//未达到触发条件 清零相应数据
//		BAT_Protect_Status &= ~PkUV_bit;	//清保护位
//		BAT_Protect_Alarm &= ~PkUV_bit;		//清告警位
//		PkUV_Timer = 0;
//	}
//	//检测值小于于CeUV_Threshold和CeUV_Recovery_Threshold，且保护位和告警位未置位，则触发告警开始延时计时
//	if((BAT_Voltage < PkUV_Threshold) && (BAT_Voltage < PkUV_Recovery_Threshold) && ((BAT_Protect_Status&PkUV_bit)==0) && ((BAT_Protect_Alarm&PkUV_bit)==0)){	
//		PkUV_Timer = 0;												//清零计时
//		BAT_Protect_Status &= ~PkUV_bit;			//清保护位
//		BAT_Protect_Alarm |= PkUV_bit;				//置告警位
//	}
//	//检测值大于PkUV_Threshold和PkUV_Recovery_Threshold，且保护位未置位，告警位置位, 表示正在触发的延时计时
//	if((BAT_Voltage < PkUV_Threshold) && (BAT_Voltage < PkUV_Recovery_Threshold) && ((BAT_Protect_Status&PkUV_bit)==0) && ((BAT_Protect_Alarm&PkUV_bit)>0)){		
//		if(PkUV_Timer > PkUV_Delay_Time){	//如果延时计时大于设定值 则置保护状态位	清告警位
//			PkUV_Timer = 0;												//清零计时
//			BAT_Protect_Status |= PkUV_bit;				//置保护位
//			BAT_Protect_Alarm &= ~PkUV_bit;				//清告警位
//		}
//	}
//	//保护位置位，告警位未置位, 表示已经触发保护
//	if(((BAT_Protect_Status&PkUV_bit)>0) && ((BAT_Protect_Alarm&PkUV_bit)==0)){		
//			BAT_Protect_Status |= PkUV_bit;
//			BAT_Protect_Alarm &= ~PkUV_bit;
//	}		
//	//检测值大于PkUV_Threshold和PkUV_Recovery_Threshold，且保护位置位，告警位未置位, 则达到恢复条件 开始恢复计时
//	if((BAT_Voltage > PkUV_Threshold) && (BAT_Voltage > PkUV_Recovery_Threshold) && ((BAT_Protect_Status&PkUV_bit)>0) && ((BAT_Protect_Alarm&PkUV_bit)==0)){
//		PkUV_Timer = 0;													//清零计时
//		BAT_Protect_Status |= PkUV_bit;					//置保护位
//		BAT_Protect_Alarm |= PkUV_bit;					//置告警位
//	}	
//	//检测值小于CeUV_Threshold和CeUV_Recovery_Threshold，且保护位置位，告警位置位, 表示正在恢复的延时计时
//	if((BAT_Voltage > PkUV_Threshold) && (BAT_Voltage > PkUV_Recovery_Threshold) && ((BAT_Protect_Status&PkUV_bit)>0) && ((BAT_Protect_Alarm&PkUV_bit)>0)){		
//		if(PkUV_Timer > PkUV_Recovery_Time){	//如果延时计时大于设定值 则清保护位	清告警位
//			PkUV_Timer = 0;												//清零计时
//			BAT_Protect_Status &= ~PkUV_bit;			//清保护位
//			BAT_Protect_Alarm &= ~PkUV_bit;				//清告警位
//		}
//	}	
	
////OT get
//	if(TEMP_Max_Data < OT_Threshold){	//未达到触发条件 清零相应数据
//		BAT_Protect_Status &= ~OT_bit;	//清保护位
//		BAT_Protect_Alarm &= ~OT_bit;		//清告警位
//		OT_Timer = 0;
//	}
//	//检测值大于OT_Threshold和OT_Recovery_Threshold，且保护位和告警位未置位，则触发告警开始延时计时
//	if((TEMP_Max_Data > OT_Threshold) && (TEMP_Max_Data > OT_Recovery_Threshold) && ((BAT_Protect_Status&OT_bit)==0) && ((BAT_Protect_Alarm&OT_bit)==0)){	
//		OT_Timer = 0;												//清零计时
//		BAT_Protect_Status &= ~OT_bit;			//清保护位
//		BAT_Protect_Alarm |= OT_bit;				//置告警位	
//	}
//	//检测值大于OT_Threshold和OT_Recovery_Threshold，且保护位未置位，告警位置位, 表示正在触发的延时计时
//	if((TEMP_Max_Data > OT_Threshold) && (TEMP_Max_Data > OT_Recovery_Threshold) && ((BAT_Protect_Status&OT_bit)==0) && ((BAT_Protect_Alarm&OT_bit)>0)){		
//		if(OT_Timer > OT_Delay_Time){	//如果延时计时大于设定值 则置保护状态位	清告警位
//			OT_Timer = 0;												//清零计时
//			BAT_Protect_Status |= OT_bit;				//置保护位
//			BAT_Protect_Alarm &= ~OT_bit;				//清告警位
//		}
//	}
//	//保护位置位，告警位未置位, 表示已经触发保护
//	if(((BAT_Protect_Status&OT_bit)>0) && ((BAT_Protect_Alarm&OT_bit)==0)){		
//			BAT_Protect_Status |= OT_bit;
//			BAT_Protect_Alarm &= ~OT_bit;
//	}		
//	//检测值小于OT_Threshold和OT_Recovery_Threshold，且保护位置位，告警位未置位, 则达到恢复条件 开始恢复计时
//	if((TEMP_Max_Data < OT_Threshold) && (TEMP_Max_Data < OT_Recovery_Threshold) && ((BAT_Protect_Status&OT_bit)>0) && ((BAT_Protect_Alarm&OT_bit)==0)){
//		OT_Timer = 0;													//清零计时
//		BAT_Protect_Status |= OT_bit;					//置保护位
//		BAT_Protect_Alarm |= OT_bit;					//置告警位
//	}	
//	//检测值小于OT_Threshold和OT_Recovery_Threshold，且保护位置位，告警位置位, 表示正在恢复的延时计时
//	if((TEMP_Max_Data < OT_Threshold) && (TEMP_Max_Data < OT_Recovery_Threshold) && ((BAT_Protect_Status&OT_bit)>0) && ((BAT_Protect_Alarm&OT_bit)>0)){		
//		if(OT_Timer > OT_Recovery_Time){	//如果延时计时大于设定值 则清保护位	清告警位
//			OT_Timer = 0;									//清零计时
//			BAT_Protect_Status &= ~OT_bit;			//清保护位
//			BAT_Protect_Alarm &= ~OT_bit;				//清告警位
//		}
//	}
	
////UT get
//	if(TEMP_Min_Data < UT_Threshold){	//未达到触发条件 清零相应数据
//		BAT_Protect_Status &= ~UT_bit;	//清保护位
//		BAT_Protect_Alarm &= ~UT_bit;		//清告警位
//		UT_Timer = 0;
//	}
//	//检测值小于于UT_Threshold和UT_Recovery_Threshold，且保护位和告警位未置位，则触发告警开始延时计时
//	if((TEMP_Min_Data < UT_Threshold) && (TEMP_Min_Data < PkUV_Recovery_Threshold) && ((BAT_Protect_Status&UT_bit)==0) && ((BAT_Protect_Alarm&UT_bit)==0)){	
//		UT_Timer = 0;												//清零计时
//		BAT_Protect_Status &= ~UT_bit;			//清保护位
//		BAT_Protect_Alarm |= UT_bit;				//置告警位
//	}
//	//检测值大于UT_Threshold和UT_Recovery_Threshold，且保护位未置位，告警位置位, 表示正在触发的延时计时
//	if((TEMP_Min_Data < UT_Threshold) && (TEMP_Min_Data < UT_Recovery_Threshold) && ((BAT_Protect_Status&UT_bit)==0) && ((BAT_Protect_Alarm&UT_bit)>0)){		
//		if(UT_Timer > UT_Delay_Time){	//如果延时计时大于设定值 则置保护状态位	清告警位
//			UT_Timer = 0;												//清零计时
//			BAT_Protect_Status |= UT_bit;				//置保护位
//			BAT_Protect_Alarm &= ~UT_bit;				//清告警位
//		}
//	}
//	//保护位置位，告警位未置位, 表示已经触发保护
//	if(((BAT_Protect_Status&UT_bit)>0) && ((BAT_Protect_Alarm&UT_bit)==0)){		
//			BAT_Protect_Status |= UT_bit;
//			BAT_Protect_Alarm &= ~UT_bit;
//	}		
//	//检测值大于UT_Threshold和UT_Recovery_Threshold，且保护位置位，告警位未置位, 则达到恢复条件 开始恢复计时
//	if((TEMP_Min_Data > UT_Threshold) && (TEMP_Min_Data > UT_Recovery_Threshold) && ((BAT_Protect_Status&UT_bit)>0) && ((BAT_Protect_Alarm&UT_bit)==0)){
//		UT_Timer = 0;													//清零计时
//		BAT_Protect_Status |= UT_bit;					//置保护位
//		BAT_Protect_Alarm |= UT_bit;					//置告警位
//	}	
//	//检测值小于UT_Threshold和UT_Recovery_Threshold，且保护位置位，告警位置位, 表示正在恢复的延时计时
//	if((TEMP_Min_Data > UT_Threshold) && (TEMP_Min_Data > UT_Recovery_Threshold) && ((BAT_Protect_Status&UT_bit)>0) && ((BAT_Protect_Alarm&UT_bit)>0)){		
//		if(UT_Timer > UT_Recovery_Time){	//如果延时计时大于设定值 则清保护位	清告警位
//			UT_Timer = 0;												//清零计时
//			BAT_Protect_Status &= ~UT_bit;			//清保护位
//			BAT_Protect_Alarm &= ~UT_bit;				//清告警位
//		}
//	}
*/

////HOT get	在加热过程中 最高温度高于HOT_Threshold告警
//	if((BAT_Work_Status&Heating_bit) > 0){			//判断为加热状态
//		if((TEMP_Max_Data+100) > HOC_Threshold)	BAT_Protect_Status |= HOT_bit;
//		else																		BAT_Protect_Status &= ~HOT_bit;
//	}else{
//		BAT_Protect_Status &= ~HOT_bit;
//	}	

		
	
////HOC get
//	if(Heat_Current > HOC_Threshold){
//		BAT_Protect_Status |= HOC_bit;
//	}else{
//		BAT_Protect_Status &= ~HOC_bit;
//	}
}

void BAT(void){
	
//	CHG_Plugged_get();
	Current_get();				//计算充放电电流值
	CAP();								//计算容量增减	
	Remain_CAP_Percent_get();	//计算剩余容量百分比	(剩余容量/设计容量)*100%	

	if(Measure_Num > 25){
		BAT_Voltage_get();		//计算电池组总电压
//		Port_Voltage_get();		//计算P+电压
//		Heat_Current_get();		//计算加热电流值


//		From_Temp_Get_RTCap();
//		if(CAP_Temp_para_old != CAP_Temp_para){
//			CAP_Temp_para_times++;
//			if(CAP_Temp_para_times > 2000){
//				CAP_Temp_para_old = CAP_Temp_para;
//				Full_CAP_RT = (Full_CAP*CAP_Temp_para)/100;							//根据 温度-容量系数 获得实时满容量
//				Rem_CAP_mAh = (Full_CAP_RT*Remain_CAP_Percent)/100;			//实时容量更新后，剩余容量百分比不变，根据百分比算得剩余容量
//			}
//		}else{
//			CAP_Temp_para_times = 0;
//		}
		
	}
}
void Heat(void){
	
}
/*
void BAT_CANSend(void){
	switch(CANSend_Timer){
		case 50:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000001;
			CAN_TxMsg[CAN1].data[0] = (BAT_Protect_Status&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[1] = (BAT_Protect_Status&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[2] = (BAT_Protect_Status&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (BAT_Protect_Status&0x000000FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (BAT_Work_Status&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[5] = (BAT_Work_Status&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[6] = (BAT_Work_Status&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (BAT_Work_Status&0x000000FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 100:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000002;
			CAN_TxMsg[CAN1].data[0] = (Full_CAP_RT&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[1] = (Full_CAP_RT&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[2] = (Full_CAP_RT&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (Full_CAP_RT&0x000000FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (Rem_CAP_mAh&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[5] = (Rem_CAP_mAh&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[6] = (Rem_CAP_mAh&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (Rem_CAP_mAh&0x000000FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break; 
		case 150:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000003;
			CAN_TxMsg[CAN1].data[0] = (Designed_CAP&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[1] = (Designed_CAP&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[2] = (Designed_CAP&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (Designed_CAP&0x000000FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (Cycle_Times&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (Cycle_Times&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = 0;
			CAN_TxMsg[CAN1].data[7] = Remain_CAP_Percent;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 200:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000004;
			CAN_TxMsg[CAN1].data[0] = (Current&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[1] = (Current&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[2] = (Current&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (Current&0x000000FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DSG_Rate&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DSG_Rate&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (CHG_Rate&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (CHG_Rate&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 250:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000005;
			CAN_TxMsg[CAN1].data[0] = (BAT_Voltage&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[1] = (BAT_Voltage&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[2] = (BAT_Voltage&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (BAT_Voltage&0x000000FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (Port_Voltage&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[5] = (Port_Voltage&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[6] = (Port_Voltage&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (Port_Voltage&0x000000FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 300:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000006;
			CAN_TxMsg[CAN1].data[0] = (DSG_CAP_Lifetime_mAh&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[1] = (DSG_CAP_Lifetime_mAh&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[2] = (DSG_CAP_Lifetime_mAh&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DSG_CAP_Lifetime_mAh&0x000000FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (CHG_CAP_Lifetime_mAh&0xFF000000)>>24;
			CAN_TxMsg[CAN1].data[5] = (CHG_CAP_Lifetime_mAh&0x00FF0000)>>16;
			CAN_TxMsg[CAN1].data[6] = (CHG_CAP_Lifetime_mAh&0x0000FF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (CHG_CAP_Lifetime_mAh&0x000000FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 500:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000100;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[5].CellVolt[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[5].CellVolt[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[5].CellVolt[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[5].CellVolt[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[5].CellVolt[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[5].CellVolt[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[5].CellVolt[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[5].CellVolt[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 550:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000101;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[5].CellVolt[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[5].CellVolt[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[5].CellVolt[5]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[5].CellVolt[5]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[5].CellVolt[6]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[5].CellVolt[6]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[5].CellVolt[7]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[5].CellVolt[7]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 600:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000102;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[5].CellVolt[8]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[5].CellVolt[8]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[5].CellVolt[9]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[5].CellVolt[9]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[5].CellVolt[10]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[5].CellVolt[10]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[5].CellVolt[11]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[5].CellVolt[11]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 650:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000103;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[4].CellVolt[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[4].CellVolt[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[4].CellVolt[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[4].CellVolt[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[4].CellVolt[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[4].CellVolt[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[4].CellVolt[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[4].CellVolt[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 700:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000104;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[4].CellVolt[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[4].CellVolt[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[4].CellVolt[5]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[4].CellVolt[5]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[4].CellVolt[6]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[4].CellVolt[6]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[4].CellVolt[7]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[4].CellVolt[7]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 750:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000105;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[4].CellVolt[8]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[4].CellVolt[8]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[4].CellVolt[9]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[4].CellVolt[9]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[4].CellVolt[10]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[4].CellVolt[10]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[4].CellVolt[11]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[4].CellVolt[11]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 800:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000106;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[3].CellVolt[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[3].CellVolt[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[3].CellVolt[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[3].CellVolt[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[3].CellVolt[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[3].CellVolt[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[3].CellVolt[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[3].CellVolt[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 850:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000107;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[3].CellVolt[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[3].CellVolt[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[3].CellVolt[5]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[3].CellVolt[5]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[3].CellVolt[6]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[3].CellVolt[6]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[3].CellVolt[7]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[3].CellVolt[7]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 900:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000108;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[3].CellVolt[8]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[3].CellVolt[8]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[3].CellVolt[9]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[3].CellVolt[9]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[3].CellVolt[10]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[3].CellVolt[10]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[3].CellVolt[11]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[3].CellVolt[11]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 950:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000109;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[2].CellVolt[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[2].CellVolt[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[2].CellVolt[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[2].CellVolt[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[2].CellVolt[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[2].CellVolt[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[2].CellVolt[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[2].CellVolt[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 1000:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x0000010A;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[2].CellVolt[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[2].CellVolt[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[2].CellVolt[5]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[2].CellVolt[5]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[2].CellVolt[6]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[2].CellVolt[6]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[2].CellVolt[7]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[2].CellVolt[7]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 1050:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x0000010B;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[2].CellVolt[8]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[2].CellVolt[8]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[2].CellVolt[9]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[2].CellVolt[9]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[2].CellVolt[10]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[2].CellVolt[10]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[2].CellVolt[11]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[2].CellVolt[11]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;			
		case 1100:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x0000010C;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[1].CellVolt[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[1].CellVolt[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[1].CellVolt[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[1].CellVolt[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[1].CellVolt[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[1].CellVolt[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[1].CellVolt[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[1].CellVolt[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;	
		case 1150:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x0000010D;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[1].CellVolt[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[1].CellVolt[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[1].CellVolt[5]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[1].CellVolt[5]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[1].CellVolt[6]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[1].CellVolt[6]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[1].CellVolt[7]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[1].CellVolt[7]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1200:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x0000010E;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[1].CellVolt[8]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[1].CellVolt[8]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[1].CellVolt[9]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[1].CellVolt[9]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[1].CellVolt[10]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[1].CellVolt[10]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[1].CellVolt[11]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[1].CellVolt[11]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1250:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x0000010F;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].CellVolt[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].CellVolt[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].CellVolt[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].CellVolt[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[0].CellVolt[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[0].CellVolt[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[0].CellVolt[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[0].CellVolt[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1300:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000110;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].CellVolt[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].CellVolt[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].CellVolt[5]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].CellVolt[5]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[0].CellVolt[6]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[0].CellVolt[6]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[0].CellVolt[7]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[0].CellVolt[7]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1350:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000111;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].CellVolt[8]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].CellVolt[8]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].CellVolt[9]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].CellVolt[9]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[0].CellVolt[10]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[0].CellVolt[10]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[0].CellVolt[11]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[0].CellVolt[11]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1400:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x000001FE;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].CellVolt_DELTA&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].CellVolt_DELTA&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = 0;
			CAN_TxMsg[CAN1].data[3] = 0;
			CAN_TxMsg[CAN1].data[4] = 0;
			CAN_TxMsg[CAN1].data[5] = 0;
			CAN_TxMsg[CAN1].data[6] = 0;
			CAN_TxMsg[CAN1].data[7] = 0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1450:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x000001FF;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].CellVolt_Max&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].CellVolt_Max&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].MAX_Cell_NUM&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].MAX_Cell_NUM&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[0].CellVolt_Min&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[0].CellVolt_Min&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[0].MIN_Cell_NUM&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[0].MIN_Cell_NUM&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1500:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000200;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[5].GPIO_NTC_TEMP[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[5].GPIO_NTC_TEMP[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[5].GPIO_NTC_TEMP[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[5].GPIO_NTC_TEMP[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[5].GPIO_NTC_TEMP[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[5].GPIO_NTC_TEMP[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[5].GPIO_NTC_TEMP[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[5].GPIO_NTC_TEMP[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;		
		case 1550:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000201;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[5].GPIO_NTC_TEMP[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[5].GPIO_NTC_TEMP[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[4].GPIO_NTC_TEMP[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[4].GPIO_NTC_TEMP[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[4].GPIO_NTC_TEMP[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[4].GPIO_NTC_TEMP[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[4].GPIO_NTC_TEMP[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[4].GPIO_NTC_TEMP[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1600:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000202;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[4].GPIO_NTC_TEMP[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[4].GPIO_NTC_TEMP[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[4].GPIO_NTC_TEMP[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[4].GPIO_NTC_TEMP[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[3].GPIO_NTC_TEMP[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[3].GPIO_NTC_TEMP[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[3].GPIO_NTC_TEMP[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[3].GPIO_NTC_TEMP[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1650:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000203;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[3].GPIO_NTC_TEMP[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[3].GPIO_NTC_TEMP[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[3].GPIO_NTC_TEMP[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[3].GPIO_NTC_TEMP[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[3].GPIO_NTC_TEMP[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[3].GPIO_NTC_TEMP[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[2].GPIO_NTC_TEMP[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[2].GPIO_NTC_TEMP[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1700:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000204;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[2].GPIO_NTC_TEMP[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[2].GPIO_NTC_TEMP[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[2].GPIO_NTC_TEMP[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[2].GPIO_NTC_TEMP[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[2].GPIO_NTC_TEMP[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[2].GPIO_NTC_TEMP[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[2].GPIO_NTC_TEMP[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[2].GPIO_NTC_TEMP[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1750:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000205;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[1].GPIO_NTC_TEMP[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[1].GPIO_NTC_TEMP[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[1].GPIO_NTC_TEMP[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[1].GPIO_NTC_TEMP[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[1].GPIO_NTC_TEMP[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[1].GPIO_NTC_TEMP[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[1].GPIO_NTC_TEMP[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[1].GPIO_NTC_TEMP[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1800:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000206;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[1].GPIO_NTC_TEMP[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[1].GPIO_NTC_TEMP[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].GPIO_NTC_TEMP[0]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].GPIO_NTC_TEMP[0]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[0].GPIO_NTC_TEMP[1]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[0].GPIO_NTC_TEMP[1]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[0].GPIO_NTC_TEMP[2]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[0].GPIO_NTC_TEMP[2]&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1850:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x00000207;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].GPIO_NTC_TEMP[3]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].GPIO_NTC_TEMP[3]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].GPIO_NTC_TEMP[4]&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].GPIO_NTC_TEMP[4]&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = 0;
			CAN_TxMsg[CAN1].data[5] = 0;
			CAN_TxMsg[CAN1].data[6] = 0;
			CAN_TxMsg[CAN1].data[7] = 0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		case 1900:
			CAN_TxMsg[CAN1].id = CAN_Addr_Base|0x000002FF;
			CAN_TxMsg[CAN1].data[0] = (DEVICE[0].LT68_NTC_Temp_MAX&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[1] = (DEVICE[0].LT68_NTC_Temp_MAX&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[2] = (DEVICE[0].LT68_NTC_Temp_MAX_NUM&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[3] = (DEVICE[0].LT68_NTC_Temp_MAX_NUM&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[4] = (DEVICE[0].LT68_NTC_Temp_MIN&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[5] = (DEVICE[0].LT68_NTC_Temp_MIN&0x00FF)>>0;
			CAN_TxMsg[CAN1].data[6] = (DEVICE[0].LT68_NTC_Temp_MIN_NUM&0xFF00)>>8;
			CAN_TxMsg[CAN1].data[7] = (DEVICE[0].LT68_NTC_Temp_MIN_NUM&0x00FF)>>0;
			CAN_TxMsg[CAN1].len = 8;	
			CAN_TxMsg[CAN1].format = 1;			//扩展帧
			CAN_TxMsg[CAN1].type = 0;				//数据帧
			CAN_waitReady(CAN1);
			CAN_send(CAN1,&CAN_TxMsg[CAN1]);
			break;
		
		case 2000:			
			CANSend_Timer = 0;
			break;
	}
}
*/

void BAT_UartSend(void){
	uint16_t k=0;
	BATUartSendData[k] = 'B';
	BATUartSendData[k+1] = 'A';
	BATUartSendData[k+2] = 'T';
	BATUartSendData[k+3] = '\r';
	BATUartSendData[k+4] = '\n';
	k = k+5;
	BATUartSendData[k] = 'B';
	BATUartSendData[k+1] = 'V';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = ' ';
	BATUartSendData[k+4] = 'm';
	BATUartSendData[k+5] = 'V';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+BAT_Voltage/100000;
	BATUartSendData[k+8] = 0x30+(BAT_Voltage%100000)/10000;
	BATUartSendData[k+9] = 0x30+(BAT_Voltage%10000)/1000;
	BATUartSendData[k+10] = 0x30+(BAT_Voltage%1000)/100;
	BATUartSendData[k+11] = 0x30+(BAT_Voltage%100)/10;
	BATUartSendData[k+12] = 0x30+(BAT_Voltage%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;
	BATUartSendData[k] = 'P';
	BATUartSendData[k+1] = 'V';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = ' ';
	BATUartSendData[k+4] = 'm';
	BATUartSendData[k+5] = 'V';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Port_Voltage/100000;
	BATUartSendData[k+8] = 0x30+(Port_Voltage%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Port_Voltage%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Port_Voltage%1000)/100;
	BATUartSendData[k+11] = 0x30+(Port_Voltage%100)/10;
	BATUartSendData[k+12] = 0x30+(Port_Voltage%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;
	BATUartSendData[k] = 'C';
	BATUartSendData[k+1] = ' ';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'O';
	BATUartSendData[k+4] = 'F';
	BATUartSendData[k+5] = 'S';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Current_offset/100000;
	BATUartSendData[k+8] = 0x30+(Current_offset%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Current_offset%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Current_offset%1000)/100;
	BATUartSendData[k+11] = 0x30+(Current_offset%100)/10;
	BATUartSendData[k+12] = 0x30+(Current_offset%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;	
	BATUartSendData[k] = 'H';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'O';
	BATUartSendData[k+4] = 'F';
	BATUartSendData[k+5] = 'S';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Heat_Current_offset/100000;
	BATUartSendData[k+8] = 0x30+(Heat_Current_offset%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Heat_Current_offset%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Heat_Current_offset%1000)/100;
	BATUartSendData[k+11] = 0x30+(Heat_Current_offset%100)/10;
	BATUartSendData[k+12] = 0x30+(Heat_Current_offset%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;	
	BATUartSendData[k] = 'C';
	BATUartSendData[k+1] = ' ';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'm';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = ' ';
	if((Current&0x80000000) == 0x80000000){
		BATUartSendData[k+6] = '-';	
	}else{
		BATUartSendData[k+6] = '+';
	}
	BATUartSendData[k+7] = 0x30+(Current&0x7FFFFFF)/100000;
	BATUartSendData[k+8] = 0x30+((Current&0x7FFFFFF)%100000)/10000;
	BATUartSendData[k+9] = 0x30+((Current&0x7FFFFFF)%10000)/1000;
	BATUartSendData[k+10] = 0x30+((Current&0x7FFFFFF)%1000)/100;
	BATUartSendData[k+11] = 0x30+((Current&0x7FFFFFF)%100)/10;
	BATUartSendData[k+12] = 0x30+((Current&0x7FFFFFF)%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;	
	BATUartSendData[k] = 'D';
	BATUartSendData[k+1] = 'S';
	BATUartSendData[k+2] = 'G';
	BATUartSendData[k+3] = 'R';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = 'T';
	BATUartSendData[k+6] = 'E';	
	BATUartSendData[k+7] = ' ';
	BATUartSendData[k+8] = 0x30+(DSG_Rate%1000)/100;
	BATUartSendData[k+9] = 0x30+(DSG_Rate%100)/10;
	BATUartSendData[k+10] = 0x30+(DSG_Rate%10)/1;
	BATUartSendData[k+11] = '\r';
	BATUartSendData[k+12] = '\n';
	k = k+13;	
	BATUartSendData[k] = 'C';
	BATUartSendData[k+1] = 'H';
	BATUartSendData[k+2] = 'G';
	BATUartSendData[k+3] = 'R';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = 'T';
	BATUartSendData[k+6] = 'E';	
	BATUartSendData[k+7] = ' ';
	BATUartSendData[k+8] = 0x30+(CHG_Rate%1000)/100;
	BATUartSendData[k+9] = 0x30+(CHG_Rate%100)/10;
	BATUartSendData[k+10] = 0x30+(CHG_Rate%10)/1;
	BATUartSendData[k+11] = '\r';
	BATUartSendData[k+12] = '\n';
	k = k+13;
	BATUartSendData[k] = 'H';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = ' ';
	BATUartSendData[k+4] = 'm';
	BATUartSendData[k+5] = 'A';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Heat_Current/100000;
	BATUartSendData[k+8] = 0x30+(Heat_Current%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Heat_Current%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Heat_Current%1000)/100;
	BATUartSendData[k+11] = 0x30+(Heat_Current%100)/10;
	BATUartSendData[k+12] = 0x30+(Heat_Current%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;	
	BATUartSendData[k] = 'F';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'm';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = 'h';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Full_CAP/100000;
	BATUartSendData[k+8] = 0x30+(Full_CAP%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Full_CAP%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Full_CAP%1000)/100;
	BATUartSendData[k+11] = 0x30+(Full_CAP%100)/10;
	BATUartSendData[k+12] = 0x30+(Full_CAP%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;	
	BATUartSendData[k] = 'F';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = 'r';
	BATUartSendData[k+3] = 'm';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = 'h';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Full_CAP_RT/100000;
	BATUartSendData[k+8] = 0x30+(Full_CAP_RT%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Full_CAP_RT%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Full_CAP_RT%1000)/100;
	BATUartSendData[k+11] = 0x30+(Full_CAP_RT%100)/10;
	BATUartSendData[k+12] = 0x30+(Full_CAP_RT%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;	
	BATUartSendData[k] = 'R';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'm';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = 'h';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Rem_CAP_mAh/100000;
	BATUartSendData[k+8] = 0x30+(Rem_CAP_mAh%100000)/10000;
	BATUartSendData[k+9] = 0x30+(Rem_CAP_mAh%10000)/1000;
	BATUartSendData[k+10] = 0x30+(Rem_CAP_mAh%1000)/100;
	BATUartSendData[k+11] = 0x30+(Rem_CAP_mAh%100)/10;
	BATUartSendData[k+12] = 0x30+(Rem_CAP_mAh%10)/1;
	BATUartSendData[k+13] = '\r';
	BATUartSendData[k+14] = '\n';
	k = k+15;
	BATUartSendData[k] = 'R';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'm';
	BATUartSendData[k+4] = 'A';
	BATUartSendData[k+5] = 'm';
	BATUartSendData[k+6] = 's';	
	BATUartSendData[k+7] = ' ';
	BATUartSendData[k+8] = 0x30+Remain_CAP_mAms/100000000;
	BATUartSendData[k+9] = 0x30+(Remain_CAP_mAms%100000000)/10000000;
	BATUartSendData[k+10] = 0x30+(Remain_CAP_mAms%10000000)/1000000;
	BATUartSendData[k+11] = 0x30+(Remain_CAP_mAms%1000000)/100000;
	BATUartSendData[k+12] = 0x30+(Remain_CAP_mAms%100000)/10000;
	BATUartSendData[k+13] = 0x30+(Remain_CAP_mAms%10000)/1000;
	BATUartSendData[k+14] = 0x30+(Remain_CAP_mAms%1000)/100;
	BATUartSendData[k+15] = 0x30+(Remain_CAP_mAms%100)/10;
	BATUartSendData[k+16] = 0x30+(Remain_CAP_mAms%10)/1;
	BATUartSendData[k+17] = '\r';
	BATUartSendData[k+18] = '\n';
	k = k+19;	
	BATUartSendData[k] = 'R';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = '%';
	BATUartSendData[k+4] = ' ';
	BATUartSendData[k+5] = ' ';
	BATUartSendData[k+6] = ' ';	
	BATUartSendData[k+7] = 0x30+Remain_CAP_Percent/100;
	BATUartSendData[k+8] = 0x30+(Remain_CAP_Percent%100)/10;
	BATUartSendData[k+9] = 0x30+(Remain_CAP_Percent%10)/1;
	BATUartSendData[k+10] = '\r';
	BATUartSendData[k+11] = '\n';
	k = k+12;	
	BATUartSendData[k] = 'L';
	BATUartSendData[k+1] = 'D';
	BATUartSendData[k+2] = 'C';
	BATUartSendData[k+3] = ' ';
	BATUartSendData[k+4] = 'm';
	BATUartSendData[k+5] = 'A';
	BATUartSendData[k+6] = 'h';	
	BATUartSendData[k+7] = ' ';
	BATUartSendData[k+8] = 0x30+DSG_CAP_Lifetime_mAh/100000000;
	BATUartSendData[k+9] = 0x30+(DSG_CAP_Lifetime_mAh%100000000)/10000000;
	BATUartSendData[k+10] = 0x30+(DSG_CAP_Lifetime_mAh%10000000)/1000000;
	BATUartSendData[k+11] = 0x30+(DSG_CAP_Lifetime_mAh%1000000)/100000;
	BATUartSendData[k+12] = 0x30+(DSG_CAP_Lifetime_mAh%100000)/10000;
	BATUartSendData[k+13] = 0x30+(DSG_CAP_Lifetime_mAh%10000)/1000;
	BATUartSendData[k+14] = 0x30+(DSG_CAP_Lifetime_mAh%1000)/100;
	BATUartSendData[k+15] = 0x30+(DSG_CAP_Lifetime_mAh%100)/10;
	BATUartSendData[k+16] = 0x30+(DSG_CAP_Lifetime_mAh%10)/1;
	BATUartSendData[k+17] = '\r';
	BATUartSendData[k+18] = '\n';
	k = k+19;	
	BATUartSendData[k] = 'L';
	BATUartSendData[k+1] = 'C';
	BATUartSendData[k+2] = 'C';
	BATUartSendData[k+3] = ' ';
	BATUartSendData[k+4] = 'm';
	BATUartSendData[k+5] = 'A';
	BATUartSendData[k+6] = 'h';	
	BATUartSendData[k+7] = ' ';
	BATUartSendData[k+8] = 0x30+CHG_CAP_Lifetime_mAh/100000000;
	BATUartSendData[k+9] = 0x30+(CHG_CAP_Lifetime_mAh%100000000)/10000000;
	BATUartSendData[k+10] = 0x30+(CHG_CAP_Lifetime_mAh%10000000)/1000000;
	BATUartSendData[k+11] = 0x30+(CHG_CAP_Lifetime_mAh%1000000)/100000;
	BATUartSendData[k+12] = 0x30+(CHG_CAP_Lifetime_mAh%100000)/10000;
	BATUartSendData[k+13] = 0x30+(CHG_CAP_Lifetime_mAh%10000)/1000;
	BATUartSendData[k+14] = 0x30+(CHG_CAP_Lifetime_mAh%1000)/100;
	BATUartSendData[k+15] = 0x30+(CHG_CAP_Lifetime_mAh%100)/10;
	BATUartSendData[k+16] = 0x30+(CHG_CAP_Lifetime_mAh%10)/1;
	BATUartSendData[k+17] = '\r';
	BATUartSendData[k+18] = '\n';
	k = k+19;	
	
	BATUartSendData[k] = 'B';
	BATUartSendData[k+1] = 'P';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'S';
	BATUartSendData[k+4] = 'T';
	BATUartSendData[k+5] = 'U';
	BATUartSendData[k+6] = ' ';	
	if(((BAT_Protect_Status&0xF0000000)>>28)>9)      	BATUartSendData[k+7] = 55+((BAT_Protect_Status&0xF0000000)>>28);
	else                                        			BATUartSendData[k+7] = 48+((BAT_Protect_Status&0xF0000000)>>28);
	if(((BAT_Protect_Status&0x0F000000)>>24)>9)      	BATUartSendData[k+8] = 55+((BAT_Protect_Status&0x0F000000)>>24);
	else                                        			BATUartSendData[k+8] = 48+((BAT_Protect_Status&0x0F000000)>>24);
	if(((BAT_Protect_Status&0x00F00000)>>20)>9)       BATUartSendData[k+9] = 55+((BAT_Protect_Status&0x00F00000)>>20);
	else                                        			BATUartSendData[k+9] = 48+((BAT_Protect_Status&0x00F00000)>>20); 
	if(((BAT_Protect_Status&0x000F0000)>>16)>9)      	BATUartSendData[k+10] = 55+((BAT_Protect_Status&0x000F0000)>>16);
	else                                        			BATUartSendData[k+10] = 48+((BAT_Protect_Status&0x000F0000)>>16);
	if(((BAT_Protect_Status&0x0000F000)>>12)>9)      	BATUartSendData[k+11] = 55+((BAT_Protect_Status&0x0000F000)>>12);
	else                                        			BATUartSendData[k+11] = 48+((BAT_Protect_Status&0x0000F000)>>12);
	if(((BAT_Protect_Status&0x00000F00)>>8)>9)       	BATUartSendData[k+12] = 55+((BAT_Protect_Status&0x00000F00)>>8);
	else                                        			BATUartSendData[k+12] = 48+((BAT_Protect_Status&0x00000F00)>>8);	
	if(((BAT_Protect_Status&0x000000F0)>>4)>9)      	BATUartSendData[k+13] = 55+((BAT_Protect_Status&0x000000F0)>>4);
	else                                        			BATUartSendData[k+13] = 48+((BAT_Protect_Status&0x000000F0)>>4);
	if(((BAT_Protect_Status&0x0000000F)>>0)>9)       	BATUartSendData[k+14] = 55+((BAT_Protect_Status&0x0000000F)>>0);
	else                                        			BATUartSendData[k+14] = 48+((BAT_Protect_Status&0x0000000F)>>0);		
	BATUartSendData[k+15] = '\r';
	BATUartSendData[k+16] = '\n';
	k = k+17;		
	
	BATUartSendData[k] = 'W';
	BATUartSendData[k+1] = 'K';
	BATUartSendData[k+2] = ' ';
	BATUartSendData[k+3] = 'S';
	BATUartSendData[k+4] = 'T';
	BATUartSendData[k+5] = 'U';
	BATUartSendData[k+6] = ' ';	
	if(((BAT_Protect_Status&0xF0000000)>>28)>9)      	BATUartSendData[k+7] = 55+((BAT_Work_Status&0xF0000000)>>28);
	else                                        			BATUartSendData[k+7] = 48+((BAT_Work_Status&0xF0000000)>>28);
	if(((BAT_Protect_Status&0x0F000000)>>24)>9)      	BATUartSendData[k+8] = 55+((BAT_Work_Status&0x0F000000)>>24);
	else                                        			BATUartSendData[k+8] = 48+((BAT_Work_Status&0x0F000000)>>24);
	if(((BAT_Protect_Status&0x00F00000)>>20)>9)       BATUartSendData[k+9] = 55+((BAT_Work_Status&0x00F00000)>>20);
	else                                        			BATUartSendData[k+9] = 48+((BAT_Work_Status&0x00F00000)>>20); 
	if(((BAT_Protect_Status&0x000F0000)>>16)>9)      	BATUartSendData[k+10] = 55+((BAT_Work_Status&0x000F0000)>>16);
	else                                        			BATUartSendData[k+10] = 48+((BAT_Work_Status&0x000F0000)>>16);
	if(((BAT_Protect_Status&0x0000F000)>>12)>9)      	BATUartSendData[k+11] = 55+((BAT_Work_Status&0x0000F000)>>12);
	else                                        			BATUartSendData[k+11] = 48+((BAT_Work_Status&0x0000F000)>>12);
	if(((BAT_Protect_Status&0x00000F00)>>8)>9)       	BATUartSendData[k+12] = 55+((BAT_Work_Status&0x00000F00)>>8);
	else                                        			BATUartSendData[k+12] = 48+((BAT_Work_Status&0x00000F00)>>8);	
	if(((BAT_Protect_Status&0x000000F0)>>4)>9)      	BATUartSendData[k+13] = 55+((BAT_Work_Status&0x000000F0)>>4);
	else                                        			BATUartSendData[k+13] = 48+((BAT_Work_Status&0x000000F0)>>4);
	if(((BAT_Protect_Status&0x0000000F)>>0)>9)       	BATUartSendData[k+14] = 55+((BAT_Work_Status&0x0000000F)>>0);
	else                                        			BATUartSendData[k+14] = 48+((BAT_Work_Status&0x0000000F)>>0);		
	BATUartSendData[k+15] = '\r';
	BATUartSendData[k+16] = '\n';
	k = k+17;
	
	BATUartSendData[k] = 'T';
	BATUartSendData[k+1] = 'M';
	BATUartSendData[k+2] = 'P';
	BATUartSendData[k+3] = 'p';
	BATUartSendData[k+4] = 'a';
	BATUartSendData[k+5] = 'r';
	BATUartSendData[k+6] = ' ';
	k = k+7;
	BATUartSendData[k] = '0'+CAP_Temp_para_old/100;
	BATUartSendData[k+1] = '0'+(CAP_Temp_para_old%100)/10;	
	BATUartSendData[k+2] = '0'+CAP_Temp_para_old%10;
	k = k+3;
	BATUartSendData[k] = '\r';
	BATUartSendData[k+1] = '\n';	
	k = k+2;	
	BATUartSendData[k] = '\r';
	BATUartSendData[k+1] = '\n';	
	k = k+2;	
	uart3_send(BATUartSendData, k);
	uart1_send(BATUartSendData, k);
}

/*
	END OF FILE
*/
