#ifndef  _SYSTEM_H
#define  _SYSTEM_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <LockConfig.h>

/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define FUNCTION_ENABLE        1      //����ʹ��   
#define FUNCTION_DISABLE       0      //����ʧ��   

#define ADMIN_NONE_REGISTERED  0xA5B7 //��ע��.
#define ADMIN_LOCAL_REGISTERED 0x0001 //���ع���Ա��ע��
#define ADMIN_APP_REGISTERED   0x0002 //APP����Ա��ע��

#define MEM_FACT_MEM_FIG				         'W'  //��Ч��־
#define MEM_USER_MASTER                          'M'
#define MEM_USER_GUEST                           'G'
#define MEM_USER_ALL                             'A'

#define LOCK_BODY_212          0x55
#define LOCK_BODY_218          0x66
#define LOCK_BODY_216          0x77

#define SYSTEM_AESKEY_LEN       16

//�洢�ṹ����
/*------------------------------------------------------------------------------------------------
#define MEM_FACTMEM_FACTORY_FIG                  0  //����������ʼ��־  G
#define MEM_FACTMEM_LOCKMODE                     1  //����ģʽ��ָ��/��������/����Կ�׵ȣ�
#define MEM_FACTMEM_FINGER_APP_NUM               2  //appָ������
#define MEM_FACTMEM_FINGER_ADMIN_NUM		     3  //���ع���Աָ������
#define MEM_FACTMEM_FINGER_GUEST_NUM			 4  //������ָͨ������
#define MEM_FACTMEM_FACE_APP_NUM                 5  //app��������
#define MEM_FACTMEM_FACE_ADMIN_NUM		         6  //���ع���Ա��������
#define MEM_FACTMEM_FACE_GUEST_NUM			     7  //������ͨ��������
#define MEM_FACTMEM_CARD_APP_NUM                 8  //app������
#define MEM_FACTMEM_CARD_ADMIN_NUM		         9  //���ع���Ա������
#define MEM_FACTMEM_CARD_GUEST_NUM			     10 //������ͨ������
#define MEM_FACTMEM_PWD_APP_NUM                  11 //app��������
#define MEM_FACTMEM_PWD_ADMIN_NUM		         12 //���ع���Ա��������
#define MEM_FACTMEM_PWD_GUEST_NUM			     13 //������ͨ��������
#define MEM_FACTMEM_SMARTKEY_NUM                 14 //ȫ������Կ������H ��20����
#define MEM_FACTMEM_WIFI_MAIN_SW                 15 //WIFI������
#define MEM_FACTMEM_WIFI_LOGUP_SW	             16 //WIFI�����ź��ϴ�����
#define MEM_FACTMEM_ADMIN_REGISTER_H             17 //�Ƿ�ע����H 
#define MEM_FACTMEM_ADMIN_REGISTER_L             18 //�Ƿ�ע����L
#define MEM_FACTMEM_LOCKLOG_LINK_H               19 //�¼���¼�ܴ���
#define MEM_FACTMEM_LOCKLOG_LINK_L               20 //�¼���¼�ܴ���
#define MEM_FACTMEM_LOCKLOG_SN_H                 21 //�¼���¼��ǰ���к�H
#define MEM_FACTMEM_LOCKLOG_SN_L                 22 //�¼���¼��ǰ���к�L
#define MEM_FACTMEM_VOICE                        23 //��������
#define MEM_FACTMEM_KEY_DEF                      24 //һ������
#define MEM_FACTMEM_IR_DEF                       25 //�ӽ���Ӧ����
#define MEM_FACTMEM_WIFI_SINGLE                  26 //���嵥˫��
#define MEM_FACTMEM_LOCKTIME	                 27 //�Զ�����ʱ��
#define MEM_FACTMEM_PROTECT_LOCK	             29 //���ŷ�ֹ��
#define MEM_FACTMEM_FINGER_FAC                   30 //ָ�Ƴ��ң�ͨ�������жϼ���
#define MEM_FACTMEM_FINGER_ASEKEY                31 //ָ��ͷAES����Կ16�ֽ�
//+16
#define MEM_FACTMEM_EEPROM_MODEL                 47 //�����ͺ� 8�ֽ�
//+8
#define MEM_FACTMEM_FACE_FAC                     55 //�������̺�
#define MEM_FACTMEM_FACE_ASEKEY                  56 //ָ��ͷAES����Կ16�ֽ�
//+16
#define MEM_FACTMEM_DONE_FIG                     65 //������־G


// ���µ�ַ���������
#define MEM_FACTMEM_MOTOR_DIRECTION              256 // ���ŷ���
#define MEM_FACTMEM_MOTOR_TORQUE				 257 //���Ť��
#define MEM_FACTMEM_MOTOR_TYPE                   258 //��������
--------------------------------------------------------------------------------------------------*/

//=============================��ַ����̻�=====================================
#define MEM_FACT_START				         0     //ϵͳ��Ϣ��ʼ�������
#define MEM_FACT_SIZE				         256   //ϵͳ��Ϣ�ܳ���

#define MEM_FIX_FACT_START				     0x100     //ϵͳ��Ϣ��ʼ���������
#define MEM_FIX_FACT_SIZE				     256   //ϵͳ��Ϣ�ܳ���

