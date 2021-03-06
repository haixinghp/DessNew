/*********************************************************************************************************************
 * @file:        App_WIFI.h
 * @author:      lixiqun
 * @version:     V01.00
 * @date:        2021-08-09
 * @Description: wifi接口功能函数文件
 * @ChangeList:  01. 初版
**********************************************************************************************************************/
  
#ifndef  _APP_WIFI_H
#define  _APP_WIFI_H


/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "LockConfig.h"
/*--------------------------------------------------宏定义-----------------------------------------------------------*/

#define PHOTO_DATA_LENGTH_MAX 	1024	//单包图片上传最大数据长度
#define OTA_DATA_ADDR_LENGTH_MAX 128	////OTA下载链接最大数据长度
#define OTA_QUERY_UPDATA_LENGTH_MAX 128	////http接口最大数据长度
/* ack  确认码 */
#define WIFI_ACK_OK           	0x55   	//应答结果为成功
#define WIFI_ACK_FAIL         	0x33   	//应答结果为失败
#define WIFI_ACK_UPLOAD_PHOTO_OK  0x00  //应答结果为成功

#define WIFI_CONFIG_FAIL	-1
#define WIFI_CONFIG_ING		0
#define WIFI_CONFIG_SUCESS	1

#define WIFI_DATA_LENGTH_MAX	128
#define WIFI_UPLOAD_DATA_LENGTH_MAX 128
#define QUEUE_SIZE 6
#define QUEUE_OK           1
#define QUEUE_PAUSE        0
#define QUEUE_EMPTY        2
#define QUEUE_FULL         3 

/* 包数据(命令字) */
typedef enum
{
	WIFI_CMD_DEFAULT				= 0x00,	//初始状态
	WIFI_CMD_QUERY_CLOUD_DATA		= 0X0A, //主动获取云端数据
	WIFI_CMD_CONF_ROUTER			= 0xF6,	//设置路由器
	WIFI_CMD_CONF_SERVER			= 0xF7,	//设置服务器
	WIFI_CMD_UPLOAD_UNLOCK_MEG		= 0x04,	//开锁信息上传
	WIFI_CMD_UPLOAD_ERROR_MEG		= 0x07,	//故障信息上传
	WIFI_CMD_UPLOAD_IMAGE_MEG		= 0x09,	//图片上传
	WIFI_CMD_SET_OTA_UPDATA			= 0x10,	//OTA更新
	WIFI_CMD_QUERY_OTA_UPDATA		= 0x11,	//OTA更新
	WIFI_CMD_SET_BAUDRATE			= 0xFA,	//通讯波特率设置
	WIFI_CMD_QUERY_MAC_ADDR			= 0xFB,	//查询WIFI模块MAC地址
	WIFI_CMD_FACTORY_CHECK			= 0xFC,	//产测扫描路由器
	WIFI_CMD_QUERY_SOFT_VERSION		= 0xFD,	//查询WIFI模块软件版本号
	WIFI_CMD_QUERY_PASSWORD			= 0xFE,	//查询当前模块内PSK密码
}WIFI_CMD_TYPE_E;
/*--------------------------------------------------枚举声明---------------------------------------------------------*/

 
/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
typedef enum
{
	WIFI_TX_PREPARE = 0,//wifi准备推送
	WIFI_POWER_ON,//wifi模块上电延时等待稳定
	WIFI_TX_START,//开始发送wifi数据
	WIFI_TX_START_WAIT,//开始发送wifi数据延时
	WIFI_TX_ING,//wifi数据发送中
	WIFI_TX_FAIL,//wifi数据发送失败
	WIFI_TX_SUCCESS,//wifi数据发送成功
	WIFI_TX_OVER,//wifi推送结束
	WIFI_POWEROFF_DELAY,//下电延时
}WIFI_TX_STATE_E;//发送状态
extern WIFI_TX_STATE_E OB_CAM_TxState;

typedef enum
{
	FINGER      = 0x01,//指纹
	PWD,               //密码
	SMARTKEY,          //智能钥匙/手环
	PHONE,             //手机
	FACE,              //人脸
	HUAWEI_WALLET,     //华为钱包
	NFC         = 0x09,//NFC
	IC          = 0x0A,//IC卡
	VEIN        = 0x0B,//指静脉
	IRIS        = 0x0C,//虹膜
	FACE_FIGURE = 0x50,//人脸+指纹
	FACE_PWD    = 0x51,//人脸+密码
	FINGER_PWD  = 0x52,//指纹+密码
	FINGER_IC   = 0x53,//指纹+IC卡
	IRIS_FINGER = 0x54,//虹膜+指纹
	IRIS_PWD    = 0x55,//虹膜+密码
	VEIN_PWD    = 0x56,//指静脉+密码
	VEIN_IC     = 0x57,//指静脉+IC卡
	
	SERVER_TEST = 0xF1,//服务器测试
	
	AUTO_LOCKUP = 0xA0,//自动上锁
	INDOOR_MANUAL_LOCKUP = 0xA1,//门内手动上锁
	OUTDOOR_MANUAL_LOCKUP = 0xA2,//门外手动上锁
	TIMING_AUTO_LOCKUP = 0xA3,//定时上锁
	
}UNLOCK_MODE_E;//开门方式

typedef enum
{
	LOCKPICKING = 0x01,//撬锁
	FALSE_LOCK,//假锁
	FORBID_TRY,//禁试
	EPLOYMENT,//布防
	DEFENSE,//主动防御
	ALARM,//门铃
	ELECTRONIC_LOCK_HEAD,//电子锁头
    FORGET_LOCK,//门未锁
	UNLOCK_MEG,//开锁信息
}ALARM_MEG_E;//报警信息

