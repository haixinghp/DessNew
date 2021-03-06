/*********************************************************************************************************************
 * @file:        App_Touch.h
 * @author:      gushengchi
 * @version:     V01.00
 * @date:        2021-07-30
 * @Description: 触摸按键应用功能函数文件
 * @ChangeList:  01. 初版
**********************************************************************************************************************/
  
#ifndef  _APP_TOUCH_H
#define  _APP_TOUCH_H

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include "..\HAL\HAL_Touch\HAL_Touch.h"  
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define  KEY_BUF_SIZE        16   //虚位密码长度

/*--------------------------------------------------枚举声明---------------------------------------------------------*/
typedef enum
{
   EM_SCAN_OFF =0,	     //关闭
   EM_SCAN_ON,	         //全部开启
   EM_SCAN_BACK_ENTER,	 //只扫描 ENTER + BACK 键
   EM_SCAN_KEY_ENTER,	 //只扫描 ENTER 键
   EM_SCAN_KEY_BACK,	 //只扫描 BACK  键
   EM_SCAN_NONE_LOCK,	 //不扫描 LOCK  键
   EM_SCAN_ONLY_BELL,	 //仅扫描 BELL  键
   EM_SCAN_PAGE_TURN,	 //翻页菜单(0/8/* 键可用)
   EM_SCAN_BACK_BELL,	 //只扫描 BACK + BELL 键
	
   EM_SCAN_SLEEP,	     //彻底关闭
	
}KEY_SCAN_CTRL_E;  
  
  
/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
 
 
/*--------------------------------------------------函数声明---------------------------------------------------------*/
void App_Touch_FileInit( void ); 
void App_Touch_WakeupInit( void ); 
void App_Touch_SleepInit( void ); 
void App_Touch_Tim10Ms( void ); 
void App_Touch_FuncEnCtrl( KEY_SCAN_CTRL_E cmd ); 

uint8_t App_Touch_GetCurrentKeyIndex( void );
uint8_t App_Touch_GetCurrentKeyValue( void );
uint8_t App_Touch_GetKeyTipsTime( void ); 

void App_Touch_GetCurrentKeyValBuf( uint8_t *pdata, uint8_t *pkeyNum );
void App_Touch_MainProcess( void ); 

#endif


/*--------------------------------------------------THE FILE END------------------------------------------------------*/




