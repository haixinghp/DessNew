/********************************************************************************************************************
 * @file:      App_GUI.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-02
 * @brief:     ϵͳ�����̺���  
 * @Description:   
 * @ChangeList:  01. ����
*********************************************************************************************************************/
  
/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
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
/*-------------------------------------------------�궨��-----------------------------------------------------------*/
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
 
#define  STAY_DETECT_TIME_S          30     //����������ʱ��       ��λ ��
#define  PUSAL_WORK_TIME_SHORT_S     15     //���⿪�ųɹ���ӽ���Ӧ��ͣ����ʱ��   ��λ ��
#define  PUSAL_WORK_TIME_LONG_S      30     //���ڿ��ųɹ���ӽ���Ӧ��ͣ����ʱ��   ��λ ��
#define  NET_CONNECT_FIRST_TIME_S    60		//���������ĳ�ʼ���ʱ�� ��λ ��
/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/
typedef enum
{
	E_CHECK_DEFAULT, 
	E_CHECK_FACE, 
	E_CHECK_FINGER, 
	
}CHECK_KEY_TYPE_E;

/*-------------------------------------------------��������---------------------------------------------------------*/

/*-------------------------------------------------ȫ�ֱ�������-----------------------------------------------------*/         
bool  FingerWorkState = false;

uint8_t UploadUnlockDoorMegEnable = 0;
/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/
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
	uint32_t AlamButtonHoldTimSec;	 //���˱�������ʱ���ʱ��ʱ��
	bool     AlamButtonNewTrigger;	 //������һ�δ���
}AlarmMeg_S;

