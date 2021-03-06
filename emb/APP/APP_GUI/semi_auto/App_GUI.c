/********************************************************************************************************************
 * @file:      App_GUI.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-02
 * @brief:     系统主流程函数  
 * @Description:   
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include <string.h>

#include "Public.h"

#include "System.h"
#include "SystemInit.h"
#include "SystemTim.h"

#include "DRV_CLK.h"
#include "DRV_BLE.h"

#include "..\HAL\HAL_ADC\HAL_ADC.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#include "..\HAL\HAL_VOICE\HAL_Voice.h"
#include "..\HAL\HAL_Motor\HAL_Motor.h"
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"

#include "..\Server\Face.h"
#include "ISR.h"

#include "App_GUI.h" 
#include "App_Export.h"
#include "App_LED.h"
#include "App_Touch.h" 
#include "App_Key.h" 
#include "App_PWD.h" 
#include "App_BLE.h"
#include "App_FACE.h"
#include "App_Finger.h"
#include "App_WIFI.h"
#include "APP_ID2.h"
#include "APP_CAM.h"
#include "APP_FACE_PRO.h"
#include "APP_Screen.h"
#include "App_IO.h"
#include "App_CpuCard.h"
#include <app_task.h>
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define  GUI_TIME_50MS      5
#define  GUI_TIME_100MS     10 
#define  GUI_TIME_200MS     20
#define  GUI_TIME_300MS     30
#define  GUI_TIME_400MS     40
#define  GUI_TIME_500MS     50
#define  GUI_TIME_600MS     60
#define  GUI_TIME_700MS     70

#define  GUI_TIME_1S        100 
#define  GUI_TIME_1200MS    120 
#define  GUI_TIME_1500MS    150 
#define  GUI_TIME_1800MS    180 
#define  GUI_TIME_2S        200 
#define  GUI_TIME_2500MS    250
#define  GUI_TIME_3S        300 
#define  GUI_TIME_3500MS    350
#define  GUI_TIME_4S        400 
#define  GUI_TIME_5S        500 
#define  GUI_TIME_6S        600 

#define  GUI_TIME_7S        700 
#define  GUI_TIME_8S        800 
#define  GUI_TIME_9S        900 
#define  GUI_TIME_10S       1000 
#define  GUI_TIME_12S       1200 
#define  GUI_TIME_13S       1300 
#define  GUI_TIME_15S       1500 
#define  GUI_TIME_16S       1600 
#define  GUI_TIME_20S       2000 
#define  GUI_TIME_25S       2500 
#define  GUI_TIME_26S       2600 
#define  GUI_TIME_30S       3000 
#define  GUI_TIME_40S       4000 
#define  GUI_TIME_50S       5000 
#define  GUI_TIME_60S       6000 
#define  GUI_TIME_900S      90000 
 
#define  STAY_DETECT_TIME_S          30     //逗留检测持续时间       单位 秒
#define  PUSAL_WORK_TIME_SHORT_S     15     //门外开门成功后接近感应暂停工作时间   单位 秒
#define  PUSAL_WORK_TIME_LONG_S      30     //门内开门成功后接近感应暂停工作时间   单位 秒
#define  NET_CONNECT_FIRST_TIME_S    60		//网络重连的初始间隔时间 单位 秒
/*-------------------------------------------------枚举定义---------------------------------------------------------*/
typedef enum
{
	E_CHECK_DEFAULT, 
	E_CHECK_FACE, 
	E_CHECK_FINGER, 
	
}CHECK_KEY_TYPE_E;

/*-------------------------------------------------常量定义---------------------------------------------------------*/

/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         
bool  FingerWorkState = false;

uint8_t UploadUnlockDoorMegEnable = 0;
/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
static void (*DispFuncPtr)(); 
static void (*DispFuncPtrPre)();  

static MenuItemType_T MenuItem;

static volatile uint32_t GuiQuitTimMs = 0;
static uint32_t GuiDelayTimMs = 0;
static uint8_t  GuiTimerSecond = 0;
static uint8_t  FingerCheckFlow = 0;
static uint8_t  BleDisconnectHoldTimeMs = 0;
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
static TRY_ALARM_E TryAlarmFirst;
#endif

typedef struct 
{
	uint32_t AlamButtonHoldTimSec;	 //防撬报警持续时间计时定时器
	bool     AlamButtonNewTrigger;	 //防撬是一次触发
}AlarmMeg_S;

typedef struct
{
	uint8_t  TouchLockNum;       //门外锁门键锁门计数器
    uint8_t  SystemSleepState;   //系统休眠状态        1:休眠   0:工作
	uint8_t  AutoLockTimPara;    //自动上锁时间        单位秒
	uint8_t  StayDetectTimPara;  //逗留检测告警时间    单位秒
	uint8_t  DoorCurState;       //门锁当前状态        0: default  1:open  2:close 
	uint8_t  NearSensePusalTim;  //红外感应检测暂停时间    单位秒
	uint32_t NetUpdatePeroidTim; //网络同步间隔时间        单位秒
    uint32_t NetUpdateSetTimPara;//网络同步设置的间隔时间  单位秒
    bool     NetUpdateFuncEnSts; //网络同步开关状态    1:使能   0:失能
 
    uint16_t WeatherUpdatePeroidTim; //天气同步间隔时间        单位秒
    uint16_t WeatherUpdateSetTimPara;//天气同步设置的间隔时间  单位秒
    uint8_t  WeatherUpdateErrCnt; 	 //天气同步错误计数    
	uint8_t  WeatherUpdateEnSts; 	 //天气同步使能状态      1:使能   0:失能
	
    bool  StayDetectFlg;           //逗留检测是否触发过   0:未触发  1:已触发
	bool  StaySenseCheckEn;        //逗留检测传感器       0:不检测  1:检测
	
	OPEN_MODEL_E   OpenDoorModel;  //开门方式
    CLOSE_MODEL_E  CloseDoorModel; //关门方式
	
	WAKEUP_TYPE_E  WakeupSourceType;  //系统的唤醒类型
	
    WORK_MODE_E    SysWorkMode;       //系统工作模式
	
	AlarmMeg_S AlamMeg;

}s_GuiData_T;

static s_GuiData_T s_GuiData = {0};
static uint32_t AgingTestTimSec = 0;
static SYSTEM_WORK_STS_E GuiSchedulerStep;

#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
static bool  FaceWorkStsFlg = false;
#endif
static bool	 FingerWorkStsFlg = false;
static bool	 TouchLockKeyPushStsFlg = false;
/*-------------------------------------------------函数声明---------------------------------------------------------*/


