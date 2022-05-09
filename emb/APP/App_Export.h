/*********************************************************************************************************************
 * @file:        App_Export.h
 * @author:      gushengchi
 * @version:     V01.00
 * @date:        2021-08-31
 * @Description: ��չ�ڶ˿ڲɼ���������
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_EXPORT_H
#define  _APP_EXPORT_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define MAX_PIN_NUM      4
 
/*--------------------------------------------------ö������---------------------------------------------------------*/
typedef enum
{
    E_PIN_FINGER_IRQ = 0,        //ָ�Ƽ������
	E_PIN_ALARM_IRQ,	         //���˼������
    E_PIN_SENSE_IRQ,             //��Ӧ�������
	E_PIN_TOUCH_IRQ,	         //���������������
	
}PIN_TYPE_E;
 
/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/             
 
 
/*--------------------------------------------------��������---------------------------------------------------------*/
void App_Export_FileInit( void ); 
void App_Export_WakeupInit( void );
void App_Export_SleepInit( void );
void App_Export_Tim10Ms( void );
void App_Export_ScanPortThread( void );
bool App_Export_GetPinState( PIN_TYPE_E type );
bool App_Export_GetAlrmWarmState( void );
void App_Export_SetAlrmWarmEn( bool cmd );
bool App_Export_GetAlrmWarmEnSts( void );

#endif



/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
 
