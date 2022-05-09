/********************************************************************************************************************
 * @file:      App_LED.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-07-28
 * @brief:     按键灯的功能接口函数文件
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "App_LED.h" 
#include "System.h"


#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
#include "..\HAL\HAL_Touch\HAL_Touch.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"

/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define  PWM_VAL_DET    2
#define  TIMER_100MS    10 
/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/


/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         


/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
typedef struct 
{
   uint8_t CtrlState;
   uint8_t LedType;
   uint8_t PwmVal;
   uint8_t ChangeDir;
 
}BreathLedMeg_T;
static BreathLedMeg_T  BreathMeg;

static uint16_t  LedTimTick = 0; 

/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_LED_FileInit()
* Description   :  功能文件初始化     根据不同文件可配置初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
#ifdef PIN_CONFIG_POW_RGB
static void app_led_rgb_ctrl( LED_CMD_E cmd ) 
{
    if(EM_LED_OFF == cmd || EM_LED_RGB_MODE_OFF == SystemSeting.LedRGBMode)
    {
        (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_HIGH ); 
        return;
    }

    // 除去年月日的时间戳
    uint32_t timestamp = Rtc_Real_Time.timestamp%(24*3600);

    switch(SystemSeting.LedRGBMode)
    {
        case EM_LED_RGB_MODE_ALLDAY:
            (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_LOW ); 
            break;
            
        case EM_LED_RGB_MODE_DAYTIME:
            if(timestamp >= APP_LED_DAYTIME_FLAG && timestamp < APP_LED_NIGHTTIME_FLAG)
            {
                (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_LOW ); 
                return;
            }
            break;
            
        case EM_LED_RGB_MODE_NIGHTTIME:
            if(timestamp >= APP_LED_NIGHTTIME_FLAG || timestamp < APP_LED_DAYTIME_FLAG)
            {
                (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_LOW ); 
                return;
            }
            break;
            
        default:
            break;
    }
    
    (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_HIGH ); 
    return;
}
#endif


/*********************************************************************************************************************
* Function Name :  App_LED_FileInit()
* Description   :  功能文件初始化     根据不同文件可配置初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_LED_FileInit( void ) 
{
    HAL_EXPORT_PinInit( EM_LED_00, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_01, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_02, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_03, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_04, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_05, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_06, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_07, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_08, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_09, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_SURE, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_BACK, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_RGB_R, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_RGB_G, DIR_OUT, POLO_RETTAIN );  

    App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 

    App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_OFF ); 

    App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_B, EM_LED_OFF ); 

    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_LOCK_G, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 

    LedTimTick = 0;

    BreathMeg.PwmVal = EM_OUT_OFF;
    BreathMeg.ChangeDir = 0;
    BreathMeg.CtrlState = 0xff;
 
#ifdef PIN_CONFIG_LED_BELL
    (void)HAL_EXPORT_PinInit(EM_LED_BELL_PIN, DIR_OUT, POLO_RETTAIN);
#endif

#ifdef PIN_CONFIG_POW_RGB
    (void)HAL_EXPORT_PinInit(EM_POW_RGB, DIR_OUT, POLO_RETTAIN);
#endif

    return;
}

/*********************************************************************************************************************
* Function Name :  App_LED_WakeupInit()
* Description   :  唤醒配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_LED_WakeupInit( void )
{
    LedTimTick = 0;

    BreathMeg.PwmVal = EM_OUT_OFF;
    BreathMeg.ChangeDir = 0;
    BreathMeg.CtrlState = 0xff;
   
#ifdef PIN_CONFIG_POW_RGB
    (void)app_led_rgb_ctrl(EM_LED_ON);
#endif
    return;
} 

/*********************************************************************************************************************
* Function Name :  App_LED_SleepInit()
* Description   :  休眠配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_LED_SleepInit( void )
{
    App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
	App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_B, EM_LED_OFF ); 
    
#ifdef PIN_CONFIG_LED_BELL
    App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );
#endif
	
#ifdef PIN_CONFIG_POW_RGB
    (void)app_led_rgb_ctrl(EM_LED_OFF);
#endif
    return;
} 

/*********************************************************************************************************************
* Function Name :  App_LED_Tim10Ms()
* Description   :  相关定时器  10Ms触发一次
* Para          :  none 
* Return        :  void
*********************************************************************************************************************/
void App_LED_Tim10Ms( void ) 
{
    if( LedTimTick > 0 )
	    LedTimTick--;
    return;
}
 
