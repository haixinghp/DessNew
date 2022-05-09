/********************************************************************************************************************
 * @file:        App_Export.c
 * @author:      gushengchi
 * @version:     V01.00
 * @date:        2021-08-31
 * @Description: 扩展口端口采集功能数据   实现实时采集扩展口的电平状态，业务逻辑处可以直接用结果
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "App_Export.h" 
 
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON 
    #define NEAR_SENSE_EN   1
#else
    #define NEAR_SENSE_EN   0
#endif 

/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/
static const uint8_t CtrlRegTab[ MAX_PIN_NUM ][ 2 ] = 
{
      	/*有效电平*/   /*使能*/
    {      	0,    	  	 1   	 },       //FINGER_IRQ  
	{      	0,      	 1   	 },       //ALARM_IRQ 
    {      	0,     NEAR_SENSE_EN },       //SENSE_IRQ  
	{       0,      	 1   	 },       //TOUCH_IRQ 
};
 

/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         
 

/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
static uint16_t ExportSystick = 0; 

typedef struct  
{
    struct  
    {
		uint8_t ValidSts      :1;               
		uint8_t CaptEn        :1;     		
		uint8_t Reverse       :6; 
              
    }StsReg;       
}ExportMeg_T; 

static volatile ExportMeg_T ExportMeg[ MAX_PIN_NUM ];
static volatile uint8_t  InputPinStatus[ MAX_PIN_NUM ] ={0}; 
static volatile bool  AlarmWorkEn = 1;   			//防撬检测功能生效  
/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_Export_FileInit()
* Description   :  相关文件初始化   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Export_FileInit( void )
{
    HAL_EXPORT_FileInit(); 
	
	(void)HAL_EXPORT_PinInit( EM_FING_IRQ, DIR_IN, POLO_RETTAIN ); 	
    (void)HAL_EXPORT_PinInit( EM_PIN_ALARM, DIR_IN, POLO_RETTAIN ); 
	(void)HAL_EXPORT_PinInit( EM_IR_IRQ, DIR_IN, POLO_RETTAIN ); 	
    (void)HAL_EXPORT_PinInit( EM_KEY_IRQ, DIR_IN, POLO_RETTAIN ); 	
	
	AlarmWorkEn = 1;
    ExportSystick = 0;
	for(uint8_t i=0; i<MAX_PIN_NUM; i++)
	{
	    ExportMeg[ i ].StsReg.CaptEn = CtrlRegTab[ i ][ 1 ];
		ExportMeg[ i ].StsReg.ValidSts = 0;
	}
} 

