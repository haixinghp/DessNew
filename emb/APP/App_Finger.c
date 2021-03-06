/*********************************************************************************************************************
 * @file:        App_Finger.c
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-07-26
 * @Description: 指纹/指静脉业务模块
 * @ChangeList:  01. 初版
*/
/* 标准头文件 */


/* 内部头文件 */
#include "App_Finger.h"


/* 外部头文件 */
#include "Public.h"
#include "App_LED.h" 
#include "../HAL/HAL_EXPORT/HAL_EXPORT.h"
#include "../HAL/HAL_Voice/HAL_Voice.h"
#include "../HAL/HAL_RTC/HAL_RTC.h"
#include "../HAL/HAL_EEPROM/HAL_EEPROM.h"


/*********************** 外部声明变量和函数************************/
/**************    为了减少耦合，后续逐步去除外部声音**********/


/*************************  0 内部使用宏定义*************************/
/*  配置- EE 出厂参数每一位即为0xFF  */
#define FINGER_APP_CFG_RESET_VALUE 0xFF
#define FINGER_APP_CFG_RESET_NULL 0x00


/*****************************  1 静态变量*****************************/
/* 业务功能状态维护变量 */
static FingerAppParam_S s_stFingerAppParam = {EM_FINGER_APP_FLOW_NONE, 0};
static uint8_t s_u8AppFlowOrderFlag  = 0;       // 记录当前业务流程运转值对应的指令运转值，对应s_u8AppFlowMap
static uint32_t s_u32AppWorkCnt = 0;              // 交互最长时间
static uint32_t s_u32GetImageCnt = 0;             // 获取图片协议尝试计数值
static bool isPutUpFinger = false;
#ifdef FINGER_PROTOCAL_V2_SUPPORT
static uint8_t s_u8FingerProtocalV2Cnt = 0;     // V2版本 支持优化的，允许尝试3次
#endif