#define MEM_PHONE_START                      0x200    //�ֻ�����ʼ
#define MEM_PHONE_LEN			             13        //����6λ
#define MEM_PHONE_NUM	                     1        //һ��

#define MEM_BOARDPWD_START                   0x300    //������ʼ
#define MEM_BOARDPWD_PWDLEN			         6        //����6λ
#define MEM_BOARDPWD_ALL                     1        //һ������

#define MEM_BOARDSOSPWD_START                0x340    //SOS������ʼ
#define MEM_BOARDSOSPWD_PWDLEN			     6        //����6λ
#define MEM_BOARDSOSPWD_ALL                  1        //һ������

#define MEM_BOARDTEMPPWD_START               0x400    //��ʱ������ʼ
#define MEM_BOARDTEMPPWD_PWDLEN			     6        //����6λ
#define MEM_BOARDTEMPPWD_ALL                 10       //һ������

#define MSG_FINGER_REG_START		         0x500	  //ָ��ע����ʼ��ַ
#define MSG_FINGER_REG_ONE_SIZE			     38		  //��ָ�������ֽ�����
#define MSG_FINGER_ONE_SIZE			         64		  //��ָ��ʵ�ʿռ�
#define MSG_FINGER_USER_NUM			         60       //ʹ������
#define MSG_FINGER_NUM_RESERVED			 ((M_FINGER_MAX_TOTAL_NUM >= 100)?100:M_FINGER_MAX_TOTAL_NUM) 	  //100��ָ�ƻ�ָ����Ԥ��
#define MSG_FINGER_ADMIN_NUM                ((M_FINGER_MAX_ADMIN_NUM >= 100)?(MSG_FINGER_NUM_RESERVED - 1):M_FINGER_MAX_ADMIN_NUM)  // ����Ա�û�������С������
#define MSG_FINGER_COMMON_NUM               (MSG_FINGER_NUM_RESERVED - MSG_FINGER_ADMIN_NUM)    // ��ͨ �û�
#define MSG_FINGER_ADMIN_LOCAL_NUM          5       //����GUI����֧�ֵĹ���Ա����
#define MSG_FINGER_COMMON_LOCAL_GUINUM      15      //����GUI����֧�ֵ���ͨ�û�����

#define MSG_FACE_REG_START		             (MSG_FINGER_REG_START+	MSG_FINGER_ONE_SIZE* MSG_FINGER_NUM_RESERVED)    //����ע����ʼ��ַ0x1D00
#define MSG_FACE_REG_ONE_SIZE			     38		  //�����������ֽ�����
#define MSG_FACE_ONE_SIZE			         64		  //������ʵ�ʿռ�
#define MSG_FACE_MASTER_NUM			         5        //��������
#define MSG_FACE_GUEST_NUM			         15       //��ͨ����
#define MSG_FACE_USER_NUM			         20       //ʹ������
#define MSG_FACE_NUM_RESERVED			     100      //100���������ĤԤ��

#define MSG_CPU_CARD_REG_START		         (MSG_FACE_REG_START+	MSG_FACE_ONE_SIZE* MSG_FACE_NUM_RESERVED)       //��ע����ʼ��ַ0x3600
#define MSG_CPU_CARD_REG_ONE_SIZE			 58		  //���������ֽ�����
#define MSG_CPU_CARD_ONE_SIZE			     64		  //����ʵ�ʿռ�
#define MSG_CPU_CARD_USER_NUM			     30       //ʹ������
#define MSG_CARD_NUM			             100      //100�鿨Ԥ��

#define MSG_SMARTKEY_REG_START		         (MSG_CPU_CARD_REG_START+	MSG_CPU_CARD_ONE_SIZE* MSG_CARD_NUM)       //��ע����ʼ��ַ0x4F00
#define MSG_SMARTKEY_REG_ONE_SIZE			 14		  //��Կ�������ֽ�����
#define MSG_SMARTKEY_ONE_SIZE			     64		  //��Կ��ʵ�ʿռ�
#define MSG_SMARTKEY_NUM			         20      //20��Կ��Ԥ��

#define MSG_LOG_RECORD_START                 (MSG_SMARTKEY_REG_START+	MSG_SMARTKEY_ONE_SIZE* MSG_SMARTKEY_NUM) //�¼���¼Ӳ��չ̻���ַ0x5400   21KB��ʼ
#define MSG_LOG_RECORD_REG_ONE_SIZE			 14		  //���¼���¼�����ֽ�����
#define MSG_LOG_RECORD_ONE_SIZE			     16		  //���¼���¼ʵ�ʿռ�
#define MSG_LOG_RECORD_NUM			         600      //600�¼���¼����Ԥ��

//=============================��ַ����̻�=====================================


