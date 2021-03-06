/********************************************************************************************************************
 * @file:        App_Key.c
 * @author:      gushenghci
 * @version:     V01.00
 * @date:        2021-08-04
 * @Description: 后面板按键接口功能函数文件
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "App_Key.h" 
#include "System.h"
#include "LockConfig.h"
#include "DRV_GPIO.h"  
#include "..\HAL\HAL_Touch\HAL_Touch.h"   
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
#include "App_GUI.h" 
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define KEY_OPEN_READ()           DRV_GpioRead( KEY_OPEN_GPIO_PIN ) 
#define KEY_CLOSE_READ()          DRV_GpioRead( KEY_CLOSE_GPIO_PIN) 
#define HANDLE_LEFT_READ()        DRV_GpioRead( HANDLE_LEFT_GPIO_PIN ) 
#define HANDLE_MIDDLE_READ()      DRV_GpioRead( HANDLE_MIDDLE_GPIO_PIN ) 
#define HANDLE_RIGHT_READ()       DRV_GpioRead( HANDLE_RIGHT_GPIO_PIN ) 
#define BUTTON_REGISTER_READ()    DRV_GpioRead( BUTTON_REGISTER_GPIO_PIN ) 
#define FINGER_IRQ_READ()    	  DRV_GpioRead( FINGER_IRQ_GPIO_PIN ) 
#define ALARM_IRQ_READ()    	  DRV_GpioRead( ALARM_IRQ_GPIO_PIN ) 
#define SENSE_IRQ_READ()    	  DRV_GpioRead( SENSE_IRQ_GPIO_PIN ) 
#define TOUCH_IRQ_READ()    	  DRV_GpioRead( TOUCH_IRQ_GPIO_PIN ) 


#define ADJUST_PIN_EN             1   //调试用 
#define ADJUST_PORT_PIN           1   //M_MCU_PIN_RERSVER   //M_MCU_PIN_RERSVER   //调试用 
#define ADJUST_PIN_READ()         DRV_GpioRead( ADJUST_PORT_PIN )   //调试用 

#define TIMER_10_MS               1
#define TIMER_5000_MS             500
#define TIMER_DOUBLE_PUSH_MS      80   //双击有效间隔时间

#define MAX_BUTTON_NUM            11    //按键个数

#ifdef HANDLER_LEFT_ON
  #define HANDLER_LEFT_EN         1   
#else 
  #define HANDLER_LEFT_EN         0  
#endif

#ifdef HANDLER_MIDDLE_ON
  #define HANDLER_MIDDLE_EN       1  
#else 
  #define HANDLER_MIDDLE_EN       0  
#endif

#ifdef HANDLER_RIGHT_ON
  #define HANDLER_RIGHT_EN        1  
#else 
  #define HANDLER_RIGHT_EN        0  
#endif

#if (FINGER_IRQ_GPIO_PIN == M_MCU_PIN_RERSVER)
  #define FINGER_IRQ_EN        	  0  
#else 
  #define FINGER_IRQ_EN        	  1  
#endif

#if (ALARM_IRQ_GPIO_PIN == M_MCU_PIN_RERSVER)
  #define ALARM_IRQ_EN        	  0  
#else 
  #define ALARM_IRQ_EN        	  1  
#endif

#if (SENSE_IRQ_GPIO_PIN == M_MCU_PIN_RERSVER)
  #define SENSE_IRQ_EN        	  0  
#else 
  #define SENSE_IRQ_EN        	  1  
#endif

#if (TOUCH_IRQ_GPIO_PIN == M_MCU_PIN_RERSVER)
  #define TOUCH_IRQ_EN        	  0  
#else 
  #define TOUCH_IRQ_EN        	  1  
#endif

#ifdef FRAME_PLATFORM_FULLY_AUTO_ON        //全自动平台
  #define BUTTON_CLOSE_EN        1  
  #define BUTTON_OPEN_EN         1  
  #define BUTTON_REGISTER_EN     0  
#elif defined FRAME_PLATFORM_HALF_AUTO_ON  //半自动平台
  #define BUTTON_CLOSE_EN        0  
  #define BUTTON_OPEN_EN         0  
  #define BUTTON_REGISTER_EN     1 
#endif



/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/
static const uint8_t CtrlRegTab[ MAX_BUTTON_NUM ][ 3 ] = 
{
     /*滤波次数*/ 	/*有效电平*/   /*使能*/
    {     5,         	0,    BUTTON_OPEN_EN   		 },       //OPEN_KEY  
	{     5,         	0,    BUTTON_CLOSE_EN   	 },       //CLOSE_KEY 
    {     5,         	0,    HANDLER_LEFT_EN   	 },       //OPEN_HANDLE    LEFT
    {     5,         	1,    HANDLER_MIDDLE_EN   	 },       //OPEN_HANDLE    MIDDLE
	{     5,         	0,    HANDLER_RIGHT_EN 	     },       //CLOSE_HANDLE   RIGHT
	{     5,         	0,    BUTTON_REGISTER_EN     },       //半自动注册键
	{     5,         	1,    FINGER_IRQ_EN 	     },       //指纹
	{     5,         	0,    ALARM_IRQ_EN 	     	 },       //防撬
	{     5,         	0,    SENSE_IRQ_EN 	     	 },       //接近传感器
	{     5,         	0,    TOUCH_IRQ_EN 	     	 },       //调按键触摸中断
	{     5,         	0,    ADJUST_PIN_EN 	     },       //调试查看电平状态用
	
};