/* 业务流程轮转状态 */
static const uint8_t s_u8AppFlowMap[EM_FINGER_APP_FLOW_ALL][FINGER_APP_FLOW_MAX+1] = 
{
    /* 清空配置流程 */
    {EM_FINGER_APP_FLOW0_CLEAR,\
    EM_FINGER_FLOW5_EMPTY,      EM_FINGER_FLOW7_AESKEY,     EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* 增加指纹流程 */
    {EM_FINGER_APP_FLOW1_ADD,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,\
    EM_FINGER_FLOW6_MERGE,      EM_FINGER_FLOW3_REG,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* 删除指纹流程 */
    {EM_FINGER_APP_FLOW2_DEL,\
    EM_FINGER_FLOW4_DEL,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* 检索指纹流程 */
    {EM_FINGER_APP_FLOW3_SEARCH,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW2_SEARCH,     EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* 清空普通用户 */
    {EM_FINGER_APP_FLOW4_CLEAR_COMMON,\
    EM_FINGER_FLOW4_DEL,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
        
    /* 指静脉录入 */
    {EM_FINGER_APP_FLOW5_SLIDE_ADD,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW9_SLIDE,      EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW9_SLIDE,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW9_SLIDE,      EM_FINGER_FLOW3_REG,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* 老化亮灯 */
    {EM_FINGER_APP_FLOW6_AGING,\
    EM_FINGER_FLOW10_AGING,     EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},

    /* 0x68 特殊睡眠流程 */
    {EM_FINGER_APP_FLOW7_68_SLEEP,\
    EM_FINGER_FLOW_EX0_GET,     EM_FINGER_FLOW_EX1_SLEEP,   EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
        
    /* 清空配置流程(增加0x60 方式) */
    {EM_FINGER_APP_FLOW8_CLEAR_V2,\
    EM_FINGER_FLOW5_EMPTY,      EM_FINGER_FLOW11_AESKEY_V2, EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* 删除指纹流程(通过customID 删除手指) */
    {EM_FINGER_APP_FLOW9_DEL_CUSTOM,\
    EM_FINGER_FLOW4_DEL,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
};



/***************************** 2  静态函数*****************************/

/****************************************************************************** 
* 函数名：app_finger_cfg_reset
* 功 能：指纹业务复位/ 恢复
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_cfg_reset(void)
{
    /* 将EE 恢复 */
    FingerAppCfg_S stFingerAppCfg = {FINGER_APP_CFG_RESET_NULL}; 
    uint8_t cnt = 0;
    for(uint8_t i = 0; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        if(HAL_EEPROM_WriteBytes(FINGER_APP_CFG_REG_START+i*MSG_FINGER_ONE_SIZE, (uint8_t *)&stFingerAppCfg, MSG_FINGER_ONE_SIZE) > 0)
        {
            cnt++;
            SystemSeting.SysFingerAllNum = 0;
            SystemSeting.SysFingerAdminNum = 0;
            SystemSeting.SysFingerGuestNum = 0;
            (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAllNum, sizeof(SystemSeting.SysFingerAllNum) );
            (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAdminNum, sizeof(SystemSeting.SysFingerAdminNum) );
            (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerGuestNum, sizeof(SystemSeting.SysFingerGuestNum) );
        }
    }
    return (MSG_FINGER_NUM_RESERVED == cnt)?true:false;
}

/****************************************************************************** 
* 函数名：app_finger_cfg_reset
* 功 能：指纹普通用户指纹复位/ 恢复
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_cfg_reset_common(void)
{
    /* 将EE 恢复 */
    FingerAppCfg_S stFingerAppCfg = {FINGER_APP_CFG_RESET_NULL}; 
    uint8_t cnt = 0;
    for(uint8_t i = MSG_FINGER_ADMIN_NUM; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        if(HAL_EEPROM_WriteBytes(FINGER_APP_CFG_REG_START + i*MSG_FINGER_ONE_SIZE, (uint8_t *)&stFingerAppCfg, MSG_FINGER_ONE_SIZE) > 0)
        {
            cnt++;
            SystemSeting.SysFingerAllNum = SystemSeting.SysFingerAdminNum;
            SystemSeting.SysFingerGuestNum = 0;
            (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAllNum, sizeof(SystemSeting.SysFingerAllNum) );
            (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerGuestNum, sizeof(SystemSeting.SysFingerGuestNum) );
        }
    }
    return ((MSG_FINGER_NUM_RESERVED - MSG_FINGER_ADMIN_NUM) == cnt)?true:false;
}

/****************************************************************************** 
* 函数名：app_finger_cfg_get_nullnum
* 功 能：指纹业务配置，得到相应区域未使用的空的序号
* 输 入：void
* 输 出：uint16_t* _pu16Num 返回空的序号
* 返 回：bool
*/
static bool app_finger_cfg_get_nullnum(FINGER_APP_USER_TYPE_E _em, uint16_t* _pu16Num)
{
    FingerAppCfg_S stCfg;
    *_pu16Num = 0;
    uint8_t i = (EM_FINGER_APP_TYPE_COMMON == _em && ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister)? M_FINGER_MAX_ADMIN_NUM : 0;
    for(; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        /* 管理员仅支持最多M_FINGER_MAX_ADMIN_NUM 个 */
        if(EM_FINGER_APP_TYPE_ADMIN == _em && M_FINGER_MAX_ADMIN_NUM == i) { break;}
        /* 兼容0xFF和0x00 两种情况  */
        memset((uint8_t *)&stCfg, 0, sizeof(stCfg));
        if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+i*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0)
        {
            if(FINGER_APP_CFG_RESET_VALUE == stCfg.acOffset[EM_FINGER_APP_CFG_EN]
                || FINGER_APP_CFG_RESET_NULL == stCfg.acOffset[EM_FINGER_APP_CFG_EN])
            {
                *_pu16Num = i;
                my_printf("# FINGER # the No.%d is null \n", i);
                return true;
            }
        }
    }
    
    my_printf("# FINGER # all is full \n", i);
    return false;
}


/****************************************************************************** 
* 函数名：app_finger_cfg_write
* 功 能：指纹业务配置，写入相应指纹配置区域
* 输 入：FingerAppCfg_S _stCfg
* 输 出：void
* 返 回：bool
*/
static bool app_finger_cfg_write(FingerAppCfg_S _stCfg)
{
    /* 序号与配置数组号一致，先获取到序号 */
    uint16_t u16Num = (_stCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] << 8) + _stCfg.acOffset[EM_FINGER_APP_CFG_NUM_L];

    /* 判定序号的有效性 */
    if(u16Num >= MSG_FINGER_NUM_RESERVED) {return false;}
    
    /* 计算校验和 */
    uint16_t u16CheckSum = 0;
    for(uint8_t i = 0; i < FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK; i++) {u16CheckSum += _stCfg.acOffset[i];}
    _stCfg.acOffset[EM_FINGER_APP_CFG_CHECK_H] = (uint8_t)(u16CheckSum>>8);
    _stCfg.acOffset[EM_FINGER_APP_CFG_CHECK_L] = (uint8_t)(u16CheckSum);
    _stCfg.acOffset[EM_FINGER_APP_CFG_EN] = FINGER_APP_CFG_EN;
    
    FingerAppCfg_S stCfg = {0};
    if((HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0))
    {
        /* 如果是空的写入 */
        if(FINGER_APP_CFG_RESET_VALUE == stCfg.acOffset[EM_FINGER_APP_CFG_EN]
            || FINGER_APP_CFG_RESET_NULL == stCfg.acOffset[EM_FINGER_APP_CFG_EN])
        {
            if(HAL_EEPROM_WriteBytes(FINGER_APP_CFG_REG_START+u16Num*FINGER_APP_CFG_ONE_SIZE, (uint8_t *)&_stCfg, sizeof(FingerAppCfg_S)) > 0)
            {
                if((HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0))
                {
                    SystemSeting.SysFingerAllNum++;
                    (MEM_USER_GUEST == _stCfg.acOffset[EM_FINGER_APP_CFG_ADMIN_EN])? (SystemSeting.SysFingerGuestNum++): (SystemSeting.SysFingerAdminNum++);
                    (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAllNum, sizeof(SystemSeting.SysFingerAllNum) );
                    (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAdminNum, sizeof(SystemSeting.SysFingerAdminNum) );
                    (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerGuestNum, sizeof(SystemSeting.SysFingerGuestNum) );
                    
                    my_printf("# FINGER # EE - ADD Finger num - %d  (ALL/ADMIN/GUEST):(%d/%d/%d)\n",\
                        (uint16_t)u16Num, SystemSeting.SysFingerAllNum, SystemSeting.SysFingerAdminNum, SystemSeting.SysFingerGuestNum);
                    return true;
                }
            }
        }
        /* 如果非空的，更新 */
        else
        {
            if(HAL_EEPROM_WriteBytes(FINGER_APP_CFG_REG_START+u16Num*FINGER_APP_CFG_ONE_SIZE, (uint8_t *)&_stCfg, sizeof(FingerAppCfg_S)) > 0)
            {
                if((HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0))
                {
                    my_printf("# FINGER # EE - Update Finger num - %d  (ALL/ADMIN/GUEST):(%d/%d/%d)\n",\
                        (uint16_t)u16Num, SystemSeting.SysFingerAllNum, SystemSeting.SysFingerAdminNum, SystemSeting.SysFingerGuestNum);
                    return true;
                }
            }
        }
    }
    return false;
}

/****************************************************************************** 
* 函数名：app_finger_cfg_read
* 功 能：指纹业务配置，读取相应指纹配置区域
* 输 入：uint16 _u16Num
* 输 出：void
* 返 回：bool
*/
static bool app_finger_cfg_read(uint16_t _u16Num, FingerAppCfg_S *_pstCfg)
{
    /* 判定序号的有效性 */
    if(_u16Num >= MSG_FINGER_NUM_RESERVED) {return false;}
    
    if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+_u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)_pstCfg, sizeof(_pstCfg)) > 0)
    {
        return true;
    }
    
    return false;
}

/****************************************************************************** 
* 函数名：app_finger_cfg_write_null
* 功 能：指纹业务配置，清空相应指纹配置区域
* 输 入：uint16 _u16Num
* 输 出：void
* 返 回：bool
*/
static bool app_finger_cfg_write_null(uint16_t _u16Num)
{
    /* 判定序号的有效性 */
    if(_u16Num >= MSG_FINGER_NUM_RESERVED) {return false;}
    FingerAppCfg_S stReadCfg = {0};
    if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+_u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)&stReadCfg, sizeof(stReadCfg)) == 0) {return false;}

    FingerAppCfg_S stCfg = {FINGER_APP_CFG_RESET_VALUE}; 
    memset(&stCfg, FINGER_APP_CFG_RESET_VALUE, sizeof(FingerAppCfg_S));
    
    if(HAL_EEPROM_WriteBytes(FINGER_APP_CFG_REG_START+_u16Num*FINGER_APP_CFG_ONE_SIZE, (uint8_t *)&stCfg, sizeof(FingerAppCfg_S)) > 0)
    {
        SystemSeting.SysFingerAllNum--;
        (MEM_USER_MASTER == stReadCfg.acOffset[EM_FINGER_APP_CFG_ADMIN_EN])? (SystemSeting.SysFingerAdminNum--): (SystemSeting.SysFingerGuestNum--);
        (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAllNum, sizeof(SystemSeting.SysFingerAllNum) );
        (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerAdminNum, sizeof(SystemSeting.SysFingerAdminNum) );
        (void)SystemWriteSeting( (uint8_t *)&SystemSeting.SysFingerGuestNum, sizeof(SystemSeting.SysFingerGuestNum) );
        return true;
    }
    return false;
}

