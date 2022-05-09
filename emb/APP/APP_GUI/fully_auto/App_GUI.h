/*********************************************************************************************************************
 * @file:      App_GUI.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-02
 * @brief:     系统主流程函数  
 * @Description:   
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
#ifndef  _APP_GUI_H
#define  _APP_GUI_H

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "App_BLE.h" 
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define DOOR_CUR_OPEN        1   //当前门开着
#define DOOR_CUR_CLSOE       2   //当前门关着

#define LOCKERR_PWD_MAX		 3	 //密码错误最大次数 触发禁试
#define LOCKERR_ALL_MAX		 5	 //验证错误总次数   触发禁试
/*--------------------------------------------------枚举声明---------------------------------------------------------*/
typedef enum
{
  /*-------桌面菜单-----------*/
	EM_MENU_MAIN_DESK,             //系统桌面菜单	
	
  /*-------系统管理菜单-------*/
	EM_MENU_SYSTEM_MANAGE,         //系统管理菜单
	EM_MENU_CHECK_ADMIN,           //验证管理员权限

  /*-------电机动作菜单-------*/
	EM_MENU_OPEN_DOOR,             //执行开门流程
    EM_MENU_CLOSE_DOOR,            //执行关门流程
	
  /*-------人脸类菜单---------*/
    EM_MENU_SET_FACE_MENU,         //人脸设置菜单
	EM_MENU_FACE_ADD_ADMIN,        //增加管理员人脸
	EM_MENU_FACE_ADD_GUEST,        //增加普通用户人脸
	EM_MENU_FACE_DELETE,           //删除人脸
    EM_MENU_FACE_CHECK_ERR,        //人脸验证失败
	
  /*-------指纹类菜单---------*/
    EM_MENU_SET_FINGER_MENU,       //指纹设置菜单
	EM_MENU_FINGER_ADD_ADMIN,      //增加管理员指纹
	EM_MENU_FINGER_ADD_GUEST,      //增加普通用户指纹
	EM_MENU_FINGER_DELETE,         //删除指纹
    EM_MENU_FINGER_CHECK_ERR,      //指纹验证失败
	
  /*-------密码类菜单---------*/
    EM_MENU_SET_PWD_MENU,          //密码设置菜单
	EM_MENU_CHANGE_PWD,            //修改密码
	EM_MENU_PWD_CHECK_ERR,         //密码验证失败
 
  /*-------告警类菜单---------*/
	EM_MENU_BAT_UNWORK,            //电压低无法工作
	EM_MENU_EEPROM_ERR,            //EEPROM故障
	EM_MENU_ALARM_WARM,            //防撬告警
	EM_MENU_TRY_PROTECT,           //禁试保护告警
	EM_MENU_STAY_WARM,             //逗留保护告警
    EM_MENU_DEPLAY_WARM,           //布防保护告警
    EM_MENU_FALSE_LOCK_WARM,       //假锁告警
	EM_MENU_FORGET_LOCK_WARM,      //门未关告警
	EM_MENU_BLE_OPEN_ERR,          //蓝牙开门失败
 
  /*-------模式类菜单---------*/
	EM_MENU_BACK_FACTORY,          //恢复出厂设置
	EM_MENU_OTA_MODEL,             //升级模式
    EM_MENU_APP_MODEL,             //APP设置模式
	
  /*-------系统设置类菜单-----*/
	EM_MENU_SYSTEM_PARA_SET,       //系统参数设置
	EM_MENU_MOTOR_DIR_SET,         //电机方向设置
	EM_MENU_MOTOR_TORSION_SET,     //电机扭力设置
    EM_MENU_AUTO_LOCK_SET,         //自动上锁设置
	EM_MENU_DOUBLE_CHECK_SET,      //双重认证设置
	EM_MENU_VOL_ADJUST_SET,        //音量调节设置
	EM_MENU_NEAR_SENSE_SET,        //接近感应设置
	EM_MENU_DEPPOY_SET,            //布防设置
	EM_MENU_STAY_CHECK_SET,        //逗留设置
 
  /*-------其他类菜单---------*/
	EM_MENU_ERROR_CHECK,           //系统故障检测
	EM_MENU_SMART_SCREEN_SHOW,     //智能屏表情展示
	EM_MENU_BELL_VIDEO,            //门铃视频处理
    EM_MENU_NETWORK_UPDATE,        //网络同步处理
	EM_MENU_WEATHER_UPDATE,        //天气同步处理
    EM_MENU_BELL_LAMP_DISPLAY,     //门铃灯显示处理
	EM_MENU_WAKEUP_BUT_SLEEP, 	   //唤醒后无感知处理
	
  /*-------老化测试菜单-------*/
	EN_MENU_AGING_TEST,            //老化测试
	
  /*--------休眠菜单---------*/
    MENU_SYSTEM_SLEEP,

  /*--------菜单为空---------*/
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
   EM_OPEN_BUTTON,      //机械按键开门
   EM_OPEN_HANDLER,     //门内把手开门
   EM_OPEN_PWD,         //密码开门
   EM_OPEN_FACE,		//人脸开门
   EM_OPEN_FIENGER,		//指纹开门
   EM_OPEN_CARD,		//智卡开门
   EM_OPEN_IRIS,		//虹膜开门
   EM_OPEN_VEIN,		//静脉开门
   EM_OPEN_TMP_PWD,		//临时开门
   EM_OPEN_SOS_PWD,		//报警开门
   EM_OPEN_PHONE,       //手机开门
   EM_OPEN_BLE_KEY,     //蓝牙钥匙
	
}OPEN_MODEL_E;	

