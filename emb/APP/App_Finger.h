/*********************************************************************************************************************
 * @file:        App_Finger.h
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-07-26
 * @Description: 指纹/指静脉业务模块
 * @ChangeList:  01. 初版
**********************************************************************************************************************/
  
#ifndef  _APP_FINGER_H_
#define  _APP_FINGER_H_

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "LockConfig.h"
#include "System.h"
#include "../SERVER/Finger.h"

/*--------------------------------------------------宏定义-----------------------------------------------------------*/
/* 指纹模组缓存区号 */
#define FINGER_BUFFER_NUM_1 0x01
#define FINGER_BUFFER_NUM_2 0x02
#define FINGER_BUFFER_NUM_3 0x03
#define FINGER_BUFFER_NUM_4 0x04
#define FINGER_BUFFER_NUM_5 0x05
#define FINGER_BUFFER_NUM_6 0x06

/* 业务流程- 调用指纹模组协议的最大上限 */
#define FINGER_APP_FLOW_MAX 16

/* 业务流程 - 第N次解析特征到对应缓存区 */
#define FINGER_APP_CONVERT_1 1
#define FINGER_APP_CONVERT_2 2
#define FINGER_APP_CONVERT_3 3
#define FINGER_APP_CONVERT_4 4
#define FINGER_APP_CONVERT_5 5
#define FINGER_APP_CONVERT_6 6

/* 指纹模组在EE 上分配的起始地址 */
#ifndef MSG_FINGER_REG_START
#define FINGER_APP_CFG_REG_START 0x400
#else
#define FINGER_APP_CFG_REG_START MSG_FINGER_REG_START
#endif

/* 指纹模组单个指纹配置的最大区块大小  */
#ifndef MSG_FINGER_ONE_SIZE
#define FINGER_APP_CFG_ONE_SIZE 64
#else
#define FINGER_APP_CFG_ONE_SIZE MSG_FINGER_ONE_SIZE
#endif

/* 不包含校验码的有效配置位数 */
#define FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK (FINGER_APP_CFG_ONE_SIZE - 2)

#define FINGER_APP_CFG_EN       'W'     // 指纹使用使能标记
#define FINGER_APP_CFG_MANAGER  'M'     // 管理员用户标记
#define FINGER_APP_CFG_GUESS    'G'     // 普通用户标记


/*--------------------------------------------------枚举声明---------------------------------------------------------*/

/* 指纹模组业务功能流程 */
typedef enum
{
    EM_FINGER_APP_FLOW0_CLEAR = 0,      // 清空指纹
    EM_FINGER_APP_FLOW1_ADD,            // 增加指纹
    EM_FINGER_APP_FLOW2_DEL,            // 删除指纹
    EM_FINGER_APP_FLOW3_SEARCH,         // 检索匹配
    EM_FINGER_APP_FLOW4_CLEAR_COMMON,   // 清空普通用户
    EM_FINGER_APP_FLOW5_SLIDE_ADD,      // 滑动添加指静脉
    EM_FINGER_APP_FLOW6_AGING,          // 指静脉老化亮灯
    EM_FINGER_APP_FLOW7_68_SLEEP,       // 0x68指纹模组休眠流程
    EM_FINGER_APP_FLOW8_CLEAR_V2,       // 清空指纹(增加0x60 方式,对应 FINGER_PROTOCAL_V3)
    EM_FINGER_APP_FLOW9_DEL_CUSTOM,     // 删除指纹(通过customID 删除手指)
    EM_FINGER_APP_FLOW_ALL,             // 主流程上限，即最大值

    EM_FINGER_APP_FLOW_NONE = 0xFF,     // 无效流程，默认值
}FINGER_APP_FLOW_TYPE_E; 

