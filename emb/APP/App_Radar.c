/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: App_IR.c 
* 作者：李锡群
* 版本：V01
* 时间：20210730
* 内容简述：雷达功能调用文件
****************************************************************************/

/* 标准头文件 */
#include "string.h"
#include <stdio.h>

/* 内部头文件 */
#include "..\DRV\DRV_IIC\DRV_IIC.h"
#include "App_Radar.h"
#include "LockConfig.h"
#include "Public.h"


/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define SELF_CHECK_TIME		2000			// Self Check Time : 0~65536 ms	
#define LIGHT_ON_TIME		100				// Light On Time : ms, 32bit
#define LIGHT_OFF_TIME		100				// Light On Time : ms, 16bit
#define LIGHT_ON_LESS_TIME		500			// Light On Less Time : ms, 32bit
#define	LIGHT_SENSOR_STATUS	     0x4a		// Default : 0x4a-OFF; 0x42-ON
#define LIGHT_SENSOR_VALUE_LOW   500		// Light Sensor Value : 0~1023, 10bit
#define LIGHT_SENSOR_VALUE_HIGH  530		// Light Sensor Value : 0~1023, 10bit
#define LIGHT_SENSOR_INIVERSE    0			// Light Sensor inverse, default 0 ; stand  0 -> 1023, more lighting
#define DELTA_WIN_LENGTH  	    3			// Delta window	
#define DELTA_WIN_THRESHOLD 	2			// Delta window	threshold
#define	DELTA_MIGAN				100			//觅感雷达DELTA判断阈值,Sense Delta: 0~1023
#define	RX_RF2_STATUS	0x45				// Default : 0x45-ON; 0x46-OFF
#define	RX_RF_STATUS	0x55				// Default : 0x55-ON; 0xaa-OFF
#define	RX_CTRL_STATUS	0xa0				// Default : 0xa0-ON; 0x50-OFF
#define DIP_TIME_LONG		15000			// dip control: 15S;  PIN3 = 0
#define DIP_TIME_SHORT		2000			// dip control: 2S;	  PIN3 = 1
#define DIP_SENSE_HIGH		68				// dip control: sen value high; PIN2 = 0
#define DIP_SENSE_LOW		145				// dip control: sen value low; PIN2 = 1
#define DIP_SENSE_GAIN_HIGH		0x3B		// 0x5C,dip control: sen value high->gain = 30db; PIN2 = 1
#define DIP_SENSE_GAIN_LOW		0x9B		// 0x5C,dip control: sen value low->gain = 12db; PIN2 = 0
#define SENSE_GAIN_DEFAULT		0x3B		// // default = 0x3B  建议默认。如在近距离感应场景（2m内）使用，可配置为0x9B
#define SENSE_GAIN_MIGAN		0X9B		//觅感雷达接收增益值
#define	PWR_40UA_SWITCH			1			// 配置功耗�TTRUE - 40uA；FALSE - 68uA
#define DISTANCE_MIGAN_70CM		150			//觅感雷达感应距离70cm
#define DISTANCE_MIGAN_30CM		300			//觅感雷达感应距离30cm

#define RADAR_SLAVEADDR  0X50  //硬件地址

#define RADAR_CHANNEL    I2C_CHANNEL0 //硬件通道
/*-------------------------------------------------结构体-----------------------------------------------------------*/




/***************************************************************************************
**函数名:       App_Radar_SoftReset
**功能描述:     软件复位
**输入参数:     
**输出参数:    
**备注: 软件复位动作后内部的数字逻辑会重新载入所有的配置参数，某些参数设置后需要对雷达进行软复位
****************************************************************************************/
void App_Radar_SoftReset(void)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x00, 0x00);// software reset
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x00, 0x01);//software Reset Release
}

/***************************************************************************************
**函数名:       App_Radar_SenseRangeAdj
**功能描述:     判断阈值(感应距离)调整
**输入参数:     range:0~1023
**输出参数:    
**备注:         
****************************************************************************************/
void App_Radar_SenseRangeAdj(uint16_t range)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x68, 0xb8);									// Read from REG
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x67, 0xf8);									// Read from REG
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x10, (unsigned char)(range));					// low
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x11, (unsigned char)(range >> 8));				// high	
}

/***************************************************************************************
**函数名:       App_Radar_TtiggerHighTime
**功能描述:     感应输出高电平延时调整（AT5815 默认有感应时输出高电平 15 秒）
**输入参数:     time 单位：ms ，最短为 2 秒左右
**输出参数:    
**备注:         
****************************************************************************************/
void App_Radar_TtiggerHighTime(uint32_t time)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x41, 0x01);									// Read from REG
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x42, (unsigned char)(time));			// 0~7
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x43, (unsigned char)(time >> 8));		// 8~15
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x44, (unsigned char)(time >> 16));	// 16~23
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x45, (unsigned char)(time >> 24));	// 24~31
}

