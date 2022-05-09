/********************************************************************************************************************
 * @file:        App_Input.c
 * @author:      lixiqun
 * @version:     V01.00
 * @date:        2022-04-14
 * @Description: 输入口端口采集功能数据   实现实时采集扩展口的电平状态，业务逻辑处可以直接用结果
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
#include "App_IO.h"
#include "App_Export.h" 
#include "App_Key.h" 
#include "LockConfig.h"
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_Input_GetPinState( PIN_INPUT_TYPE_E type )
* Description   :  获取输入有效电平
* Para          :  type - 引脚类型
* Return        :  引脚状态  0-无效 1-有效
*********************************************************************************************************************/
bool App_Input_GetPinState( PIN_INPUT_TYPE_E type )
{
	uint8_t ret = 0;
	switch(type)
	{
		case E_INPUT_FINGER_IRQ: //指纹检测引脚
			#if defined MCU_FINGER_IRQ //引脚接在MCU上
				ret = App_Key_GetKeyValidState(FINGER_IRQ);
			#else //引脚接在9555上
				ret = App_Export_GetPinState( E_PIN_FINGER_IRQ );
			#endif
			break;
		case E_INPUT_ALARM_IRQ: //防撬检测引脚
			#if defined MCU_ALARM_IRQ //引脚接在MCU上
				ret = App_Key_GetKeyValidState(ALARM_IRQ);
			#else
				ret = App_Export_GetPinState( E_PIN_ALARM_IRQ );
			#endif
			break;
		case E_INPUT_SENSE_IRQ: //感应检测引脚
			#if defined MCU_SENSE_IRQ //引脚接在MCU上
				ret = App_Key_GetKeyValidState(SENSE_IRQ);
			#else
				ret = App_Export_GetPinState( E_PIN_SENSE_IRQ );
			#endif
			break;
		case E_INPUT_TOUCH_IRQ://触摸按键检测引脚
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