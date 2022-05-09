#ifndef  _SYSTEM_H
#define  _SYSTEM_H

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <LockConfig.h>

/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define FUNCTION_ENABLE        1      //功能使能   
#define FUNCTION_DISABLE       0      //功能失能   

#define ADMIN_NONE_REGISTERED  0xA5B7 //无注册.
#define ADMIN_LOCAL_REGISTERED 0x0001 //本地管理员已注册
#define ADMIN_APP_REGISTERED   0x0002 //APP管理员已注册

#define MEM_FACT_MEM_FIG				         'W'  //有效标志
#define MEM_USER_MASTER                          'M'
#define MEM_USER_GUEST                           'G'
#define MEM_USER_ALL                             'A'

#define LOCK_BODY_212          0x55
#define LOCK_BODY_218          0x66
#define LOCK_BODY_216          0x77

#define SYSTEM_AESKEY_LEN       16

//存储结构定义
/*------------------------------------------------------------------------------------------------
#define MEM_FACTMEM_FACTORY_FIG                  0  //工厂数据起始标志  G
#define MEM_FACTMEM_LOCKMODE                     1  //开锁模式（指纹/密码蓝牙/蓝牙钥匙等）
#define MEM_FACTMEM_FINGER_APP_NUM               2  //app指纹数量
#define MEM_FACTMEM_FINGER_ADMIN_NUM		     3  //本地管理员指纹数量
#define MEM_FACTMEM_FINGER_GUEST_NUM			 4  //本地普通指纹数量
#define MEM_FACTMEM_FACE_APP_NUM                 5  //app人脸数量
#define MEM_FACTMEM_FACE_ADMIN_NUM		         6  //本地管理员人脸数量
#define MEM_FACTMEM_FACE_GUEST_NUM			     7  //本地普通人脸数量
#define MEM_FACTMEM_CARD_APP_NUM                 8  //app卡数量
#define MEM_FACTMEM_CARD_ADMIN_NUM		         9  //本地管理员卡数量
#define MEM_FACTMEM_CARD_GUEST_NUM			     10 //本地普通卡数量
#define MEM_FACTMEM_PWD_APP_NUM                  11 //app密码数量
#define MEM_FACTMEM_PWD_ADMIN_NUM		         12 //本地管理员密码数量
#define MEM_FACTMEM_PWD_GUEST_NUM			     13 //本地普通密码数量
#define MEM_FACTMEM_SMARTKEY_NUM                 14 //全部蓝牙钥匙数量H （20个）
#define MEM_FACTMEM_WIFI_MAIN_SW                 15 //WIFI主开关
#define MEM_FACTMEM_WIFI_LOGUP_SW	             16 //WIFI开门信号上传开关
#define MEM_FACTMEM_ADMIN_REGISTER_H             17 //是否注册标记H 
#define MEM_FACTMEM_ADMIN_REGISTER_L             18 //是否注册标记L
#define MEM_FACTMEM_LOCKLOG_LINK_H               19 //事件记录总次数
#define MEM_FACTMEM_LOCKLOG_LINK_L               20 //事件记录总次数
#define MEM_FACTMEM_LOCKLOG_SN_H                 21 //事件记录当前序列号H
#define MEM_FACTMEM_LOCKLOG_SN_L                 22 //事件记录当前序列号L
#define MEM_FACTMEM_VOICE                        23 //语音开关
#define MEM_FACTMEM_KEY_DEF                      24 //一键布防
#define MEM_FACTMEM_IR_DEF                       25 //接近感应设置
#define MEM_FACTMEM_WIFI_SINGLE                  26 //门铃单双向
#define MEM_FACTMEM_LOCKTIME	                 27 //自动上锁时间
#define MEM_FACTMEM_PROTECT_LOCK	             29 //锁门防止误触
#define MEM_FACTMEM_FINGER_FAC                   30 //指纹厂家，通过厂家判断加密
#define MEM_FACTMEM_FINGER_ASEKEY                31 //指纹头AES根秘钥16字节
//+16
#define MEM_FACTMEM_EEPROM_MODEL                 47 //锁具型号 8字节
//+8
#define MEM_FACTMEM_FACE_FAC                     55 //人脸厂商号
#define MEM_FACTMEM_FACE_ASEKEY                  56 //指纹头AES根秘钥16字节
//+16
#define MEM_FACTMEM_DONE_FIG                     65 //工厂标志G


// 以下地址不允许清空
#define MEM_FACTMEM_MOTOR_DIRECTION              256 // 开门方向
#define MEM_FACTMEM_MOTOR_TORQUE				 257 //电机扭力
#define MEM_FACTMEM_MOTOR_TYPE                   258 //锁体类型
--------------------------------------------------------------------------------------------------*/

//=============================地址分配固化=====================================
#define MEM_FACT_START				         0     //系统信息起始，可清空
#define MEM_FACT_SIZE				         256   //系统信息总长度

