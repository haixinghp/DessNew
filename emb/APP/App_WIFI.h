/*********************************************************************************************************************
 * @file:        App_WIFI.h
 * @author:      lixiqun
 * @version:     V01.00
 * @date:        2021-08-09
 * @Description: wifi�ӿڹ��ܺ����ļ�
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_WIFI_H
#define  _APP_WIFI_H


/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "LockConfig.h"
/*--------------------------------------------------�궨��-----------------------------------------------------------*/

#define PHOTO_DATA_LENGTH_MAX 	1024	//����ͼƬ�ϴ�������ݳ���
#define OTA_DATA_ADDR_LENGTH_MAX 128	////OTA��������������ݳ���
#define OTA_QUERY_UPDATA_LENGTH_MAX 128	////http�ӿ�������ݳ���
/* ack  ȷ���� */
#define WIFI_ACK_OK           	0x55   	//Ӧ����Ϊ�ɹ�
#define WIFI_ACK_FAIL         	0x33   	//Ӧ����Ϊʧ��
#define WIFI_ACK_UPLOAD_PHOTO_OK  0x00  //Ӧ����Ϊ�ɹ�

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

/* ������(������) */
typedef enum
{
	WIFI_CMD_DEFAULT				= 0x00,	//��ʼ״̬
	WIFI_CMD_QUERY_CLOUD_DATA		= 0X0A, //������ȡ�ƶ�����
	WIFI_CMD_CONF_ROUTER			= 0xF6,	//����·����
	WIFI_CMD_CONF_SERVER			= 0xF7,	//���÷�����
	WIFI_CMD_UPLOAD_UNLOCK_MEG		= 0x04,	//������Ϣ�ϴ�
	WIFI_CMD_UPLOAD_ERROR_MEG		= 0x07,	//������Ϣ�ϴ�
	WIFI_CMD_UPLOAD_IMAGE_MEG		= 0x09,	//ͼƬ�ϴ�
	WIFI_CMD_SET_OTA_UPDATA			= 0x10,	//OTA����
	WIFI_CMD_QUERY_OTA_UPDATA		= 0x11,	//OTA����
	WIFI_CMD_SET_BAUDRATE			= 0xFA,	//ͨѶ����������
	WIFI_CMD_QUERY_MAC_ADDR			= 0xFB,	//��ѯWIFIģ��MAC��ַ
	WIFI_CMD_FACTORY_CHECK			= 0xFC,	//����ɨ��·����
	WIFI_CMD_QUERY_SOFT_VERSION		= 0xFD,	//��ѯWIFIģ�������汾��
	WIFI_CMD_QUERY_PASSWORD			= 0xFE,	//��ѯ��ǰģ����PSK����
}WIFI_CMD_TYPE_E;
/*--------------------------------------------------ö������---------------------------------------------------------*/

 
/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/             
typedef enum
{
	WIFI_TX_PREPARE = 0,//wifi׼������
	WIFI_POWER_ON,//wifiģ���ϵ���ʱ�ȴ��ȶ�
	WIFI_TX_START,//��ʼ����wifi����
	WIFI_TX_START_WAIT,//��ʼ����wifi������ʱ
	WIFI_TX_ING,//wifi���ݷ�����
	WIFI_TX_FAIL,//wifi���ݷ���ʧ��
	WIFI_TX_SUCCESS,//wifi���ݷ��ͳɹ�
	WIFI_TX_OVER,//wifi���ͽ���
	WIFI_POWEROFF_DELAY,//�µ���ʱ
}WIFI_TX_STATE_E;//����״̬
extern WIFI_TX_STATE_E OB_CAM_TxState;

typedef enum
{
	FINGER      = 0x01,//ָ��
	PWD,               //����
	SMARTKEY,          //����Կ��/�ֻ�
	PHONE,             //�ֻ�
	FACE,              //����
	HUAWEI_WALLET,     //��ΪǮ��
	NFC         = 0x09,//NFC
	IC          = 0x0A,//IC��
	VEIN        = 0x0B,//ָ����
	IRIS        = 0x0C,//��Ĥ
	FACE_FIGURE = 0x50,//����+ָ��
	FACE_PWD    = 0x51,//����+����
	FINGER_PWD  = 0x52,//ָ��+����
	FINGER_IC   = 0x53,//ָ��+IC��
	IRIS_FINGER = 0x54,//��Ĥ+ָ��
	IRIS_PWD    = 0x55,//��Ĥ+����
	VEIN_PWD    = 0x56,//ָ����+����
	VEIN_IC     = 0x57,//ָ����+IC��
	
	SERVER_TEST = 0xF1,//����������
	
	AUTO_LOCKUP = 0xA0,//�Զ�����
	INDOOR_MANUAL_LOCKUP = 0xA1,//�����ֶ�����
	OUTDOOR_MANUAL_LOCKUP = 0xA2,//�����ֶ�����
	TIMING_AUTO_LOCKUP = 0xA3,//��ʱ����
	
}UNLOCK_MODE_E;//���ŷ�ʽ

typedef enum
{
	LOCKPICKING = 0x01,//����
	FALSE_LOCK,//����
	FORBID_TRY,//����
	EPLOYMENT,//����
	DEFENSE,//��������
	ALARM,//����
	ELECTRONIC_LOCK_HEAD,//������ͷ
    FORGET_LOCK,//��δ��
	UNLOCK_MEG,//������Ϣ
}ALARM_MEG_E;//������Ϣ

typedef enum
{
	NONE= 0x00,//ʲô���ܶ�����
	FAMILY = 0x0f,//�����鹦��
	SOS = 0xf0,//����������
	FAMILY_SOS = 0xff,//�ȴ����鹦���ִ���������
}UNLOCK_ATTRIBUTE_E;//���Դ���������ʽ�Ƿ������ͽ�������

typedef enum
{
	VALUE19200 = 0x33,
	VALUE115200 = 0x55,
	VALUE57600 = 0xEE,
	VALUE9600 = 0x66,
}BAUDRATE_SET_E;
typedef struct
{
	uint8_t Reserve;//Ԥ����0(1byte)
	uint8_t FactoryInfo;//OEM/ODM������Ϣ(1byte)��0x00��ʩ����
	uint8_t FingerModuleInfo;//ָ��ģ�鳧����Ϣ(1byte)��0x55/0x00δ֪��0x66����΢��0x67ָ����0x68������
	uint8_t MotorUnlokCount;//������ţ���������(1byte)
	uint8_t MotorUnlokTime[2];//����ʱ��������ʱ��(2byte)����λms��
	uint8_t MotorUnlokCurrentVal[2];//���ŵ������ƽ��ֵ2byte
	uint8_t MotorUnlokAdc[4];//���ŵ�����2����ֵ4byte
}DevInfor_T;//�豸��Ϣ

typedef struct
{
	uint8_t way1;
	uint8_t id1;
	uint8_t way2;
	uint8_t id2;
}PAGEID_U;

typedef struct
{
	uint16_t DataLenth;//���ݰ�����
	
	uint8_t Ssid[33];//����·����ssid
	uint8_t Passwd[65];//����·����passwd
	uint8_t severname[50];//������������
	uint8_t portno[2];//���������˿ں�
	
	uint8_t DevType;//�豸���ͣ�1byte)
	DevInfor_T 	DevInfor;//�豸��Ϣ(18byte)
	uint8_t BatteryPow[2];//������ʾ(2byte)
	UNLOCK_MODE_E UnlockMode;//������ʽ(1byte)
	PAGEID_U PageID;//PageID(4byte)
	UNLOCK_ATTRIBUTE_E Attribute;//���ԣ�1byte��
	uint8_t UnlockWay1SuccessTime[2];//������ʽ��Ӧ��ʽһ��ʷ�ɹ�����
	uint8_t UnlockWay1FailTime[2];//������ʽ��Ӧ��ʽһ��ʷʧ�ܴ���
	uint8_t UnlockWay2SuccessTime[2];//������ʽ��Ӧ��ʽ����ʷ�ɹ�����
	uint8_t UnlockWay2FailTime[2];//������ʽ��Ӧ��ʽ����ʷʧ�ܴ���
	
	uint8_t WifiFactoryTestNum[19];//С�ι�װwifi�����·��ֻ���
	uint8_t WifiFactoryTest;
	
	ALARM_MEG_E AlarmMeg;//������Ϣ(1byte)

	uint8_t	PhotoPakageSum;//�ܰ���
	uint8_t	PhotoPackageNum;//����
	uint8_t PhotoUploadType;//ͼƬ����(1byte)��0x01��������0x02����ץ�ģ�
	uint8_t	PhotoAES[16];//��Կ��16 byte��
	uint8_t	PhotoData[PHOTO_DATA_LENGTH_MAX];//ͼƬ����
	
	uint8_t OtaUpdateDataAddr[OTA_DATA_ADDR_LENGTH_MAX];//��������
	
	uint8_t OtaQueryUpdateData[OTA_QUERY_UPDATA_LENGTH_MAX];//http�ӿ�
	
	BAUDRATE_SET_E BaudrateSet;//ͨѶ����������
	
	WIFI_TX_STATE_E State;//����״̬
	WIFI_TX_STATE_E ConfigState;//����״̬
	WIFI_TX_STATE_E PhotoState;//����״̬
	uint8_t Cmd;//����ָ��
	int8_t Result;//�������
}WifiLockUploadMeg_T;//����ʵʱ�ϴ����ݰ���֡
extern WifiLockUploadMeg_T WifiLockMeg;


typedef struct
{
	uint8_t	PhotoPakageSum;//�ܰ���
	uint8_t	PhotoPackageNum;//����
	uint8_t AckResult;//ͨ��Ӧ����
	uint8_t WifiMac[6];//WIFI������MAC��ַ
	uint8_t SignalIntensity;//�ź�ǿ��
	uint8_t SoftwareVersion[5];//WIFI�������汾��
	uint8_t WifiPassword[20];//Psk����
}WifiRxMeg_T;//�������ݰ�����
extern WifiRxMeg_T WifiRxData;

typedef struct
{
	uint8_t data[128];
	uint16_t length;
}WifiTx;
extern WifiTx WifiTxTemp;

/*--------------------------------------------------��������---------------------------------------------------------*/
extern uint8_t WIFI_UploadDataGetLength(void);
extern uint8_t WIFI_UploadDataFill(const uint8_t *buf, uint16_t length);
extern uint8_t WIFI_UploadData_Get(uint8_t *buf, uint16_t *length);
void App_WIFI_Init(void);
void App_WIFI_WakeupInit(void);
void App_Wifi_Tim10Ms(void);
uint8_t App_WIFI_CommomTx(uint8_t cmd);
int8_t App_WIFI_ConfigThread(uint8_t cmd, uint8_t *first_flag);//����ָ��ͺ���
void App_WIFI_PhotoUpload(void);
void App_WIFI_ScanProcess(void);
int8_t APP_WIFI_TxState(void);
void APP_Wifi_Sleep(void);
void App_WIFI_PhotoUartInit(void);
void App_WIFI_UartPowerOn(void);
int8_t App_WIFI_ACK_Thread(uint8_t cmd, uint8_t *first_flag);
#endif
