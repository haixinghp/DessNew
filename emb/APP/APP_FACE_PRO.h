#ifndef _APP_FACE_PRO_H_
#define _APP_FACE_PRO_H_

#ifdef __cplusplus
 extern "C" {
#endif
/* 内部头文件 */

/* 标准头文件 */
#include "stdint.h"

typedef struct
{
	uint8_t Result;
	uint8_t Weather;
	uint8_t Festival;
	uint8_t Minimun_Temperature;
	uint8_t Maximun_Temperature;
	uint8_t Date;
}FaceProCloudData_T;
extern FaceProCloudData_T FaceProCloudData;//云端数据



/* 天气枚举*/
typedef enum
{
    EM_WEATHER_SUNNY_DAY = 1,       // 晴天 
    EM_WEATHER_CLOUDY_DAY,          // 多云 
    EM_WEATHER_OVERCAST_DAY,        // 阴天 
    EM_WEATHER_RAINY_DAY,           // 雨天 
    EM_WEATHER_SNOWY_DAY,           // 雪天 
    
    /* 总数量 */
    EM_WEATHER_ALL,                 //最大值
}WEATHER_TYPE_E; 


/* 节日枚举*/
typedef enum
{
    EM_FESTIVAL_LUNAR_1230 = 1,     // 除夕
    EM_FESTIVAL_LUNAR_0115,         // 元宵
    EM_FESTIVAL_LUNAR_0505,         // 端午
    EM_FESTIVAL_LUNAR_0707,         // 七夕
    EM_FESTIVAL_LUNAR_0815,         // 端午
    EM_FESTIVAL_HOLIDAY_0101,       // 元旦
    EM_FESTIVAL_HOLIDAY_0214,       // 情人节
    EM_FESTIVAL_HOLIDAY_0501,       // 劳动节
    EM_FESTIVAL_HOLIDAY_0601,       // 儿童节
    EM_FESTIVAL_HOLIDAY_1001,       // 国庆节
    EM_FESTIVAL_HOLIDAY_1225,       // 圣诞节
    
    /* 总数量 */
    EM_FESTIVAL_ALL,                 //最大值
}FESTIVAL_TYPE_E; 


void APP_FACE_PRO_Tim1s (void);
void FaceProDeleteFile (void);
void FaceProSetLightSensor (void);
int8_t FaceProSetSSID (uint8_t *ssid ,uint8_t len);
uint8_t FaceProCallBell(uint8_t Bell);
uint8_t FaceProQueryCloudData(uint8_t *data ,uint8_t len);
uint8_t FaceProAlarm(uint8_t type ,uint8_t *data ,uint8_t len);
int8_t FaceProScanProcess(void);
int8_t FaceProOta(void);
uint8_t FaceProNetworking(void);
int8_t FaceProGetWifiIntensity(uint8_t *intensity);
void FaceProWifiPush(void);
#ifdef __cplusplus
}
#endif

#endif
//.end of the file.