/* 指纹业务流程状态执行结果 */
typedef enum
{
    FINGER_APP_RESULT_IDLE = 0,         // 空闲
    FINGER_APP_RESULT_RUNNING,          // 运行中    
    
    FINGER_APP_RESULT_SUC,              // 执行成功
    FINGER_APP_RESULT_FAIL,             // 执行失败
    FINGER_APP_RESULT_TIMEOUT,          // 超时
    FINGER_APP_PROTOCAL_ERR,            // 协议解析异常
    FINGER_APP_RESULT_ALL,              //流程最大值
}FINGER_APP_FLOW_RESULT_E; 

/* 指纹业务录入流程状态枚举 */
typedef enum
{
    FINGER_APP_ADD_NONE = 0,            // 无状态
    FINGER_APP_ADD_1ST,                 // 第一次录入
    FINGER_APP_ADD_2ND,                 // 第二次录入
    FINGER_APP_ADD_3RD,                 // 第三次录入
    FINGER_APP_ADD_4TH,                 // 第四次录入
    FINGER_APP_ADD_5TH,                 // 第五次录入
    FINGER_APP_ADD_6TH,                 // 第六次录入
    FINGER_APP_ADD_OVER,                // 模组存放指纹
    FINGER_APP_ADD_ALL,                 //流程最大值
}FINGER_APP_ADD_FLOW_E; 

/* 指纹配置类型 */
typedef enum
{
    EM_FINGER_APP_TYPE_ADMIN = 0,       // 管理员用户
    EM_FINGER_APP_TYPE_COMMON,          // 普通用户
    EM_FINGER_APP_TYPE_ALL              // 所有用户
}FINGER_APP_USER_TYPE_E;


/* 指纹模组单个配置内存分配 

0xX00                       0xX01                       0xX02                       0xX03                       0xX04                       0xX05                       0xX06                       0xX07
指纹使能        模组编号H       模组编号L       管理员标记      亲情标记        SOS胁迫标记     时效标记                周
0xX08                       0xX09                       0xX0A                       0xX0B                      0xX0C                       0xX0D                       0xX0E                       0xX0F
开始年                  月                          日                          时                          分                          秒                          结束年                  月

0xX10                       0xX11                       0xX12                       0xX13                       0xX14                       0xX15                       0xX16                       0xX17
日                             时                          分                          秒                         预留                      预留                    预留                     预留
0xX18                       0xX19                       0xX1A                       0xX1B                      0xX1C                       0xX1D                       0xX1E                       0xX1F
预留                      预留                     预留                    预留                     预留                     预留                       预留                    预留

0xX20                       0xX21                       0xX22                       0xX23                       0xX24                       0xX25                       0xX26                       0xX27
预留                      预留                     预留                    预留                     预留                     预留                       预留                    预留
0xX28                       0xX29                       0xX2A                       0xX2B                      0xX2C                       0xX2D                       0xX2E                       0xX2F
预留                      预留                     预留                    预留                     预留                     预留                       预留                    预留

0xX30                       0xX31                       0xX32                       0xX33                       0xX34                       0xX35                       0xX36                       0xX37
预留                      预留                     预留                    预留                     预留                     预留                       预留                    预留
0xX38                       0xX39                       0xX3A                       0xX3B                      0xX3C                       0xX3D                       0xX3E                       0xX3F
预留                      预留                     预留                    预留                     预留                     预留                    校验位H            校验位L


*/
/* 指纹模组单个配置内存分配 */
typedef enum
{
    EM_FINGER_APP_CFG_EN = 0x00,            // 指纹使能
    EM_FINGER_APP_CFG_NUM_H,                // 模组编号H
    EM_FINGER_APP_CFG_NUM_L,                // 模组编号L
    EM_FINGER_APP_CFG_ADMIN_EN,             // 管理员标记
    EM_FINGER_APP_CFG_FAMILY_EN,            // 亲情标记
    EM_FINGER_APP_CFG_SOS_EN,               // SOS胁迫标记 
    EM_FINGER_APP_CFG_TIME_EN,              // 时效标记  
    EM_FINGER_APP_CFG_WEEK,                 // 周

    
    EM_FINGER_APP_CFG_START_YEAR = 0x08,    // 开始时间: 年
    EM_FINGER_APP_CFG_START_MONTH,          // 开始时间: 月
    EM_FINGER_APP_CFG_START_DAY,            // 开始时间: 日
    EM_FINGER_APP_CFG_START_HOUR,           // 开始时间: 时
    EM_FINGER_APP_CFG_START_MIN,            // 开始时间: 分
    EM_FINGER_APP_CFG_START_SEC,            // 开始时间: 秒
    EM_FINGER_APP_CFG_END_YEAR,             // 结束时间: 年
    EM_FINGER_APP_CFG_END_MONTH,            // 结束时间: 月
    
    EM_FINGER_APP_CFG_END_DAY = 0x10,       // 结束时间: 日
    EM_FINGER_APP_CFG_END_HOUR,             // 结束时间: 时
    EM_FINGER_APP_CFG_END_MIN,              // 结束时间: 分
    EM_FINGER_APP_CFG_END_SEC,              // 结束时间: 秒
    EM_FINGER_APP_CFG_CUSTOM_ID_H,          // 用户自定义ID 高位H
    EM_FINGER_APP_CFG_CUSTOM_ID_L,          // 用户自定义ID 低位L
    
    EM_FINGER_APP_CFG_VALID_MAX,            // 有效最大值
    
    EM_FINGER_APP_CFG_CHECK_H = FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK,      // 校验和H
    EM_FINGER_APP_CFG_CHECK_L = FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK+1,    // 校验和L

}FINGER_APP_CFG_OFFSET_E; 