//ϵͳ�����ṹ�壬����4�ֽڶ���
typedef struct
{
    uint8_t SysFactStartFig;         //0   //����������ʼ��־  G
	uint8_t SysLockMode;             //1   //����ģʽ��ָ��/��������/����Կ�׵ȣ�
    uint8_t SysFingerAllNum;         //2   //ָ������.
    uint8_t SysFingerAdminNum;       //3   //���ع���Աָ������
	
    uint8_t SysFingerGuestNum;       //4   //������ָͨ������
	uint8_t SysFaceAllNum;           //5   //����������
	uint8_t SysFaceAdminNum;	     //6   //���ع���Ա��������
	uint8_t SysFaceGuestNum;		 //7   //������ͨ��������
	
	uint8_t SysCardAllNum;           //8   //��������
	uint8_t SysCardAdminNum	;	     //9   //���ع���Ա������
	uint8_t SysCardGuestNum	;		 //10  //������ͨ������
	uint8_t SysPwdAllNum;            //11  //����������
	
	uint8_t SysPwdAdminNum;		     //12  //���ع���Ա��������
	uint8_t SysPwdGuestNum;			 //13  //������ͨ��������
	uint8_t SysSmartKeyNum;          //14  //ȫ������Կ������h ��20����
	uint8_t SysWifiMainSw;           //15  //wifi������
	
	uint16_t SystemAdminRegister;    //16  //�Ƿ�ע����h
	uint8_t DoorUnlockWarmSw;        //18  //��δ�ر�������  0= �رձ��� 1= ��������
	uint8_t FaceCheckEnable;         //19  //������֤����  0x55= ����  0x66= �ر�

	uint8_t LockBodyMode;            //20  //��������  0x55= 218����  0x66= 212���� 
	uint8_t FingerFlag;              //21 ָ��ͷ���
	uint8_t SysWifiLogSw;	         //22 //wifi�����ź��ϴ�����
	uint8_t SysVoice;                 //23  //��������
	
	uint8_t SysKeyDef;                //24   //һ������
	uint8_t SysHumanIrDef;            //25   //��������(����ʱ��)   0:�ر�  (1-255��)��   
	uint8_t SysWifiSingle;            //26   //���嵥˫��     0:˫�� 1:����   
	uint8_t SysAutoLockTime;	      //27   //�Զ�����ʱ��
	
	uint8_t Sysprotect_lock;	      //28   //���ŷ�ֹ��,ȷ�ϼ�
	uint8_t SysFingerFac;             //29   //ָ�Ƴ��ң�ͨ�������жϼ���
	uint8_t SysFaceFac;               //30   //�������ң�ͨ�������жϼ���
	uint8_t SysDrawNear;              //31   //�ӽ���Ӧ����
	
	uint8_t SysFingerAsekey[SYSTEM_AESKEY_LEN];      //32   //ָ��ͷaes����Կ16�ֽ�
	uint8_t SysFaceAsekey[SYSTEM_AESKEY_LEN];        //48   //����AES����Կ16�ֽ�
	
	uint8_t SysCompoundOpen;           //64 ��Ͽ���(˫����֤����)
	uint8_t SysFactDoneFig;            //65 �̶����
	uint8_t SystemVersion;             //66 �汾��¼
	uint8_t FingerProtocalVersion;     //67 ָ��Э��汾
	
	uint8_t CheckErrAllCnt;            //68 ��֤ʧ���ܼƴ�
	uint8_t CheckErrPwdCnt;            //69 ������֤ʧ�ܼƴ�
	uint8_t LedRGBMode;                //��Χ��ģʽ����ӦAPP_LED.h��LED_RGB_MODE_E
	uint8_t reserved71;                //71 Ԥ��
	
	uint32_t TryForbitUtc ;            //72-75 ������Է���ʱ��
 
	uint16_t FaceOrIrisUnlockSuccessCnt;		//76-77���������ɹ�����
	uint16_t FaceOrIrisUnlockFailCnt;			//78-79��������ʧ�ܴ���
	
	uint16_t FingerOrVeinUnlockSuccessCnt;	//80-81ָ�ƿ����ɹ�����
	uint16_t FingerOrVeinUnlockFailCnt;		//82-83ָ�ƿ���ʧ�ܴ���
	
	uint8_t reserved84;                //84 Ԥ��
	uint8_t reserved85;                //85 Ԥ��
	uint8_t reserved86;                //86 Ԥ��
	uint8_t reserved87;                //87 Ԥ��
	
} SYSTEM_SETTING;

//===============================��������==================================

#define DOUBLE_CHECK_SW_ON      0x55     //��Ͽ��ŷ�ʽ����
#define DOUBLE_CHECK_SW_OFF     0x05     //��Ͽ��ŷ�ʽ�ر�

//���ŷ�ʽ��ͬ����Э��
typedef enum 
{
    LOCK_MODE_PSW = 0x01,//���뿪�ŷ�ʽ.	
	LOCK_MODE_FINGER = 0x02,//ָ�ƿ��ŷ�ʽ.
	LOCK_MODE_BLE = 0x04,//�������ŷ�ʽ.
	LOCK_MODE_FACE = 0x08,//�������ŷ�ʽ.
	LOCK_MODE_CARD = 0x10,//�����ŷ�ʽ
	LOCK_MODE_VEIN = 0x20,//ָ����
	LOCK_MODE_KEY = 0x40, //����Կ�׿���
	LOCK_MODE_IRIS = 0x80, //��Ĥ
}LOCK_MODE_ENUM;

//���ŷ���ṹ��
typedef enum 
{
	RIGHT_HAND_DOOR=0x55,
	LEFT_HAND_DOOR=0x01
}MOTOR_DIRECTION; 