/***************************************************************************************
**函数名:       App_Radar_LightOffTime
**功能描述:     关灯保护时间调整
**输入参数:     time 单位：ms
**输出参数:    
**备注:         AT5815 在感应输出到达延时时间，切换到低电平输出后会有 2 秒钟的保护时间，保护时间内没有感应功能
****************************************************************************************/
void App_Radar_LightOffTime(uint16_t time)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x4e, (unsigned char)(time));			// 0~7
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x4f, (unsigned char)(time >> 8));	// 8~15
}

/***************************************************************************************
**函数名:       App_Radar_TurnOn
**功能描述:     开启光敏检测
**输入参数:     
**输出参数:    
**备注:         mode: 0x4a-OFF; 0x42-ON
****************************************************************************************/
void App_Radar_TurnOn(void)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x66, 0x42);// 0x4a: light sensor Off    ; 0x42: light sensor On
}

/***************************************************************************************
**函数名:       App_Radar_TurnOff
**功能描述:     关闭光敏检测
**输入参数:     
**输出参数:    
**备注:         mode: 0x4a-OFF; 0x42-ON
****************************************************************************************/
void App_Radar_TurnOff(void)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x66, 0x4a);// 0x4a: light sensor Off    ; 0x42: light sensor On
}

/***************************************************************************************
**函数名:       App_Radar_SelfCheckTime
**功能描述:     开机自检时间调整
**输入参数:     
**输出参数:    
**备注:         AT5815 默认开机自检(输出高电平)时间约 5~7 秒
****************************************************************************************/
void App_Radar_SelfCheckTime(uint16_t time)
{
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x38, (unsigned char)(time));		// low
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x39, (unsigned char)(time >> 8));	// high
}

/***************************************************************************************
**函数名:       App_Radar_SetDistance
**功能描述:     配置功耗，判断阈值/感应距离，接收增益调整
**输入参数:     pwr_40uA_switch，delta，gain
**输出参数:    
**备注:         
****************************************************************************************/
void App_Radar_SetDistance(uint8_t pwr_40uA_switch, uint8_t delta, uint8_t gain)
{
	// 配置功耗
    if (pwr_40uA_switch == 1)  // 40uA
    {
		DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x68, (0x48 & 0xc7) | 0x38);
		DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x67, (0xf3 & 0xf0) | 0x08);
	}
    else   // 70uA
    {
		DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x68, 0x48);
		DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x67, 0xf3);
	}
    // 写入delta值（判断阈值/感应距离调整）
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x10, (uint8_t)(delta));
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x11, (uint8_t)(delta >> 8));
	// 写入gain值（接收增益调整）
	DRV_IICWriteByte(RADAR_SLAVEADDR, RADAR_CHANNEL, 0x5C, gain);
}

/***************************************************************************************
**函数名:       DRV_RADAR_SetMode
**功能描述:     雷达模块设置
**输入参数:     mode
**输出参数:    
**备注:         
****************************************************************************************/
void App_Radar_SetMode(uint8_t mode)
{
	my_printf("****************************** \n");
	my_printf("App_Radar__SetMode\n");
	if(RadarControl.Mode == RADAR_FAR)//远距离
	{
		App_Radar_SetDistance(PWR_40UA_SWITCH, DELTA_MIGAN, SENSE_GAIN_MIGAN);//配置功耗，判断阈值/感应距离，接收增益调整
		App_Radar_TtiggerHighTime(LIGHT_ON_TIME);//感应输出高电平延时调整2S
		App_Radar_LightOffTime(LIGHT_OFF_TIME);//关灯保护时间调整1S
		App_Radar_SenseRangeAdj(DISTANCE_MIGAN_70CM);//判断阈值DIP_SENSE_HIGH(感应距离)调整68
		App_Radar_SoftReset();//软件复位
		my_printf("IR_SENSE_SET remote mode \n");
	}
	else if(RadarControl.Mode == RADAR_NEAR)//近距离
	{
		App_Radar_SetDistance(PWR_40UA_SWITCH, DELTA_MIGAN, SENSE_GAIN_MIGAN);//配置功耗，判断阈值/感应距离，接收增益调整
		App_Radar_TtiggerHighTime(LIGHT_ON_TIME);//感应输出高电平延时调整2S
		App_Radar_LightOffTime(LIGHT_OFF_TIME);//关灯保护时间调整1S
		App_Radar_SenseRangeAdj(DISTANCE_MIGAN_30CM);//判断阈值DIP_SENSE_LOW(感应距离)调整145
		App_Radar_SoftReset();//软件复位
		my_printf("IR_SENSE_SET close mode \n");
	}
	else if(RadarControl.Mode == RADAR_OFF)//关闭，实际改成超短距
	{
		App_Radar_TurnOn();//关闭光敏检测
		App_Radar_SoftReset();//软件复位
		my_printf("IR_SENSE_SET off mode \n");
	}
	my_printf("****************************** \n");
}