/*********************************************************************************************************************
* Function Name :  App_LED_OutputCtrl()
* Description   :  功能文件初始化
* Para          :  type- LED名称   cmd- 控制输出状态  
* Return        :  void
*********************************************************************************************************************/
void App_LED_OutputCtrl( LED_TYPE_E type, LED_CMD_E cmd ) 
{
    PIN_SET_E setcmd;
    
#ifdef PIN_CONFIG_POW_RGB
    (void)app_led_rgb_ctrl(EM_LED_ON);
#endif
   
    switch( type )
    {
        case EM_LED_0:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_00, setcmd ); 
        break;

        case EM_LED_1:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_01, setcmd ); 
        break;

        case EM_LED_2:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_02, setcmd ); 
        break; 

        case EM_LED_3:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_03, setcmd ); 
        break; 

        case EM_LED_4:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_04, setcmd ); 
        break; 

        case EM_LED_5:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_05, setcmd ); 
        break;  

        case EM_LED_6:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_06, setcmd ); 
        break;  

        case EM_LED_7:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_07, setcmd ); 
        break;  

        case EM_LED_8:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_08, setcmd ); 
        break;  

        case EM_LED_9:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_09, setcmd ); 
        break; 

        case EM_LED_ENTER:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_SURE, setcmd ); 
        break; 

        case EM_LED_CANCLE:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_BACK, setcmd ); 
        break; 

        case EM_LED_POW_R:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_LOW ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 				
        	}
        	else
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 
        	}
        break; 

        case EM_LED_POW_G:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_LOW ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 				
        	}
        	else
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 
        	}
        break; 

        case EM_LED_POW_B:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_LOW ); 				
        	}
        	else
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 
        	}
        break; 
        	
        case EM_LED_ALL:   //全屏幕
        	    setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_00, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_01, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_02, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_03, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_04, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_05, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_06, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_07, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_08, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_09, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, setcmd ); 
        break; 

        case EM_LED_E:         //EEPROM故障显示  12345789
            HAL_EXPORT_PinSet( EM_LED_00, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_HIGH ); 
        break; 	

        case EM_LED_X:         //故障显示  13579
            HAL_EXPORT_PinSet( EM_LED_00, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_HIGH ); 
        break; 

        case EM_LED_CFG_NET:   //配网显示  25846
            HAL_EXPORT_PinSet( EM_LED_00, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_HIGH ); 
        break; 	

        case EM_LED_PAGE_TURN:   //翻页显示    08*
            HAL_EXPORT_PinSet( EM_LED_00, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_LOW ); 
        break; 	

        case EM_LED_LOCK_R:
            #ifdef BREATHE_LAMP
            App_LED_BreathLampCtrlEn(BREATH_LED_G, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_B, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_R, (cmd == EM_LED_ON)?true:false);
            #else
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_ON ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
            #endif
        break; 

        case EM_LED_LOCK_G:
            #ifdef BREATHE_LAMP
            App_LED_BreathLampCtrlEn(BREATH_LED_R, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_B, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_G, (cmd == EM_LED_ON)?true:false);
            #else
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_ON ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
            #endif
        break; 

        case EM_LED_LOCK_B:
            #ifdef BREATHE_LAMP
            App_LED_BreathLampCtrlEn(BREATH_LED_R, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_G, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_B, (cmd == EM_LED_ON)?true:false);
            #else
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_ON ); 	
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
            #endif
        break; 

        case EM_LED_LOCK_W:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_W, EM_OUT_ON ); 
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_W, EM_OUT_OFF ); 	
        	}
        break; 	

		case EM_LED_BELL:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_BELL_PIN, OUT_LOW ); 
        	}
        	else 
        	{
        	   HAL_EXPORT_PinSet( EM_LED_BELL_PIN, OUT_HIGH ); 	
        	}
        break; 
        default:break;	   
    }
    return;
} 
 
 
/*********************************************************************************************************************
* Function Name :  App_LED_BreathCtrl()
* Description   :  呼吸灯控制
* Para          :  type- LED名称   cmd- 控制输出状态  
* Return        :  void
*********************************************************************************************************************/
void App_LED_BreathLampCtrlEn( uint8_t channel, uint8_t cmd )  
{
    if( FUNC_ENABLE == cmd )        //功能开启
	{
		BreathMeg.PwmVal = EM_OUT_OFF - PWM_VAL_DET;
		BreathMeg.ChangeDir = 1;
        HAL_Touch_PwmOutputCtrl( channel, BreathMeg.PwmVal );	
		BreathMeg.CtrlState = FUNC_ENABLE;
		BreathMeg.LedType = channel;
	}
	else if( FUNC_DISABLE == cmd )  //功能关闭
	{
		BreathMeg.PwmVal = EM_OUT_OFF;
		BreathMeg.ChangeDir = 0;
		HAL_Touch_PwmOutputCtrl( channel, BreathMeg.PwmVal );	
		BreathMeg.CtrlState = FUNC_DISABLE;
		BreathMeg.LedType = channel;
	}	
    return;
}

