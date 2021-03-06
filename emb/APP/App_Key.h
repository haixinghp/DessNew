/*********************************************************************************************************************
 * @file:        App_Key.h
 * @author:      gushenghci
 * @version:     V01.00
 * @date:        2021-08-04
 * @Description: 后面板按键接口功能函数文件
 * @ChangeList:  01. 初版
**********************************************************************************************************************/
  
#ifndef  _APP_KEY_H
#define  _APP_KEY_H

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "LockConfig.h"
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define KEY_OPEN_GPIO_PIN		  M_KEY_OPEN_GPIO_PIN
#define KEY_CLOSE_GPIO_PIN	      M_KEY_CLOSE_GPIO_PIN
#define HANDLE_LEFT_GPIO_PIN	  M_HANDLE_LEFT_GPIO_PIN
#define HANDLE_MIDDLE_GPIO_PIN	  M_HANDLE_MIDDLE_GPIO_PIN
#define HANDLE_RIGHT_GPIO_PIN	  M_HANDLE_RIGHT_GPIO_PIN

#ifndef M_BUTTON_REGISTER_GPIO_PIN
	#define BUTTON_REGISTER_GPIO_PIN  M_MCU_PIN_RERSVER
#else
	#define BUTTON_REGISTER_GPIO_PIN  M_BUTTON_REGISTER_GPIO_PIN
#endif

#ifndef M_FINGER_IRQ_GPIO_PIN
	#define FINGER_IRQ_GPIO_PIN  M_MCU_PIN_RERSVER
#else
	#define FINGER_IRQ_GPIO_PIN  M_FINGER_IRQ_GPIO_PIN
	#define MCU_FINGER_IRQ 1
#endif

#ifndef M_ALARM_IRQ_GPIO_PIN
	#define ALARM_IRQ_GPIO_PIN  M_MCU_PIN_RERSVER
#else
	#define ALARM_IRQ_GPIO_PIN  M_ALARM_IRQ_GPIO_PIN
	#define MCU_ALARM_IRQ 1
#endif

#ifndef M_SENSE_IRQ_GPIO_PIN
	#define SENSE_IRQ_GPIO_PIN  M_MCU_PIN_RERSVER
#else
	#define SENSE_IRQ_GPIO_PIN  M_SENSE_IRQ_GPIO_PIN
	#define MCU_SENSE_IRQ 1
#endif

#ifndef M_TOUCH_IRQ_GPIO_PIN
	#define TOUCH_IRQ_GPIO_PIN  M_MCU_PIN_RERSVER
#else
	#define TOUCH_IRQ_GPIO_PIN  M_TOUCH_IRQ_GPIO_PIN
	#define MCU_TOUCH_IRQ 1
#endif
/*--------------------------------------------------枚举声明---------------------------------------------------------*/

typedef enum
{
    EM_SCANNING_KEY = 0,         //扫描中,无按键
	EM_SCAN_NONE_KEY,	         //有键按下但未生成按键     
	EM_OPEN_DOOR_KEY,	         //开门 	
	EM_CLOSE_DOOR_KEY,	         //关门	
	EM_BACK_FACTORY_KEY,	     //恢复出厂设置
	EM_ENTER_APP_MODEL_KEY,	     //进入APP模式
	EM_ENTER_LOCAL_MODEL_KEY,	 //进入工程模式
	EM_ENTER_OTA_MODEL_KEY,	     //进入升级模式
	
}BUTTON_TYPE_E;

typedef enum
{
   OPEN_KEY = 0,	//开门键
   CLOSE_KEY,	 	//关门键
   LEFT_HANDLE,     //把手左开
   MIDDLE_HANDLE,   //把手中 
   RIGHT_HANDLE,    //把手右开	
   REGISTER_KEY,    //注册键
   FINGER_IRQ,		//指纹
   ALARM_IRQ,		//防撬
   SENSE_IRQ,		//接近传感器中断
   TOUCH_IRQ,		//触摸按键
   ADJUST_LEVEL     //调试端口状态用
	
}KEY_TYPE_E;

/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
typedef struct  
{
    struct  
    {
		uint8_t CurPhySts     :1;        
		uint8_t CurSts        :1;         
		uint8_t PrevSts       :1;         
		uint8_t ValidSts      :1;               
		uint8_t CaptEn        :1;   
		uint8_t DoublePushSts :1;     		
		uint8_t Reverse       :2; 
              
    }StsReg;
    struct CTRL_REG_TYPE 
    {
		uint8_t  SampCnt; 		
		uint16_t PopTime;  
		uint16_t PushTime;  
		
    }CtrlReg;  
                      
}InputIoMeg_T; 

extern volatile InputIoMeg_T InputKeyTabl[];  
/*--------------------------------------------------函数声明---------------------------------------------------------*/
void App_Key_FileInit( void );
void App_Key_WakeupInit( void );
void App_Key_SleepInit( void );
void App_Key_Tim10Ms( void );
void App_Key_ScanKeyProcess(void);
bool App_Key_GetOpenHandleSts( void );
bool App_Key_GetCloseHandleSts( void );
bool App_Key_GetKeyValidState( KEY_TYPE_E type );
	
BUTTON_TYPE_E App_Key_GetCombinKeyState( void );  
void App_Key_ResetCombinKeyFlow( void ); 

#endif



/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
 

