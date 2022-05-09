/*********************************************************************************************************************
 * @file:      App_LED.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-07-28
 * @brief:     �����ƵĹ��ܽӿں���ͷ�ļ�
**********************************************************************************************************************/
  
#ifndef  _APP_LED_H
#define  _APP_LED_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h> 
 
/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define  FUNC_ENABLE    1
#define  FUNC_DISABLE   0

#define  BREATH_LED_R     HW_LED_LOCK_R 
#define  BREATH_LED_G     HW_LED_LOCK_G 
#define  BREATH_LED_B     HW_LED_LOCK_B 
#define  BREATH_LED_W     HW_LED_LOCK_W 

#define  APP_LED_DAYTIME_FLAG       (7*3600)        // ����7��
#define  APP_LED_NIGHTTIME_FLAG     (18*3600)       // ����6��(18��)


/*--------------------------------------------------ö������---------------------------------------------------------*/
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
	
   EM_LED_ALL,      //ȫ��Ļ
   EM_LED_X,        //������ʾ     13579
   EM_LED_CFG_NET,  //������ʾ     25846
   EM_LED_PAGE_TURN,  //��ҳ��ʾ    08*
   EM_LED_E,        //E2������ʾ   25846
   
}LED_TYPE_E;

typedef enum
{
   EM_LED_OFF =0,	 //��
   EM_LED_ON,	     //��
	
}LED_CMD_E;

typedef enum
{
   EM_LED_RGB_MODE_OFF =    0,      // �ر�
   EM_LED_RGB_MODE_ALLDAY,	        // ȫ��ģʽ- 24Сʱ��Ч
   EM_LED_RGB_MODE_DAYTIME,         // �ռ�ģʽ- ����7��������6��
   EM_LED_RGB_MODE_NIGHTTIME,       // ҹ��ģʽ- ����6��������7��
}LED_RGB_MODE_E;


/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/             
 
 
/*--------------------------------------------------��������---------------------------------------------------------*/
void App_LED_FileInit( void );
void App_LED_WakeupInit( void );
void App_LED_SleepInit( void );
void App_LED_Tim10Ms( void ); 
void App_LED_OutputCtrl( LED_TYPE_E type, LED_CMD_E cmd ); 
void App_LED_BreathLampCtrlEn( uint8_t channel, uint8_t cmd );  
void App_LED_BreathThread( void );  

#endif
/*--------------------------------------------------THE FILE END-----------------------------------------------------*/