/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         
volatile InputIoMeg_T InputKeyTabl[ MAX_BUTTON_NUM ];

/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
static uint16_t KeySystick = 0;
static volatile uint8_t  InputKeyStatus[ MAX_BUTTON_NUM ]; 
static uint8_t  buttonCheckFlg = 0;
/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_Key_FileInit()
* Description   :  相关文件初始化   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Key_FileInit( void )
{
	DRV_GpioInputPullnull( KEY_OPEN_GPIO_PIN );
	DRV_GpioInputPullnull( KEY_CLOSE_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_LEFT_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_MIDDLE_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_RIGHT_GPIO_PIN );
	DRV_GpioInputPullnull( BUTTON_REGISTER_GPIO_PIN );
	DRV_GpioInputPullnull( FINGER_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( ALARM_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( SENSE_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( TOUCH_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( ADJUST_PORT_PIN );
	
	KeySystick = 0;
	buttonCheckFlg = 0;
	for(uint8_t i=0; i<MAX_BUTTON_NUM; i++)
	{
		InputKeyTabl[ i ].StsReg.CaptEn = CtrlRegTab[ i ][ 2 ];
		InputKeyTabl[ i ].StsReg.CurPhySts = InputKeyStatus[i];
		InputKeyTabl[ i ].StsReg.CurSts = 0;
		InputKeyTabl[ i ].StsReg.PrevSts =  ~CtrlRegTab[ i ][ 1 ];
		InputKeyTabl[ i ].StsReg.ValidSts = 0;
		InputKeyTabl[ i ].StsReg.DoublePushSts = 0;
		
		InputKeyTabl[ i ].CtrlReg.SampCnt = 0;
		InputKeyTabl[ i ].CtrlReg.PopTime = 0xffff;
		InputKeyTabl[ i ].CtrlReg.PushTime = 0 ;
	}
} 

/*********************************************************************************************************************
* Function Name :  App_Key_WakeupInit()
* Description   :  唤醒配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Key_WakeupInit( void )
{
	DRV_GpioInputPullnull( KEY_OPEN_GPIO_PIN );
	DRV_GpioInputPullnull( KEY_CLOSE_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_LEFT_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_MIDDLE_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_RIGHT_GPIO_PIN );
	DRV_GpioInputPullnull( BUTTON_REGISTER_GPIO_PIN );
	DRV_GpioInputPullnull( ADJUST_PORT_PIN );

	KeySystick = 0;
	buttonCheckFlg = 0;
	for(uint8_t i=0; i<MAX_BUTTON_NUM; i++)
	{
		InputKeyTabl[ i ].StsReg.CaptEn = CtrlRegTab[ i ][ 2 ];
		InputKeyTabl[ i ].StsReg.CurPhySts = InputKeyStatus[i];
		InputKeyTabl[ i ].StsReg.CurSts = 0;
		InputKeyTabl[ i ].StsReg.PrevSts =  ~CtrlRegTab[ i ][ 1 ];;
		InputKeyTabl[ i ].StsReg.ValidSts = 0;
		InputKeyTabl[ i ].StsReg.DoublePushSts = 0;
		
		InputKeyTabl[ i ].CtrlReg.SampCnt = 0;
		InputKeyTabl[ i ].CtrlReg.PopTime = 0xffff;
		InputKeyTabl[ i ].CtrlReg.PushTime = 0 ;
	}
} 

/*********************************************************************************************************************
* Function Name :  App_Key_SleepInit()
* Description   :  休眠配置   
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Key_SleepInit( void )
{
	DRV_GpioInputPullnull( KEY_OPEN_GPIO_PIN );
	DRV_GpioInputPullnull( KEY_CLOSE_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_LEFT_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_MIDDLE_GPIO_PIN );
	DRV_GpioInputPullnull( HANDLE_RIGHT_GPIO_PIN );
	DRV_GpioInputPullnull( BUTTON_REGISTER_GPIO_PIN );
	DRV_GpioInputPullnull( FINGER_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( ALARM_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( SENSE_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( TOUCH_IRQ_GPIO_PIN );
	DRV_GpioInputPullnull( ADJUST_PORT_PIN );
} 
 
/*********************************************************************************************************************
* Function Name :  App_Key_Tim10Ms()
* Description   :  功能相关定时器   10ms触发一次
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Key_Tim10Ms(void)
{
	if(KeySystick > 0)
	   KeySystick--;
} 
 
/*********************************************************************************************************************
* Function Name :  App_Key_GetMotorTurnDir()
* Description   :  获取电机开门方向 
* Para          :  无
* Return        :  1= 左开  2= 右开  othe= 无效
*********************************************************************************************************************/
static uint8_t App_Key_GetMotorTurnDir( void )
{
	if( SystemFixSeting.MotorDirection == LEFT_HAND_DOOR )
	{
		return 1;
	}
	else if( SystemFixSeting.MotorDirection == RIGHT_HAND_DOOR )
	{
		return 2;
	}
 
	return 0;
} 
 
/*********************************************************************************************************************
* Function Name :  App_Key_KeyScan()
* Description   :  输入端口扫描
* Para          :  pkey--指向存储电口状态的数组的指针
* Return        :  void
*********************************************************************************************************************/
static void App_Key_KeyScan( volatile uint8_t *pkey )
{
    pkey[ OPEN_KEY ]  = KEY_OPEN_READ();    
	pkey[ CLOSE_KEY ] = KEY_CLOSE_READ();
    pkey[ LEFT_HANDLE ]  = HANDLE_LEFT_READ();    
	pkey[ MIDDLE_HANDLE ]= HANDLE_MIDDLE_READ();    
	pkey[ RIGHT_HANDLE ] = HANDLE_RIGHT_READ();
	pkey[ REGISTER_KEY ] = BUTTON_REGISTER_READ();
	pkey[ FINGER_IRQ ] = FINGER_IRQ_READ();
	pkey[ ALARM_IRQ ] = ALARM_IRQ_READ();
	pkey[ SENSE_IRQ ] = SENSE_IRQ_READ();
	pkey[ TOUCH_IRQ ] = TOUCH_IRQ_READ();
	pkey[ ADJUST_LEVEL ] = ADJUST_PIN_READ();
	
}

/*********************************************************************************************************************
* Function Name :  App_Key_KeyCapture
* Description   :  输入端口扫描  10ms调用一次
* Para          :  pInputKey--指向存储电口状态的数组的指针   pKeySts- 物理电平状态  pCtrlRegTabl- 设置属性指针
* Return        :  void
*********************************************************************************************************************/
static void  App_Key_KeyCapture( volatile InputIoMeg_T *pInputKey, volatile uint8_t *pKeySts, const uint8_t (*pCtrlRegTabl)[3] )
{
   uint8_t i=0;
   
   App_Key_KeyScan( pKeySts );
   
   for(i=0; i<MAX_BUTTON_NUM; i++, pInputKey++)
   {                          
        if( 0 == pInputKey->StsReg.CaptEn )
        { 
            continue;
        }

		pInputKey->StsReg.CurPhySts = pKeySts[i];                
		if( pInputKey->StsReg.CurPhySts != pInputKey->StsReg.PrevSts )
		{
			pInputKey->CtrlReg.SampCnt++;
			
			if( pInputKey->StsReg.CurPhySts == pCtrlRegTabl[i][1] )
			{
				if( 0 == HAL_Touch_GetIICBusState() )  //IIC通信失败
				{
					pInputKey->CtrlReg.SampCnt = 0;
				}
			}
		}
		else
		{
			pInputKey->CtrlReg.SampCnt = 0;
		}   

		if( pInputKey->CtrlReg.SampCnt == pCtrlRegTabl[i][0] )
		{
		    pInputKey->CtrlReg.SampCnt = 0;

			if( pInputKey->StsReg.CurPhySts == pCtrlRegTabl[i][1] )  
			{
				pInputKey->StsReg.ValidSts = 1;   
				
				if( pInputKey->CtrlReg.PopTime <= TIMER_DOUBLE_PUSH_MS )
				{
					pInputKey->StsReg.DoublePushSts = 1;   
				}
				else 
				{
					pInputKey->StsReg.DoublePushSts = 0;  
				}
			}
			else
			{
				pInputKey->StsReg.ValidSts = 0;  			   
			}
			pInputKey->StsReg.PrevSts = pInputKey->StsReg.CurPhySts;   
		} 
		
		if( 1 == pInputKey->StsReg.ValidSts )
		{
		   if( pInputKey->CtrlReg.PushTime < 0xFFFE )
		   {
			   pInputKey->CtrlReg.PushTime++;   
		   }   
		   pInputKey->CtrlReg.PopTime = 0; 
		}
		else if( 0 == pInputKey->StsReg.ValidSts )
		{
		   pInputKey->CtrlReg.PushTime = 0;  
		   if( pInputKey->CtrlReg.PopTime < 0xFFFE )
		   {
			   pInputKey->CtrlReg.PopTime++;     
		   }   
		   if( pInputKey->CtrlReg.PopTime > TIMER_DOUBLE_PUSH_MS )
		   {
			  pInputKey->StsReg.DoublePushSts = 0;   
		   }
		}  
   }
}

/*********************************************************************************************************************
* Function Name :  App_Key_GetKeyValidState()
* Description   :  查询按键是否有效按下  已消抖
* Para          :  type- 按键类型
* Return        :  1：按键有效按下	0：按键没按下
*********************************************************************************************************************/
bool App_Key_GetKeyValidState( KEY_TYPE_E type )
{
	return InputKeyTabl[ type ].StsReg.ValidSts;
}

/*********************************************************************************************************************
* Function Name :  App_Key_GetDoublePushState()
* Description   :  查询按键是否按下2次有效
* Para          :  type- 按键类型
* Return        :  1:两次有效	0：两次无效
*********************************************************************************************************************/
static bool App_Key_GetDoublePushState( KEY_TYPE_E type )
{
	return InputKeyTabl[ type ].StsReg.DoublePushSts;
}

/*********************************************************************************************************************
* Function Name :  App_Key_GetPushTime()
* Description   :  查询按键按下时间
* Para          :  type- 按键类型
* Return        :  按键按下时间 单位：10ms
*********************************************************************************************************************/
static uint16_t App_Key_GetPushTime( KEY_TYPE_E type )
{
	return InputKeyTabl[ type ].CtrlReg.PushTime;
}

/*********************************************************************************************************************
* Function Name :  App_Key_ResetPressTime()
* Description   :  清除按键按下时间
* Para          :  type- 按键类型
* Return        :  none
*********************************************************************************************************************/
static void App_Key_ResetPushTime( KEY_TYPE_E type )
{
	InputKeyTabl[ type ].CtrlReg.PushTime = 0;
}
 
/*********************************************************************************************************************
* Function Name :  App_Key_GetOpenHandleSts()
* Description   :  获取把手开门信号    右开门电机和把手方向一致  把手方向判定是从里至外
* Para          :  none
* Return        :  1：按键有效按下	0：按键没按下
*********************************************************************************************************************/
bool App_Key_GetOpenHandleSts( void )
{
    #ifdef HANDLER_MIDDLE_ON
	if( 1 == App_Key_GetKeyValidState( MIDDLE_HANDLE )) 
	{
		return 1;
	}
	#else 
	uint8_t ret = App_Key_GetMotorTurnDir();
	if( 1 == ret )      //左开
	{
 	   	if( 1 == App_Key_GetKeyValidState( RIGHT_HANDLE )) 
		{
			return 1;
		}
	}
	else if( 2 == ret ) //右开
	{
 	   	if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE )) 
		{
			return 1;
		}
	}
	#endif
 
    return 0;
}

/*********************************************************************************************************************
* Function Name :  App_Key_GetCloseHandleSts()
* Description   :  获取把手关门信号    把手方向判定是从里至外
* Para          :  none
* Return        :  1：按键有效按下	0：按键没按下
*********************************************************************************************************************/
bool App_Key_GetCloseHandleSts( void )
{
	uint8_t ret = App_Key_GetMotorTurnDir();
	if( 1 == ret )      //左开
	{
 	   	if( 1 == App_Key_GetKeyValidState( LEFT_HANDLE )) 
		{
			return 1;
		}
	}
	else if( 2 == ret ) //右开
	{
 	   	if( 1 == App_Key_GetKeyValidState( RIGHT_HANDLE )) 
		{
			return 1;
		}
	}
 
    return 0;
}

/*********************************************************************************************************************
* Function Name :  GetCombinButtonState()
* Description   :  获取组合按键状态
* Para          :  无
* Return        :  获得按键的组合状态    
*********************************************************************************************************************/
BUTTON_TYPE_E App_Key_GetCombinKeyState( void ) 
{
#ifdef FRAME_PLATFORM_FULLY_AUTO_ON        //全自动平台
	uint8_t open, close;
	static uint8_t  doublePushFlg;
	static uint16_t lockOpenTim; 
	static uint16_t lockCloseTim; 
	
	if( buttonCheckFlg == 0 )
	{
		lockOpenTim    = 0;
		lockCloseTim   = 0;
		App_Key_ResetPushTime( OPEN_KEY );
		App_Key_ResetPushTime( CLOSE_KEY );
	}
	
	open  = App_Key_GetKeyValidState( OPEN_KEY );
	close = App_Key_GetKeyValidState( CLOSE_KEY );
	
	if( (open == 1) && (close == 1) )   //组合键
	{
		if( buttonCheckFlg != 1 )
		{
			buttonCheckFlg = 1;
			lockOpenTim    = 0;
			lockCloseTim   = 0;
			App_Key_ResetPushTime( OPEN_KEY );
			App_Key_ResetPushTime( CLOSE_KEY );
		}
		
		lockOpenTim  = App_Key_GetPushTime( OPEN_KEY );
		lockCloseTim = App_Key_GetPushTime( CLOSE_KEY );
		if( (lockOpenTim >= TIMER_5000_MS) && (lockCloseTim >= TIMER_5000_MS) )	
		{
			buttonCheckFlg = 0;
			return EM_BACK_FACTORY_KEY;
		}
	}
	else if( close == 1 ) //关门键
	{
		if( buttonCheckFlg != 2 )
		{
			buttonCheckFlg = 2;
			lockOpenTim    = 0;
			lockCloseTim   = 0;
			App_Key_ResetPushTime( OPEN_KEY );
			App_Key_ResetPushTime( CLOSE_KEY );
		}
		lockCloseTim = App_Key_GetPushTime( CLOSE_KEY );
	    if( lockCloseTim >= TIMER_5000_MS )
		{
			buttonCheckFlg = 0;
			return EM_ENTER_APP_MODEL_KEY;
		}
	}
	else if( open == 1 )   //开门键
	{
		if( buttonCheckFlg != 3 )
		{
			buttonCheckFlg = 3;
			lockOpenTim    = 0;
			lockCloseTim   = 0;
			App_Key_ResetPushTime( OPEN_KEY );
			App_Key_ResetPushTime( CLOSE_KEY );
		}
		lockOpenTim = App_Key_GetPushTime( OPEN_KEY );
		if( 1 == App_Key_GetDoublePushState( OPEN_KEY ) )
		{
			doublePushFlg = 1;
		}
		else if( lockOpenTim >= TIMER_5000_MS )
		{
			buttonCheckFlg = 0;
			doublePushFlg = 0;
			return EM_ENTER_LOCAL_MODEL_KEY;
		}
		else 
		{
			doublePushFlg = 0;
		}
	}	
	else 
	{
		if( buttonCheckFlg == 1 )       //组合释放
		{
			if( lockCloseTim > 0 )
			{
				buttonCheckFlg = 0;
				return EM_CLOSE_DOOR_KEY;
			}
		}
		else if( buttonCheckFlg == 2 ) //关门键释放
		{
			if( lockCloseTim > 0 )
			{
				buttonCheckFlg = 0;
				return EM_CLOSE_DOOR_KEY;
			}
		}
		else if( buttonCheckFlg == 3 ) //开门键释放
		{
			if( doublePushFlg == 1 )
			{
				doublePushFlg = 0;
				buttonCheckFlg = 0;
				return EM_OPEN_DOOR_KEY;
			}
		}
		buttonCheckFlg = 0;
	}
	if( buttonCheckFlg != 0 )
	{
		return EM_SCAN_NONE_KEY;
	}
	
#elif defined FRAME_PLATFORM_HALF_AUTO_ON  //半自动平台
	if( buttonCheckFlg == 0 )
	{
		App_Key_ResetPushTime( REGISTER_KEY );
	}
	
	if( 1 == App_Key_GetKeyValidState( REGISTER_KEY ))  //有按键被按下
	{
		buttonCheckFlg = 1;
		if( App_Key_GetPushTime( REGISTER_KEY ) >= TIMER_5000_MS)
		{
			buttonCheckFlg = 0;
			if( E_SYSTEM_SELFCHECK == App_GUI_GetSystemWorkSts() )
			{
				return EM_BACK_FACTORY_KEY;
			}
			else 
			{
				return EM_ENTER_APP_MODEL_KEY;
			}
		}
	}
	else 
	{
		buttonCheckFlg = 0;
	}
	if( buttonCheckFlg != 0 )  //有键被按下
	{
		return EM_SCAN_NONE_KEY;
	}
#endif
	
    return EM_SCANNING_KEY;
}

/*********************************************************************************************************************
* Function Name :  App_Key_ResetCombinKeyFlow()
* Description   :  强制复位组合按键检测流程     组合按键正常情况下可以自动复位，但是在被其他流程打断的情况下再次使用需要手动复位
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Key_ResetCombinKeyFlow( void ) 
{
	buttonCheckFlg = 0;
}

/*********************************************************************************************************************
* Function Name :  App_Key_ScanKeyProcess()
* Description   :  按键扫描进程
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_Key_ScanKeyProcess( void )
{
	App_Key_KeyCapture( InputKeyTabl, InputKeyStatus, CtrlRegTab );
}



/*-------------------------------------------------THE FILE END-----------------------------------------------------*/


