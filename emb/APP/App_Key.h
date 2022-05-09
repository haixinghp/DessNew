/*********************************************************************************************************************
 * @file:        App_Key.h
 * @author:      gushenghci
 * @version:     V01.00
 * @date:        2021-08-04
 * @Description: ����尴���ӿڹ��ܺ����ļ�
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_KEY_H
#define  _APP_KEY_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "LockConfig.h"
/*--------------------------------------------------�궨��-----------------------------------------------------------*/
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
/*--------------------------------------------------ö������---------------------------------------------------------*/

typedef enum
{
    EM_SCANNING_KEY = 0,         //ɨ����,�ް���
	EM_SCAN_NONE_KEY,	         //�м����µ�δ���ɰ���     
	EM_OPEN_DOOR_KEY,	         //���� 	
	EM_CLOSE_DOOR_KEY,	         //����	
	EM_BACK_FACTORY_KEY,	     //�ָ���������
	EM_ENTER_APP_MODEL_KEY,	     //����APPģʽ
	EM_ENTER_LOCAL_MODEL_KEY,	 //���빤��ģʽ
	EM_ENTER_OTA_MODEL_KEY,	     //��������ģʽ
	
}BUTTON_TYPE_E;

typedef enum
{
   OPEN_KEY = 0,	//���ż�
   CLOSE_KEY,	 	//���ż�
   LEFT_HANDLE,     //������
   MIDDLE_HANDLE,   //������ 
   RIGHT_HANDLE,    //�����ҿ�	
   REGISTER_KEY,    //ע���
   FINGER_IRQ,		//ָ��
   ALARM_IRQ,		//����
   SENSE_IRQ,		//�ӽ��������ж�
   TOUCH_IRQ,		//��������
   ADJUST_LEVEL     //���Զ˿�״̬��
	
}KEY_TYPE_E;

/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/             
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
/*--------------------------------------------------��������---------------------------------------------------------*/
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
 
