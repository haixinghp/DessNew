/*********************************************************************************************************************
 * @file:      App_CpuCard.h
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-11-12
 * @brief:     cpu功能应用程头文件
**********************************************************************************************************************/
  
#ifndef _APP_CPU_CARD_H_
#define _APP_CPU_CARD_H_

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include "stdint.h"
#include "System.h"

#include "../HAL/HAL_RTC/HAL_RTC.h"
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
/****************************/
#define CARD_ID     1 	
#define USER_ID     2 
#define DF_EXTERN_KEY_SIZE   8   //外部认证密钥长度 
#define DF_INSIDE_KEY_SIZE   8   //内部认证密钥长度 

/*--------------------------------------------------枚举声明---------------------------------------------------------*/
typedef enum
{
    eAPDUSuccess =0,
    eAPDUMatchFail,
    eAPDUFailSelectMF,
    eAPDUFailGetRandom,
    eAPDUFailGetExternalAuthentication,
    eAPDUFailEmptyCard,
    eAPDUFailCreateMFKeyFile,
    eAPDUFailAddMFKeyFilePwd,
    eAPDUFailCreateDF,
    eAPDUFailSelectDF,
    eAPDUFailCreateDFKeyFile,
    eAPDUFailAddDFExternalKey,
    eAPDUFailAddDFInsideKey,
    eAPDUFailCreateBin,
    eAPDUFailSelectBin,
    eAPDUFailWriteBin,
    eAPDUFailGetInsideAuthentication,
    eAPDUFailReadBin,
	
} TYPEe_XX; 
 

typedef enum
{
    CPUCARD_ADD_SUCCESSFUL, // 添加成功
    CPUCARD_ADD_REGISTERED, // 已注册
    CPUCARD_ADD_ERROR,      // 添加失败
    CPUCARD_FINDING_ERROR,  // 寻卡失败
}CPUCARD_ADD_USER_E; //注册过程


/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
#pragma pack(1)
typedef union 
{
	uint8_t tab[ MSG_CPU_CARD_REG_ONE_SIZE ];
	struct
	{
		uint8_t  UserValue;        //用户权限  'W'-有用户   0xff-无用户
		uint8_t  Privileges;       //用户权限  'M'-管理员  'G'-普通用户 'C'-客人用户
		uint8_t  TimelinessState;  //时效状态  0-关闭  1-开启
		uint8_t  Abducted;         //挟持状态
		uint8_t  unknow;           //预留  

		uint16_t UserId;           //用户编号   
		uint16_t PageId;       

		uint8_t  ExternAuthenPublicKey[DF_EXTERN_KEY_SIZE];//外部验签密钥
		uint8_t  InnerAuthenPublicKey[DF_INSIDE_KEY_SIZE]; //内部验签密钥

		uint32_t CardId;           //cpu卡ID 
        
        struct {
            uint8_t  fig;	
            RTCType start; //起始时间
            RTCType stop;  //结束时间
            uint8_t  wday;		//周周时效 BIT1 -BIT 7表示周一到周日
        }tm_vaild;			//时效
            
	}data;
	 
}CARD_MEG_Def;  //cpu卡信息
#pragma pack()

/*--------------------------------------------------函数声明---------------------------------------------------------*/
uint8_t CpuCard_QueryUserCpuCardMegFromEeprom( uint8_t type, uint32_t numId, uint32_t *paddr, CARD_MEG_Def *pCardMeg );
uint8_t CpuCardEnrollPro(uint8_t CpuCardLim, uint16_t UserID);
uint8_t CpuCardGetVeifyState(uint16_t *Pageid);
void CpuCardEepromEmpty (void);
uint8_t CpuCardDeleteID(uint16_t UserID);
uint8_t CpuCardDeleteComparison(void);

#endif

