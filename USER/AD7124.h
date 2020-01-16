#ifndef __AD7124_H__
#define __AD7124_H__

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "ssp1.h"
#include "uart.h"

//寄存器地址
#define COMMS					0x00
#define Status				0x00
#define ADC_Ctrl			0x01
#define Data					0x02
#define IO_Ctrl1			0x03
#define IO_Ctrl2			0x04
#define ID						0x05
#define Error					0x06
#define Error_EN			0x07
#define MCLK_COUNT		0x08
#define CHANNEL_0			0x09
#define CHANNEL_1			0x0A
#define CHANNEL_2			0x0B
#define CHANNEL_3			0x0C
#define CHANNEL_4			0x0D
#define CHANNEL_5			0x0E
#define CHANNEL_6			0x0F
#define CHANNEL_7			0x10
#define CHANNEL_8			0x11
#define CHANNEL_9			0x12
#define CHANNEL_10		0x13
#define CHANNEL_11		0x14
#define CHANNEL_12		0x15
#define CHANNEL_13		0x16
#define CHANNEL_14		0x17
#define CHANNEL_15		0x18
#define CONFIG_0			0x19
#define CONFIG_1			0x1A
#define CONFIG_2			0x1B
#define CONFIG_3			0x1C
#define CONFIG_4			0x1D
#define CONFIG_5			0x1E
#define CONFIG_6			0x1F
#define CONFIG_7			0x20
#define FILTER_0			0x21
#define FILTER_1			0x22
#define FILTER_2			0x23
#define FILTER_3			0x24
#define FILTER_4			0x25
#define FILTER_5			0x26
#define FILTER_6			0x27
#define FILTER_7			0x28
#define OFFSET_0			0x29
#define OFFSET_1			0x2A
#define OFFSET_2			0x2B
#define OFFSET_3			0x2C
#define OFFSET_4			0x2D
#define OFFSET_5			0x2E
#define OFFSET_6			0x2F
#define OFFSET_7			0x30
#define GAIN_0				0x31
#define GAIN_1				0x32
#define GAIN_2				0x33
#define GAIN_3				0x34
#define GAIN_4				0x35
#define GAIN_5				0x36
#define GAIN_6				0x37
#define GAIN_7				0x38

#define CH_0			0
#define CH_1			1
#define CH_2			2
#define CH_3			3
#define CH_4			4
#define CH_5			5
#define CH_6			6
#define CH_7			7
#define CH_8			8
#define CH_9			9
#define CH_10			10
#define CH_11			11
#define CH_12			12
#define CH_13			13
#define CH_14			14
#define CH_15			15


//COMMS寄存器位定义
#define	COMMS_int					0x00
#define WEN(x)						((x&0x01)<<7)
#define WEN_H							0x01
#define WEN_L							0x00
#define RW(x)							((x&0x01)<<6)
#define RW_Read						0x01
#define RW_Write					0x00
#define RS(x)							((x&0x3F)<<0)

//Status寄存器位定义

//ADC_Ctrl寄存器位定义
#define	ADC_Ctrl_int			0x0000
#define DOUT_RDY_DEL(x)		((x&0x01)<<12)
#define CONT_READ(x)			((x&0x01)<<11)
#define DATA_STATUS(x)		((x&0x01)<<10)
#define CS_EN(x)					((x&0x01)<<9)
#define REF_EN(x)					((x&0x01)<<8)
#define POWER_MODE(x)			((x&0x03)<<6)
#define LOW_POWER					0x00
#define MID_POWER					0x01
#define FULL_POWER				0x03
#define MODE(x)						((x&0x0F)<<2)
#define CONTINUOUS				0x00		//Continuous conversion mode (default).
#define SINGLE						0x01		//Single conversion mode.
#define STANDBY						0x02		//Standby mode.
#define POWERDOWN					0x03		//Power-down mode.
#define IDLE							0x04		//Idle mode.
#define MODE5							0x05		//Internal zero-scale (offset) calibration.
#define MODE6							0x03		//Internal full-scale (gain) calibration.
#define MODE7							0x04		//System zero-scale (offset) calibration.
#define MODE8							0x05		//System full-scale (gain) calibration.
#define CLK_SEL(x)				((x&0x03)<<0)
#define SEL1							0x00		//internal 614.4 kHz clock. The internal clock is not available at the CLK pin.
#define SEL2							0x01		//internal 614.4 kHz clock. This clock is available at the CLK pin.
#define SEL3							0x10		//external 614.4 kHz clock.
#define SEL4							0x11		//external clock. The external clock is divided by 4 within the AD7124-4.

