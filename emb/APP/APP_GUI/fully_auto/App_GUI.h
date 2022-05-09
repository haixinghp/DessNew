/*********************************************************************************************************************
 * @file:      App_GUI.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-02
 * @brief:     ϵͳ�����̺���  
 * @Description:   
 * @ChangeList:  01. ����
*********************************************************************************************************************/
#ifndef  _APP_GUI_H
#define  _APP_GUI_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "App_BLE.h" 
/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define DOOR_CUR_OPEN        1   //��ǰ�ſ���
#define DOOR_CUR_CLSOE       2   //��ǰ�Ź���

#define LOCKERR_PWD_MAX		 3	 //������������� ��������
#define LOCKERR_ALL_MAX		 5	 //��֤�����ܴ���   ��������
/*--------------------------------------------------ö������---------------------------------------------------------*/
typedef enum
{
  /*-------����˵�-----------*/
	EM_MENU_MAIN_DESK,             //ϵͳ����˵�	
	
  /*-------ϵͳ����˵�-------*/
	EM_MENU_SYSTEM_MANAGE,         //ϵͳ����˵�
	EM_MENU_CHECK_ADMIN,           //��֤����ԱȨ��

  /*-------��������˵�-------*/
	EM_MENU_OPEN_DOOR,             //ִ�п�������
    EM_MENU_CLOSE_DOOR,            //ִ�й�������
	
  /*-------������˵�---------*/
    EM_MENU_SET_FACE_MENU,         //�������ò˵�
	EM_MENU_FACE_ADD_ADMIN,        //���ӹ���Ա����
	EM_MENU_FACE_ADD_GUEST,        //������ͨ�û�����
	EM_MENU_FACE_DELETE,           //ɾ������
    EM_MENU_FACE_CHECK_ERR,        //������֤ʧ��
	
  /*-------ָ����˵�---------*/
    EM_MENU_SET_FINGER_MENU,       //ָ�����ò˵�
	EM_MENU_FINGER_ADD_ADMIN,      //���ӹ���Աָ��
	EM_MENU_FINGER_ADD_GUEST,      //������ͨ�û�ָ��
	EM_MENU_FINGER_DELETE,         //ɾ��ָ��
    EM_MENU_FINGER_CHECK_ERR,      //ָ����֤ʧ��
	
  /*-------������˵�---------*/
    EM_MENU_SET_PWD_MENU,          //�������ò˵�
	EM_MENU_CHANGE_PWD,            //�޸�����
	EM_MENU_PWD_CHECK_ERR,         //������֤ʧ��
 
  /*-------�澯��˵�---------*/
	EM_MENU_BAT_UNWORK,            //��ѹ���޷�����
	EM_MENU_EEPROM_ERR,            //EEPROM����
	EM_MENU_ALARM_WARM,            //���˸澯
	EM_MENU_TRY_PROTECT,           //���Ա����澯
	EM_MENU_STAY_WARM,             //���������澯
    EM_MENU_DEPLAY_WARM,           //���������澯
    EM_MENU_FALSE_LOCK_WARM,       //�����澯
	EM_MENU_FORGET_LOCK_WARM,      //��δ�ظ澯
	EM_MENU_BLE_OPEN_ERR,          //��������ʧ��
 
  /*-------ģʽ��˵�---------*/
	EM_MENU_BACK_FACTORY,          //�ָ���������
	EM_MENU_OTA_MODEL,             //����ģʽ
    EM_MENU_APP_MODEL,             //APP����ģʽ
	
  /*-------ϵͳ������˵�-----*/
	EM_MENU_SYSTEM_PARA_SET,       //ϵͳ��������
	EM_MENU_MOTOR_DIR_SET,         //�����������
	EM_MENU_MOTOR_TORSION_SET,     //���Ť������
    EM_MENU_AUTO_LOCK_SET,         //�Զ���������
	EM_MENU_DOUBLE_CHECK_SET,      //˫����֤����
	EM_MENU_VOL_ADJUST_SET,        //������������
	EM_MENU_NEAR_SENSE_SET,        //�ӽ���Ӧ����
	EM_MENU_DEPPOY_SET,            //��������
	EM_MENU_STAY_CHECK_SET,        //��������
 
  /*-------������˵�---------*/
	EM_MENU_ERROR_CHECK,           //ϵͳ���ϼ��
	EM_MENU_SMART_SCREEN_SHOW,     //����������չʾ
	EM_MENU_BELL_VIDEO,            //������Ƶ����
    EM_MENU_NETWORK_UPDATE,        //����ͬ������
	EM_MENU_WEATHER_UPDATE,        //����ͬ������
    EM_MENU_BELL_LAMP_DISPLAY,     //�������ʾ����
	EM_MENU_WAKEUP_BUT_SLEEP, 	   //���Ѻ��޸�֪����
	
  /*-------�ϻ����Բ˵�-------*/
	EN_MENU_AGING_TEST,            //�ϻ�����
	
  /*--------���߲˵�---------*/
    MENU_SYSTEM_SLEEP,

  /*--------�˵�Ϊ��---------*/
    MENU_NULL,			 
 	
}MenuIndexEnum_E;  
 