/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_GUI_FileInit()
* Description   :  相关初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_FileInit( void )
{
	s_GuiData.TouchLockNum = 0;
	s_GuiData.SystemSleepState = 0;
	s_GuiData.OpenDoorModel = EM_OPEN_DEFAULT;
	s_GuiData.CloseDoorModel= EM_CLOSE_DEFAULT;
	s_GuiData.WakeupSourceType = E_WAKE_DEFAULT;
	s_GuiData.SysWorkMode = E_MODE_DEFAULT;
	s_GuiData.DoorCurState = 0;
	s_GuiData.AutoLockTimPara = 0;
	s_GuiData.StayDetectTimPara = 0;
	s_GuiData.StayDetectFlg = 0;
	s_GuiData.StaySenseCheckEn = false;
	s_GuiData.NearSensePusalTim = 0;
	s_GuiData.NetUpdatePeroidTim = 0;
	s_GuiData.NetUpdateSetTimPara = NET_CONNECT_FIRST_TIME_S;
	s_GuiData.NetUpdateFuncEnSts = false;

	(void)DRV_InterGenerateRandVec( (uint8_t *)&s_GuiData.WeatherUpdateSetTimPara, 2 );//随机数2个
	s_GuiData.WeatherUpdateSetTimPara %= 7200;
	
	s_GuiData.AlamMeg.AlamButtonNewTrigger = false;
	s_GuiData.AlamMeg.AlamButtonHoldTimSec = 0;
	
	s_GuiData.WeatherUpdatePeroidTim = 0;
	s_GuiData.WeatherUpdateErrCnt = 0;
	s_GuiData.WeatherUpdateEnSts = 0;
	App_GUI_MenuJump( MENU_NULL );
	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_WakeupInit()
* Description   :  唤醒后配置
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_WakeupInit( void )
{
	if( (E_WAKE_FINGER == App_GUI_GetSysWakeupType()) ||
		(E_WAKE_NEAR_SENSE == App_GUI_GetSysWakeupType()) ||
		(E_WAKE_TOUCH == App_GUI_GetSysWakeupType()) )   //前面板唤醒
	{
		if(SystemSeting.SysHumanIrDef == 0)
		{
			s_GuiData.StayDetectTimPara = 0;  //逗留检测赋值 
		}
		else if(s_GuiData.StayDetectTimPara == 0)
		{
			s_GuiData.StayDetectTimPara = SystemSeting.SysHumanIrDef;  //逗留检测赋值
			my_printf("s_GuiData.StayDetectTimPara = %d\n", SystemSeting.SysHumanIrDef);
		}
	}
	s_GuiData.OpenDoorModel = EM_OPEN_DEFAULT;
	s_GuiData.CloseDoorModel= EM_CLOSE_DEFAULT;
	if( E_WAKE_AUTO_LOCK != App_GUI_GetSysWakeupType() )      //自动上锁不打断门内开门红外暂停工作的时间
	{
	    s_GuiData.NearSensePusalTim = 0;
	}
	
	(void)DRV_InterGenerateRandVec( (uint8_t *)&s_GuiData.WeatherUpdateSetTimPara, 2 );//随机数2个
	my_printf( "WeatherUpdateSetTimPara= %d!\n", s_GuiData.WeatherUpdateSetTimPara ); 
	s_GuiData.WeatherUpdateSetTimPara %= 7200;
	
	UploadUnlockDoorMegEnable = 0;
	
	App_GUI_MenuJump( MENU_NULL );	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SleepInit()
* Description   :  休眠后配置
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SleepInit( void )
{
	s_GuiData.OpenDoorModel = EM_OPEN_DEFAULT;
	s_GuiData.CloseDoorModel= EM_CLOSE_DEFAULT;
	s_GuiData.WakeupSourceType = E_WAKE_DEFAULT;
	
	s_GuiData.AlamMeg.AlamButtonNewTrigger = false;
	s_GuiData.AlamMeg.AlamButtonHoldTimSec = 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSystemWorkSts()
* Description   :  设置系统工作状态
* Input  Para   :  workstate- 系统工作状态
* Output Para   :  none
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetSystemWorkSts( SYSTEM_WORK_STS_E workstate )
{
    GuiSchedulerStep = workstate;
} 

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSystemWorkSts()
* Description   :  获取系统工作状态
* Input  Para   :  none
* Output Para   :  none
* Return        :  SYSTEM_WORK_STS_E- 工作状态
*********************************************************************************************************************/
SYSTEM_WORK_STS_E App_GUI_GetSystemWorkSts( void )
{
	return GuiSchedulerStep;
} 

/*********************************************************************************************************************
* Function Name :  App_GUI_StopStayDetectTim()
* Description   :  停止当次逗留计时
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_StopStayDetectTim(void)
{
	s_GuiData.StayDetectTimPara = 0;  //逗留检测赋值 
	s_GuiData.StayDetectFlg = 0;	//逗留检测是否触发过   0:未触发  1:已触发
	s_GuiData.StaySenseCheckEn = false;
}
/*********************************************************************************************************************
* Function Name :  App_GUI_Tim10Ms()
* Description   :  相关定时器  10ms执行一次
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_Tim10Ms( void )
{
	if( GuiQuitTimMs > 0 )
		GuiQuitTimMs--;

	if( GuiDelayTimMs > 0 )
		GuiDelayTimMs--;
	
	if( AgingTestTimSec > 0 )
		AgingTestTimSec--;
	
	if( s_GuiData.AlamMeg.AlamButtonHoldTimSec > 0 )
		s_GuiData.AlamMeg.AlamButtonHoldTimSec--;

	
	if( GuiTimerSecond > 0 )
		GuiTimerSecond--;
	if( GuiTimerSecond == 0 )
	{
		GuiTimerSecond = 100;
		
		if( s_GuiData.AutoLockTimPara > 0 )
		    s_GuiData.AutoLockTimPara--;
		
	   /*-------接近感应暂停工作时间------*/
		if( s_GuiData.NearSensePusalTim > 0 )
			s_GuiData.NearSensePusalTim--;
		
		if( BleDisconnectHoldTimeMs > 0 )
			BleDisconnectHoldTimeMs--;
		
	   /*-------天气同步重连时间---------*/
		if( s_GuiData.WeatherUpdatePeroidTim )
		{
			s_GuiData.WeatherUpdatePeroidTim--;
		} 
		
//		my_printf( "GuiQuitTimMs = %d\n", GuiQuitTimMs ); 
		
			/*-----------逗留检测-------------*/
		if( s_GuiData.StayDetectTimPara > 0 )
		{
			s_GuiData.StayDetectTimPara--;
			my_printf("s_GuiData.StayDetectTimPara = %d\n", s_GuiData.StayDetectTimPara);
		}
		#if defined IR_FUNCTION_ON
		if( (s_GuiData.NearSensePusalTim == 0) 
		 && (1 == s_GuiData.StayDetectTimPara) 
		 && (SystemSeting.SysHumanIrDef) 
		 && (SystemSeting.SysDrawNear != E_SENSE_OFF) )
		{
			s_GuiData.StaySenseCheckEn = true;
		}
		#elif defined RADAR_FUNCTION_ON
		if( (s_GuiData.NearSensePusalTim == 0) 
		&& (App_GUI_StayDetectSts() == 1) 
		&& (SystemSeting.SysHumanIrDef) 
		&& (SystemSeting.SysDrawNear != E_SENSE_OFF) )
		{
			s_GuiData.StaySenseCheckEn = true;	
		}
		#endif
	}	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckStayDetectAction()
* Description   :  将测逗留检测是否触发
* Para          :  无
* Return        :  false= 未触发  true= 已触发
*********************************************************************************************************************/
static bool App_GUI_CheckStayDetectAction( void )
{
	if( s_GuiData.StaySenseCheckEn == true )
	{
		s_GuiData.StaySenseCheckEn = false;
		if( 0 == HAL_EXPORT_PinGet( EM_IR_IRQ ) )  //逗留检测
		{	
			my_printf( "wake by auto stay detect!\n" ); 
			return true;
		}
	}
	return false;
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_GetAutoLockTimPara()
* Description   :  获取自动上锁设定的时间
* Para          :  无
* Return        :  自动上锁时间参数 单位 秒 
*********************************************************************************************************************/
static uint8_t App_GUI_GetAutoLockTimPara( void )
{
	return SystemSeting.SysAutoLockTime;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetAutoLockSts()
* Description   :  获取自动上锁的条件
* Para          :  none 
* Return        :  0: 不满足   1: 满足
*********************************************************************************************************************/
static bool App_GUI_GetAutoLockSts( void )
{
	#ifdef LOCK_BODY_212_MOTOR
	if( (s_GuiData.AutoLockTimPara == 0) && (App_GUI_GetAutoLockTimPara()) && (s_GuiData.DoorCurState == DOOR_CUR_OPEN) )
	{
		return 1;
	}
	#elif defined LOCK_BODY_AUTO_MOTOR
    if( LockConfigMode == LOCK_BODY_212 )
	{
		if( (s_GuiData.AutoLockTimPara == 0) && (App_GUI_GetAutoLockTimPara()) && (s_GuiData.DoorCurState == DOOR_CUR_OPEN) )
		{
			return 1;
		}
	}
	#endif
	return 0;
}


/*********************************************************************************************************************
* Function Name :  App_GUI_StayDetectSts()
* Description   :  逗留时间达到的条件
* Para          :  none 
* Return        :  0: 不满足   1: 满足
*********************************************************************************************************************/
uint8_t App_GUI_StayDetectSts( void )
{
	if((3 == s_GuiData.StayDetectTimPara) || (1 == s_GuiData.StayDetectTimPara) || (2 == s_GuiData.StayDetectTimPara))
	{
		return 1;
	}
	return 0;
}
/*********************************************************************************************************************
* Function Name :  App_GUI_Tim1000Ms()
* Description   :  相关定时器  1000ms执行一次 休眠后启用 唤醒后关闭该定时器
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_Tim1000Ms( void )
{	
  #ifdef WEATHER_FORECAST_ON
	static uint32_t datetamp;
  #endif
   /*-----------自动上锁判定----------*/
	if( s_GuiData.AutoLockTimPara > 0 )
	{
	    s_GuiData.AutoLockTimPara--;	
	}
	if( 1 == App_GUI_GetAutoLockSts() )
	{
		my_printf( "wake by auto lock!\n" ); 
        App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  //设置唤醒源		
		App_GUI_SetSysSleepSts( false );	
	}
	
   /*-------接近感应暂停工作时间------*/
	if( s_GuiData.NearSensePusalTim > 0 )
		s_GuiData.NearSensePusalTim--;
	
   
   /*-----------门未关+假锁检测------*/
    HAL_Motor_Tim1000Ms(); 
	if( true == HAL_Motor_GetForgetLockWarmState() )      //门未关告警
	{
		my_printf( "wake by fortget lock door!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}
    else if( true == HAL_Motor_GetFalseLockWarmState() )  //假锁告警
	{
		my_printf( "wake by false lock door!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}
    else if( true == HAL_Motor_GetHandleTryForbitWarmState() )  //把手试玩告警
	{
		my_printf( "wake by handler try forbit!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}

   /*-----------网络同步检测---------*/
	if( s_GuiData.NetUpdatePeroidTim > 0 )
	    s_GuiData.NetUpdatePeroidTim--;
	
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON   //二代猫眼
	if( (SystemSeting.SysWifiMainSw == FUNCTION_ENABLE)  //已配网
      &&(true == s_GuiData.NetUpdateFuncEnSts)           //功能使能
	  &&(0 == s_GuiData.NetUpdatePeroidTim)              //计时到
	  &&(true == App_GUI_GetNetworkErrState())           //网络连接异常
	   )  
	{
		my_printf( "wake by network update!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_NETWORK_UPDATE );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}
	#endif
 
   /*-----------天气同步检测---------*/
	#ifdef WEATHER_FORECAST_ON
	if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )  //已配网
	{
		if( s_GuiData.WeatherUpdatePeroidTim )
		{
			s_GuiData.WeatherUpdatePeroidTim--;
		} 
	
        if( s_GuiData.WeatherUpdateEnSts == 1 )
		{
			if( ((Rtc_Real_Time.timestamp + 28800) % 86400) > (7200 + s_GuiData.WeatherUpdateSetTimPara)&&((Rtc_Real_Time.timestamp + 28800)% 86400 < 14400 ) )
			{
				if( datetamp != (Rtc_Real_Time.timestamp + 28800)/86400 )
				{
					datetamp = (Rtc_Real_Time.timestamp + 28800)/86400;
					s_GuiData.WeatherUpdateErrCnt = 0;
					my_printf( "wake by weather update!\n" ); 
					App_GUI_SetSysWakeupType( E_WAKE_WEATHER_UPDATE );    					
					App_GUI_SetSysSleepSts( false );
					return;
				}
			}
			
			if( s_GuiData.WeatherUpdateErrCnt == 1 )  //同步失败
			{
				if( s_GuiData.WeatherUpdatePeroidTim == 0 )
				{
					my_printf( "wake by weather update 2!\n" ); 
					App_GUI_SetSysWakeupType( E_WAKE_WEATHER_UPDATE );    					
					App_GUI_SetSysSleepSts( false );
					return;
				} 
			}
		}
	}
	#endif 
	
			/*-----------逗留检测-------------*/
	if( s_GuiData.StayDetectTimPara > 0 )
	{
		s_GuiData.StayDetectTimPara--;
		my_printf("s_GuiData.StayDetectTimPara = %d\n", s_GuiData.StayDetectTimPara);
	}
	#if defined IR_FUNCTION_ON
	if( (s_GuiData.NearSensePusalTim == 0) && (1 == s_GuiData.StayDetectTimPara) && (SystemSeting.SysHumanIrDef) && (SystemSeting.SysDrawNear != E_SENSE_OFF) )
	{
			if( 0 == HAL_EXPORT_PinGet( EM_IR_IRQ ) )  //逗留检测
			{
				s_GuiData.StayDetectFlg = 0;
				s_GuiData.StaySenseCheckEn = false;
				my_printf( "wake by auto stay detect!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  //设置唤醒源						
				App_GUI_SetSysSleepSts( false );
				return;
			}
	}
	#elif defined RADAR_FUNCTION_ON
	if( (s_GuiData.NearSensePusalTim == 0) 
	&& (App_GUI_StayDetectSts() == 1) 
	&& (SystemSeting.SysHumanIrDef) 
	&& (SystemSeting.SysDrawNear != E_SENSE_OFF) )
	{
		if( 0 == HAL_EXPORT_PinGet( EM_IR_IRQ ) )  //逗留检测
		{
			s_GuiData.StayDetectFlg = 0;
			s_GuiData.StaySenseCheckEn = false;
			my_printf( "wake by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  //设置唤醒源						
			App_GUI_SetSysSleepSts( false );
			return;
		}	
	}
	#endif
   /*-----------THE END--------------*/
}

/*********************************************************************************************************************
* Function Name :  App_GUI_MenuJump()
* Description   :  GUI界面跳转函数
* Para          :  pageNo- 待跳转的界面编号
* Return        :  void
*********************************************************************************************************************/
void App_GUI_MenuJump( MenuIndexEnum_E pageNo )
{
    MenuItem.MenuIndexType.Currently = pageNo; 
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetCurMenuNo()
* Description   :  获取当前菜单编号
* Para          :  none
* Return        :  当前菜单编号
*********************************************************************************************************************/
MenuIndexEnum_E  App_GUI_GetCurMenuNo( void )
{
	return MenuItem.MenuIndexType.Currently; 
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetRegisterSts()
* Description   :  获取系统的注册状态
* Para          :  无
* Return        :  0xA5B7= 未注册 1= 本地管理员已注册  2= APP管理员已注册
*********************************************************************************************************************/
uint16_t App_GUI_GetRegisterSts( void )
{
	return SystemSeting.SystemAdminRegister;
}


/*********************************************************************************************************************
* Function Name :  App_GUI_GetRegisterSts()
* Description   :  设置系统的注册状态
* Para          :  无
* Return        :  0xA5B7= 未注册 1= 本地管理员已注册  2= APP管理员已注册
*********************************************************************************************************************/
static void App_GUI_SetRegisterSts( uint16_t type )
{
    SystemSeting.SystemAdminRegister = type;
	(void)SystemWriteSeting( (uint8_t *)&SystemSeting.SystemAdminRegister, sizeof SystemSeting.SystemAdminRegister );
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetDoubleCheckSwSts()
* Description   :  获取双重认证功能开关状态
* Para          :  无
* Return        :  0= 关闭   1= 开启   
*********************************************************************************************************************/
bool App_GUI_GetDoubleCheckSwSts( void )
{
	uint8_t ret =0;
	if( SystemSeting.SysCompoundOpen == DOUBLE_CHECK_SW_OFF )
	{
		ret = 0;
	}
    else if( SystemSeting.SysCompoundOpen == DOUBLE_CHECK_SW_ON )
	{
		ret = 1;
	}
	else 
	{
		ret = 0;
	}
	return ret;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetDoubleCheckType()
* Description   :  获取双重认证组合类型
* Para          :  无
* Return        :  组合状态
*********************************************************************************************************************/
DoubleCheckType_U App_GUI_GetDoubleCheckType( void )
{
	DoubleCheckType_U type; 
	type.data = SystemSeting.SysLockMode;
	return type;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSysSleepSts()
* Description   :  获取系统休眠状态
* Para          :  无
* Return        :  0= 正常   1= 休眠中   
*********************************************************************************************************************/
uint8_t App_GUI_GetSysSleepSts( void )
{
	return s_GuiData.SystemSleepState; 
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysSleepSts()
* Description   :  设置系统休眠状态
* Para          :  无
* Return        :  0= 关闭   1= 开启   
*********************************************************************************************************************/
void App_GUI_SetSysSleepSts( uint8_t para )
{
	s_GuiData.SystemSleepState = para;
	#if LOCK_PROJECT_CHIP ==LOCK_PROJECT_RTL8762 
	if(para==0)//确认本次是否唤醒
	{
		APP_TaskSendMsg();/* Send msg to app task */
	}
	#endif	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_RelieveTryProtect()
* Description   :  解除禁止保护
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_RelieveTryProtect( void )
{
	s_GuiData.TouchLockNum = 0;
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
	TryAlarmFirst = EM_TRY_DEFAULT;
#endif
	if( SystemSeting.CheckErrAllCnt )  //防止频繁写入
	{
		SystemSeting.CheckErrAllCnt = 0;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
	}
	if( SystemSeting.CheckErrPwdCnt )  //防止频繁写入
	{
		SystemSeting.CheckErrPwdCnt = 0;  
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrPwdCnt, sizeof SystemSeting.CheckErrPwdCnt );	
	}
	App_GUI_StopStayDetectTim();//停止当次逗留计时
}

/*********************************************************************************************************************
* Function Name :  App_GUI_UpdateMenuQuitTime()
* Description   :  更新GUI退出时间    
* Para          :  para-待刷新的数据   单位10ms  mode- 是否强制更新   false= 非强制   true= 强制
* Return        :  void 
*********************************************************************************************************************/
void App_GUI_UpdateMenuQuitTime( uint32_t para, bool mode )
{
	if( mode == true )
	{
		GuiQuitTimMs = para;
	}
	else 
	{
	 	if( GuiQuitTimMs < para )
		{
			GuiQuitTimMs = para;
		}
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetOpenModel()
* Description   :  获取开门方式
* Para          :  none
* Return        :  OPEN_MODEL_E -开门方式   
*********************************************************************************************************************/
OPEN_MODEL_E App_GUI_GetOpenModel( void )
{
 	return s_GuiData.OpenDoorModel;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetCloseModel()
* Description   :  获取关门方式
* Para          :  none
* Return        :  CLOSE_MODEL_E -关门方式  
*********************************************************************************************************************/
CLOSE_MODEL_E App_GUI_GetCloseModel( void )
{
 	return s_GuiData.CloseDoorModel;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetNearSenseUnworkCurTim()
* Description   :  获取开门成功后接近感应不工作的时间 单位秒
* Para          :  none
* Return        :  当前剩余时间
*********************************************************************************************************************/
uint8_t App_GUI_GetNearSenseUnworkCurTim( void )
{
 	return s_GuiData.NearSensePusalTim;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetOpenModel()
* Description   :  设置开门方式
* Para          :  OPEN_MODEL_E -开门方式   
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetOpenModel( OPEN_MODEL_E type )
{
	my_printf( "App_GUI_SetOpenModel()\n" );   
    s_GuiData.OpenDoorModel = type;
	s_GuiData.DoorCurState = DOOR_CUR_OPEN;
	s_GuiData.AutoLockTimPara = App_GUI_GetAutoLockTimPara();
	
	if( (type == EM_OPEN_HANDLER) || (type == EM_OPEN_BUTTON) )
	{
		s_GuiData.NearSensePusalTim	= PUSAL_WORK_TIME_LONG_S;
	}
	else 
	{
		s_GuiData.NearSensePusalTim	= PUSAL_WORK_TIME_SHORT_S;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetCloseModel()
* Description   :  设置关门方式
* Para          :  CLOSE_MODEL_E -关门方式  
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetCloseModel( CLOSE_MODEL_E type )
{
	my_printf( "App_GUI_SetCloseModel()\n" );   
    s_GuiData.CloseDoorModel = type;
	s_GuiData.DoorCurState = DOOR_CUR_CLSOE;
	s_GuiData.AutoLockTimPara = 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DefaultDoorState()
* Description   :  恢复门的状态
* Para          :  none
* Return        :  void
*********************************************************************************************************************/
void App_GUI_DefaultDoorState( void )
{
	s_GuiData.DoorCurState = 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSysWakeupType()
* Description   :  获取系统唤醒方式
* Para          :  none 
* Return        :  系统唤醒方式  WAKEUP_TYPE_E
*********************************************************************************************************************/
WAKEUP_TYPE_E  App_GUI_GetSysWakeupType( void )
{
   return s_GuiData.WakeupSourceType;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysWakeupType()
* Description   :  设置关门方式
* Para Input    :  type -唤醒方式  
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetSysWakeupType( WAKEUP_TYPE_E type )
{
	s_GuiData.WakeupSourceType = type;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSysWorkMode()
* Description   :  获取系统工作模式
* Para          :  none 
* Return        :  系统工作模式  WORK_MODE_E
*********************************************************************************************************************/
WORK_MODE_E  App_GUI_GetSysWorkMode( void )
{
   return s_GuiData.SysWorkMode;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysSysWorkMode()
* Description   :  设置系统工作模式
* Para Input    :  mode -系统工作模式  
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetSysSysWorkMode( WORK_MODE_E mode )
{
	s_GuiData.SysWorkMode = mode;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetWifiUploadSwSts()
* Description   :  获取WIFI数据上传的开关状态
* Para Input    :  none
* Return        :  0: 关闭  1: 开启
*********************************************************************************************************************/
bool App_GUI_GetWifiUploadSwSts( void )
{
    if( (SystemSeting.SysWifiMainSw == FUNCTION_ENABLE) && (SystemSeting.SysWifiLogSw == FUNCTION_ENABLE) )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //小电池未查
			{
				return false;	
			}
			else 
				return true;
		#else
			return true;
		#endif
	}
	return false;	
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_GetWifiWarmingSwSts()
* Description   :  获取WIFI 推送告警记录的开关状态
* Para Input    :  none
* Return        :  0: 关闭  1: 开启
*********************************************************************************************************************/
static bool App_GUI_GetWifiWarmingSwSts( void )
{
    if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //小电池未查
			{
				return false;	
			}
			else 
				return true;
		#else
			return true;
		#endif
	}
	return false;	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_PwdCheckLock()
* Description   :  密码验证处理
* Para Input    :  checkEnable: 密码验证使能位  0=不验证密码  1=验证密码
				   otherPwdCheckEn: 报警&临时密码验证使能位 0=不验证密码  1=验证密码
* Para Output   :  pPwdMeg: 验证通过后的密码信息
* Return        :  -1= 验证失败 0= 执行中  1= 验证成功  2= 故障查询  3= 老化测试  4= 验证管理员 
                    5= 按键锁门 6= 首次使用流程  7= 门铃 8= 临时密码验证成功 9= 报警密码验证成功
*********************************************************************************************************************/
static int8_t App_GUI_PwdCheckAndTouchHandler( uint8_t checkEnable, uint8_t otherPwdCheckEn, PwdMeg_T *pPwdMeg )
{
    uint8_t pwdLen =0;
	uint8_t pwdBuf[KEY_BUF_SIZE+1] = {0};
	uint8_t pwddata[KEY_BUF_SIZE+1]= {0};
	
	uint8_t tp1 = App_Touch_GetCurrentKeyValue();
	if( TOUCH_KEY_ENTER == tp1 )     //确认键
	{
		uint8_t tm1 = App_Touch_GetCurrentKeyIndex();
		App_Touch_GetCurrentKeyValBuf( pwdBuf, &pwdLen );
		
        if( true == TouchLockKeyPushStsFlg )   //防误触开启
		{
			TouchLockKeyPushStsFlg = false;
		    if( s_GuiData.TouchLockNum < 5 )
			{
				s_GuiData.TouchLockNum++;
			}
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
			return 5; 	
		}
		
		if( false==checkEnable && false==otherPwdCheckEn )  //判定是否验证密码/临时密码
			return 0;
		
		if( tm1 >= 4 )  //暗码6688#
		{
			#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
			
			#else 
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //体验模式
			{
				uint8_t key_register[4]={0x36,0x36,0x38,0x38};
				PUBLIC_ChangeDecToString( pwddata, pwdBuf, pwdLen );
				if( 0 == memcmp( pwddata, key_register, 4 ) )
				{
					return 6;
				} 
			}
			#endif
		}
		if( tm1 >= 6 )  //密码or暗码处理
		{
			if( 7 == tm1 )    //故障码查询暗码处理
			{
		        #if defined(XM_CAM_FUNCTION_ON)||defined(SMART_SCREEN_ON)
				uint8_t key_temp[7]={0x32,0x35,0x38,0x37,0x34,0x35,0x31};
				PUBLIC_ChangeDecToString( pwddata, pwdBuf, pwdLen );
				if( 0 == memcmp( pwddata, key_temp, 7 ) )
				{
					return 2;
				}
                #endif
			}
			else if( 10 == tm1 ) //暗码处理
			{
				uint8_t born_temp[10]={0x32,0x35,0x38,0x33,0x36,0x39,0x37,0x34,0x35,0x31};
				PUBLIC_ChangeDecToString( pwddata, pwdBuf, pwdLen );
				if( 0 == memcmp( pwddata, born_temp, 10) )
				{
					if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //体验模式方能老化处理
					{
						return 3;
					}
				}
			}
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //体验模式 不验证密码
			{
				return 0;
			}
			
			my_printf("input pwd is:");  
			for(uint8_t i=0; i<pwdLen; i++)
			{
				my_printf(" %d", pwdBuf[i]);  
			}
			my_printf("\r\n");  
			
			PUBLIC_ChangeDecToString( pwddata, pwdBuf, pwdLen );
			pwddata[ pwdLen ] = '\0';
			if( true == checkEnable )  //判定是否验证密码 
			{
				my_printf("check pwd process\n"); 
				if( 1 == App_PWD_VerifyUserPwd( PWD_LIMIT_ALL, pPwdMeg, (char *)pwddata ) )  //匹配到了开门密码
				{
					return 1;
				}
			}
			if( true == otherPwdCheckEn )  //判定是否验报警密码/临时密码
			{
				my_printf("check sos or temp pwd process\n"); 
				if( 1 == App_PWD_VerifySosPwd( (char *)pwddata ) )       //匹配到了报警密码
				{
					return 9;
				}
				else if( 1 == App_PWD_VerifyTempPwd( (char *)pwddata ))  //匹配到了临时密码
				{
					return 8;
				}
			}
			return -1;	
		}
	}
	else if( TOUCH_KEY_BACK == tp1 ) //取消键
	{
		if( ADMIN_LOCAL_REGISTERED == App_GUI_GetRegisterSts() )  //非体验模式
		{
			return 4;
		}
	}
	else if( TOUCH_KEY_LOCK == tp1 ) //上锁键
	{
		if( FUNCTION_ENABLE == SystemSeting.Sysprotect_lock )  //按键防误触开启
		{
			if( s_GuiData.TouchLockNum < 5 )
			{
				TouchLockKeyPushStsFlg = true;
				App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
				App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 	
				App_Touch_FuncEnCtrl( EM_SCAN_KEY_ENTER ); 			
				App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON );
			}
		}
		else
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() )  //体验模式
			{
				s_GuiData.TouchLockNum = 0;
			}
			if( s_GuiData.TouchLockNum < 5 )
			{
				s_GuiData.TouchLockNum++;
				return 5; 
			}
		}
	}
	else if( TOUCH_KEY_BELL == tp1 ) //门铃键
	{	
		return 7;
	}
	
	return 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_FingerCheck()
* Description   :  指纹验证流程
* Para          :  无
* Return        :  -1: 验证失败  0: 验证中  1: 验证成功
*********************************************************************************************************************/
static int8_t App_GUI_FingerCheck( uint16_t *pageId, uint8_t userLimit )
{
    #ifdef FINGER_FUNCTION_ON
	static uint8_t errcnt;
	#endif
	FingerAppParam_S fingerPara;
	FingerAppCfg_S   fingermeg;
	FINGER_APP_FLOW_RESULT_E checkresult;
	
	switch( FingerCheckFlow )
	{
		case 0:  
			    if( FingerWorkState == false )
				{
                    #ifdef FINGER_FUNCTION_ON
					errcnt = 0;
                    #endif
					FingerCheckFlow = 1;
					my_printf( "finger check after! \n" ); 
				}
				else if( FingerWorkState == true )
				{
					my_printf( "finger check befor! \n" ); 
					FingerWorkState = false;
                    #ifdef FINGER_FUNCTION_ON
					errcnt = 0;
                    #endif
					FingerCheckFlow = 2;	
				}
		break;
		
		case 1: //启动指纹检验流程	
				fingerPara.emAppFlow = EM_FINGER_APP_FLOW3_SEARCH;
				APP_FINGER_Operate( fingerPara );   //启动验证指纹流程
		        FingerCheckFlow = 2;
		break;
		
		case 2: //等待检测结果	
				checkresult = APP_FINGER_GetFlowResult( pageId );
				if( FINGER_APP_RESULT_SUC == checkresult )         //验证成功
				{
					if( userLimit == MEM_USER_ALL )  //通用验证
					{
						my_printf( "finger check is all ok! \n" ); 
						APP_FINGER_Sleep();  //关闭指纹模组
						FingerCheckFlow = 0;					
						return 1;
					}
					else
					{
						if( true == APP_FINGER_CfgRead( *pageId, &fingermeg ) )
						{
							if( fingermeg.acOffset[EM_FINGER_APP_CFG_ADMIN_EN] == userLimit )  //验证权限通过
							{
								my_printf( "finger check is limit! \n" ); 
								APP_FINGER_Sleep();  //关闭指纹模组
								FingerCheckFlow = 0;					
								return 1;
							}
						}
					}
					
					my_printf( "finger check is error! \n" ); 
					APP_FINGER_Sleep();  //关闭指纹模组
					FingerCheckFlow = 0;					
					return -1;
				}
				else if( FINGER_APP_RESULT_FAIL == checkresult )   //验证失败
				{ 
					if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) 
					{
						my_printf( "finger check is ok at try mode! \n" );  
						APP_FINGER_Sleep();  //关闭指纹模组
						FingerCheckFlow = 0;
						return 1;
					}
                    #ifdef FINGER_FUNCTION_ON
					if( errcnt == 0 )  //二次验证
					{
						errcnt = 1;
						my_printf( "finger check twice\n" ); 
						FingerCheckFlow = 1;
					}
					else
                    #endif
					{
						APP_FINGER_Sleep();  //关闭指纹模组
						FingerCheckFlow = 0;
						return -1;
					}
				}
                else if(FINGER_APP_PROTOCAL_ERR == checkresult || FINGER_APP_RESULT_TIMEOUT == checkresult)// 超时、协议解析错误等
                {
					APP_FINGER_Sleep();  //关闭指纹模组
					FingerCheckFlow = 0;
					return -2;
                }
		break; 
		
		default:break;
	}	
	return 0;
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_FingerCheckStop()
* Description   :  结束指纹验证流程
* Para Input    :  none
* Para Output   :  none
* Return        :  none
*********************************************************************************************************************/
static void App_GUI_FingerCheckStop( bool *pfisrtflg )
{
	if( *pfisrtflg == false )
		return;
	if( *pfisrtflg == true )
	{
		*pfisrtflg = false;
        APP_FINGER_Sleep();  //关闭指纹模组
		my_printf( "App_GUI_FingerCheckStop()\n" );   
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_FaceCheck()
* Description   :  人脸验证流程
* Para Input    :  userLimit: 用户权限  'M'-管理员  'A'-所有用户    
* Para Output   :  pageId: 人脸id
* Return        :  -1: 验证失败  0: 验证中  1: 验证成功
*********************************************************************************************************************/
#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
static int8_t App_GUI_FaceCheck( uint8_t userLimit, uint16_t *pageId, uint8_t *unlockStatus)
{
	uint8_t tp1;
	tp1 = (userLimit == MEM_USER_MASTER) ? 1:0;
	uint8_t faceret = FaceGetVeifyState( tp1, pageId, unlockStatus);//识别流程  时效检测关闭
	if( FACE_VERIFY_SUCCESS == faceret )           //验证成功
	{
		my_printf( "open door by face \n" );  
		return 1;
	}
	else if( FACE_VERIFY_MODULE_FAIL == faceret || FACE_VERIFY_TIME_FAIL== faceret     \
		  || FACE_VERIFY_EE_FAIL== faceret || FACE_VERIFY_ADMIN_FAIL== faceret )   //验证失败
	{
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) 
		{
			return 1;
		}
		return -1;
	}
	else if( FACE_VERIFY_NOFACE == faceret )      //未检测到人脸
	{
		my_printf( "find none face!\n" );  
		return 2;
	}
 
	return 0;
}
#endif

/*********************************************************************************************************************
* Function Name :  App_GUI_FaceCheckStop()
* Description   :  结束人脸验证流程
* Para Input    :  none
* Para Output   :  none
* Return        :  none
*********************************************************************************************************************/
#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
static void App_GUI_FaceCheckStop( bool *pfisrtflg )
{
	if( *pfisrtflg == false )
		return;
	if( *pfisrtflg == true )
	{
		*pfisrtflg = false;
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S); 
		#endif
		my_printf( "App_GUI_FaceCheckStop()\n" );   
	}
}
#endif
 
/*********************************************************************************************************************
* Function Name :  App_GUI_CheckNetworkAction()
* Description   :  检测是否执行网络同步功能
* Para Input    :  none
* Para Output   :  none
* Return        :  0= 不执行网络同步  1= 执行网络同步
* author        :  gushengchi 
*********************************************************************************************************************/
uint8_t App_GUI_CheckNetworkAction( void )
{
	if( (SystemSeting.SysWifiMainSw == FUNCTION_ENABLE)&&(true == App_GUI_GetNetworkErrState()) )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //小电池未查
			{
				return 0;	
			}
			else 
				return 1;
		#else
			return 1;
		#endif
	}
	return 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckWeatherUpdateAction()
* Description   :  检测是否执行天气同步功能
* Para Input    :  none
* Para Output   :  none
* Return        :  0= 不执行同步  1= 执行同步
* author & date :  gushengchi  2021/11/22
*********************************************************************************************************************/
uint8_t App_GUI_CheckWeatherUpdateAction( void )
{
	if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //小电池未查
			{
				return 0;	
			}
			else 
				return 1;
		#else
			return 1;
		#endif
	}
	return 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckBellVideoAction()
* Description   :  检测门铃视频触发动作
* Para Input    :  none
* Para Output   :  none
* Return        :  0= 不执行  1= 执行
* author        :  gushengchi 
*********************************************************************************************************************/
static uint8_t App_GUI_CheckBellVideoAction( void )
{
	if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //小电池未查
			{
				return 0;	
			}
			else 
				return 1;
		#else
			return 1;
		#endif
	}
	return 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetNetworkErrState()
* Description   :  获取网络故障状态
* Para Input    :  none
* Para Output   :  none
* Return        :  false= 正常  true= 异常
* author        :  gushengchi 
*********************************************************************************************************************/
bool App_GUI_GetNetworkErrState( void )
{
    bool ret = false;
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON   //二代猫眼是否开启处理
	ret = ( MEDIA_STATE_ERROR == NetWorkStateGet() )? true:false;
	#endif
	
	return ret;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetFaceCheckEnState()
* Description   :  获取人脸验证开关状态
* Para Input    :  none
* Para Output   :  none
* Return        :  false= 关闭  true= 开启
* author        :  gushengchi 
*********************************************************************************************************************/
#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
static bool App_GUI_GetFaceCheckEnState( void )
{
    bool ret = false;
	#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
	ret = ( SystemSeting.FaceCheckEnable == 0x66 )? false:true;
	#endif
	
	return ret;
}
#endif

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckAlarmButtonActionWarm()
* Description   :  检测防撬触发状态
* Para Input    :  none
* Para Output   :  none
* Return        :  false= 正常  true= 触发防撬
* author        :  gushengchi 
*********************************************************************************************************************/
static bool App_GUI_CheckAlarmButtonActionWarm( void )
{
    if( true == App_Export_GetAlrmWarmState() && ( s_GuiData.AlamMeg.AlamButtonNewTrigger == false ) ) 
	{
		return true;
	}
	return false;
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_CheckAlarmButtonRecoveryWarm()
* Description   :  检测防撬解除状态
* Para Input    :  none
* Para Output   :  none
* Return        :  false= 正常  true= 防撬恢复
* author        :  gushengchi 
*********************************************************************************************************************/
static bool App_GUI_CheckAlarmButtonRecoveryWarm( void )
{
	if( s_GuiData.AlamMeg.AlamButtonNewTrigger == true )  //已触发
	{
		if( false == App_Export_GetAlrmWarmState() )
		{
			s_GuiData.AlamMeg.AlamButtonNewTrigger = false;
			s_GuiData.AlamMeg.AlamButtonHoldTimSec = 0;
			return true;
		}
		else if( s_GuiData.AlamMeg.AlamButtonHoldTimSec == 0 )
		{
			App_Export_SetAlrmWarmEn( false );
			s_GuiData.AlamMeg.AlamButtonNewTrigger = false;
			s_GuiData.AlamMeg.AlamButtonHoldTimSec = 0;
			return true;
		}
	}
	return false;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_MainDeskMenu()
* Description   :  系统主菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_MainDeskMenu( void )          //系统桌面菜单
{
	int8_t tp1;
    #if defined(FINGER_FUNCTION_ON) || defined(FINGER_VEIN_FUNCTION_ON)
	int8_t fingerret =0;
    #endif 
    #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
    int8_t faceret =0;
	static uint8_t unlockStatus = 0;
    #endif
	static uint16_t facePageId =0;
	static uint16_t fingerPageId =0;
	
	static bool firstTimFlg;
	static bool touchsetflg;
	static bool facePowerOffFlg;
	static bool faceCheckResult;
	static bool fingerCheckResult;
	static bool autoLockCheckFlg;

	PwdMeg_T pwdmeg = {0};
	DoubleCheckType_U  KeyTppe;
	static CHECK_KEY_TYPE_E checkKeyType;
    
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_MainDeskMenu()\n" );   
		my_printf("System start time =%d\n", SystemTick);
		my_printf("System hold time =%d\n", SystemWorkHoldTim);
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
		if( E_WAKE_BELL_KEY == App_GUI_GetSysWakeupType() )  //门铃唤醒  键盘提前开启
		{
			if( SystemTick <= GUI_TIME_2S )
			{
				HAL_Voice_BellCtrl( true );
				my_printf( "Bell is working!\n" );  
				#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON    //二代猫眼
				if( 1 == App_GUI_CheckBellVideoAction() )
				{
					App_GUI_MenuJump( EM_MENU_BELL_VIDEO );
					return;
				}
				#elif defined XM_CAM_FUNCTION_ON   //单猫眼
				if( 1 == App_GUI_CheckBellVideoAction() )
				{
					CAM_SendCommandStart(CAM_CMD_BELL, 0, 0);
				}
				#endif  
			}   
		}
		
		if( SystemPowerLedStsFlg == true )
		{
			App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_ON ); 
		}
		else 
		{
			App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_ON );  
		}
		App_Touch_FuncEnCtrl( EM_SCAN_OFF );   //按键扫描配置	
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_ON ); 
        #ifdef LOCK_KEY_WHITE_LED_ON
        App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_ON ); //亮锁门键白灯
        #endif
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_ON );//有门铃灯的产品点亮门铃灯
		App_Key_ResetCombinKeyFlow();         //强制复位组合按键检测流程
		checkKeyType = E_CHECK_DEFAULT;
	    #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		if( true == App_GUI_GetFaceCheckEnState() )
		{
			checkKeyType = E_CHECK_FACE;
		}
	    #endif
		OPEN_MODEL_E  openType  = App_GUI_GetOpenModel();
		CLOSE_MODEL_E closeType = App_GUI_GetCloseModel();
		if( (openType == EM_OPEN_BUTTON) || (openType == EM_OPEN_HANDLER)     \
          ||(closeType == EM_CLOSE_BUTTON) || (closeType == EM_CLOSE_HANDLER)  \
		  ||(E_WAKE_MOTOR_LATCH == App_GUI_GetSysWakeupType() )    \
		  ||(E_WAKE_MOTOR_BOLT == App_GUI_GetSysWakeupType() )     \
		  ||(E_WAKE_MOTOR_TRIGGER == App_GUI_GetSysWakeupType() )  \
		  ||(E_WAKE_OPEN_BUTTON == App_GUI_GetSysWakeupType() )    \
		  ||(E_WAKE_CLOSE_BUTTON == App_GUI_GetSysWakeupType() )   \
		  ||(E_WAKE_BELL_KEY == App_GUI_GetSysWakeupType() )       \
		  )  //门内开关门 + 锁体唤醒
		{
			checkKeyType = E_CHECK_DEFAULT;
			my_printf( "open door in room!\n" ); 
		}
		else  //非门内开关门
		{
			if( 1 == App_GUI_GetDoubleCheckSwSts() )  //双重认证开启
			{
				#ifdef FACE_FUNCTION_ON
				HAL_Voice_PlayingVoice( EM_CHECK_FACE_MP3, 0 );   
				#elif defined IRIS_FUNCTION_ON
				HAL_Voice_PlayingVoice( EM_CHECK_IRIS_MP3, 0 );  
				#elif defined FINGER_FUNCTION_ON
				if( 0 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//未检测到手指 
				{
				    HAL_Voice_PlayingVoice( EM_PUT_FINGER_MP3, 0 );   
				}
				else
				{
					#ifdef FINGER_FUNCTION_ON
				    if( SystemPowerOnFlg == true )
					{
						HAL_Voice_PlayingVoice( EM_WELCOME_TIPS_MP3, 0 );  
					}
					#endif
				}
				#elif defined FINGER_VEIN_FUNCTION_ON
				if( 0 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//未检测到手指 
				{
				    HAL_Voice_PlayingVoice( EM_CHECK_VEIN_MP3, 0 );   
				}
				#endif
			}
			else 
			{
				if( 0 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//未检测到手指 
				{
					HAL_Voice_PlayingVoice( EM_WELCOME_TIPS_MP3, 0 );  
				}
				else
				{
					#ifdef FINGER_FUNCTION_ON
				    if( SystemPowerOnFlg == true )
					{
						HAL_Voice_PlayingVoice( EM_WELCOME_TIPS_MP3, 0 );  
					}
					#endif
					checkKeyType = E_CHECK_DEFAULT;
				}
			}
		}
		firstTimFlg = 0;
		touchsetflg = 0;
		facePowerOffFlg = 0;
		FingerCheckFlow = 0;
		faceCheckResult = false;
		fingerCheckResult = false;
	    facePageId =0;
	    fingerPageId =0;
        #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		FaceWorkStsFlg = false;
        #endif
		FingerWorkStsFlg = false;
		if( FingerWorkState == true )
		{
			FingerWorkStsFlg = true;
		}
		autoLockCheckFlg = false;
		TouchLockKeyPushStsFlg = false;
		UploadUnlockDoorMegEnable = 0;
		#ifdef IRIS_FUNCTION_ON
		GuiQuitTimMs = GUI_TIME_16S;
		#else
		GuiQuitTimMs = GUI_TIME_12S;
		#endif
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
	 /*----------------配置触摸按键--------------------*/
		if( SystemWorkHoldTim >= GUI_TIME_1S )  //防止触摸按键唤醒后打断语音播报
		{
			if( touchsetflg == 0 )
			{
				touchsetflg = 1;
				App_Touch_FuncEnCtrl( EM_SCAN_ON );   //按键扫描配置
			}
		}
	 /*----------------组合按键处理流程----------------*/
		BUTTON_TYPE_E ret = App_Key_GetCombinKeyState();  //获取机械按键组合键
		if( EM_OPEN_DOOR_KEY == ret )          //按键开门
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetOpenModel( EM_OPEN_BUTTON );
			SystemEventLogSave( KEY_OPEN_IN_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		#ifdef CLOSE_BUTTON_ON
		else if( EM_CLOSE_DOOR_KEY == ret )    //按键关门
		{
            App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetCloseModel( EM_CLOSE_BUTTON );
			SystemEventLogSave( KEY_CLOSE_IN_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			return;
		}
		#endif
		else if( EM_BACK_FACTORY_KEY == ret )  //恢复出厂设置
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_BACK_FACTORY );
			return;
		}
		else if( EM_ENTER_APP_MODEL_KEY==ret)  //APP设置模式
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_APP_MODEL );
			return;
		}
		else if( EM_ENTER_LOCAL_MODEL_KEY==ret)//进入工程模式
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() )  //非体验模式
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
				App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
				return;
				#endif
			}
		}
		else if( EM_SCAN_NONE_KEY == ret )     //有按键被按下 
		{
            #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
			if( facePowerOffFlg == 0 )
			{
				facePowerOffFlg = 1;
				my_printf( "face break by button!\n" ); 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				if( checkKeyType == E_CHECK_FACE )
				{
					checkKeyType = E_CHECK_DEFAULT;
				}
			}
            #endif
		}
		else if( EM_SCANNING_KEY == ret )      //无按键被按下 
		{
			/*------------------防撬解除-----------------*/	 
			if( true == App_GUI_CheckAlarmButtonRecoveryWarm())
			{
				my_printf("alrm button is recovery!\n");
				HAL_Voice_WorkModeCtrl( false );//语音退出报警模式
			    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );  
				if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
				{
					HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
				}
			}
			/*------------------防撬触发-----------------*/	 
			if( true == App_GUI_CheckAlarmButtonActionWarm() ) 
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_ALARM_WARM );
				return;
			}
			/*------------------检测到有手指-------------*/	 
			if( 1 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )
			{
				if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证开启
				{
					KeyTppe = App_GUI_GetDoubleCheckType(); 
					if( (KeyTppe.bit.FingerCheckEnable == FUNCTION_ENABLE ) 
					  ||(KeyTppe.bit.FingerVeinCheckEn == FUNCTION_ENABLE ))
					{
						#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
                        if( faceCheckResult )
						#endif 
						{
							if( firstTimFlg == 0 )
							{
								firstTimFlg = 1;
								checkKeyType = E_CHECK_FINGER;
								my_printf( "finger is checkde\n" ); 
							}
						}
					}
				}
				else 
				{
					if( firstTimFlg == 0 )
					{
						firstTimFlg = 1;
						#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
						App_GUI_FaceCheckStop( &FaceWorkStsFlg );
						#endif
						checkKeyType = E_CHECK_FINGER;
						my_printf( "finger is checkde\n" ); 
					}
				}
			}
			
			/*----------------蓝牙方式进入APP模式--------*/	
		    if( DRV_GetBleConnect() )    
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_APP_MODEL );
				return;
			}
			/*------------------把手关门-----------------*/	
			if( 1 == App_Key_GetCloseHandleSts() )     
			{
				if( EM_CLOSE_HANDLER != App_GUI_GetCloseModel() )  //避免多次进入
				{
					App_GUI_RelieveTryProtect();  //解除禁试
					App_GUI_SetCloseModel( EM_CLOSE_HANDLER ); 
 				    SystemEventLogSave( BAC_CLOSE_IN_DOOR, 0 );  
				}
			}
			/*------------------把手开门-----------------*/	
			else if( 1 == App_Key_GetOpenHandleSts() )  
			{
				if( EM_OPEN_HANDLER != App_GUI_GetOpenModel() )  //避免多次进入
				{
					App_GUI_RelieveTryProtect();  //解除禁试
					App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
					SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
				}
				if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //布防状态
				{
					#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
					App_GUI_FaceCheckStop( &FaceWorkStsFlg );
					#endif
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					App_GUI_MenuJump( EM_MENU_DEPLAY_WARM );
					return;
				}
			}
			else if( 1 == HAL_Motor_DefendActionCheck(false) )  
			{
				my_printf( "auto lock pus door!\n"); 
				if( EM_OPEN_HANDLER != App_GUI_GetOpenModel() )  //避免多次进入
				{
					App_GUI_RelieveTryProtect();  //解除禁试
					App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
					SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
				}
				if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //布防状态
				{
					#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
					App_GUI_FaceCheckStop( &FaceWorkStsFlg );
					#endif
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					App_GUI_MenuJump( EM_MENU_DEPLAY_WARM );
					return;
				}
			}
			/*------------------非自动锁体自动上锁-------*/	
			if( 1 == App_GUI_GetAutoLockSts() )    
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_SetCloseModel( EM_CLOSE_AUTO );
				SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
				App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
				return;
			}
			/*------------------假锁报警-----------------*/	
			if( true == HAL_Motor_FalseLockWarmCheck() )
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_FALSE_LOCK_WARM );
				return;
			}
			/*------------------门未锁报警---------------*/	
			if( true == HAL_Motor_ForgetLockWarmCheck() )
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_FORGET_LOCK_WARM );
				return;
			}
			/*------------------把手试玩报警-------------*/	
			if( true == HAL_Motor_HandleTryForbitWarmCheck() )
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
			    App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			    App_GUI_MenuJump( EM_MENU_FALSE_LOCK_WARM );
				return;
			}

			/*------------------自动锁体上锁-------------*/	
			if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg )) 
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_SetCloseModel( EM_CLOSE_AUTO );
				SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
				App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			    return;
			}
			/*------------------触发逗留报警-------------*/	
			if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
			{
				s_GuiData.StayDetectFlg = 0;
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_STAY_WARM );
				return;
			}
		}
		
	 /*----------------人脸+指纹验证处理流程-----------------*/
		switch( checkKeyType )
		{
            #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
			case E_CHECK_FACE:      //人脸/虹膜验证
			{
				FaceWorkStsFlg = true;
				faceret = App_GUI_FaceCheck( MEM_USER_ALL, &facePageId, &unlockStatus);
				if( faceret == 1 )        //验证成功
				{
					FaceWorkStsFlg = false;
					faceCheckResult = true;
					checkKeyType = E_CHECK_DEFAULT;
					if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证开启
					{
						KeyTppe = App_GUI_GetDoubleCheckType(); 
						if( KeyTppe.bit.FingerCheckEnable == FUNCTION_ENABLE ) 
						{
							HAL_Voice_PlayingVoice( EM_PUT_FINGER_MP3, 0 );   
						}
						else if( KeyTppe.bit.FingerVeinCheckEn == FUNCTION_ENABLE )
						{
							HAL_Voice_PlayingVoice( EM_CHECK_VEIN_MP3, 0 );   
						}
						else if( KeyTppe.bit.PwdCheckEnable == FUNCTION_ENABLE )
						{
							HAL_Voice_PlayingVoice( EM_INPUT_PWD_MP3, 0 );  
						}
					}
					else 
					{
						#if defined  FACE_FUNCTION_ON
						App_GUI_SetOpenModel( EM_OPEN_FACE );
						
						bool retbool = false;
						if(unlockStatus == 0xCC)//解锁过程中睁闭眼状态
						{
							retbool = App_GUI_GetWifiWarmingSwSts();
					    }
						else 
						{
							retbool = App_GUI_GetWifiUploadSwSts();
						}
						if( true == retbool )
						{ 
							WifiLockMeg.UnlockMode = FACE;
							WifiLockMeg.PageID.way1 = 0;
							WifiLockMeg.PageID.id1 = 0;
							WifiLockMeg.PageID.way2 = 0;
							WifiLockMeg.PageID.id2 = facePageId;
							if(unlockStatus == 0xCC)//解锁过程中睁闭眼状态
							{
								my_printf("Face WifiLockMeg.Attribute = SOS;\n");
								WifiLockMeg.Attribute = SOS;
							}
							else
							{
								WifiLockMeg.Attribute = NONE;
							}
//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
						    UploadUnlockDoorMegEnable = 1;
						}
						SystemEventLogSave( FACE_OPEN, facePageId );
						#elif defined IRIS_FUNCTION_ON
						App_GUI_SetOpenModel( EM_OPEN_IRIS );
						if( true == App_GUI_GetWifiUploadSwSts() )
						{ 
							WifiLockMeg.UnlockMode = IRIS;
							WifiLockMeg.PageID.way1 = 0;
							WifiLockMeg.PageID.id1 = 0;
							WifiLockMeg.PageID.way2 = 0;
							WifiLockMeg.PageID.id2 = facePageId;
							WifiLockMeg.Attribute = NONE;
//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
							UploadUnlockDoorMegEnable = 1;
						}
						SystemEventLogSave( IRIS_OPEN, facePageId );
						#endif

						App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
						return;
					}
				}
				else if( faceret == -1 )  //验证失败
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FACE_CHECK_ERR );
					return;
				}
				else if( faceret == 2 )   //未检测到人脸
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					return;
				}
			}
			break;
			#endif
			case E_CHECK_FINGER:   //指纹/指静脉验证
			{
				FingerWorkStsFlg = true;
				fingerret = App_GUI_FingerCheck( &fingerPageId, MEM_USER_ALL );
				if( fingerret == 1 )        //验证成功
				{
					my_printf("finger check success =%d\n", SystemTick); 
					FingerWorkStsFlg = false;
					fingerCheckResult = true;
					checkKeyType = E_CHECK_DEFAULT;
					if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证开启
					{
						KeyTppe = App_GUI_GetDoubleCheckType(); 
						if( KeyTppe.data == LOCK_MODE_FINGER ) //开锁方式设置为仅指纹开锁模式下
						{
							#if defined  FINGER_FUNCTION_ON
							App_GUI_SetOpenModel( EM_OPEN_FIENGER );
							if( true == App_GUI_GetWifiUploadSwSts() )
							{ 
								WifiLockMeg.UnlockMode = FINGER;
								WifiLockMeg.PageID.way1 = 0;
								WifiLockMeg.PageID.id1 = 0;
								WifiLockMeg.PageID.way2 = 0;
								WifiLockMeg.PageID.id2 = fingerPageId;
								WifiLockMeg.Attribute = NONE;
	//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
								UploadUnlockDoorMegEnable = 1;
							}
							SystemEventLogSave( FINGER_OPEN, fingerPageId );
							#elif defined FINGER_VEIN_FUNCTION_ON 
							App_GUI_SetOpenModel( EM_OPEN_VEIN );
							if( true == App_GUI_GetWifiUploadSwSts() )
							{ 
								WifiLockMeg.UnlockMode = VEIN;
								WifiLockMeg.PageID.way1 = 0;
								WifiLockMeg.PageID.id1 = 0;
								WifiLockMeg.PageID.way2 = 0;
								WifiLockMeg.PageID.id2 = fingerPageId;
								WifiLockMeg.Attribute = NONE;
	//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
								UploadUnlockDoorMegEnable = 1;
							}
							SystemEventLogSave( VEIN_OPEN, fingerPageId );
							#endif
							App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
							return;
						}
						
						
						
						#if defined  FACE_FUNCTION_ON
						App_GUI_SetOpenModel( EM_OPEN_FIENGER );
						bool retbool1 = false;
						if(unlockStatus == 0xCC)//解锁过程中睁闭眼状态
						{
							retbool1 = App_GUI_GetWifiWarmingSwSts();
					    }
						else 
						{
							retbool1 = App_GUI_GetWifiUploadSwSts();
						}
						if( true == retbool1 )
						{ 
							#ifdef FINGER_VEIN_FUNCTION_ON 
                            WifiLockMeg.UnlockMode = FACE_FIGURE;
							#else
                            WifiLockMeg.UnlockMode = FACE_FIGURE;
							#endif
							if(unlockStatus == 0xCC)//解锁过程中睁闭眼状态
							{
								my_printf("Face WifiLockMeg.Attribute = SOS;\n");
								WifiLockMeg.PageID.way1 = SOS;
							}
							else
							{
								WifiLockMeg.PageID.way1 = NONE;
							}
							WifiLockMeg.PageID.id1  = facePageId;
							WifiLockMeg.PageID.way2 = NONE;
							WifiLockMeg.PageID.id2  = fingerPageId;
							WifiLockMeg.Attribute = NONE;
//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
							UploadUnlockDoorMegEnable = 1;
						}
						#ifdef FINGER_FUNCTION_ON 
						SystemEventLogSave( FINGER_OPEN, fingerPageId );
						#elif defined FINGER_VEIN_FUNCTION_ON
						SystemEventLogSave( VEIN_OPEN, fingerPageId );
						#endif
						App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
						return;
						#elif defined IRIS_FUNCTION_ON 
						App_GUI_SetOpenModel( EM_OPEN_VEIN );
						if( true == App_GUI_GetWifiUploadSwSts() )
						{ 
							#ifdef FINGER_VEIN_FUNCTION_ON 
                            WifiLockMeg.UnlockMode = IRIS_FINGER;
							#else
                            WifiLockMeg.UnlockMode = IRIS_FINGER;
							#endif
							WifiLockMeg.PageID.way1 = NONE;
							WifiLockMeg.PageID.id1  = facePageId;
							WifiLockMeg.PageID.way2 = NONE;
							WifiLockMeg.PageID.id2  = fingerPageId;
							WifiLockMeg.Attribute = NONE;
//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
							UploadUnlockDoorMegEnable = 1;
						}
						#ifdef FINGER_FUNCTION_ON 
						SystemEventLogSave( FINGER_OPEN, fingerPageId );
						#elif defined FINGER_VEIN_FUNCTION_ON
						SystemEventLogSave(VEIN_OPEN, fingerPageId );
						#endif
						App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
						return;
						#else 
						KeyTppe = App_GUI_GetDoubleCheckType(); 
						if( KeyTppe.bit.PwdCheckEnable == FUNCTION_ENABLE ) 
						{
							HAL_Voice_PlayingVoice( EM_INPUT_PWD_MP3, 0 );   	
						}
						#endif 
					}
					else 
					{
						#if defined  FINGER_FUNCTION_ON
						App_GUI_SetOpenModel( EM_OPEN_FIENGER );
						if( true == App_GUI_GetWifiUploadSwSts() )
						{ 
							WifiLockMeg.UnlockMode = FINGER;
							WifiLockMeg.PageID.way1 = 0;
							WifiLockMeg.PageID.id1 = 0;
							WifiLockMeg.PageID.way2 = 0;
							WifiLockMeg.PageID.id2 = fingerPageId;
							WifiLockMeg.Attribute = NONE;
//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
							UploadUnlockDoorMegEnable = 1;
						}
						SystemEventLogSave( FINGER_OPEN, fingerPageId );
						#elif defined FINGER_VEIN_FUNCTION_ON 
						App_GUI_SetOpenModel( EM_OPEN_VEIN );
						if( true == App_GUI_GetWifiUploadSwSts() )
						{ 
							WifiLockMeg.UnlockMode = VEIN;
							WifiLockMeg.PageID.way1 = 0;
							WifiLockMeg.PageID.id1 = 0;
							WifiLockMeg.PageID.way2 = 0;
							WifiLockMeg.PageID.id2 = fingerPageId;
							WifiLockMeg.Attribute = NONE;
//							App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
							UploadUnlockDoorMegEnable = 1;
						}
						SystemEventLogSave( VEIN_OPEN, fingerPageId );
						#endif
						App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
						return;
					}
				}
				else if( fingerret == -1 )  //验证失败
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FINGER_CHECK_ERR );
					return;
				}
                else if( fingerret == -2 )  //超时或协议错误
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
					return;
				}
			}
			break;
			default: FingerCheckFlow = 0;  break;
		}
   
		/*----------------IC卡处理流程----------------*/
		#ifdef IC_CARD_FUNCTION_ON
		if( FUNCTION_DISABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证关闭
		{
			uint16_t icid = 0;
			tp1 = CpuCardGetVeifyState(&icid);
			if(1 == tp1)
			{
				App_GUI_SetOpenModel( EM_OPEN_CARD );
				if( true == App_GUI_GetWifiUploadSwSts() )
				{ 
					WifiLockMeg.UnlockMode = IC;
					WifiLockMeg.PageID.way1 = 0;
					WifiLockMeg.PageID.id1 = 0;
					WifiLockMeg.PageID.way2 = 0;
					WifiLockMeg.PageID.id2 = icid;
					WifiLockMeg.Attribute = NONE;
					UploadUnlockDoorMegEnable = 1;
				}
				App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
				return;
			}
			else if(2 == tp1)
			{
				App_GUI_MenuJump( EM_MENU_CARD_CHECK_ERR );
				return;
			}
		}
		#endif
	 /*----------------密码+键盘处理流程----------------*/
		if( App_Touch_GetCurrentKeyIndex() >= 2 )  //2个按键关闭人脸识别
		{
			if( FUNCTION_DISABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证关闭
			{
				if( facePowerOffFlg == 0 )
				{
					facePowerOffFlg = 1;
				    #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
					my_printf( "face break by touch!\n" ); 
				    App_GUI_FaceCheckStop( &FaceWorkStsFlg );
					if( checkKeyType == E_CHECK_FACE )
					{
						checkKeyType = E_CHECK_DEFAULT;
					}
					#endif
				}
			}
		}
		bool pwdCheckEn = true;
		bool OtherTypePwdCheckEn = true;
		if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证开启
		{
			KeyTppe = App_GUI_GetDoubleCheckType(); 
			if( KeyTppe.bit.PwdCheckEnable == FUNCTION_ENABLE ) //密码验证使能
			{
				if( (KeyTppe.bit.FaceCheckEnable == FUNCTION_ENABLE)
				  ||(KeyTppe.bit.IrisOpenCheckEn == FUNCTION_ENABLE) ) 
				{
					pwdCheckEn = faceCheckResult; 
				}
				else if( (KeyTppe.bit.FingerCheckEnable  == FUNCTION_ENABLE)
					   ||( KeyTppe.bit.FingerVeinCheckEn == FUNCTION_ENABLE) )
				{
					pwdCheckEn = fingerCheckResult; 
					OtherTypePwdCheckEn = fingerCheckResult; 
				}
			}
			else if( KeyTppe.bit.PwdCheckEnable == FUNCTION_DISABLE )
			{
			    pwdCheckEn = false;	
			}
			
			if( (KeyTppe.bit.FaceCheckEnable == FUNCTION_ENABLE)
			  ||(KeyTppe.bit.IrisOpenCheckEn == FUNCTION_ENABLE) ) 
			{
				OtherTypePwdCheckEn = faceCheckResult; 
			}
			else if( (KeyTppe.bit.FingerCheckEnable  == FUNCTION_ENABLE)
				   ||( KeyTppe.bit.FingerVeinCheckEn == FUNCTION_ENABLE) )
			{
				OtherTypePwdCheckEn = fingerCheckResult; 
			}
		}
		tp1 = App_GUI_PwdCheckAndTouchHandler( pwdCheckEn, OtherTypePwdCheckEn, &pwdmeg );  //密码验证+按键处理
		if( (1 == tp1) || (8 == tp1) || (9 == tp1) )       //验证成功 (密码/临时密码/报警密码)
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			bool retbool2 = false;
			#if defined  FACE_FUNCTION_ON
			if(unlockStatus == 0xCC)//解锁过程中睁闭眼状态
			{
				retbool2 = App_GUI_GetWifiWarmingSwSts();
			}
			else 
			{
				retbool2 = App_GUI_GetWifiUploadSwSts();
			}
			#else 
			retbool2 = App_GUI_GetWifiUploadSwSts();
			#endif
			if( true == retbool2 )   //数据推送
			{ 
				if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //双重认证开启
				{
					if( faceCheckResult == true )
					{ 
						#if defined  FACE_FUNCTION_ON
						WifiLockMeg.UnlockMode = FACE_PWD;
						if(unlockStatus == 0xCC)//解锁过程中睁闭眼状态
						{
							my_printf("Face WifiLockMeg.Attribute = SOS;\n");
							WifiLockMeg.PageID.way1 = SOS;
						}
						else
						{
							WifiLockMeg.PageID.way1 = NONE;
						}
						#elif defined IRIS_FUNCTION_ON
						WifiLockMeg.UnlockMode = IRIS_PWD;
						WifiLockMeg.PageID.way1 = NONE;
						#endif
						WifiLockMeg.PageID.id1  = facePageId;
					}
					else if( fingerCheckResult == true )
					{
						#if defined  FINGER_FUNCTION_ON
						WifiLockMeg.UnlockMode = FINGER_PWD;
						#elif defined FINGER_VEIN_FUNCTION_ON
                        WifiLockMeg.UnlockMode = VEIN_PWD;
						#endif
                        WifiLockMeg.PageID.way1 = NONE;
						WifiLockMeg.PageID.id1  = fingerPageId;	
					}
                    
					if( 1 == tp1 )      //密码
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_USER_ID;
					}
					else if( 8 == tp1 ) //临时密码
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_TEMP_ID;
					}
					else if( 9 == tp1 ) //报警密码
					{
						WifiLockMeg.PageID.way2 = SOS;
						WifiLockMeg.PageID.id2  = PWD_SOS_ID;
					}
                    
					WifiLockMeg.Attribute = NONE;
//				    App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
					UploadUnlockDoorMegEnable = 1;
				}
				else
				{
					WifiLockMeg.UnlockMode = PWD;
					WifiLockMeg.PageID.way1 = 0;
					WifiLockMeg.PageID.id1 = 0;
					if( 1 == tp1 )      //密码
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_USER_ID;
						WifiLockMeg.Attribute = NONE;
					}
					else if( 8 == tp1 ) //临时密码
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_TEMP_ID;
						WifiLockMeg.Attribute = NONE;
					}
					else if( 9 == tp1 ) //报警密码
					{
						WifiLockMeg.PageID.way2 = SOS;
						WifiLockMeg.PageID.id2  = PWD_SOS_ID;
						WifiLockMeg.Attribute = SOS;
					}
//					App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
					UploadUnlockDoorMegEnable = 1;
				}
			}
			
			if( 1 == tp1 )      //密码
			{
				App_GUI_SetOpenModel( EM_OPEN_PWD );
				SystemEventLogSave( PASSWORD_OPEN, PWD_USER_ID );
			}
			else if( 8 == tp1 ) //临时密码
			{
				App_GUI_SetOpenModel( EM_OPEN_TMP_PWD );
				SystemEventLogSave( TEMP_PASSWORD_OPEN, PWD_TEMP_ID ); 
			}
			else if( 9 == tp1 ) //报警密码
			{
				App_GUI_SetOpenModel( EM_OPEN_SOS_PWD );
				SystemEventLogSave( SOS_PASSWORD_OPEN, PWD_SOS_ID );  
			}
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		else if( -1 == tp1 ) //验证失败 
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_PWD_CHECK_ERR );
			return;
		}
		
#if defined (XM_CAM_FUNCTION_ON) || defined (SMART_SCREEN_ON)
		else if( 2 == tp1 )  //故障查询  
		{
			if( false == HAL_Voice_WorkModeGet() )  //非报警模式
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				#ifdef XM_CAM_FUNCTION_ON
				App_GUI_MenuJump( EM_MENU_ERROR_CHECK );
				#elif defined SMART_SCREEN_ON
				App_GUI_MenuJump( EM_MENU_SMART_SCREEN_SHOW );
				#endif
				return;
			}
		}