//���Ť��
typedef enum 
{
	LOW_TORQUE=0x01,
	HIGH_TORQUE=0x02
}MOTOR_TORQUE; 

//��������
typedef enum 
{
    OFF_VOICE_VOL,       //�ر�
	LOW_VOICE_VOL,     	 //������
	MEDIUM_VOICE_VOL, 	 //������
    HIGH_VOICE_VOL,		 //������
	
}VOL_SET_E; 

//�ӽ���Ӧ����
typedef enum 
{
    E_SENSE_HIGH = SENSE_HIGH_GRADE,	//Զ����
	E_SENSE_LOW = SENSE_LOW_GRADE,     	//������
    E_SENSE_OFF = SENSE_OFF_GRADE,      //�ر�
	
}NEAR_SENSE_GRADE_T; 

 
typedef struct
{
	//��ʼ��ַ256
	uint8_t SysFixStartFig;                         //����������ʼ��־  G
	uint8_t MotorDirection;						    //���ŷ���      
	uint8_t MotorTorque;                            //���Ť��
	uint8_t LockType;                               //��������	
	uint8_t LockNameType[8];                        //���ͺ�
	uint16_t SysLockLogAll;                         //�¼���¼�ܴ���
	uint16_t SysLockLogSn;                          //�¼���¼��ǰλ��
	uint8_t SysClearCase[14];                       //Ӳ����¼�	
	
} SYSTEM_FIX_SETTING;


//�¼���¼ö��
typedef enum 
{
	NOTHING_CASE, 				//�����޲���
	BAC_OPEN_IN_DOOR,   		//���ڰ��ֿ�
	BAC_CLOSE_IN_DOOR,  		//���ڰ��ֹ�
	FALSE_LOCK_ALARM,  			//��������
	CLOSE_OUT_DOOR,      		//��������
	AUTO_CLOSE_DOOR,     		//�Զ�����
	KEY_OPEN_IN_DOOR,      		//���ڰ�����
	KEY_CLOSE_IN_DOOR,     		//���ڰ�����
	EMPTY_LOCK,        			//�������
	TEMP_PASSWORD_OPEN,      	//��ʱ���뿪��
	NFC_OPEN,         			//NFC����
	
	DELETE_PASSWORD,       		//����ɾ��
	ADD_PASSWORD,            	//��������
	PASSWORD_OPEN,            	//���뿪��
    PWD_ADMIN_CHECK,            //�������˵� 
	
	DELETE_SOS_PASSWORD,       	//��������ɾ��
	ADD_SOS_PASSWORD,           //���ӱ�������
	SOS_PASSWORD_OPEN,          //�������뿪��
	
	ADD_FACE,            		//��������
	DELETE_FACE,          		//ɾ������
	FACE_OPEN,           		//��������
	FACE_ADMIN_CHECK,           //��������˵� 
	
	TRY_OPRN_ALARM,      		//���Ա���
	PICK_OPRN_ALARM,         	//��������
	
	ADD_BLE,            		//���������˺�
	DELETE_BLE,           		//ɾ�������˺�
	BLE_OPEN,           		//�����˺ſ���
	
	ADD_CARD,            		//���ӿ�
	DELETE_CARD,           		//ɾ����
	CARD_OPEN,           		//������
	
	ADD_SMART_KEY,            	//���ӵ���Կ��
	DELETE_SMART_KEY,           //ɾ������Կ��
	SMART_KEY_OPEN,           	//����Կ�׿���
	
	ADD_FINGER,            		//����ָ��
	DELETE_FINGER,           	//ɾ��ָ��
	FINGER_OPEN,           		//ָ�ƿ���
    FINGER_ADMIN_CHECK,         //ָ�ƽ���˵� 
	
	ADD_VEIN,            		//����ָ����
	DELETE_VEIN,             	//ɾ��ָ����
	VEIN_OPEN,           		//ָ��������
    VEIN_ADMIN_CHECK,           //ָ��������˵�
	
	ADD_IRIS,            		//���Ӻ�Ĥ
	DELETE_IRIS,            	//ɾ����Ĥ
	IRIS_OPEN,           		//��Ĥ����
    IRIS_ADMIN_CHECK,           //��Ĥ����˵�

}LOCK_EVENT_LOG;
//==============================================================================
extern uint8_t OpenDoorTimeCnt; 
extern uint8_t LockConfigMode; 
extern SYSTEM_FIX_SETTING SystemFixSeting;  //ϵͳ�̻��������ָ����������
extern SYSTEM_SETTING SystemSeting ;//ϵͳ�����ṹ��

uint8_t SystemCfgVersionUpdate(bool isForce);
uint8_t SystemInitFlash( void );  
uint8_t SystemReadFlash( void ) ;
uint8_t SystemReadSeting( uint8_t *setting, uint8_t len );  
uint8_t SystemWriteSeting( uint8_t *setting, uint8_t len );
uint8_t SystemReadFixSeting( uint8_t *setting, uint8_t len );
uint8_t SystemWriteFixSeting(uint8_t *setting, uint8_t len );
void SystemEventLogSave( LOCK_EVENT_LOG event_type, uint16_t pageid );
void SystemEventLogClear( void );

#endif