typedef enum
{
	EM_MENU_STEP_1 = 0,          
	EM_MENU_STEP_2,   
	EM_MENU_STEP_3,   		
	EM_MENU_STEP_4,   
	EM_MENU_STEP_5,  
    EM_MENU_STEP_6,   
	EM_MENU_STEP_7,  
	EM_MENU_STEP_8,  
    EM_MENU_STEP_9,   
	EM_MENU_STEP_10,
	EM_MENU_STEP_11,
	EM_MENU_STEP_12,
	EM_MENU_STEP_13,
 	
}MenuStepIndex_E;  


typedef enum
{
   EM_OPEN_DEFAULT,
   EM_OPEN_BUTTON,      //��е��������
   EM_OPEN_HANDLER,     //���ڰ��ֿ���
   EM_OPEN_PWD,         //���뿪��
   EM_OPEN_FACE,		//��������
   EM_OPEN_FIENGER,		//ָ�ƿ���
   EM_OPEN_CARD,		//�ǿ�����
   EM_OPEN_IRIS,		//��Ĥ����
   EM_OPEN_VEIN,		//��������
   EM_OPEN_TMP_PWD,		//��ʱ����
   EM_OPEN_SOS_PWD,		//��������
   EM_OPEN_PHONE,       //�ֻ�����
   EM_OPEN_BLE_KEY,     //����Կ��
	
}OPEN_MODEL_E;	

typedef enum
{
   EM_CLOSE_DEFAULT,
   EM_CLOSE_BUTTON,      //��е��������
   EM_CLOSE_HANDLER,     //���ڰ�������
   EM_CLOSE_TOUCH,       //������������
   EM_CLOSE_AUTO,        //�Զ���������
	
}CLOSE_MODEL_E;	

typedef enum
{
   E_WAKE_DEFAULT,
   E_WAKE_OPEN_BUTTON,      //��е�������Ż���
   E_WAKE_CLOSE_BUTTON,     //��е�������Ż���
   E_WAKE_LEFT_HANDLER,     //���ڰ�����ת������
   E_WAKE_MIDDLE_HANDLER,   //���ڰ����м�ת������
   E_WAKE_RIGHT_HANDLER,    //���ڰ�����ת������
   E_WAKE_TOUCH,      		//������������ 
   E_WAKE_ALARM,            //���˻���
   E_WAKE_FINGER,           //ָ�ƻ���
   E_WAKE_NEAR_SENSE,       //�ӽ���Ӧ����
   E_WAKE_AUTO_LOCK,        //�Զ��������Ż���
   E_WAKE_STAY_DEFENSE,     //������������
   E_WAKE_DEPLOY_WARM,      //�����澯����
   E_WAKE_BLE_COM,          //����ͨ�Ż���
   E_WAKE_MOTOR_LATCH,      //б�໽��
   E_WAKE_MOTOR_BOLT,       //���໽��
   E_WAKE_MOTOR_TRIGGER,    //�����໽��
   E_WAKE_FORGET_LOCK,      //δ���Ÿ澯����
   E_WAKE_FALSE_LOCK,       //�����澯����
   E_WAKE_HANDLE_TRY,       //��������澯����
   E_WAKE_CAMERA_WIFI,      //è��WiFi���� 
   E_WAKE_NETWORK_UPDATE,   //����ͬ������ 
   E_WAKE_BELL_KEY,         //���廽�� 
   E_WAKE_WEATHER_UPDATE,   //������������  
   E_WAKE_ALARM_BREAK,      //�жϷ��˻���
   E_WAKE_REGISTER_BUTTON,  //ע������ѻ���

   E_WAKE_OTHERS,           //������ʽ����
	
}WAKEUP_TYPE_E;	