typedef enum
{
	NONE= 0x00,//什么功能都不带
	FAMILY = 0x0f,//带亲情功能
	SOS = 0xf0,//带紧急功能
	FAMILY_SOS = 0xff,//既带亲情功能又带紧急功能
}UNLOCK_ATTRIBUTE_E;//属性代表开锁方式是否带亲情和紧急功能

typedef enum
{
	VALUE19200 = 0x33,
	VALUE115200 = 0x55,
	VALUE57600 = 0xEE,
	VALUE9600 = 0x66,
}BAUDRATE_SET_E;
typedef struct
{
	uint8_t Reserve;//预留补0(1byte)
	uint8_t FactoryInfo;//OEM/ODM厂家信息(1byte)（0x00德施曼）
	uint8_t FingerModuleInfo;//指纹模块厂家信息(1byte)（0x55/0x00未知，0x66迈瑞微，0x67指安，0x68集创）
	uint8_t MotorUnlokCount;//电机开门，工作次数(1byte)
	uint8_t MotorUnlokTime[2];//开门时间电机工作时间(2byte)（单位ms）
	uint8_t MotorUnlokCurrentVal[2];//开门电机电流平均值2byte
	uint8_t MotorUnlokAdc[4];//开门电机最后2采样值4byte
}DevInfor_T;//设备信息

typedef struct
{
	uint8_t way1;
	uint8_t id1;
	uint8_t way2;
	uint8_t id2;
}PAGEID_U;

typedef struct
{
	uint16_t DataLenth;//数据包长度
	
	uint8_t Ssid[33];//设置路由器ssid
	uint8_t Passwd[65];//设置路由器passwd
	uint8_t severname[50];//设置域名参数
	uint8_t portno[2];//设置域名端口号
	
	uint8_t DevType;//设备类型（1byte)
	DevInfor_T 	DevInfor;//设备信息(18byte)
	uint8_t BatteryPow[2];//电量提示(2byte)
	UNLOCK_MODE_E UnlockMode;//开锁方式(1byte)
	PAGEID_U PageID;//PageID(4byte)
	UNLOCK_ATTRIBUTE_E Attribute;//属性（1byte）
	uint8_t UnlockWay1SuccessTime[2];//开锁方式对应方式一历史成功次数
	uint8_t UnlockWay1FailTime[2];//开锁方式对应方式一历史失败次数
	uint8_t UnlockWay2SuccessTime[2];//开锁方式对应方式二历史成功次数
	uint8_t UnlockWay2FailTime[2];//开锁方式对应方式二历史失败次数
	
	uint8_t WifiFactoryTestNum[19];//小滴工装wifi产测下发手机号
	uint8_t WifiFactoryTest;
	
	ALARM_MEG_E AlarmMeg;//报警信息(1byte)

	uint8_t	PhotoPakageSum;//总包数
	uint8_t	PhotoPackageNum;//包序
	uint8_t PhotoUploadType;//图片类型(1byte)（0x01代表门铃0x02代表抓拍）
	uint8_t	PhotoAES[16];//秘钥（16 byte）
	uint8_t	PhotoData[PHOTO_DATA_LENGTH_MAX];//图片数据
	
	uint8_t OtaUpdateDataAddr[OTA_DATA_ADDR_LENGTH_MAX];//下载链接
	
	uint8_t OtaQueryUpdateData[OTA_QUERY_UPDATA_LENGTH_MAX];//http接口
	
	BAUDRATE_SET_E BaudrateSet;//通讯波特率设置
	
	WIFI_TX_STATE_E State;//发送状态
	WIFI_TX_STATE_E ConfigState;//发送状态
	WIFI_TX_STATE_E PhotoState;//发送状态
	uint8_t Cmd;//发送指令
	int8_t Result;//配网结果
}WifiLockUploadMeg_T;//开锁实时上传数据包组帧
extern WifiLockUploadMeg_T WifiLockMeg;


typedef struct
{
	uint8_t	PhotoPakageSum;//总包数
	uint8_t	PhotoPackageNum;//包序
	uint8_t AckResult;//通信应答结果
	uint8_t WifiMac[6];//WIFI的物理MAC地址
	uint8_t SignalIntensity;//信号强度
	uint8_t SoftwareVersion[5];//WIFI的软件版本号
	uint8_t WifiPassword[20];//Psk密码
}WifiRxMeg_T;//接收数据包内容
extern WifiRxMeg_T WifiRxData;

typedef struct
{
	uint8_t data[128];
	uint16_t length;
}WifiTx;
extern WifiTx WifiTxTemp;

/*--------------------------------------------------函数声明---------------------------------------------------------*/
extern uint8_t WIFI_UploadDataGetLength(void);
extern uint8_t WIFI_UploadDataFill(const uint8_t *buf, uint16_t length);
extern uint8_t WIFI_UploadData_Get(uint8_t *buf, uint16_t *length);
void App_WIFI_Init(void);
void App_WIFI_WakeupInit(void);
void App_Wifi_Tim10Ms(void);
uint8_t App_WIFI_CommomTx(uint8_t cmd);
int8_t App_WIFI_ConfigThread(uint8_t cmd, uint8_t *first_flag);//配网指令发送函数
void App_WIFI_PhotoUpload(void);
void App_WIFI_ScanProcess(void);
int8_t APP_WIFI_TxState(void);
void APP_Wifi_Sleep(void);
void App_WIFI_PhotoUartInit(void);
void App_WIFI_UartPowerOn(void);
int8_t App_WIFI_ACK_Thread(uint8_t cmd, uint8_t *first_flag);
#endif

