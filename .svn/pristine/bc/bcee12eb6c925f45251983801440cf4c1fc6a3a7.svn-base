/*********************************************************************************************************************
 * @file:      App_LED.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-07-28
 * @brief:     按键灯的功能接口函数头文件
**********************************************************************************************************************/
  
#ifndef  _APP_LED_H
#define  _APP_LED_H

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h> 
 
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define  FUNC_ENABLE    1
#define  FUNC_DISABLE   0

#define  BREATH_LED_R     HW_LED_LOCK_R 
#define  BREATH_LED_G     HW_LED_LOCK_G 
#define  BREATH_LED_B     HW_LED_LOCK_B 
#define  BREATH_LED_W     HW_LED_LOCK_W 

#define  APP_LED_DAYTIME_FLAG       (7*3600)        // 早上7点
#define  APP_LED_NIGHTTIME_FLAG     (18*3600)       // 晚上6点(18点)


/*--------------------------------------------------枚举声明---------------------------------------------------------*/
typedef enum
{
   EM_LED_0 = 0, 	
   EM_LED_1, 	
   EM_LED_2, 	
   EM_LED_3, 
   EM_LED_4, 	
   EM_LED_5, 	
   EM_LED_6,
   EM_LED_7, 	
   EM_LED_8, 	
   EM_LED_9, 
   EM_LED_ENTER,    	  
   EM_LED_CANCLE, 	
   EM_LED_POW_R, 
   EM_LED_POW_G, 
   EM_LED_POW_B, 
   EM_LED_LOCK_R, 
   EM_LED_LOCK_G, 
   EM_LED_LOCK_B,
   EM_LED_LOCK_W,
   EM_LED_BELL,
	
   EM_LED_ALL,      //全屏幕
   EM_LED_X,        //故障显示     13579
   EM_LED_CFG_NET,  //配网显示     25846
   EM_LED_PAGE_TURN,  //翻页显示    08*
   EM_LED_E,        //E2故障显示   25846
   
}LED_TYPE_E;

typedef enum
{
   EM_LED_OFF =0,	 //灭
   EM_LED_ON,	     //亮
	
}LED_CMD_E;

typedef enum
{
   EM_LED_RGB_MODE_OFF =    0,      // 关闭
   EM_LED_RGB_MODE_ALLDAY,	        // 全天模式- 24小时有效
   EM_LED_RGB_MODE_DAYTIME,         // 日间模式- 早上7点至晚上6点
   EM_LED_RGB_MODE_NIGHTTIME,       // 夜间模式- 晚上6点至早上7点
}LED_RGB_MODE_E;


/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
 
 
/*--------------------------------------------------函数声明---------------------------------------------------------*/
void App_LED_FileInit( void );
void App_LED_WakeupInit( void );
void App_LED_SleepInit( void );
void App_LED_Tim10Ms( void ); 
void App_LED_OutputCtrl( LED_TYPE_E type, LED_CMD_E cmd ); 
void App_LED_BreathLampCtrlEn( uint8_t channel, uint8_t cmd );  
void App_LED_BreathThread( void );  

#endif
/*--------------------------------------------------THE FILE END-----------------------------------------------------*/






