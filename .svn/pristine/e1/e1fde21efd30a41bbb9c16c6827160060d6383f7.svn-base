/*********************************************************************************************************************
 * @file:      App_PWD.h
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-09
 * @brief:     密码功能函数  
 * @Description:   
 * @ChangeList:  01. 初版
**********************************************************************************************************************/
  
#ifndef  _APP_PWD_H
#define  _APP_PWD_H

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h> 
#include "System.h" 
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#define  MEM_PWD_VALID_FLG          MEM_FACT_MEM_FIG       //密码是否有效判断依据  'W'有效
#define  MSG_PWD_BYTE_SIZE          MEM_BOARDPWD_PWDLEN    //密码长度

#define  PWD_LIMIT_ADMIN            MEM_USER_MASTER
#define  PWD_LIMIT_GUEST            MEM_USER_GUEST
#define  PWD_LIMIT_ALL              MEM_USER_ALL

#define  TEMP_PWD_ALL_NUM           MEM_BOARDTEMPPWD_ALL         //临时密码总个数
#define  TEMP_PWD_BYTE_SIZE         MEM_BOARDTEMPPWD_PWDLEN      //临时密码长度
#define  SOS_PWD_ALL_NUM            MEM_BOARDSOSPWD_ALL          //报警密码总个数
#define  SOS_PWD_BYTE_SIZE          MEM_BOARDSOSPWD_PWDLEN       //报警密码长度

#define  PWD_USER_ID            1               //管理员密码用户ID
#define  PWD_TEMP_ID            2               //临时密码用户ID
#define  PWD_SOS_ID             3               //报警密码用户ID
#define  WEEKDAY_VALUE        	0x7F            //周期1---周日
#define  UTC_START_TIME       	1609459200      //2021/01/01 :00:00:00
#define  UTC_STOP_TIME        	4070995199      //2099/01/01 :23:59:59
#define  UTC_STOP_YEAR         	0x99            //2099 
#define  UTC_STOP_HOUR      	0x23            //23时 
#define  UTC_STOP_MINU      	0x59            //59分钟 
#define  UTC_STOP_SECOND    	0x59            //59秒
/*--------------------------------------------------枚举声明---------------------------------------------------------*/
 
 
/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
typedef struct
{
    uint8_t   UserValue;       //用户是否有效  	'W'-有用户   0xff-无用户
	uint8_t   Privileges;      //用户权限      	'M'-管理员   'G'-普通用户 'C'-客人用户
	uint8_t   Attribute;       //用户属性  		'T'-临时     'W'-报警     'O'-一次性密码
	uint8_t   Timeliness;      //时效状态  		 0-关闭  1-开启  
	uint16_t  UserId;          //用户编号
	uint8_t   Weekday;         //星期
	uint8_t   Unknow;          //预留
	uint8_t   Unknow1;         //预留1
	
    uint8_t   Password[ MSG_PWD_BYTE_SIZE+1 ];     //用户密码（字符串）

	uint32_t  StartTim;        //生效时间 UTC
	uint32_t  StopTim;         //失效时间 UTC	 
	
}PwdMeg_T;	  //开锁密码
 
typedef struct
{
    uint8_t   UserValue;       //用户是否有效  	'W'-有用户   0xff-无用户
    uint8_t   Password[ TEMP_PWD_BYTE_SIZE+1 ];     //用户密码（字符串）
 
}TmpPwdMeg_T;  //临时密码
 

typedef struct
{
    uint8_t   UserValue;       //用户是否有效  	'W'-有用户   0xff-无用户
    uint8_t   Password[ SOS_PWD_BYTE_SIZE+1 ];     //用户密码（字符串）
 
}SosPwdMeg_T;  //报警密码
 
/*--------------------------------------------------函数声明---------------------------------------------------------*/
void App_PWD_FileInit( void );  
void App_PWD_Tim10Ms( void ); 
void App_PWD_ClearAllPwdMegFromEeprom( void );
void App_PWD_SaveOnePwdMegIntoEeprom( PwdMeg_T *ppwdmeg );

uint8_t App_PWD_QueryPwdByStringFromEeprom( uint8_t userType, PwdMeg_T *ppwdmeg, char *pdata );
uint8_t App_PWD_QueryByIdFromEeprom( uint16_t pwdId, uint32_t *paddr, PwdMeg_T *ppwdmeg );   
uint8_t App_PWD_DelPwdIdFromEeprom( uint16_t pwdId );
uint8_t App_PWD_ExchangePwdMegIntoEeprom( PwdMeg_T *ppwdmeg );
int8_t  App_PWD_VerifyUserPwd( uint8_t userType, PwdMeg_T *ppwdmeg, char *pdata );

void App_PWD_CreateTempPwds( TmpPwdMeg_T *pTmPwdmeg );
void App_PWD_SaveTempPwdsIntoEeprom( TmpPwdMeg_T *pTmPwdmeg );
void App_PWD_ClearAllTmpPwdsFromEeprom( void );
int8_t App_PWD_VerifyTempPwd( char *pdata );

void App_PWD_SaveSosPwdIntoEeprom( SosPwdMeg_T *pSosPwdmeg );
void App_PWD_ClearAllSosPwdsFromEeprom( void );
int8_t App_PWD_VerifySosPwd( char *pdata );


#endif
/*--------------------------------------------------THE FILE END-----------------------------------------------------*/