#endif
		else if( 3 == tp1 )  //老化测试  
		{
			if( false == HAL_Voice_WorkModeGet() )  //非报警模式
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EN_MENU_AGING_TEST );
				return;
			}
		}
		else if( 4 == tp1 )  //验证管理员   
		{
			if( false == HAL_Voice_WorkModeGet() )  //非报警模式
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_CHECK_ADMIN );
				return;
			}
		}
		else if( 5 == tp1 )  //触摸按键上锁   
		{
			#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
			App_GUI_FaceCheckStop( &FaceWorkStsFlg );
			#endif
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetCloseModel( EM_CLOSE_TOUCH );
			SystemEventLogSave( CLOSE_OUT_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			return;
		}
		else if( 6 == tp1 )  //非人脸款首次使用   
		{
			if( false == HAL_Voice_WorkModeGet() )  //非报警模式
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
				return;
			}
		}
		else if( 7 == tp1 )  //门铃   
		{
			if( facePowerOffFlg == 0 )
			{
			    facePowerOffFlg = 1;
				my_printf( "face break by bell!\n" ); 
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				if( checkKeyType == E_CHECK_FACE )
				{
					checkKeyType = E_CHECK_DEFAULT;
				}
				#endif
			}
			HAL_Voice_BellCtrl( true );
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON    //二代猫眼
			if( 1 == App_GUI_CheckBellVideoAction() )
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_BELL_VIDEO );
				return;
			}
			#elif defined XM_CAM_FUNCTION_ON   //单猫眼
				if( 1 == App_GUI_CheckBellVideoAction() )
				{
					CAM_SendCommandStart(CAM_CMD_BELL, 0, 0);
				}
			#endif 
		}
	 /*------------------THE END------------------*/	
	}
    else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //确保语音播报完成
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		App_GUI_FaceCheckStop( &FaceWorkStsFlg );
		#endif
		App_GUI_FingerCheckStop( &FingerWorkStsFlg );
		//SystemEventLogSave( NOTHING_CASE, 0 );  //事件记录
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SystemManage()
* Description   :  系统管理员菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SystemManageMenu( void )      //系统管理菜单
{
	uint8_t facesetkey   = 0xF0;
	uint8_t fingersetkey = 0xF0;
	uint8_t pwdsetkey    = 0xF0;
	uint8_t syscfgkey    = 0xF0;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SystemManage()\n" );   
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );       //按键扫描开启
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        App_GUI_RelieveTryProtect();  //解除禁试
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
		HAL_Voice_PlayingVoice( EM_ADMIN_MANAGE_MENU_MP3, 0 );	
	    MenuItem.CurMenuNum = EM_MENU_STEP_1;
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();

		
		if( TOUCH_KEY_NO_1 == tp1 )        //增加用户
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;	
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //修改密码
		{
			App_GUI_MenuJump( EM_MENU_SET_PWD_MENU );
			return;	
		}
		else if( TOUCH_KEY_NO_3 == tp1 )    //删除用户
		{
			App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;	
		}
		else if( TOUCH_KEY_NO_4 == tp1 )    //系统时间设置
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_TIME_SETTING );
			return;		
		}
		else if( TOUCH_KEY_NO_5 == tp1 ) //开锁方式设置
		{
		    App_GUI_MenuJump( EM_MENU_UNLOCK_WAY_SETTING );
			return;		
		}
		else if( TOUCH_KEY_NO_6 == tp1 ) //语音设置
		{
		    App_GUI_MenuJump( EM_MENU_VOICE_SETTING );
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;		
		}
	}
 	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;	
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckAdminMenu()
* Description   :  验证管理员权限
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckAdminMenu( void )        //验证管理员权限
{
	//uint8_t  inputBuf[ KEY_BUF_SIZE+1 ] = {0};
    //PwdMeg_T pwdmeg = {0}; 
#if (defined FINGER_FUNCTION_ON || defined FINGER_VEIN_FUNCTION_ON) && !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
	int8_t fingerret =0;
#endif
    #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
    int8_t faceret =0;
	uint8_t unlockStatus = 0;
    #endif
	uint16_t pageid =0;
	#if !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
	static bool firstTimFlg;
    #endif
	static bool facePowerOffFlg;
    static CHECK_KEY_TYPE_E checkKeyType;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckAdmin()\n" );   
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 	
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Key_ResetCombinKeyFlow();   //调用之前强制复位组合按键检测流程  以防万一
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
		
		checkKeyType = E_CHECK_DEFAULT;
	    #ifdef FACE_FUNCTION_ON
		checkKeyType = E_CHECK_FACE;
	    HAL_Voice_PlayingVoice( EM_CHECK_ADMIN_FACE_TIPS_MP3, 0 );
		#elif defined IRIS_FUNCTION_ON
		checkKeyType = E_CHECK_FACE;
	    HAL_Voice_PlayingVoice( EM_CHECK_ADMIN_IRIS_TIPS_MP3, 0 );
		#elif defined FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_CHECK_ADMIN_FINGER_TIPS_MP3, 0 );
		#elif defined FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_CHECK_ADMIN_VEIN_TIPS_MP3, 0 );
	    #endif
		
        #if !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
		firstTimFlg = 0;
        #endif
		facePowerOffFlg = 0;
		FingerCheckFlow =0;
        #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
		FaceWorkStsFlg = false;
        #endif
		FingerWorkStsFlg = false;
		FingerWorkState = false;
		GuiQuitTimMs = GUI_TIME_15S;
	}
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //按键扫描获取密码
	{
	 /*----------------组合按键处理流程-----------------*/
		BUTTON_TYPE_E ret = App_Key_GetCombinKeyState();  //获取机械按键组合键
		if( EM_OPEN_DOOR_KEY == ret )   //按键开门
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetOpenModel( EM_OPEN_BUTTON );
			SystemEventLogSave( KEY_OPEN_IN_DOOR, 0 );   
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
	    #ifdef CLOSE_BUTTON_ON
		else if( EM_CLOSE_DOOR_KEY == ret )       //按键关门
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetCloseModel( EM_CLOSE_BUTTON );
			SystemEventLogSave( KEY_CLOSE_IN_DOOR, 0 );   
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			return;
		}
		#endif
		else if( EM_BACK_FACTORY_KEY == ret )//恢复出厂设置
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_BACK_FACTORY );
			return;
		}
		else if( EM_ENTER_APP_MODEL_KEY == ret )//APP设置模式
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_APP_MODEL );
			return;
		}
		else if( EM_SCAN_NONE_KEY == ret )   //有按键被按下 
		{
			if( facePowerOffFlg == 0 )
			{
				facePowerOffFlg = 1;
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				if( checkKeyType == E_CHECK_FACE )
				{
					checkKeyType = E_CHECK_DEFAULT;
				}
				#endif
			}
		}
		else if( EM_SCANNING_KEY == ret )    //无按键被按下 
		{
//			if( 1 == App_Export_GetPinState( E_PIN_ALARM_IRQ ) )     //防撬触发
//			{
//				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
//				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
//				#endif
//			    App_GUI_FingerCheckStop( &FingerWorkStsFlg );
//				App_GUI_MenuJump( EM_MENU_ALARM_WARM );
//				return;
//			}
			#if !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
			if( 1 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//检测到有手指
			{
				if( firstTimFlg == 0 )
				{
					firstTimFlg = 1;
					#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
					App_GUI_FaceCheckStop( &FaceWorkStsFlg );
					#endif
					checkKeyType = E_CHECK_FINGER;
					my_printf( "finger is checkde\n" ); 
				}
			}
			#endif
 
		    if( 1 == App_Key_GetCloseHandleSts() )     //把手关门
			{
				if( EM_CLOSE_HANDLER != App_GUI_GetCloseModel() )  //避免多次进入
				{
					App_GUI_RelieveTryProtect();  //解除禁试
					App_GUI_SetCloseModel( EM_CLOSE_HANDLER );   
				}
			}
			else if( 1 == App_Key_GetOpenHandleSts() ) //把手开门
			{
				if( EM_OPEN_HANDLER != App_GUI_GetOpenModel() )  //避免多次进入
				{
					App_GUI_RelieveTryProtect();  //解除禁试
					App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
					SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  //事件记录
				}
				if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //布防状态
				{
					#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
					App_GUI_FaceCheckStop( &FaceWorkStsFlg );
					#endif
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					App_GUI_MenuJump( EM_MENU_DEPLAY_WARM );
					return;
				}	
			}
		}
		
	 /*----------------人脸+指纹验证处理流程-----------------*/
		switch( checkKeyType )
		{
            #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
			case E_CHECK_FACE:   //人脸验证
			{
				FaceWorkStsFlg = true;
				faceret = App_GUI_FaceCheck( MEM_USER_MASTER, &pageid, &unlockStatus);
				if( faceret == 1 )        //验证成功
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					HAL_Voice_PlayingVoice( EM_CHECK_SUCCESS_MP3, GUI_TIME_1500MS );	
					#if defined FACE_FUNCTION_ON
                    SystemEventLogSave( FACE_ADMIN_CHECK, pageid );  
                    #elif defined IRIS_FUNCTION_ON
                    SystemEventLogSave( IRIS_ADMIN_CHECK, pageid );  
                    #endif
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					return;
				}
				else if( faceret == -1 )  //验证失败
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FACE_CHECK_ERR );
					return;
				}
				else if( faceret == 2 )   //未检测到人脸
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					MenuItem.CurMenuNum = EM_MENU_STEP_3;
					return;
				}
			}
			break;
            #endif
			#if (defined FINGER_FUNCTION_ON || defined FINGER_VEIN_FUNCTION_ON) && !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
			case E_CHECK_FINGER: //指纹验证
			{
				FingerWorkStsFlg = true;
				fingerret = App_GUI_FingerCheck( &pageid, MEM_USER_MASTER );
				if( fingerret == 1 )        //验证成功
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					HAL_Voice_PlayingVoice( EM_CHECK_SUCCESS_MP3, GUI_TIME_1500MS );	
                    #ifdef FINGER_VEIN_FUNCTION_ON
                    SystemEventLogSave( VEIN_ADMIN_CHECK, pageid );  
                    #else
                    SystemEventLogSave( FINGER_ADMIN_CHECK, pageid );  
                    #endif
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					return;
				}
				else if( fingerret == -1 )  //验证失败
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FINGER_CHECK_ERR );
					return;
				}
                else if( fingerret == -2 )  //超时或协议错误
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
					return;
				}
			}
			break;
			#endif
			default: FingerCheckFlow = 0;  break;
		}
	 
	 /*----------------密码+键盘处理流程----------------*/
	 #if 0
		if( App_Touch_GetCurrentKeyIndex() >= 2 )  //2个按键关闭人脸识别
		{
			if( facePowerOffFlg == 0 )
			{
				facePowerOffFlg = 1;
				my_printf( "face break by touch!\n" ); 
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				if( checkKeyType == E_CHECK_FACE )
				{
					checkKeyType = E_CHECK_DEFAULT;
				}
				#endif
			}
		}
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tm1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen );   
			if( tm1 >= 6 )  //密码 	
			{
				my_printf("input pwd is:");  
				for(uint8_t i=0; i<buflen; i++)
				{
					my_printf(" %d", inputBuf[i]);  
				}
				my_printf("\r\n");  
				PUBLIC_ChangeDecToString( inputBuf, inputBuf, buflen );	
				inputBuf[ buflen ] = '\0';
				int8_t ret = App_PWD_VerifyUserPwd( PWD_LIMIT_ADMIN, &pwdmeg, (char *)inputBuf );
				if( ret == 1 )        //验证成功
				{
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					HAL_Voice_PlayingVoice( EM_CHECK_SUCCESS_MP3, GUI_TIME_1500MS );	
					SystemEventLogSave( PWD_ADMIN_CHECK, pwdmeg.UserId );  
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					return;
				}
				else if( ret == -1 )  //验证失败
				{
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					App_GUI_MenuJump( EM_MENU_PWD_CHECK_ERR );
					return;
				}
			}
		}
		#endif
	 /*---------------------THE END---------------------*/
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //等待语音播报完毕
	{
		if( 0 == HAL_Voice_GetBusyState() )
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}	
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //等待语音播报完毕
	{
		if( 0 == HAL_Voice_GetBusyState() )
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}	
	}
	
	if( 0 == GuiQuitTimMs )
	{
		my_printf( "gui option timeout!\n" );
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
		App_GUI_FaceCheckStop( &FaceWorkStsFlg );
		#endif
		App_GUI_FingerCheckStop( &FingerWorkStsFlg );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_OpenDoorMenu()
* Description   :  开门流程
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_OpenDoorMenu( void )		  //执行开门流程
{
	static uint8_t flowStep;
	static uint8_t defendWarmFlow;
	static uint8_t registTipsFlow;
	static uint8_t openButNotPushFlg;
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
	static uint8_t DeploymentFlag;
#endif
	static bool autoLockCheckFlg;
    static bool AutoLockActionCheck=false;
	#ifdef SMART_SCREEN_ON
	static uint8_t screenDispay;
    #endif

    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_OpenDoorMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
		App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
		if( EM_OPEN_BUTTON == App_GUI_GetOpenModel() )
		{
			App_LED_OutputCtrl( EM_LED_LOCK_G, EM_LED_OFF ); 
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		}
		else
		{
			App_LED_OutputCtrl( EM_LED_LOCK_G, EM_LED_ON ); 
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON ); 	
			if(SystemSeting.SysKeyDef == FUNCTION_ENABLE )
			{
				SystemSeting.SysKeyDef = FUNCTION_DISABLE;
				(void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysKeyDef, sizeof SystemSeting.SysKeyDef );
			}
		}
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		
		if( E_WAKE_ALARM_BREAK == App_GUI_GetSysWakeupType() )    //中断防撬唤醒
		{
			HAL_Voice_WorkModeCtrl( false );   //语音退出报警模式
			App_Export_SetAlrmWarmEn( false ); //防撬功能失能
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
			if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
			{
				HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
			}	
		}
		App_GUI_RelieveTryProtect();           //解除禁试保护
		HAL_ADC_UpBatValLockCtrl( true );      //上方电池电压锁住
		HAL_ADC_UnderBatValLockCtrl( true );   //下方电池电压锁住

		#ifndef SMART_SCREEN_ON
		/*-----------开门记录推送---------*/
#ifdef XM_CAM_SCREEN_FUNCTION_ON
        switch(WifiLockMeg.UnlockMode)
        {
            case FACE:
                CAM_OpenDoorGiveNotice(FACE_OPEN, WifiLockMeg.PageID.id1, NOTHING_CASE, 0);
                break;
            case FINGER:
                CAM_OpenDoorGiveNotice(FINGER_OPEN, WifiLockMeg.PageID.id1, NOTHING_CASE, 0);
                break;
            case PWD:
                CAM_OpenDoorGiveNotice(PASSWORD_OPEN, WifiLockMeg.PageID.id1, NOTHING_CASE, 0);
                break;
            case FACE_FIGURE:
                CAM_OpenDoorGiveNotice(FACE_OPEN, WifiLockMeg.PageID.id1, FINGER_OPEN, WifiLockMeg.PageID.id2);
                break;
            case FACE_PWD:
                CAM_OpenDoorGiveNotice(FACE_OPEN, WifiLockMeg.PageID.id1, PASSWORD_OPEN, WifiLockMeg.PageID.id2);
                break;
            case FINGER_PWD:
                CAM_OpenDoorGiveNotice(FINGER_OPEN, WifiLockMeg.PageID.id1, PASSWORD_OPEN, WifiLockMeg.PageID.id2);
                break;
            default:
                break;
        }
#endif
		if( 1 == UploadUnlockDoorMegEnable )
		{
			UploadUnlockDoorMegEnable =0;
			App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
		}
		/*-----------THE END--------------*/
		#endif

		flowStep = 0;
		defendWarmFlow = 0;
		registTipsFlow = 0;
		openButNotPushFlg = 0;
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		DeploymentFlag = 0;
#endif
		autoLockCheckFlg = false;
		AutoLockActionCheck=false;
#ifdef SMART_SCREEN_ON
		screenDispay = 0;
#endif
		App_GUI_StopStayDetectTim();//停止当次逗留计时
		GuiQuitTimMs = GUI_TIME_60S;
	}

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		HAL_Voice_PlayingVoice( EM_OPEN_DOOR_OK_MP3, GUI_TIME_1500MS );	
		MenuItem.CurMenuNum = EM_MENU_STEP_2;
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册
		{
			registTipsFlow = 1;
			return;
		}
	}
 
	if( flowStep == 0 )
	{
		/*----------未注册提醒--------------*/
		if( registTipsFlow == 1 )
		{
			if( 0 == HAL_Voice_GetBusyState() )  
			{		
				#if defined FINGER_FUNCTION_ON
				HAL_Voice_PlayingVoice( EM_FIRST_USE_FINGER_TIPS_MP3, GUI_TIME_4S );		
				#elif defined FINGER_VEIN_FUNCTION_ON
				HAL_Voice_PlayingVoice( EM_FIRST_USE_VEIN_TIPS_MP3, GUI_TIME_4S );		
				#endif
				flowStep = 1;
			}
		}
	}

    if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( registTipsFlow == 1 )
		{
			if( flowStep )
			{
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
			}
			return;			
		}
		else if( 1 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}

	
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //未注册
	{
	    if( 1 == HAL_Voice_GetBusyState() )   
		{
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_OpenDoorMenu()
* Description   :  关门流程
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CloseDoorMenu( void )		  //执行关门流程
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CloseDoorMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
        CLOSE_MODEL_E closeType = App_GUI_GetCloseModel();
		if( (closeType == EM_CLOSE_BUTTON) || (closeType == EM_CLOSE_HANDLER) )
		{
			App_GUI_RelieveTryProtect();       //解除禁试保护
		}
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		
		if( closeType == EM_CLOSE_BUTTON ) 
		{
			if( E_WAKE_ALARM_BREAK == App_GUI_GetSysWakeupType() )    //中断防撬唤醒
			{
				HAL_Voice_WorkModeCtrl( false );   //语音退出报警模式
				App_Export_SetAlrmWarmEn( false ); //防撬功能失能
				HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
				if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
				{
					HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
				}	
			}
		}
		HAL_ADC_UpBatValLockCtrl( true );      //上方电池电压锁住
		HAL_ADC_UnderBatValLockCtrl( true );   //下方电池电压锁住
		GuiQuitTimMs = GUI_TIME_12S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		int8_t tp1 = 0;
		if( (EM_CLOSE_BUTTON == App_GUI_GetCloseModel())     \
		 || (EM_CLOSE_TOUCH == App_GUI_GetCloseModel())      \
		  )
		{
		    tp1 = HAL_Motor_ForceCloseDoorThread();
		}
		else 
		{
			tp1 = HAL_Motor_CloseDoorThread();
		}
        if( 1 == tp1 )      //关门流程结束
		{
			if( EM_CLOSE_TOUCH == App_GUI_GetCloseModel() )
			{
				#ifdef LOCK_BODY_212_MOTOR
				HAL_Voice_PlayingVoice( EM_LOCKED_DOOR_MP3, GUI_TIME_2S );
				#elif defined LOCK_BODY_AUTO_MOTOR
				if( LockConfigMode == LOCK_BODY_212 )
				{
					HAL_Voice_PlayingVoice( EM_LOCKED_DOOR_MP3, GUI_TIME_2S );
				}
				#endif
			}
			else 
			{
				HAL_Voice_PlayingVoice( EM_LOCKED_DOOR_MP3, GUI_TIME_2S );
			}
			
			if( true == App_GUI_GetWifiUploadSwSts() )
			{ 
				WifiLockMeg.PageID.way1 = 0;
				WifiLockMeg.PageID.id1 = 0;
				WifiLockMeg.PageID.way2 = 0;
				WifiLockMeg.PageID.id2 = 0;
				WifiLockMeg.Attribute = NONE;
				if( App_GUI_GetCloseModel() == EM_CLOSE_BUTTON )
				{
					WifiLockMeg.UnlockMode = INDOOR_MANUAL_LOCKUP;
					App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
				}
				else if( App_GUI_GetCloseModel() == EM_CLOSE_TOUCH ) 
				{
					WifiLockMeg.UnlockMode = OUTDOOR_MANUAL_LOCKUP;
					App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
				}
				else if( App_GUI_GetCloseModel() == EM_CLOSE_AUTO )
				{	
					#ifdef LOCK_BODY_212_MOTOR
						WifiLockMeg.UnlockMode = TIMING_AUTO_LOCKUP;
					#elif defined LOCK_BODY_216_MOTOR
						WifiLockMeg.UnlockMode = AUTO_LOCKUP;
					#elif defined LOCK_BODY_218_MOTOR
						WifiLockMeg.UnlockMode = AUTO_LOCKUP;
					#elif defined LOCK_BODY_AUTO_MOTOR
						WifiLockMeg.UnlockMode = (LockConfigMode == LOCK_BODY_218) ? AUTO_LOCKUP: TIMING_AUTO_LOCKUP;
					#endif
					App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
				}
			}
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( -1 == tp1 )//关门方向参数异常
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		else if( -2 == tp1 )//假锁报警
		{
			App_GUI_MenuJump( EM_MENU_FALSE_LOCK_WARM );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
	}
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetFaceMenu()
* Description   :  人脸设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetFaceMenu( void )			  //人脸设置菜单
{
	#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetFaceMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        if( ADMIN_NONE_REGISTERED != App_GUI_GetRegisterSts() )  //非体验模式	
		{
			#if defined FACE_FUNCTION_ON 
            HAL_Voice_PlayingVoice( EM_FACE_SET_MENU_MP3, 0 );	
			#elif defined IRIS_FUNCTION_ON
			HAL_Voice_PlayingVoice( EM_IRIS_SET_MENU_MP3, 0 );	
			#endif
		}	
        else 
		{
            APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
			#if defined FACE_FUNCTION_ON 
			HAL_Voice_PlayingVoice( EM_FIRST_USE_FACE_TIPS_MP3, GUI_TIME_3S );
			#elif defined IRIS_FUNCTION_ON
			HAL_Voice_PlayingVoice( EM_FIRST_USE_IRIS_TIPS_MP3, GUI_TIME_3S );
			#endif
            MenuItem.CurMenuNum = EM_MENU_STEP_11;			
		}			
		GuiQuitTimMs = GUI_TIME_20S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //关闭人脸模组
	{
        MenuItem.CurMenuNum = EM_MENU_STEP_2;
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )      //增加管理员人脸
		{
			my_printf("SystemSeting.SysFaceAdminNum = %d\n", SystemSeting.SysFaceAdminNum);
		    if( SystemSeting.SysFaceAdminNum >= MSG_FACE_MASTER_NUM )   //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_FACE_ADD_ADMIN );
				return;	
			}
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //增加普通用户人脸
		{
			my_printf("SystemSeting.SysFaceGuestNum = %d\n", SystemSeting.SysFaceGuestNum);
		    if( SystemSeting.SysFaceGuestNum >= MSG_FACE_GUEST_NUM )   //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_FACE_ADD_GUEST );
				return;	
			}
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //删除人脸
		{
			App_GUI_MenuJump( EM_MENU_FACE_DELETE );
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			DispFuncPtrPre = NULL;
			return;
		}
	}
	else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_FACE_ADD_ADMIN );
			return;
		}
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}

    #endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_AddAdminFaceMenu()
