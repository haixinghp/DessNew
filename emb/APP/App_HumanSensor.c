/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: HAL_HumanSensor.c 
* 作者：邓业豪
* 版本：V01
* 时间：20210728
* 内容简述：人体感应传感中层接口
****************************************************************************/
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
/* 内部头文件 */
#include "LockConfig.h"
#include "..\DRV\DRV_Radar\DRV_Radar.h"
#include "App_HumanSensor.h"
#include "App_IR.h"
/*----------------------------------------------局部变量定义--------------------------------------------------------*/

/***************************************************************************************
**函数名:       HAL_HumanSensorFileInit
**功能描述:     接近感应模块设置初始化
**输入参数:     distance设置模式     
**输出参数:      
**备注:         
****************************************************************************************/
void App_HumanSensorFileInit(uint8_t distance )
{
    #ifdef  RADAR_FUNCTION_ON      
    uint8_t tp1 = 0;;	
    (void)App_HumanSensorSet( distance, &tp1 );
    #endif
}

/***************************************************************************************
**函数名:       HAL_HumanSensorWakeupInit
**功能描述:     接近感应模块唤醒初始化
**输入参数:     distance设置模式     
**输出参数:      
**备注:         
****************************************************************************************/
void App_HumanSensorWakeupInit(uint8_t distance )
{
     App_HumanSensorFileInit( distance );
}

/***************************************************************************************
**函数名:       App_HumanSensor_Tim10Ms
**功能描述:     10ms定时器
**输入参数:     none
**输出参数:     none  
**备注:         
****************************************************************************************/
void App_HumanSensor_Tim10Ms( void )
{
   #ifdef IR_FUNCTION_ON   
		App_IR_Tim10Ms();  
   #endif
}

/***************************************************************************************
**函数名:       HAL_HumanSensorSet
**功能描述:     接近感应模块设置距离
**输入参数:     distance设置模式    pfirtflg= 单次处理
**输出参数:     -1= 设置失败  0= 执行中  1= 设置成功
**备注:         
****************************************************************************************/
int8_t App_HumanSensorSet(uint8_t distance, uint8_t *pfirtflg)
{
    #if defined  IR_FUNCTION_ON               
    return App_IR_SetSenseMode( (DRV_NEAR_MODE_E)distance, pfirtflg );
	#elif defined RADAR_FUNCTION_ON   	
    DRV_Radar_SetMode( distance ); 
    #endif
	return 1;
}


 
