#ifndef  _SYSTEM_H
#define  _SYSTEM_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <LockConfig.h>

/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define FUNCTION_ENABLE        1      //����ʹ��   
#define FUNCTION_DISABLE       0      //����ʧ��   

#define ADMIN_NONE_REGISTERED  0xA5B7 //��ע��.
#define ADMIN_LOCAL_REGISTERED 0x0001 //���ع���Ա��ע��
#define ADMIN_APP_REGISTERED   0x0002 //APP����Ա��ע��

#define MEM_FACT_MEM_FIG				         'W'  //��Ч��־
#define MEM_USER_MASTER                          'M'
#define MEM_USER_GUEST                           'G'
#define MEM_USER_ALL                             'A'

#define LOCK_BODY_212          0x55
#define LOCK_BODY_218          0x66
#define LOCK_BODY_216          0x77

#define SYSTEM_AESKEY_LEN       16

//�洢�ṹ����
/*------------------------------------------------------------------------------------------------
#define MEM_FACTMEM_FACTORY_FIG                  0  //����������ʼ��־  G
#define MEM_FACTMEM_LOCKMODE                     1  //����ģʽ��ָ��/��������/����Կ�׵ȣ�
#define MEM_FACTMEM_FINGER_APP_NUM               2  //appָ������
#define MEM_FACTMEM_FINGER_ADMIN_NUM		     3  //���ع���Աָ������
#define MEM_FACTMEM_FINGER_GUEST_NUM			 4  //������ָͨ������
#define MEM_FACTMEM_FACE_APP_NUM                 5  //app��������
#define MEM_FACTMEM_FACE_ADMIN_NUM		         6  //���ع���Ա��������
#define MEM_FACTMEM_FACE_GUEST_NUM			     7  //������ͨ��������
#define MEM_FACTMEM_CARD_APP_NUM                 8  //app������
#define MEM_FACTMEM_CARD_ADMIN_NUM		         9  //���ع���Ա������
#define MEM_FACTMEM_CARD_GUEST_NUM			     10 //������ͨ������
#define MEM_FACTMEM_PWD_APP_NUM                  11 //app��������
#define MEM_FACTMEM_PWD_ADMIN_NUM		         12 //���ع���Ա��������
#define MEM_FACTMEM_PWD_GUEST_NUM			     13 //������ͨ��������
#define MEM_FACTMEM_SMARTKEY_NUM                 14 //ȫ������Կ������H ��20����
#define MEM_FACTMEM_WIFI_MAIN_SW                 15 //WIFI������
#define MEM_FACTMEM_WIFI_LOGUP_SW	             16 //WIFI�����ź��ϴ�����
#define MEM_FACTMEM_ADMIN_REGISTER_H             17 //�Ƿ�ע����H 
#define MEM_FACTMEM_ADMIN_REGISTER_L             18 //�Ƿ�ע����L
#define MEM_FACTMEM_LOCKLOG_LINK_H               19 //�¼���¼�ܴ���
#define MEM_FACTMEM_LOCKLOG_LINK_L               20 //�¼���¼�ܴ���
#define MEM_FACTMEM_LOCKLOG_SN_H                 21 //�¼���¼��ǰ���к�H
#define MEM_FACTMEM_LOCKLOG_SN_L                 22 //�¼���¼��ǰ���к�L
#define MEM_FACTMEM_VOICE                        23 //��������
#define MEM_FACTMEM_KEY_DEF                      24 //һ������
#define MEM_FACTMEM_IR_DEF                       25 //�ӽ���Ӧ����
#define MEM_FACTMEM_WIFI_SINGLE                  26 //���嵥˫��
#define MEM_FACTMEM_LOCKTIME	                 27 //�Զ�����ʱ��
#define MEM_FACTMEM_PROTECT_LOCK	             29 //���ŷ�ֹ��
#define MEM_FACTMEM_FINGER_FAC                   30 //ָ�Ƴ��ң�ͨ�������жϼ���
#define MEM_FACTMEM_FINGER_ASEKEY                31 //ָ��ͷAES����Կ16�ֽ�
//+16
#define MEM_FACTMEM_EEPROM_MODEL                 47 //�����ͺ� 8�ֽ�
//+8
#define MEM_FACTMEM_FACE_FAC                     55 //�������̺�
#define MEM_FACTMEM_FACE_ASEKEY                  56 //ָ��ͷAES����Կ16�ֽ�
//+16
#define MEM_FACTMEM_DONE_FIG                     65 //������־G


// ���µ�ַ���������
#define MEM_FACTMEM_MOTOR_DIRECTION              256 // ���ŷ���
#define MEM_FACTMEM_MOTOR_TORQUE				 257 //���Ť��
#define MEM_FACTMEM_MOTOR_TYPE                   258 //��������
--------------------------------------------------------------------------------------------------*/

//=============================��ַ����̻�=====================================
#define MEM_FACT_START				         0     //ϵͳ��Ϣ��ʼ�������
#define MEM_FACT_SIZE				         256   //ϵͳ��Ϣ�ܳ���

#define MEM_FIX_FACT_START				     0x100     //ϵͳ��Ϣ��ʼ���������
#define MEM_FIX_FACT_SIZE				     256   //ϵͳ��Ϣ�ܳ���

#define MEM_PHONE_START                      0x200    //�ֻ�����ʼ
#define MEM_PHONE_LEN			             13        //����6λ
#define MEM_PHONE_NUM	                     1        //һ��

#define MEM_BOARDPWD_START                   0x300    //������ʼ
#define MEM_BOARDPWD_PWDLEN			         6        //����6λ
#define MEM_BOARDPWD_ALL                     1        //һ������

#define MEM_BOARDSOSPWD_START                0x340    //SOS������ʼ
#define MEM_BOARDSOSPWD_PWDLEN			     6        //����6λ
#define MEM_BOARDSOSPWD_ALL                  1        //һ������

#define MEM_BOARDTEMPPWD_START               0x400    //��ʱ������ʼ
#define MEM_BOARDTEMPPWD_PWDLEN			     6        //����6λ
#define MEM_BOARDTEMPPWD_ALL                 10       //һ������

#define MSG_FINGER_REG_START		         0x500	  //ָ��ע����ʼ��ַ
#define MSG_FINGER_REG_ONE_SIZE			     38		  //��ָ�������ֽ�����
#define MSG_FINGER_ONE_SIZE			         64		  //��ָ��ʵ�ʿռ�
#define MSG_FINGER_USER_NUM			         60       //ʹ������
#define MSG_FINGER_NUM_RESERVED			 ((M_FINGER_MAX_TOTAL_NUM >= 100)?100:M_FINGER_MAX_TOTAL_NUM) 	  //100��ָ�ƻ�ָ����Ԥ��
#define MSG_FINGER_ADMIN_NUM                ((M_FINGER_MAX_ADMIN_NUM >= 100)?(MSG_FINGER_NUM_RESERVED - 1):M_FINGER_MAX_ADMIN_NUM)  // ����Ա�û�������С������
#define MSG_FINGER_COMMON_NUM               (MSG_FINGER_NUM_RESERVED - MSG_FINGER_ADMIN_NUM)    // ��ͨ �û�
#define MSG_FINGER_ADMIN_LOCAL_NUM          5       //����GUI����֧�ֵĹ���Ա����
#define MSG_FINGER_COMMON_LOCAL_GUINUM      15      //����GUI����֧�ֵ���ͨ�û�����

#define MSG_FACE_REG_START		             (MSG_FINGER_REG_START+	MSG_FINGER_ONE_SIZE* MSG_FINGER_NUM_RESERVED)    //����ע����ʼ��ַ0x1D00
#define MSG_FACE_REG_ONE_SIZE			     38		  //�����������ֽ�����
#define MSG_FACE_ONE_SIZE			         64		  //������ʵ�ʿռ�
#define MSG_FACE_MASTER_NUM			         5        //��������
#define MSG_FACE_GUEST_NUM			         15       //��ͨ����
#define MSG_FACE_USER_NUM			         20       //ʹ������
#define MSG_FACE_NUM_RESERVED			     100      //100���������ĤԤ��

#define MSG_CPU_CARD_REG_START		         (MSG_FACE_REG_START+	MSG_FACE_ONE_SIZE* MSG_FACE_NUM_RESERVED)       //��ע����ʼ��ַ0x3600
#define MSG_CPU_CARD_REG_ONE_SIZE			 58		  //���������ֽ�����
#define MSG_CPU_CARD_ONE_SIZE			     64		  //����ʵ�ʿռ�
#define MSG_CPU_CARD_USER_NUM			     30       //ʹ������
#define MSG_CARD_NUM			             100      //100�鿨Ԥ��

#define MSG_SMARTKEY_REG_START		         (MSG_CPU_CARD_REG_START+	MSG_CPU_CARD_ONE_SIZE* MSG_CARD_NUM)       //��ע����ʼ��ַ0x4F00
#define MSG_SMARTKEY_REG_ONE_SIZE			 14		  //��Կ�������ֽ�����
#define MSG_SMARTKEY_ONE_SIZE			     64		  //��Կ��ʵ�ʿռ�
#define MSG_SMARTKEY_NUM			         20      //20��Կ��Ԥ��

#define MSG_LOG_RECORD_START                 (MSG_SMARTKEY_REG_START+	MSG_SMARTKEY_ONE_SIZE* MSG_SMARTKEY_NUM) //�¼���¼Ӳ��չ̻���ַ0x5400   21KB��ʼ
#define MSG_LOG_RECORD_REG_ONE_SIZE			 14		  //���¼���¼�����ֽ�����
#define MSG_LOG_RECORD_ONE_SIZE			     16		  //���¼���¼ʵ�ʿռ�
#define MSG_LOG_RECORD_NUM			         600      //600�¼���¼����Ԥ��

//=============================��ַ����̻�=====================================