* Description   :  增加管理员人脸
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddAdminFaceMenu( void )	  //增加管理员人脸
{
	#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
	static uint8_t flag;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddAdminFaceMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册
		{
			App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
		}
		else 
		{
			App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK ); 
		}
		GuiQuitTimMs = GUI_TIME_60S;
		flag = 0;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
    {
		uint8_t tp = FaceEnrollPro( MEM_USER_MASTER );
		if(FACE_ADD_OVER == tp)  //启动添加人脸流程
		{	
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if(FACE_ADD_ERROR == tp)
		{
			flag = 1;
		}
		else if( TOUCH_KEY_BACK == App_Touch_GetCurrentKeyValue() ) //返回
		{
			(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			if(flag == 1)
			{
				if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //体验模式  
				{
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
					return;
				}
			}
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}	
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //体验模式  
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
    #endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_AddGuestFaceMenu()
* Description   :  增加普通用户人脸
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddGuestFaceMenu( void )	  //增加普通用户人脸
{
	#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddGuestFaceMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		GuiQuitTimMs = GUI_TIME_60S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if(FACE_ADD_OVER==FaceEnrollPro( MEM_USER_GUEST ))  //启动添加人脸流程
		{	
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == App_Touch_GetCurrentKeyValue() ) //返回
		{
			(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
    #endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFaceMenu()
* Description   :  删除人脸
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelFaceMenu( void )			  //删除人脸
{
    #if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DelFaceMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		FaceDeleteGuestUser();
		HAL_Voice_PlayingVoice( EM_DEL_SUCCESS_MP3, GUI_TIME_1500MS );	
		#if defined FACE_FUNCTION_ON
			SystemEventLogSave( DELETE_FACE, 0);
		#elif defined IRIS_FUNCTION_ON
			SystemEventLogSave( DELETE_IRIS, 0);
		#endif 
		GuiQuitTimMs = GUI_TIME_20S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}
	
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckFaceErrMenu()
* Description   :  人脸验证失败
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckFaceErrMenu( void )	  //人脸验证失败	
{
	#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON

    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckFaceErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			HAL_Voice_PlayingVoice( EM_CHECK_ERR_ALARM_MP3, GUI_TIME_4S );	
		}
		else
		{
			HAL_Voice_PlayingVoice( EM_CHECK_FAIL_MP3, GUI_TIME_1500MS );	
		}
		GuiQuitTimMs = GUI_TIME_2S;
	}

    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			TryAlarmFirst = EM_TRY_DEFAULT;
			SystemSeting.TryForbitUtc = Rtc_Real_Time.timestamp;
			(void)SystemWriteSeting( (uint8_t *)&SystemSeting.TryForbitUtc, sizeof SystemSeting.TryForbitUtc );	
		    if( true == App_GUI_GetWifiWarmingSwSts() )
			{ 
				WifiLockMeg.AlarmMeg = FORBID_TRY;
				App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
			}
			SystemEventLogSave( TRY_OPRN_ALARM, 0 );
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else 
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		
		if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
		{
			App_GUI_MenuJump( EM_MENU_TRY_PROTECT );
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
	
    #endif
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_SetFingerMenu()
* Description   :  指纹设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetFingerMenu( void )		  //指纹设置菜单
{
    uint8_t addAdminFingerkey = 0xF0;
	uint8_t addGuestFingerkey = 0xF0;
	uint8_t delFingerkey      = 0xF0;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetFingerMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		if( ADMIN_NONE_REGISTERED != App_GUI_GetRegisterSts() )  //非体验模式	
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
			#if defined  FINGER_FUNCTION_ON
			HAL_Voice_PlayingVoice( EM_FINGER_SET_MENU_MP3, 0 );
			#elif defined  FINGER_VEIN_FUNCTION_ON
			HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
			#endif
		}
		else 
		{
            APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
			App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
			#if defined  FINGER_FUNCTION_ON
			HAL_Voice_PlayingVoice( EM_FIRST_USE_FINGER_TIPS_MP3, GUI_TIME_4S ); 
			#elif defined  FINGER_VEIN_FUNCTION_ON
			HAL_Voice_PlayingVoice( EM_FIRST_USE_VEIN_TIPS_MP3, GUI_TIME_4S );	
			#endif

			MenuItem.CurMenuNum = EM_MENU_STEP_11;
		}
		
		GuiQuitTimMs = GUI_TIME_26S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		#if !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
			addAdminFingerkey = TOUCH_KEY_NO_1;
			addGuestFingerkey = TOUCH_KEY_NO_2;
			delFingerkey      = TOUCH_KEY_NO_3;
		#else
			addGuestFingerkey = TOUCH_KEY_NO_1;
			delFingerkey      = TOUCH_KEY_NO_2;
		#endif

		if( addAdminFingerkey == tp1 )      //增加管理员指纹
		{
		    if( SystemSeting.SysFingerAdminNum >= MSG_FINGER_ADMIN_LOCAL_NUM )       //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_FINGER_ADD_ADMIN );
				return;	
			}
		}
		else if( addGuestFingerkey == tp1 ) //增加普通用户指纹
		{
		    if( SystemSeting.SysFingerGuestNum >= MSG_FINGER_COMMON_LOCAL_GUINUM )   //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_FINGER_ADD_GUEST );
				return;	
			}
		}
		else if( delFingerkey == tp1 ) //删除指纹
		{
			App_GUI_MenuJump( EM_MENU_FINGER_DELETE );
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			DispFuncPtrPre = NULL;
			return;
		}
	}
	else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_FINGER_ADD_ADMIN );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}


/*********************************************************************************************************************
* Function Name :  App_GUI_AddAdminFingerMenu()
* Description   :  增加管理员指纹
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddAdminFingerMenu( void )	  //增加管理员指纹
{
	uint8_t  addIdlen = 0;
	static uint8_t ADD_ID[3] = {0};
	static uint16_t addId = 0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddAdminFingerMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		else 
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //请输入3位数字编号,按确认键确认
		GuiQuitTimMs = GUI_TIME_60S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//输入格式正确3位数字编号
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(false == APP_FINGER_CfgCheck_CustomID(addId))//customID 重复
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//编号已存在请重新输入 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					FingerAppParam_S fingerPara;
					memset( (void*)&fingerPara, 0, sizeof fingerPara );
					fingerPara.emAppFlow = EM_FINGER_APP_FLOW1_ADD;
					fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ] = MEM_USER_MASTER;
					fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_CUSTOM_ID_H ] = (uint8_t)(addId>>8);
					fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_CUSTOM_ID_L ] = (uint8_t)addId;
					APP_FINGER_Operate( fingerPara );   //启动增加指纹流程
//					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//请刷卡 
					App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//语音提示“格式不正确请重新输入”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //重新来一次
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint16_t pageId = 0;
        FINGER_APP_FLOW_RESULT_E ret = APP_FINGER_GetFlowResult( &pageId );
		if( FINGER_APP_RESULT_SUC == ret )         //添加成功
		{
			App_GUI_SetRegisterSts( ADMIN_LOCAL_REGISTERED );	 
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_1500MS );	
            #ifdef FINGER_VEIN_FUNCTION_ON
            SystemEventLogSave( ADD_VEIN, pageId );
            #else
            SystemEventLogSave( ADD_FINGER, pageId );
            #endif
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
		else if( FINGER_APP_RESULT_FAIL == ret || FINGER_APP_RESULT_TIMEOUT == ret)   //添加失败
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_1500MS );	
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
			
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	uint8_t tp1 = App_Touch_GetCurrentKeyValue();
	if( TOUCH_KEY_BACK == tp1 )     //返回
	{
		APP_FINGER_Sleep();  //关闭指纹模组
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
		return;
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		APP_FINGER_Sleep();  //关闭指纹模组
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册  
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}
}
/*********************************************************************************************************************
* Function Name :  App_GUI_AddGuestFingerMenu()
* Description   :  增加普通用户指纹
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddGuestFingerMenu( void )    //增加普通用户指纹
{
	uint8_t  addIdlen = 0;
	static uint8_t ADD_ID[3] = {0};
	static uint16_t addId = 0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddAdminFingerMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		else 
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //请输入3位数字编号,按确认键确认
		GuiQuitTimMs = GUI_TIME_60S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//输入格式正确3位数字编号
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(false == APP_FINGER_CfgCheck_CustomID(addId))//customID 重复
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//编号已存在请重新输入 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					FingerAppParam_S fingerPara;
					memset( (void*)&fingerPara, 0, sizeof fingerPara );
					fingerPara.emAppFlow = EM_FINGER_APP_FLOW1_ADD;
					fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ] = MEM_USER_GUEST;
					fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_CUSTOM_ID_H ] = (uint8_t)(addId>>8);
					fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_CUSTOM_ID_L ] = (uint8_t)addId;
					APP_FINGER_Operate( fingerPara );   //启动增加指纹流程
//					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//请刷卡 
					App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//语音提示“格式不正确请重新输入”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //重新来一次
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint16_t pageId = 0;
        FINGER_APP_FLOW_RESULT_E ret = APP_FINGER_GetFlowResult( &pageId );
		if( FINGER_APP_RESULT_SUC == ret )         //添加成功
		{
			App_GUI_SetRegisterSts( ADMIN_LOCAL_REGISTERED );	 
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_1500MS );	
            #ifdef FINGER_VEIN_FUNCTION_ON
            SystemEventLogSave( ADD_VEIN, pageId );
            #else
            SystemEventLogSave( ADD_FINGER, pageId );
            #endif
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
		else if( FINGER_APP_RESULT_FAIL == ret || FINGER_APP_RESULT_TIMEOUT == ret)   //添加失败
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_1500MS );	
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
			
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	uint8_t tp1 = App_Touch_GetCurrentKeyValue();
	if( TOUCH_KEY_BACK == tp1 )     //返回
	{
		APP_FINGER_Sleep();  //关闭指纹模组
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
		return;
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		APP_FINGER_Sleep();  //关闭指纹模组
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //未注册  
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFingerMenu()
* Description   :  删除指纹
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelFingerMenu( void )         //删除指纹
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DelFingerMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )   //指纹删除触发
	{
		FingerAppParam_S fingerPara;
		memset( (void*)&fingerPara, 0, sizeof fingerPara );
		fingerPara.emAppFlow = EM_FINGER_APP_FLOW4_CLEAR_COMMON;
		APP_FINGER_Operate(fingerPara);

		MenuItem.CurMenuNum = EM_MENU_STEP_2;
		return;
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint16_t temp;
        FINGER_APP_FLOW_RESULT_E  ret = APP_FINGER_GetFlowResult( &temp );
		if( ret == FINGER_APP_RESULT_SUC )       //success
		{
            HAL_Voice_PlayingVoice( EM_DEL_SUCCESS_MP3, GUI_TIME_1500MS );
            #ifdef FINGER_VEIN_FUNCTION_ON
            SystemEventLogSave( DELETE_VEIN, temp );
            #else
            SystemEventLogSave( DELETE_FINGER, temp );
            #endif
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else if( ret == FINGER_APP_RESULT_FAIL || FINGER_APP_RESULT_TIMEOUT == ret ) //fail
		{
			HAL_Voice_PlayingVoice( EM_DEL_FAIL_MP3, GUI_TIME_1500MS );	
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}

}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckFingerErrMenu()
* Description   :  指纹验证失败
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckFingerErrMenu( void )	  //指纹验证失败
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckFingerErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			HAL_Voice_PlayingVoice( EM_CHECK_ERR_ALARM_MP3, GUI_TIME_4S );	
		}
		else
		{
			HAL_Voice_PlayingVoice( EM_CHECK_FAIL_MP3, GUI_TIME_1500MS );	
		}
		GuiQuitTimMs = GUI_TIME_2S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
			TryAlarmFirst = EM_TRY_DEFAULT;
#endif
			SystemSeting.TryForbitUtc = Rtc_Real_Time.timestamp;
			(void)SystemWriteSeting( (uint8_t *)&SystemSeting.TryForbitUtc, sizeof SystemSeting.TryForbitUtc );	
		    if( true == App_GUI_GetWifiWarmingSwSts() )
			{ 
				WifiLockMeg.AlarmMeg = FORBID_TRY;
				App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
			}
			SystemEventLogSave( TRY_OPRN_ALARM, 0 );
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		
		if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
		{
			App_GUI_MenuJump( EM_MENU_TRY_PROTECT );
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_SetPwdMenu()
* Description   :  密码设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetPwdMenu( void )			  //密码设置菜单	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetPwdMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
        App_GUI_MenuJump( EM_MENU_CHANGE_PWD );
	}
 
	if( 0 == GuiQuitTimMs )
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_ChangePwdMenu()
* Description   :  修改密码菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_ChangePwdMenu( void ) 		  //修改密码
{
	uint8_t  buflen = 0;
	uint8_t  inputBuf[KEY_BUF_SIZE] = {0};
    PwdMeg_T pwdmeg = {0}; 
    static uint8_t  pwdBuf[2][MSG_PWD_BYTE_SIZE] = {0};

    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_ChangePwdMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		HAL_Voice_PlayingVoice( EM_INPUT_NEW_PWD_TIPS_MP3, 0 );	
		MenuItem.CurMenuNum = EM_MENU_STEP_1; 
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //输入新密码
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
			if( tp1 >= 6 )
			{
				PUBLIC_ChangeDecToString( pwdBuf[0], inputBuf, MSG_PWD_BYTE_SIZE );	
				HAL_Voice_PlayingVoice( EM_INPUT_PWD_AGIN_TIPS_MP3, 0 );
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //再次输入新密码
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t ret = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
			if( ret >= 6 )
			{
				PUBLIC_ChangeDecToString( pwdBuf[1], inputBuf, MSG_PWD_BYTE_SIZE );	
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
		
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) //确认新密码
	{
		if( 0 == memcmp( pwdBuf[0], pwdBuf[1], MSG_PWD_BYTE_SIZE ))  //2次密码一致
		{
	        if(((pwdBuf[0][0] == pwdBuf[0][1]) &&
				(pwdBuf[0][0] == pwdBuf[0][2]) &&
				(pwdBuf[0][0] == pwdBuf[0][3]) &&
				(pwdBuf[0][0] == pwdBuf[0][4]) &&
				(pwdBuf[0][0] == pwdBuf[0][5]))||
			   ((pwdBuf[0][0] == pwdBuf[0][1]+1) &&
				(pwdBuf[0][1] == pwdBuf[0][2]+1) &&
				(pwdBuf[0][2] == pwdBuf[0][3]+1) &&
				(pwdBuf[0][3] == pwdBuf[0][4]+1) &&
				(pwdBuf[0][4] == pwdBuf[0][5]+1))||
			   ((pwdBuf[0][0] == pwdBuf[0][1]-1) &&
				(pwdBuf[0][1] == pwdBuf[0][2]-1) &&
				(pwdBuf[0][2] == pwdBuf[0][3]-1) &&
				(pwdBuf[0][3] == pwdBuf[0][4]-1) &&
				(pwdBuf[0][4] == pwdBuf[0][5]-1)))
			{
				HAL_Voice_PlayingVoice( EM_PWD_TOO_SIMPLE_MP3, GUI_TIME_4S ); //密码过于简单
				MenuItem.CurMenuNum = EM_MENU_STEP_5;
			}
			else 
			{
				MenuItem.CurMenuNum = EM_MENU_STEP_4;
			}
			return;
		}
		else  //2次密码不一致
		{
			HAL_Voice_PlayingVoice( EM_PWD_FAIL_AND_INPUT_MP3, GUI_TIME_4S ); 
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
		}
		return;
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum ) //密码存储
	{
		pwdmeg.UserValue  = MEM_PWD_VALID_FLG;
		pwdmeg.Privileges = PWD_LIMIT_ADMIN;
		pwdmeg.UserId     = PWD_USER_ID;
        pwdmeg.Timeliness = 0;
        pwdmeg.Attribute  = 0;
	    pwdmeg.Weekday    = WEEKDAY_VALUE;
		memcpy( pwdmeg.Password, pwdBuf[0], MSG_PWD_BYTE_SIZE );
		pwdmeg.Password[ MSG_PWD_BYTE_SIZE ] = '\0';
		
		RTC_TimeUpdate(RTC_TIME_CLOCK_BCD); //获取最新时间
		RTCType dateClock = Rtc_Real_Time;
		dateClock.hour    = 0;
	    dateClock.minuter = 0;
	    dateClock.second  = 0;
	    uint32_t utcTim = HAL_RTC_TmToTime( &dateClock ); //获取当前日期时间戳
        pwdmeg.StartTim   = utcTim;
		
		dateClock = Rtc_Real_Time;
		dateClock.year    = UTC_STOP_YEAR;
		dateClock.hour    = UTC_STOP_HOUR;
	    dateClock.minuter = UTC_STOP_MINU;
	    dateClock.second  = UTC_STOP_SECOND;
	    utcTim = HAL_RTC_TmToTime( &dateClock );    //获取当前日期时间戳
        pwdmeg.StopTim    = utcTim;
 	
	    App_PWD_SaveOnePwdMegIntoEeprom( &pwdmeg );
		SystemEventLogSave( ADD_PASSWORD, PWD_USER_ID );  
		HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_2S ); 
		App_GUI_SetRegisterSts( ADMIN_LOCAL_REGISTERED );	 
	    MenuItem.CurMenuNum = EM_MENU_STEP_5;
	    return;	
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum ) //语音播报延时
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
        if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
		else if( 0 == HAL_Voice_GetBusyState() )
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;	
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckPwdErrMenu()
* Description   :  密码验证失败菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckPwdErrMenu( void )       //密码验证失败
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckPwdErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
	    SystemSeting.CheckErrPwdCnt++;  
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrPwdCnt, sizeof SystemSeting.CheckErrPwdCnt );	
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			HAL_Voice_PlayingVoice( EM_CHECK_ERR_ALARM_MP3, GUI_TIME_4S );	
		}
		else
		{
			HAL_Voice_PlayingVoice( EM_CHECK_FAIL_MP3, GUI_TIME_1500MS );	
		}
		
		GuiQuitTimMs = GUI_TIME_2S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || ( SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX ) )  //禁试
		{
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
			TryAlarmFirst = EM_TRY_DEFAULT;
#endif
			SystemSeting.TryForbitUtc = Rtc_Real_Time.timestamp;
			(void)SystemWriteSeting( (uint8_t *)&SystemSeting.TryForbitUtc, sizeof SystemSeting.TryForbitUtc );	
		    if( true == App_GUI_GetWifiWarmingSwSts() )
			{ 
				WifiLockMeg.AlarmMeg = FORBID_TRY;
				App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
			}
			SystemEventLogSave( TRY_OPRN_ALARM, 0 );
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else 
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
		{
			App_GUI_MenuJump( EM_MENU_TRY_PROTECT );
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

 
/*********************************************************************************************************************
* Function Name :  App_GUI_BatVolLowErrMenu()
* Description   :  电池电压低无法工作处理
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BatVolLowErrMenu( void )	  //电压低无法工作
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BatVolLowErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}	
	    App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON );
	    App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_ON ); 
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_LOW_POWER);
		#ifndef BAT_UNINSET_WARM_TIP_ON   //电池未插提醒开启
		HAL_Voice_PlayingVoice( EM_BAT_DONOT_WORK_MP3, 0 );  
		MenuItem.CurMenuNum = EM_MENU_STEP_11;
		GuiQuitTimMs = GUI_TIME_5S;
		#else 
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
		GuiQuitTimMs = GUI_TIME_10S;
		#endif
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		#ifdef UPPER_BAT_ADC_ON
		int8_t tp1 = HAL_ADC_GetCellBatVolState( E_UPPER_BAT );
		if( -3 == tp1 )      //未插电池
		{	
			#ifdef BATTERY_MP3_TIPS_ON
				HAL_Voice_PlayingVoice( EM_INSERT_UNDER_BAT_TIPS_MP3, GUI_TIME_3S );
			#else
				HAL_Voice_PlayingVoice( EM_INSERT_UPPER_BAT_TIPS_MP3, GUI_TIME_3S );  
			#endif
		}
		else if( -2 == tp1 ) //电压低无法工作
		{
			#ifdef BATTERY_MP3_TIPS_ON
				HAL_Voice_PlayingVoice( EM_UNDER_BAT_UNWORK_MP3, GUI_TIME_3S );
			#else
				HAL_Voice_PlayingVoice( EM_UPPER_BAT_UNWORK_MP3, GUI_TIME_3S );  
			#endif
		}
		#endif
		MenuItem.CurMenuNum = EM_MENU_STEP_2;
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 1 == HAL_Voice_GetBusyState() )  //语音未播报完毕
			return;
		
		#ifdef UNDER_BAT_ADC_ON
		int8_t sp1 = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
		if( -3 == sp1 )      //未插电池
		{	
			#ifdef BATTERY_MP3_TIPS_ON
				HAL_Voice_PlayingVoice( EM_INSERT_UPPER_BAT_TIPS_MP3, GUI_TIME_3S );
			#else
				HAL_Voice_PlayingVoice( EM_INSERT_UNDER_BAT_TIPS_MP3, GUI_TIME_3S );  
			#endif
		}
		else if( -2 == sp1 ) //电压低无法工作
		{
			#ifdef BATTERY_MP3_TIPS_ON
				HAL_Voice_PlayingVoice( EM_UPPER_BAT_UNWORK_MP3, GUI_TIME_3S );
			#else
				HAL_Voice_PlayingVoice( EM_UNDER_BAT_UNWORK_MP3, GUI_TIME_3S );  
			#endif
		}
		#endif
		MenuItem.CurMenuNum = EM_MENU_STEP_3;
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音未播报完毕
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP ); 
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_EepromErrorMenu()
* Description   :  eeprom故障处理
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_EepromErrorMenu( void )       //EEPROM故障
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_EepromErrorMenu()\n" );    
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
	    App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_E, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		GuiQuitTimMs = GUI_TIME_5S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		
	}
	if( 0 == GuiQuitTimMs )
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_AlarmHandlerMenu()
* Description   :  防撬告警处理
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AlarmHandlerMenu( void )      //防撬告警处理
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_AlarmHandlerMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_2;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_ON ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 0 );	
		HAL_Voice_WorkModeCtrl( true );//语音进入报警模式
		GuiQuitTimMs = GUI_TIME_900S;
		
		if( s_GuiData.AlamMeg.AlamButtonNewTrigger == false )  //首次触发
		{
			s_GuiData.AlamMeg.AlamButtonNewTrigger = true;
			if( true == App_GUI_GetWifiWarmingSwSts() )
			{ 
				WifiLockMeg.AlarmMeg = LOCKPICKING;
				App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
				MenuItem.CurMenuNum = EM_MENU_STEP_1;
			}
			SystemEventLogSave( PICK_OPRN_ALARM, 0 );
			s_GuiData.AlamMeg.AlamButtonHoldTimSec = GUI_TIME_900S;
		}
	}

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )      //猫眼视频抓拍
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
			if( true == App_GUI_CheckBellVideoAction() )
			{
				if(FaceProAlarm(LOCKPICKING, WifiTxTemp.data, WifiTxTemp.length) == 1)
				{
					my_printf( "face capture is over!\n" ); 
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
				}
			}
		#else
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		#endif	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )      //防撬过程中其他方式打断处理
	{
		/*-------------防撬解除---------------*/	 
		if( true == App_GUI_CheckAlarmButtonRecoveryWarm())
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
	   /*--------------触摸板唤醒-------------*/	
	    if( 1 == App_Export_GetPinState( E_PIN_TOUCH_IRQ ) )     
		{
			my_printf( "alarm break by key touch!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------门内开门按键唤醒-------*/
		else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   
		{
			my_printf( "alarm break by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------门内关门按键唤醒-------*/
		else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  
		{
			my_printf( "alarm break by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  
			SystemWakeupInit();   
			return;
		}
	   /*--------------指纹唤醒---------------*/
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	    
		{
			my_printf( "alarm break by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  			
			SystemWakeupInit();    
			return;
		}	
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //统一关闭当前声音播报
	{
		HAL_Voice_WorkModeCtrl( false );//语音退出报警模式
		HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
		if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
		}	
		MenuItem.CurMenuNum = EM_MENU_STEP_4;
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )  //统一去休眠
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_TryProtectMenu()
* Description   :  禁试保护告警
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_TryProtectMenu( void )        //禁试保护告警
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_TryProtectMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ONLY_BELL );  //按键扫描
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		HAL_Voice_PlayingVoice( EM_ERR_PROTECT_ALARM_MP3, 0 );	
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_ON );//有门铃灯的产品点亮门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		GuiQuitTimMs = GUI_TIME_7S;
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		if(( true == App_GUI_CheckBellVideoAction() ) && (EM_TRY_DEFAULT == TryAlarmFirst))
		{
			GuiDelayTimMs = GUI_TIME_40S;
		}
		else
		{
			GuiDelayTimMs = GUI_TIME_7S;
		}
		#else
			GuiDelayTimMs = GUI_TIME_7S;
		#endif
	}
	
	if( 0 == GuiDelayTimMs )  
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		if( true == App_GUI_CheckBellVideoAction() )
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		#else
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		#endif
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		if(EM_TRY_DEFAULT == TryAlarmFirst)
		{
			if( true == App_GUI_CheckBellVideoAction() )
			{
				if(FaceProAlarm(FORBID_TRY, WifiTxTemp.data, WifiTxTemp.length) == 1)
				{
					TryAlarmFirst = EM_TRY_FIRST_OVER;//180s内抓拍成功过一次
					my_printf("FaceProAlarmResult = 1;\n");
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
				}
			}
		}
		
		if(TOUCH_KEY_BELL == tp1) //门铃触发
		{
			HAL_Voice_BellCtrl( true );
			if(EM_TRY_FIRST_OVER == TryAlarmFirst)//180s内抓拍成功过一次
			{
				App_GUI_MenuJump( EM_MENU_BELL_VIDEO );
				return;
			}
		}
		#else
		if( TOUCH_KEY_BELL == tp1 )  //门铃触发
		{
			HAL_Voice_BellCtrl( true );
			#if defined XM_CAM_FUNCTION_ON   //单猫眼
				if( 1 == App_GUI_CheckBellVideoAction() )
				{
					CAM_SendCommandStart(CAM_CMD_BELL, 0, 0);
				} 
			#endif 
		}
		#endif
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		if( true == App_GUI_CheckBellVideoAction() )
		{
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			}
		}
		else
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
		#endif
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_StayWarmMenu()
* Description   :  逗留保护告警
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_StayWarmMenu( void )		  //逗留保护告警
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_StayWarmMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //按键扫描
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		HAL_Voice_PlayingVoice( EM_HI_TELL_ADMIN_MP3, 0 );	
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//单人脸模组
		
		#else
		if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			WifiLockMeg.AlarmMeg = DEFENSE;
			App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
		}
		#endif
		App_GUI_StopStayDetectTim();//停止当次逗留计时
		GuiQuitTimMs = GUI_TIME_40S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		if( true == App_GUI_CheckBellVideoAction() )
		{
			if(FaceProAlarm(DEFENSE, WifiTxTemp.data, WifiTxTemp.length) == 1)
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			}
		}
		#endif
		#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//单人脸模组
		if( true == App_GUI_CheckBellVideoAction() )//抓拍图片
		{
			if(FaceGneralTaskFlow(FACE_CMD_SNAPIMAGE,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
			{
				FaceImage.LenAll = 0;
				FaceImage.LenOffset = 0;
				FaceImage.LenSur = 0;
				WifiLockMeg.PhotoPackageNum = 0;
				WifiLockMeg.PhotoPakageSum = 0;
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
				my_printf("WIFI_PowerOn\n");
			}
		}
		#endif
	}
	#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//单人脸模组
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )//获取待传输图片的编号
	{
		if(FaceGneralTaskFlow(FACE_CMD_GETSAVEDIMAGE, 0, 0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
		{
			App_WIFI_UartPowerOn();
			WifiLockMeg.PhotoUploadType = 0x02;//图片类型(1byte)（0x01代表门铃0x02代表抓拍）
			FaceImage.LenSur = FaceImage.LenAll;
			WifiLockMeg.PhotoPakageSum = FaceImage.LenAll / FACE_IMAGE_LEN;
			if(FaceImage.LenAll % FACE_IMAGE_LEN)
			{
				WifiLockMeg.PhotoPakageSum += 1;
			}
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			GuiQuitTimMs = GUI_TIME_20S;
			my_printf("FACE_CMD_GETSAVEDIMAGE\n");
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )//获取待上传图片的偏移量和此次上传图片的大小
	{
		if(FaceGneralTaskFlow(FACE_CMD_UPLOADIMAGE,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
			GuiQuitTimMs = GUI_TIME_20S;
			WifiLockMeg.PhotoState = WIFI_TX_PREPARE;
			my_printf("FACE_CMD_UPLOADIMAGE\n");
		}
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )//wifi发送图片的一包数据
	{
		if(WifiLockMeg.PhotoPackageNum == WifiLockMeg.PhotoPakageSum)//发完图片
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
			GuiQuitTimMs = GUI_TIME_20S;
			my_printf("EM_MENU_STEP_4 WifiLockMeg.PhotoPackageNum == WifiLockMeg.PhotoPakageSum");
			return;
		}
		if(WifiLockMeg.PhotoState == WIFI_TX_ING)
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
			GuiQuitTimMs = GUI_TIME_30S;
			FaceUartInit();
			AppFaceWorkPro.TaskFlow = TASK_START;
			my_printf("EM_MENU_STEP_4 MenuItem.CurMenuNum = WIFI_TX_ING\n");
		}
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )//获取待上传图片的偏移量和此次上传图片的大小
	{
		if(FaceGneralTaskFlow(FACE_CMD_UPLOADIMAGE,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
			GuiQuitTimMs = GUI_TIME_20S;
			//串口初始化
			App_WIFI_PhotoUartInit();
			my_printf("EM_MENU_STEP_5 FACE_CMD_UPLOADIMAGE\n");
		}
	}
	else if( EM_MENU_STEP_6 == MenuItem.CurMenuNum )
	{
		if(WifiLockMeg.PhotoState == WIFI_TX_START)
		{
			my_printf("EM_MENU_STEP_6 WifiLockMeg.PhotoState == WIFI_TX_START\n");
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}
		else if(WifiLockMeg.PhotoState == WIFI_TX_OVER)
		{
			FacePowerDown();
			my_printf("EM_MENU_STEP_6 FACE_CMD_POWERDOWN\n");
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
	}
	#endif
	if( 0 == GuiQuitTimMs )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
			if( true == App_GUI_CheckBellVideoAction() )
			{
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
				if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
				{
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				}
			}
			else
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			}
		#else
			#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//单人脸模组
			WifiLockMeg.PhotoState = WIFI_TX_OVER;
			WifiLockMeg.State = WIFI_TX_OVER;
			#endif
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		#endif
	}
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_DeployWarmMenu()
* Description   :  布防保护告警
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DeployWarmMenu( void )        //布防保护告警
{
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
	static uint8_t FaceProAlarmResult;
#endif
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_DeployWarmMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //按键扫描 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, GUI_TIME_15S );	
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		FaceProAlarmResult = 0;
#endif
		
		if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			WifiLockMeg.AlarmMeg = EPLOYMENT;
			App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
		}
		
		GuiQuitTimMs = GUI_TIME_60S;
		GuiDelayTimMs = GUI_TIME_60S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
		if( true == App_GUI_CheckBellVideoAction() )
		{
			if(FaceProAlarmResult == 0)
			{
				if(FaceProAlarm(EPLOYMENT, WifiTxTemp.data, WifiTxTemp.length) == 1)
				{
					FaceProAlarmResult = 1;
					return;
				}
			}
		}
		if( (0 == HAL_Voice_GetBusyState()) && (FaceProAlarmResult == 1) )  //语音播报完毕
		{
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); //语音终止
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;
		}
		#else
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); //语音终止
			(void)HAL_Motor_DefendActionCheck(true);
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		#endif
		
		if( 0 == GuiDelayTimMs )
		{
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			#else
				(void)HAL_Motor_DefendActionCheck(true);
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			#endif
		}
	}	
	else if(EM_MENU_STEP_2 == MenuItem.CurMenuNum)
	{
		if( true == App_GUI_CheckBellVideoAction() )
		{
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				(void)HAL_Motor_DefendActionCheck(true);
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			}
			#endif
		}
		else
		{
			(void)HAL_Motor_DefendActionCheck(true);
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
	}
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_FalseLockWarmMenu()
* Description   :  假锁保护告警
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_FalseLockWarmMenu( void )     //假锁保护告警
{
	static bool autoLockCheckFlg;
	static uint8_t timcnt, warmcnt;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_FalseLockWarmMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //按键扫描
		
	#ifdef BAT_UNINSET_WARM_TIP_ON
		if( true == HAL_ADC_GetSysVolLowWarmState() )   //系统电压低
		{
			App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_ON ); 
		}
		else 
		{
			App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_ON ); 
		}
	#else 
		int8_t tp1;
		tp1 = HAL_ADC_GetVolLowGradeWarm(); 
		if( tp1 == 1 )
		{
			if( (-3 == HAL_ADC_GetCellBatVolState( E_UPPER_BAT )) && (-3== HAL_ADC_GetCellBatVolState( E_UNDER_BAT ))) 
			{
			    App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_ON );   //电池未插
			}
			else 
			{
				App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_ON ); 
			}
		}
		else if( tp1 < 0 )
		{
			App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_ON );  
		}
	#endif
		
	    App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_NOLOCK);
        HAL_Voice_PlayingVoice( EM_FALSE_LOCK_TIPS_MP3, GUI_TIME_3S );	 //门未锁好
		SystemEventLogSave( FALSE_LOCK_ALARM, 0 );  
		if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			WifiLockMeg.AlarmMeg = FALSE_LOCK;
			App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
		}
		autoLockCheckFlg = false;
		timcnt = 0;
		warmcnt = 0;
		GuiQuitTimMs = GUI_TIME_13S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
	    if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))  //自动锁体上锁
		{
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 ); 
			if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF );
			}			
			return;
		}
		
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			if( warmcnt >= 2)
			{
				if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
				{
					HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF );
				}
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
			
			uint16_t tp1 = 0;
			timcnt %= 10;
			
			if( timcnt%2 == 0 )
			{
				tp1 = GUI_TIME_100MS;
			}
			else if( timcnt%2 == 1 )
			{
				tp1 = GUI_TIME_1200MS;
				if( timcnt == 9 )
				{
					tp1 = GUI_TIME_3S;
					if( warmcnt ) 
					{
						tp1 = GUI_TIME_300MS;
					}	
					warmcnt++;
				}
			}
 
			timcnt++;
			HAL_Voice_PlayingVoice( EM_WARMING_TIPS_MP3, tp1 );	
		}	
	}
 
