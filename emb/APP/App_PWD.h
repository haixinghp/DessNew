/*********************************************************************************************************************
 * @file:      App_PWD.h
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-09
 * @brief:     ���빦�ܺ���  
 * @Description:   
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_PWD_H
#define  _APP_PWD_H

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h> 
#include "System.h" 
/*--------------------------------------------------�궨��-----------------------------------------------------------*/
#define  MEM_PWD_VALID_FLG          MEM_FACT_MEM_FIG       //�����Ƿ���Ч�ж�����  'W'��Ч
#define  MSG_PWD_BYTE_SIZE          MEM_BOARDPWD_PWDLEN    //���볤��

#define  PWD_LIMIT_ADMIN            MEM_USER_MASTER
#define  PWD_LIMIT_GUEST            MEM_USER_GUEST
#define  PWD_LIMIT_ALL              MEM_USER_ALL

#define  TEMP_PWD_ALL_NUM           MEM_BOARDTEMPPWD_ALL         //��ʱ�����ܸ���
#define  TEMP_PWD_BYTE_SIZE         MEM_BOARDTEMPPWD_PWDLEN      //��ʱ���볤��
#define  SOS_PWD_ALL_NUM            MEM_BOARDSOSPWD_ALL          //���������ܸ���
#define  SOS_PWD_BYTE_SIZE          MEM_BOARDSOSPWD_PWDLEN       //�������볤��

#define  PWD_USER_ID            1               //����Ա�����û�ID
#define  PWD_TEMP_ID            2               //��ʱ�����û�ID
#define  PWD_SOS_ID             3               //���������û�ID
#define  WEEKDAY_VALUE        	0x7F            //����1---����
#define  UTC_START_TIME       	1609459200      //2021/01/01 :00:00:00
#define  UTC_STOP_TIME        	4070995199      //2099/01/01 :23:59:59
#define  UTC_STOP_YEAR         	0x99            //2099 
#define  UTC_STOP_HOUR      	0x23            //23ʱ 
#define  UTC_STOP_MINU      	0x59            //59���� 
#define  UTC_STOP_SECOND    	0x59            //59��
/*--------------------------------------------------ö������---------------------------------------------------------*/
 
 
/*--------------------------------------------------��������---------------------------------------------------------*/


/*--------------------------------------------------��������---------------------------------------------------------*/             
typedef struct
{
    uint8_t   UserValue;       //�û��Ƿ���Ч  	'W'-���û�   0xff-���û�
	uint8_t   Privileges;      //�û�Ȩ��      	'M'-����Ա   'G'-��ͨ�û� 'C'-�����û�
	uint8_t   Attribute;       //�û�����  		'T'-��ʱ     'W'-����     'O'-һ��������
	uint8_t   Timeliness;      //ʱЧ״̬  		 0-�ر�  1-����  
	uint16_t  UserId;          //�û����
	uint8_t   Weekday;         //����
	uint8_t   Unknow;          //Ԥ��
	uint8_t   Unknow1;         //Ԥ��1
	
    uint8_t   Password[ MSG_PWD_BYTE_SIZE+1 ];     //�û����루�ַ�����

	uint32_t  StartTim;        //��Чʱ�� UTC
	uint32_t  StopTim;         //ʧЧʱ�� UTC	 
	
}PwdMeg_T;	  //��������
 
typedef struct
{
    uint8_t   UserValue;       //�û��Ƿ���Ч  	'W'-���û�   0xff-���û�
    uint8_t   Password[ TEMP_PWD_BYTE_SIZE+1 ];     //�û����루�ַ�����
 
}TmpPwdMeg_T;  //��ʱ����
 

typedef struct
{
    uint8_t   UserValue;       //�û��Ƿ���Ч  	'W'-���û�   0xff-���û�
    uint8_t   Password[ SOS_PWD_BYTE_SIZE+1 ];     //�û����루�ַ�����
 
}SosPwdMeg_T;  //��������
 
/*--------------------------------------------------��������---------------------------------------------------------*/
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