#define MEM_FIX_FACT_START				     0x100     //系统信息起始，不可清空
#define MEM_FIX_FACT_SIZE				     256   //系统信息总长度

#define MEM_PHONE_START                      0x200    //手机号起始
#define MEM_PHONE_LEN			             13        //长度6位
#define MEM_PHONE_NUM	                     1        //一组

#define MEM_BOARDPWD_START                   0x300    //密码起始
#define MEM_BOARDPWD_PWDLEN			         6        //长度6位
#define MEM_BOARDPWD_ALL                     1        //一组密码

#define MEM_BOARDSOSPWD_START                0x340    //SOS密码起始
#define MEM_BOARDSOSPWD_PWDLEN			     6        //长度6位
#define MEM_BOARDSOSPWD_ALL                  1        //一组密码

#define MEM_BOARDTEMPPWD_START               0x400    //临时密码起始
#define MEM_BOARDTEMPPWD_PWDLEN			     6        //长度6位
#define MEM_BOARDTEMPPWD_ALL                 10       //一组密码

#define MSG_FINGER_REG_START		         0x500	  //指纹注册起始地址
#define MSG_FINGER_REG_ONE_SIZE			     38		  //单指纹所用字节数量
#define MSG_FINGER_ONE_SIZE			         64		  //单指纹实际空间
#define MSG_FINGER_USER_NUM			         60       //使用数量
#define MSG_FINGER_NUM_RESERVED			 ((M_FINGER_MAX_TOTAL_NUM >= 100)?100:M_FINGER_MAX_TOTAL_NUM) 	  //100组指纹或指静脉预留
#define MSG_FINGER_ADMIN_NUM                ((M_FINGER_MAX_ADMIN_NUM >= 100)?(MSG_FINGER_NUM_RESERVED - 1):M_FINGER_MAX_ADMIN_NUM)  // 管理员用户数必须小于总数
#define MSG_FINGER_COMMON_NUM               (MSG_FINGER_NUM_RESERVED - MSG_FINGER_ADMIN_NUM)    // 普通 用户
#define MSG_FINGER_ADMIN_LOCAL_NUM          5       //本地GUI界面支持的管理员数量
#define MSG_FINGER_COMMON_LOCAL_GUINUM      15      //本地GUI界面支持的普通用户数量

#define MSG_FACE_REG_START		             (MSG_FINGER_REG_START+	MSG_FINGER_ONE_SIZE* MSG_FINGER_NUM_RESERVED)    //人脸注册起始地址0x1D00
#define MSG_FACE_REG_ONE_SIZE			     38		  //单人脸所用字节数量
#define MSG_FACE_ONE_SIZE			         64		  //单人脸实际空间
#define MSG_FACE_MASTER_NUM			         5        //管理数量
#define MSG_FACE_GUEST_NUM			         15       //普通数量
#define MSG_FACE_USER_NUM			         20       //使用数量
#define MSG_FACE_NUM_RESERVED			     100      //100组人脸或虹膜预留

#define MSG_CPU_CARD_REG_START		         (MSG_FACE_REG_START+	MSG_FACE_ONE_SIZE* MSG_FACE_NUM_RESERVED)       //卡注册起始地址0x3600
#define MSG_CPU_CARD_REG_ONE_SIZE			 58		  //单卡所用字节数量
#define MSG_CPU_CARD_ONE_SIZE			     64		  //单卡实际空间
#define MSG_CPU_CARD_USER_NUM			     30       //使用数量
#define MSG_CARD_NUM			             100      //100组卡预留

#define MSG_SMARTKEY_REG_START		         (MSG_CPU_CARD_REG_START+	MSG_CPU_CARD_ONE_SIZE* MSG_CARD_NUM)       //卡注册起始地址0x4F00
#define MSG_SMARTKEY_REG_ONE_SIZE			 14		  //单钥匙所用字节数量
#define MSG_SMARTKEY_ONE_SIZE			     64		  //单钥匙实际空间
#define MSG_SMARTKEY_NUM			         20      //20组钥匙预留

#define MSG_LOG_RECORD_START                 (MSG_SMARTKEY_REG_START+	MSG_SMARTKEY_ONE_SIZE* MSG_SMARTKEY_NUM) //事件记录硬清空固化地址0x5400   21KB起始
#define MSG_LOG_RECORD_REG_ONE_SIZE			 14		  //单事件记录所用字节数量
#define MSG_LOG_RECORD_ONE_SIZE			     16		  //单事件记录实际空间
#define MSG_LOG_RECORD_NUM			         600      //600事件记录数量预留

//=============================地址分配固化=====================================