//	if( 0 == GuiQuitTimMs )
//	{
//		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
//		return;
//	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_ForgetLockWarmMenu()
* Description   :  门未锁保护告警
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_ForgetLockWarmMenu( void )    //门未锁保护告警
{
	static bool autoLockCheckFlg;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_ForgetLockWarmMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
	    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //按键扫描
	    App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_NOLOCK);
        HAL_Voice_PlayingVoice( EM_FALSE_LOCK_TIPS_MP3, GUI_TIME_3S );	 //门未锁好
		SystemEventLogSave( FALSE_LOCK_ALARM, 0 );  
	    if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			WifiLockMeg.AlarmMeg = FORGET_LOCK;
			App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
		}
		autoLockCheckFlg = false;
		GuiQuitTimMs = GUI_TIME_10S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
	    if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))  //自动锁体上锁
		{
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
		    if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF );
			}
			return;
		}

		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF );
			}
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF );
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_BleOpenErrWarmMenu()
* Description   :  蓝牙开门失败告警
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BleOpenErrWarmMenu( void )    //蓝牙开门失败告警
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BleOpenErrWarmMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);

		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			HAL_Voice_PlayingVoice( EM_CHECK_ERR_ALARM_MP3, GUI_TIME_4S );	
		}
		else
		{
			HAL_Voice_PlayingVoice( EM_CHECK_FAIL_MP3, GUI_TIME_1500MS );	
		}
		
		GuiQuitTimMs = GUI_TIME_2S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
			TryAlarmFirst = EM_TRY_DEFAULT;
