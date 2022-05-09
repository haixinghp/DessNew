#ifndef _APP_HUMANSENSOR_H_
#define _APP_HUMANSENSOR_H_
#ifdef __cplusplus
extern "C"
{
#endif

/* ��׼ͷ�ļ� */
#include "stdint.h"

/* �ڲ�ͷ�ļ� */
 
void App_HumanSensorFileInit(uint8_t distance );
void App_HumanSensor_Tim10Ms( void );
void App_HumanSensorWakeupInit(uint8_t distance );
int8_t App_HumanSensorSet(uint8_t distance, uint8_t *pfirtflg);
 

#ifdef __cplusplus
}
#endif
#endif
//.end of the file.