//系统参数结构体，必须4字节对齐
typedef struct
{
    uint8_t SysFactStartFig;         //0   //工厂数据起始标志  G
	uint8_t SysLockMode;             //1   //开锁模式（指纹/密码蓝牙/蓝牙钥匙等）
    uint8_t SysFingerAllNum;         //2   //指纹总量.
    uint8_t SysFingerAdminNum;       //3   //本地管理员指纹数量
	
    uint8_t SysFingerGuestNum;       //4   //本地普通指纹数量
	uint8_t SysFaceAllNum;           //5   //人脸总数量
	uint8_t SysFaceAdminNum;	     //6   //本地管理员人脸数量
	uint8_t SysFaceGuestNum;		 //7   //本地普通人脸数量
	
	uint8_t SysCardAllNum;           //8   //卡总数量
	uint8_t SysCardAdminNum	;	     //9   //本地管理员卡数量
	uint8_t SysCardGuestNum	;		 //10  //本地普通卡数量
	uint8_t SysPwdAllNum;            //11  //密码总数量
	
	uint8_t SysPwdAdminNum;		     //12  //本地管理员密码数量
	uint8_t SysPwdGuestNum;			 //13  //本地普通密码数量
	uint8_t SysSmartKeyNum;          //14  //全部蓝牙钥匙数量h （20个）
	uint8_t SysWifiMainSw;           //15  //wifi主开关
	
	uint16_t SystemAdminRegister;    //16  //是否注册标记h
	uint8_t DoorUnlockWarmSw;        //18  //门未关报警设置  0= 关闭报警 1= 开启报警
	uint8_t FaceCheckEnable;         //19  //人脸验证开关  0x55= 开启  0x66= 关闭

	uint8_t LockBodyMode;            //20  //锁体类型  0x55= 218锁体  0x66= 212锁体 
	uint8_t FingerFlag;              //21 指纹头标记
	uint8_t SysWifiLogSw;	         //22 //wifi开门信号上传开关
	uint8_t SysVoice;                 //23  //语音开关
	
	uint8_t SysKeyDef;                //24   //一键布防
	uint8_t SysHumanIrDef;            //25   //主动防御(工作时间)   0:关闭  (1-255秒)打开   
	uint8_t SysWifiSingle;            //26   //门铃单双向     0:双向 1:单向   
	uint8_t SysAutoLockTime;	      //27   //自动上锁时间
	
	uint8_t Sysprotect_lock;	      //28   //锁门防止误触,确认键
	uint8_t SysFingerFac;             //29   //指纹厂家，通过厂家判断加密
	uint8_t SysFaceFac;               //30   //人脸厂家，通过厂家判断加密
	uint8_t SysDrawNear;              //31   //接近感应距离
	
	uint8_t SysFingerAsekey[SYSTEM_AESKEY_LEN];      //32   //指纹头aes根秘钥16字节
	uint8_t SysFaceAsekey[SYSTEM_AESKEY_LEN];        //48   //人脸AES根秘钥16字节
	
	uint8_t SysCompoundOpen;           //64 组合开门(双重认证开关)
	uint8_t SysFactDoneFig;            //65 固定标记
	uint8_t SystemVersion;             //66 版本记录
	uint8_t FingerProtocalVersion;     //67 指纹协议版本
	
	uint8_t CheckErrAllCnt;            //68 验证失败总计次
	uint8_t CheckErrPwdCnt;            //69 密码验证失败计次
	uint8_t LedRGBMode;                //氛围灯模式，对应APP_LED.h的LED_RGB_MODE_E
	uint8_t reserved71;                //71 预留
	
	uint32_t TryForbitUtc ;            //72-75 错误禁试发生时间
 
	uint16_t FaceOrIrisUnlockSuccessCnt;		//76-77人脸开锁成功次数
	uint16_t FaceOrIrisUnlockFailCnt;			//78-79人脸开锁失败次数
	
	uint16_t FingerOrVeinUnlockSuccessCnt;	//80-81指纹开锁成功次数
	uint16_t FingerOrVeinUnlockFailCnt;		//82-83指纹开锁失败次数
	
	uint8_t reserved84;                //84 预留
	uint8_t reserved85;                //85 预留
	uint8_t reserved86;                //86 预留
	uint8_t reserved87;                //87 预留
	
} SYSTEM_SETTING;

//===============================参数定义==================================

#define DOUBLE_CHECK_SW_ON      0x55     //组合开门方式开启
#define DOUBLE_CHECK_SW_OFF     0x05     //组合开门方式关闭

//开门方式，同蓝牙协议
typedef enum 
{
    LOCK_MODE_PSW = 0x01,//密码开门方式.	
	LOCK_MODE_FINGER = 0x02,//指纹开门方式.
	LOCK_MODE_BLE = 0x04,//蓝牙开门方式.
	LOCK_MODE_FACE = 0x08,//人脸开门方式.
	LOCK_MODE_CARD = 0x10,//卡开门方式
	LOCK_MODE_VEIN = 0x20,//指静脉
	LOCK_MODE_KEY = 0x40, //电子钥匙开门
	LOCK_MODE_IRIS = 0x80, //虹膜
}LOCK_MODE_ENUM;

