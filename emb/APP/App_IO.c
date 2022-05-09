/********************************************************************************************************************
 * @file:        App_Input.c
 * @author:      lixiqun
 * @version:     V01.00
 * @date:        2022-04-14
 * @Description: ����ڶ˿ڲɼ���������   ʵ��ʵʱ�ɼ���չ�ڵĵ�ƽ״̬��ҵ���߼�������ֱ���ý��
 * @ChangeList:  01. ����
*********************************************************************************************************************/
#include "App_IO.h"
#include "App_Export.h" 
#include "App_Key.h" 
#include "LockConfig.h"
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
/*-------------------------------------------------��������---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_Input_GetPinState( PIN_INPUT_TYPE_E type )
* Description   :  ��ȡ������Ч��ƽ
* Para          :  type - ��������
* Return        :  ����״̬  0-��Ч 1-��Ч
*********************************************************************************************************************/
bool App_Input_GetPinState( PIN_INPUT_TYPE_E type )
{
	uint8_t ret = 0;
	switch(type)
	{
		case E_INPUT_FINGER_IRQ: //ָ�Ƽ������
			#if defined MCU_FINGER_IRQ //���Ž���MCU��
				ret = App_Key_GetKeyValidState(FINGER_IRQ);
			#else //���Ž���9555��
				ret = App_Export_GetPinState( E_PIN_FINGER_IRQ );
			#endif
			break;
		case E_INPUT_ALARM_IRQ: //���˼������
			#if defined MCU_ALARM_IRQ //���Ž���MCU��
				ret = App_Key_GetKeyValidState(ALARM_IRQ);
			#else
				ret = App_Export_GetPinState( E_PIN_ALARM_IRQ );
			#endif
			break;
		case E_INPUT_SENSE_IRQ: //��Ӧ�������
			#if defined MCU_SENSE_IRQ //���Ž���MCU��
				ret = App_Key_GetKeyValidState(SENSE_IRQ);
			#else
				ret = App_Export_GetPinState( E_PIN_SENSE_IRQ );
			#endif
			break;
		case E_INPUT_TOUCH_IRQ://���������������
			#if defined MCU_TOUCH_IRQ
				ret = App_Key_GetKeyValidState(TOUCH_IRQ);
			#else
				ret = App_Export_GetPinState( E_PIN_TOUCH_IRQ );
			#endif
			break;
		case E_INPUT_CAMERA_IRQ:
				HAL_EXPORT_PinGet( EM_CAMERA_IRQ );
			break;
		case E_INPUT_RTC_IRQ:
				HAL_EXPORT_PinGet( EM_RTC_INT );
			break;
		case E_INPUT_OPEN_KEY:
				ret = App_Key_GetKeyValidState( OPEN_KEY );
			break;
		case E_INPUT_CLOSE_KEY:
				ret = App_Key_GetKeyValidState( CLOSE_KEY );
			break;
		case E_INPUT_LEFT_HANDLE:
				ret = App_Key_GetKeyValidState( LEFT_HANDLE );
			break;
		case E_INPUT_MIDDLE_HANDLE:
				ret = App_Key_GetKeyValidState( MIDDLE_HANDLE );
			break;
		case E_INPUT_RIGHT_HANDLE:
				ret = App_Key_GetKeyValidState( RIGHT_HANDLE );
			break;
		case E_INPUT_REGISTER_KEY:
				ret = App_Key_GetKeyValidState( REGISTER_KEY );
			break;
		case E_INPUT_ADJUST_LEVEL:
				ret = App_Key_GetKeyValidState( ADJUST_LEVEL );
			break;
		default:
			break;
	}
	return ret;
}