#endif
			SystemSeting.TryForbitUtc = Rtc_Real_Time.timestamp;
			(void)SystemWriteSeting( (uint8_t *)&SystemSeting.TryForbitUtc, sizeof SystemSeting.TryForbitUtc );	
		    if( true == App_GUI_GetWifiWarmingSwSts() )
			{ 
				WifiLockMeg.AlarmMeg = FORBID_TRY;
				App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
			}
			SystemEventLogSave( TRY_OPRN_ALARM, 0 );
			MenuItem.CurMenuNum = EM_MENU_STEP_3; 
		}
		else 
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
		{
			App_GUI_MenuJump( EM_MENU_TRY_PROTECT );
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_BackFactoryMenu()
* Description   :  恢复出厂设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BackFactoryMenu( void )		  //恢复出厂设置
{
	static bool optionflg;
    static uint8_t firstFlg;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BackFactoryMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_BACK_ENTER );      //触摸按按键处理 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );    //灭屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_FACTORY);
		HAL_Voice_PlayingVoice( EM_BACK_FACTORY_COMFIRM_MP3, 0 );  //语音播报
		
		firstFlg = 0;
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			GuiQuitTimMs = GUI_TIME_60S;
			App_Touch_FuncEnCtrl( EM_SCAN_OFF );      //触摸按按键处理 	
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_OFF );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;	
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //待语按键音播报完毕
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		HAL_Voice_PlayingVoice( EM_BACKING_FACTORY_MP3, GUI_TIME_2500MS );   
		MenuItem.CurMenuNum = EM_MENU_STEP_3;
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //恢复出厂设置中
	{
		int8_t ret = SystemParaBackFactoryIntoFlash( &firstFlg ); 
		if( ret == 1 )       //success
		{
			SystemEventLogSave( EMPTY_LOCK, 0 );  
			optionflg = 1; 
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}	
		else if( ret == -1 ) //fail
		{
			optionflg = 0; 
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )  //设定结果
	{
        if( 0 == HAL_Voice_GetBusyState() )
		{
            if( optionflg == 0 )
            {
                if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
                {
                    HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
                }
                App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
                App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
                App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
                App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
                HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 0 );
                PUBLIC_Delayms(3000);//报警音持续3秒
                HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );
            }
            
			VoiceType_E mp3 = ( optionflg == 1 ) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_2S );   
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
		}
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )   
	{
        if( 0 == HAL_Voice_GetBusyState() )
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;	
		}
	}
	if( 0 == GuiQuitTimMs )  //超时返回
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP ); 
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GotoOtaModelMenu()
* Description   :  升级模式
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_GotoOtaModelMenu( void )	  //升级模式
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_GotoOtaModelMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
		GuiQuitTimMs = GUI_TIME_12S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		
	}
	if( 0 == GuiQuitTimMs )
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GotoAppModelMenu()
* Description   :  APP设置模式
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_GotoAppModelMenu( void )	  //APP设置模式
{
	static bool bleConnectSts = false;
	static bool enterAppMode = false;   //进入APP模式的方式  false= ble   true= key
	static uint8_t proStep;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_GotoAppModelMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );      //灭屏
		App_LED_OutputCtrl( EM_LED_CFG_NET, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Key_ResetCombinKeyFlow();         //强制复位组合按键检测流程
		APP_BleSetAdminState( E_BLE_ADMIN );
		enterAppMode = false;
		/*--ID2唤醒配置------*/
		APP_ID2Init();
		my_printf("APP_ID2Init =%d\n", SystemTick);
	    if( !DRV_GetBleConnect() )    //蓝牙非连接状态
		{
			my_printf( "Ble connect agin!\n" ); 
			APP_BleSetAdminState( E_KEY_ADMIN );
			enterAppMode = true;
			//初始化蓝牙直接开始广播
			APP_BleInit( ADMIN_MODE ,GENERAL_FLAGS);//Dsmn
		    HAL_Voice_PlayingVoice( EM_APP_MODE_ON_MP3, 0 ); 
		}
		bleConnectSts = false;
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
		GuiQuitTimMs = GUI_TIME_60S;
		proStep = 0;
		my_printf( "app gui init is ok!\n" ); 
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
        if( DRV_GetBleConnect() )    
		{
			bleConnectSts = true;
			BleDisconnectHoldTimeMs = 5;
		}
 
		/*----蓝牙任务调度---*/	
	    APP_BleServerProcess();
		
		/*----手机蓝牙开门---*/	
		if( E_PHONE_CHECK_OK == APP_BleGetAdminState() )           //success
		{
			APP_BleSetAdminState( E_DEFAULT );
			App_GUI_SetOpenModel( EM_OPEN_PHONE );
			APP_FINGER_Sleep();  //关闭指纹模组
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		else if( E_PHONE_CHECK_ERR == APP_BleGetAdminState() )     //fail
		{
			APP_FINGER_Sleep();  //关闭指纹模组
			APP_BleSetAdminState( E_DEFAULT );
			App_GUI_MenuJump( EM_MENU_BLE_OPEN_ERR );
			return;
		}
		/*----蓝牙钥匙开门---*/	
	    if( E_SMARTKEKY_CHECK_OK == APP_BleGetAdminState() )       //success
		{
			APP_BleSetAdminState( E_DEFAULT );
			App_GUI_SetOpenModel( EM_OPEN_BLE_KEY );
			APP_FINGER_Sleep();  //关闭指纹模组
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		else if( E_SMARTKEKY_CHECK_ERR == APP_BleGetAdminState() ) //fail
		{
			APP_BleSetAdminState( E_DEFAULT );
			APP_FINGER_Sleep();  //关闭指纹模组
			App_GUI_MenuJump( EM_MENU_BLE_OPEN_ERR );
			return;
		}	
		
		if( E_VIDEO_ADMIN == APP_BleGetAdminState() )  //分离屏模式
		{
			if( proStep == 0 )
			{
			    App_Touch_FuncEnCtrl( EM_SCAN_ON );  
				App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );      //灭屏
				proStep = 1;
			}
			else if( proStep == 1 )
			{
				BUTTON_TYPE_E ret = App_Key_GetCombinKeyState();  //获取机械按键组合键
				if( EM_OPEN_DOOR_KEY == ret )          //按键开门
				{
					APP_BleSetAdminState( E_DEFAULT );
					App_GUI_SetOpenModel( EM_OPEN_BUTTON );
					SystemEventLogSave( KEY_OPEN_IN_DOOR, 0 );  
					App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
					return;
				}
				#ifdef CLOSE_BUTTON_ON
				else if( EM_CLOSE_DOOR_KEY == ret )    //按键关门
				{
					APP_BleSetAdminState( E_DEFAULT );
					App_GUI_SetCloseModel( EM_CLOSE_BUTTON );
					SystemEventLogSave( KEY_CLOSE_IN_DOOR, 0 );  
					App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
					return;
				}
				#endif

				if( TOUCH_KEY_BACK == App_Touch_GetCurrentKeyValue() )     //back键
				{
					my_printf( "touch key is back!\n" );
					APP_BleSetAdminState( E_DEFAULT );
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
					return;
				}
			}
		}
	}
	
    if( !DRV_GetBleConnect() )   //确认是否为断开
	{
		if( (bleConnectSts == true) && (enterAppMode == false) )
		{
			if( 0 == BleDisconnectHoldTimeMs )
			{
				bleConnectSts = false;
				APP_BleSetAdminState( E_DEFAULT );
				APP_FINGER_Sleep();  //关闭指纹模组
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
		}
	}

	if( 0 == GuiQuitTimMs )
	{
		my_printf( "gui option timeout!\n" );
		APP_BleSetAdminState( E_DEFAULT );
		APP_FINGER_Sleep();  //关闭指纹模组
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysParaMenu()
* Description   :  系统参数设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetSysParaMenu( void )		  //系统参数设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetSysParaMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_SYS_SET_MENU_MP3, 0 );   //语音播报
		
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //电机方向
		{
			App_GUI_MenuJump( EM_MENU_MOTOR_DIR_SET );
			return;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //电机扭力
		{
			App_GUI_MenuJump( EM_MENU_MOTOR_TORSION_SET );
			return;
		}
//		else if( TOUCH_KEY_NO_3 == tp1 ) //自动上锁
//		{
//			App_GUI_MenuJump( EM_MENU_AUTO_LOCK_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_4 == tp1 ) //双重认证
//		{
//			App_GUI_MenuJump( EM_MENU_DOUBLE_CHECK_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_5 == tp1 ) //音量调节
//		{
//			App_GUI_MenuJump( EM_MENU_VOL_ADJUST_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_6 == tp1 ) //接近感应
//		{
//			App_GUI_MenuJump( EM_MENU_NEAR_SENSE_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_7 == tp1 ) //布防设置
//		{
//			App_GUI_MenuJump( EM_MENU_DEPPOY_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_8 == tp1 ) //逗留设置
//		{
//			App_GUI_MenuJump( EM_MENU_STAY_CHECK_SET );
//			return;
//		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetMotorDirMenu()
* Description   :  设置电机开门方向
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetMotorDirMenu( void )       //电机方向设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetMotorDirMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );    //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_DIR_SET_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //左开
		{
			SystemFixSeting.MotorDirection = LEFT_HAND_DOOR;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorDirection, sizeof SystemFixSeting.MotorDirection );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报

			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //右开
		{
			SystemFixSeting.MotorDirection = RIGHT_HAND_DOOR;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorDirection, sizeof SystemFixSeting.MotorDirection );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )   //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetMotorTorsionMenu()
* Description   :  设置电机扭力 
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetMotorTorsionMenu( void )   //电机扭力设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetMotorTorsionMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_TORQUE_SET_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //低扭力
		{
			SystemFixSeting.MotorTorque = LOW_TORQUE;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorTorque, sizeof SystemFixSeting.MotorTorque );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //高扭力
		{
			SystemFixSeting.MotorTorque = HIGH_TORQUE;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorTorque, sizeof SystemFixSeting.MotorTorque );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetAutoLockMenu()
* Description   :  设置自动上锁
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetAutoLockMenu( void )		  //自动上锁设置
{    
	static uint8_t pwdLen =0;
	static uint8_t pwddata[KEY_BUF_SIZE+1]= {0};
	static uint8_t playStep;
	static uint8_t errcnt;
	static uint8_t paraSet;
	
    if( DispFuncPtrPre != DispFuncPtr )
	{
		my_printf( "App_GUI_SetAutoLockMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_ATUO_LOCK_MENU_MP3, 0 ); //语音播报
		
		GuiQuitTimMs = GUI_TIME_30S;
		playStep = 0;
		errcnt = 0;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		App_Touch_GetCurrentKeyValBuf( pwddata, &pwdLen ); //释放缓存
		if( TOUCH_KEY_NO_1 == tp1 )     //时间设置
		{
			HAL_Voice_PlayingVoice( EM_INPUT_2NUM_AND_COMFIRM_MP3, 0 ); //语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //关闭自动上锁功能
		{
			SystemSeting.SysAutoLockTime = 0;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemSeting.SysAutoLockTime, sizeof SystemSeting.SysAutoLockTime );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_11;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //设定时间
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //时间设置
		{
			App_Touch_GetCurrentKeyValBuf( pwddata, &pwdLen ); //释放缓存
			paraSet = pwddata[0]*10 + pwddata[1]; 
			if( (pwdLen == 2) && (paraSet >= 10) && (paraSet <= 99) )
			{
				HAL_Voice_PlayingVoice( EM_AUTO_LOCK_TIME_MP3, GUI_TIME_2500MS );//语音播报
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
			}
			else
			{
				errcnt++;
				if( errcnt>= 3 )
				{
					App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
					return;
				}
				else 
				{
					HAL_Voice_PlayingVoice( EM_FORMAT_WRONG_TIPS_MP3, GUI_TIME_600MS );//语音播报
					GuiQuitTimMs = GUI_TIME_15S;
				}
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //设定时间
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //时间确认
		{
			SystemSeting.SysAutoLockTime = paraSet;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysAutoLockTime, sizeof SystemSeting.SysAutoLockTime );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_11;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
		
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			if( playStep < 0xFE )
				playStep++;
			if( playStep == 1 )
			{
				HAL_Voice_PlayingVoice( (VoiceType_E)(EM_NUM_0_MP3+pwddata[0]), GUI_TIME_400MS );   //十位数
			}
			else if( playStep == 2 )
			{
				HAL_Voice_PlayingVoice( EM_NUM_10_MP3, GUI_TIME_400MS ); //十
			}
			else if( playStep == 3 )
			{
				if( pwddata[1] != 0 )
				{
					HAL_Voice_PlayingVoice( (VoiceType_E)(EM_NUM_0_MP3+pwddata[1]), GUI_TIME_400MS );//个位数
				}
				else
				{
					HAL_Voice_PlayingVoice( EM_SECOND_MP3, GUI_TIME_400MS );//秒
				}
			}
			else if( playStep == 4 )
			{
				if( pwddata[1] != 0 )
				{
					HAL_Voice_PlayingVoice( EM_SECOND_MP3, GUI_TIME_400MS );//秒
				}
			}
			else if( playStep == 5 )
			{
				HAL_Voice_PlayingVoice( EM_COMFIRM_OR_BACK_TIPS_MP3, 0 );//确认提醒
				GuiQuitTimMs = GUI_TIME_15S;
			}
		}
	}
    else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum )  //设定时间
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetDoubleCheckMenu()
* Description   :  设置双重认证 
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetDoubleCheckMenu( void )	  //双重认证设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetDoubleCheckMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_VERIFY_SET_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //开启
		{
			SystemSeting.SysCompoundOpen = DOUBLE_CHECK_SW_ON;
			uint8_t ret = SystemWriteSeting( (uint8_t *)&SystemSeting.SysCompoundOpen, sizeof SystemSeting.SysCompoundOpen );
			DoubleCheckType_U  KeyTppe ={0};
			#ifdef  FACE_FUNCTION_ON
		    KeyTppe.bit.FaceCheckEnable = FUNCTION_ENABLE;
			KeyTppe.bit.FingerCheckEnable = FUNCTION_ENABLE;
//			KeyTppe.bit.PwdCheckEnable = FUNCTION_ENABLE;
			#else
			KeyTppe.bit.FingerCheckEnable = FUNCTION_ENABLE;
			KeyTppe.bit.PwdCheckEnable = FUNCTION_ENABLE;
			#endif
			SystemSeting.SysLockMode = KeyTppe.data;
			ret = SystemWriteSeting( (uint8_t *)&SystemSeting.SysLockMode, sizeof SystemSeting.SysLockMode );
			
			VoiceType_E mp3 = (ret == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //关闭
		{
			SystemSeting.SysCompoundOpen = DOUBLE_CHECK_SW_OFF;
			uint8_t ret = SystemWriteSeting( (uint8_t *)&SystemSeting.SysCompoundOpen, sizeof SystemSeting.SysCompoundOpen );
			SystemSeting.SysLockMode = 0;
			ret = SystemWriteSeting( (uint8_t *)&SystemSeting.SysLockMode, sizeof SystemSeting.SysLockMode );
			VoiceType_E mp3 = (ret == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetVolGradeMenu()
* Description   :  设置音量等级
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetVolGradeMenu( void )       //音量调节设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetVolGradeMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_VOL_SET_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //高音量
		{
			SystemSeting.SysVoice = HIGH_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH ); 
			}
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //中音量
		{
			SystemSeting.SysVoice = MEDIUM_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_MED ); 
			}
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //低音量
		{
			SystemSeting.SysVoice = LOW_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_LOW ); 
			}
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //静音模式
		{
			SystemSeting.SysVoice = OFF_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF ); 
			}
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetNearSenseMenu()
* Description   :  设置接近感应
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetNearSenseMenu( void )	  //接近感应设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetNearSenseMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_NEAR_REACT_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //开启
		{
			SystemSeting.SysDrawNear = FUNCTION_ENABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysDrawNear, sizeof SystemSeting.SysDrawNear );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //关闭
		{
			SystemSeting.SysDrawNear = FUNCTION_DISABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysDrawNear, sizeof SystemSeting.SysDrawNear );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetDeployMenu()
* Description   :  设置布防设置
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetDeployMenu( void )		  //布防设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetDeployMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_DEPLOY_SET_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //开启
		{
			SystemSeting.SysKeyDef = FUNCTION_ENABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysKeyDef, sizeof SystemSeting.SysKeyDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //关闭
		{
			SystemSeting.SysKeyDef = FUNCTION_DISABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysKeyDef, sizeof SystemSeting.SysKeyDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
} 

/*********************************************************************************************************************
* Function Name :  App_GUI_SetStayCheckMenu()
* Description   :  设置逗留设置
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetStayCheckMenu( void )	  //逗留设置
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetStayCheckMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_STAY_SET_MENU_MP3, 0 );//语音播报
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //开启
		{
			SystemSeting.SysHumanIrDef = 30;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysHumanIrDef, sizeof SystemSeting.SysHumanIrDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //关闭
		{
			SystemSeting.SysHumanIrDef = FUNCTION_DISABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysHumanIrDef, sizeof SystemSeting.SysHumanIrDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//语音播报
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_ErrorCheckMenu()
* Description   :  系统故障检测处理流程
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_ErrorCheckMenu( void )		  //系统故障检测	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_ErrorCheckMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
        (void)HAL_Voice_PlayingVoice(EM_BUTTON_TIPS_MP3, 0);
		GuiQuitTimMs = GUI_TIME_60S;
	}
	
#ifdef  XM_CAM_FUNCTION_ON
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )       //PIR产测
	{
        uint8_t ret = CAM_PirWakeUpTest();
	    if(1 == ret)
        {
			App_LED_OutputCtrl( EM_LED_0, EM_LED_ON );
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON );
			App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_ON );
		    HAL_Voice_PlayingVoice( EM_BUTTON_TIPS_MP3, 0 );	
            MenuItem.CurMenuNum = EM_MENU_STEP_2;
            GuiDelayTimMs = GUI_TIME_3S;//3S计时退出
        }
        else if(2 == ret)
        {
			App_LED_OutputCtrl( EM_LED_9, EM_LED_ON );
		    HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 0 );	
            MenuItem.CurMenuNum = EM_MENU_STEP_3;
            GuiDelayTimMs = GUI_TIME_3S;//3S计时退出
        }
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //PIR产测,猫眼返回成功
	{
		if(GuiDelayTimMs == 0)
        {
		    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );
		    (void)App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
            return;
        }  
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) //PIR产测,猫眼返回失败
	{
		if(GuiDelayTimMs == 0)
        {
		    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );
		    (void)App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
            return;
        }

	}
#else
        (void)App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
#endif
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
	return;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SmartScreenShowMenu
* Description   :  智能屏表情展示
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SmartScreenShowMenu( void )
{
#ifndef SMART_SCREEN_ON
    App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
#else
    static SCREEN_SHOW_MOTION_TYPE_E showEm = (SCREEN_SHOW_MOTION_TYPE_E)0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SmartScreenShowMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_PAGE_TURN );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_8, EM_LED_ON); 
		App_LED_OutputCtrl( EM_LED_0, EM_LED_ON); 
		App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_ON); 
        //APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
		GuiQuitTimMs = GUI_TIME_60S;
        showEm = (SCREEN_SHOW_MOTION_TYPE_E)0;
	}

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW8_SCREEN_SHOW , showEm);
        MenuItem.CurMenuNum = EM_MENU_STEP_2;
		GuiQuitTimMs = GUI_TIME_60S;
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //PIR产测,猫眼返回成功
	{
	    uint8_t tp =  App_Touch_GetCurrentKeyValue();
        if(TOUCH_KEY_NO_8 == tp)
        {
            if(0 == showEm)
            {
                showEm = EM_SCREEN_FLOW0_MAX;
            }
            else
            {
                showEm--;
            }
            MenuItem.CurMenuNum = EM_MENU_STEP_1;
        }
        else if(TOUCH_KEY_BACK == tp)
        {
            MenuItem.CurMenuNum = EM_MENU_STEP_3;
        }
        else if(TOUCH_KEY_NO_0 == tp || GuiQuitTimMs < GUI_TIME_50S)
        {
            if(EM_SCREEN_FLOW0_MAX <= showEm)
            {
                showEm = (SCREEN_SHOW_MOTION_TYPE_E)0;
            }
            else
            {
                showEm++;
            }
            MenuItem.CurMenuNum = EM_MENU_STEP_1;
        }     
	}
    else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
    {
		GuiQuitTimMs = 0;
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
    }
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
#endif
	return;
}


/*********************************************************************************************************************
* Function Name :  App_GUI_BellVideoMenu()
* Description   :  门铃视频处理菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BellVideoMenu( void )		  //门铃视频处理菜单	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BellVideoMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		GuiDelayTimMs = GUI_TIME_10S;
		if(E_WAKE_CAMERA_WIFI != App_GUI_GetSysWakeupType())
		{
			(void)HAL_Voice_PlayingVoice(EM_CALLING_WAIT_HANG_UP_BY_BACK_MP3, 300);
		}
	}
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
	if(EM_MENU_STEP_1 == MenuItem.CurMenuNum)
	{
		if( E_WAKE_CAMERA_WIFI == App_GUI_GetSysWakeupType() )
		{
			 if(FaceProCallBell(0) == 1)//手机端发起
			 {
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				 return;
			 }
		}
		else if(FaceProCallBell(1) == 1) //主动按门铃
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
		uint8_t keynum = App_Touch_GetCurrentKeyValue();
		if( (TOUCH_KEY_BACK == keynum) && (FaceWifiStatus.media_state == 0x02) )  //联网模组正在工作中按返回键才可退出
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		if(TOUCH_KEY_BACK == keynum)
		{
			my_printf("Modual_State = %d\n", Modual_State);
		}
		if((TOUCH_KEY_BACK == keynum) && (Modual_State == MODUAL_ERROR))//模组没插或故障
		{
			Modual_State = MODUAL_DEAFAULT;
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
	}

	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) //断电
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
	}	
	if((GuiDelayTimMs == 0) && (Modual_State == MODUAL_ERROR))
	{
		my_printf("Modual_State = %d\n", Modual_State);
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_NetworkUpdateMenu()
* Description   :  网络同步处理
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_NetworkUpdateMenu( void )	  //网络同步处理	
{
	#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_NetworkUpdateMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ON );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_LED_OutputCtrl( EM_LED_0, EM_LED_OFF ); 
		GuiQuitTimMs = GUI_TIME_30S;
		GuiDelayTimMs = GUI_TIME_400MS;
		if( SystemPowerOnFlg == true )  //首次上电 
		{
			HAL_Voice_PlayingVoice( EM_NETWORKING_WAIT_MP3, 0 );//组网中，请稍后
		}
	}
	
	if( SystemPowerOnFlg == true )  //首次上电 
	{
		if(GuiDelayTimMs == GUI_TIME_200MS)
		{
			App_LED_OutputCtrl( EM_LED_0, EM_LED_OFF );
		}
		if(GuiDelayTimMs == 0)	
		{
			App_LED_OutputCtrl( EM_LED_0, EM_LED_ON );
			GuiDelayTimMs = GUI_TIME_400MS;
		}
	}
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )    //可被打断方式
	{
		uint8_t ret = FaceProNetworking();

		if(0 == ret)  //连接网络处理中.。。。。。
		{
			if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )        //OPEN KEY打断处理
			{
				my_printf( "break by open button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
				return;
			}
			else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  //CLOSE KEY打断处理
			{
				my_printf( "break by close button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
				MenuItem.CurMenuNum = EM_MENU_STEP_2; 
				return;
			}
			else if( 1 == App_Export_GetPinState( E_PIN_TOUCH_IRQ ) )//touch打断
			{
				my_printf( "break by touch!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_TOUCH );  
				MenuItem.CurMenuNum = EM_MENU_STEP_2;   
				return;
			}
			else if( 1 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) ) //finger打断
			{
				if( SystemPowerOnFlg == false )  //非首次上电 
				{
					my_printf( "break by finger!\n" ); 
					App_GUI_SetSysWakeupType( E_WAKE_FINGER );  
					MenuItem.CurMenuNum = EM_MENU_STEP_2;   
					return;
				}
			}
		}
		else if(1 == ret) //连接网络完成。
		{
			if( false == App_GUI_GetNetworkErrState() )  //网络连接正常
			{
				s_GuiData.NetUpdateFuncEnSts = false;
				s_GuiData.NetUpdateSetTimPara = NET_CONNECT_FIRST_TIME_S;
				s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
			}
			
			if( SystemPowerOnFlg == true )  //首次上电 
			{
				App_GUI_MenuJump( EM_MENU_WEATHER_UPDATE );	
				return;
			}
			else 
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );	
				return;
			}
		}
		else if(2 == ret) //连接网络失败。
		{
			my_printf( "gui option timeout!\n" );
			/*--------更新间隔时间----------*/
			if( s_GuiData.NetUpdateSetTimPara*2 <= 12*60*60 )
			{
				s_GuiData.NetUpdateSetTimPara *= 2;
				s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
			}
			else 
			{
				s_GuiData.NetUpdateSetTimPara = 12*60*60;
				s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
			}
			
			if( SystemPowerOnFlg == true )  //首次上电 
			{
				App_GUI_MenuJump( EM_MENU_MAIN_DESK );	
				return;
			}
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
		{
			SystemWakeupInit(); 
			return;
		}
	}
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		/*--------更新间隔时间----------*/
		if( s_GuiData.NetUpdateSetTimPara*2 <= 12*60*60 )
		{
			s_GuiData.NetUpdateSetTimPara *= 2;
			s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
		}
		else 
		{
			s_GuiData.NetUpdateSetTimPara = 12*60*60;
			s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
		}
		
		if( SystemPowerOnFlg == true )  //首次上电 
		{
			App_GUI_MenuJump( EM_MENU_MAIN_DESK );	
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
	#endif
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_WeatherUpdateMenu()
* Description   :  天气预报同步处理
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_WeatherUpdateMenu( void )	  //天气预报同步处理	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_WeatherUpdateMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			App_WIFI_CommomTx( WIFI_CMD_QUERY_CLOUD_DATA );
		}
		GuiQuitTimMs = GUI_TIME_30S;
 
	}
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
	if( SystemPowerOnFlg == true )  //首次上电 
	{
		if(GuiDelayTimMs == GUI_TIME_200MS)
		{
			App_LED_OutputCtrl( EM_LED_0, EM_LED_OFF );
		}
		if(GuiDelayTimMs == 0)	
		{
			App_LED_OutputCtrl( EM_LED_0, EM_LED_ON );
			GuiDelayTimMs = GUI_TIME_400MS;
		}
	}
	
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )    //可被打断方式
	{
		 uint8_t tp = FaceProQueryCloudData(WifiTxTemp.data, WifiTxTemp.length);
		 if(tp == 1)//成功获取天气信息
		 {
		    /* 同步设置默认表情 */
            uint32_t u32Tmp = EM_SCREEN_FLOW0_SUNNY_DAY;
            if(FaceProCloudData.Festival > 0)
            {
                switch(FaceProCloudData.Festival)
                {
                    case EM_FESTIVAL_LUNAR_1230: u32Tmp = EM_SCREEN_FLOW0_LUNAR_1230;break;
                    case EM_FESTIVAL_LUNAR_0115: u32Tmp = EM_SCREEN_FLOW0_LUNAR_0115;break;
                    case EM_FESTIVAL_LUNAR_0505: u32Tmp = EM_SCREEN_FLOW0_LUNAR_0505;break;
                    case EM_FESTIVAL_LUNAR_0707: u32Tmp = EM_SCREEN_FLOW0_LUNAR_0707;break;
                    case EM_FESTIVAL_LUNAR_0815: u32Tmp = EM_SCREEN_FLOW0_LUNAR_0815;break;
                    case EM_FESTIVAL_HOLIDAY_0101: u32Tmp = EM_SCREEN_FLOW0_HOLIDAY_0101;break;
                    case EM_FESTIVAL_HOLIDAY_0214: u32Tmp = EM_SCREEN_FLOW0_HOLIDAY_0214;break;
                    case EM_FESTIVAL_HOLIDAY_0501: u32Tmp = EM_SCREEN_FLOW0_HOLIDAY_0501;break;
                    case EM_FESTIVAL_HOLIDAY_0601: u32Tmp = EM_SCREEN_FLOW0_HOLIDAY_0601;break;
                    case EM_FESTIVAL_HOLIDAY_1001: u32Tmp = EM_SCREEN_FLOW0_HOLIDAY_1001;break;
                    case EM_FESTIVAL_HOLIDAY_1225: u32Tmp = EM_SCREEN_FLOW0_HOLIDAY_1225;break;
                    default: break;
                }
            }
            else if(FaceProCloudData.Weather > 0)
            {
                switch(FaceProCloudData.Weather)
                {
                    case EM_WEATHER_SUNNY_DAY: u32Tmp = EM_SCREEN_FLOW0_SUNNY_DAY;break;
                    case EM_WEATHER_CLOUDY_DAY: u32Tmp = EM_SCREEN_FLOW0_CLOUDY_DAY;break;
                    case EM_WEATHER_OVERCAST_DAY: u32Tmp = EM_SCREEN_FLOW0_OVERCAST_DAY;break;
                    case EM_WEATHER_RAINY_DAY: u32Tmp = EM_SCREEN_FLOW0_RAINY_DAY;break;
                    case EM_WEATHER_SNOWY_DAY: u32Tmp = EM_SCREEN_FLOW0_SNOWY_DAY;break;
                    default: break;
                }
            }
            APP_SCREEN_Operate(EM_SCREEN_APP_FLOW4_SWITCH_BASE, u32Tmp);

			s_GuiData.WeatherUpdateErrCnt =0;
			if( SystemPowerOnFlg == true )  //首次上电 
			{
				App_GUI_MenuJump( EM_MENU_MAIN_DESK );	
			}
			else 
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );	
			}
			return;
		 }
		 else if(tp == 2)
		 {
			my_printf( "the server didn't reply!\n" );
			s_GuiData.WeatherUpdateErrCnt++;
			s_GuiData.WeatherUpdatePeroidTim =60*60;
			if( SystemPowerOnFlg == true )  //首次上电 
			{
				App_GUI_MenuJump( EM_MENU_MAIN_DESK );	
			}
			else 
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );	
			}
			return;
		 }
	}
	#endif
	if( 0 == GuiQuitTimMs )  //获取天气信息失败，GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		s_GuiData.WeatherUpdateErrCnt++;
		s_GuiData.WeatherUpdatePeroidTim =60*60;
		
		if( SystemPowerOnFlg == true )  //首次上电 
		{
			App_GUI_MenuJump( EM_MENU_MAIN_DESK );	
		}
		else 
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );	
		}
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_BellLampMenu()
* Description   :  门铃灯显示处理         非人脸/虹膜锁才会进入
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BellLampMenu( void )	  	  //门铃灯显示处理	
{
	static bool autoLockCheckFlg;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BellLampMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ON );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_ON );//点亮门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
		App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		autoLockCheckFlg = false;
		GuiQuitTimMs = GUI_TIME_5S;
	}
 
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )   
	{
	   /*--------------接近感应持续检测-------*/	
		if( 1 == App_Export_GetPinState( E_PIN_SENSE_IRQ ) )      
		{
			GuiQuitTimMs = GUI_TIME_5S;
		}
		/*-----------蓝牙方式进入APP模式------*/	
		if( DRV_GetBleConnect() )    
		{
			my_printf( "wakeup by blue!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_BLE_COM );   	
			SystemWakeupInit();   
			return;
		}
	   /*--------------触摸板唤醒-------------*/	
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_BELL == tp1 )     //门铃键	
		{
			HAL_Voice_BellCtrl( true );
			#ifdef XM_CAM_FUNCTION_ON   //单猫眼
			if( 1 == App_GUI_CheckBellVideoAction() )
			{
				CAM_SendCommandStart(CAM_CMD_BELL, 0, 0);
			}
			#endif 
		}
		else if( TOUCH_KEY_NONE != tp1 )//其他键值
		{
			my_printf( "wakeup by touch keyboard!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_TOUCH );   	
			SystemWakeupInit();   
			return;
		}
	   /*--------------门内开门按键唤醒-------*/
		else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   
		{
			my_printf( "wakeup by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------门内关门按键唤醒-------*/
		else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  
		{
			my_printf( "wakeup by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
			SystemWakeupInit();   
			return;
		}
	   /*--------------指纹唤醒---------------*/
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	    
		{
			my_printf( "wakeup by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit();    
			return;
		}
	   /*---------非自动锁体自动上锁唤醒-------*/
		else if( 1 == App_GUI_GetAutoLockSts() )   
		{
			my_printf( "wakeup by not autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );   	
			return;
		}
	   /*---------自动锁体自动上锁唤醒--------*/
		else if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))   
		{
			my_printf( "wakeup by autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  	
			return;
		}
	   /*-----------触发逗留报警唤醒----------*/
		else if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
		{
			s_GuiData.StayDetectFlg = 0;
			my_printf( "wakeup by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  
			SystemWakeupInit();  
			return;
		}	
	   /*-------------把手左转唤醒------------*/
	    else if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE ) ) 
		{
			my_printf( "wakeup by left handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_LEFT_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------把手中间唤醒------------*/
	    else if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE ) ) 
		{
			my_printf( "wakeup by middle handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_MIDDLE_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------把手右转唤醒------------*/
	    else if( 1 == App_Key_GetCloseHandleSts() )            
		{
			my_printf( "wakeup by right handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_RIGHT_HANDLER );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------假锁报警唤醒------------*/
		else if( true == HAL_Motor_FalseLockWarmCheck() )      
		{
			my_printf( "wakeup by false lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------把手试玩报警唤醒--------*/
		else if( true == HAL_Motor_HandleTryForbitWarmCheck() )//把手试玩报警
		{
			my_printf( "wakeup by handle try forbit!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------门未锁报警唤醒----------*/
		else if( true == HAL_Motor_ForgetLockWarmCheck() )    //门未锁报警
		{
			my_printf( "wakeup by forget lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-----------触发布防告警唤醒----------*/
		else if( 1 == HAL_Motor_DefendActionCheck(false) )   //检测是否触发布防告警
		{
			my_printf( "auto lock pus door!\n"); 
			App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
		    SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
			if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //布防状态
			{
				my_printf( "wakeup by defend warm!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_DEPLOY_WARM );  
				SystemWakeupInit();  
				return;
			}
		}
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_WakeButSleepMenu()
* Description   :  唤醒后无感知处理   但能正常唤醒处理 
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_WakeButSleepMenu( void )	  //唤醒后无感知处理
{
	static bool autoLockCheckFlg;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_WakeButSleepMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//点亮门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
		App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		autoLockCheckFlg = false;
		GuiQuitTimMs = GUI_TIME_5S;
	}
 
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )   
	{
	   /*--------------触摸板唤醒-------------*/
		if( 1 == App_Export_GetPinState( E_PIN_TOUCH_IRQ ) )      
		{
			my_printf( "wakeup by touch keyboard!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_TOUCH );   	
			SystemWakeupInit();   
			return;
		}
		/*-----------蓝牙方式进入APP模式------*/	
		else if( DRV_GetBleConnect() )    
		{
			my_printf( "wakeup by blue!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_BLE_COM );   	
			SystemWakeupInit();   
			return;
		}
	   /*--------------门内开门按键唤醒-------*/
		else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   
		{
			my_printf( "wakeup by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------门内关门按键唤醒-------*/
		else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  
		{
			my_printf( "wakeup by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
			SystemWakeupInit();   
			return;
		}
	   /*--------------指纹唤醒---------------*/
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	    
		{
			my_printf( "wakeup by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit();    
			return;
		}
	   /*---------非自动锁体自动上锁唤醒-------*/
		else if( 1 == App_GUI_GetAutoLockSts() )   
		{
			my_printf( "wakeup by not autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );   	
			return;
		}
	   /*---------自动锁体自动上锁唤醒--------*/
		else if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))   
		{
			my_printf( "wakeup by autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  	
			return;
		}
	   /*-----------触发逗留报警唤醒----------*/
		else if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
		{
			s_GuiData.StayDetectFlg = 0;
			my_printf( "wakeup by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  
			SystemWakeupInit();  
			return;
		}	
	   /*-------------把手左转唤醒------------*/
	    else if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE ) ) 
		{
			my_printf( "wakeup by left handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_LEFT_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------把手中间唤醒------------*/
	    else if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE ) ) 
		{
			my_printf( "wakeup by middle handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_MIDDLE_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------把手右转唤醒------------*/
	    else if( 1 == App_Key_GetCloseHandleSts() )            
		{
			my_printf( "wakeup by right handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_RIGHT_HANDLER );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------假锁报警唤醒------------*/
		else if( true == HAL_Motor_FalseLockWarmCheck() )      
		{
			my_printf( "wakeup by false lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------把手试玩报警唤醒--------*/
		else if( true == HAL_Motor_HandleTryForbitWarmCheck() )//把手试玩报警
		{
			my_printf( "wakeup by handle try forbit!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------门未锁报警唤醒----------*/
		else if( true == HAL_Motor_ForgetLockWarmCheck() )    //门未锁报警
		{
			my_printf( "wakeup by forget lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-----------触发布防告警唤醒----------*/
		else if( 1 == HAL_Motor_DefendActionCheck(false) )   //检测是否触发布防告警
		{
			if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //布防状态
			{
				my_printf( "wakeup by defend warm!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_DEPLOY_WARM );  
				SystemWakeupInit();  
				return;
			}
		}
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_ErrorCheckMenu()
* Description   :  老化测试流程
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AginTestMenu( void )		  //系统老化测试
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AginTestMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
 
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S); 
		#endif
		
		GuiQuitTimMs = GUI_TIME_12S;
	}

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );       //亮屏
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_ON ); 
 
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		 uint8_t  tp1 = 'A';
		 uint16_t pageId;
		 uint8_t unlockStatus;
		(void)FaceGetVeifyState( tp1, &pageId, &unlockStatus); 
		 AgingTestTimSec = GUI_TIME_5S;
		 MenuItem.CurMenuNum = EM_MENU_STEP_2;
		#else
		MenuItem.CurMenuNum = EM_MENU_STEP_3;
		#endif
	}	
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( AgingTestTimSec == 0 )
		{
			#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
			(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S); 
			#endif	
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
        FingerAppParam_S fingerPara;
        memset((void*)&fingerPara, 0, sizeof(fingerPara));
	#ifdef FINGER_VEIN_FUNCTION_ON
		 fingerPara.emAppFlow = EM_FINGER_APP_FLOW6_AGING;
    #else
        fingerPara.emAppFlow = EM_FINGER_APP_FLOW3_SEARCH;
    #endif
		 APP_FINGER_Operate( fingerPara );   //启动验证指纹流程
		 AgingTestTimSec = GUI_TIME_2S;
		 MenuItem.CurMenuNum = EM_MENU_STEP_4;
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )
	{
		if( AgingTestTimSec == 0 )
		{
			APP_FINGER_Sleep();  //关闭指纹模组
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
		}
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )  //open door
	{
        int8_t tp1 = HAL_Motor_ForceOpenDoorThread();
        if( 1 == tp1 )      //开门流程结束
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
		}
		else if( 2 == tp1 ) //主锁舌+斜舌缩回
		{
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON ); 	
			App_LED_OutputCtrl( EM_LED_LOCK_G, EM_LED_ON );
			HAL_Voice_PlayingVoice( EM_OPEN_DOOR_OK_MP3, GUI_TIME_1500MS );
		}
		else if( -1 == tp1 )//开门方向参数异常
		{ 
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
		}
	}
	else if( EM_MENU_STEP_6 == MenuItem.CurMenuNum )  //close door
	{
		if( 1 == HAL_Voice_GetBusyState() )   
			return;
 
		#if defined FACE_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_FIRST_USE_FACE_TIPS_MP3, GUI_TIME_3500MS );		
		#elif defined IRIS_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_FIRST_USE_IRIS_TIPS_MP3, GUI_TIME_3500MS );		
		#elif defined FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_FIRST_USE_FINGER_TIPS_MP3, GUI_TIME_3500MS );		
		#elif defined FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_FIRST_USE_VEIN_TIPS_MP3, GUI_TIME_3500MS );		
		#endif
		
		MenuItem.CurMenuNum = EM_MENU_STEP_7;
	}
	else if( EM_MENU_STEP_7 == MenuItem.CurMenuNum )  //close door
	{
		if( 1 == HAL_Voice_GetBusyState() )   
			return;
		    
		int8_t tp1 = HAL_Motor_BurnTestCloseDoorThread();
        if( 1 == tp1 )      //关门流程结束
		{
			HAL_Voice_PlayingVoice( EM_LOCKED_DOOR_MP3, GUI_TIME_2S );
			MenuItem.CurMenuNum = EM_MENU_STEP_8;
		}
		else if( -1 == tp1 )//关门方向参数异常
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_8;
		}
	}
    else if( EM_MENU_STEP_8 == MenuItem.CurMenuNum )   
	{
		if( 0 == HAL_Voice_GetBusyState() )   
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_1;
		}
	}
	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GotoSleepModelMenu()
* Description   :  系统休眠菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_GotoSleepModelMenu( void )	  //系统休眠菜单
{
	static bool autoLockCheckFlg;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_GotoSleepModelMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_SLEEP );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif

		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON   //二代猫眼是否开启处理
		if( App_GUI_CheckNetworkAction() )  //执行网络同步判定
		{
			s_GuiData.NetUpdateFuncEnSts = true;
		}
		else 
		{
			s_GuiData.NetUpdateFuncEnSts = false;
			s_GuiData.NetUpdateSetTimPara= NET_CONNECT_FIRST_TIME_S; 
			s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
	    }
		#endif
		
		/*------------关闭天气同步-------------*/
		#ifdef WEATHER_FORECAST_ON  //天气同步唤醒
		uint8_t tp1 = App_GUI_CheckWeatherUpdateAction();
		if( 0 == tp1 )      //不执行同步
		{
			s_GuiData.WeatherUpdateErrCnt = 0;
			s_GuiData.WeatherUpdateEnSts =0;
		}
		else if( 1 == tp1 )  //执行同步
		{
			s_GuiData.WeatherUpdateEnSts =1;
		}				
		#endif 

        if( 1 == HAL_Motor_ReleaseTryProtectCheck())
	    {
		    App_GUI_RelieveTryProtect();  //解除禁试 
	    }
		GuiQuitTimMs = GUI_TIME_10S;
		GuiDelayTimMs = GUI_TIME_100MS;
		autoLockCheckFlg = false;
		App_GUI_SetSysWakeupType( E_WAKE_DEFAULT );                //清除唤醒源  
		
		/*--BLE初始化------*/
		SystemAppBleInit();      //需要在app_timer_init后
	}
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //等待语音播报完毕
	{
		if( (0 == HAL_Voice_GetBusyState()) || (0 == GuiDelayTimMs) )
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		/*------------------防撬解除-----------------*/	 
		if( true == App_GUI_CheckAlarmButtonRecoveryWarm() )
		{
			my_printf("alrm button is recovery!\n");
			HAL_Voice_WorkModeCtrl( false );//语音退出报警模式
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );  
		}
		
		if( 1 == App_GUI_GetAutoLockSts() )  //非自动锁体自动上锁
		{
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );   	
			return;
		}
		else if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))  //自动锁体上锁
		{
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_RelieveTryProtect();  //解除禁试
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  	
			return;
		}
		else if( 1 == HAL_Motor_DefendActionCheck(false) )   //检测是否触发布防告警
		{
			my_printf( "auto lock pus door!\n"); 
			App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
			SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
			if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //布防状态
			{
				my_printf( "break by defend warm!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_DEPLOY_WARM );  
				SystemWakeupInit();  
				return;
			}
		}
		 /*------------------触发逗留报警-------------*/	
		else if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
		{
			s_GuiData.StayDetectFlg = 0;
			my_printf( "wake by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  
			SystemWakeupInit();  
			return;
		}
		
        if( 1 == APP_WIFI_TxState() )         //确保WIFI推送完成
		{
	        if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   //门内开门按键打断处理
			{
				my_printf( "break by open button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
				SystemWakeupInit();  
				return;
			}
			else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  //门内关门按键打断处理
			{
				my_printf( "break by close button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
				SystemWakeupInit();   
				return;
			}
		    MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
	  #if !(defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON)
		if( 0 == HAL_EXPORT_PinGet( EM_KEY_IRQ ) )        //触摸按键打断处理
		{
			my_printf( "break by touch!\n" ); 
		    App_GUI_SetSysWakeupType( E_WAKE_TOUCH );  
			SystemWakeupInit();   
			return;
		}
	  #ifdef FINGER_VEIN_FUNCTION_ON
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	   //指纹打断处理
		{
			my_printf( "break by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit();    
			return;
		}
	  #endif
	    else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   //门内开门按键打断处理
		{
			my_printf( "break by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
			SystemWakeupInit();  
			return;
		}
	    else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  //门内关门按键打断处理
		{
			my_printf( "break by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
			SystemWakeupInit();   
			return;
		}
	    else if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE ) )//把手左转打断处理
		{
			my_printf( "break by left handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_LEFT_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	    else if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE ) )//把手中间打断处理
		{
			my_printf( "break by middle handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_MIDDLE_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	    else if( 1 == App_Key_GetCloseHandleSts() )            //把手右转打断处理
		{
			my_printf( "break by right handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_RIGHT_HANDLER );   					
			SystemWakeupInit();   
			return;
		}
		else if( true == HAL_Motor_FalseLockWarmCheck() )      //假锁报警
		{
			my_printf( "break by false lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );   					
			SystemWakeupInit();   
			return;
		}
		else if( true == HAL_Motor_HandleTryForbitWarmCheck() )//把手试玩报警
		{
			my_printf( "break by handle try forbit!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );   					
			SystemWakeupInit();   
			return;
		}
		else if( true == HAL_Motor_ForgetLockWarmCheck() )    //门未锁报警
		{
			my_printf( "break by forget lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	#endif
	}
    else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
    {
        if(FINGER_SleepEx())
        {
            MenuItem.CurMenuNum = EM_MENU_STEP_11;
        }
        else
        {
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit(); 
        }
    }
    else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum )  //确认防撬报警
    {
		if( true == App_Export_GetAlrmWarmState() && 1 == HAL_ADC_GetVolLowGradeErr() )//防撬打断处理 (非低压报警)
		{
			my_printf( "sleep break by alarm!\n" ); 
			App_GUI_MenuJump( EM_MENU_ALARM_WARM );
			App_GUI_SetSysWakeupType( E_WAKE_ALARM );   	
			return;
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_4;
    }
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )  //系统休眠
	{
        APP_SCREEN_Sleep();
		SystemSleepInit();              //外设休眠 
		App_GUI_SetSysSleepSts( true ); //系统休眠
		MenuItem.CurMenuNum = EM_MENU_STEP_5;
		my_printf("/*---------------sleepping--------------------*/\r\n");	
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )
	{
		#if LOCK_PROJECT_CHIP == LOCK_PROJECT_RTL8762
		if( App_GUI_GetSysSleepSts()==0 ) //可能还会进这个STEP
		{
			Dlps_Enabled=false; //唤醒
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
		}
		else
		{
			DRV_ConncetCallBackReg(BleISRhandler); //注册蓝牙服务程序		
			DRV_SysMgmt_Run();	//使能睡眠
		}
		#else
		while( App_GUI_GetSysSleepSts() )  
		{
			DRV_SysMgmt_Run();	
			if(DRV_GetBleConnect())   //确认是否为蓝牙唤醒
			{
				if( E_WAKE_DEFAULT == App_GUI_GetSysWakeupType() )
				{
					App_GUI_SetSysSleepSts(false);
					App_GUI_SetSysWakeupType( E_WAKE_BLE_COM );   //设定唤醒源
					my_printf( "wake up source is ble!\n" ); 
				}
			}
		}
		SystemWakeupInit();  //系统唤醒处理 
		#ifdef SMARTKEY_XRO_ENC_ON  //变更广播
		if( E_WAKE_BLE_COM != App_GUI_GetSysWakeupType() )  //不是蓝牙唤醒 
		{
			if(SystemSeting.SystemAdminRegister==ADMIN_APP_REGISTERED)//已注册
			{
				APP_BleInit(REGISTERED,LIMITED_FLAGS); //广播改成020105 白色钥匙用
			}
		}
		#endif
		return;
		#endif	
	}
	else if( EM_MENU_STEP_6 == MenuItem.CurMenuNum )
	{
#if LOCK_PROJECT_CHIP == LOCK_PROJECT_RTL8762
		DRV_ConncetCallBackReg(0); //蓝牙服务程序关闭
#endif	
		SystemWakeupInit();  //系统唤醒处理 
	}
    return;
}
 

/*********************************************************************************************************************
* Function Name :  App_GUI_AddUserMenu()
* Description   :  增加用户菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddUserMenu( void )		  //增加用户菜单
{
    uint8_t addAdminFingerkey = 0xF0;
	uint8_t addGuestFingerkey = 0xF0;
	uint8_t delFingerkey      = 0xF0;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddUserMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		#if defined  FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_ADD_USER_MENU_MP3, 0 );
		#elif defined  FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
		#endif
		
		GuiQuitTimMs = GUI_TIME_26S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )      //增加管理员指纹
		{
		    if( SystemSeting.SysFingerAdminNum >= MSG_FINGER_ADMIN_LOCAL_NUM )       //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_FINGER_ADD_ADMIN );
				return;	
			}
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //增加普通用户指纹
		{
		    if( SystemSeting.SysFingerGuestNum >= MSG_FINGER_COMMON_LOCAL_GUINUM )   //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_FINGER_ADD_GUEST );
				return;	
			}
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //增加卡
		{
			if( SystemSeting.SysCardAllNum >= MSG_CPU_CARD_USER_NUM )   //登记数量已满
			{
		        HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, GUI_TIME_1200MS );	
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
			}
			else 
			{
				App_GUI_MenuJump( EM_MENU_CARD_ADD );
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			DispFuncPtrPre = NULL;
			return;
		}
	}
	else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_FINGER_ADD_ADMIN );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DeleteUser()
* Description   :  删除用户菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DeleteUser( void )		  //删除用户菜单
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DeleteUser()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		#if defined  FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_DELETE_USER_MENU_MP3, 0 );
		#elif defined  FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
		#endif
		
		GuiQuitTimMs = GUI_TIME_26S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )      // 删除普通用户指纹
		{
			App_GUI_MenuJump( EM_MENU_FINGER_DELETE_GUEST );
			return;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //删除卡
		{
			App_GUI_MenuJump( EM_MENU_CARD_DELETE );
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) 
	{

	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DeleteUser()
* Description   :  系统时间设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SystemTimeSetting( void )		  //系统时间设置菜单
{
    uint8_t  buflen = 0;
	static uint8_t  inputBuf[KEY_BUF_SIZE] = {0}; 
    static uint8_t  pwdBuf[2][MSG_PWD_BYTE_SIZE] = {0};
	static RTCType rtc_w;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SystemTimeSetting()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		#if defined  FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_SYSTEM_DATE_SET_MENU_MP3, 0 );
		#elif defined  FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
		#endif

		
		GuiQuitTimMs = GUI_TIME_50S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //输入年月日8位数字
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
			if( tp1 == 8 )//输入格式正确的年月日
			{
				App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );

				HAL_Voice_PlayVoiceNum(EM_CURRENT_DATE_MP3);
				PUBLIC_Delayms(2000);
				HAL_Voice_PlayVoiceNum(EM_NUM_0_MP3 + inputBuf[0]);
				PUBLIC_Delayms(400);
				HAL_Voice_PlayVoiceNum(EM_NUM_0_MP3 + inputBuf[1]);
				PUBLIC_Delayms(400);
				HAL_Voice_PlayVoiceNum(EM_NUM_0_MP3 + inputBuf[2]);
				PUBLIC_Delayms(400);
				HAL_Voice_PlayVoiceNum(EM_NUM_0_MP3 + inputBuf[3]);
				PUBLIC_Delayms(400);
				HAL_Voice_PlayVoiceNum(EM_YEAR_MP3);
				PUBLIC_Delayms(400);
				if((inputBuf[4] == 1) && (inputBuf[5] == 0))
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
				}
				else if(inputBuf[4] == 1)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[5] );
					PUBLIC_Delayms(400);
				}
				else
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[5] );
					PUBLIC_Delayms(400);
				}
				HAL_Voice_PlayVoiceNum( EM_MONTH_MP3 );//月
				PUBLIC_Delayms(400);
				if(inputBuf[6] == 1)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
				}
				else if(inputBuf[6] > 1)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[6] );
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
				}
				if(inputBuf[7] != 0)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[7] );
					PUBLIC_Delayms(400);
				}
				HAL_Voice_PlayVoiceNum( EM_DAY_MP3);//日
				PUBLIC_Delayms(400);
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
				HAL_Voice_PlayingVoice( EM_CHANGE_PRESS_RETURN_KEY_MP3, 0);/*---确认请按#键,如需更改请按返回键---*/
			}
			else//语音提示“格式不正确”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				MenuItem.CurMenuNum = EM_MENU_STEP_5;
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //确认再输入时分秒6位数字
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			rtc_w.year =(inputBuf[2]<<4)|inputBuf[3];
            rtc_w.month =(inputBuf[4]<<4)|inputBuf[5];
            rtc_w.day = (inputBuf[6]<<4)|inputBuf[7];
			HAL_Voice_PlayingVoice( EM_SYSTEM_VOICE_PROMPTS_TIME_MP3, 0);//请输入时分秒6位数字,按确认键结束
			GuiQuitTimMs = GUI_TIME_50S;
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    DispFuncPtrPre = NULL; //重新来一次
			return;		
		}
		
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) //输入时分秒6位数字
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( &inputBuf[8], &buflen ); 
			if( tp1 == 6 )//时分秒6位数字
			{
				App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );

				HAL_Voice_PlayVoiceNum(EM_CURRENT_TIME_MP3);
				PUBLIC_Delayms(2000);
				if(inputBuf[8] != 0) 
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[8]);
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[9]);
					PUBLIC_Delayms(400);
				}
				else if(inputBuf[9] != 0)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[9] );
					PUBLIC_Delayms(400);
				}
				else
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3);
					PUBLIC_Delayms(400);
				}
				HAL_Voice_PlayVoiceNum( EM_HOUR_MP3 );//时
				PUBLIC_Delayms(400);
				if(inputBuf[10] != 0) 
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[10]);
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[11]);
					PUBLIC_Delayms(400);
				}
				else if(inputBuf[11] != 0)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[11] );
					PUBLIC_Delayms(400);
				}
				else
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3);
					PUBLIC_Delayms(400);
				}
				HAL_Voice_PlayVoiceNum( EM_MINUTER_MP3 );//分
				PUBLIC_Delayms(400);
				if(inputBuf[12] != 0) 
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[12]);
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_10_MP3 );
					PUBLIC_Delayms(400);
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[13]);
					PUBLIC_Delayms(400);
				}
				else if(inputBuf[13] != 0)
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3 + inputBuf[13] );
					PUBLIC_Delayms(400);
				}
				else
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_0_MP3);
					PUBLIC_Delayms(400);
				}
				HAL_Voice_PlayVoiceNum( EM_SECOND_MP3 );//秒
				PUBLIC_Delayms(400);
				HAL_Voice_PlayingVoice( EM_CHANGE_PRESS_RETURN_KEY_MP3, GUI_TIME_5S);/*---确认请按#键,如需更改请按返回键---*/
				MenuItem.CurMenuNum = EM_MENU_STEP_4;
			}
			else//语音提示“格式不正确，请重新输入”
			{
				PUBLIC_Delayms(400);
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1500MS);
				GuiQuitTimMs = GUI_TIME_50S;
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
				return;
			}
		}
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum ) //确认时间
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			rtc_w.hour =(inputBuf[8]<<4)|inputBuf[9];
            rtc_w.minuter =(inputBuf[10]<<4)|inputBuf[11];
            rtc_w.second = (inputBuf[12]<<4)|inputBuf[13];
			rtc_w.week = 1;
			HAL_RTC_WriteTime(&rtc_w);
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS);//设定成功
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) 	
		{
		    HAL_Voice_PlayingVoice( EM_SYSTEM_VOICE_PROMPTS_TIME_MP3, 0);//请输入时分秒6位数字,按确认键结束
			GuiQuitTimMs = GUI_TIME_50S;
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;		
		}
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum ) //语音播报延时
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
        if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
		else if( 0 == HAL_Voice_GetBusyState() )
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;	
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DeleteUser()
* Description   :  开锁方式设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_UnlockWaySetting( void )		  //开锁方式设置菜单
{
    uint8_t addAdminFingerkey = 0xF0;
	uint8_t addGuestFingerkey = 0xF0;
	uint8_t delFingerkey      = 0xF0;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SystemTimeSetting()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		#if defined  FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_UNLOCK_WAY_MENU_MP3, 0 );
		#elif defined  FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
		#endif
		
		GuiQuitTimMs = GUI_TIME_26S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 ) // 指纹开锁
		{
			SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_ON; 
		    SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //写组合开门
			SystemSeting.SysLockMode = 0x02; //第二个字节是模式
			SystemWriteSeting(&SystemSeting.SysLockMode,1);
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) // 指纹或卡或密码开锁
		{
			SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_OFF; 
		    SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //写组合开门
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) // 指纹加密码开锁
		{
			SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_ON; 
		    SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //写组合开门
			SystemSeting.SysLockMode = 0x03; //第二个字节是模式
			SystemWriteSeting(&SystemSeting.SysLockMode,1);
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{		
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_VoiceSetting()
* Description   :  语音设置菜单
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_VoiceSetting( void )		  //语音设置菜单
{
    uint8_t addAdminFingerkey = 0xF0;
	uint8_t addGuestFingerkey = 0xF0;
	uint8_t delFingerkey      = 0xF0;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_VoiceSetting()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		#if defined  FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VOICE_SET_MENU_MP3, 0 );
		#elif defined  FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
		#endif
		
		GuiQuitTimMs = GUI_TIME_26S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 ) // 低音量
		{
			SystemSeting.SysVoice=LOW_VOICE_VOL; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //中音量
		{
			SystemSeting.SysVoice=MEDIUM_VOICE_VOL; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //高音量
		{
			SystemSeting.SysVoice=HIGH_VOICE_VOL; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{		
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_AddCardMenu()
* Description   :  增加卡
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddCardMenu( void )	  //增加卡
{
	uint8_t  addIdlen = 0;
	static uint8_t ADD_ID[3] = {0};
	static uint16_t addId = 0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddCardMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ON ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //请输入3位数字编号,按确认键确认
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//输入格式正确3位数字编号
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//编号已存在请重新输入 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//请刷卡 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//语音提示“格式不正确请重新输入”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //重新来一次
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		uint8_t result = CpuCardEnrollPro(0, addId);
		if(result == CPUCARD_ADD_SUCCESSFUL)// 添加成功
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_2S );//登记成功
			MenuItem.CurMenuNum = EM_MENU_STEP_3;		
		}
		else if(result == CPUCARD_ADD_REGISTERED)// 已注册
		{
			HAL_Voice_PlayingVoice( EM_CARD_REGISTED_MP3, GUI_TIME_2S );//卡已注册   
			MenuItem.CurMenuNum = EM_MENU_STEP_3;			
		}
		else if(result == CPUCARD_ADD_ERROR)// 添加失败
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_2S );//登记失败
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_ADD_USER );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFaceMenu()
* Description   :  删除卡
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelCardMenu( void )			  //删除卡
{
	uint8_t  addIdlen = 0;
	static uint8_t ADD_ID[3] = {0};
	static uint16_t addId = 0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DelCardMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ON );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		HAL_Voice_PlayingVoice( EM_DEL_TYPE_OPT_MP3, GUI_TIME_4S );//删除类型选择提示( 编号删除请按1,比对删除请按2 ) 
		#if defined FACE_FUNCTION_ON
			SystemEventLogSave( DELETE_FACE, 0);
		#elif defined IRIS_FUNCTION_ON
			SystemEventLogSave( DELETE_IRIS, 0);
		#endif 
		GuiQuitTimMs = GUI_TIME_30S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 ) //编号删除
		{
			HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //请输入3位数字编号,按确认键确认
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //对比删除
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//请刷卡 
			App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//输入格式正确3位数字编号
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(0 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_INEXISTS_MP3);//编号不存在 
					PUBLIC_Delayms(2000);
					DispFuncPtrPre = NULL; //重新来一次
					return;
				}
				else
				{
					if(CpuCardDeleteID(addId))
					{
						HAL_Voice_PlayingVoice( EM_DEL_SUCCESS_MP3, GUI_TIME_1500MS );//删除成功	 
					}			
					else
					{
						HAL_Voice_PlayingVoice( EM_DEL_FAIL_MP3, GUI_TIME_1500MS );//删除失败	 
					}
					MenuItem.CurMenuNum = EM_MENU_STEP_4;
				}
			}
			else//语音提示“格式不正确请重新输入”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr;
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		uint8_t temp = CpuCardDeleteComparison();
		if(temp == 1)
		{
			HAL_Voice_PlayingVoice(EM_DEL_SUCCESS_MP3, 1200); //删除成功
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}
		else if(temp == 2)
		{
			HAL_Voice_PlayingVoice(EM_DEL_FAIL_MP3, 1200); //删除失败	
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}
		if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;		
		}
	}
	if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_DELETE_USER );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckCardErrMenu()