//开门方向结构体
typedef enum 
{
	RIGHT_HAND_DOOR=0x55,
	LEFT_HAND_DOOR=0x01
}MOTOR_DIRECTION; 

//电机扭力
typedef enum 
{
	LOW_TORQUE=0x01,
	HIGH_TORQUE=0x02
}MOTOR_TORQUE; 

//音量调节
typedef enum 
{
    OFF_VOICE_VOL,       //关闭
	LOW_VOICE_VOL,     	 //低音量
	MEDIUM_VOICE_VOL, 	 //中音量
    HIGH_VOICE_VOL,		 //高音量
	
}VOL_SET_E; 

//接近感应调节
typedef enum 
{
    E_SENSE_HIGH = SENSE_HIGH_GRADE,	//远距离
	E_SENSE_LOW = SENSE_LOW_GRADE,     	//近距离
    E_SENSE_OFF = SENSE_OFF_GRADE,      //关闭
	
}NEAR_SENSE_GRADE_T; 

 
typedef struct
{
	//起始地址256
	uint8_t SysFixStartFig;                         //工厂数据起始标志  G
	uint8_t MotorDirection;						    //开门方向      
	uint8_t MotorTorque;                            //电机扭力
	uint8_t LockType;                               //锁体类型	
	uint8_t LockNameType[8];                        //锁型号
	uint16_t SysLockLogAll;                         //事件记录总次数
	uint16_t SysLockLogSn;                          //事件记录当前位置
	uint8_t SysClearCase[14];                       //硬清空事件	
	
} SYSTEM_FIX_SETTING;


//事件记录枚举
typedef enum 
{
	NOTHING_CASE, 				//唤醒无操作
	BAC_OPEN_IN_DOOR,   		//门内把手开
	BAC_CLOSE_IN_DOOR,  		//门内把手关
	FALSE_LOCK_ALARM,  			//假锁报警
	CLOSE_OUT_DOOR,      		//门外上锁
	AUTO_CLOSE_DOOR,     		//自动上锁
	KEY_OPEN_IN_DOOR,      		//门内按键开
	KEY_CLOSE_IN_DOOR,     		//门内按键关
	EMPTY_LOCK,        			//清空锁具
	TEMP_PASSWORD_OPEN,      	//临时密码开门
	NFC_OPEN,         			//NFC开门
	
	DELETE_PASSWORD,       		//密码删除
	ADD_PASSWORD,            	//增加密码
	PASSWORD_OPEN,            	//密码开门
    PWD_ADMIN_CHECK,            //密码进入菜单 
	
	DELETE_SOS_PASSWORD,       	//报警密码删除
	ADD_SOS_PASSWORD,           //增加报警密码
	SOS_PASSWORD_OPEN,          //报警密码开门
	
	ADD_FACE,            		//增加人脸
	DELETE_FACE,          		//删除人脸
	FACE_OPEN,           		//人脸开门
	FACE_ADMIN_CHECK,           //人脸进入菜单 
	
	TRY_OPRN_ALARM,      		//禁试报警
	PICK_OPRN_ALARM,         	//撬锁报警
	
	ADD_BLE,            		//增加蓝牙账号
	DELETE_BLE,           		//删除蓝牙账号
	BLE_OPEN,           		//蓝牙账号开门
	
	ADD_CARD,            		//增加卡
	DELETE_CARD,           		//删除卡
	CARD_OPEN,           		//卡开门
	
	ADD_SMART_KEY,            	//增加电子钥匙
	DELETE_SMART_KEY,           //删除电子钥匙
	SMART_KEY_OPEN,           	//电子钥匙开门
	
	ADD_FINGER,            		//增加指纹
	DELETE_FINGER,           	//删除指纹
	FINGER_OPEN,           		//指纹开门
    FINGER_ADMIN_CHECK,         //指纹进入菜单 
	
	ADD_VEIN,            		//增加指静脉
	DELETE_VEIN,             	//删除指静脉
	VEIN_OPEN,           		//指静脉开门
    VEIN_ADMIN_CHECK,           //指静脉进入菜单
	
	ADD_IRIS,            		//增加虹膜
	DELETE_IRIS,            	//删除虹膜
	IRIS_OPEN,           		//虹膜开门
    IRIS_ADMIN_CHECK,           //虹膜进入菜单

}LOCK_EVENT_LOG;
//==============================================================================
extern uint8_t OpenDoorTimeCnt; 
extern uint8_t LockConfigMode; 
extern SYSTEM_FIX_SETTING SystemFixSeting;  //系统固化参数，恢复出厂不清空
extern SYSTEM_SETTING SystemSeting ;//系统参数结构体

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

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <LockConfig.h>

