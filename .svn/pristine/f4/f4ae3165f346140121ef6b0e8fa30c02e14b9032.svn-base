/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: APP_Update.c 
* 作者：范淑毓
* 版本：V01
* 时间：20220105
* 内容简述：业务通用升级功能(小嘀协议接收，I2C推送模组)
****************************************************************************/
/* 标准头文件 */
#include <stdint.h>   

/* 内部头文件 */

/* 外部头文件 */

#ifndef _APP_UPDATE_H_
#define _APP_UPDATE_H_

#define APP_UPDATE_PACK_CACHE_CNT   16               // 缓存数量
#define APP_UPDATE_PACK_CACHE_LEN   512             // 单包缓存大小
#define APP_UPDATE_PACK_MAX (APP_UPDATE_PACK_CACHE_CNT*APP_UPDATE_PACK_CACHE_LEN)   // 缓存最大大小


/* 通用OTA 升级类型枚举 */
typedef enum
{
    EM_APP_UPDATE_NONE = 0,         //无效流程，默认值
    EM_APP_UPDATE_TYPE_VOICE,       // 语音芯片OTA
    EM_APP_UPDATE_TYPE_SCREEN,      // 智能屏主程序 OTA
    EM_APP_UPDATE_TYPE_EMOJI,       // 智能屏表情包 OTA
    /* 总流程数量 */
    EM_APP_UPDATE_ALL,              //主流程上限，即最大值
}APP_UPDATE_TYPE_E; 


typedef struct
{
	uint8_t     Enabled;
	uint8_t     Step;
	uint8_t     I2cSpeed;
    uint8_t     SinglePackageSize;  //A15: 内容包最长长度
	uint32_t    Package;
	
	uint32_t    FileSize;
	uint32_t    offset;
	uint32_t    FileChack;          // A7/A8/A9/A10: 固件校验和
	uint32_t    RxChack;            // A11/A12/A13/A14: 固件校验和
	
	uint16_t    stage;              // A16/A17: 阶段包
	uint16_t    BinPos;

    APP_UPDATE_TYPE_E emUpdateType;
    
	uint8_t     Bin[APP_UPDATE_PACK_CACHE_LEN];
}MODULE_UPDATE_T;

uint32_t APP_UpdateGetWorkCnt(void);
uint32_t APP_UpdateWorkCntCountDown(void);
void APP_UpdateWorkCntReset(void);

void APP_UpdateDataWrite(void);
bool APP_UpdateSetType(APP_UPDATE_TYPE_E _emUpdateType);
uint8_t APP_UpdateDataHandler(uint8_t* _pu8BleData, uint8_t _u8DataLen);

#endif