* Description   :  卡验证失败
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckCardErrMenu( void )	  //卡验证失败
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckCardErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			HAL_Voice_PlayingVoice( EM_CHECK_ERR_ALARM_MP3, GUI_TIME_4S );	
		}
		else
		{
			HAL_Voice_PlayingVoice( EM_CHECK_FAIL_MP3, GUI_TIME_1500MS );	
		}
		GuiQuitTimMs = GUI_TIME_2S;
	}

    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //禁试
		{
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
			TryAlarmFirst = EM_TRY_DEFAULT;
			#endif
			SystemSeting.TryForbitUtc = Rtc_Real_Time.timestamp;
			(void)SystemWriteSeting( (uint8_t *)&SystemSeting.TryForbitUtc, sizeof SystemSeting.TryForbitUtc );	
		    if( true == App_GUI_GetWifiWarmingSwSts() )
			{ 
				WifiLockMeg.AlarmMeg = FORBID_TRY;
				App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
			}
			SystemEventLogSave( TRY_OPRN_ALARM, 0 );
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else 
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
 
	if( 0 == GuiQuitTimMs )
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		
		if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
		{
			App_GUI_MenuJump( EM_MENU_TRY_PROTECT );
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFingerGuestMenu()
* Description   :  删除普通用户指纹
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelFingerGuestMenu( void )         //删除普通用户指纹
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DelFingerGuestMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		HAL_Voice_PlayingVoice( EM_DEL_TYPE_OPT_MP3, GUI_TIME_4S );//删除类型选择提示( 编号删除请按1,比对删除请按2 ) 
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum ) 
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 ) //编号删除
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //对比删除
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{

	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}

}

