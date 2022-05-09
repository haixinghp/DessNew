/********************************************************************************************************************
 * @file:        App_Touch.c
 * @author:      gushengchi
 * @version:     V01.00
 * @date:        2021-07-30
 * @Description: 触摸按键应用功能函数文件
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdbool.h>
#include "Public.h"

#include "App_Touch.h" 
#include "App_LED.h" 
#include "App_GUI.h" 

#include "..\HAL\HAL_VOICE\HAL_Voice.h"
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define  TOUCH_TIM_30MS      3 
#define  TOUCH_TIM_50MS      5 
#define  TOUCH_TIM_8000MS    800 
/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/
 

/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         
typedef struct
{
	uint8_t CurrentKeyVal;                   //当前键值
	uint8_t CurrentKeyIdx;                   //当前键值索引
	uint8_t KeyValBuf[ KEY_BUF_SIZE ];       //当前键值缓存
 
}KeyScanMeg_T;

static  KeyScanMeg_T  KeyMsg ={0};
 
static  uint16_t  TouchKeyTick = 0;
static  uint8_t   TouchTipsTimMs = 0;
static  uint8_t   TouchVal = 0;

static  KEY_SCAN_CTRL_E  TouchKeyEnable = EM_SCAN_OFF;
/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
 

/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_Touch_FileInit()
* Description   :  功能文件初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_Touch_FileInit( void ) 
{
    HAL_Touch_FileInit(); 
	App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
}
 
/*********************************************************************************************************************
* Function Name :  App_Touch_WakeupInit()
* Description   :  唤醒后配置
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_Touch_WakeupInit( void ) 
{
	HAL_Touch_FileInit(); 
	App_Touch_FuncEnCtrl( EM_SCAN_OFF );
}

/*********************************************************************************************************************
* Function Name :  App_Touch_SleepInit()
* Description   :  休眠后配置
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_Touch_SleepInit( void ) 
{
	HAL_Touch_SleepInit();	
	App_Touch_FuncEnCtrl( EM_SCAN_OFF ); 
}

/*********************************************************************************************************************
* Function Name :  App_Touch_Tim10Ms()
* Description   :  功能相关定时器   10ms触发一次
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_Touch_Tim10Ms( void ) 
{
   if( TouchKeyTick > 0 )
	   TouchKeyTick--;
   
   if( TouchTipsTimMs > 0 )
	   TouchTipsTimMs--;
   
}

/*********************************************************************************************************************
* Function Name :  MemcpyByteTobyte()
* Description   :  复制数组
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void MemcpyByteTobyte( uint8_t *pbuf1, const uint8_t *pbuf2, uint8_t size)
{
   if( size == 0 )
	   return;
	
   while( size-- )
   {
	  *pbuf1++ = *pbuf2++;  
   }	
}

/*********************************************************************************************************************
* Function Name :  MemcpyByteTobyte()
* Description   :  复制数组
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void MemsetByte( uint8_t *pbuf, uint8_t val, uint8_t size)
{
   if( size == 0 )
	   return;
	
   while( size-- )
   {
	  *pbuf++ = val;  
   }	
}


/*********************************************************************************************************************
* Function Name :  App_Touch_GetKeyTipsTime()
* Description   :  获取按键水滴声的播放时间   
* Para          :  none 
* Return        :  剩余时间
*********************************************************************************************************************/
uint8_t App_Touch_GetKeyTipsTime( void ) 
{
    return TouchTipsTimMs;
}

/*********************************************************************************************************************
* Function Name :  App_Touch_FuncEnCtrl()
* Description   :  按键功能使能控制   
* Para          :  cmd- 控制命令    
* Return        :  void
*********************************************************************************************************************/
void App_Touch_FuncEnCtrl( KEY_SCAN_CTRL_E cmd ) 
{
    TouchKeyEnable = cmd;

	TouchVal = INVALUE_KEY;
	KeyMsg.CurrentKeyIdx = 0;
	KeyMsg.CurrentKeyVal = TOUCH_KEY_NONE;
	MemsetByte( KeyMsg.KeyValBuf, 0, KEY_BUF_SIZE );
}

/*********************************************************************************************************************
* Function Name :  App_Touch_GetCurrentKeyIndex()
* Description   :  获取当前键值总个数
* Para          :  无
* Return        :  键值总个数
*********************************************************************************************************************/
uint8_t App_Touch_GetCurrentKeyIndex( void )
{
	return  KeyMsg.CurrentKeyIdx;  	
}	

/*********************************************************************************************************************
* Function Name :  App_Touch_GetCurrentKeyValue()
* Description   :  获取当前键值  用完就销毁
* Para          :  无
* Return        :  当前键值 
*********************************************************************************************************************/
uint8_t App_Touch_GetCurrentKeyValue( void )
{
	uint8_t tmp = KeyMsg.CurrentKeyVal;
	KeyMsg.CurrentKeyVal = TOUCH_KEY_NONE;
	return  tmp;  
}	

