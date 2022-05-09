/********************************************************************************************************************
 * @file:        App_Export.c
 * @author:      gushengchi
 * @version:     V01.00
 * @date:        2021-08-31
 * @Description: ��չ�ڶ˿ڲɼ���������   ʵ��ʵʱ�ɼ���չ�ڵĵ�ƽ״̬��ҵ���߼�������ֱ���ý��
 * @ChangeList:  01. ����
*********************************************************************************************************************/
  
/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
#include "App_Export.h" 
 
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
/*-------------------------------------------------�궨��-----------------------------------------------------------*/
#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON 
    #define NEAR_SENSE_EN   1
#else
    #define NEAR_SENSE_EN   0
#endif 

/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/


/*-------------------------------------------------��������---------------------------------------------------------*/
static const uint8_t CtrlRegTab[ MAX_PIN_NUM ][ 2 ] = 
{
      	/*��Ч��ƽ*/   /*ʹ��*/
    {      	0,    	  	 1   	 },       //FINGER_IRQ  
	{      	0,      	 1   	 },       //ALARM_IRQ 
    {      	0,     NEAR_SENSE_EN },       //SENSE_IRQ  
	{       0,      	 1   	 },       //TOUCH_IRQ 
};
 

/*-------------------------------------------------ȫ�ֱ�������-----------------------------------------------------*/         
 

/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/
static uint16_t ExportSystick = 0; 

typedef struct  
{
    struct  
    {
		uint8_t ValidSts      :1;               
		uint8_t CaptEn        :1;     		
		uint8_t Reverse       :6; 
              
    }StsReg;       
}ExportMeg_T; 

static volatile ExportMeg_T ExportMeg[ MAX_PIN_NUM ];
static volatile uint8_t  InputPinStatus[ MAX_PIN_NUM ] ={0}; 
static volatile bool  AlarmWorkEn = 1;   			//���˼�⹦����Ч  
/*-------------------------------------------------��������---------------------------------------------------------*/
 