/*********************************************************************************************************************
* Function Name :  App_LED_BreathThread()
* Description   :  呼吸灯输出功能函数
* Para          :  none 
* Return        :  void
*********************************************************************************************************************/
void App_LED_BreathThread( void )  
{
    if( FUNC_ENABLE == BreathMeg.CtrlState )        //功能开启
	{
		if( BreathMeg.ChangeDir == 1 )
		{
			if( BreathMeg.PwmVal < PWM_VAL_DET )
			{
				BreathMeg.ChangeDir = 0;
			}
			else
			{
				BreathMeg.PwmVal -= PWM_VAL_DET; 
			}
		}
		else if( BreathMeg.ChangeDir == 0 )
		{
			if( BreathMeg.PwmVal < EM_OUT_OFF + PWM_VAL_DET )
			{
				BreathMeg.PwmVal += PWM_VAL_DET; 
				if( BreathMeg.PwmVal >= EM_OUT_OFF + PWM_VAL_DET )
				{
					BreathMeg.PwmVal = EM_OUT_OFF; 
				    BreathMeg.ChangeDir = 1;
				}
			}	
		}		
        HAL_Touch_PwmOutputCtrl( BreathMeg.LedType, BreathMeg.PwmVal );	
	}
	else if( FUNC_DISABLE == BreathMeg.CtrlState )  //功能关闭
	{
		BreathMeg.PwmVal = EM_OUT_OFF;
		HAL_Touch_PwmOutputCtrl( BreathMeg.LedType, BreathMeg.PwmVal );	
		BreathMeg.CtrlState = FUNC_DISABLE;
		BreathMeg.ChangeDir = 0;
	}	
    return;
}
  

/*-------------------------------------------------THE FILE END-----------------------------------------------------*/