typedef struct
{
	uint8_t  TouchLockNum;       //�������ż����ż�����
    uint8_t  SystemSleepState;   //ϵͳ����״̬        1:����   0:����
	uint8_t  AutoLockTimPara;    //�Զ�����ʱ��        ��λ��
	uint8_t  StayDetectTimPara;  //�������澯ʱ��    ��λ��
	uint8_t  DoorCurState;       //������ǰ״̬        0: default  1:open  2:close 
	uint8_t  NearSensePusalTim;  //�����Ӧ�����ͣʱ��    ��λ��
	uint32_t NetUpdatePeroidTim; //����ͬ�����ʱ��        ��λ��
    uint32_t NetUpdateSetTimPara;//����ͬ�����õļ��ʱ��  ��λ��
    bool     NetUpdateFuncEnSts; //����ͬ������״̬    1:ʹ��   0:ʧ��
 
    uint16_t WeatherUpdatePeroidTim; //����ͬ�����ʱ��        ��λ��
    uint16_t WeatherUpdateSetTimPara;//����ͬ�����õļ��ʱ��  ��λ��
    uint8_t  WeatherUpdateErrCnt; 	 //����ͬ���������    
	uint8_t  WeatherUpdateEnSts; 	 //����ͬ��ʹ��״̬      1:ʹ��   0:ʧ��
	
    bool  StayDetectFlg;           //��������Ƿ񴥷���   0:δ����  1:�Ѵ���
	bool  StaySenseCheckEn;        //������⴫����       0:�����  1:���
	
	OPEN_MODEL_E   OpenDoorModel;  //���ŷ�ʽ
    CLOSE_MODEL_E  CloseDoorModel; //���ŷ�ʽ
	
	WAKEUP_TYPE_E  WakeupSourceType;  //ϵͳ�Ļ�������
	
    WORK_MODE_E    SysWorkMode;       //ϵͳ����ģʽ
	
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
/*-------------------------------------------------��������---------------------------------------------------------*/


/*-------------------------------------------------��������---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_GUI_FileInit()
* Description   :  ��س�ʼ��
* Para          :  ��
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

	(void)DRV_InterGenerateRandVec( (uint8_t *)&s_GuiData.WeatherUpdateSetTimPara, 2 );//�����2��
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
* Description   :  ���Ѻ�����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
void App_GUI_WakeupInit( void )
{
	if( (E_WAKE_FINGER == App_GUI_GetSysWakeupType()) ||
		(E_WAKE_NEAR_SENSE == App_GUI_GetSysWakeupType()) ||
		(E_WAKE_TOUCH == App_GUI_GetSysWakeupType()) )   //ǰ��廽��
	{
		if(SystemSeting.SysHumanIrDef == 0)
		{
			s_GuiData.StayDetectTimPara = 0;  //������⸳ֵ 
		}
		else if(s_GuiData.StayDetectTimPara == 0)
		{
			s_GuiData.StayDetectTimPara = SystemSeting.SysHumanIrDef;  //������⸳ֵ
			my_printf("s_GuiData.StayDetectTimPara = %d\n", SystemSeting.SysHumanIrDef);
		}
	}
	s_GuiData.OpenDoorModel = EM_OPEN_DEFAULT;
	s_GuiData.CloseDoorModel= EM_CLOSE_DEFAULT;
	if( E_WAKE_AUTO_LOCK != App_GUI_GetSysWakeupType() )      //�Զ�������������ڿ��ź�����ͣ������ʱ��
	{
	    s_GuiData.NearSensePusalTim = 0;
	}
	
	(void)DRV_InterGenerateRandVec( (uint8_t *)&s_GuiData.WeatherUpdateSetTimPara, 2 );//�����2��
	my_printf( "WeatherUpdateSetTimPara= %d!\n", s_GuiData.WeatherUpdateSetTimPara ); 
	s_GuiData.WeatherUpdateSetTimPara %= 7200;
	
	UploadUnlockDoorMegEnable = 0;
	
	App_GUI_MenuJump( MENU_NULL );	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SleepInit()
* Description   :  ���ߺ�����
* Para          :  ��
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
* Description   :  ����ϵͳ����״̬
* Input  Para   :  workstate- ϵͳ����״̬
* Output Para   :  none
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetSystemWorkSts( SYSTEM_WORK_STS_E workstate )
{
    GuiSchedulerStep = workstate;
} 

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSystemWorkSts()
* Description   :  ��ȡϵͳ����״̬
* Input  Para   :  none
* Output Para   :  none
* Return        :  SYSTEM_WORK_STS_E- ����״̬
*********************************************************************************************************************/
SYSTEM_WORK_STS_E App_GUI_GetSystemWorkSts( void )
{
	return GuiSchedulerStep;
} 

/*********************************************************************************************************************
* Function Name :  App_GUI_StopStayDetectTim()
* Description   :  ֹͣ���ζ�����ʱ
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
void App_GUI_StopStayDetectTim(void)
{
	s_GuiData.StayDetectTimPara = 0;  //������⸳ֵ 
	s_GuiData.StayDetectFlg = 0;	//��������Ƿ񴥷���   0:δ����  1:�Ѵ���
	s_GuiData.StaySenseCheckEn = false;
}
/*********************************************************************************************************************
* Function Name :  App_GUI_Tim10Ms()
* Description   :  ��ض�ʱ��  10msִ��һ��
* Para          :  ��
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
		
	   /*-------�ӽ���Ӧ��ͣ����ʱ��------*/
		if( s_GuiData.NearSensePusalTim > 0 )
			s_GuiData.NearSensePusalTim--;
		
		if( BleDisconnectHoldTimeMs > 0 )
			BleDisconnectHoldTimeMs--;
		
	   /*-------����ͬ������ʱ��---------*/
		if( s_GuiData.WeatherUpdatePeroidTim )
		{
			s_GuiData.WeatherUpdatePeroidTim--;
		} 
		
//		my_printf( "GuiQuitTimMs = %d\n", GuiQuitTimMs ); 
		
			/*-----------�������-------------*/
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
* Description   :  ���ⶺ������Ƿ񴥷�
* Para          :  ��
* Return        :  false= δ����  true= �Ѵ���
*********************************************************************************************************************/
static bool App_GUI_CheckStayDetectAction( void )
{
	if( s_GuiData.StaySenseCheckEn == true )
	{
		s_GuiData.StaySenseCheckEn = false;
		if( 0 == HAL_EXPORT_PinGet( EM_IR_IRQ ) )  //�������
		{	
			my_printf( "wake by auto stay detect!\n" ); 
			return true;
		}
	}
	return false;
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_GetAutoLockTimPara()
* Description   :  ��ȡ�Զ������趨��ʱ��
* Para          :  ��
* Return        :  �Զ�����ʱ����� ��λ �� 
*********************************************************************************************************************/
static uint8_t App_GUI_GetAutoLockTimPara( void )
{
	return SystemSeting.SysAutoLockTime;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetAutoLockSts()
* Description   :  ��ȡ�Զ�����������
* Para          :  none 
* Return        :  0: ������   1: ����
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
* Description   :  ����ʱ��ﵽ������
* Para          :  none 
* Return        :  0: ������   1: ����
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
* Description   :  ��ض�ʱ��  1000msִ��һ�� ���ߺ����� ���Ѻ�رոö�ʱ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
void App_GUI_Tim1000Ms( void )
{	
  #ifdef WEATHER_FORECAST_ON
	static uint32_t datetamp;
  #endif
   /*-----------�Զ������ж�----------*/
	if( s_GuiData.AutoLockTimPara > 0 )
	{
	    s_GuiData.AutoLockTimPara--;	
	}
	if( 1 == App_GUI_GetAutoLockSts() )
	{
		my_printf( "wake by auto lock!\n" ); 
        App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  //���û���Դ		
		App_GUI_SetSysSleepSts( false );	
	}
	
   /*-------�ӽ���Ӧ��ͣ����ʱ��------*/
	if( s_GuiData.NearSensePusalTim > 0 )
		s_GuiData.NearSensePusalTim--;
	
   
   /*-----------��δ��+�������------*/
    HAL_Motor_Tim1000Ms(); 
	if( true == HAL_Motor_GetForgetLockWarmState() )      //��δ�ظ澯
	{
		my_printf( "wake by fortget lock door!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}
    else if( true == HAL_Motor_GetFalseLockWarmState() )  //�����澯
	{
		my_printf( "wake by false lock door!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}
    else if( true == HAL_Motor_GetHandleTryForbitWarmState() )  //��������澯
	{
		my_printf( "wake by handler try forbit!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}

   /*-----------����ͬ�����---------*/
	if( s_GuiData.NetUpdatePeroidTim > 0 )
	    s_GuiData.NetUpdatePeroidTim--;
	
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON   //����è��
	if( (SystemSeting.SysWifiMainSw == FUNCTION_ENABLE)  //������
      &&(true == s_GuiData.NetUpdateFuncEnSts)           //����ʹ��
	  &&(0 == s_GuiData.NetUpdatePeroidTim)              //��ʱ��
	  &&(true == App_GUI_GetNetworkErrState())           //���������쳣
	   )  
	{
		my_printf( "wake by network update!\n" ); 
		App_GUI_SetSysWakeupType( E_WAKE_NETWORK_UPDATE );    					
		App_GUI_SetSysSleepSts( false );
		return;
	}
	#endif
 
   /*-----------����ͬ�����---------*/
	#ifdef WEATHER_FORECAST_ON
	if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )  //������
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
			
			if( s_GuiData.WeatherUpdateErrCnt == 1 )  //ͬ��ʧ��
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
	
			/*-----------�������-------------*/
	if( s_GuiData.StayDetectTimPara > 0 )
	{
		s_GuiData.StayDetectTimPara--;
		my_printf("s_GuiData.StayDetectTimPara = %d\n", s_GuiData.StayDetectTimPara);
	}
	#if defined IR_FUNCTION_ON
	if( (s_GuiData.NearSensePusalTim == 0) && (1 == s_GuiData.StayDetectTimPara) && (SystemSeting.SysHumanIrDef) && (SystemSeting.SysDrawNear != E_SENSE_OFF) )
	{
			if( 0 == HAL_EXPORT_PinGet( EM_IR_IRQ ) )  //�������
			{
				s_GuiData.StayDetectFlg = 0;
				s_GuiData.StaySenseCheckEn = false;
				my_printf( "wake by auto stay detect!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  //���û���Դ						
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
		if( 0 == HAL_EXPORT_PinGet( EM_IR_IRQ ) )  //�������
		{
			s_GuiData.StayDetectFlg = 0;
			s_GuiData.StaySenseCheckEn = false;
			my_printf( "wake by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  //���û���Դ						
			App_GUI_SetSysSleepSts( false );
			return;
		}	
	}
	#endif
   /*-----------THE END--------------*/
}

/*********************************************************************************************************************
* Function Name :  App_GUI_MenuJump()
* Description   :  GUI������ת����
* Para          :  pageNo- ����ת�Ľ�����
* Return        :  void
*********************************************************************************************************************/
void App_GUI_MenuJump( MenuIndexEnum_E pageNo )
{
    MenuItem.MenuIndexType.Currently = pageNo; 
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetCurMenuNo()
* Description   :  ��ȡ��ǰ�˵����
* Para          :  none
* Return        :  ��ǰ�˵����
*********************************************************************************************************************/
MenuIndexEnum_E  App_GUI_GetCurMenuNo( void )
{
	return MenuItem.MenuIndexType.Currently; 
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetRegisterSts()
* Description   :  ��ȡϵͳ��ע��״̬
* Para          :  ��
* Return        :  0xA5B7= δע�� 1= ���ع���Ա��ע��  2= APP����Ա��ע��
*********************************************************************************************************************/
uint16_t App_GUI_GetRegisterSts( void )
{
	return SystemSeting.SystemAdminRegister;
}


/*********************************************************************************************************************
* Function Name :  App_GUI_GetRegisterSts()
* Description   :  ����ϵͳ��ע��״̬
* Para          :  ��
* Return        :  0xA5B7= δע�� 1= ���ع���Ա��ע��  2= APP����Ա��ע��
*********************************************************************************************************************/
static void App_GUI_SetRegisterSts( uint16_t type )
{
    SystemSeting.SystemAdminRegister = type;
	(void)SystemWriteSeting( (uint8_t *)&SystemSeting.SystemAdminRegister, sizeof SystemSeting.SystemAdminRegister );
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetDoubleCheckSwSts()
* Description   :  ��ȡ˫����֤���ܿ���״̬
* Para          :  ��
* Return        :  0= �ر�   1= ����   
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
* Description   :  ��ȡ˫����֤�������
* Para          :  ��
* Return        :  ���״̬
*********************************************************************************************************************/
DoubleCheckType_U App_GUI_GetDoubleCheckType( void )
{
	DoubleCheckType_U type; 
	type.data = SystemSeting.SysLockMode;
	return type;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSysSleepSts()
* Description   :  ��ȡϵͳ����״̬
* Para          :  ��
* Return        :  0= ����   1= ������   
*********************************************************************************************************************/
uint8_t App_GUI_GetSysSleepSts( void )
{
	return s_GuiData.SystemSleepState; 
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysSleepSts()
* Description   :  ����ϵͳ����״̬
* Para          :  ��
* Return        :  0= �ر�   1= ����   
*********************************************************************************************************************/
void App_GUI_SetSysSleepSts( uint8_t para )
{
	s_GuiData.SystemSleepState = para;
	#if LOCK_PROJECT_CHIP ==LOCK_PROJECT_RTL8762 
	if(para==0)//ȷ�ϱ����Ƿ���
	{
		APP_TaskSendMsg();/* Send msg to app task */
	}
	#endif	
}

/*********************************************************************************************************************
* Function Name :  App_GUI_RelieveTryProtect()
* Description   :  �����ֹ����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
void App_GUI_RelieveTryProtect( void )
{
	s_GuiData.TouchLockNum = 0;
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
	TryAlarmFirst = EM_TRY_DEFAULT;
#endif
	if( SystemSeting.CheckErrAllCnt )  //��ֹƵ��д��
	{
		SystemSeting.CheckErrAllCnt = 0;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
	}
	if( SystemSeting.CheckErrPwdCnt )  //��ֹƵ��д��
	{
		SystemSeting.CheckErrPwdCnt = 0;  
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrPwdCnt, sizeof SystemSeting.CheckErrPwdCnt );	
	}
	App_GUI_StopStayDetectTim();//ֹͣ���ζ�����ʱ
}

/*********************************************************************************************************************
* Function Name :  App_GUI_UpdateMenuQuitTime()
* Description   :  ����GUI�˳�ʱ��    
* Para          :  para-��ˢ�µ�����   ��λ10ms  mode- �Ƿ�ǿ�Ƹ���   false= ��ǿ��   true= ǿ��
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
* Description   :  ��ȡ���ŷ�ʽ
* Para          :  none
* Return        :  OPEN_MODEL_E -���ŷ�ʽ   
*********************************************************************************************************************/
OPEN_MODEL_E App_GUI_GetOpenModel( void )
{
 	return s_GuiData.OpenDoorModel;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetCloseModel()
* Description   :  ��ȡ���ŷ�ʽ
* Para          :  none
* Return        :  CLOSE_MODEL_E -���ŷ�ʽ  
*********************************************************************************************************************/
CLOSE_MODEL_E App_GUI_GetCloseModel( void )
{
 	return s_GuiData.CloseDoorModel;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetNearSenseUnworkCurTim()
* Description   :  ��ȡ���ųɹ���ӽ���Ӧ��������ʱ�� ��λ��
* Para          :  none
* Return        :  ��ǰʣ��ʱ��
*********************************************************************************************************************/
uint8_t App_GUI_GetNearSenseUnworkCurTim( void )
{
 	return s_GuiData.NearSensePusalTim;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetOpenModel()
* Description   :  ���ÿ��ŷ�ʽ
* Para          :  OPEN_MODEL_E -���ŷ�ʽ   
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
* Description   :  ���ù��ŷ�ʽ
* Para          :  CLOSE_MODEL_E -���ŷ�ʽ  
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
* Description   :  �ָ��ŵ�״̬
* Para          :  none
* Return        :  void
*********************************************************************************************************************/
void App_GUI_DefaultDoorState( void )
{
	s_GuiData.DoorCurState = 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSysWakeupType()
* Description   :  ��ȡϵͳ���ѷ�ʽ
* Para          :  none 
* Return        :  ϵͳ���ѷ�ʽ  WAKEUP_TYPE_E
*********************************************************************************************************************/
WAKEUP_TYPE_E  App_GUI_GetSysWakeupType( void )
{
   return s_GuiData.WakeupSourceType;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysWakeupType()
* Description   :  ���ù��ŷ�ʽ
* Para Input    :  type -���ѷ�ʽ  
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetSysWakeupType( WAKEUP_TYPE_E type )
{
	s_GuiData.WakeupSourceType = type;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetSysWorkMode()
* Description   :  ��ȡϵͳ����ģʽ
* Para          :  none 
* Return        :  ϵͳ����ģʽ  WORK_MODE_E
*********************************************************************************************************************/
WORK_MODE_E  App_GUI_GetSysWorkMode( void )
{
   return s_GuiData.SysWorkMode;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysSysWorkMode()
* Description   :  ����ϵͳ����ģʽ
* Para Input    :  mode -ϵͳ����ģʽ  
* Return        :  void
*********************************************************************************************************************/
void App_GUI_SetSysSysWorkMode( WORK_MODE_E mode )
{
	s_GuiData.SysWorkMode = mode;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetWifiUploadSwSts()
* Description   :  ��ȡWIFI�����ϴ��Ŀ���״̬
* Para Input    :  none
* Return        :  0: �ر�  1: ����
*********************************************************************************************************************/
bool App_GUI_GetWifiUploadSwSts( void )
{
    if( (SystemSeting.SysWifiMainSw == FUNCTION_ENABLE) && (SystemSeting.SysWifiLogSw == FUNCTION_ENABLE) )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //С���δ��
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
* Description   :  ��ȡWIFI ���͸澯��¼�Ŀ���״̬
* Para Input    :  none
* Return        :  0: �ر�  1: ����
*********************************************************************************************************************/
static bool App_GUI_GetWifiWarmingSwSts( void )
{
    if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //С���δ��
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
* Description   :  ������֤����
* Para Input    :  checkEnable: ������֤ʹ��λ  0=����֤����  1=��֤����
				   otherPwdCheckEn: ����&��ʱ������֤ʹ��λ 0=����֤����  1=��֤����
* Para Output   :  pPwdMeg: ��֤ͨ�����������Ϣ
* Return        :  -1= ��֤ʧ�� 0= ִ����  1= ��֤�ɹ�  2= ���ϲ�ѯ  3= �ϻ�����  4= ��֤����Ա 
                    5= �������� 6= �״�ʹ������  7= ���� 8= ��ʱ������֤�ɹ� 9= ����������֤�ɹ�
*********************************************************************************************************************/
static int8_t App_GUI_PwdCheckAndTouchHandler( uint8_t checkEnable, uint8_t otherPwdCheckEn, PwdMeg_T *pPwdMeg )
{
    uint8_t pwdLen =0;
	uint8_t pwdBuf[KEY_BUF_SIZE+1] = {0};
	uint8_t pwddata[KEY_BUF_SIZE+1]= {0};
	
	uint8_t tp1 = App_Touch_GetCurrentKeyValue();
	if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
	{
		uint8_t tm1 = App_Touch_GetCurrentKeyIndex();
		App_Touch_GetCurrentKeyValBuf( pwdBuf, &pwdLen );
		
        if( true == TouchLockKeyPushStsFlg )   //���󴥿���
		{
			TouchLockKeyPushStsFlg = false;
		    if( s_GuiData.TouchLockNum < 5 )
			{
				s_GuiData.TouchLockNum++;
			}
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
			return 5; 	
		}
		
		if( false==checkEnable && false==otherPwdCheckEn )  //�ж��Ƿ���֤����/��ʱ����
			return 0;
		
		if( tm1 >= 4 )  //����6688#
		{
			#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
			
			#else 
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //����ģʽ
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
		if( tm1 >= 6 )  //����or���봦��
		{
			if( 7 == tm1 )    //�������ѯ���봦��
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
			else if( 10 == tm1 ) //���봦��
			{
				uint8_t born_temp[10]={0x32,0x35,0x38,0x33,0x36,0x39,0x37,0x34,0x35,0x31};
				PUBLIC_ChangeDecToString( pwddata, pwdBuf, pwdLen );
				if( 0 == memcmp( pwddata, born_temp, 10) )
				{
					if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //����ģʽ�����ϻ�����
					{
						return 3;
					}
				}
			}
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //����ģʽ ����֤����
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
			if( true == checkEnable )  //�ж��Ƿ���֤���� 
			{
				my_printf("check pwd process\n"); 
				if( 1 == App_PWD_VerifyUserPwd( PWD_LIMIT_ALL, pPwdMeg, (char *)pwddata ) )  //ƥ�䵽�˿�������
				{
					return 1;
				}
			}
			if( true == otherPwdCheckEn )  //�ж��Ƿ��鱨������/��ʱ����
			{
				my_printf("check sos or temp pwd process\n"); 
				if( 1 == App_PWD_VerifySosPwd( (char *)pwddata ) )       //ƥ�䵽�˱�������
				{
					return 9;
				}
				else if( 1 == App_PWD_VerifyTempPwd( (char *)pwddata ))  //ƥ�䵽����ʱ����
				{
					return 8;
				}
			}
			return -1;	
		}
	}
	else if( TOUCH_KEY_BACK == tp1 ) //ȡ����
	{
		if( ADMIN_LOCAL_REGISTERED == App_GUI_GetRegisterSts() )  //������ģʽ
		{
			return 4;
		}
	}
	else if( TOUCH_KEY_LOCK == tp1 ) //������
	{
		if( FUNCTION_ENABLE == SystemSeting.Sysprotect_lock )  //�������󴥿���
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
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() )  //����ģʽ
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
	else if( TOUCH_KEY_BELL == tp1 ) //�����
	{	
		return 7;
	}
	
	return 0;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_FingerCheck()
* Description   :  ָ����֤����
* Para          :  ��
* Return        :  -1: ��֤ʧ��  0: ��֤��  1: ��֤�ɹ�
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
		
		case 1: //����ָ�Ƽ�������	
				fingerPara.emAppFlow = EM_FINGER_APP_FLOW3_SEARCH;
				APP_FINGER_Operate( fingerPara );   //������ָ֤������
		        FingerCheckFlow = 2;
		break;
		
		case 2: //�ȴ������	
				checkresult = APP_FINGER_GetFlowResult( pageId );
				if( FINGER_APP_RESULT_SUC == checkresult )         //��֤�ɹ�
				{
					if( userLimit == MEM_USER_ALL )  //ͨ����֤
					{
						my_printf( "finger check is all ok! \n" ); 
						APP_FINGER_Sleep();  //�ر�ָ��ģ��
						FingerCheckFlow = 0;					
						return 1;
					}
					else
					{
						if( true == APP_FINGER_CfgRead( *pageId, &fingermeg ) )
						{
							if( fingermeg.acOffset[EM_FINGER_APP_CFG_ADMIN_EN] == userLimit )  //��֤Ȩ��ͨ��
							{
								my_printf( "finger check is limit! \n" ); 
								APP_FINGER_Sleep();  //�ر�ָ��ģ��
								FingerCheckFlow = 0;					
								return 1;
							}
						}
					}
					
					my_printf( "finger check is error! \n" ); 
					APP_FINGER_Sleep();  //�ر�ָ��ģ��
					FingerCheckFlow = 0;					
					return -1;
				}
				else if( FINGER_APP_RESULT_FAIL == checkresult )   //��֤ʧ��
				{ 
					if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) 
					{
						my_printf( "finger check is ok at try mode! \n" );  
						APP_FINGER_Sleep();  //�ر�ָ��ģ��
						FingerCheckFlow = 0;
						return 1;
					}
                    #ifdef FINGER_FUNCTION_ON
					if( errcnt == 0 )  //������֤
					{
						errcnt = 1;
						my_printf( "finger check twice\n" ); 
						FingerCheckFlow = 1;
					}
					else
                    #endif
					{
						APP_FINGER_Sleep();  //�ر�ָ��ģ��
						FingerCheckFlow = 0;
						return -1;
					}
				}
                else if(FINGER_APP_PROTOCAL_ERR == checkresult || FINGER_APP_RESULT_TIMEOUT == checkresult)// ��ʱ��Э����������
                {
					APP_FINGER_Sleep();  //�ر�ָ��ģ��
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
* Description   :  ����ָ����֤����
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
        APP_FINGER_Sleep();  //�ر�ָ��ģ��
		my_printf( "App_GUI_FingerCheckStop()\n" );   
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_FaceCheck()
* Description   :  ������֤����
* Para Input    :  userLimit: �û�Ȩ��  'M'-����Ա  'A'-�����û�    
* Para Output   :  pageId: ����id
* Return        :  -1: ��֤ʧ��  0: ��֤��  1: ��֤�ɹ�
*********************************************************************************************************************/
#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
static int8_t App_GUI_FaceCheck( uint8_t userLimit, uint16_t *pageId, uint8_t *unlockStatus)
{
	uint8_t tp1;
	tp1 = (userLimit == MEM_USER_MASTER) ? 1:0;
	uint8_t faceret = FaceGetVeifyState( tp1, pageId, unlockStatus);//ʶ������  ʱЧ���ر�
	if( FACE_VERIFY_SUCCESS == faceret )           //��֤�ɹ�
	{
		my_printf( "open door by face \n" );  
		return 1;
	}
	else if( FACE_VERIFY_MODULE_FAIL == faceret || FACE_VERIFY_TIME_FAIL== faceret     \
		  || FACE_VERIFY_EE_FAIL== faceret || FACE_VERIFY_ADMIN_FAIL== faceret )   //��֤ʧ��
	{
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) 
		{
			return 1;
		}
		return -1;
	}
	else if( FACE_VERIFY_NOFACE == faceret )      //δ��⵽����
	{
		my_printf( "find none face!\n" );  
		return 2;
	}
 
	return 0;
}
#endif

/*********************************************************************************************************************
* Function Name :  App_GUI_FaceCheckStop()
* Description   :  ����������֤����
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
* Description   :  ����Ƿ�ִ������ͬ������
* Para Input    :  none
* Para Output   :  none
* Return        :  0= ��ִ������ͬ��  1= ִ������ͬ��
* author        :  gushengchi 
*********************************************************************************************************************/
uint8_t App_GUI_CheckNetworkAction( void )
{
	if( (SystemSeting.SysWifiMainSw == FUNCTION_ENABLE)&&(true == App_GUI_GetNetworkErrState()) )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //С���δ��
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
* Description   :  ����Ƿ�ִ������ͬ������
* Para Input    :  none
* Para Output   :  none
* Return        :  0= ��ִ��ͬ��  1= ִ��ͬ��
* author & date :  gushengchi  2021/11/22
*********************************************************************************************************************/
uint8_t App_GUI_CheckWeatherUpdateAction( void )
{
	if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //С���δ��
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
* Description   :  ���������Ƶ��������
* Para Input    :  none
* Para Output   :  none
* Return        :  0= ��ִ��  1= ִ��
* author        :  gushengchi 
*********************************************************************************************************************/
static uint8_t App_GUI_CheckBellVideoAction( void )
{
	if( SystemSeting.SysWifiMainSw == FUNCTION_ENABLE )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON || defined XM_CAM_FUNCTION_ON   
		    int8_t batDownSts = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
			if( -3 == batDownSts )  //С���δ��
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
* Description   :  ��ȡ�������״̬
* Para Input    :  none
* Para Output   :  none
* Return        :  false= ����  true= �쳣
* author        :  gushengchi 
*********************************************************************************************************************/
bool App_GUI_GetNetworkErrState( void )
{
    bool ret = false;
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON   //����è���Ƿ�������
	ret = ( MEDIA_STATE_ERROR == NetWorkStateGet() )? true:false;
	#endif
	
	return ret;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GetFaceCheckEnState()
* Description   :  ��ȡ������֤����״̬
* Para Input    :  none
* Para Output   :  none
* Return        :  false= �ر�  true= ����
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
* Description   :  �����˴���״̬
* Para Input    :  none
* Para Output   :  none
* Return        :  false= ����  true= ��������
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
* Description   :  �����˽��״̬
* Para Input    :  none
* Para Output   :  none
* Return        :  false= ����  true= ���˻ָ�
* author        :  gushengchi 
*********************************************************************************************************************/
static bool App_GUI_CheckAlarmButtonRecoveryWarm( void )
{
	if( s_GuiData.AlamMeg.AlamButtonNewTrigger == true )  //�Ѵ���
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
* Description   :  ϵͳ���˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_MainDeskMenu( void )          //ϵͳ����˵�
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
		if( E_WAKE_BELL_KEY == App_GUI_GetSysWakeupType() )  //���廽��  ������ǰ����
		{
			if( SystemTick <= GUI_TIME_2S )
			{
				HAL_Voice_BellCtrl( true );
				my_printf( "Bell is working!\n" );  
				#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON    //����è��
				if( 1 == App_GUI_CheckBellVideoAction() )
				{
					App_GUI_MenuJump( EM_MENU_BELL_VIDEO );
					return;
				}
				#elif defined XM_CAM_FUNCTION_ON   //��è��
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
		App_Touch_FuncEnCtrl( EM_SCAN_OFF );   //����ɨ������	
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_ON ); 
        #ifdef LOCK_KEY_WHITE_LED_ON
        App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_ON ); //�����ż��׵�
        #endif
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_ON );//������ƵĲ�Ʒ���������
		App_Key_ResetCombinKeyFlow();         //ǿ�Ƹ�λ��ϰ����������
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
		  )  //���ڿ����� + ���廽��
		{
			checkKeyType = E_CHECK_DEFAULT;
			my_printf( "open door in room!\n" ); 
		}
		else  //�����ڿ�����
		{
			if( 1 == App_GUI_GetDoubleCheckSwSts() )  //˫����֤����
			{
				#ifdef FACE_FUNCTION_ON
				HAL_Voice_PlayingVoice( EM_CHECK_FACE_MP3, 0 );   
				#elif defined IRIS_FUNCTION_ON
				HAL_Voice_PlayingVoice( EM_CHECK_IRIS_MP3, 0 );  
				#elif defined FINGER_FUNCTION_ON
				if( 0 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//δ��⵽��ָ 
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
				if( 0 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//δ��⵽��ָ 
				{
				    HAL_Voice_PlayingVoice( EM_CHECK_VEIN_MP3, 0 );   
				}
				#endif
			}
			else 
			{
				if( 0 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//δ��⵽��ָ 
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
	 /*----------------���ô�������--------------------*/
		if( SystemWorkHoldTim >= GUI_TIME_1S )  //��ֹ�����������Ѻ�����������
		{
			if( touchsetflg == 0 )
			{
				touchsetflg = 1;
				App_Touch_FuncEnCtrl( EM_SCAN_ON );   //����ɨ������
			}
		}
	 /*----------------��ϰ�����������----------------*/
		BUTTON_TYPE_E ret = App_Key_GetCombinKeyState();  //��ȡ��е������ϼ�
		if( EM_OPEN_DOOR_KEY == ret )          //��������
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetOpenModel( EM_OPEN_BUTTON );
			SystemEventLogSave( KEY_OPEN_IN_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		#ifdef CLOSE_BUTTON_ON
		else if( EM_CLOSE_DOOR_KEY == ret )    //��������
		{
            App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetCloseModel( EM_CLOSE_BUTTON );
			SystemEventLogSave( KEY_CLOSE_IN_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			return;
		}
		#endif
		else if( EM_BACK_FACTORY_KEY == ret )  //�ָ���������
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_BACK_FACTORY );
			return;
		}
		else if( EM_ENTER_APP_MODEL_KEY==ret)  //APP����ģʽ
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_APP_MODEL );
			return;
		}
		else if( EM_ENTER_LOCAL_MODEL_KEY==ret)//���빤��ģʽ
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() )  //������ģʽ
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
				App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
				return;
				#endif
			}
		}
		else if( EM_SCAN_NONE_KEY == ret )     //�а��������� 
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
		else if( EM_SCANNING_KEY == ret )      //�ް��������� 
		{
			/*------------------���˽��-----------------*/	 
			if( true == App_GUI_CheckAlarmButtonRecoveryWarm())
			{
				my_printf("alrm button is recovery!\n");
				HAL_Voice_WorkModeCtrl( false );//�����˳�����ģʽ
			    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );  
				if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
				{
					HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
				}
			}
			/*------------------���˴���-----------------*/	 
			if( true == App_GUI_CheckAlarmButtonActionWarm() ) 
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_ALARM_WARM );
				return;
			}
			/*------------------��⵽����ָ-------------*/	 
			if( 1 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )
			{
				if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤����
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
			
			/*----------------������ʽ����APPģʽ--------*/	
		    if( DRV_GetBleConnect() )    
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_APP_MODEL );
				return;
			}
			/*------------------���ֹ���-----------------*/	
			if( 1 == App_Key_GetCloseHandleSts() )     
			{
				if( EM_CLOSE_HANDLER != App_GUI_GetCloseModel() )  //�����ν���
				{
					App_GUI_RelieveTryProtect();  //�������
					App_GUI_SetCloseModel( EM_CLOSE_HANDLER ); 
 				    SystemEventLogSave( BAC_CLOSE_IN_DOOR, 0 );  
				}
			}
			/*------------------���ֿ���-----------------*/	
			else if( 1 == App_Key_GetOpenHandleSts() )  
			{
				if( EM_OPEN_HANDLER != App_GUI_GetOpenModel() )  //�����ν���
				{
					App_GUI_RelieveTryProtect();  //�������
					App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
					SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
				}
				if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //����״̬
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
				if( EM_OPEN_HANDLER != App_GUI_GetOpenModel() )  //�����ν���
				{
					App_GUI_RelieveTryProtect();  //�������
					App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
					SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
				}
				if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //����״̬
				{
					#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
					App_GUI_FaceCheckStop( &FaceWorkStsFlg );
					#endif
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					App_GUI_MenuJump( EM_MENU_DEPLAY_WARM );
					return;
				}
			}
			/*------------------���Զ������Զ�����-------*/	
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
			/*------------------��������-----------------*/	
			if( true == HAL_Motor_FalseLockWarmCheck() )
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_FALSE_LOCK_WARM );
				return;
			}
			/*------------------��δ������---------------*/	
			if( true == HAL_Motor_ForgetLockWarmCheck() )
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_FORGET_LOCK_WARM );
				return;
			}
			/*------------------�������汨��-------------*/	
			if( true == HAL_Motor_HandleTryForbitWarmCheck() )
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
			    App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			    App_GUI_MenuJump( EM_MENU_FALSE_LOCK_WARM );
				return;
			}

			/*------------------�Զ���������-------------*/	
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
			/*------------------������������-------------*/	
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
		
	 /*----------------����+ָ����֤��������-----------------*/
		switch( checkKeyType )
		{
            #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
			case E_CHECK_FACE:      //����/��Ĥ��֤
			{
				FaceWorkStsFlg = true;
				faceret = App_GUI_FaceCheck( MEM_USER_ALL, &facePageId, &unlockStatus);
				if( faceret == 1 )        //��֤�ɹ�
				{
					FaceWorkStsFlg = false;
					faceCheckResult = true;
					checkKeyType = E_CHECK_DEFAULT;
					if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤����
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
						if(unlockStatus == 0xCC)//����������������״̬
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
							if(unlockStatus == 0xCC)//����������������״̬
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
				else if( faceret == -1 )  //��֤ʧ��
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FACE_CHECK_ERR );
					return;
				}
				else if( faceret == 2 )   //δ��⵽����
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					return;
				}
			}
			break;
			#endif
			case E_CHECK_FINGER:   //ָ��/ָ������֤
			{
				FingerWorkStsFlg = true;
				fingerret = App_GUI_FingerCheck( &fingerPageId, MEM_USER_ALL );
				if( fingerret == 1 )        //��֤�ɹ�
				{
					my_printf("finger check success =%d\n", SystemTick); 
					FingerWorkStsFlg = false;
					fingerCheckResult = true;
					checkKeyType = E_CHECK_DEFAULT;
					if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤����
					{
						KeyTppe = App_GUI_GetDoubleCheckType(); 
						if( KeyTppe.data == LOCK_MODE_FINGER ) //������ʽ����Ϊ��ָ�ƿ���ģʽ��
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
						if(unlockStatus == 0xCC)//����������������״̬
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
							if(unlockStatus == 0xCC)//����������������״̬
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
				else if( fingerret == -1 )  //��֤ʧ��
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FINGER_CHECK_ERR );
					return;
				}
                else if( fingerret == -2 )  //��ʱ��Э�����
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
   
		/*----------------IC����������----------------*/
		#ifdef IC_CARD_FUNCTION_ON
		if( FUNCTION_DISABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤�ر�
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
	 /*----------------����+���̴�������----------------*/
		if( App_Touch_GetCurrentKeyIndex() >= 2 )  //2�������ر�����ʶ��
		{
			if( FUNCTION_DISABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤�ر�
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
		if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤����
		{
			KeyTppe = App_GUI_GetDoubleCheckType(); 
			if( KeyTppe.bit.PwdCheckEnable == FUNCTION_ENABLE ) //������֤ʹ��
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
		tp1 = App_GUI_PwdCheckAndTouchHandler( pwdCheckEn, OtherTypePwdCheckEn, &pwdmeg );  //������֤+��������
		if( (1 == tp1) || (8 == tp1) || (9 == tp1) )       //��֤�ɹ� (����/��ʱ����/��������)
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			bool retbool2 = false;
			#if defined  FACE_FUNCTION_ON
			if(unlockStatus == 0xCC)//����������������״̬
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
			if( true == retbool2 )   //��������
			{ 
				if( FUNCTION_ENABLE == App_GUI_GetDoubleCheckSwSts() )  //˫����֤����
				{
					if( faceCheckResult == true )
					{ 
						#if defined  FACE_FUNCTION_ON
						WifiLockMeg.UnlockMode = FACE_PWD;
						if(unlockStatus == 0xCC)//����������������״̬
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
                    
					if( 1 == tp1 )      //����
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_USER_ID;
					}
					else if( 8 == tp1 ) //��ʱ����
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_TEMP_ID;
					}
					else if( 9 == tp1 ) //��������
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
					if( 1 == tp1 )      //����
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_USER_ID;
						WifiLockMeg.Attribute = NONE;
					}
					else if( 8 == tp1 ) //��ʱ����
					{
						WifiLockMeg.PageID.way2 = NONE;
						WifiLockMeg.PageID.id2  = PWD_TEMP_ID;
						WifiLockMeg.Attribute = NONE;
					}
					else if( 9 == tp1 ) //��������
					{
						WifiLockMeg.PageID.way2 = SOS;
						WifiLockMeg.PageID.id2  = PWD_SOS_ID;
						WifiLockMeg.Attribute = SOS;
					}
//					App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
					UploadUnlockDoorMegEnable = 1;
				}
			}
			
			if( 1 == tp1 )      //����
			{
				App_GUI_SetOpenModel( EM_OPEN_PWD );
				SystemEventLogSave( PASSWORD_OPEN, PWD_USER_ID );
			}
			else if( 8 == tp1 ) //��ʱ����
			{
				App_GUI_SetOpenModel( EM_OPEN_TMP_PWD );
				SystemEventLogSave( TEMP_PASSWORD_OPEN, PWD_TEMP_ID ); 
			}
			else if( 9 == tp1 ) //��������
			{
				App_GUI_SetOpenModel( EM_OPEN_SOS_PWD );
				SystemEventLogSave( SOS_PASSWORD_OPEN, PWD_SOS_ID );  
			}
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		else if( -1 == tp1 ) //��֤ʧ�� 
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_PWD_CHECK_ERR );
			return;
		}
		
#if defined (XM_CAM_FUNCTION_ON) || defined (SMART_SCREEN_ON)
		else if( 2 == tp1 )  //���ϲ�ѯ  
		{
			if( false == HAL_Voice_WorkModeGet() )  //�Ǳ���ģʽ
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
		else if( 3 == tp1 )  //�ϻ�����  
		{
			if( false == HAL_Voice_WorkModeGet() )  //�Ǳ���ģʽ
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EN_MENU_AGING_TEST );
				return;
			}
		}
		else if( 4 == tp1 )  //��֤����Ա   
		{
			if( false == HAL_Voice_WorkModeGet() )  //�Ǳ���ģʽ
			{
				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
				#endif
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_CHECK_ADMIN );
				return;
			}
		}
		else if( 5 == tp1 )  //������������   
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
		else if( 6 == tp1 )  //���������״�ʹ��   
		{
			if( false == HAL_Voice_WorkModeGet() )  //�Ǳ���ģʽ
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
				return;
			}
		}
		else if( 7 == tp1 )  //����   
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
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON    //����è��
			if( 1 == App_GUI_CheckBellVideoAction() )
			{
				App_GUI_FingerCheckStop( &FingerWorkStsFlg );
				App_GUI_MenuJump( EM_MENU_BELL_VIDEO );
				return;
			}
			#elif defined XM_CAM_FUNCTION_ON   //��è��
				if( 1 == App_GUI_CheckBellVideoAction() )
				{
					CAM_SendCommandStart(CAM_CMD_BELL, 0, 0);
				}
			#endif 
		}
	 /*------------------THE END------------------*/	
	}
    else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //ȷ�������������
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		App_GUI_FaceCheckStop( &FaceWorkStsFlg );
		#endif
		App_GUI_FingerCheckStop( &FingerWorkStsFlg );
		//SystemEventLogSave( NOTHING_CASE, 0 );  //�¼���¼
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SystemManage()
* Description   :  ϵͳ����Ա�˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SystemManageMenu( void )      //ϵͳ�����˵�
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
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );       //����ɨ�迪��
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        App_GUI_RelieveTryProtect();  //�������
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_GEAR);
		HAL_Voice_PlayingVoice( EM_ADMIN_MANAGE_MENU_MP3, 0 );	
	    MenuItem.CurMenuNum = EM_MENU_STEP_1;
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();

		
		if( TOUCH_KEY_NO_1 == tp1 )        //�����û�
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;	
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�޸�����
		{
			App_GUI_MenuJump( EM_MENU_SET_PWD_MENU );
			return;	
		}
		else if( TOUCH_KEY_NO_3 == tp1 )    //ɾ���û�
		{
			App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;	
		}
		else if( TOUCH_KEY_NO_4 == tp1 )    //ϵͳʱ������
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_TIME_SETTING );
			return;		
		}
		else if( TOUCH_KEY_NO_5 == tp1 ) //������ʽ����
		{
		    App_GUI_MenuJump( EM_MENU_UNLOCK_WAY_SETTING );
			return;		
		}
		else if( TOUCH_KEY_NO_6 == tp1 ) //��������
		{
		    App_GUI_MenuJump( EM_MENU_VOICE_SETTING );
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;		
		}
	}
 	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;	
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckAdminMenu()
* Description   :  ��֤����ԱȨ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckAdminMenu( void )        //��֤����ԱȨ��
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		App_Key_ResetCombinKeyFlow();   //����֮ǰǿ�Ƹ�λ��ϰ����������  �Է���һ
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
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //����ɨ���ȡ����
	{
	 /*----------------��ϰ�����������-----------------*/
		BUTTON_TYPE_E ret = App_Key_GetCombinKeyState();  //��ȡ��е������ϼ�
		if( EM_OPEN_DOOR_KEY == ret )   //��������
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetOpenModel( EM_OPEN_BUTTON );
			SystemEventLogSave( KEY_OPEN_IN_DOOR, 0 );   
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
	    #ifdef CLOSE_BUTTON_ON
		else if( EM_CLOSE_DOOR_KEY == ret )       //��������
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_SetCloseModel( EM_CLOSE_BUTTON );
			SystemEventLogSave( KEY_CLOSE_IN_DOOR, 0 );   
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			return;
		}
		#endif
		else if( EM_BACK_FACTORY_KEY == ret )//�ָ���������
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_BACK_FACTORY );
			return;
		}
		else if( EM_ENTER_APP_MODEL_KEY == ret )//APP����ģʽ
		{
			App_GUI_FingerCheckStop( &FingerWorkStsFlg );
			App_GUI_MenuJump( EM_MENU_APP_MODEL );
			return;
		}
		else if( EM_SCAN_NONE_KEY == ret )   //�а��������� 
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
		else if( EM_SCANNING_KEY == ret )    //�ް��������� 
		{
//			if( 1 == App_Export_GetPinState( E_PIN_ALARM_IRQ ) )     //���˴���
//			{
//				#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
//				App_GUI_FaceCheckStop( &FaceWorkStsFlg );
//				#endif
//			    App_GUI_FingerCheckStop( &FingerWorkStsFlg );
//				App_GUI_MenuJump( EM_MENU_ALARM_WARM );
//				return;
//			}
			#if !(defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON)
			if( 1 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) )//��⵽����ָ
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
 
		    if( 1 == App_Key_GetCloseHandleSts() )     //���ֹ���
			{
				if( EM_CLOSE_HANDLER != App_GUI_GetCloseModel() )  //�����ν���
				{
					App_GUI_RelieveTryProtect();  //�������
					App_GUI_SetCloseModel( EM_CLOSE_HANDLER );   
				}
			}
			else if( 1 == App_Key_GetOpenHandleSts() ) //���ֿ���
			{
				if( EM_OPEN_HANDLER != App_GUI_GetOpenModel() )  //�����ν���
				{
					App_GUI_RelieveTryProtect();  //�������
					App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
					SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  //�¼���¼
				}
				if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //����״̬
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
		
	 /*----------------����+ָ����֤��������-----------------*/
		switch( checkKeyType )
		{
            #if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON  
			case E_CHECK_FACE:   //������֤
			{
				FaceWorkStsFlg = true;
				faceret = App_GUI_FaceCheck( MEM_USER_MASTER, &pageid, &unlockStatus);
				if( faceret == 1 )        //��֤�ɹ�
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
				else if( faceret == -1 )  //��֤ʧ��
				{
					FaceWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FACE_CHECK_ERR );
					return;
				}
				else if( faceret == 2 )   //δ��⵽����
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
			case E_CHECK_FINGER: //ָ����֤
			{
				FingerWorkStsFlg = true;
				fingerret = App_GUI_FingerCheck( &pageid, MEM_USER_MASTER );
				if( fingerret == 1 )        //��֤�ɹ�
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
				else if( fingerret == -1 )  //��֤ʧ��
				{
					FingerWorkStsFlg = false;
					checkKeyType = E_CHECK_DEFAULT;
					App_GUI_MenuJump( EM_MENU_FINGER_CHECK_ERR );
					return;
				}
                else if( fingerret == -2 )  //��ʱ��Э�����
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
	 
	 /*----------------����+���̴�������----------------*/
	 #if 0
		if( App_Touch_GetCurrentKeyIndex() >= 2 )  //2�������ر�����ʶ��
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
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tm1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen );   
			if( tm1 >= 6 )  //���� 	
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
				if( ret == 1 )        //��֤�ɹ�
				{
					App_GUI_FingerCheckStop( &FingerWorkStsFlg );
					HAL_Voice_PlayingVoice( EM_CHECK_SUCCESS_MP3, GUI_TIME_1500MS );	
					SystemEventLogSave( PWD_ADMIN_CHECK, pwdmeg.UserId );  
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					return;
				}
				else if( ret == -1 )  //��֤ʧ��
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
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //�ȴ������������
	{
		if( 0 == HAL_Voice_GetBusyState() )
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}	
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //�ȴ������������
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
* Description   :  ��������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_OpenDoorMenu( void )		  //ִ�п�������
{
	static uint8_t flowStep;
	static uint8_t defendWarmFlow;
	static uint8_t registTipsFlow;
	static uint8_t openButNotPushFlg;
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		
		if( E_WAKE_ALARM_BREAK == App_GUI_GetSysWakeupType() )    //�жϷ��˻���
		{
			HAL_Voice_WorkModeCtrl( false );   //�����˳�����ģʽ
			App_Export_SetAlrmWarmEn( false ); //���˹���ʧ��
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
			if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
			{
				HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
			}	
		}
		App_GUI_RelieveTryProtect();           //������Ա���
		HAL_ADC_UpBatValLockCtrl( true );      //�Ϸ���ص�ѹ��ס
		HAL_ADC_UnderBatValLockCtrl( true );   //�·���ص�ѹ��ס

		#ifndef SMART_SCREEN_ON
		/*-----------���ż�¼����---------*/
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
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
		DeploymentFlag = 0;
#endif
		autoLockCheckFlg = false;
		AutoLockActionCheck=false;
#ifdef SMART_SCREEN_ON
		screenDispay = 0;
#endif
		App_GUI_StopStayDetectTim();//ֹͣ���ζ�����ʱ
		GuiQuitTimMs = GUI_TIME_60S;
	}

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		HAL_Voice_PlayingVoice( EM_OPEN_DOOR_OK_MP3, GUI_TIME_1500MS );	
		MenuItem.CurMenuNum = EM_MENU_STEP_2;
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��
		{
			registTipsFlow = 1;
			return;
		}
	}
 
	if( flowStep == 0 )
	{
		/*----------δע������--------------*/
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
		else if( 1 == HAL_Voice_GetBusyState() )  //�����������
		{
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}

	
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //δע��
	{
	    if( 1 == HAL_Voice_GetBusyState() )   
		{
			return;
		}
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_OpenDoorMenu()
* Description   :  ��������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CloseDoorMenu( void )		  //ִ�й�������
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
			App_GUI_RelieveTryProtect();       //������Ա���
		}
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		
		if( closeType == EM_CLOSE_BUTTON ) 
		{
			if( E_WAKE_ALARM_BREAK == App_GUI_GetSysWakeupType() )    //�жϷ��˻���
			{
				HAL_Voice_WorkModeCtrl( false );   //�����˳�����ģʽ
				App_Export_SetAlrmWarmEn( false ); //���˹���ʧ��
				HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
				if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
				{
					HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
				}	
			}
		}
		HAL_ADC_UpBatValLockCtrl( true );      //�Ϸ���ص�ѹ��ס
		HAL_ADC_UnderBatValLockCtrl( true );   //�·���ص�ѹ��ס
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
        if( 1 == tp1 )      //�������̽���
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
		else if( -1 == tp1 )//���ŷ�������쳣
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		else if( -2 == tp1 )//��������
		{
			App_GUI_MenuJump( EM_MENU_FALSE_LOCK_WARM );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
	}
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetFaceMenu()
* Description   :  �������ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetFaceMenu( void )			  //�������ò˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        if( ADMIN_NONE_REGISTERED != App_GUI_GetRegisterSts() )  //������ģʽ	
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
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //�ر�����ģ��
	{
        MenuItem.CurMenuNum = EM_MENU_STEP_2;
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )      //���ӹ���Ա����
		{
			my_printf("SystemSeting.SysFaceAdminNum = %d\n", SystemSeting.SysFaceAdminNum);
		    if( SystemSeting.SysFaceAdminNum >= MSG_FACE_MASTER_NUM )   //�Ǽ���������
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
		else if( TOUCH_KEY_NO_2 == tp1 ) //������ͨ�û�����
		{
			my_printf("SystemSeting.SysFaceGuestNum = %d\n", SystemSeting.SysFaceGuestNum);
		    if( SystemSeting.SysFaceGuestNum >= MSG_FACE_GUEST_NUM )   //�Ǽ���������
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
		else if( TOUCH_KEY_NO_3 == tp1 ) //ɾ������
		{
			App_GUI_MenuJump( EM_MENU_FACE_DELETE );
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			DispFuncPtrPre = NULL;
			return;
		}
	}
	else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_FACE_ADD_ADMIN );
			return;
		}
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}

    #endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_AddAdminFaceMenu()
* Description   :  ���ӹ���Ա����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddAdminFaceMenu( void )	  //���ӹ���Ա����
{
	#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
	static uint8_t flag;
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddAdminFaceMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��
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
		if(FACE_ADD_OVER == tp)  //����������������
		{	
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if(FACE_ADD_ERROR == tp)
		{
			flag = 1;
		}
		else if( TOUCH_KEY_BACK == App_Touch_GetCurrentKeyValue() ) //����
		{
			(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			if(flag == 1)
			{
				if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //����ģʽ  
				{
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
					return;
				}
			}
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}	
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //����ģʽ  
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
* Description   :  ������ͨ�û�����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddGuestFaceMenu( void )	  //������ͨ�û�����
{
	#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AddGuestFaceMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		GuiQuitTimMs = GUI_TIME_60S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		if(FACE_ADD_OVER==FaceEnrollPro( MEM_USER_GUEST ))  //����������������
		{	
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == App_Touch_GetCurrentKeyValue() ) //����
		{
			(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SET_FACE_MENU );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S);
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
    #endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFaceMenu()
* Description   :  ɾ������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelFaceMenu( void )			  //ɾ������
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}
	
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckFaceErrMenu()
* Description   :  ������֤ʧ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckFaceErrMenu( void )	  //������֤ʧ��	
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
* Description   :  ָ�����ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetFingerMenu( void )		  //ָ�����ò˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		if( ADMIN_NONE_REGISTERED != App_GUI_GetRegisterSts() )  //������ģʽ	
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

		if( addAdminFingerkey == tp1 )      //���ӹ���Աָ��
		{
		    if( SystemSeting.SysFingerAdminNum >= MSG_FINGER_ADMIN_LOCAL_NUM )       //�Ǽ���������
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
		else if( addGuestFingerkey == tp1 ) //������ͨ�û�ָ��
		{
		    if( SystemSeting.SysFingerGuestNum >= MSG_FINGER_COMMON_LOCAL_GUINUM )   //�Ǽ���������
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
		else if( delFingerkey == tp1 ) //ɾ��ָ��
		{
			App_GUI_MenuJump( EM_MENU_FINGER_DELETE );
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			DispFuncPtrPre = NULL;
			return;
		}
	}
	else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_FINGER_ADD_ADMIN );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}


/*********************************************************************************************************************
* Function Name :  App_GUI_AddAdminFingerMenu()
* Description   :  ���ӹ���Աָ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddAdminFingerMenu( void )	  //���ӹ���Աָ��
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		else 
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //������3λ���ֱ��,��ȷ�ϼ�ȷ��
		GuiQuitTimMs = GUI_TIME_60S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//�����ʽ��ȷ3λ���ֱ��
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(false == APP_FINGER_CfgCheck_CustomID(addId))//customID �ظ�
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//����Ѵ������������� 
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
					APP_FINGER_Operate( fingerPara );   //��������ָ������
//					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//��ˢ�� 
					App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//������ʾ����ʽ����ȷ���������롱
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //������һ��
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint16_t pageId = 0;
        FINGER_APP_FLOW_RESULT_E ret = APP_FINGER_GetFlowResult( &pageId );
		if( FINGER_APP_RESULT_SUC == ret )         //���ӳɹ�
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
		else if( FINGER_APP_RESULT_FAIL == ret || FINGER_APP_RESULT_TIMEOUT == ret)   //����ʧ��
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_1500MS );	
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
			
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	uint8_t tp1 = App_Touch_GetCurrentKeyValue();
	if( TOUCH_KEY_BACK == tp1 )     //����
	{
		APP_FINGER_Sleep();  //�ر�ָ��ģ��
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
		return;
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		APP_FINGER_Sleep();  //�ر�ָ��ģ��
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��  
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}
}
/*********************************************************************************************************************
* Function Name :  App_GUI_AddGuestFingerMenu()
* Description   :  ������ͨ�û�ָ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddGuestFingerMenu( void )    //������ͨ�û�ָ��
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		else 
		{
			App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		}
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //������3λ���ֱ��,��ȷ�ϼ�ȷ��
		GuiQuitTimMs = GUI_TIME_60S;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//�����ʽ��ȷ3λ���ֱ��
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(false == APP_FINGER_CfgCheck_CustomID(addId))//customID �ظ�
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//����Ѵ������������� 
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
					APP_FINGER_Operate( fingerPara );   //��������ָ������
//					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//��ˢ�� 
					App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//������ʾ����ʽ����ȷ���������롱
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //������һ��
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint16_t pageId = 0;
        FINGER_APP_FLOW_RESULT_E ret = APP_FINGER_GetFlowResult( &pageId );
		if( FINGER_APP_RESULT_SUC == ret )         //���ӳɹ�
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
		else if( FINGER_APP_RESULT_FAIL == ret || FINGER_APP_RESULT_TIMEOUT == ret)   //����ʧ��
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_1500MS );	
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
			
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	uint8_t tp1 = App_Touch_GetCurrentKeyValue();
	if( TOUCH_KEY_BACK == tp1 )     //����
	{
		APP_FINGER_Sleep();  //�ر�ָ��ģ��
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
		return;
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		APP_FINGER_Sleep();  //�ر�ָ��ģ��
		if( ADMIN_NONE_REGISTERED == App_GUI_GetRegisterSts() ) //δע��  
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFingerMenu()
* Description   :  ɾ��ָ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelFingerMenu( void )         //ɾ��ָ��
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DelFingerMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )   //ָ��ɾ������
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
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}

}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckFingerErrMenu()
* Description   :  ָ����֤ʧ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckFingerErrMenu( void )	  //ָ����֤ʧ��
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckFingerErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
* Description   :  �������ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetPwdMenu( void )			  //�������ò˵�	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetPwdMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
* Description   :  �޸�����˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_ChangePwdMenu( void ) 		  //�޸�����
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		HAL_Voice_PlayingVoice( EM_INPUT_NEW_PWD_TIPS_MP3, 0 );	
		MenuItem.CurMenuNum = EM_MENU_STEP_1; 
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //����������
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
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
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //�ٴ�����������
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t ret = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
			if( ret >= 6 )
			{
				PUBLIC_ChangeDecToString( pwdBuf[1], inputBuf, MSG_PWD_BYTE_SIZE );	
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
		
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) //ȷ��������
	{
		if( 0 == memcmp( pwdBuf[0], pwdBuf[1], MSG_PWD_BYTE_SIZE ))  //2������һ��
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
				HAL_Voice_PlayingVoice( EM_PWD_TOO_SIMPLE_MP3, GUI_TIME_4S ); //������ڼ�
				MenuItem.CurMenuNum = EM_MENU_STEP_5;
			}
			else 
			{
				MenuItem.CurMenuNum = EM_MENU_STEP_4;
			}
			return;
		}
		else  //2�����벻һ��
		{
			HAL_Voice_PlayingVoice( EM_PWD_FAIL_AND_INPUT_MP3, GUI_TIME_4S ); 
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
		}
		return;
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum ) //����洢
	{
		pwdmeg.UserValue  = MEM_PWD_VALID_FLG;
		pwdmeg.Privileges = PWD_LIMIT_ADMIN;
		pwdmeg.UserId     = PWD_USER_ID;
        pwdmeg.Timeliness = 0;
        pwdmeg.Attribute  = 0;
	    pwdmeg.Weekday    = WEEKDAY_VALUE;
		memcpy( pwdmeg.Password, pwdBuf[0], MSG_PWD_BYTE_SIZE );
		pwdmeg.Password[ MSG_PWD_BYTE_SIZE ] = '\0';
		
		RTC_TimeUpdate(RTC_TIME_CLOCK_BCD); //��ȡ����ʱ��
		RTCType dateClock = Rtc_Real_Time;
		dateClock.hour    = 0;
	    dateClock.minuter = 0;
	    dateClock.second  = 0;
	    uint32_t utcTim = HAL_RTC_TmToTime( &dateClock ); //��ȡ��ǰ����ʱ���
        pwdmeg.StartTim   = utcTim;
		
		dateClock = Rtc_Real_Time;
		dateClock.year    = UTC_STOP_YEAR;
		dateClock.hour    = UTC_STOP_HOUR;
	    dateClock.minuter = UTC_STOP_MINU;
	    dateClock.second  = UTC_STOP_SECOND;
	    utcTim = HAL_RTC_TmToTime( &dateClock );    //��ȡ��ǰ����ʱ���
        pwdmeg.StopTim    = utcTim;
 	
	    App_PWD_SaveOnePwdMegIntoEeprom( &pwdmeg );
		SystemEventLogSave( ADD_PASSWORD, PWD_USER_ID );  
		HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_2S ); 
		App_GUI_SetRegisterSts( ADMIN_LOCAL_REGISTERED );	 
	    MenuItem.CurMenuNum = EM_MENU_STEP_5;
	    return;	
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum ) //����������ʱ
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
        if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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
* Description   :  ������֤ʧ�ܲ˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckPwdErrMenu( void )       //������֤ʧ��
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckPwdErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
	    SystemSeting.CheckErrPwdCnt++;  
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrPwdCnt, sizeof SystemSeting.CheckErrPwdCnt );	
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || ( SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX ) )  //����
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
* Description   :  ��ص�ѹ���޷���������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BatVolLowErrMenu( void )	  //��ѹ���޷�����
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
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_LOW_POWER);
		#ifndef BAT_UNINSET_WARM_TIP_ON   //���δ�����ѿ���
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
		if( -3 == tp1 )      //δ����
		{	
			#ifdef BATTERY_MP3_TIPS_ON
				HAL_Voice_PlayingVoice( EM_INSERT_UNDER_BAT_TIPS_MP3, GUI_TIME_3S );
			#else
				HAL_Voice_PlayingVoice( EM_INSERT_UPPER_BAT_TIPS_MP3, GUI_TIME_3S );  
			#endif
		}
		else if( -2 == tp1 ) //��ѹ���޷�����
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
		if( 1 == HAL_Voice_GetBusyState() )  //����δ�������
			return;
		
		#ifdef UNDER_BAT_ADC_ON
		int8_t sp1 = HAL_ADC_GetCellBatVolState( E_UNDER_BAT );
		if( -3 == sp1 )      //δ����
		{	
			#ifdef BATTERY_MP3_TIPS_ON
				HAL_Voice_PlayingVoice( EM_INSERT_UPPER_BAT_TIPS_MP3, GUI_TIME_3S );
			#else
				HAL_Voice_PlayingVoice( EM_INSERT_UNDER_BAT_TIPS_MP3, GUI_TIME_3S );  
			#endif
		}
		else if( -2 == sp1 ) //��ѹ���޷�����
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
		if( 0 == HAL_Voice_GetBusyState() )  //����δ�������
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
* Description   :  eeprom���ϴ���
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_EepromErrorMenu( void )       //EEPROM����
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
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
* Description   :  ���˸澯����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AlarmHandlerMenu( void )      //���˸澯����
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_ON ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 0 );	
		HAL_Voice_WorkModeCtrl( true );//�������뱨��ģʽ
		GuiQuitTimMs = GUI_TIME_900S;
		
		if( s_GuiData.AlamMeg.AlamButtonNewTrigger == false )  //�״δ���
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

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )      //è����Ƶץ��
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )      //���˹�����������ʽ��ϴ���
	{
		/*-------------���˽��---------------*/	 
		if( true == App_GUI_CheckAlarmButtonRecoveryWarm())
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
	   /*--------------�����廽��-------------*/	
	    if( 1 == App_Export_GetPinState( E_PIN_TOUCH_IRQ ) )     
		{
			my_printf( "alarm break by key touch!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------���ڿ��Ű�������-------*/
		else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   
		{
			my_printf( "alarm break by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------���ڹ��Ű�������-------*/
		else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  
		{
			my_printf( "alarm break by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  
			SystemWakeupInit();   
			return;
		}
	   /*--------------ָ�ƻ���---------------*/
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	    
		{
			my_printf( "alarm break by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_ALARM_BREAK );  			
			SystemWakeupInit();    
			return;
		}	
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //ͳһ�رյ�ǰ��������
	{
		HAL_Voice_WorkModeCtrl( false );//�����˳�����ģʽ
		HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
		if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
		{
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
		}	
		MenuItem.CurMenuNum = EM_MENU_STEP_4;
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )  //ͳһȥ����
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_TryProtectMenu()
* Description   :  ���Ա����澯
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_TryProtectMenu( void )        //���Ա����澯
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
	    App_Touch_FuncEnCtrl( EM_SCAN_ONLY_BELL );  //����ɨ��
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		HAL_Voice_PlayingVoice( EM_ERR_PROTECT_ALARM_MP3, 0 );	
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_ON );//������ƵĲ�Ʒ���������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		GuiQuitTimMs = GUI_TIME_7S;
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
		if(EM_TRY_DEFAULT == TryAlarmFirst)
		{
			if( true == App_GUI_CheckBellVideoAction() )
			{
				if(FaceProAlarm(FORBID_TRY, WifiTxTemp.data, WifiTxTemp.length) == 1)
				{
					TryAlarmFirst = EM_TRY_FIRST_OVER;//180s��ץ�ĳɹ���һ��
					my_printf("FaceProAlarmResult = 1;\n");
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
				}
			}
		}
		
		if(TOUCH_KEY_BELL == tp1) //���崥��
		{
			HAL_Voice_BellCtrl( true );
			if(EM_TRY_FIRST_OVER == TryAlarmFirst)//180s��ץ�ĳɹ���һ��
			{
				App_GUI_MenuJump( EM_MENU_BELL_VIDEO );
				return;
			}
		}
		#else
		if( TOUCH_KEY_BELL == tp1 )  //���崥��
		{
			HAL_Voice_BellCtrl( true );
			#if defined XM_CAM_FUNCTION_ON   //��è��
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
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
* Description   :  ���������澯
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_StayWarmMenu( void )		  //���������澯
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
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //����ɨ��
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		HAL_Voice_PlayingVoice( EM_HI_TELL_ADMIN_MP3, 0 );	
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//������ģ��
		
		#else
		if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			WifiLockMeg.AlarmMeg = DEFENSE;
			App_WIFI_CommomTx( WIFI_CMD_UPLOAD_ERROR_MEG );
		}
		#endif
		App_GUI_StopStayDetectTim();//ֹͣ���ζ�����ʱ
		GuiQuitTimMs = GUI_TIME_40S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
		if( true == App_GUI_CheckBellVideoAction() )
		{
			if(FaceProAlarm(DEFENSE, WifiTxTemp.data, WifiTxTemp.length) == 1)
			{
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			}
		}
		#endif
		#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//������ģ��
		if( true == App_GUI_CheckBellVideoAction() )//ץ��ͼƬ
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
	#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//������ģ��
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )//��ȡ������ͼƬ�ı��
	{
		if(FaceGneralTaskFlow(FACE_CMD_GETSAVEDIMAGE, 0, 0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
		{
			App_WIFI_UartPowerOn();
			WifiLockMeg.PhotoUploadType = 0x02;//ͼƬ����(1byte)��0x01��������0x02����ץ�ģ�
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
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )//��ȡ���ϴ�ͼƬ��ƫ�����ʹ˴��ϴ�ͼƬ�Ĵ�С
	{
		if(FaceGneralTaskFlow(FACE_CMD_UPLOADIMAGE,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
			GuiQuitTimMs = GUI_TIME_20S;
			WifiLockMeg.PhotoState = WIFI_TX_PREPARE;
			my_printf("FACE_CMD_UPLOADIMAGE\n");
		}
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )//wifi����ͼƬ��һ������
	{
		if(WifiLockMeg.PhotoPackageNum == WifiLockMeg.PhotoPakageSum)//����ͼƬ
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
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )//��ȡ���ϴ�ͼƬ��ƫ�����ʹ˴��ϴ�ͼƬ�Ĵ�С
	{
		if(FaceGneralTaskFlow(FACE_CMD_UPLOADIMAGE,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
			GuiQuitTimMs = GUI_TIME_20S;
			//���ڳ�ʼ��
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
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
			#if defined FACE_FUNCTION_ON && !defined OB_CAM_FUNCTION_ON && !defined ST_CAM_FUNCTION_ON//������ģ��
			WifiLockMeg.PhotoState = WIFI_TX_OVER;
			WifiLockMeg.State = WIFI_TX_OVER;
			#endif
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		#endif
	}
}
 
/*********************************************************************************************************************
* Function Name :  App_GUI_DeployWarmMenu()
* Description   :  ���������澯
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DeployWarmMenu( void )        //���������澯
{
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //����ɨ�� 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_SAFETY_TIP);
		HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, GUI_TIME_15S );	
#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
		if( (0 == HAL_Voice_GetBusyState()) && (FaceProAlarmResult == 1) )  //�����������
		{
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); //������ֹ
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;
		}
		#else
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); //������ֹ
			(void)HAL_Motor_DefendActionCheck(true);
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;
		}
		#endif
		
		if( 0 == GuiDelayTimMs )
		{
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
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
* Description   :  ���������澯
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_FalseLockWarmMenu( void )     //���������澯
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
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //����ɨ��
		
	#ifdef BAT_UNINSET_WARM_TIP_ON
		if( true == HAL_ADC_GetSysVolLowWarmState() )   //ϵͳ��ѹ��
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
			    App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_ON );   //���δ��
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_NOLOCK);
        HAL_Voice_PlayingVoice( EM_FALSE_LOCK_TIPS_MP3, GUI_TIME_3S );	 //��δ����
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
	    if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))  //�Զ���������
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
		
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
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
* Description   :  ��δ�������澯
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_ForgetLockWarmMenu( void )    //��δ�������澯
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
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  //����ɨ��
	    App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
	    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_NOLOCK);
        HAL_Voice_PlayingVoice( EM_FALSE_LOCK_TIPS_MP3, GUI_TIME_3S );	 //��δ����
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
	    if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))  //�Զ���������
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

		if( 0 == HAL_Voice_GetBusyState() )  //�����������
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
* Description   :  ��������ʧ�ܸ澯
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BleOpenErrWarmMenu( void )    //��������ʧ�ܸ澯
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BleOpenErrWarmMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);

		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
* Description   :  �ָ��������ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BackFactoryMenu( void )		  //�ָ���������
{
	static bool optionflg;
    static uint8_t firstFlg;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BackFactoryMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_BACK_ENTER );      //�������������� 
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );    //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_ACT_FACTORY);
		HAL_Voice_PlayingVoice( EM_BACK_FACTORY_COMFIRM_MP3, 0 );  //��������
		
		firstFlg = 0;
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			GuiQuitTimMs = GUI_TIME_60S;
			App_Touch_FuncEnCtrl( EM_SCAN_OFF );      //�������������� 	
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_OFF );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
			return;	
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //���ﰴ�����������
	{
		if( 1 == HAL_Voice_GetBusyState() )
			return;
		HAL_Voice_PlayingVoice( EM_BACKING_FACTORY_MP3, GUI_TIME_2500MS );   
		MenuItem.CurMenuNum = EM_MENU_STEP_3;
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //�ָ�����������
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
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )  //�趨���
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
                PUBLIC_Delayms(3000);//����������3��
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
	if( 0 == GuiQuitTimMs )  //��ʱ����
	{
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP ); 
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_GotoOtaModelMenu()
* Description   :  ����ģʽ
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_GotoOtaModelMenu( void )	  //����ģʽ
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
        my_printf( "App_GUI_GotoOtaModelMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
* Description   :  APP����ģʽ
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_GotoAppModelMenu( void )	  //APP����ģʽ
{
	static bool bleConnectSts = false;
	static bool enterAppMode = false;   //����APPģʽ�ķ�ʽ  false= ble   true= key
	static uint8_t proStep;
	
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_GotoAppModelMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );      //����
		App_LED_OutputCtrl( EM_LED_CFG_NET, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		App_Key_ResetCombinKeyFlow();         //ǿ�Ƹ�λ��ϰ����������
		APP_BleSetAdminState( E_BLE_ADMIN );
		enterAppMode = false;
		/*--ID2��������------*/
		APP_ID2Init();
		my_printf("APP_ID2Init =%d\n", SystemTick);
	    if( !DRV_GetBleConnect() )    //����������״̬
		{
			my_printf( "Ble connect agin!\n" ); 
			APP_BleSetAdminState( E_KEY_ADMIN );
			enterAppMode = true;
			//��ʼ������ֱ�ӿ�ʼ�㲥
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
 
		/*----�����������---*/	
	    APP_BleServerProcess();
		
		/*----�ֻ���������---*/	
		if( E_PHONE_CHECK_OK == APP_BleGetAdminState() )           //success
		{
			APP_BleSetAdminState( E_DEFAULT );
			App_GUI_SetOpenModel( EM_OPEN_PHONE );
			APP_FINGER_Sleep();  //�ر�ָ��ģ��
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		else if( E_PHONE_CHECK_ERR == APP_BleGetAdminState() )     //fail
		{
			APP_FINGER_Sleep();  //�ر�ָ��ģ��
			APP_BleSetAdminState( E_DEFAULT );
			App_GUI_MenuJump( EM_MENU_BLE_OPEN_ERR );
			return;
		}
		/*----����Կ�׿���---*/	
	    if( E_SMARTKEKY_CHECK_OK == APP_BleGetAdminState() )       //success
		{
			APP_BleSetAdminState( E_DEFAULT );
			App_GUI_SetOpenModel( EM_OPEN_BLE_KEY );
			APP_FINGER_Sleep();  //�ر�ָ��ģ��
			App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
			return;
		}
		else if( E_SMARTKEKY_CHECK_ERR == APP_BleGetAdminState() ) //fail
		{
			APP_BleSetAdminState( E_DEFAULT );
			APP_FINGER_Sleep();  //�ر�ָ��ģ��
			App_GUI_MenuJump( EM_MENU_BLE_OPEN_ERR );
			return;
		}	
		
		if( E_VIDEO_ADMIN == APP_BleGetAdminState() )  //������ģʽ
		{
			if( proStep == 0 )
			{
			    App_Touch_FuncEnCtrl( EM_SCAN_ON );  
				App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );      //����
				proStep = 1;
			}
			else if( proStep == 1 )
			{
				BUTTON_TYPE_E ret = App_Key_GetCombinKeyState();  //��ȡ��е������ϼ�
				if( EM_OPEN_DOOR_KEY == ret )          //��������
				{
					APP_BleSetAdminState( E_DEFAULT );
					App_GUI_SetOpenModel( EM_OPEN_BUTTON );
					SystemEventLogSave( KEY_OPEN_IN_DOOR, 0 );  
					App_GUI_MenuJump( EM_MENU_OPEN_DOOR );
					return;
				}
				#ifdef CLOSE_BUTTON_ON
				else if( EM_CLOSE_DOOR_KEY == ret )    //��������
				{
					APP_BleSetAdminState( E_DEFAULT );
					App_GUI_SetCloseModel( EM_CLOSE_BUTTON );
					SystemEventLogSave( KEY_CLOSE_IN_DOOR, 0 );  
					App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
					return;
				}
				#endif

				if( TOUCH_KEY_BACK == App_Touch_GetCurrentKeyValue() )     //back��
				{
					my_printf( "touch key is back!\n" );
					APP_BleSetAdminState( E_DEFAULT );
					App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
					return;
				}
			}
		}
	}
	
    if( !DRV_GetBleConnect() )   //ȷ���Ƿ�Ϊ�Ͽ�
	{
		if( (bleConnectSts == true) && (enterAppMode == false) )
		{
			if( 0 == BleDisconnectHoldTimeMs )
			{
				bleConnectSts = false;
				APP_BleSetAdminState( E_DEFAULT );
				APP_FINGER_Sleep();  //�ر�ָ��ģ��
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				return;
			}
		}
	}

	if( 0 == GuiQuitTimMs )
	{
		my_printf( "gui option timeout!\n" );
		APP_BleSetAdminState( E_DEFAULT );
		APP_FINGER_Sleep();  //�ر�ָ��ģ��
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetSysParaMenu()
* Description   :  ϵͳ�������ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetSysParaMenu( void )		  //ϵͳ��������
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetSysParaMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_SYS_SET_MENU_MP3, 0 );   //��������
		
		GuiQuitTimMs = GUI_TIME_20S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //�������
		{
			App_GUI_MenuJump( EM_MENU_MOTOR_DIR_SET );
			return;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //���Ť��
		{
			App_GUI_MenuJump( EM_MENU_MOTOR_TORSION_SET );
			return;
		}
//		else if( TOUCH_KEY_NO_3 == tp1 ) //�Զ�����
//		{
//			App_GUI_MenuJump( EM_MENU_AUTO_LOCK_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_4 == tp1 ) //˫����֤
//		{
//			App_GUI_MenuJump( EM_MENU_DOUBLE_CHECK_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_5 == tp1 ) //��������
//		{
//			App_GUI_MenuJump( EM_MENU_VOL_ADJUST_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_6 == tp1 ) //�ӽ���Ӧ
//		{
//			App_GUI_MenuJump( EM_MENU_NEAR_SENSE_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_7 == tp1 ) //��������
//		{
//			App_GUI_MenuJump( EM_MENU_DEPPOY_SET );
//			return;
//		}
//		else if( TOUCH_KEY_NO_8 == tp1 ) //��������
//		{
//			App_GUI_MenuJump( EM_MENU_STAY_CHECK_SET );
//			return;
//		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetMotorDirMenu()
* Description   :  ���õ�����ŷ���
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetMotorDirMenu( void )       //�����������
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetMotorDirMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );    //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_DIR_SET_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //��
		{
			SystemFixSeting.MotorDirection = LEFT_HAND_DOOR;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorDirection, sizeof SystemFixSeting.MotorDirection );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������

			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�ҿ�
		{
			SystemFixSeting.MotorDirection = RIGHT_HAND_DOOR;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorDirection, sizeof SystemFixSeting.MotorDirection );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )   //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetMotorTorsionMenu()
* Description   :  ���õ��Ť�� 
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetMotorTorsionMenu( void )   //���Ť������
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetMotorTorsionMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_TORQUE_SET_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //��Ť��
		{
			SystemFixSeting.MotorTorque = LOW_TORQUE;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorTorque, sizeof SystemFixSeting.MotorTorque );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //��Ť��
		{
			SystemFixSeting.MotorTorque = HIGH_TORQUE;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemFixSeting.MotorTorque, sizeof SystemFixSeting.MotorTorque );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetAutoLockMenu()
* Description   :  �����Զ�����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetAutoLockMenu( void )		  //�Զ���������
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
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_ATUO_LOCK_MENU_MP3, 0 ); //��������
		
		GuiQuitTimMs = GUI_TIME_30S;
		playStep = 0;
		errcnt = 0;
	}
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		App_Touch_GetCurrentKeyValBuf( pwddata, &pwdLen ); //�ͷŻ���
		if( TOUCH_KEY_NO_1 == tp1 )     //ʱ������
		{
			HAL_Voice_PlayingVoice( EM_INPUT_2NUM_AND_COMFIRM_MP3, 0 ); //��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�ر��Զ���������
		{
			SystemSeting.SysAutoLockTime = 0;
			tp1 = SystemWriteFixSeting( (uint8_t *)&SystemSeting.SysAutoLockTime, sizeof SystemSeting.SysAutoLockTime );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_11;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )  //�趨ʱ��
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ʱ������
		{
			App_Touch_GetCurrentKeyValBuf( pwddata, &pwdLen ); //�ͷŻ���
			paraSet = pwddata[0]*10 + pwddata[1]; 
			if( (pwdLen == 2) && (paraSet >= 10) && (paraSet <= 99) )
			{
				HAL_Voice_PlayingVoice( EM_AUTO_LOCK_TIME_MP3, GUI_TIME_2500MS );//��������
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
					HAL_Voice_PlayingVoice( EM_FORMAT_WRONG_TIPS_MP3, GUI_TIME_600MS );//��������
					GuiQuitTimMs = GUI_TIME_15S;
				}
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )  //�趨ʱ��
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ʱ��ȷ��
		{
			SystemSeting.SysAutoLockTime = paraSet;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysAutoLockTime, sizeof SystemSeting.SysAutoLockTime );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_11;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
		
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			if( playStep < 0xFE )
				playStep++;
			if( playStep == 1 )
			{
				HAL_Voice_PlayingVoice( (VoiceType_E)(EM_NUM_0_MP3+pwddata[0]), GUI_TIME_400MS );   //ʮλ��
			}
			else if( playStep == 2 )
			{
				HAL_Voice_PlayingVoice( EM_NUM_10_MP3, GUI_TIME_400MS ); //ʮ
			}
			else if( playStep == 3 )
			{
				if( pwddata[1] != 0 )
				{
					HAL_Voice_PlayingVoice( (VoiceType_E)(EM_NUM_0_MP3+pwddata[1]), GUI_TIME_400MS );//��λ��
				}
				else
				{
					HAL_Voice_PlayingVoice( EM_SECOND_MP3, GUI_TIME_400MS );//��
				}
			}
			else if( playStep == 4 )
			{
				if( pwddata[1] != 0 )
				{
					HAL_Voice_PlayingVoice( EM_SECOND_MP3, GUI_TIME_400MS );//��
				}
			}
			else if( playStep == 5 )
			{
				HAL_Voice_PlayingVoice( EM_COMFIRM_OR_BACK_TIPS_MP3, 0 );//ȷ������
				GuiQuitTimMs = GUI_TIME_15S;
			}
		}
	}
    else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum )  //�趨ʱ��
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetDoubleCheckMenu()
* Description   :  ����˫����֤ 
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetDoubleCheckMenu( void )	  //˫����֤����
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetDoubleCheckMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_VERIFY_SET_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //����
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
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�ر�
		{
			SystemSeting.SysCompoundOpen = DOUBLE_CHECK_SW_OFF;
			uint8_t ret = SystemWriteSeting( (uint8_t *)&SystemSeting.SysCompoundOpen, sizeof SystemSeting.SysCompoundOpen );
			SystemSeting.SysLockMode = 0;
			ret = SystemWriteSeting( (uint8_t *)&SystemSeting.SysLockMode, sizeof SystemSeting.SysLockMode );
			VoiceType_E mp3 = (ret == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetVolGradeMenu()
* Description   :  ���������ȼ�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetVolGradeMenu( void )       //������������
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetVolGradeMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_VOL_SET_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //������
		{
			SystemSeting.SysVoice = HIGH_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH ); 
			}
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //������
		{
			SystemSeting.SysVoice = MEDIUM_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_MED ); 
			}
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //������
		{
			SystemSeting.SysVoice = LOW_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_LOW ); 
			}
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //����ģʽ
		{
			SystemSeting.SysVoice = OFF_VOICE_VOL;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysVoice, sizeof SystemSeting.SysVoice );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			if( tp1 == 1 )
			{
				HAL_Voice_VolumeSet( EM_VOL_GRADE_OFF ); 
			}
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetNearSenseMenu()
* Description   :  ���ýӽ���Ӧ
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetNearSenseMenu( void )	  //�ӽ���Ӧ����
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetNearSenseMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_NEAR_REACT_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //����
		{
			SystemSeting.SysDrawNear = FUNCTION_ENABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysDrawNear, sizeof SystemSeting.SysDrawNear );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�ر�
		{
			SystemSeting.SysDrawNear = FUNCTION_DISABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysDrawNear, sizeof SystemSeting.SysDrawNear );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SetDeployMenu()
* Description   :  ���ò�������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetDeployMenu( void )		  //��������
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetDeployMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_DEPLOY_SET_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //����
		{
			SystemSeting.SysKeyDef = FUNCTION_ENABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysKeyDef, sizeof SystemSeting.SysKeyDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�ر�
		{
			SystemSeting.SysKeyDef = FUNCTION_DISABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysKeyDef, sizeof SystemSeting.SysKeyDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
} 

/*********************************************************************************************************************
* Function Name :  App_GUI_SetStayCheckMenu()
* Description   :  ���ö�������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SetStayCheckMenu( void )	  //��������
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_SetStayCheckMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );        //����
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		HAL_Voice_PlayingVoice( EM_STAY_SET_MENU_MP3, 0 );//��������
		
		GuiQuitTimMs = GUI_TIME_16S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 )     //����
		{
			SystemSeting.SysHumanIrDef = 30;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysHumanIrDef, sizeof SystemSeting.SysHumanIrDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�ر�
		{
			SystemSeting.SysHumanIrDef = FUNCTION_DISABLE;
			tp1 = SystemWriteSeting( (uint8_t *)&SystemSeting.SysHumanIrDef, sizeof SystemSeting.SysHumanIrDef );
			VoiceType_E mp3 = (tp1 == 1) ? EM_SET_SUCCESS_MP3 : EM_SET_FAIL_MP3;
			HAL_Voice_PlayingVoice( mp3, GUI_TIME_3S );//��������
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //����
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}	
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		App_GUI_MenuJump( EM_MENU_SYSTEM_PARA_SET );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_ErrorCheckMenu()
* Description   :  ϵͳ���ϼ�⴦������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_ErrorCheckMenu( void )		  //ϵͳ���ϼ��	
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
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )       //PIR����
	{
        uint8_t ret = CAM_PirWakeUpTest();
	    if(1 == ret)
        {
			App_LED_OutputCtrl( EM_LED_0, EM_LED_ON );
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON );
			App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_ON );
		    HAL_Voice_PlayingVoice( EM_BUTTON_TIPS_MP3, 0 );	
            MenuItem.CurMenuNum = EM_MENU_STEP_2;
            GuiDelayTimMs = GUI_TIME_3S;//3S��ʱ�˳�
        }
        else if(2 == ret)
        {
			App_LED_OutputCtrl( EM_LED_9, EM_LED_ON );
		    HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 0 );	
            MenuItem.CurMenuNum = EM_MENU_STEP_3;
            GuiDelayTimMs = GUI_TIME_3S;//3S��ʱ�˳�
        }
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //PIR����,è�۷��سɹ�
	{
		if(GuiDelayTimMs == 0)
        {
		    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );
		    (void)App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
            return;
        }  
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) //PIR����,è�۷���ʧ��
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
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
	return;
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SmartScreenShowMenu
* Description   :  ����������չʾ
* Para          :  ��
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
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //PIR����,è�۷��سɹ�
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
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
	}
#endif
	return;
}


/*********************************************************************************************************************
* Function Name :  App_GUI_BellVideoMenu()
* Description   :  ������Ƶ�����˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BellVideoMenu( void )		  //������Ƶ�����˵�	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_BellVideoMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		GuiDelayTimMs = GUI_TIME_10S;
		if(E_WAKE_CAMERA_WIFI != App_GUI_GetSysWakeupType())
		{
			(void)HAL_Voice_PlayingVoice(EM_CALLING_WAIT_HANG_UP_BY_BACK_MP3, 300);
		}
	}
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
	if(EM_MENU_STEP_1 == MenuItem.CurMenuNum)
	{
		if( E_WAKE_CAMERA_WIFI == App_GUI_GetSysWakeupType() )
		{
			 if(FaceProCallBell(0) == 1)//�ֻ��˷���
			 {
				App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
				 return;
			 }
		}
		else if(FaceProCallBell(1) == 1) //����������
		{
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
		uint8_t keynum = App_Touch_GetCurrentKeyValue();
		if( (TOUCH_KEY_BACK == keynum) && (FaceWifiStatus.media_state == 0x02) )  //����ģ�����ڹ����а����ؼ��ſ��˳�
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
		if(TOUCH_KEY_BACK == keynum)
		{
			my_printf("Modual_State = %d\n", Modual_State);
		}
		if((TOUCH_KEY_BACK == keynum) && (Modual_State == MODUAL_ERROR))//ģ��û������
		{
			Modual_State = MODUAL_DEAFAULT;
			App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		}
	}

	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) //�ϵ�
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
* Description   :  ����ͬ������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_NetworkUpdateMenu( void )	  //����ͬ������	
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		App_LED_OutputCtrl( EM_LED_0, EM_LED_OFF ); 
		GuiQuitTimMs = GUI_TIME_30S;
		GuiDelayTimMs = GUI_TIME_400MS;
		if( SystemPowerOnFlg == true )  //�״��ϵ� 
		{
			HAL_Voice_PlayingVoice( EM_NETWORKING_WAIT_MP3, 0 );//�����У����Ժ�
		}
	}
	
	if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )    //�ɱ���Ϸ�ʽ
	{
		uint8_t ret = FaceProNetworking();

		if(0 == ret)  //�������紦����.����������
		{
			if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )        //OPEN KEY��ϴ���
			{
				my_printf( "break by open button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
				return;
			}
			else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  //CLOSE KEY��ϴ���
			{
				my_printf( "break by close button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
				MenuItem.CurMenuNum = EM_MENU_STEP_2; 
				return;
			}
			else if( 1 == App_Export_GetPinState( E_PIN_TOUCH_IRQ ) )//touch���
			{
				my_printf( "break by touch!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_TOUCH );  
				MenuItem.CurMenuNum = EM_MENU_STEP_2;   
				return;
			}
			else if( 1 == App_Input_GetPinState( E_INPUT_FINGER_IRQ ) ) //finger���
			{
				if( SystemPowerOnFlg == false )  //���״��ϵ� 
				{
					my_printf( "break by finger!\n" ); 
					App_GUI_SetSysWakeupType( E_WAKE_FINGER );  
					MenuItem.CurMenuNum = EM_MENU_STEP_2;   
					return;
				}
			}
		}
		else if(1 == ret) //����������ɡ�
		{
			if( false == App_GUI_GetNetworkErrState() )  //������������
			{
				s_GuiData.NetUpdateFuncEnSts = false;
				s_GuiData.NetUpdateSetTimPara = NET_CONNECT_FIRST_TIME_S;
				s_GuiData.NetUpdatePeroidTim = s_GuiData.NetUpdateSetTimPara;
			}
			
			if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
		else if(2 == ret) //��������ʧ�ܡ�
		{
			my_printf( "gui option timeout!\n" );
			/*--------���¼��ʱ��----------*/
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
			
			if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		/*--------���¼��ʱ��----------*/
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
		
		if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
* Description   :  ����Ԥ��ͬ������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_WeatherUpdateMenu( void )	  //����Ԥ��ͬ������	
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_WeatherUpdateMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		if( true == App_GUI_GetWifiWarmingSwSts() )
		{ 
			App_WIFI_CommomTx( WIFI_CMD_QUERY_CLOUD_DATA );
		}
		GuiQuitTimMs = GUI_TIME_30S;
 
	}
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
	if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
	
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )    //�ɱ���Ϸ�ʽ
	{
		 uint8_t tp = FaceProQueryCloudData(WifiTxTemp.data, WifiTxTemp.length);
		 if(tp == 1)//�ɹ���ȡ������Ϣ
		 {
		    /* ͬ������Ĭ�ϱ��� */
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
			if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
			if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
	if( 0 == GuiQuitTimMs )  //��ȡ������Ϣʧ�ܣ�GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		s_GuiData.WeatherUpdateErrCnt++;
		s_GuiData.WeatherUpdatePeroidTim =60*60;
		
		if( SystemPowerOnFlg == true )  //�״��ϵ� 
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
* Description   :  �������ʾ����         ������/��Ĥ���Ż����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_BellLampMenu( void )	  	  //�������ʾ����	
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_ON );//���������
		#ifdef LOCK_KEY_WHITE_LED_ON
		App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		autoLockCheckFlg = false;
		GuiQuitTimMs = GUI_TIME_5S;
	}
 
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )   
	{
	   /*--------------�ӽ���Ӧ�������-------*/	
		if( 1 == App_Export_GetPinState( E_PIN_SENSE_IRQ ) )      
		{
			GuiQuitTimMs = GUI_TIME_5S;
		}
		/*-----------������ʽ����APPģʽ------*/	
		if( DRV_GetBleConnect() )    
		{
			my_printf( "wakeup by blue!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_BLE_COM );   	
			SystemWakeupInit();   
			return;
		}
	   /*--------------�����廽��-------------*/	
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_BELL == tp1 )     //�����	
		{
			HAL_Voice_BellCtrl( true );
			#ifdef XM_CAM_FUNCTION_ON   //��è��
			if( 1 == App_GUI_CheckBellVideoAction() )
			{
				CAM_SendCommandStart(CAM_CMD_BELL, 0, 0);
			}
			#endif 
		}
		else if( TOUCH_KEY_NONE != tp1 )//������ֵ
		{
			my_printf( "wakeup by touch keyboard!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_TOUCH );   	
			SystemWakeupInit();   
			return;
		}
	   /*--------------���ڿ��Ű�������-------*/
		else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   
		{
			my_printf( "wakeup by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------���ڹ��Ű�������-------*/
		else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  
		{
			my_printf( "wakeup by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
			SystemWakeupInit();   
			return;
		}
	   /*--------------ָ�ƻ���---------------*/
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	    
		{
			my_printf( "wakeup by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit();    
			return;
		}
	   /*---------���Զ������Զ���������-------*/
		else if( 1 == App_GUI_GetAutoLockSts() )   
		{
			my_printf( "wakeup by not autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );   	
			return;
		}
	   /*---------�Զ������Զ���������--------*/
		else if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))   
		{
			my_printf( "wakeup by autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  	
			return;
		}
	   /*-----------����������������----------*/
		else if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
		{
			s_GuiData.StayDetectFlg = 0;
			my_printf( "wakeup by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  
			SystemWakeupInit();  
			return;
		}	
	   /*-------------������ת����------------*/
	    else if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE ) ) 
		{
			my_printf( "wakeup by left handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_LEFT_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------�����м份��------------*/
	    else if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE ) ) 
		{
			my_printf( "wakeup by middle handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_MIDDLE_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------������ת����------------*/
	    else if( 1 == App_Key_GetCloseHandleSts() )            
		{
			my_printf( "wakeup by right handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_RIGHT_HANDLER );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------������������------------*/
		else if( true == HAL_Motor_FalseLockWarmCheck() )      
		{
			my_printf( "wakeup by false lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------�������汨������--------*/
		else if( true == HAL_Motor_HandleTryForbitWarmCheck() )//�������汨��
		{
			my_printf( "wakeup by handle try forbit!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------��δ����������----------*/
		else if( true == HAL_Motor_ForgetLockWarmCheck() )    //��δ������
		{
			my_printf( "wakeup by forget lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-----------���������澯����----------*/
		else if( 1 == HAL_Motor_DefendActionCheck(false) )   //����Ƿ񴥷������澯
		{
			my_printf( "auto lock pus door!\n"); 
			App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
		    SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
			if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //����״̬
			{
				my_printf( "wakeup by defend warm!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_DEPLOY_WARM );  
				SystemWakeupInit();  
				return;
			}
		}
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_WakeButSleepMenu()
* Description   :  ���Ѻ��޸�֪����   �����������Ѵ��� 
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_WakeButSleepMenu( void )	  //���Ѻ��޸�֪����
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//���������
		#ifdef LOCK_KEY_WHITE_LED_ON
		App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		autoLockCheckFlg = false;
		GuiQuitTimMs = GUI_TIME_5S;
	}
 
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )   
	{
	   /*--------------�����廽��-------------*/
		if( 1 == App_Export_GetPinState( E_PIN_TOUCH_IRQ ) )      
		{
			my_printf( "wakeup by touch keyboard!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_TOUCH );   	
			SystemWakeupInit();   
			return;
		}
		/*-----------������ʽ����APPģʽ------*/	
		else if( DRV_GetBleConnect() )    
		{
			my_printf( "wakeup by blue!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_BLE_COM );   	
			SystemWakeupInit();   
			return;
		}
	   /*--------------���ڿ��Ű�������-------*/
		else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   
		{
			my_printf( "wakeup by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
			SystemWakeupInit();  
			return;
		}
	   /*--------------���ڹ��Ű�������-------*/
		else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  
		{
			my_printf( "wakeup by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
			SystemWakeupInit();   
			return;
		}
	   /*--------------ָ�ƻ���---------------*/
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	    
		{
			my_printf( "wakeup by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit();    
			return;
		}
	   /*---------���Զ������Զ���������-------*/
		else if( 1 == App_GUI_GetAutoLockSts() )   
		{
			my_printf( "wakeup by not autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );   	
			return;
		}
	   /*---------�Զ������Զ���������--------*/
		else if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))   
		{
			my_printf( "wakeup by autolock!\n" ); 
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  	
			return;
		}
	   /*-----------����������������----------*/
		else if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
		{
			s_GuiData.StayDetectFlg = 0;
			my_printf( "wakeup by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  
			SystemWakeupInit();  
			return;
		}	
	   /*-------------������ת����------------*/
	    else if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE ) ) 
		{
			my_printf( "wakeup by left handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_LEFT_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------�����м份��------------*/
	    else if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE ) ) 
		{
			my_printf( "wakeup by middle handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_MIDDLE_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	   /*-------------������ת����------------*/
	    else if( 1 == App_Key_GetCloseHandleSts() )            
		{
			my_printf( "wakeup by right handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_RIGHT_HANDLER );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------������������------------*/
		else if( true == HAL_Motor_FalseLockWarmCheck() )      
		{
			my_printf( "wakeup by false lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------�������汨������--------*/
		else if( true == HAL_Motor_HandleTryForbitWarmCheck() )//�������汨��
		{
			my_printf( "wakeup by handle try forbit!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );   					
			SystemWakeupInit();   
			return;
		}
	   /*-------------��δ����������----------*/
		else if( true == HAL_Motor_ForgetLockWarmCheck() )    //��δ������
		{
			my_printf( "wakeup by forget lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FORGET_LOCK );   					
			SystemWakeupInit();   
			return;
		}
	   /*-----------���������澯����----------*/
		else if( 1 == HAL_Motor_DefendActionCheck(false) )   //����Ƿ񴥷������澯
		{
			if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //����״̬
			{
				my_printf( "wakeup by defend warm!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_DEPLOY_WARM );  
				SystemWakeupInit();  
				return;
			}
		}
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( MENU_SYSTEM_SLEEP );
		return;
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_ErrorCheckMenu()
* Description   :  �ϻ���������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AginTestMenu( void )		  //ϵͳ�ϻ�����
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_AginTestMenu()\n" ); 
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
 
		#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON 
		(void)FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S); 
		#endif
		
		GuiQuitTimMs = GUI_TIME_12S;
	}

	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );       //����
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
		 APP_FINGER_Operate( fingerPara );   //������ָ֤������
		 AgingTestTimSec = GUI_TIME_2S;
		 MenuItem.CurMenuNum = EM_MENU_STEP_4;
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )
	{
		if( AgingTestTimSec == 0 )
		{
			APP_FINGER_Sleep();  //�ر�ָ��ģ��
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
		}
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )  //open door
	{
        int8_t tp1 = HAL_Motor_ForceOpenDoorThread();
        if( 1 == tp1 )      //�������̽���
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
		}
		else if( 2 == tp1 ) //������+б������
		{
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
			App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_ON ); 	
			App_LED_OutputCtrl( EM_LED_LOCK_G, EM_LED_ON );
			HAL_Voice_PlayingVoice( EM_OPEN_DOOR_OK_MP3, GUI_TIME_1500MS );
		}
		else if( -1 == tp1 )//���ŷ�������쳣
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
        if( 1 == tp1 )      //�������̽���
		{
			HAL_Voice_PlayingVoice( EM_LOCKED_DOOR_MP3, GUI_TIME_2S );
			MenuItem.CurMenuNum = EM_MENU_STEP_8;
		}
		else if( -1 == tp1 )//���ŷ�������쳣
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
* Description   :  ϵͳ���߲˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_GotoSleepModelMenu( void )	  //ϵͳ���߲˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif

		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON   //����è���Ƿ�������
		if( App_GUI_CheckNetworkAction() )  //ִ������ͬ���ж�
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
		
		/*------------�ر�����ͬ��-------------*/
		#ifdef WEATHER_FORECAST_ON  //����ͬ������
		uint8_t tp1 = App_GUI_CheckWeatherUpdateAction();
		if( 0 == tp1 )      //��ִ��ͬ��
		{
			s_GuiData.WeatherUpdateErrCnt = 0;
			s_GuiData.WeatherUpdateEnSts =0;
		}
		else if( 1 == tp1 )  //ִ��ͬ��
		{
			s_GuiData.WeatherUpdateEnSts =1;
		}				
		#endif 

        if( 1 == HAL_Motor_ReleaseTryProtectCheck())
	    {
		    App_GUI_RelieveTryProtect();  //������� 
	    }
		GuiQuitTimMs = GUI_TIME_10S;
		GuiDelayTimMs = GUI_TIME_100MS;
		autoLockCheckFlg = false;
		App_GUI_SetSysWakeupType( E_WAKE_DEFAULT );                //�������Դ  
		
		/*--BLE��ʼ��------*/
		SystemAppBleInit();      //��Ҫ��app_timer_init��
	}
    if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //�ȴ������������
	{
		if( (0 == HAL_Voice_GetBusyState()) || (0 == GuiDelayTimMs) )
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		/*------------------���˽��-----------------*/	 
		if( true == App_GUI_CheckAlarmButtonRecoveryWarm() )
		{
			my_printf("alrm button is recovery!\n");
			HAL_Voice_WorkModeCtrl( false );//�����˳�����ģʽ
			HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 );  
		}
		
		if( 1 == App_GUI_GetAutoLockSts() )  //���Զ������Զ�����
		{
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );   	
			return;
		}
		else if( 1 == HAL_Motor_AutoLockCheck( &autoLockCheckFlg ))  //�Զ���������
		{
			App_GUI_SetCloseModel( EM_CLOSE_AUTO );
			SystemEventLogSave( AUTO_CLOSE_DOOR, 0 );  
			App_GUI_MenuJump( EM_MENU_CLOSE_DOOR );
			App_GUI_RelieveTryProtect();  //�������
			App_GUI_SetSysWakeupType( E_WAKE_AUTO_LOCK );  	
			return;
		}
		else if( 1 == HAL_Motor_DefendActionCheck(false) )   //����Ƿ񴥷������澯
		{
			my_printf( "auto lock pus door!\n"); 
			App_GUI_SetOpenModel( EM_OPEN_HANDLER );   
			SystemEventLogSave( BAC_OPEN_IN_DOOR, 0 );  
			if( SystemSeting.SysKeyDef == FUNCTION_ENABLE )  //����״̬
			{
				my_printf( "break by defend warm!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_DEPLOY_WARM );  
				SystemWakeupInit();  
				return;
			}
		}
		 /*------------------������������-------------*/	
		else if( (s_GuiData.StayDetectFlg == 1)||( true == App_GUI_CheckStayDetectAction()) )
		{
			s_GuiData.StayDetectFlg = 0;
			my_printf( "wake by auto stay detect!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_STAY_DEFENSE );  
			SystemWakeupInit();  
			return;
		}
		
        if( 1 == APP_WIFI_TxState() )         //ȷ��WIFI�������
		{
	        if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   //���ڿ��Ű�����ϴ���
			{
				my_printf( "break by open button!\n" ); 
				App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
				SystemWakeupInit();  
				return;
			}
			else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  //���ڹ��Ű�����ϴ���
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
		if( 0 == HAL_EXPORT_PinGet( EM_KEY_IRQ ) )        //����������ϴ���
		{
			my_printf( "break by touch!\n" ); 
		    App_GUI_SetSysWakeupType( E_WAKE_TOUCH );  
			SystemWakeupInit();   
			return;
		}
	  #ifdef FINGER_VEIN_FUNCTION_ON
		else if( 0 == HAL_EXPORT_PinGet( EM_FING_IRQ ) )   	   //ָ�ƴ�ϴ���
		{
			my_printf( "break by finger!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_FINGER );    	
			SystemWakeupInit();    
			return;
		}
	  #endif
	    else if( 1 == App_Key_GetKeyValidState( OPEN_KEY ) )   //���ڿ��Ű�����ϴ���
		{
			my_printf( "break by open button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_OPEN_BUTTON );  
			SystemWakeupInit();  
			return;
		}
	    else if( 1 == App_Key_GetKeyValidState( CLOSE_KEY ) )  //���ڹ��Ű�����ϴ���
		{
			my_printf( "break by close button!\n" ); 
			App_GUI_SetSysWakeupType( E_WAKE_CLOSE_BUTTON );  
			SystemWakeupInit();   
			return;
		}
	    else if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE ) )//������ת��ϴ���
		{
			my_printf( "break by left handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_LEFT_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	    else if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE ) )//�����м��ϴ���
		{
			my_printf( "break by middle handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_MIDDLE_HANDLER );   				
			SystemWakeupInit();   
			return;
		}
	    else if( 1 == App_Key_GetCloseHandleSts() )            //������ת��ϴ���
		{
			my_printf( "break by right handle!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_RIGHT_HANDLER );   					
			SystemWakeupInit();   
			return;
		}
		else if( true == HAL_Motor_FalseLockWarmCheck() )      //��������
		{
			my_printf( "break by false lock!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_FALSE_LOCK );   					
			SystemWakeupInit();   
			return;
		}
		else if( true == HAL_Motor_HandleTryForbitWarmCheck() )//�������汨��
		{
			my_printf( "break by handle try forbit!\n" );  	
			App_GUI_SetSysWakeupType( E_WAKE_HANDLE_TRY );   					
			SystemWakeupInit();   
			return;
		}
		else if( true == HAL_Motor_ForgetLockWarmCheck() )    //��δ������
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
    else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum )  //ȷ�Ϸ��˱���
    {
		if( true == App_Export_GetAlrmWarmState() && 1 == HAL_ADC_GetVolLowGradeErr() )//���˴�ϴ��� (�ǵ�ѹ����)
		{
			my_printf( "sleep break by alarm!\n" ); 
			App_GUI_MenuJump( EM_MENU_ALARM_WARM );
			App_GUI_SetSysWakeupType( E_WAKE_ALARM );   	
			return;
		}
		MenuItem.CurMenuNum = EM_MENU_STEP_4;
    }
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )  //ϵͳ����
	{
        APP_SCREEN_Sleep();
		SystemSleepInit();              //�������� 
		App_GUI_SetSysSleepSts( true ); //ϵͳ����
		MenuItem.CurMenuNum = EM_MENU_STEP_5;
		my_printf("/*---------------sleepping--------------------*/\r\n");	
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum )
	{
		#if LOCK_PROJECT_CHIP == LOCK_PROJECT_RTL8762
		if( App_GUI_GetSysSleepSts()==0 ) //���ܻ�������STEP
		{
			Dlps_Enabled=false; //����
			MenuItem.CurMenuNum = EM_MENU_STEP_6;
		}
		else
		{
			DRV_ConncetCallBackReg(BleISRhandler); //ע�������������		
			DRV_SysMgmt_Run();	//ʹ��˯��
		}
		#else
		while( App_GUI_GetSysSleepSts() )  
		{
			DRV_SysMgmt_Run();	
			if(DRV_GetBleConnect())   //ȷ���Ƿ�Ϊ��������
			{
				if( E_WAKE_DEFAULT == App_GUI_GetSysWakeupType() )
				{
					App_GUI_SetSysSleepSts(false);
					App_GUI_SetSysWakeupType( E_WAKE_BLE_COM );   //�趨����Դ
					my_printf( "wake up source is ble!\n" ); 
				}
			}
		}
		SystemWakeupInit();  //ϵͳ���Ѵ��� 
		#ifdef SMARTKEY_XRO_ENC_ON  //����㲥
		if( E_WAKE_BLE_COM != App_GUI_GetSysWakeupType() )  //������������ 
		{
			if(SystemSeting.SystemAdminRegister==ADMIN_APP_REGISTERED)//��ע��
			{
				APP_BleInit(REGISTERED,LIMITED_FLAGS); //�㲥�ĳ�020105 ��ɫԿ����
			}
		}
		#endif
		return;
		#endif	
	}
	else if( EM_MENU_STEP_6 == MenuItem.CurMenuNum )
	{
#if LOCK_PROJECT_CHIP == LOCK_PROJECT_RTL8762
		DRV_ConncetCallBackReg(0); //�����������ر�
#endif	
		SystemWakeupInit();  //ϵͳ���Ѵ��� 
	}
    return;
}
 

/*********************************************************************************************************************
* Function Name :  App_GUI_AddUserMenu()
* Description   :  �����û��˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddUserMenu( void )		  //�����û��˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
		if( TOUCH_KEY_NO_1 == tp1 )      //���ӹ���Աָ��
		{
		    if( SystemSeting.SysFingerAdminNum >= MSG_FINGER_ADMIN_LOCAL_NUM )       //�Ǽ���������
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
		else if( TOUCH_KEY_NO_2 == tp1 ) //������ͨ�û�ָ��
		{
		    if( SystemSeting.SysFingerGuestNum >= MSG_FINGER_COMMON_LOCAL_GUINUM )   //�Ǽ���������
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
		else if( TOUCH_KEY_NO_3 == tp1 ) //���ӿ�
		{
			if( SystemSeting.SysCardAllNum >= MSG_CPU_CARD_USER_NUM )   //�Ǽ���������
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
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			DispFuncPtrPre = NULL;
			return;
		}
	}
	else if( EM_MENU_STEP_11 == MenuItem.CurMenuNum ) 
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_FINGER_ADD_ADMIN );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DeleteUser()
* Description   :  ɾ���û��˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DeleteUser( void )		  //ɾ���û��˵�
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DeleteUser()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
        App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
		if( TOUCH_KEY_NO_1 == tp1 )      // ɾ����ͨ�û�ָ��
		{
			App_GUI_MenuJump( EM_MENU_FINGER_DELETE_GUEST );
			return;
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //ɾ����
		{
			App_GUI_MenuJump( EM_MENU_CARD_DELETE );
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) 
	{

	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DeleteUser()
* Description   :  ϵͳʱ�����ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SystemTimeSetting( void )		  //ϵͳʱ�����ò˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK ); 
		#if defined  FINGER_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_SYSTEM_DATE_SET_MENU_MP3, 0 );
		#elif defined  FINGER_VEIN_FUNCTION_ON
		HAL_Voice_PlayingVoice( EM_VEIN_SET_MENU_MP3, 0 );	
		#endif

		
		GuiQuitTimMs = GUI_TIME_50S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )  //����������8λ����
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
			if( tp1 == 8 )//�����ʽ��ȷ��������
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
				HAL_Voice_PlayVoiceNum( EM_MONTH_MP3 );//��
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
				HAL_Voice_PlayVoiceNum( EM_DAY_MP3);//��
				PUBLIC_Delayms(400);
				MenuItem.CurMenuNum = EM_MENU_STEP_2;
				HAL_Voice_PlayingVoice( EM_CHANGE_PRESS_RETURN_KEY_MP3, 0);/*---ȷ���밴#��,��������밴���ؼ�---*/
			}
			else//������ʾ����ʽ����ȷ��
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				MenuItem.CurMenuNum = EM_MENU_STEP_5;
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum ) //ȷ��������ʱ����6λ����
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			rtc_w.year =(inputBuf[2]<<4)|inputBuf[3];
            rtc_w.month =(inputBuf[4]<<4)|inputBuf[5];
            rtc_w.day = (inputBuf[6]<<4)|inputBuf[7];
			HAL_Voice_PlayingVoice( EM_SYSTEM_VOICE_PROMPTS_TIME_MP3, 0);//������ʱ����6λ����,��ȷ�ϼ�����
			GuiQuitTimMs = GUI_TIME_50S;
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    DispFuncPtrPre = NULL; //������һ��
			return;		
		}
		
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum ) //����ʱ����6λ����
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( &inputBuf[8], &buflen ); 
			if( tp1 == 6 )//ʱ����6λ����
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
				HAL_Voice_PlayVoiceNum( EM_HOUR_MP3 );//ʱ
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
				HAL_Voice_PlayVoiceNum( EM_MINUTER_MP3 );//��
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
				HAL_Voice_PlayVoiceNum( EM_SECOND_MP3 );//��
				PUBLIC_Delayms(400);
				HAL_Voice_PlayingVoice( EM_CHANGE_PRESS_RETURN_KEY_MP3, GUI_TIME_5S);/*---ȷ���밴#��,��������밴���ؼ�---*/
				MenuItem.CurMenuNum = EM_MENU_STEP_4;
			}
			else//������ʾ����ʽ����ȷ�����������롱
			{
				PUBLIC_Delayms(400);
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1500MS);
				GuiQuitTimMs = GUI_TIME_50S;
				MenuItem.CurMenuNum = EM_MENU_STEP_3;
				return;
			}
		}
	}
	else if( EM_MENU_STEP_4 == MenuItem.CurMenuNum ) //ȷ��ʱ��
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			rtc_w.hour =(inputBuf[8]<<4)|inputBuf[9];
            rtc_w.minuter =(inputBuf[10]<<4)|inputBuf[11];
            rtc_w.second = (inputBuf[12]<<4)|inputBuf[13];
			rtc_w.week = 1;
			HAL_RTC_WriteTime(&rtc_w);
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS);//�趨�ɹ�
			MenuItem.CurMenuNum = EM_MENU_STEP_5;
			return;	
		}
		else if( TOUCH_KEY_BACK == tp1 ) 	
		{
		    HAL_Voice_PlayingVoice( EM_SYSTEM_VOICE_PROMPTS_TIME_MP3, 0);//������ʱ����6λ����,��ȷ�ϼ�����
			GuiQuitTimMs = GUI_TIME_50S;
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;		
		}
	}
	else if( EM_MENU_STEP_5 == MenuItem.CurMenuNum ) //����������ʱ
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
        if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DeleteUser()
* Description   :  ������ʽ���ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_UnlockWaySetting( void )		  //������ʽ���ò˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
		if( TOUCH_KEY_NO_1 == tp1 ) // ָ�ƿ���
		{
			SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_ON; 
		    SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //д��Ͽ���
			SystemSeting.SysLockMode = 0x02; //�ڶ����ֽ���ģʽ
			SystemWriteSeting(&SystemSeting.SysLockMode,1);
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) // ָ�ƻ򿨻����뿪��
		{
			SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_OFF; 
		    SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //д��Ͽ���
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) // ָ�Ƽ����뿪��
		{
			SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_ON; 
		    SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //д��Ͽ���
			SystemSeting.SysLockMode = 0x03; //�ڶ����ֽ���ģʽ
			SystemWriteSeting(&SystemSeting.SysLockMode,1);
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{		
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_VoiceSetting()
* Description   :  �������ò˵�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_VoiceSetting( void )		  //�������ò˵�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
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
		if( TOUCH_KEY_NO_1 == tp1 ) // ������
		{
			SystemSeting.SysVoice=LOW_VOICE_VOL; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //������
		{
			SystemSeting.SysVoice=MEDIUM_VOICE_VOL; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_3 == tp1 ) //������
		{
			SystemSeting.SysVoice=HIGH_VOICE_VOL; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, GUI_TIME_1500MS );
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	else if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{		
			App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;
		}
	}

	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_AddCardMenu()
* Description   :  ���ӿ�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_AddCardMenu( void )	  //���ӿ�
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //������3λ���ֱ��,��ȷ�ϼ�ȷ��
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//�����ʽ��ȷ3λ���ֱ��
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//����Ѵ������������� 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//��ˢ�� 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//������ʾ����ʽ����ȷ���������롱
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //������һ��
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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
		if(result == CPUCARD_ADD_SUCCESSFUL)// ���ӳɹ�
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_2S );//�Ǽǳɹ�
			MenuItem.CurMenuNum = EM_MENU_STEP_3;		
		}
		else if(result == CPUCARD_ADD_REGISTERED)// ��ע��
		{
			HAL_Voice_PlayingVoice( EM_CARD_REGISTED_MP3, GUI_TIME_2S );//����ע��   
			MenuItem.CurMenuNum = EM_MENU_STEP_3;			
		}
		else if(result == CPUCARD_ADD_ERROR)// ����ʧ��
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_2S );//�Ǽ�ʧ��
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_ADD_USER );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_DelFaceMenu()
* Description   :  ɾ����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelCardMenu( void )			  //ɾ����
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		HAL_Voice_PlayingVoice( EM_DEL_TYPE_OPT_MP3, GUI_TIME_4S );//ɾ������ѡ����ʾ( ���ɾ���밴1,�ȶ�ɾ���밴2 ) 
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
		if( TOUCH_KEY_NO_1 == tp1 ) //���ɾ��
		{
			HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //������3λ���ֱ��,��ȷ�ϼ�ȷ��
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�Ա�ɾ��
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//��ˢ�� 
			App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_SYSTEM_MANAGE );
			return;		
		}
	}
	if( EM_MENU_STEP_2 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//�����ʽ��ȷ3λ���ֱ��
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(0 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayVoiceNum( EM_NUM_INEXISTS_MP3);//��Ų����� 
					PUBLIC_Delayms(2000);
					DispFuncPtrPre = NULL; //������һ��
					return;
				}
				else
				{
					if(CpuCardDeleteID(addId))
					{
						HAL_Voice_PlayingVoice( EM_DEL_SUCCESS_MP3, GUI_TIME_1500MS );//ɾ���ɹ�	 
					}			
					else
					{
						HAL_Voice_PlayingVoice( EM_DEL_FAIL_MP3, GUI_TIME_1500MS );//ɾ��ʧ��	 
					}
					MenuItem.CurMenuNum = EM_MENU_STEP_4;
				}
			}
			else//������ʾ����ʽ����ȷ���������롱
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr;
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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
			HAL_Voice_PlayingVoice(EM_DEL_SUCCESS_MP3, 1200); //ɾ���ɹ�
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}
		else if(temp == 2)
		{
			HAL_Voice_PlayingVoice(EM_DEL_FAIL_MP3, 1200); //ɾ��ʧ��	
			MenuItem.CurMenuNum = EM_MENU_STEP_4;
		}
		if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;		
		}
	}
	if( EM_MENU_STEP_4 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_DELETE_USER );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_DELETE_USER );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_CheckCardErrMenu()
* Description   :  ����֤ʧ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_CheckCardErrMenu( void )	  //����֤ʧ��
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_CheckCardErrMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );   
		App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
		App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
        APP_SCREEN_Operate(EM_SCREEN_APP_FLOW0_SHOW_MOTION , EM_SCREEN_FLOW0_FACE_FAIL);
		
		SystemSeting.CheckErrAllCnt++;    
		(void)SystemWriteSeting( (uint8_t *)&SystemSeting.CheckErrAllCnt, sizeof SystemSeting.CheckErrAllCnt );
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_PWD_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
		if( (SystemSeting.CheckErrAllCnt >= LOCKERR_ALL_MAX) || (SystemSeting.CheckErrPwdCnt >= LOCKERR_PWD_MAX) )  //����
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
* Description   :  ɾ����ͨ�û�ָ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_DelFingerGuestMenu( void )         //ɾ����ͨ�û�ָ��
{
    if( DispFuncPtrPre != DispFuncPtr )
    {
		my_printf( "App_GUI_DelFingerGuestMenu()\n" );  
	    DispFuncPtrPre = DispFuncPtr;
		MenuItem.CurMenuNum = EM_MENU_STEP_1;
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
        App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		HAL_Voice_PlayingVoice( EM_DEL_TYPE_OPT_MP3, GUI_TIME_4S );//ɾ������ѡ����ʾ( ���ɾ���밴1,�ȶ�ɾ���밴2 ) 
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum ) 
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_NO_1 == tp1 ) //���ɾ��
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_2;
			return;		
		}
		else if( TOUCH_KEY_NO_2 == tp1 ) //�Ա�ɾ��
		{
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
			return;		
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
			return;
		}
	}
	
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_SET_FINGER_MENU );
	}

}

/*********************************************************************************************************************
* Function Name :  App_GUI_SEMI_AddAdminFingerMenu()
* Description   :  ���ӹ���Աָ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SEMI_AddAdminFingerMenu( void )	  //���ӹ���Աָ��
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //������3λ���ֱ��,��ȷ�ϼ�ȷ��
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//�����ʽ��ȷ3λ���ֱ��
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//����Ѵ������������� 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//��ˢ�� 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//������ʾ����ʽ����ȷ���������롱
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //������һ��
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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
		if(result == CPUCARD_ADD_SUCCESSFUL)// ���ӳɹ�
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_2S );//�Ǽǳɹ�
			MenuItem.CurMenuNum = EM_MENU_STEP_3;		
		}
		else if(result == CPUCARD_ADD_REGISTERED)// ��ע��
		{
			HAL_Voice_PlayingVoice( EM_CARD_REGISTED_MP3, GUI_TIME_2S );//����ע��   
			MenuItem.CurMenuNum = EM_MENU_STEP_3;			
		}
		else if(result == CPUCARD_ADD_ERROR)// ����ʧ��
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_2S );//�Ǽ�ʧ��
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_ADD_USER );
	}
}

/*********************************************************************************************************************
* Function Name :  App_GUI_SEMI_AddGuestFingerMenu()
* Description   :  ������ͨ�û�ָ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_GUI_SEMI_AddGuestFingerMenu( void )	  //������ͨ�û�ָ��
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
		App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );//�ص������
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );
		#ifdef LOCK_KEY_WHITE_LED_ON
			App_LED_OutputCtrl( EM_LED_LOCK_W, EM_LED_OFF ); //�ص����ż��׵�
		#endif
		memset(ADD_ID,0,3);
		HAL_Voice_PlayingVoice( EM_IN_USER_NUM_MP3, GUI_TIME_4S ); //������3λ���ֱ��,��ȷ�ϼ�ȷ��
		GuiQuitTimMs = GUI_TIME_30S;
	}
	
	if( EM_MENU_STEP_1 == MenuItem.CurMenuNum )
	{
		uint8_t tp1 = App_Touch_GetCurrentKeyValue();
		if( TOUCH_KEY_ENTER == tp1 )     //ȷ�ϼ�
		{
			uint8_t tp1 = App_Touch_GetCurrentKeyIndex();
			App_Touch_GetCurrentKeyValBuf( ADD_ID, &addIdlen ); 
			if( tp1 == 3 )//�����ʽ��ȷ3λ���ֱ��
			{
				addId  = ADD_ID[0]*100 + ADD_ID[1]*10 + ADD_ID[2];
				my_printf("add id is =%d\n", addId);
				uint32_t address =0;
				CARD_MEG_Def  cardMeg= {0};
				if(1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, addId, &address, &cardMeg ))
				{
					HAL_Voice_PlayingVoice( EM_NUM_EXISTS_MP3, GUI_TIME_1S );//����Ѵ������������� 
					DispFuncPtrPre = DispFuncPtr;
					return;
				}
				else
				{
					MenuItem.CurMenuNum = EM_MENU_STEP_2;
					HAL_Voice_PlayingVoice( EM_PUT_CARD_MP3, GUI_TIME_1S );//��ˢ�� 
					App_Touch_FuncEnCtrl( EM_SCAN_KEY_BACK );
				}
			}
			else//������ʾ����ʽ����ȷ���������롱
			{
				HAL_Voice_PlayingVoice( EM_FORMAT_ERROR_RE_ENTER_MP3, GUI_TIME_1S );
				DispFuncPtrPre = DispFuncPtr; //������һ��
				return;	
			}
		}
		else if( TOUCH_KEY_BACK == tp1 ) //���ذ���
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
		if(result == CPUCARD_ADD_SUCCESSFUL)// ���ӳɹ�
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, GUI_TIME_2S );//�Ǽǳɹ�
			MenuItem.CurMenuNum = EM_MENU_STEP_3;		
		}
		else if(result == CPUCARD_ADD_REGISTERED)// ��ע��
		{
			HAL_Voice_PlayingVoice( EM_CARD_REGISTED_MP3, GUI_TIME_2S );//����ע��   
			MenuItem.CurMenuNum = EM_MENU_STEP_3;			
		}
		else if(result == CPUCARD_ADD_ERROR)// ����ʧ��
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, GUI_TIME_2S );//�Ǽ�ʧ��
			MenuItem.CurMenuNum = EM_MENU_STEP_3;
		}
		if( TOUCH_KEY_BACK == tp1 ) //���ذ���
		{
		    App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;		
		}
	}
	else if( EM_MENU_STEP_3 == MenuItem.CurMenuNum )
	{
		if( 0 == HAL_Voice_GetBusyState() )  //�����������
		{
			App_GUI_MenuJump( EM_MENU_ADD_USER );
			return;
		}	
	}
 
	if( 0 == GuiQuitTimMs )  //GUI��ʱ�˳�
	{
		my_printf( "gui option timeout!\n" );
		App_GUI_MenuJump( EM_MENU_ADD_USER );
	}
}
const KdbTabType_T DispTab[] =
{
   /*-------����˵���һ���˵���-----------*/
    {EM_MENU_MAIN_DESK, 1, (*App_GUI_MainDeskMenu)},          //ϵͳ����˵�

   /*-------ϵͳ�����˵�-------*/
	{EM_MENU_SYSTEM_MANAGE, 1, (*App_GUI_SystemManageMenu)},  //ϵͳ�����˵�
	{EM_MENU_CHECK_ADMIN, 1, (*App_GUI_CheckAdminMenu)},      //��֤����ԱȨ��
   
   /*-------��������˵�-------*/
	{EM_MENU_OPEN_DOOR, 1, (*App_GUI_OpenDoorMenu)},          //ִ�п�������
	{EM_MENU_CLOSE_DOOR, 1, (*App_GUI_CloseDoorMenu)},        //ִ�й�������
	
   /*-------������˵�---------*/
	{EM_MENU_SET_FACE_MENU, 1, (*App_GUI_SetFaceMenu)},          //�������ò˵�
	{EM_MENU_FACE_ADD_ADMIN, 1, (*App_GUI_AddAdminFaceMenu)},    //���ӹ���Ա����
	{EM_MENU_FACE_ADD_GUEST, 1, (*App_GUI_AddGuestFaceMenu)},    //������ͨ�û�����
	{EM_MENU_FACE_DELETE, 1, (*App_GUI_DelFaceMenu)},            //ɾ������
	{EM_MENU_FACE_CHECK_ERR, 1, (*App_GUI_CheckFaceErrMenu)},    //������֤ʧ��
	
   /*-------ָ����˵�---------*/
	{EM_MENU_SET_FINGER_MENU, 1, (*App_GUI_SetFingerMenu)},      //ָ�����ò˵�
	{EM_MENU_FINGER_ADD_ADMIN, 1, (*App_GUI_AddAdminFingerMenu)},//���ӹ���Աָ��
	{EM_MENU_FINGER_ADD_GUEST, 1, (*App_GUI_AddGuestFingerMenu)},//������ͨ�û�ָ��
	{EM_MENU_FINGER_DELETE, 1, (*App_GUI_DelFingerMenu)},        //ɾ��ָ��
	{EM_MENU_FINGER_CHECK_ERR, 1, (*App_GUI_CheckFingerErrMenu)},//ָ����֤ʧ��
 
   /*-------������˵�---------*/
	{EM_MENU_SET_PWD_MENU, 1, (*App_GUI_SetPwdMenu)},   	    //�������ò˵�
	{EM_MENU_CHANGE_PWD, 1, (*App_GUI_ChangePwdMenu)},          //�޸�����
	{EM_MENU_PWD_CHECK_ERR, 1, (*App_GUI_CheckPwdErrMenu)},     //������֤ʧ��
	
   /*-------�澯��˵�---------*/
	{EM_MENU_BAT_UNWORK, 1, (*App_GUI_BatVolLowErrMenu)},       //��ѹ���޷�����
	{EM_MENU_EEPROM_ERR, 1, (*App_GUI_EepromErrorMenu)},        //EEPROM����
	{EM_MENU_ALARM_WARM, 1, (*App_GUI_AlarmHandlerMenu)},       //���˸澯����
	{EM_MENU_TRY_PROTECT, 1, (*App_GUI_TryProtectMenu)},        //���Ա����澯
	{EM_MENU_STAY_WARM, 1, (*App_GUI_StayWarmMenu)},            //���������澯
	{EM_MENU_DEPLAY_WARM, 1, (*App_GUI_DeployWarmMenu)},        //���������澯
	{EM_MENU_FALSE_LOCK_WARM, 1,(*App_GUI_FalseLockWarmMenu)},  //�����澯
	{EM_MENU_FORGET_LOCK_WARM, 1,(*App_GUI_ForgetLockWarmMenu)},//��δ�ظ澯
	{EM_MENU_BLE_OPEN_ERR, 1,(*App_GUI_BleOpenErrWarmMenu)},    //��������ʧ��
 
   /*-------ģʽ��˵�---------*/
	{EM_MENU_BACK_FACTORY, 1, (*App_GUI_BackFactoryMenu)},      //�ָ���������
	{EM_MENU_OTA_MODEL, 1, (*App_GUI_GotoOtaModelMenu)},        //����ģʽ
	{EM_MENU_APP_MODEL, 1, (*App_GUI_GotoAppModelMenu)},        //APP����ģʽ
	
   /*-------ϵͳ������˵�-----*/
    {EM_MENU_SYSTEM_PARA_SET, 1, (*App_GUI_SetSysParaMenu)},        //ϵͳ��������
	{EM_MENU_MOTOR_DIR_SET, 1, (*App_GUI_SetMotorDirMenu)},         //�����������
	{EM_MENU_MOTOR_TORSION_SET, 1, (*App_GUI_SetMotorTorsionMenu)}, //���Ť������
	{EM_MENU_AUTO_LOCK_SET, 1, (*App_GUI_SetAutoLockMenu)}, 		//�Զ���������
	{EM_MENU_DOUBLE_CHECK_SET, 1, (*App_GUI_SetDoubleCheckMenu)},   //˫����֤����
	{EM_MENU_VOL_ADJUST_SET, 1, (*App_GUI_SetVolGradeMenu)},        //������������
	{EM_MENU_NEAR_SENSE_SET, 1, (*App_GUI_SetNearSenseMenu)}, 		//�ӽ���Ӧ����
	{EM_MENU_DEPPOY_SET, 1, (*App_GUI_SetDeployMenu)},   			//��������
	{EM_MENU_STAY_CHECK_SET, 1, (*App_GUI_SetStayCheckMenu)},       //��������
 
   /*-------������˵�---------*/
	{EM_MENU_ERROR_CHECK, 1, (*App_GUI_ErrorCheckMenu)},      //ϵͳ���ϼ��
	{EM_MENU_SMART_SCREEN_SHOW, 1, (*App_GUI_SmartScreenShowMenu)},      //ϵͳ���ϼ��
	{EM_MENU_BELL_VIDEO, 1, (*App_GUI_BellVideoMenu)},        //������Ƶ����
	{EM_MENU_NETWORK_UPDATE, 1,(*App_GUI_NetworkUpdateMenu)}, //����ͬ������
	{EM_MENU_WEATHER_UPDATE, 1,(*App_GUI_WeatherUpdateMenu)}, //����ͬ������
	{EM_MENU_BELL_LAMP_DISPLAY, 1,(*App_GUI_BellLampMenu)},   //�������ʾ����
	{EM_MENU_WAKEUP_BUT_SLEEP, 1,(*App_GUI_WakeButSleepMenu)},//���Ѻ��޸�֪����
	
	/*-------�����˵������Զ��---------*/
	{EM_MENU_ADD_USER, 1, (*App_GUI_AddUserMenu)},      //�����û��˵�
	{EM_MENU_DELETE_USER, 1, (*App_GUI_DeleteUser)},		// ɾ���û��˵�
	{EM_MENU_SYSTEM_TIME_SETTING, 1, (*App_GUI_SystemTimeSetting)},		//ϵͳʱ�����ò˵�
	{EM_MENU_UNLOCK_WAY_SETTING, 1, (*App_GUI_UnlockWaySetting)},		// ������ʽ���ò˵�
	{EM_MENU_VOICE_SETTING, 1, (*App_GUI_VoiceSetting)},		//�������ò˵�
	
	/*-------�����˵�(���Զ���)---------*/
	{EM_MENU_FINGER_DELETE_GUEST, 1, (*App_GUI_DelFingerGuestMenu)},        //ɾ����ͨ�û�ָ��
	
	/*-------IC���˵�---------*/
    {EM_MENU_CARD_ADD, 1, (*App_GUI_AddCardMenu)},          //���ӿ�
	{EM_MENU_CARD_DELETE, 1, (*App_GUI_DelCardMenu)},            //ɾ����
	{EM_MENU_CARD_CHECK_ERR, 1, (*App_GUI_CheckCardErrMenu)},         //����֤ʧ��
	
   /*-------�ϻ����Բ˵�-------*/
	{EN_MENU_AGING_TEST, 1, (*App_GUI_AginTestMenu)},      	  //ϵͳ�ϻ�����
	
   /*--------���߲˵�---------*/
	{MENU_SYSTEM_SLEEP, 1, (*App_GUI_GotoSleepModelMenu)},    //ϵͳ���߲˵�
	
}; 

/*********************************************************************************************************************
* Function Name :  App_GUI_Scheduler()
* Description   :  GUI�����̵ĵ��ȹ��ܺ���
* Para          :  ��
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
* Description   :  �������
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
void App_GUI_MenuProcess( void )
{
	switch( App_GUI_GetSystemWorkSts() )
	{
		case E_SYSTEM_SELFCHECK:  //ϵͳ�Լ�  ����GUI����
                if( 1 == SystemSelfCheck() )
				{
					App_GUI_SetSystemWorkSts( E_SYSTEM_VOICE_CFG );
					my_printf("SystemSelfCheck=%d\n", SystemTick);
					my_printf( "SystemSelfCheck()\n" ); 
				}
		break;
		
		case E_SYSTEM_VOICE_CFG:  //ȷ������оƬ�ȶ�������
				if( SystemWorkHoldTim >= SYS_STEADY_TIMER_MS )  
				{
				   if( E_WAKE_ALARM_BREAK != App_GUI_GetSysWakeupType() )    //�жϷ��˻���
				   {
					   HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice );  
				   }
				   App_GUI_SetSystemWorkSts( E_SYSTEM_WORKING );
				}
		break;		
				
		case E_SYSTEM_WORKING:  //ִ��GUI
		        App_GUI_Scheduler();
		break;
		
		default:break;
	}
}
 
 
 
 
 
/*-------------------------------------------------THE FILE END-----------------------------------------------------*/