/****************************************************************************** 
* 函数名：app_finger_cfg_get_registid
* 功 能：批量获取已被注册的ID号
* 输 入：uint8_t _u8Len  缓存_pu8ID 的实际大小
* 输 出：uint8_t *_pu8ID 缓存地址, uint8_t *_pu8Len 实际写入的长度
* 返 回：bool
*/
static bool app_finger_cfg_get_registid(uint8_t _u8Len, uint8_t *_pu8ID, uint8_t *_pu8Len)
{
    /* 判定序号的有效性 */
    if(_u8Len >= MSG_FINGER_NUM_RESERVED) {return false;}
    
    *_pu8Len = 0;
    uint8_t cnt = 0;
    for(uint8_t i = 0; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        FingerAppCfg_S stCfg = {0};
        if((HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+i*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0)
            &&(FINGER_APP_CFG_RESET_VALUE != stCfg.acOffset[EM_FINGER_APP_CFG_EN]
            && FINGER_APP_CFG_RESET_NULL != stCfg.acOffset[EM_FINGER_APP_CFG_EN]))
        {
            _pu8ID[cnt] = i;
            cnt++;
        }
    }
    *_pu8Len = cnt;
    return true;
    
}

/****************************************************************************** 
* 函数名：app_finger_cfg_get_customid
* 功 能：检索对应ID号的custom ID
* 输 入：uint16_t u16CustomID, uint16_t *pu16PageID
* 输 出：bool
* 返 回：bool
*/
static bool app_finger_cfg_customid_convert_pageid(uint16_t u16CustomID, uint16_t *pu16PageID)
{
    FingerAppCfg_S stCfg = {0};
    uint16_t customid = 0;
    for(uint8_t i = 0; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+i*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0)
        {
            customid = (stCfg.acOffset[EM_FINGER_APP_CFG_CUSTOM_ID_H] << 8) + stCfg.acOffset[EM_FINGER_APP_CFG_CUSTOM_ID_L];
            if(customid == u16CustomID)
            {
                *pu16PageID = i;
                return true;
            }
        }
    }

    return false;
}

/****************************************************************************** 
* 函数名：app_finger_cfg_customid_check
* 功 能：custom ID 查重
* 输 入：uint16_t u16CustomID, uint16_t *pu16PageID
* 输 出：bool   true - customID 可以使用   false - customID 重复
* 返 回：bool
*/
static bool app_finger_cfg_customid_check(uint16_t u16CustomID)
{
    FingerAppCfg_S stCfg = {0};
    uint16_t customid = 0;
    for(uint8_t i = 0; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+i*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0)
        {
            customid = (stCfg.acOffset[EM_FINGER_APP_CFG_CUSTOM_ID_H] << 8) + stCfg.acOffset[EM_FINGER_APP_CFG_CUSTOM_ID_L];
            if(customid == u16CustomID)
            {
                return false;
            }
        }
    }

    return true;
}

/****************************************************************************** 
* 函数名：app_finger_work_cnt
* 功 能：提供给外部定时器(每10毫秒调用一次)，用于业务逻辑延迟
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
static uint32_t app_finger_work_cnt(void)
{
    return (s_u32AppWorkCnt > 0) ? (s_u32AppWorkCnt--) : 0;
}

/****************************************************************************** 
* 函数名：app_finger_set_countdown
* 功 能：设置执行业务延时时间
* 输 入：uint8_t 
* 输 出：void
* 返 回：bool 设置是否成功
*/ 
static bool app_finger_work_set_10ms(uint32_t _u32AppWorkCnt)
{
    /* 业务流程轮转延时，不宜过大，放到10ms定时器，需要*100  */
    if(_u32AppWorkCnt > 5000) { return false; }

    s_u32AppWorkCnt = _u32AppWorkCnt;
    return true;
}

/****************************************************************************** 
* 函数名：app_finger_get_countdown
* 功 能：获取执行业务延时时间
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
static uint32_t app_finger_work_get(void)
{
    return s_u32AppWorkCnt;
}

/****************************************************************************** 
* 函数名：app_finger_img_cnt
* 功 能：提供给外部定时器(每10毫秒调用一次)，每一次尝试获取图像，减少一次计数
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
static uint32_t app_finger_img_cnt(void)
{
    return (s_u32GetImageCnt > 0) ? (s_u32GetImageCnt--) : 0;
}

/****************************************************************************** 
* 函数名：app_finger_img_reset
* 功 能：重置图像尝试延时，单位是 10ms
* 输 入：void 
* 输 出：void
* 返 回：void
*/ 
static void app_finger_img_reset( uint32_t _u32AppWorkCnt)
{
    /* 放到10ms定时器，需要*10   ,不宜过大 */
    s_u32GetImageCnt = _u32AppWorkCnt;
    return;
}

/****************************************************************************** 
* 函数名：app_finger_img_get
* 功 能：获取当前图像尝试次数的计数值
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
static uint32_t app_finger_img_get(void)
{
    return s_u32GetImageCnt;
}

/****************************************************************************** 
* 函数名：app_finger_led
* 功 能：指纹录入灯板轮巡点亮
* 输 入：uint8_t _u8Led
* 输 出：void
* 返 回：void
*/
static void app_finger_get_led(uint8_t _u8Led)
{
    if(EM_FINGER_APP_FLOW1_ADD != s_stFingerAppParam.emAppFlow && EM_FINGER_APP_FLOW5_SLIDE_ADD != s_stFingerAppParam.emAppFlow) { return; }
    
    /* 亮灯 */
    switch(_u8Led)
    {
        case 1: (void)App_LED_OutputCtrl(EM_LED_1, EM_LED_ON); break;
        case 2: (void)App_LED_OutputCtrl(EM_LED_2, EM_LED_ON); break;
        case 3: (void)App_LED_OutputCtrl(EM_LED_3, EM_LED_ON); break;
        case 4: (void)App_LED_OutputCtrl(EM_LED_4, EM_LED_ON); break;
        case 5: (void)App_LED_OutputCtrl(EM_LED_5, EM_LED_ON); break;
        case 6: (void)App_LED_OutputCtrl(EM_LED_6, EM_LED_ON); break;
        default: break;
    }
    
    return;
}