/*-------------------------------------------------��������---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_Export_FileInit()
* Description   :  ����ļ���ʼ��   
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_Export_FileInit( void )
{
    HAL_EXPORT_FileInit(); 
	
	(void)HAL_EXPORT_PinInit( EM_FING_IRQ, DIR_IN, POLO_RETTAIN ); 	
    (void)HAL_EXPORT_PinInit( EM_PIN_ALARM, DIR_IN, POLO_RETTAIN ); 
	(void)HAL_EXPORT_PinInit( EM_IR_IRQ, DIR_IN, POLO_RETTAIN ); 	
    (void)HAL_EXPORT_PinInit( EM_KEY_IRQ, DIR_IN, POLO_RETTAIN ); 	
	
	AlarmWorkEn = 1;
    ExportSystick = 0;
	for(uint8_t i=0; i<MAX_PIN_NUM; i++)
	{
	    ExportMeg[ i ].StsReg.CaptEn = CtrlRegTab[ i ][ 1 ];
		ExportMeg[ i ].StsReg.ValidSts = 0;
	}
} 

/*********************************************************************************************************************
* Function Name :  App_Export_WakeupInit()
* Description   :  ��������   
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_Export_WakeupInit( void )
{
	HAL_EXPORT_WakeupInit(); 
	
    ExportSystick = 0;
	for(uint8_t i=0; i<MAX_PIN_NUM; i++)
	{
	    ExportMeg[ i ].StsReg.CaptEn = CtrlRegTab[ i ][ 1 ];
		ExportMeg[ i ].StsReg.ValidSts = 0;
	}
} 

/*********************************************************************************************************************
* Function Name :  App_Export_SleepInit()
* Description   :  ��������   
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_Export_SleepInit( void )
{
	HAL_EXPORT_SleepInit(); 
} 
 
/*********************************************************************************************************************
* Function Name :  App_Export_Tim10Ms()
* Description   :  ������ض�ʱ��   10ms����һ��
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_Export_Tim10Ms( void )
{
	if( ExportSystick > 0 )
		ExportSystick--;
} 
 
/*********************************************************************************************************************
* Function Name :  App_Export_ScanPortThread()
* Description   :  ���ܵ���  10ms����һ��  
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_Export_ScanPortThread( void )
{
 	static uint8_t timecnt;
	
	timecnt++;
	timecnt %= 10;
	
	if( timecnt == 0 )
	{
		if( CtrlRegTab[ E_PIN_FINGER_IRQ ][ 1 ] == ExportMeg[ E_PIN_FINGER_IRQ ].StsReg.CaptEn )
		{
			if( CtrlRegTab[ E_PIN_FINGER_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_FING_IRQ ) )
			{
				
				ExportMeg[ E_PIN_FINGER_IRQ ].StsReg.ValidSts = 1; 
			}
			else 
			{
				ExportMeg[ E_PIN_FINGER_IRQ ].StsReg.ValidSts = 0; 
			}	
		}
	}
	else if( timecnt == 1 )
	{
		if( CtrlRegTab[ E_PIN_ALARM_IRQ ][ 1 ] == ExportMeg[ E_PIN_ALARM_IRQ ].StsReg.CaptEn )
		{
			if( CtrlRegTab[ E_PIN_ALARM_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_PIN_ALARM ) )
			{
				ExportMeg[ E_PIN_ALARM_IRQ ].StsReg.ValidSts = (AlarmWorkEn == 1)? 1:0; 
			}
			else 
			{
				ExportMeg[ E_PIN_ALARM_IRQ ].StsReg.ValidSts = 0; 
			}	
		}
	}
	else if( timecnt == 2 )
	{
		if( CtrlRegTab[ E_PIN_SENSE_IRQ ][ 1 ] == ExportMeg[ E_PIN_SENSE_IRQ ].StsReg.CaptEn )
		{
			if( CtrlRegTab[ E_PIN_SENSE_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_IR_IRQ ) )
			{
				ExportMeg[ E_PIN_SENSE_IRQ ].StsReg.ValidSts = 1; 
			}
			else 
			{
				ExportMeg[ E_PIN_SENSE_IRQ ].StsReg.ValidSts = 0; 
			}	
		}
	}
	else if( timecnt == 3 )
	{
	    if( CtrlRegTab[ E_PIN_TOUCH_IRQ ][ 1 ] == ExportMeg[ E_PIN_TOUCH_IRQ ].StsReg.CaptEn )
		{
			if(  CtrlRegTab[ E_PIN_TOUCH_IRQ ][ 0 ] == HAL_EXPORT_PinGet( EM_KEY_IRQ ) )
			{
				ExportMeg[ E_PIN_TOUCH_IRQ ].StsReg.ValidSts = 1; 
			}
			else 
			{
				ExportMeg[ E_PIN_TOUCH_IRQ ].StsReg.ValidSts = 0; 
			}	
		}		
	}
} 
 
/*********************************************************************************************************************
* Function Name :  App_Export_GetPinState()
* Description   :  ��ȡ��չ��������Ч��ƽ
* Para Inpuut   :  type - ��������
* Return        :  ����״̬  0-��Ч 1-��Ч
*********************************************************************************************************************/
bool App_Export_GetPinState( PIN_TYPE_E type )
{
	return ExportMeg[ type ].StsReg.ValidSts; 
}


/*********************************************************************************************************************
* Function Name :  App_Key_GetAlrmWarmState()
* Description   :  ��ȡ���˰���״̬
* Para          :  ��
* Return        : 0= ��Ч  1= ��Ч
*********************************************************************************************************************/
bool App_Export_GetAlrmWarmState( void )
{
	bool ret;
	uint8_t tp1 = HAL_EXPORT_PinGet( EM_PIN_ALARM );
	ret = (tp1 == CtrlRegTab[ E_PIN_ALARM_IRQ ][ 0 ]) ?  1 : 0;
	
	if( AlarmWorkEn == 0 )
		ret = 0;
	
	return ret;
}

/*********************************************************************************************************************
* Function Name :  App_Export_SetAlrmWarmEn()
* Description   :  ���÷��˰�����⹤��״̬  
* Para          :  0= ʧ��  1= ʹ��
* Return        :  void
*********************************************************************************************************************/
void App_Export_SetAlrmWarmEn( bool cmd )
{
	AlarmWorkEn = cmd;
}
 
/*********************************************************************************************************************
* Function Name :  App_Export_GetAlrmWarmEnSts()
* Description   :  ��ȡ���˰�����⹤��״̬  
* Para          :  none
* Return        :  0= ʧ��  1= ʹ��
*********************************************************************************************************************/
bool App_Export_GetAlrmWarmEnSts( void )
{
	return AlarmWorkEn;
}


/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