typedef enum
{
   E_MODE_DEFAULT,
   E_MODE_DEPLOY,           //һ������ģʽ
   E_MODE_LEAVE_HOME,       //���ģʽ
   E_MODE_AT_HOME,          //�ڼ�ģʽ
	
}WORK_MODE_E;	

typedef enum
{
   EM_TRY_DEFAULT,
   EM_TRY_FIRST_OVER,      //3�����ڵ�һ�ν�����
}TRY_ALARM_E;

typedef enum
{
   E_SYSTEM_INIT,        //��ʼ��״̬
   E_SYSTEM_SELFCHECK,   //�Լ��״̬  
   E_SYSTEM_VOICE_CFG,   //��������״̬  
   E_SYSTEM_WORKING, 	 //������ 
	
}SYSTEM_WORK_STS_E;

/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/      


typedef struct 
{  
	uint8_t CurSorStep;     //��ǰmenu���λ�� 
	uint8_t CurMenuNum;	    //��ǰ��ѡ���Ӳ˵�
	struct
	{
		MenuIndexEnum_E Currently;
		MenuIndexEnum_E Next;
	}MenuIndexType;
	
}MenuItemType_T;


typedef struct
{
	//MenuIndexEnum MenuStatusIndexParent;	//�ϼ��˵� 
	MenuIndexEnum_E MenuStatusIndex;	
	uint8_t MenuCount;	//�����˵����Ӳ˵�	
	void (*CurrentOperate)();
	
}KdbTabType_T;
  
extern bool FingerWorkState;

extern uint8_t UploadUnlockDoorMegEnable;
/*--------------------------------------------------��������---------------------------------------------------------*/
void App_GUI_FileInit( void );
void App_GUI_WakeupInit( void );
void App_GUI_SleepInit( void );
void App_GUI_Tim10Ms( void );
void App_GUI_Tim1000Ms( void );
void App_GUI_MenuProcess( void );
void App_GUI_MenuJump( MenuIndexEnum_E pageNo );
void App_GUI_UpdateMenuQuitTime( uint32_t para, bool mode );
void App_GUI_RelieveTryProtect( void );
void App_GUI_SetOpenModel( OPEN_MODEL_E type );
void App_GUI_SetCloseModel( CLOSE_MODEL_E type );
uint8_t App_GUI_GetSysSleepSts( void );
void App_GUI_SetSysSleepSts( uint8_t para );	
bool App_GUI_GetWifiUploadSwSts( void );
bool App_GUI_GetDoubleCheckSwSts( void );
DoubleCheckType_U App_GUI_GetDoubleCheckType( void );
void App_GUI_DefaultDoorState( void );
uint8_t  App_GUI_GetNearSenseUnworkCurTim( void );
uint16_t App_GUI_GetRegisterSts( void );
void App_GUI_SetSysWakeupType( WAKEUP_TYPE_E type );
WAKEUP_TYPE_E  App_GUI_GetSysWakeupType( void );
uint8_t App_GUI_CheckNetworkAction( void );
uint8_t App_GUI_CheckWeatherUpdateAction( void );
bool App_GUI_GetNetworkErrState( void );

WORK_MODE_E  App_GUI_GetSysWorkMode( void );
void App_GUI_SetSysSysWorkMode( WORK_MODE_E mode );

OPEN_MODEL_E  App_GUI_GetOpenModel( void );
CLOSE_MODEL_E App_GUI_GetCloseModel( void );

MenuIndexEnum_E  App_GUI_GetCurMenuNo( void );

uint8_t CAM_PirWakeUpTest(void);
uint8_t App_GUI_StayDetectSts( void );
  
void App_GUI_SetSystemWorkSts( SYSTEM_WORK_STS_E workstate );
SYSTEM_WORK_STS_E App_GUI_GetSystemWorkSts( void );











#endif
/*--------------------------------------------------THE FILE END-----------------------------------------------------*/