/*********************************************************************************************************************
* Function Name :  App_Export_WakeupInit()
* Description   :  唤醒配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Export_WakeupInit( void )
{
	HAL_EXPORT_WakeupInit(); 
	
    ExportSystick = 0;
	for(uint8_t i=0; i<MAX_PIN_NUM; i++)
	{
	    ExportMeg[ i ].StsReg.CaptEn = CtrlRegTab[ i ][ 1 ];
		ExportMeg[ i ].StsReg.ValidSts = 0;
	}
} 

/*********************************************************************************************************************
* Function Name :  App_Export_SleepInit()
* Description   :  休眠配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Export_SleepInit( void )
{
	HAL_EXPORT_SleepInit(); 
} 
 
/*********************************************************************************************************************
* Function Name :  App_Export_Tim10Ms()
* Description   :  功能相关定时器   10ms触发一次
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Export_Tim10Ms( void )
{
	if( ExportSystick > 0 )
		ExportSystick--;
} 
 
/*********************************************************************************************************************
* Function Name :  App_Export_ScanPortThread()
* Description   :  功能调度  10ms调度一次  
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Export_ScanPortThread( void )
{
 	static uint8_t timecnt;
	
	timecnt++;
	timecnt %= 10;
	
	if( timecnt == 0 )
	{
		if( CtrlRegTab[ E_PIN_FINGER_IRQ ][ 1 ] == ExportMeg[ E_PIN_FINGER_IRQ ].StsReg.CaptEn )
		{
			if( CtrlRegTab[ E_PIN_FINGER_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_FING_IRQ ) )
			{
				
				ExportMeg[ E_PIN_FINGER_IRQ ].StsReg.ValidSts = 1; 
			}
			else 
			{
				ExportMeg[ E_PIN_FINGER_IRQ ].StsReg.ValidSts = 0; 
			}	
		}
	}
	else if( timecnt == 1 )
	{
		if( CtrlRegTab[ E_PIN_ALARM_IRQ ][ 1 ] == ExportMeg[ E_PIN_ALARM_IRQ ].StsReg.CaptEn )
		{
			if( CtrlRegTab[ E_PIN_ALARM_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_PIN_ALARM ) )
			{
				ExportMeg[ E_PIN_ALARM_IRQ ].StsReg.ValidSts = (AlarmWorkEn == 1)? 1:0; 
			}
			else 
			{
				ExportMeg[ E_PIN_ALARM_IRQ ].StsReg.ValidSts = 0; 
			}	
		}
	}
	else if( timecnt == 2 )
	{
		if( CtrlRegTab[ E_PIN_SENSE_IRQ ][ 1 ] == ExportMeg[ E_PIN_SENSE_IRQ ].StsReg.CaptEn )
		{
			if( CtrlRegTab[ E_PIN_SENSE_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_IR_IRQ ) )
			{
				ExportMeg[ E_PIN_SENSE_IRQ ].StsReg.ValidSts = 1; 
			}
			else 
			{
				ExportMeg[ E_PIN_SENSE_IRQ ].StsReg.ValidSts = 0; 
			}	
		}
	}
	else if( timecnt == 3 )
	{
	    if( CtrlRegTab[ E_PIN_TOUCH_IRQ ][ 1 ] == ExportMeg[ E_PIN_TOUCH_IRQ ].StsReg.CaptEn )
		{
			if(  CtrlRegTab[ E_PIN_TOUCH_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_KEY_IRQ ) )
			{
				ExportMeg[ E_PIN_TOUCH_IRQ ].StsReg.ValidSts = 1; 
			}
			else 
			{
				ExportMeg[ E_PIN_TOUCH_IRQ ].StsReg.ValidSts = 0; 
			}	
		}		
	}
} 
 
/*********************************************************************************************************************
* Function Name :  App_Export_GetPinState()
* Description   :  获取扩展口输入有效电平
* Para Inpuut   :  type - 引脚类型
* Return        :  引脚状态  0-无效 1-有效
*********************************************************************************************************************/
bool App_Export_GetPinState( PIN_TYPE_E type )
{
	return ExportMeg[ type ].StsReg.ValidSts; 
}


/*********************************************************************************************************************
* Function Name :  App_Key_GetAlrmWarmState()
* Description   :  获取防撬按键状态
* Para          :  无
* Return        : 0= 无效  1= 有效
*********************************************************************************************************************/
bool App_Export_GetAlrmWarmState( void )
{
	bool ret;
	uint8_t tp1 = HAL_EXPORT_PinGet( EM_PIN_ALARM );
	ret = (tp1 == CtrlRegTab[ E_PIN_ALARM_IRQ ][ 0 ]) ?  1 : 0;
	
	if( AlarmWorkEn == 0 )
		ret = 0;
	
	return ret;
}

/*********************************************************************************************************************
* Function Name :  App_Export_SetAlrmWarmEn()
* Description   :  设置防撬按键检测工作状态  
* Para          :  0= 失能  1= 使能
* Return        :  void
*********************************************************************************************************************/
void App_Export_SetAlrmWarmEn( bool cmd )
{
	AlarmWorkEn = cmd;
}
 
/*********************************************************************************************************************
* Function Name :  App_Export_GetAlrmWarmEnSts()
* Description   :  获取防撬按键检测工作状态  
* Para          :  none
* Return        :  0= 失能  1= 使能
*********************************************************************************************************************/
bool App_Export_GetAlrmWarmEnSts( void )
{
	return AlarmWorkEn;
}


/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