/*********************************************************************************************************************
* Function Name :  App_Touch_GetCurrentKeyValBuf()
* Description   :  获取按键数据簇和数据长度    取出后自动销毁
* Para          :  pdata-待取出的数据指针  pkeyNum- 待取出的数据长度
* Return        :  void
*********************************************************************************************************************/
void App_Touch_GetCurrentKeyValBuf( uint8_t *pdata, uint8_t *pkeyNum )
{
    *pkeyNum = KeyMsg.CurrentKeyIdx;
	 MemcpyByteTobyte( pdata, KeyMsg.KeyValBuf, KeyMsg.CurrentKeyIdx );
 
	KeyMsg.CurrentKeyIdx = 0;
    KeyMsg.CurrentKeyVal = TOUCH_KEY_NONE;
	MemsetByte( KeyMsg.KeyValBuf, 0, KEY_BUF_SIZE );
}

/*********************************************************************************************************************
* Function Name :  App_Touch_MainProcess()
* Description   :  主进程
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_Touch_MainProcess( void ) 
{
	static uint8_t step;
 
	if( EM_SCAN_SLEEP == TouchKeyEnable )  //功能关闭
	{
		step = 0;
		return;
	}
	uint8_t keyVal = HAL_Touch_GetKeyValue();
	
	switch( step )
	{
	    case 0:  //确认是否有按键被按下
				if( EM_SCAN_OFF == TouchKeyEnable )  //功能关闭
				{
					return;
				}
				else if( EM_SCAN_BACK_ENTER == TouchKeyEnable )  //ENTER + BACK
				{
					if( (TOUCH_KEY_ENTER != keyVal) && (TOUCH_KEY_BACK != keyVal) )
					{
						return;
					}
				}
				else if( EM_SCAN_KEY_ENTER == TouchKeyEnable )  //ENTER  
				{
					if( TOUCH_KEY_ENTER != keyVal )
					{
						return;
					}
				}
				else if( EM_SCAN_KEY_BACK == TouchKeyEnable )   //BACK  
				{
					if( TOUCH_KEY_BACK != keyVal )
					{
						return;
					}
				}
				else if( EM_SCAN_NONE_LOCK == TouchKeyEnable )   //EXCEPT LOCK 
				{
					if( TOUCH_KEY_LOCK == keyVal )
					{
						return;
					}
				}
				else if( EM_SCAN_ONLY_BELL == TouchKeyEnable )   //ONLY BELL 
				{
					if( TOUCH_KEY_BELL != keyVal )
					{
						return;
					}
				}
				else if( EM_SCAN_BACK_BELL == TouchKeyEnable )   // BACK + BELL 键
				{
					if( (TOUCH_KEY_BELL != keyVal) && (TOUCH_KEY_BACK != keyVal) )
					{
						return;
					}
				}
				else if( EM_SCAN_PAGE_TURN == TouchKeyEnable )   //0/8/*
				{
					if( TOUCH_KEY_NO_0 != keyVal && TOUCH_KEY_NO_8 != keyVal && TOUCH_KEY_BACK != keyVal )
					{
						return;
					}
				}
				
				if( keyVal != NONE_KEY )
				{
					App_GUI_UpdateMenuQuitTime( TOUCH_TIM_8000MS, false ); //刷新GUI休眠时间
					TouchVal = keyVal; 
					KeyMsg.CurrentKeyVal = keyVal;
					if( KeyMsg.CurrentKeyIdx < KEY_BUF_SIZE )
					{
						if( KeyMsg.CurrentKeyVal <= TOUCH_KEY_NO_9 )  // 数字键
						{
							KeyMsg.KeyValBuf[ KeyMsg.CurrentKeyIdx++ ] = KeyMsg.CurrentKeyVal;
						}
					}
					if( KeyMsg.CurrentKeyVal <= TOUCH_KEY_BACK )     //按键灯有效键值
					{
						App_LED_OutputCtrl( (LED_TYPE_E)TouchVal, EM_LED_OFF );  
					}
					
					if( (KeyMsg.CurrentKeyVal <= TOUCH_KEY_BACK )||( KeyMsg.CurrentKeyVal == TOUCH_KEY_LOCK ))     //按键灯有效键值
					{
						HAL_Voice_PlayingVoice( EM_BUTTON_TIPS_MP3, TOUCH_TIM_30MS );  
						TouchTipsTimMs = 5;
					}
					step = 1;
				}
		break;
		
	    case 1: //确认是否有按键被释放
				if( TouchVal == INVALUE_KEY )   //界面切换用
				{
					if( keyVal == NONE_KEY )
					{
						if( TouchVal <= TOUCH_KEY_BACK )      
						{
							App_LED_OutputCtrl( (LED_TYPE_E)TouchVal, EM_LED_ON );  
						}
						step = 0;
					}
				}
				else if( keyVal != TouchVal )  //按键变化用
				{
					if( TouchVal <= TOUCH_KEY_BACK )      
					{
						App_LED_OutputCtrl( (LED_TYPE_E)TouchVal, EM_LED_ON );  
					}
					step = 0;
				}
		break;
				
		default:break;
	}
}


 

/*-------------------------------------------------THE FILE END-----------------------------------------------------*/