//IO_CONTROL_1寄存器位定义
#define	IO_Ctrl1_int			0x00000000
#define GPIO_DAT2(x)			((x&0x01)<<23)
#define GPIO_DAT1(x)			((x&0x01)<<22)
#define GPIO_CTRL2(x)			((x&0x01)<<19)
#define GPIO_CTRL1(x)			((x&0x01)<<18)
#define PDSW(x)						((x&0x01)<<15)
#define IOUT1(x)					((x&0x07)<<11)
#define IOUT0(x)					((x&0x07)<<8)
#define CUR0							0x00		//off
#define CUR1							0x01		//50uA
#define CUR2							0x02		//100uA
#define CUR3							0x03		//250uA
#define CUR4							0x04		//500uA
#define CUR5							0x05		//750uA
#define CUR6							0x06		//1000uA
#define CUR7							0x07		//1000uA
#define IOUT1_CH(x)				((x&0x0F)<<4)
#define IOUT0_CH(x)				((x&0x0F)<<0)
#define CH0								0x00		//on the AIN0 pin
#define CH1								0x01		//on the AIN1 pin
#define CH2								0x04		//on the AIN2 pin
#define CH3								0x05		//on the AIN3 pin
#define CH4								0x0A		//on the AIN4 pin
#define CH5								0x0B		//on the AIN5 pin
#define CH6								0x0E		//on the AIN6 pin
#define CH7								0x0F		//on the AIN7 pin

//IO_CONTROL_2寄存器位定义
#define	IO_Ctrl2_int			0x0000
#define VBIAS7(x)					((x&0x01)<<15)
#define VBIAS6(x)					((x&0x01)<<14)
#define VBIAS5(x)					((x&0x01)<<11)
#define VBIAS4(x)					((x&0x01)<<10)
#define VBIAS3(x)					((x&0x01)<<5)
#define VBIAS2(x)					((x&0x01)<<4)
#define VBIAS1(x)					((x&0x01)<<1)
#define VBIAS0(x)					((x&0x01)<<0)

//ID寄存器位定义
#define AD7124_4					0x04
#define AD7124_4B					0x06

//ERROR寄存器位定义
#define	ERROR_int					0x00000000
#define LDO_CAP_ERR				(1<<19)
#define ADC_CAL_ERR				(1<<18)
#define ADC_CONV_ERR			(1<<17)
#define ADC_SAT_ERR				(1<<16)
#define AINP_OV_ERR				(1<<15)
#define AINP_UV_ERR				(1<<14)
#define AINM_OV_ERR				(1<<13)
#define AINM_UV_ERR				(1<<12)
#define REF_DET_ERR				(1<<11)
#define DLDO_PSM_ERR			(1<<9)
#define ALDO_PSM_ERR			(1<<7)
#define SPI_IGNORE_ERR		(1<<6)
#define SPI_SCLK_CNT_ERR	(1<<5)
#define SPI_READ_ERR			(1<<4)
#define SPI_WRITE_ERR			(1<<3)
#define SPI_CRC_ERR				(1<<2)
#define MM_CRC_ERR				(1<<1)
#define ROM_CRC_ERR				(1<<0)

//ERROR_EN寄存器位定义
#define	ERROR_EN_int						0x00000000
#define MCLK_CNT_EN(x)					((x&0x01)<<22)
#define LDO_CAP_CHK_TEST_EN(x)	((x&0x01)<<21)
#define LDO_CAP_CHK(x)					((x&0x03)<<19)
#define ADC_CAL_ERR_EN(x)				((x&0x01)<<18)
#define ADC_CONV_ERR_EN(x)			((x&0x01)<<17)
#define ADC_SAT_ERR_EN(x)				((x&0x01)<<16)
#define AINP_OV_ERR_EN(x)				((x&0x01)<<15)
#define AINP_UV_ERR_EN(x)				((x&0x01)<<14)
#define AINM_OV_ERR_EN(x)				((x&0x01)<<13)
#define AINM_UV_ERR_EN(x)				((x&0x01)<<12)
#define REF_DET_ERR_EN(x)				((x&0x01)<<11)
#define DLDO_PSM_TRIP_TEST_EN(x)	((x&0x01)<<10)
#define DLDO_PSM_ERR_EN(x)			((x&0x01)<<9)
#define ALDO_PSM_TRIP_TEST_EN(x)	((x&0x01)<<8)
#define ALDO_PSM_ERR_EN(x)			((x&0x01)<<7)
#define SPI_IGNORE_ERR_EN(x)		((x&0x01)<<6)
#define SPI_SCLK_CNT_ERR_EN(x)	((x&0x01)<<5)
#define SPI_READ_ERR_EN(x)			((x&0x01)<<4)
#define SPI_WRITE_ERR_EN(x)			((x&0x01)<<3)
#define SPI_CRC_ERR_EN(x)				((x&0x01)<<2)
#define MM_CRC_ERR_EN(x)				((x&0x01)<<1)
#define ROM_CRC_ERR_EN(x)				((x&0x01)<<0)