/********************************************************************************************************************
 * @file:      App_LED.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-07-28
 * @brief:     按键灯的功能接口函数文件
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "App_LED.h" 
#include "System.h"


#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
#include "..\HAL\HAL_Touch\HAL_Touch.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"

/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define  PWM_VAL_DET    2
#define  TIMER_100MS    10 
/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/


/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         


/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
typedef struct 
{
   uint8_t CtrlState;
   uint8_t LedType;
   uint8_t PwmVal;
   uint8_t ChangeDir;
 
}BreathLedMeg_T;
static BreathLedMeg_T  BreathMeg;

static uint16_t  LedTimTick = 0; 

/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_LED_FileInit()
* Description   :  功能文件初始化     根据不同文件可配置初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
#ifdef PIN_CONFIG_POW_RGB
static void app_led_rgb_ctrl( LED_CMD_E cmd ) 
{
    if(EM_LED_OFF == cmd || EM_LED_RGB_MODE_OFF == SystemSeting.LedRGBMode)
    {
        (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_HIGH ); 
        return;
    }

    // 除去年月日的时间戳
    uint32_t timestamp = Rtc_Real_Time.timestamp%(24*3600);

    switch(SystemSeting.LedRGBMode)
    {
        case EM_LED_RGB_MODE_ALLDAY:
            (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_LOW ); 
            break;
            
        case EM_LED_RGB_MODE_DAYTIME:
            if(timestamp >= APP_LED_DAYTIME_FLAG && timestamp < APP_LED_NIGHTTIME_FLAG)
            {
                (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_LOW ); 
                return;
            }
            break;
            
        case EM_LED_RGB_MODE_NIGHTTIME:
            if(timestamp >= APP_LED_NIGHTTIME_FLAG || timestamp < APP_LED_DAYTIME_FLAG)
            {
                (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_LOW ); 
                return;
            }
            break;
            
        default:
            break;
    }
    
    (void)HAL_EXPORT_PinSet( EM_POW_RGB, OUT_HIGH ); 
    return;
}
#endif


/*********************************************************************************************************************
* Function Name :  App_LED_FileInit()
* Description   :  功能文件初始化     根据不同文件可配置初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_LED_FileInit( void ) 
{
    HAL_EXPORT_PinInit( EM_LED_00, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_01, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_02, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_03, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_04, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_05, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_06, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_07, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_08, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_09, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_SURE, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_BACK, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_RGB_R, DIR_OUT, POLO_RETTAIN );  
    HAL_EXPORT_PinInit( EM_LED_RGB_G, DIR_OUT, POLO_RETTAIN );  

    App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 

    App_LED_OutputCtrl( EM_LED_ENTER, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_CANCLE, EM_LED_OFF ); 

    App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_B, EM_LED_OFF ); 

    App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_LOCK_G, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 

    LedTimTick = 0;

    BreathMeg.PwmVal = EM_OUT_OFF;
    BreathMeg.ChangeDir = 0;
    BreathMeg.CtrlState = 0xff;
 
#ifdef PIN_CONFIG_LED_BELL
    (void)HAL_EXPORT_PinInit(EM_LED_BELL_PIN, DIR_OUT, POLO_RETTAIN);
#endif

#ifdef PIN_CONFIG_POW_RGB
    (void)HAL_EXPORT_PinInit(EM_POW_RGB, DIR_OUT, POLO_RETTAIN);
#endif

    return;
}

/*********************************************************************************************************************
* Function Name :  App_LED_WakeupInit()
* Description   :  唤醒配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_LED_WakeupInit( void )
{
    LedTimTick = 0;

    BreathMeg.PwmVal = EM_OUT_OFF;
    BreathMeg.ChangeDir = 0;
    BreathMeg.CtrlState = 0xff;
   
#ifdef PIN_CONFIG_POW_RGB
    (void)app_led_rgb_ctrl(EM_LED_ON);
#endif
    return;
} 

/*********************************************************************************************************************
* Function Name :  App_LED_SleepInit()
* Description   :  休眠配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_LED_SleepInit( void )
{
    App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF ); 
	App_LED_OutputCtrl( EM_LED_POW_R, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_G, EM_LED_OFF ); 
    App_LED_OutputCtrl( EM_LED_POW_B, EM_LED_OFF ); 
    
#ifdef PIN_CONFIG_LED_BELL
    App_LED_OutputCtrl( EM_LED_BELL, EM_LED_OFF );
#endif
	
#ifdef PIN_CONFIG_POW_RGB
    (void)app_led_rgb_ctrl(EM_LED_OFF);
#endif
    return;
} 

/*********************************************************************************************************************
* Function Name :  App_LED_Tim10Ms()
* Description   :  相关定时器  10Ms触发一次
* Para          :  none 
* Return        :  void
*********************************************************************************************************************/
void App_LED_Tim10Ms( void ) 
{
    if( LedTimTick > 0 )
	    LedTimTick--;
    return;
}
 