/****************************************************************************** 
* 函数名：app_finger_check_time
* 功 能： 校验手指是否具有开门权限以及ID的时效性
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_check_access(void)
{
    if(0 == FINGER_GetSearchScore()) {return false;}
    
    bool ret = false;
    s_stFingerAppParam.u16OutPad = FINGER_GetSearchID();
    FingerAppCfg_S stFingermeg = {0};
    if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+s_stFingerAppParam.u16OutPad*MSG_FINGER_ONE_SIZE, (uint8_t *)&stFingermeg, sizeof(stFingermeg)) == 0)
    {
        return false;
    }
        
    /* 时效标记未打开，直接成功 */
    if(0 == stFingermeg.acOffset[EM_FINGER_APP_CFG_TIME_EN])
    {
        my_printf("# FINGER # Open ID(%d) without check time \n", (uint32_t)s_stFingerAppParam.u16OutPad);
        ret = true;
    }
    /* 获取对应ID 的 配置，获取时效标记和时间 ，
                        时效标记打开的情况下才做时效范围判定*/
    else if( true == app_finger_cfg_read(s_stFingerAppParam.u16OutPad, &stFingermeg))
    {
        RTCType start = {stFingermeg.acOffset[EM_FINGER_APP_CFG_START_SEC], stFingermeg.acOffset[EM_FINGER_APP_CFG_START_MIN],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_START_HOUR], 0,
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_START_DAY],stFingermeg.acOffset[EM_FINGER_APP_CFG_START_MONTH],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_START_YEAR], 0};
        RTCType end = {stFingermeg.acOffset[EM_FINGER_APP_CFG_END_SEC], stFingermeg.acOffset[EM_FINGER_APP_CFG_END_MIN],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_END_HOUR], 0,
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_END_DAY],stFingermeg.acOffset[EM_FINGER_APP_CFG_END_MONTH],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_END_YEAR], 0};    //结束时间

        if(RTC_Successfully == HAL_RTC_TimeIsTimesize(&start, &end, stFingermeg.acOffset[EM_FINGER_APP_CFG_WEEK]))
        {
            my_printf("# FINGER # Open ID = %d \n", (uint32_t)s_stFingerAppParam.u16OutPad);
            ret = true;
        }
        else
        {
            my_printf("# FINGER # Open ID = %d ,But check time error\n", (uint32_t)s_stFingerAppParam.u16OutPad);
        }
    }

    return ret;
}

/****************************************************************************** 
* 函数名：app_finger_report
* 功 能：语音播报动作统一封装接口
* 输 入：VoiceType_E _em
* 输 出：void
* 返 回：void
*/
static void app_finger_report(VoiceType_E _em, bool isForceReport)
{
    static VoiceType_E emFlag = EM_VOICE_BREAK_CMD;
    uint16_t holdTimeMs = 1200;
    switch(_em)
    {
        case EM_PUT_FINGER_MP3:
        case EM_MOVE_FINGER_MP3:
        #ifdef FINGER_VEIN_FUNCTION_ON
            if(EM_MOVE_FINGER_MP3 == _em) {_em = EM_RELEASE_FINGER_MP3;}
            else if(EM_PUT_FINGER_MP3 == _em && (1 == s_u8AppFlowOrderFlag)) { _em = EM_CONTACT_AT_BOTH_ENDS_MP3; holdTimeMs = 2800; }
            else { _em = EM_PLACE_FINGER_MP3; }
        #elif defined(FRAME_PLATFORM_FULLY_AUTO_ON)
            if(ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister)
            {
                if(EM_MOVE_FINGER_MP3 == _em) {holdTimeMs = 600;}
                else if(EM_PUT_FINGER_MP3 == _em ) { holdTimeMs = 400; }
            }
        #endif
            break;
            
        #ifdef FINGER_VEIN_FUNCTION_ON
        case EM_SLIDE_VEIN_TIPS_MP3: holdTimeMs = 2000; break;
        #endif

        default: emFlag = EM_VOICE_BREAK_CMD; return;
    }
    if(emFlag != _em || isForceReport)
    {
        emFlag = _em;
        /* 增加体验，先打断上一次语音，延时后再播报新语音  */
        (void)HAL_Voice_PlayingVoice(EM_VOICE_BREAK_CMD, 0);
        PUBLIC_Delayms(300);

        (void)HAL_Voice_PlayingVoice(_em, holdTimeMs);
        /* 给与1秒时间播报 */
        (void)app_finger_work_set_10ms(holdTimeMs/10);
        /* 复位指纹模组尝试操作获取图像的计数值 */
        (void)app_finger_img_reset(holdTimeMs);
    }
    
    return;
}

/****************************************************************************** 
* 函数名：app_finger_reset_flag
* 功 能：指纹业务接口，重置相关flag标记变量
* 输 入：FINGER_APP_FLOW_RESULT_E
* 输 出：void
* 返 回：void
*/
static void app_finger_reset_flag(FINGER_APP_FLOW_RESULT_E _em, bool _bForce)
{
    /*  1. 状态运转过程中，异常退出 
            2. 状态运转过程中，但运转结果已复位
            3. 强制复位(睡眠) */
    if((0 != s_u8AppFlowOrderFlag && FINGER_APP_RESULT_RUNNING == s_stFingerAppParam.emFlowResult)
        || (0 != s_u8AppFlowOrderFlag && FINGER_APP_RESULT_IDLE == s_stFingerAppParam.emFlowResult)
        || (FINGER_APP_RESULT_TIMEOUT == _em)
        || true == _bForce)
    {
        (void)FINGER_Sleep();
        memset((void*)&s_stFingerAppParam.stFingerAppCfg, 0, sizeof(s_stFingerAppParam.stFingerAppCfg));
        s_stFingerAppParam.emAppFlow = EM_FINGER_APP_FLOW_NONE;
        s_stFingerAppParam.emFlowResult = _bForce?FINGER_APP_RESULT_IDLE:_em;
        s_u8AppFlowOrderFlag = 0;
        /* 重置音频播报标记 */
        app_finger_report(EM_VOICE_BREAK_CMD, false);
    }
    return;
}

