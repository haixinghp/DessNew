#ifndef _RADAR_H_
#define _RADAR_H_

#ifdef __cplusplus
extern "C"
{
#endif



/* 标准头文件 */
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

extern void App_Radar_SetMode(uint8_t mode);//红外感应距离调节


#endif
//.end of the file.