/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define FUNCTION_ENABLE        1      //功能使能   
#define FUNCTION_DISABLE       0      //功能失能   

#define ADMIN_NONE_REGISTERED  0xA5B7 //无注册.
#define ADMIN_LOCAL_REGISTERED 0x0001 //本地管理员已注册
#define ADMIN_APP_REGISTERED   0x0002 //APP管理员已注册

#define MEM_FACT_MEM_FIG				         'W'  //有效标志
#define MEM_USER_MASTER                          'M'
#define MEM_USER_GUEST                           'G'
#define MEM_USER_ALL                             'A'

#define LOCK_BODY_212          0x55
#define LOCK_BODY_218          0x66
#define LOCK_BODY_216          0x77

#define SYSTEM_AESKEY_LEN       16

//存储结构定义
/*------------------------------------------------------------------------------------------------
#define MEM_FACTMEM_FACTORY_FIG                  0  //工厂数据起始标志  G
#define MEM_FACTMEM_LOCKMODE                     1  //开锁模式（指纹/密码蓝牙/蓝牙钥匙等）
#define MEM_FACTMEM_FINGER_APP_NUM               2  //app指纹数量
#define MEM_FACTMEM_FINGER_ADMIN_NUM		     3  //本地管理员指纹数量
#define MEM_FACTMEM_FINGER_GUEST_NUM			 4  //本地普通指纹数量
#define MEM_FACTMEM_FACE_APP_NUM                 5  //app人脸数量
#define MEM_FACTMEM_FACE_ADMIN_NUM		         6  //本地管理员人脸数量
#define MEM_FACTMEM_FACE_GUEST_NUM			     7  //本地普通人脸数量
#define MEM_FACTMEM_CARD_APP_NUM                 8  //app卡数量
#define MEM_FACTMEM_CARD_ADMIN_NUM		         9  //本地管理员卡数量
#define MEM_FACTMEM_CARD_GUEST_NUM			     10 //本地普通卡数量
#define MEM_FACTMEM_PWD_APP_NUM                  11 //app密码数量
#define MEM_FACTMEM_PWD_ADMIN_NUM		         12 //本地管理员密码数量
#define MEM_FACTMEM_PWD_GUEST_NUM			     13 //本地普通密码数量
#define MEM_FACTMEM_SMARTKEY_NUM                 14 //全部蓝牙钥匙数量H （20个）
#define MEM_FACTMEM_WIFI_MAIN_SW                 15 //WIFI主开关
#define MEM_FACTMEM_WIFI_LOGUP_SW	             16 //WIFI开门信号上传开关
#define MEM_FACTMEM_ADMIN_REGISTER_H             17 //是否注册标记H 
#define MEM_FACTMEM_ADMIN_REGISTER_L             18 //是否注册标记L
#define MEM_FACTMEM_LOCKLOG_LINK_H               19 //事件记录总次数
#define MEM_FACTMEM_LOCKLOG_LINK_L               20 //事件记录总次数
#define MEM_FACTMEM_LOCKLOG_SN_H                 21 //事件记录当前序列号H
#define MEM_FACTMEM_LOCKLOG_SN_L                 22 //事件记录当前序列号L
#define MEM_FACTMEM_VOICE                        23 //语音开关
#define MEM_FACTMEM_KEY_DEF                      24 //一键布防
#define MEM_FACTMEM_IR_DEF                       25 //接近感应设置
#define MEM_FACTMEM_WIFI_SINGLE                  26 //门铃单双向
#define MEM_FACTMEM_LOCKTIME	                 27 //自动上锁时间
#define MEM_FACTMEM_PROTECT_LOCK	             29 //锁门防止误触
#define MEM_FACTMEM_FINGER_FAC                   30 //指纹厂家，通过厂家判断加密
#define MEM_FACTMEM_FINGER_ASEKEY                31 //指纹头AES根秘钥16字节
//+16
#define MEM_FACTMEM_EEPROM_MODEL                 47 //锁具型号 8字节
//+8
#define MEM_FACTMEM_FACE_FAC                     55 //人脸厂商号
#define MEM_FACTMEM_FACE_ASEKEY                  56 //指纹头AES根秘钥16字节
//+16
#define MEM_FACTMEM_DONE_FIG                     65 //工厂标志G


// 以下地址不允许清空
#define MEM_FACTMEM_MOTOR_DIRECTION              256 // 开门方向
#define MEM_FACTMEM_MOTOR_TORQUE				 257 //电机扭力
#define MEM_FACTMEM_MOTOR_TYPE                   258 //锁体类型
--------------------------------------------------------------------------------------------------*/

//=============================地址分配固化=====================================
#define MEM_FACT_START				         0     //系统信息起始，可清空
#define MEM_FACT_SIZE				         256   //系统信息总长度