/*********************************************************************************************************************
* Function Name :  App_GUI_SEMI_AddAdminFingerMenu()
* Description   :  增加管理员指纹
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SEMI_AddAdminFingerMenu( void )	  //增加管理员指纹
{
	uint8_t  addIdlen = 0;
	static uint8_t ADD_ID[3] = {0};
	static uint16_t addId = 0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddCardMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ON ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //请输入3位数字编号,按确认键确认
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//输入格式正确3位数字编号
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//编号已存在请重新输入 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//请刷卡 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//语音提示“格式不正确请重新输入”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //重新来一次
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		uint8_t result = CpuCardEnrollPro(0, addId);
		if(result == CPUCARD_ADD_SUCCESSFUL)// 添加成功
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_2S );//登记成功
			MenuItem.CurMenuNum = EM_MENU_STEP_3;		
		}
		else if(result == CPUCARD_ADD_REGISTERED)// 已注册
		{
			HAL_Voice_PlayingVoice( EM_CARD_REGISTED_MP3, GUI_TIME_2S );//卡已注册   
			MenuItem.CurMenuNum = EM_MENU_STEP_3;			
		}
		else if(result == CPUCARD_ADD_ERROR)// 添加失败
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_2S );//登记失败
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_ADD_USER );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SEMI_AddGuestFingerMenu()
* Description   :  增加普通用户指纹
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SEMI_AddGuestFingerMenu( void )	  //增加普通用户指纹
{
	uint8_t  addIdlen = 0;
	static uint8_t ADD_ID[3] = {0};
	static uint16_t addId = 0;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddCardMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_ON ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//关掉门铃灯
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //关掉锁门键白灯
		#endif
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //请输入3位数字编号,按确认键确认
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //确认键
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//输入格式正确3位数字编号
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//编号已存在请重新输入 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//请刷卡 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//语音提示“格式不正确请重新输入”
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //重新来一次
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		uint8_t result = CpuCardEnrollPro(0, addId);
		if(result == CPUCARD_ADD_SUCCESSFUL)// 添加成功
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_2S );//登记成功
			MenuItem.CurMenuNum = EM_MENU_STEP_3;		
		}
		else if(result == CPUCARD_ADD_REGISTERED)// 已注册
		{
			HAL_Voice_PlayingVoice( EM_CARD_REGISTED_MP3, GUI_TIME_2S );//卡已注册   
			MenuItem.CurMenuNum = EM_MENU_STEP_3;			
		}
		else if(result == CPUCARD_ADD_ERROR)// 添加失败
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_2S );//登记失败
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		if( TOUCH_KEY_BACK == tp1 ) //返回按键
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //语音播报完毕
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI超时退出
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_ADD_USER );
	}
}
const KdbTabType_T DispTab[] =
{
   /*-------桌面菜单（一级菜单）-----------*/
    {EM_MENU_MAIN_DESK, 1, (*App_GUI_MainDeskMenu)},          //系统桌面菜单

   /*-------系统管理菜单-------*/
	{EM_MENU_SYSTEM_MANAGE, 1, (*App_GUI_SystemManageMenu)},  //系统管理菜单
	{EM_MENU_CHECK_ADMIN, 1, (*App_GUI_CheckAdminMenu)},      //验证管理员权限
   
   /*-------电机动作菜单-------*/
	{EM_MENU_OPEN_DOOR, 1, (*App_GUI_OpenDoorMenu)},          //执行开门流程
	{EM_MENU_CLOSE_DOOR, 1, (*App_GUI_CloseDoorMenu)},        //执行关门流程
	
   /*-------人脸类菜单---------*/
	{EM_MENU_SET_FACE_MENU, 1, (*App_GUI_SetFaceMenu)},          //人脸设置菜单
	{EM_MENU_FACE_ADD_ADMIN, 1, (*App_GUI_AddAdminFaceMenu)},    //增加管理员人脸
	{EM_MENU_FACE_ADD_GUEST, 1, (*App_GUI_AddGuestFaceMenu)},    //增加普通用户人脸
	{EM_MENU_FACE_DELETE, 1, (*App_GUI_DelFaceMenu)},            //删除人脸
	{EM_MENU_FACE_CHECK_ERR, 1, (*App_GUI_CheckFaceErrMenu)},    //人脸验证失败
	
   /*-------指纹类菜单---------*/
	{EM_MENU_SET_FINGER_MENU, 1, (*App_GUI_SetFingerMenu)},      //指纹设置菜单
	{EM_MENU_FINGER_ADD_ADMIN, 1, (*App_GUI_AddAdminFingerMenu)},//增加管理员指纹
	{EM_MENU_FINGER_ADD_GUEST, 1, (*App_GUI_AddGuestFingerMenu)},//增加普通用户指纹
	{EM_MENU_FINGER_DELETE, 1, (*App_GUI_DelFingerMenu)},        //删除指纹
	{EM_MENU_FINGER_CHECK_ERR, 1, (*App_GUI_CheckFingerErrMenu)},//指纹验证失败
 
   /*-------密码类菜单---------*/
	{EM_MENU_SET_PWD_MENU, 1, (*App_GUI_SetPwdMenu)},   	    //密码设置菜单
	{EM_MENU_CHANGE_PWD, 1, (*App_GUI_ChangePwdMenu)},          //修改密码
	{EM_MENU_PWD_CHECK_ERR, 1, (*App_GUI_CheckPwdErrMenu)},     //密码验证失败
	
   /*-------告警类菜单---------*/
	{EM_MENU_BAT_UNWORK, 1, (*App_GUI_BatVolLowErrMenu)},       //电压低无法工作
	{EM_MENU_EEPROM_ERR, 1, (*App_GUI_EepromErrorMenu)},        //EEPROM故障
	{EM_MENU_ALARM_WARM, 1, (*App_GUI_AlarmHandlerMenu)},       //防撬告警处理
	{EM_MENU_TRY_PROTECT, 1, (*App_GUI_TryProtectMenu)},        //禁试保护告警
	{EM_MENU_STAY_WARM, 1, (*App_GUI_StayWarmMenu)},            //逗留保护告警
	{EM_MENU_DEPLAY_WARM, 1, (*App_GUI_DeployWarmMenu)},        //布防保护告警
	{EM_MENU_FALSE_LOCK_WARM, 1,(*App_GUI_FalseLockWarmMenu)},  //假锁告警
	{EM_MENU_FORGET_LOCK_WARM, 1,(*App_GUI_ForgetLockWarmMenu)},//门未关告警
	{EM_MENU_BLE_OPEN_ERR, 1,(*App_GUI_BleOpenErrWarmMenu)},    //蓝牙开门失败
 
   /*-------模式类菜单---------*/
	{EM_MENU_BACK_FACTORY, 1, (*App_GUI_BackFactoryMenu)},      //恢复出厂设置
	{EM_MENU_OTA_MODEL, 1, (*App_GUI_GotoOtaModelMenu)},        //升级模式
	{EM_MENU_APP_MODEL, 1, (*App_GUI_GotoAppModelMenu)},        //APP设置模式
	
   /*-------系统设置类菜单-----*/
    {EM_MENU_SYSTEM_PARA_SET, 1, (*App_GUI_SetSysParaMenu)},        //系统参数设置
	{EM_MENU_MOTOR_DIR_SET, 1, (*App_GUI_SetMotorDirMenu)},         //电机方向设置
	{EM_MENU_MOTOR_TORSION_SET, 1, (*App_GUI_SetMotorTorsionMenu)}, //电机扭力设置
	{EM_MENU_AUTO_LOCK_SET, 1, (*App_GUI_SetAutoLockMenu)}, 		//自动上锁设置
	{EM_MENU_DOUBLE_CHECK_SET, 1, (*App_GUI_SetDoubleCheckMenu)},   //双重认证设置
	{EM_MENU_VOL_ADJUST_SET, 1, (*App_GUI_SetVolGradeMenu)},        //音量调节设置
	{EM_MENU_NEAR_SENSE_SET, 1, (*App_GUI_SetNearSenseMenu)}, 		//接近感应设置
	{EM_MENU_DEPPOY_SET, 1, (*App_GUI_SetDeployMenu)},   			//布防设置
	{EM_MENU_STAY_CHECK_SET, 1, (*App_GUI_SetStayCheckMenu)},       //逗留设置
 
   /*-------其他类菜单---------*/
	{EM_MENU_ERROR_CHECK, 1, (*App_GUI_ErrorCheckMenu)},      //系统故障检测
	{EM_MENU_SMART_SCREEN_SHOW, 1, (*App_GUI_SmartScreenShowMenu)},      //系统故障检测
	{EM_MENU_BELL_VIDEO, 1, (*App_GUI_BellVideoMenu)},        //门铃视频处理
	{EM_MENU_NETWORK_UPDATE, 1,(*App_GUI_NetworkUpdateMenu)}, //网络同步处理
	{EM_MENU_WEATHER_UPDATE, 1,(*App_GUI_WeatherUpdateMenu)}, //天气同步处理
	{EM_MENU_BELL_LAMP_DISPLAY, 1,(*App_GUI_BellLampMenu)},   //门铃灯显示处理
	{EM_MENU_WAKEUP_BUT_SLEEP, 1,(*App_GUI_WakeButSleepMenu)},//唤醒后无感知处理
	
	/*-------二级菜单（半自动款）---------*/
	{EM_MENU_ADD_USER, 1, (*App_GUI_AddUserMenu)},      //增加用户菜单
	{EM_MENU_DELETE_USER, 1, (*App_GUI_DeleteUser)},		// 删除用户菜单
	{EM_MENU_SYSTEM_TIME_SETTING, 1, (*App_GUI_SystemTimeSetting)},		//系统时间设置菜单
	{EM_MENU_UNLOCK_WAY_SETTING, 1, (*App_GUI_UnlockWaySetting)},		// 开锁方式设置菜单
	{EM_MENU_VOICE_SETTING, 1, (*App_GUI_VoiceSetting)},		//语音设置菜单
	
	/*-------三级菜单(半自动款)---------*/
	{EM_MENU_FINGER_DELETE_GUEST, 1, (*App_GUI_DelFingerGuestMenu)},        //删除普通用户指纹
	
	/*-------IC卡菜单---------*/
    {EM_MENU_CARD_ADD, 1, (*App_GUI_AddCardMenu)},          //增加卡
	{EM_MENU_CARD_DELETE, 1, (*App_GUI_DelCardMenu)},            //删除卡
	{EM_MENU_CARD_CHECK_ERR, 1, (*App_GUI_CheckCardErrMenu)},         //卡验证失败
	
   /*-------老化测试菜单-------*/
	{EN_MENU_AGING_TEST, 1, (*App_GUI_AginTestMenu)},      	  //系统老化测试
	
   /*--------休眠菜单---------*/
	{MENU_SYSTEM_SLEEP, 1, (*App_GUI_GotoSleepModelMenu)},    //系统休眠菜单
	
}; 

/*********************************************************************************************************************
* Function Name :  App_GUI_Scheduler()
* Description   :  GUI主流程的调度功能函数
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_Scheduler( void )
{
	if( MenuItem.MenuIndexType.Currently >= MENU_NULL )
	{
		return;
	}
	DispFuncPtr = DispTab[MenuItem.MenuIndexType.Currently].CurrentOperate;
	(*DispFuncPtr)();
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_MenuProcess()
* Description   :  任务调度
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_GUI_MenuProcess( void )
{
	switch( App_GUI_GetSystemWorkSts() )
	{
		case E_SYSTEM_SELFCHECK:  //系统自检  决定GUI走向
                if( 1 == SystemSelfCheck() )
				{
					App_GUI_SetSystemWorkSts( E_SYSTEM_VOICE_CFG );
					my_printf("SystemSelfCheck=%d\n", SystemTick);
					my_printf( "SystemSelfCheck()\n" ); 
				}
		break;
		
		case E_SYSTEM_VOICE_CFG:  //确保语音芯片稳定可设置
				if( SystemWorkHoldTim >= SYS_STEADY_TIMER_MS )  
				{
				   if( E_WAKE_ALARM_BREAK != App_GUI_GetSysWakeupType() )    //中断防撬唤醒
				   {
					   HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
				   }
				   App_GUI_SetSystemWorkSts( E_SYSTEM_WORKING );
				}
		break;		
				
		case E_SYSTEM_WORKING:  //执行GUI
		        App_GUI_Scheduler();
		break;
		
		default:break;
	}
}
 
 
 
 
 
/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
