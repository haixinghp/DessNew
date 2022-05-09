/*********************************************************************************************************************
 * @file:        App_Touch.h
 * @author:      gushengchi
 * @version:     V01.00
 * @date:        2021-07-30
 * @Description: ��������Ӧ�ù��ܺ����ļ�
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_TOUCH_H
#define  _APP_TOUCH_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include "..\HAL\HAL_Touch\HAL_Touch.h"  
/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define  KEY_BUF_SIZE        16   //��λ���볤��

/*--------------------------------------------------ö������---------------------------------------------------------*/
typedef enum
{
   EM_SCAN_OFF =0,	     //�ر�
   EM_SCAN_ON,	         //ȫ������
   EM_SCAN_BACK_ENTER,	 //ֻɨ�� ENTER + BACK ��
   EM_SCAN_KEY_ENTER,	 //ֻɨ�� ENTER ��
   EM_SCAN_KEY_BACK,	 //ֻɨ�� BACK  ��
   EM_SCAN_NONE_LOCK,	 //��ɨ�� LOCK  ��
   EM_SCAN_ONLY_BELL,	 //��ɨ�� BELL  ��
   EM_SCAN_PAGE_TURN,	 //��ҳ�˵�(0/8/* ������)
   EM_SCAN_BACK_BELL,	 //ֻɨ�� BACK + BELL ��
	
   EM_SCAN_SLEEP,	     //���׹ر�
	
}KEY_SCAN_CTRL_E;  
  
  
/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/             
 
 
/*--------------------------------------------------��������---------------------------------------------------------*/
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