//ϵͳ�����ṹ�壬����4�ֽڶ���
typedef struct
{
    uint8_t SysFactStartFig;         //0   //����������ʼ��־  G
	uint8_t SysLockMode;             //1   //����ģʽ��ָ��/��������/����Կ�׵ȣ�
    uint8_t SysFingerAllNum;         //2   //ָ������.
    uint8_t SysFingerAdminNum;       //3   //���ع���Աָ������
	
    uint8_t SysFingerGuestNum;       //4   //������ָͨ������
	uint8_t SysFaceAllNum;           //5   //����������
	uint8_t SysFaceAdminNum;	     //6   //���ع���Ա��������
	uint8_t SysFaceGuestNum;		 //7   //������ͨ��������
	
	uint8_t SysCardAllNum;           //8   //��������
	uint8_t SysCardAdminNum	;	     //9   //���ع���Ա������
	uint8_t SysCardGuestNum	;		 //10  //������ͨ������
	uint8_t SysPwdAllNum;            //11  //����������
	
	uint8_t SysPwdAdminNum;		     //12  //���ع���Ա��������
	uint8_t SysPwdGuestNum;			 //13  //������ͨ��������
	uint8_t SysSmartKeyNum;          //14  //ȫ������Կ������h ��20����
	uint8_t SysWifiMainSw;           //15  //wifi������
	
	uint16_t SystemAdminRegister;    //16  //�Ƿ�ע����h
	uint8_t DoorUnlockWarmSw;        //18  //��δ�ر�������  0= �رձ��� 1= ��������
	uint8_t FaceCheckEnable;         //19  //������֤����  0x55= ����  0x66= �ر�

	uint8_t LockBodyMode;            //20  //��������  0x55= 218����  0x66= 212���� 
	uint8_t FingerFlag;              //21 ָ��ͷ���
	uint8_t SysWifiLogSw;	         //22 //wifi�����ź��ϴ�����
	uint8_t SysVoice;                 //23  //��������
	
	uint8_t SysKeyDef;                //24   //һ������
	uint8_t SysHumanIrDef;            //25   //��������(����ʱ��)   0:�ر�  (1-255��)��   
	uint8_t SysWifiSingle;            //26   //���嵥˫��     0:˫�� 1:����   
	uint8_t SysAutoLockTime;	      //27   //�Զ�����ʱ��
	
	uint8_t Sysprotect_lock;	      //28   //���ŷ�ֹ��,ȷ�ϼ�
	uint8_t SysFingerFac;             //29   //ָ�Ƴ��ң�ͨ�������жϼ���
	uint8_t SysFaceFac;               //30   //�������ң�ͨ�������жϼ���
	uint8_t SysDrawNear;              //31   //�ӽ���Ӧ����
	
	uint8_t SysFingerAsekey[SYSTEM_AESKEY_LEN];      //32   //ָ��ͷaes����Կ16�ֽ�
	uint8_t SysFaceAsekey[SYSTEM_AESKEY_LEN];        //48   //����AES����Կ16�ֽ�
	
	uint8_t SysCompoundOpen;           //64 ��Ͽ���(˫����֤����)
	uint8_t SysFactDoneFig;            //65 �̶����
	uint8_t SystemVersion;             //66 �汾��¼
	uint8_t FingerProtocalVersion;     //67 ָ��Э��汾
	
	uint8_t CheckErrAllCnt;            //68 ��֤ʧ���ܼƴ�
	uint8_t CheckErrPwdCnt;            //69 ������֤ʧ�ܼƴ�
	uint8_t LedRGBMode;                //��Χ��ģʽ����ӦAPP_LED.h��LED_RGB_MODE_E
	uint8_t reserved71;                //71 Ԥ��
	
	uint32_t TryForbitUtc ;            //72-75 ������Է���ʱ��
 
	uint16_t FaceOrIrisUnlockSuccessCnt;		//76-77���������ɹ�����
	uint16_t FaceOrIrisUnlockFailCnt;			//78-79��������ʧ�ܴ���
	
	uint16_t FingerOrVeinUnlockSuccessCnt;	//80-81ָ�ƿ����ɹ�����
	uint16_t FingerOrVeinUnlockFailCnt;		//82-83ָ�ƿ���ʧ�ܴ���
	
	uint8_t reserved84;                //84 Ԥ��
	uint8_t reserved85;                //85 Ԥ��
	uint8_t reserved86;                //86 Ԥ��
	uint8_t reserved87;                //87 Ԥ��
	
} SYSTEM_SETTING;

//===============================��������==================================

#define DOUBLE_CHECK_SW_ON      0x55     //��Ͽ��ŷ�ʽ����
#define DOUBLE_CHECK_SW_OFF     0x05     //��Ͽ��ŷ�ʽ�ر�

//���ŷ�ʽ��ͬ����Э��
typedef enum 
{
    LOCK_MODE_PSW = 0x01,//���뿪�ŷ�ʽ.	
	LOCK_MODE_FINGER = 0x02,//ָ�ƿ��ŷ�ʽ.
	LOCK_MODE_BLE = 0x04,//�������ŷ�ʽ.
	LOCK_MODE_FACE = 0x08,//�������ŷ�ʽ.
	LOCK_MODE_CARD = 0x10,//�����ŷ�ʽ
	LOCK_MODE_VEIN = 0x20,//ָ����
	LOCK_MODE_KEY = 0x40, //����Կ�׿���
	LOCK_MODE_IRIS = 0x80, //��Ĥ
}LOCK_MODE_ENUM;

//���ŷ���ṹ��
typedef enum 
{
	RIGHT_HAND_DOOR=0x55,
	LEFT_HAND_DOOR=0x01
}MOTOR_DIRECTION; 