/****************************************************************************** 
* 函数名：app_finger_ack_ok
* 功 能： 流程返回值OK后处理处理写EE
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_ack_ok(void)
{
    my_printf("# FINGER # app_finger_ack_ok \n");
    bool bRet = false;
    /* 最后完成 */
    if(EM_FINGER_FLOW3_REG == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        /* 配置写EE 操作 */
        uint8_t u8AdminEn = s_stFingerAppParam.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ];
        FINGER_APP_USER_TYPE_E _em = (MEM_USER_MASTER == u8AdminEn)? EM_FINGER_APP_TYPE_ADMIN : EM_FINGER_APP_TYPE_COMMON;
        
        if(true == app_finger_cfg_get_nullnum(_em, &s_stFingerAppParam.u16OutPad))
        {
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_ADMIN_EN] = u8AdminEn;
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] = (uint8_t)(s_stFingerAppParam.u16OutPad>>8);
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_L] = (uint8_t)(s_stFingerAppParam.u16OutPad);
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_WEEK] = 0xFE;

            /* 存储成功则播报登记成功 */
            if(app_finger_cfg_write(s_stFingerAppParam.stFingerAppCfg))
            {
                (void)app_finger_reset_flag(FINGER_APP_RESULT_SUC, false);
                return true;
            }
        }
        (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
    }
    else if(EM_FINGER_FLOW0_GET == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag+1])
    {
        // 指静脉强制走完滑动录入过程中的语音时长
        if(EM_FINGER_APP_FLOW5_SLIDE_ADD == s_stFingerAppParam.emAppFlow)
        {
            app_finger_img_reset(50);
            while(app_finger_img_get() > 0) {;}
        }
        
        (void)app_finger_report(EM_MOVE_FINGER_MP3, false);
        bRet = false;
    }
    else if(EM_FINGER_FLOW0_GET == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        my_printf("# FINGER # app_finger_ack_ok  - flag - %d   get img - %d    isPutUpFinger = %d \n", s_u8AppFlowOrderFlag, app_finger_img_get(), isPutUpFinger);
#ifdef FRAME_PLATFORM_FULLY_AUTO_ON
        if((1 == s_u8AppFlowOrderFlag || isPutUpFinger == true || ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister) 
            && (app_finger_img_get() > 0 ))
#else
        if((1 == s_u8AppFlowOrderFlag || isPutUpFinger == true) 
            && (app_finger_img_get() > 0 ))
#endif
        {
            my_printf("# FINGER #  ack ok #1 \n");
            /* 针对指纹添加流程的灯板 */
            (void)app_finger_get_led((s_u8AppFlowOrderFlag)/2 + 1);
            (void)app_finger_report(EM_PUT_FINGER_MP3, false);
            (void)app_finger_img_reset(600);
        }
        else if(0 == app_finger_img_get())
        {
            my_printf("# FINGER #  ack ok #2 \n");
            (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
        }
        else
        {
            my_printf("# FINGER #  ack ok #3 \n");
            s_u8AppFlowOrderFlag--;
            bRet = true;
        }
    }
    return bRet;
}

/****************************************************************************** 
* 函数名：app_finger_ack_err
* 功 能： 流程返回值ERROR 后处理处理写EE
* 输 入：uint8_t ack
* 输 出：void
* 返 回：bool
*/
static bool app_finger_ack_err(uint8_t ack)
{
    my_printf("# FINGER # app_finger_ack_err \n");
    isPutUpFinger = true;
    
    // 开门流程不需要播报放上手指语音
    if(EM_FINGER_FLOW0_GET == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag] && EM_FINGER_APP_FLOW3_SEARCH != s_stFingerAppParam.emAppFlow)
    {
        /* 针对指纹添加流程的灯板 */
        (void)app_finger_get_led((s_u8AppFlowOrderFlag)/2 + 1);
        (void)app_finger_report(EM_PUT_FINGER_MP3, false);
    }
    else if(EM_FINGER_FLOW9_SLIDE == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag] && EM_FINGER_APP_FLOW3_SEARCH != s_stFingerAppParam.emAppFlow)
    {
        (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
        return false;
    }
    
    if(app_finger_img_get() > 0)
    {
        if(EM_FINGER_APP_FLOW1_ADD == s_stFingerAppParam.emAppFlow \
            && EM_FINGER_FLOW1_CONVERT == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
        {
#ifdef FINGER_PROTOCAL_V2_SUPPORT
            s_u8FingerProtocalV2Cnt++;
            my_printf("# FINGER # convert err, need get image ! try cnt - %d\n", s_u8FingerProtocalV2Cnt);
#ifdef FRAME_PLATFORM_FULLY_AUTO_ON
            if(s_u8FingerProtocalV2Cnt <= 3 || ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister)
#else
            if(s_u8FingerProtocalV2Cnt <= 3 )
#endif
            {
                // 录入指纹时，转特征码失败，进行重新采图
                s_u8AppFlowOrderFlag -= 2;
                return true;
            }
#else
            // 录入指纹时，转特征码失败，进行重新采图
            my_printf("# FINGER # convert err, need get image ! \n");
            s_u8AppFlowOrderFlag -= 2;
            return true;

#endif
        }
        else
        {
            s_u8AppFlowOrderFlag--;
            return true;
        }
    
    }
    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
    
    return false;
}


/****************************************************************************** 
* 函数名：app_finger_add
* 功 能： 用于EM_FINGER_APP_FLOW1_ADD 和 EM_FINGER_APP_FLOW5_SLIDE_ADD 流程 ，
                     针对滑动录入指静脉处理返回值
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_add(void)
{
    /* 非EM_FINGER_APP_FLOW5_SLIDE_ADD 不适用 */
    if(EM_FINGER_APP_FLOW1_ADD != s_stFingerAppParam.emAppFlow && EM_FINGER_APP_FLOW5_SLIDE_ADD != s_stFingerAppParam.emAppFlow) {return true;}

    bool bRet = true;
    if(0 != s_u8AppFlowOrderFlag && (EM_FINGER_FLOW_NONE != s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag]))
    {
        uint8_t ack = FINGER_GetAckCode();
        switch(ack)
        {
            case FINGER_ACK_OK: bRet = app_finger_ack_ok(); break;

            default:
                my_printf("app_finger_add default action # flag - %d ack - 0x%x \n", s_u8AppFlowOrderFlag, ack);
                if(FINGER_ACK_NONE != ack) 
                {
#ifdef FINGER_FUNCTION_ON
                    /* 指纹模组概率手指放上返回录入不成功，做重试 */
                    bRet = app_finger_ack_err(ack);
#else
                    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
                    bRet = false;
#endif
                }
                break;
        }
    }

    return bRet;
}