#define MEM_FIX_FACT_START				     0x100     //系统信息起始，不可清空
#define MEM_FIX_FACT_SIZE				     256   //系统信息总长度

#define MEM_PHONE_START                      0x200    //手机号起始
#define MEM_PHONE_LEN			             13        //长度6位
#define MEM_PHONE_NUM	                     1        //一组

#define MEM_BOARDPWD_START                   0x300    //密码起始
#define MEM_BOARDPWD_PWDLEN			         6        //长度6位
#define MEM_BOARDPWD_ALL                     1        //一组密码

#define MEM_BOARDSOSPWD_START                0x340    //SOS密码起始
#define MEM_BOARDSOSPWD_PWDLEN			     6        //长度6位
#define MEM_BOARDSOSPWD_ALL                  1        //一组密码

#define MEM_BOARDTEMPPWD_START               0x400    //临时密码起始
#define MEM_BOARDTEMPPWD_PWDLEN			     6        //长度6位
#define MEM_BOARDTEMPPWD_ALL                 10       //一组密码

#define MSG_FINGER_REG_START		         0x500	  //指纹注册起始地址
#define MSG_FINGER_REG_ONE_SIZE			     38		  //单指纹所用字节数量
#define MSG_FINGER_ONE_SIZE			         64		  //单指纹实际空间
#define MSG_FINGER_USER_NUM			         60       //使用数量
#define MSG_FINGER_NUM_RESERVED			 ((M_FINGER_MAX_TOTAL_NUM >= 100)?100:M_FINGER_MAX_TOTAL_NUM) 	  //100组指纹或指静脉预留
#define MSG_FINGER_ADMIN_NUM                ((M_FINGER_MAX_ADMIN_NUM >= 100)?(MSG_FINGER_NUM_RESERVED - 1):M_FINGER_MAX_ADMIN_NUM)  // 管理员用户数必须小于总数
#define MSG_FINGER_COMMON_NUM               (MSG_FINGER_NUM_RESERVED - MSG_FINGER_ADMIN_NUM)    // 普通 用户
#define MSG_FINGER_ADMIN_LOCAL_NUM          5       //本地GUI界面支持的管理员数量
#define MSG_FINGER_COMMON_LOCAL_GUINUM      15      //本地GUI界面支持的普通用户数量

#define MSG_FACE_REG_START		             (MSG_FINGER_REG_START+	MSG_FINGER_ONE_SIZE* MSG_FINGER_NUM_RESERVED)    //人脸注册起始地址0x1D00
#define MSG_FACE_REG_ONE_SIZE			     38		  //单人脸所用字节数量
#define MSG_FACE_ONE_SIZE			         64		  //单人脸实际空间
#define MSG_FACE_MASTER_NUM			         5        //管理数量
#define MSG_FACE_GUEST_NUM			         15       //普通数量
#define MSG_FACE_USER_NUM			         20       //使用数量
#define MSG_FACE_NUM_RESERVED			     100      //100组人脸或虹膜预留

#define MSG_CPU_CARD_REG_START		         (MSG_FACE_REG_START+	MSG_FACE_ONE_SIZE* MSG_FACE_NUM_RESERVED)       //卡注册起始地址0x3600
#define MSG_CPU_CARD_REG_ONE_SIZE			 58		  //单卡所用字节数量
#define MSG_CPU_CARD_ONE_SIZE			     64		  //单卡实际空间
#define MSG_CPU_CARD_USER_NUM			     30       //使用数量
#define MSG_CARD_NUM			             100      //100组卡预留

#define MSG_SMARTKEY_REG_START		         (MSG_CPU_CARD_REG_START+	MSG_CPU_CARD_ONE_SIZE* MSG_CARD_NUM)       //卡注册起始地址0x4F00
#define MSG_SMARTKEY_REG_ONE_SIZE			 14		  //单钥匙所用字节数量
#define MSG_SMARTKEY_ONE_SIZE			     64		  //单钥匙实际空间
#define MSG_SMARTKEY_NUM			         20      //20组钥匙预留

#define MSG_LOG_RECORD_START                 (MSG_SMARTKEY_REG_START+	MSG_SMARTKEY_ONE_SIZE* MSG_SMARTKEY_NUM) //事件记录硬清空固化地址0x5400   21KB起始
#define MSG_LOG_RECORD_REG_ONE_SIZE			 14		  //单事件记录所用字节数量
#define MSG_LOG_RECORD_ONE_SIZE			     16		  //单事件记录实际空间
#define MSG_LOG_RECORD_NUM			         600      //600事件记录数量预留

//=============================地址分配固化=====================================


