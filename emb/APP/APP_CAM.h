#ifndef _APP_CAM_H_
#define _APP_CAM_H_

#ifdef __cplusplus
 extern "C" {
#endif


/* 标准头文件 */
#include "stdint.h"
#include "stdbool.h"

/* 外部头文件 */
#include "System.h" 


//协议命令
#define  CAM_CMD_HEAD         0x7B

#define  CAM_CMD_NET_SLEEP_STATE  0X80
#define  CAM_CMD_SLEEP_STATE      0X82
#define  CAM_CMD_READY            0X90
#define  CAM_CMD_READY_ACK        0X91
#define  CAM_CMD_PIR              0XC7
#define  CAM_CMD_GET_WIFI_STATE   0XD6
#define  CAM_CMD_WAKE_REASON      0XDC
#define  CAM_CMD_DATA_SEND        0XD9
#define  CAM_CMD_LINKKEY_SEND     0XD5
#define  CAM_CMD_SN_GET           0XDA
#define  CAM_CMD_SSID_SEND        0XD8
#define  CAM_CMD_FAST_SLEEP_SET   0XDE
#define  CAM_CMD_ALARM_SEND       0XCA
#define  CAM_CMD_SINGLE_MODE_SET  0XA3
#define  CAM_CMD_GET_IP           0XDB

#define  CAM_CMD_GOOUT_TIP        0XE0
#define  CAM_CMD_OPENDOOR_TIP     0XE1
#define  CAM_CMD_SET_FOV          0XE3

//自定义命令
#define  CAM_CMD_BELL             0XFF
#define  CAM_CMD_ALARM            0XFE
#define  CAM_CMD_CLEAR            0XFD

#define CAM_QUEUE_SIZE           6
#define CAM_QUEUE_BUF_MAX        256
#define CAM_QUEUE_OK             1
#define CAM_QUEUE_PAUSE          0
#define CAM_QUEUE_EMPTY          2
#define CAM_QUEUE_FULL           3 

typedef enum
{
    CAM_WAKE_UP = 0,   //唤醒模组
	CAM_READY,     //模组ready
	CAM_ACK_READY,
	CAM_COMMAND,   //送命令
	CAM_RESPONSE,  //模组应答
	CAM_SUCCESSFUL,//下发成功
	CAM_FAIL
} CAM_STATE_E;

typedef struct
{
	uint8_t RxDataBuf[CAM_QUEUE_BUF_MAX];  //数据接收BUF
	uint8_t TxDataBuf[CAM_QUEUE_BUF_MAX];  //数据BUF
	uint8_t Txlen;
	uint8_t TxCdm;        //发送帧数据
	uint8_t RxPos;       //当前接受位置
	
	uint16_t Systick;
	CAM_STATE_E  State;        //状态
}CAM_CONTROL; 

typedef struct
{
	uint8_t HeadPtr;	//头指针
	uint8_t TailPtr;	//尾指针
	CAM_CONTROL astCamCtrl[CAM_QUEUE_SIZE];//wifi上行存储数据
}CamQueue_T;


void App_CAM_Tim10Ms (void);
int8_t CAM_SendCommandStart(uint8_t cmd , uint8_t *pdata ,uint8_t len);
int8_t CAM_ClearCameraData(void);
bool CAM_GetQueueClearState(void);
CAM_STATE_E CAM_GetServerState(uint8_t _i8TaskId);
void CAM_ServerScan(void);
uint8_t CAM_GetCameraData(uint8_t CMD, uint8_t *pdata , uint8_t taskId);
int8_t CAM_AppConfigThread(uint8_t cmd, uint8_t *first_flag);
void CAM_SetCapture(uint8_t Type);
uint8_t CAM_GetPushAlarmState(void);

bool CAM_AnalysisWifiSignalStrength(uint8_t *pdata , uint8_t taskId);

int8_t CAM_GoOutGiveNotice(void);
int8_t CAM_OpenDoorGiveNotice(LOCK_EVENT_LOG _em, uint16_t _id, LOCK_EVENT_LOG _em2, uint16_t _id2);
int8_t CAM_SetFovParam(uint8_t _u8Param);
int8_t CAM_GetWifiSignalStrength(void);


#ifdef __cplusplus
}
#endif

#endif
//.end of the file.