//CHANNEL寄存器位定义
#define	CHANNEL_int					0x0000
#define Enable(x)						((x&0x01)<<15)
#define Setup(x)						((x&0x07)<<12)	//0～7
#define AINP(x)							((x&0x1F)<<5)
#define AINM(x)							((x&0x1F)<<0)
#define AIN0								0x00
#define AIN1								0x01
#define AIN2								0x02
#define AIN3								0x03
#define AIN4								0x04
#define AIN5								0x05
#define AIN6								0x06
#define AIN7								0x07
#define TEMP_Sensor					0x10
#define AVss								0x11
#define INT_REF							0x12
#define DGND								0x13
#define AVdd_monitor_P			0x14
#define AVdd_monitor_N			0x15
#define IOVdd_monitor_P			0x16
#define IOVdd_monitor_N			0x17
#define ALDO_monitor_P			0x18
#define ALDO_monitor_N			0x19
#define DLDO_monitor_P			0x1A
#define DLDO_monitor_N			0x1B
#define V20mV_P							0x1C
#define V20mV_N							0x1D

//CONFIGURATION寄存器位定义
#define	CONFIGURATION_int		0x0000
#define Bipolar(x)					((x&0x01)<<11)
#define Burnout(x)					((x&0x03)<<9)
#define OFF									0x00
#define I500nA							0x01
#define I2uA								0x02
#define I4uA								0x03
#define REF_BUFP(x)					((x&0x01)<<8)
#define REF_BUFM(x)					((x&0x01)<<7)
#define AIN_BUFP(x)					((x&0x01)<<6)
#define AIN_BUFM(x)					((x&0x01)<<5)
#define REF_SEL(x)					((x&0x03)<<3)
#define REFIN1							0x00
#define REFIN2							0x01
#define int_ref							0x02
#define AVdd								0x03
#define PGA(x)							((x&0x07)<<0)
#define G_1									0x00
#define G_2									0x01
#define G_4									0x02
#define G_8									0x03
#define G_16								0x04
#define G_32								0x05
#define G_64								0x06
#define G_128								0x07

//FILTER寄存器位定义
#define	FILTER_int					0x00000000
#define FILTER_SEL(x)				((x&0x07)<<21)
#define SINC4								0x00
#define SINC3								0x02
#define FAST_SINC4					0x04
#define FAST_SINC3					0x05
#define post_filter					0x07
#define REJ60(x)						((x&0x01)<<20)
#define POST_FILTER(x)			((x&0x07)<<21)
#define POST1								0x02	
#define POST2								0x03
#define POST3								0x05
#define POST4								0x06
#define SINGLE_CYCLE(x)			((x&0x01)<<16)
#define FS(x)								((x&0x3FF)<<0)	//see noise tables

///////////////////////////////////////////////////////////////////////
#define ADC_REF							2367		//参考电压
extern uint32_t CH_OffSet[20];	//1mV 初始化期间测量各模拟输入的零点电位基准
extern uint32_t AD7124_COUNT;
extern uint32_t CH_Volt[20];	//1mV
extern uint8_t	AD7124_DataReady_Flag;
void AD7124_mission(void);
void AD7124_Write_CONFIGURATION(	uint8_t ch,
																	uint8_t bipolar,
																	uint8_t burnout,
																	uint8_t ref_bufp,
																	uint8_t ref_bufm,
																	uint8_t ain_bufp,
																	uint8_t ain_bufm,
																	uint8_t ref_sel,
																	uint8_t pga																
																);
void AD7124_Write_FILTER(	uint8_t ch,
													uint8_t filter_sel,
													uint8_t rej,
													uint8_t post,
													uint8_t single,
													uint16_t fs														
												);
void AD7124_Write_CHANNEL(uint8_t ch,uint8_t ainp,uint8_t ainm,uint8_t setup,uint8_t enable);
void AD7124_Read_DATA(uint8_t ch);
uint32_t AD7124_CH_VoltGet(uint32_t data);
void AD7124_UartSend(void);

#endif