/*--------------------------------------------------变量声明---------------------------------------------------------*/

/* 指纹模组单个配置分配 */
typedef struct
{
    uint8_t  acOffset[FINGER_APP_CFG_ONE_SIZE];  // 指纹单个配置大小，offset具体偏移根据枚举FINGER_APP_CFG_OFFSET_E确定
}FingerAppCfg_S; 

/* 指纹模组业务功能流程及入参结构体 */
typedef struct
{
    /* 入参 */
    FINGER_APP_FLOW_TYPE_E  emAppFlow;          // 记录当前业务流程运转值
    FingerAppCfg_S          stFingerAppCfg;     // 录入、修改指纹配置赋值传入
    
    /* 出参 */
    FINGER_APP_FLOW_RESULT_E    emFlowResult;   // 运行结果,作为入参无效
    uint16_t                    u16OutPad;      // 操作出参，目前对外支持入参可配的仅ADD和DEL指纹，用于传出模板号
    
}FingerAppParam_S; 

/*--------------------------------------------------函数声明---------------------------------------------------------*/

/* 业务接口 */
void APP_FINGER_Init(void);
void APP_FINGER_Sleep(void);
bool APP_FINGER_Operate(FingerAppParam_S _stFingerAppParam);
FINGER_APP_FLOW_RESULT_E APP_FINGER_GetFlowResult(uint16_t *pu16Pad);
bool APP_FINGER_GetFingerID(uint8_t _u8Len, uint8_t *_pu8ID, uint8_t *_pu8Len);
uint8_t APP_FINGER_GetPowerFlag(void);
uint8_t APP_FINGER_GetProtocalVersion(void);
FINGER_APP_ADD_FLOW_E APP_FINGER_GetAddFingerFlow(void);
bool APP_FINGER_Get_Breakdown(void);
void APP_FINGER_Scan(void);

/* 业务:  指纹业务延时操作接口*/
void App_FINGER_Tim10Ms(void);

/*  配置业务:  操作接口*/
bool APP_FINGER_CfgWrite(FingerAppCfg_S _stCfg);
bool APP_FINGER_CfgRead(uint16_t _u16Num, FingerAppCfg_S *_pstCfg);
bool APP_FINGER_CfgGetNullNum(FINGER_APP_USER_TYPE_E _em, uint16_t* _pu16Num);
bool APP_FINGER_CfgCheck_CustomID(uint16_t _u16CustomID);


#endif