/*********************************************************************************************************************
* Function Name :  App_LED_OutputCtrl()
* Description   :  功能文件初始化
* Para          :  type- LED名称   cmd- 控制输出状态  
* Return        :  void
*********************************************************************************************************************/
void App_LED_OutputCtrl( LED_TYPE_E type, LED_CMD_E cmd ) 
{
    PIN_SET_E setcmd;
    
#ifdef PIN_CONFIG_POW_RGB
    (void)app_led_rgb_ctrl(EM_LED_ON);
#endif
   
    switch( type )
    {
        case EM_LED_0:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_00, setcmd ); 
        break;

        case EM_LED_1:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_01, setcmd ); 
        break;

        case EM_LED_2:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_02, setcmd ); 
        break; 

        case EM_LED_3:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_03, setcmd ); 
        break; 

        case EM_LED_4:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_04, setcmd ); 
        break; 

        case EM_LED_5:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_05, setcmd ); 
        break;  

        case EM_LED_6:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_06, setcmd ); 
        break;  

        case EM_LED_7:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_07, setcmd ); 
        break;  

        case EM_LED_8:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_08, setcmd ); 
        break;  

        case EM_LED_9:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_09, setcmd ); 
        break; 

        case EM_LED_ENTER:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_SURE, setcmd ); 
        break; 

        case EM_LED_CANCLE:
            setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_BACK, setcmd ); 
        break; 

        case EM_LED_POW_R:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_LOW ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 				
        	}
        	else
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 
        	}
        break; 

        case EM_LED_POW_G:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_LOW ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 				
        	}
        	else
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 
        	}
        break; 

        case EM_LED_POW_B:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_LOW ); 				
        	}
        	else
        	{
        	   HAL_EXPORT_PinSet( EM_LED_RGB_R, OUT_HIGH ); 	
        	   HAL_EXPORT_PinSet( EM_LED_RGB_G, OUT_HIGH ); 
               HAL_EXPORT_PinSet( EM_LED_RGB_B, OUT_HIGH ); 
        	}
        break; 
        	
        case EM_LED_ALL:   //全屏幕
        	    setcmd = (cmd == EM_LED_ON)? OUT_LOW : OUT_HIGH;
            HAL_EXPORT_PinSet( EM_LED_00, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_01, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_02, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_03, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_04, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_05, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_06, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_07, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_08, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_09, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, setcmd ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, setcmd ); 
        break; 

        case EM_LED_E:         //EEPROM故障显示  12345789
            HAL_EXPORT_PinSet( EM_LED_00, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_HIGH ); 
        break; 	

        case EM_LED_X:         //故障显示  13579
            HAL_EXPORT_PinSet( EM_LED_00, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_HIGH ); 
        break; 

        case EM_LED_CFG_NET:   //配网显示  25846
            HAL_EXPORT_PinSet( EM_LED_00, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_HIGH ); 
        break; 	

        case EM_LED_PAGE_TURN:   //翻页显示    08*
            HAL_EXPORT_PinSet( EM_LED_00, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_01, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_02, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_03, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_04, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_05, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_06, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_07, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_08, OUT_LOW ); 
            HAL_EXPORT_PinSet( EM_LED_09, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_SURE, OUT_HIGH ); 
            HAL_EXPORT_PinSet( EM_LED_BACK, OUT_LOW ); 
        break; 	

        case EM_LED_LOCK_R:
            #ifdef BREATHE_LAMP
            App_LED_BreathLampCtrlEn(BREATH_LED_G, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_B, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_R, (cmd == EM_LED_ON)?true:false);
            #else
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_ON ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
            #endif
        break; 

        case EM_LED_LOCK_G:
            #ifdef BREATHE_LAMP
            App_LED_BreathLampCtrlEn(BREATH_LED_R, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_B, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_G, (cmd == EM_LED_ON)?true:false);
            #else
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_ON ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
            #endif
        break; 

        case EM_LED_LOCK_B:
            #ifdef BREATHE_LAMP
            App_LED_BreathLampCtrlEn(BREATH_LED_R, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_G, false);
            App_LED_BreathLampCtrlEn(BREATH_LED_B, (cmd == EM_LED_ON)?true:false);
            #else
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_ON ); 	
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_R, EM_OUT_OFF ); 
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_G, EM_OUT_OFF ); 	
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_B, EM_OUT_OFF ); 	
        	}
            #endif
        break; 

        case EM_LED_LOCK_W:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_W, EM_OUT_ON ); 
        	}
        	else 
        	{
        	   HAL_Touch_PwmOutputCtrl( BREATH_LED_W, EM_OUT_OFF ); 	
        	}
        break; 	

		case EM_LED_BELL:
            if( cmd == EM_LED_ON )  
        	{
        	   HAL_EXPORT_PinSet( EM_LED_BELL_PIN, OUT_LOW ); 
        	}
        	else 
        	{
        	   HAL_EXPORT_PinSet( EM_LED_BELL_PIN, OUT_HIGH ); 	
        	}
        break; 
        default:break;	   
    }
    return;
} 
 
 
/*********************************************************************************************************************
* Function Name :  App_LED_BreathCtrl()
* Description   :  呼吸灯控制
* Para          :  type- LED名称   cmd- 控制输出状态  
* Return        :  void
*********************************************************************************************************************/
void App_LED_BreathLampCtrlEn( uint8_t channel, uint8_t cmd )  
{
    if( FUNC_ENABLE == cmd )        //功能开启
	{
		BreathMeg.PwmVal = EM_OUT_OFF - PWM_VAL_DET;
		BreathMeg.ChangeDir = 1;
        HAL_Touch_PwmOutputCtrl( channel, BreathMeg.PwmVal );	
		BreathMeg.CtrlState = FUNC_ENABLE;
		BreathMeg.LedType = channel;
	}
	else if( FUNC_DISABLE == cmd )  //功能关闭
	{
		BreathMeg.PwmVal = EM_OUT_OFF;
		BreathMeg.ChangeDir = 0;
		HAL_Touch_PwmOutputCtrl( channel, BreathMeg.PwmVal );	
		BreathMeg.CtrlState = FUNC_DISABLE;
		BreathMeg.LedType = channel;
	}	
    return;
}