//系统参数结构体，必须4字节对齐
typedef struct
{
    uint8_t SysFactStartFig;         //0   //工厂数据起始标志  G
	uint8_t SysLockMode;             //1   //开锁模式（指纹/密码蓝牙/蓝牙钥匙等）
    uint8_t SysFingerAllNum;         //2   //指纹总量.
    uint8_t SysFingerAdminNum;       //3   //本地管理员指纹数量
	
    uint8_t SysFingerGuestNum;       //4   //本地普通指纹数量
	uint8_t SysFaceAllNum;           //5   //人脸总数量
	uint8_t SysFaceAdminNum;	     //6   //本地管理员人脸数量
	uint8_t SysFaceGuestNum;		 //7   //本地普通人脸数量
	
	uint8_t SysCardAllNum;           //8   //卡总数量
	uint8_t SysCardAdminNum	;	     //9   //本地管理员卡数量
	uint8_t SysCardGuestNum	;		 //10  //本地普通卡数量
	uint8_t SysPwdAllNum;            //11  //密码总数量
	
	uint8_t SysPwdAdminNum;		     //12  //本地管理员密码数量
	uint8_t SysPwdGuestNum;			 //13  //本地普通密码数量
	uint8_t SysSmartKeyNum;          //14  //全部蓝牙钥匙数量h （20个）
	uint8_t SysWifiMainSw;           //15  //wifi主开关
	
	uint16_t SystemAdminRegister;    //16  //是否注册标记h
	uint8_t DoorUnlockWarmSw;        //18  //门未关报警设置  0= 关闭报警 1= 开启报警
	uint8_t FaceCheckEnable;         //19  //人脸验证开关  0x55= 开启  0x66= 关闭

	uint8_t LockBodyMode;            //20  //锁体类型  0x55= 218锁体  0x66= 212锁体 
	uint8_t FingerFlag;              //21 指纹头标记
	uint8_t SysWifiLogSw;	         //22 //wifi开门信号上传开关
	uint8_t SysVoice;                 //23  //语音开关
	
	uint8_t SysKeyDef;                //24   //一键布防
	uint8_t SysHumanIrDef;            //25   //主动防御(工作时间)   0:关闭  (1-255秒)打开   
	uint8_t SysWifiSingle;            //26   //门铃单双向     0:双向 1:单向   
	uint8_t SysAutoLockTime;	      //27   //自动上锁时间
	
	uint8_t Sysprotect_lock;	      //28   //锁门防止误触,确认键
	uint8_t SysFingerFac;             //29   //指纹厂家，通过厂家判断加密
	uint8_t SysFaceFac;               //30   //人脸厂家，通过厂家判断加密
	uint8_t SysDrawNear;              //31   //接近感应距离
	
	uint8_t SysFingerAsekey[SYSTEM_AESKEY_LEN];      //32   //指纹头aes根秘钥16字节
	uint8_t SysFaceAsekey[SYSTEM_AESKEY_LEN];        //48   //人脸AES根秘钥16字节
	
	uint8_t SysCompoundOpen;           //64 组合开门(双重认证开关)
	uint8_t SysFactDoneFig;            //65 固定标记
	uint8_t SystemVersion;             //66 版本记录
	uint8_t FingerProtocalVersion;     //67 指纹协议版本
	
	uint8_t CheckErrAllCnt;            //68 验证失败总计次
	uint8_t CheckErrPwdCnt;            //69 密码验证失败计次
	uint8_t LedRGBMode;                //氛围灯模式，对应APP_LED.h的LED_RGB_MODE_E
	uint8_t reserved71;                //71 预留
	
	uint32_t TryForbitUtc ;            //72-75 错误禁试发生时间
 
	uint16_t FaceOrIrisUnlockSuccessCnt;		//76-77人脸开锁成功次数
	uint16_t FaceOrIrisUnlockFailCnt;			//78-79人脸开锁失败次数
	
	uint16_t FingerOrVeinUnlockSuccessCnt;	//80-81指纹开锁成功次数
	uint16_t FingerOrVeinUnlockFailCnt;		//82-83指纹开锁失败次数
	
	uint8_t reserved84;                //84 预留
	uint8_t reserved85;                //85 预留
	uint8_t reserved86;                //86 预留
	uint8_t reserved87;                //87 预留
	
} SYSTEM_SETTING;

//===============================参数定义==================================

#define DOUBLE_CHECK_SW_ON      0x55     //组合开门方式开启
#define DOUBLE_CHECK_SW_OFF     0x05     //组合开门方式关闭

//开门方式，同蓝牙协议
typedef enum 
{
    LOCK_MODE_PSW = 0x01,//密码开门方式.	
	LOCK_MODE_FINGER = 0x02,//指纹开门方式.
	LOCK_MODE_BLE = 0x04,//蓝牙开门方式.
	LOCK_MODE_FACE = 0x08,//人脸开门方式.
	LOCK_MODE_CARD = 0x10,//卡开门方式
	LOCK_MODE_VEIN = 0x20,//指静脉
	LOCK_MODE_KEY = 0x40, //电子钥匙开门
	LOCK_MODE_IRIS = 0x80, //虹膜
}LOCK_MODE_ENUM;