/****************************************************************************** 
* 函数名：app_finger_search
* 功 能： 用于EM_FINGER_APP_FLOW3_SEARCH 流程 ， 针对EM_FINGER_FLOW2_SEARCH 指纹图像获取的业务尝试处理
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_search(void)
{
    /* 非EM_FINGER_APP_FLOW3_SEARCH 不适用 */
    if(EM_FINGER_APP_FLOW3_SEARCH != s_stFingerAppParam.emAppFlow) {return true;}
    
    bool bRet = true;
    if(0 != s_u8AppFlowOrderFlag && EM_FINGER_FLOW_NONE != s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        switch(FINGER_GetAckCode())
        {
            case FINGER_ACK_OK:
                if(EM_FINGER_FLOW2_SEARCH == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
                {
                    if(true == app_finger_check_access()){ (void)app_finger_reset_flag(FINGER_APP_RESULT_SUC, false); }
                }
                break;
                
            case FINGER_ACK_NOFINGER:
                if(EM_FINGER_FLOW0_GET == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
                {
                    if(app_finger_img_get() > 0) { app_finger_img_reset(200);s_u8AppFlowOrderFlag--; break; }
                    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
                    bRet = false;
                }
                break;
            case FINGER_ACK_PROTOCAL_ERR:
                (void)app_finger_reset_flag(FINGER_APP_PROTOCAL_ERR, false);
                bRet = false;
                break;
                
            default:
                if(EM_FINGER_FLOW2_SEARCH == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
                {
                    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
                }
                else if(EM_FINGER_FLOW0_GET == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
                {
                    if(app_finger_img_get() > 0) { s_u8AppFlowOrderFlag--; break; }
                    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
                    bRet = false;
                }
                break;
        }
    }

    return bRet;
}

/****************************************************************************** 
* 函数名：app_finger_delete
* 功 能： 用于EM_FINGER_APP_FLOW2_DEL 流程 ， 针对EM_FINGER_FLOW4_DEL 指纹图像获取的业务尝试处理
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_delete(void)
{
    /* 非EM_FINGER_APP_FLOW2_DEL 不适用 */
    if(EM_FINGER_APP_FLOW2_DEL != s_stFingerAppParam.emAppFlow) {return true;}
    
    if(0 != s_u8AppFlowOrderFlag && EM_FINGER_FLOW_NONE != s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        switch(FINGER_GetAckCode())
        {
            case FINGER_ACK_OK:
                {
                    uint16_t u16Pad = (s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] << 8) + s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_L];
                    /* 模组已返回删除成功了，不论EE写入是否成功，判定无意义 */
                    (void)app_finger_cfg_write_null(u16Pad);
                    my_printf("# FINGER # EE - DEL Finger num - %d  (ALL/ADMIN/GUEST):(%d/%d/%d)\n",\
                        (uint16_t)u16Pad, SystemSeting.SysFingerAllNum, SystemSeting.SysFingerAdminNum, SystemSeting.SysFingerGuestNum);
                    (void)app_finger_reset_flag(FINGER_APP_RESULT_SUC, false);
                }
                break;
                
            default: (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false); break;
        }
    }

    return true;
}

/****************************************************************************** 
* 函数名：app_finger_clear
* 功 能： 用于EM_FINGER_APP_FLOW0_CLEAR 流程 ， 针对EM_FINGER_FLOW5_EMPTY 指纹图像获取的业务尝试处理
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_clear(void)
{
    /* 非EM_FINGER_APP_FLOW0_CLEAR 不适用 */
    if(EM_FINGER_APP_FLOW0_CLEAR != s_stFingerAppParam.emAppFlow && EM_FINGER_APP_FLOW8_CLEAR_V2 != s_stFingerAppParam.emAppFlow) {return true;}
    
    if(0 != s_u8AppFlowOrderFlag \
        && (EM_FINGER_FLOW7_AESKEY == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag] \
            || EM_FINGER_FLOW7_AESKEY == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag]))
    {
        switch(FINGER_GetAckCode())
        {
            case FINGER_ACK_OK:
                (void)app_finger_cfg_reset();
                my_printf("# FINGER # EE - CLEAR Finger (ALL/ADMIN/GUEST):(%d/%d/%d)\n",\
                    SystemSeting.SysFingerAllNum, SystemSeting.SysFingerAdminNum, SystemSeting.SysFingerGuestNum);
                (void)app_finger_reset_flag(FINGER_APP_RESULT_SUC, false);
                break;
                
            default: (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false); break;
        }
    }

    return true;
}

/****************************************************************************** 
* 函数名：app_finger_clear_common
* 功 能： 用于EM_FINGER_APP_FLOW4_CLEAR_COMMON 流程 ， 针对EM_FINGER_FLOW5_EMPTY 指纹图像获取的业务尝试处理
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_clear_common(void)
{
    /* 非EM_FINGER_APP_FLOW4_CLEAR_COMMON 不适用 */
    if(EM_FINGER_APP_FLOW4_CLEAR_COMMON != s_stFingerAppParam.emAppFlow) {return true;}
    
    if(0 != s_u8AppFlowOrderFlag && EM_FINGER_FLOW_NONE != s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        switch(FINGER_GetAckCode())
        {
            case FINGER_ACK_OK:
                (void)app_finger_cfg_reset_common();
                (void)app_finger_reset_flag(FINGER_APP_RESULT_SUC, false);
                my_printf("# FINGER # EE - CLEAR Common Finger (ALL/ADMIN/GUEST):(%d/%d/%d)\n",\
                    SystemSeting.SysFingerAllNum, SystemSeting.SysFingerAdminNum, SystemSeting.SysFingerGuestNum);
                break;
                
            default:
                (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
                break;
        }
    }

    return true;
}

/****************************************************************************** 
* 函数名：app_finger_ack_scan
* 功 能： 对协议返回结果处理 
* 输 入：void
* 输 出：void
* 返 回：bool
*/
static bool app_finger_ack_scan(void)
{
    return (!app_finger_add()|| !app_finger_search()|| !app_finger_delete()|| !app_finger_clear()|| !app_finger_clear_common()) ? false : true;
}

/****************************************************************************** 
* 函数名：app_finger_inparam_scan
* 功 能： 状态运转，准备入参和预制动作
* 输 入：uint16_t *_pu16Pad
* 输 出：void
* 返 回：bool
*/
static bool app_finger_inparam_scan(uint32_t *_pu32Pad)
{
    /* 状态运转，非法操作则复位 */
    s_u8AppFlowOrderFlag ++;
    if(s_stFingerAppParam.emAppFlow >= EM_FINGER_APP_FLOW_ALL || EM_FINGER_FLOW_NONE == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag] || s_u8AppFlowOrderFlag > FINGER_APP_FLOW_MAX)
    {
        (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
        return false;
    }
    
    *_pu32Pad = 0;
    switch(s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
         case EM_FINGER_FLOW0_GET:
            /* 录入指纹或指静脉时，第一次录入不检测也不需要抬起手指动作 */
#ifdef FRAME_PLATFORM_FULLY_AUTO_ON
            if(1 == s_u8AppFlowOrderFlag && ADMIN_APP_REGISTERED == SystemSeting.SystemAdminRegister) { isPutUpFinger = true; }
#else
            if(1 == s_u8AppFlowOrderFlag) { isPutUpFinger = true; }
#endif
            break;
        case EM_FINGER_FLOW9_SLIDE:
            (void)app_finger_report(EM_SLIDE_VEIN_TIPS_MP3, false);
        case EM_FINGER_FLOW1_CONVERT:
            isPutUpFinger = false;
            /* 根据MAP s_u8AppFlowMap 对流程分析得到的计算方式，得到的u16Pad为实际需要写入对应buffer 的编号 */
            *_pu32Pad = s_u8AppFlowOrderFlag/2;
            break;

        case EM_FINGER_FLOW3_REG:
            {
                uint8_t u8AdminEn = s_stFingerAppParam.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ];
                FINGER_APP_USER_TYPE_E _em = (MEM_USER_MASTER == u8AdminEn)? EM_FINGER_APP_TYPE_ADMIN : EM_FINGER_APP_TYPE_COMMON;
                uint16_t u16Pad = 0;
                if(false == app_finger_cfg_get_nullnum(_em , &u16Pad))
                {
                    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
                    return false;
                }
                *_pu32Pad = (uint32_t)u16Pad;
            }
            break;
            
        case EM_FINGER_FLOW4_DEL:
            if(EM_FINGER_APP_FLOW4_CLEAR_COMMON == s_stFingerAppParam.emAppFlow)
            {
                uint16_t u16Offet = MSG_FINGER_ADMIN_NUM;
                uint16_t u16Num = MSG_FINGER_NUM_RESERVED - MSG_FINGER_ADMIN_NUM;
                *_pu32Pad = (uint32_t)(u16Num << 16)  + u16Offet ;
            }
            else if(EM_FINGER_APP_FLOW2_DEL == s_stFingerAppParam.emAppFlow)
            {
                *_pu32Pad = (uint32_t)(s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] << 8) + s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_L];
                *_pu32Pad += (uint32_t)((uint8_t)1 << 16);
            }
            break;
            
        default:
            break;
    }

    return true;
}