typedef enum
{
   EM_CLOSE_DEFAULT,
   EM_CLOSE_BUTTON,      //机械按键锁门
   EM_CLOSE_HANDLER,     //门内把手锁门
   EM_CLOSE_TOUCH,       //触摸按键锁门
   EM_CLOSE_AUTO,        //自动上锁锁门
	
}CLOSE_MODEL_E;	

typedef enum
{
   E_WAKE_DEFAULT,
   E_WAKE_OPEN_BUTTON,      //机械按键开门唤醒
   E_WAKE_CLOSE_BUTTON,     //机械按键锁门唤醒
   E_WAKE_LEFT_HANDLER,     //门内把手左转动唤醒
   E_WAKE_MIDDLE_HANDLER,   //门内把手中键转动唤醒
   E_WAKE_RIGHT_HANDLER,    //门内把手右转动唤醒
   E_WAKE_TOUCH,      		//触摸按键唤醒 
   E_WAKE_ALARM,            //防撬唤醒
   E_WAKE_FINGER,           //指纹唤醒
   E_WAKE_NEAR_SENSE,       //接近感应唤醒
   E_WAKE_AUTO_LOCK,        //自动上锁锁门唤醒
   E_WAKE_STAY_DEFENSE,     //主动防御唤醒
   E_WAKE_DEPLOY_WARM,      //布防告警唤醒
   E_WAKE_BLE_COM,          //蓝牙通信唤醒
   E_WAKE_MOTOR_LATCH,      //斜舌唤醒
   E_WAKE_MOTOR_BOLT,       //主舌唤醒
   E_WAKE_MOTOR_TRIGGER,    //三角舌唤醒
   E_WAKE_FORGET_LOCK,      //未锁门告警唤醒
   E_WAKE_FALSE_LOCK,       //假锁告警唤醒
   E_WAKE_HANDLE_TRY,       //把手试玩告警唤醒
   E_WAKE_CAMERA_WIFI,      //猫眼WiFi唤醒 
   E_WAKE_NETWORK_UPDATE,   //网络同步唤醒 
   E_WAKE_BELL_KEY,         //门铃唤醒 
   E_WAKE_WEATHER_UPDATE,   //更新天气唤醒  
   E_WAKE_ALARM_BREAK,      //中断防撬唤醒
   E_WAKE_REGISTER_BUTTON,  //注册键唤醒唤醒

   E_WAKE_OTHERS,           //其他方式唤醒
	
}WAKEUP_TYPE_E;	

typedef enum
{
   E_MODE_DEFAULT,
   E_MODE_DEPLOY,           //一键布防模式
   E_MODE_LEAVE_HOME,       //离家模式
   E_MODE_AT_HOME,          //在家模式
	
}WORK_MODE_E;	

typedef enum
{
   EM_TRY_DEFAULT,
   EM_TRY_FIRST_OVER,      //3分钟内第一次进禁试
}TRY_ALARM_E;

typedef enum
{
   E_SYSTEM_INIT,        //初始化状态
   E_SYSTEM_SELFCHECK,   //自检测状态  
   E_SYSTEM_VOICE_CFG,   //语音配置状态  
   E_SYSTEM_WORKING, 	 //工作中 
	
}SYSTEM_WORK_STS_E;

/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/      


typedef struct 
{  
	uint8_t CurSorStep;     //当前menu光标位置 
	uint8_t CurMenuNum;	    //当前所选的子菜单
	struct
	{
		MenuIndexEnum_E Currently;
		MenuIndexEnum_E Next;
	}MenuIndexType;
	
}MenuItemType_T;


typedef struct
{
	//MenuIndexEnum MenuStatusIndexParent;	//上级菜单 
	MenuIndexEnum_E MenuStatusIndex;	
	uint8_t MenuCount;	//本级菜单的子菜单	
	void (*CurrentOperate)();
	
}KdbTabType_T;
  
extern bool FingerWorkState;

extern uint8_t UploadUnlockDoorMegEnable;
/*--------------------------------------------------函数声明---------------------------------------------------------*/
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