//开门方向结构体
typedef enum 
{
	RIGHT_HAND_DOOR=0x55,
	LEFT_HAND_DOOR=0x01
}MOTOR_DIRECTION; 

//电机扭力
typedef enum 
{
	LOW_TORQUE=0x01,
	HIGH_TORQUE=0x02
}MOTOR_TORQUE; 

//音量调节
typedef enum 
{
    OFF_VOICE_VOL,       //关闭
	LOW_VOICE_VOL,     	 //低音量
	MEDIUM_VOICE_VOL, 	 //中音量
    HIGH_VOICE_VOL,		 //高音量
	
}VOL_SET_E; 

//接近感应调节
typedef enum 
{
    E_SENSE_HIGH = SENSE_HIGH_GRADE,	//远距离
	E_SENSE_LOW = SENSE_LOW_GRADE,     	//近距离
    E_SENSE_OFF = SENSE_OFF_GRADE,      //关闭
	
}NEAR_SENSE_GRADE_T; 

 
typedef struct
{
	//起始地址256
	uint8_t SysFixStartFig;                         //工厂数据起始标志  G
	uint8_t MotorDirection;						    //开门方向      
	uint8_t MotorTorque;                            //电机扭力
	uint8_t LockType;                               //锁体类型	
	uint8_t LockNameType[8];                        //锁型号
	uint16_t SysLockLogAll;                         //事件记录总次数
	uint16_t SysLockLogSn;                          //事件记录当前位置
	uint8_t SysClearCase[14];                       //硬清空事件	
	
} SYSTEM_FIX_SETTING;


//事件记录枚举
typedef enum 
{
	NOTHING_CASE, 				//唤醒无操作
	BAC_OPEN_IN_DOOR,   		//门内把手开
	BAC_CLOSE_IN_DOOR,  		//门内把手关
	FALSE_LOCK_ALARM,  			//假锁报警
	CLOSE_OUT_DOOR,      		//门外上锁
	AUTO_CLOSE_DOOR,     		//自动上锁
	KEY_OPEN_IN_DOOR,      		//门内按键开
	KEY_CLOSE_IN_DOOR,     		//门内按键关
	EMPTY_LOCK,        			//清空锁具
	TEMP_PASSWORD_OPEN,      	//临时密码开门
	NFC_OPEN,         			//NFC开门
	
	DELETE_PASSWORD,       		//密码删除
	ADD_PASSWORD,            	//增加密码
	PASSWORD_OPEN,            	//密码开门
    PWD_ADMIN_CHECK,            //密码进入菜单 
	
	DELETE_SOS_PASSWORD,       	//报警密码删除
	ADD_SOS_PASSWORD,           //增加报警密码
	SOS_PASSWORD_OPEN,          //报警密码开门
	
	ADD_FACE,            		//增加人脸
	DELETE_FACE,          		//删除人脸
	FACE_OPEN,           		//人脸开门
	FACE_ADMIN_CHECK,           //人脸进入菜单 
	
	TRY_OPRN_ALARM,      		//禁试报警
	PICK_OPRN_ALARM,         	//撬锁报警
	
	ADD_BLE,            		//增加蓝牙账号
	DELETE_BLE,           		//删除蓝牙账号
	BLE_OPEN,           		//蓝牙账号开门
	
	ADD_CARD,            		//增加卡
	DELETE_CARD,           		//删除卡
	CARD_OPEN,           		//卡开门
	
	ADD_SMART_KEY,            	//增加电子钥匙
	DELETE_SMART_KEY,           //删除电子钥匙
	SMART_KEY_OPEN,           	//电子钥匙开门
	
	ADD_FINGER,            		//增加指纹
	DELETE_FINGER,           	//删除指纹
	FINGER_OPEN,           		//指纹开门
    FINGER_ADMIN_CHECK,         //指纹进入菜单 
	
	ADD_VEIN,            		//增加指静脉
	DELETE_VEIN,             	//删除指静脉
	VEIN_OPEN,           		//指静脉开门
    VEIN_ADMIN_CHECK,           //指静脉进入菜单
	
	ADD_IRIS,            		//增加虹膜
	DELETE_IRIS,            	//删除虹膜
	IRIS_OPEN,           		//虹膜开门
    IRIS_ADMIN_CHECK,           //虹膜进入菜单

}LOCK_EVENT_LOG;
//==============================================================================
extern uint8_t OpenDoorTimeCnt; 
extern uint8_t LockConfigMode; 
extern SYSTEM_FIX_SETTING SystemFixSeting;  //系统固化参数，恢复出厂不清空
extern SYSTEM_SETTING SystemSeting ;//系统参数结构体

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




