#ifndef _RADAR_H_
#define _RADAR_H_

#ifdef __cplusplus
extern "C"
{
#endif



/* ��׼ͷ�ļ� */
#include "stdint.h"

typedef enum
{
    RADAR_OFF,         
    RADAR_NEAR,         
    RADAR_MEDIUM,        
	RADAR_FAR
}RADAR_MODE_E;


typedef struct
{
	RADAR_MODE_E Mode;
}RADAR_CONTROL_MODE;
RADAR_CONTROL_MODE RadarControl;

extern void App_Radar_SetMode(uint8_t mode);//�����Ӧ�������


#endif
//.end of the file.