//���Ť��
typedef enum 
{
	LOW_TORQUE=0x01,
	HIGH_TORQUE=0x02
}MOTOR_TORQUE; 

//��������
typedef enum 
{
    OFF_VOICE_VOL,       //�ر�
	LOW_VOICE_VOL,     	 //������
	MEDIUM_VOICE_VOL, 	 //������
    HIGH_VOICE_VOL,		 //������
	
}VOL_SET_E; 

//�ӽ���Ӧ����
typedef enum 
{
    E_SENSE_HIGH = SENSE_HIGH_GRADE,	//Զ����
	E_SENSE_LOW = SENSE_LOW_GRADE,     	//������
    E_SENSE_OFF = SENSE_OFF_GRADE,      //�ر�
	
}NEAR_SENSE_GRADE_T; 

 
typedef struct
{
	//��ʼ��ַ256
	uint8_t SysFixStartFig;                         //����������ʼ��־  G
	uint8_t MotorDirection;						    //���ŷ���      
	uint8_t MotorTorque;                            //���Ť��
	uint8_t LockType;                               //��������	
	uint8_t LockNameType[8];                        //���ͺ�
	uint16_t SysLockLogAll;                         //�¼���¼�ܴ���
	uint16_t SysLockLogSn;                          //�¼���¼��ǰλ��
	uint8_t SysClearCase[14];                       //Ӳ����¼�	
	
} SYSTEM_FIX_SETTING;


//�¼���¼ö��
typedef enum 
{
	NOTHING_CASE, 				//�����޲���
	BAC_OPEN_IN_DOOR,   		//���ڰ��ֿ�
	BAC_CLOSE_IN_DOOR,  		//���ڰ��ֹ�
	FALSE_LOCK_ALARM,  			//��������
	CLOSE_OUT_DOOR,      		//��������
	AUTO_CLOSE_DOOR,     		//�Զ�����
	KEY_OPEN_IN_DOOR,      		//���ڰ�����
	KEY_CLOSE_IN_DOOR,     		//���ڰ�����
	EMPTY_LOCK,        			//�������
	TEMP_PASSWORD_OPEN,      	//��ʱ���뿪��
	NFC_OPEN,         			//NFC����
	
	DELETE_PASSWORD,       		//����ɾ��
	ADD_PASSWORD,            	//��������
	PASSWORD_OPEN,            	//���뿪��
    PWD_ADMIN_CHECK,            //�������˵� 
	
	DELETE_SOS_PASSWORD,       	//��������ɾ��
	ADD_SOS_PASSWORD,           //���ӱ�������
	SOS_PASSWORD_OPEN,          //�������뿪��
	
	ADD_FACE,            		//��������
	DELETE_FACE,          		//ɾ������
	FACE_OPEN,           		//��������
	FACE_ADMIN_CHECK,           //��������˵� 
	
	TRY_OPRN_ALARM,      		//���Ա���
	PICK_OPRN_ALARM,         	//��������
	
	ADD_BLE,            		//���������˺�
	DELETE_BLE,           		//ɾ�������˺�
	BLE_OPEN,           		//�����˺ſ���
	
	ADD_CARD,            		//���ӿ�
	DELETE_CARD,           		//ɾ����
	CARD_OPEN,           		//������
	
	ADD_SMART_KEY,            	//���ӵ���Կ��
	DELETE_SMART_KEY,           //ɾ������Կ��
	SMART_KEY_OPEN,           	//����Կ�׿���
	
	ADD_FINGER,            		//����ָ��
	DELETE_FINGER,           	//ɾ��ָ��
	FINGER_OPEN,           		//ָ�ƿ���
    FINGER_ADMIN_CHECK,         //ָ�ƽ���˵� 
	
	ADD_VEIN,            		//����ָ����
	DELETE_VEIN,             	//ɾ��ָ����
	VEIN_OPEN,           		//ָ��������
    VEIN_ADMIN_CHECK,           //ָ��������˵�
	
	ADD_IRIS,            		//���Ӻ�Ĥ
	DELETE_IRIS,            	//ɾ����Ĥ
	IRIS_OPEN,           		//��Ĥ����
    IRIS_ADMIN_CHECK,           //��Ĥ����˵�

}LOCK_EVENT_LOG;
//==============================================================================
extern uint8_t OpenDoorTimeCnt; 
extern uint8_t LockConfigMode; 
extern SYSTEM_FIX_SETTING SystemFixSeting;  //ϵͳ�̻��������ָ����������
extern SYSTEM_SETTING SystemSeting ;//ϵͳ�����ṹ��

uint8_t SystemCfgVersionUpdate(bool isForce);
uint8_t SystemInitFlash( void );  
uint8_t SystemReadFlash( void ) ;
uint8_t SystemReadSeting( uint8_t *setting, uint8_t len );  
uint8_t SystemWriteSeting( uint8_t *setting, uint8_t len );
uint8_t SystemReadFixSeting( uint8_t *setting, uint8_t len );
uint8_t SystemWriteFixSeting(uint8_t *setting, uint8_t len );
void SystemEventLogSave( LOCK_EVENT_LOG event_type, uint16_t pageid );
void SystemEventLogClear( void );

#endif