/***************************** 3 对外函数*****************************/

/****************************************************************************** 
* 函数名：APP_FINGER_Init
* 功 能：指纹动作扫描
* 输 入：void
* 输 出：void
* 返 回：void
*/
void APP_FINGER_Init(void)
{
    /* 重置复位指纹业务功能内部参数 */
    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
    
    /* 指纹协议服务初始化，开启指纹模组功能 */
    (void)FINGER_Init();
    
    return;
}

/****************************************************************************** 
* 函数名：APP_FINGER_Sleep
* 功 能：休眠
* 输 入：void
* 输 出：void
* 返 回：void
*/
void APP_FINGER_Sleep(void)
{
    (void)app_finger_reset_flag(FINGER_APP_RESULT_IDLE, true);

    return;
}

/****************************************************************************** 
* 函数名：App_FINGER_Tim10Ms
* 功 能：10MS 定时器调用函数
* 输 入：void
* 输 出：void
* 返 回：void
*/
void App_FINGER_Tim10Ms(void)
{
    (void)app_finger_work_cnt();
    (void)app_finger_img_cnt();

    /*内部1S 定时*/
    static uint32_t u32Cnt = 100;
    u32Cnt--;
    if(0 == u32Cnt)
    {
        (void)FINGER_WorkCountDown();
        u32Cnt = 100;
    }
    
    return;
}

/****************************************************************************** 
* 函数名：APP_FINGER_Operate
* 功 能：指纹操作合集
* 输 入：FingerAppParam_S _stFingerAppParam
* 输 出：void
* 返 回：bool
*/
bool APP_FINGER_Operate(FingerAppParam_S _stFingerAppParam)
{
    /* 业务流程判定 */
    if(EM_FINGER_APP_FLOW_NONE != _stFingerAppParam.emAppFlow && _stFingerAppParam.emAppFlow >= EM_FINGER_APP_FLOW_ALL) { return false; }
    
    /* 满了则直接返回，不再允许添加指纹 */
    uint16_t u16Pad = 0;
    uint8_t u8AdminEn = _stFingerAppParam.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ];
    FINGER_APP_USER_TYPE_E _em = (MEM_USER_MASTER == u8AdminEn)? EM_FINGER_APP_TYPE_ADMIN : EM_FINGER_APP_TYPE_COMMON;
    if(EM_FINGER_APP_FLOW1_ADD == _stFingerAppParam.emAppFlow && false == app_finger_cfg_get_nullnum(_em, &u16Pad)) { return false; }

#ifdef FINGER_VEIN_FUNCTION_ON
    /* 指静脉更改为滑动录入 */
    if(EM_FINGER_APP_FLOW1_ADD == _stFingerAppParam.emAppFlow) {_stFingerAppParam.emAppFlow = EM_FINGER_APP_FLOW5_SLIDE_ADD;isPutUpFinger = false;}
#endif

    /* 流程支持打断，打断需要给模式睡眠、复位 */
    if(FINGER_APP_RESULT_RUNNING == s_stFingerAppParam.emFlowResult)
    {
        (void)FINGER_Sleep();
    }

    /* 部分流程参数入参检查 */
    if(EM_FINGER_APP_FLOW9_DEL_CUSTOM == _stFingerAppParam.emAppFlow)
    {
        uint16_t customid = (_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_CUSTOM_ID_H] << 8) + _stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_CUSTOM_ID_L];
        uint16_t pageid = 0;
        if(0 == customid || false == app_finger_cfg_customid_convert_pageid(customid, &pageid))
        {
            (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
            return false;
        }
        _stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] = (uint8_t)(pageid >> 8);
        _stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_L] = (uint8_t)(pageid);
    }

    /* 每次操作指纹模组需要唤醒模块 */
    (void)FINGER_WakeUp();

    /* 记录当前业务流程状态值*/
#ifdef FINGER_PROTOCAL_V2_SUPPORT
    s_u8FingerProtocalV2Cnt = 0;
#endif
    s_u8AppFlowOrderFlag = 0;
    s_stFingerAppParam.u16OutPad = 0;
    s_stFingerAppParam.emFlowResult = FINGER_APP_RESULT_RUNNING;
    s_stFingerAppParam.emAppFlow = _stFingerAppParam.emAppFlow;
    memcpy((void*)&s_stFingerAppParam.stFingerAppCfg, (void*)&_stFingerAppParam.stFingerAppCfg, sizeof(s_stFingerAppParam.stFingerAppCfg));
    /* 复位指纹模组尝试操作获取图像的计数值 */
    (void)app_finger_img_reset(100);
    
    return true;
}

/****************************************************************************** 
* 函数名：APP_FINGER_GetFlowResult
* 功 能：返回指纹业务流程结果
* 输 入：void
* 输 出：uint16_t *pu16Pad
* 返 回：FINGER_APP_FLOW_RESULT_E
*/ 
FINGER_APP_FLOW_RESULT_E APP_FINGER_GetFlowResult(uint16_t *pu16Pad)
{
    FINGER_APP_FLOW_RESULT_E em = s_stFingerAppParam.emFlowResult;
    if(FINGER_APP_RESULT_IDLE != s_stFingerAppParam.emFlowResult
        && FINGER_APP_RESULT_RUNNING != s_stFingerAppParam.emFlowResult)
    {
        *pu16Pad = s_stFingerAppParam.u16OutPad;
        s_stFingerAppParam.u16OutPad = 0;
        s_stFingerAppParam.emFlowResult = FINGER_APP_RESULT_IDLE;
    }

    return em;
}