/*********************************************************************************************************************
* Function Name :  App_LED_BreathThread()
* Description   :  呼吸灯输出功能函数
* Para          :  none 
* Return        :  void
*********************************************************************************************************************/
void App_LED_BreathThread( void )  
{
    if( FUNC_ENABLE == BreathMeg.CtrlState )        //功能开启
	{
		if( BreathMeg.ChangeDir == 1 )
		{
			if( BreathMeg.PwmVal < PWM_VAL_DET )
			{
				BreathMeg.ChangeDir = 0;
			}
			else
			{
				BreathMeg.PwmVal -= PWM_VAL_DET; 
			}
		}
		else if( BreathMeg.ChangeDir == 0 )
		{
			if( BreathMeg.PwmVal < EM_OUT_OFF + PWM_VAL_DET )
			{
				BreathMeg.PwmVal += PWM_VAL_DET; 
				if( BreathMeg.PwmVal >= EM_OUT_OFF + PWM_VAL_DET )
				{
					BreathMeg.PwmVal = EM_OUT_OFF; 
				    BreathMeg.ChangeDir = 1;
				}
			}	
		}		
        HAL_Touch_PwmOutputCtrl( BreathMeg.LedType, BreathMeg.PwmVal );	
	}
	else if( FUNC_DISABLE == BreathMeg.CtrlState )  //功能关闭
	{
		BreathMeg.PwmVal = EM_OUT_OFF;
		HAL_Touch_PwmOutputCtrl( BreathMeg.LedType, BreathMeg.PwmVal );	
		BreathMeg.CtrlState = FUNC_DISABLE;
		BreathMeg.ChangeDir = 0;
	}	
    return;
}
  

/*-------------------------------------------------THE FILE END-----------------------------------------------------*/