/****************************************************************************** 
* 函数名：APP_FINGER_GetFlowResult
* 功 能：返回所有已被注册的ID号
* 输 入：uint8_t _u8Len  缓存_pu8ID 的实际大小
* 输 出：uint8_t *_pu8ID 缓存地址, uint8_t *_pu8Len 实际写入的长度
* 返 回：void
*/ 
bool APP_FINGER_GetFingerID(uint8_t _u8Len, uint8_t *_pu8ID, uint8_t *_pu8Len)
{
    return app_finger_cfg_get_registid(_u8Len, _pu8ID, _pu8Len);
}

    
/****************************************************************************** 
* 函数名：APP_FINGER_GetPowerFlag
* 功 能：返回指纹模组的上电标记
* 输 入：void
* 输 出：void
* 返 回：uint8_t
*/ 
uint8_t APP_FINGER_GetPowerFlag(void)
{
    return FINGER_GetPowerFlag();
}

/****************************************************************************** 
* 函数名：APP_FINGER_GetProtocalVersion
* 功 能：返回指纹协议版本
* 输 入：void
* 输 出：void
* 返 回：uint8_t
*/ 
uint8_t APP_FINGER_GetProtocalVersion(void)
{
    return FINGER_GetProtocalVersion();
}


/****************************************************************************** 
* 函数名：APP_FINGER_GetFlowResult
* 功 能：返回指纹业务流程结果
* 输 入：void
* 输 出：void
* 返 回：FINGER_APP_FLOW_RESULT_E
*/ 
FINGER_APP_ADD_FLOW_E APP_FINGER_GetAddFingerFlow(void)
{
    /* 当前如果非ADD流程，返回无状态，允许外部终止流程进行录入指纹 */
    if(EM_FINGER_APP_FLOW1_ADD != s_stFingerAppParam.emAppFlow && EM_FINGER_APP_FLOW5_SLIDE_ADD != s_stFingerAppParam.emAppFlow) {return FINGER_APP_ADD_NONE;}

    FINGER_APP_ADD_FLOW_E em = FINGER_APP_ADD_NONE;
    switch(s_stFingerAppParam.emFlowResult)
    {
        case FINGER_APP_RESULT_RUNNING:
        {
            if(EM_FINGER_APP_FLOW5_SLIDE_ADD == s_stFingerAppParam.emAppFlow)
            {
                em = (FINGER_APP_ADD_FLOW_E)s_u8AppFlowOrderFlag;
                break;
            }
            em = (FINGER_APP_ADD_FLOW_E)(s_u8AppFlowOrderFlag/2);
        }
        break;

        case FINGER_APP_RESULT_SUC:
        case FINGER_APP_RESULT_FAIL:{em =  FINGER_APP_ADD_OVER;} break;

        case FINGER_APP_RESULT_IDLE:
        default:{em =  FINGER_APP_ADD_NONE;} break;
    }

    return em;
}

/****************************************************************************** 
* 函数名：APP_FINGER_Get_Breakdown
* 功 能：获取设备故障情况
* 输 入：void
* 输 出：void
* 返 回：bool - true - 正常 false - 故障
*/ 
bool APP_FINGER_Get_Breakdown(void)
{
    (void)APP_FINGER_Init();

    bool bPowerStatus = FINGER_GetPowerStatus();

    (void)APP_FINGER_Sleep();
    
    return bPowerStatus;
}


/****************************************************************************** 
* 函数名：APP_FINGER_Scan
* 功 能：状态轮训，通过预制MAP，改为自动调用FINGER接口，执行业务流程
* 输 入：void
* 输 出：void
* 返 回：void
*/
void APP_FINGER_Scan(void)
{
    /* 指纹模组协议执行轮巡 */
    (void)FINGER_Scan();

    /* 访问模组超时 */
    if(EM_FINGER_FLOW_NONE == FINGER_GetFlowStatus() && FINGER_GetTxRxFlag())  { (void)app_finger_reset_flag(FINGER_APP_RESULT_TIMEOUT, false); return; }

    /* 模组未上电，不执行业务 */
    if(false == FINGER_GetPowerStatus() && EM_FINGER_FLOW_NONE == FINGER_GetFlowStatus()) 
    {
        return;
    }
 
    /* 业务流程判定 */
    if(s_stFingerAppParam.emAppFlow >= EM_FINGER_APP_FLOW_ALL) { (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false); return; }

    /* 协议流程当前结束无状态执行时才下行 */
    if(EM_FINGER_FLOW_NONE != FINGER_GetFlowStatus() || FINGER_APP_RESULT_RUNNING != s_stFingerAppParam.emFlowResult) {return;}


    /* 业务类计时完成后再执行 */
    if(0 == app_finger_work_get())
    {
        /* 对协议返回结果处理 */
        if(false == app_finger_ack_scan()) {return;}

        /* 状态运转，准备入参和预制动作 */
        uint32_t u32Pad = 0;
        if(false == app_finger_inparam_scan(&u32Pad)) {return;}

        /* 执行协议动作 */
        (void)FINGER_Operate((FINGER_FLOW_TYPE_E)s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag], u32Pad);
    }
    
    return;
}

/****************************************************************************** 
* 函数名：APP_FINGER_CfgGetNullNum
* 功 能：指纹业务配置，得到相应区域未使用的空的序号
* 输 入：void
* 输 出：uint16_t* _pu16Num 返回空的序号
* 返 回：bool
*/
bool APP_FINGER_CfgGetNullNum(FINGER_APP_USER_TYPE_E _em, uint16_t* _pu16Num)
{
    return app_finger_cfg_get_nullnum(_em, _pu16Num);
}

/****************************************************************************** 
* 函数名：APP_FINGER_CfgWrite
* 功 能：指纹业务配置，写入相应指纹配置区域
* 输 入：FingerAppCfg_S _stCfg
* 输 出：void
* 返 回：bool
*/
bool APP_FINGER_CfgWrite(FingerAppCfg_S _stCfg)
{
    return app_finger_cfg_write(_stCfg);
}

/****************************************************************************** 
* 函数名：APP_FINGER_CfgRead
* 功 能：指纹业务配置，读取相应指纹配置区域
* 输 入：uint16 _u16Num
* 输 出：void
* 返 回：bool
*/
bool APP_FINGER_CfgRead(uint16_t _u16Num, FingerAppCfg_S *_pstCfg)
{
    return app_finger_cfg_read(_u16Num, _pstCfg);
}

/****************************************************************************** 
* 函数名：APP_FINGER_CfgCheck_CustomID
* 功 能：指纹customID 查重
* 输 入：uint16 _u16Num
* 输 出：void
* 返 回：bool
*/
bool APP_FINGER_CfgCheck_CustomID(uint16_t _u16CustomID)
{
    return app_finger_cfg_customid_check(_u16CustomID);
}


