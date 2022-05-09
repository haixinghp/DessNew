/*********************************************************************************************************************
 * @file:        App_Finger.c
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-07-26
 * @Description: ָ��/ָ����ҵ��ģ��
 * @ChangeList:  01. ����
*/
/* ��׼ͷ�ļ� */


/* �ڲ�ͷ�ļ� */
#include "App_Finger.h"


/* �ⲿͷ�ļ� */
#include "Public.h"
#include "App_LED.h" 
#include "../HAL/HAL_EXPORT/HAL_EXPORT.h"
#include "../HAL/HAL_Voice/HAL_Voice.h"
#include "../HAL/HAL_RTC/HAL_RTC.h"
#include "../HAL/HAL_EEPROM/HAL_EEPROM.h"


/*********************** �ⲿ���������ͺ���************************/
/**************    Ϊ�˼�����ϣ�������ȥ���ⲿ����**********/


/*************************  0 �ڲ�ʹ�ú궨��*************************/
/*  ����- EE ��������ÿһλ��Ϊ0xFF  */
#define FINGER_APP_CFG_RESET_VALUE 0xFF
#define FINGER_APP_CFG_RESET_NULL 0x00


/*****************************  1 ��̬����*****************************/
/* ҵ����״̬ά������ */
static FingerAppParam_S s_stFingerAppParam = {EM_FINGER_APP_FLOW_NONE, 0};
static uint8_t s_u8AppFlowOrderFlag  = 0;       // ��¼��ǰҵ��������תֵ��Ӧ��ָ����תֵ����Ӧs_u8AppFlowMap
static uint32_t s_u32AppWorkCnt = 0;              // �����ʱ��
static uint32_t s_u32GetImageCnt = 0;             // ��ȡͼƬЭ�鳢�Լ���ֵ
static bool isPutUpFinger = false;
#ifdef FINGER_PROTOCAL_V2_SUPPORT
static uint8_t s_u8FingerProtocalV2Cnt = 0;     // V2�汾 ֧���Ż��ģ���������3��
#endif

/* ҵ��������ת״̬ */
static const uint8_t s_u8AppFlowMap[EM_FINGER_APP_FLOW_ALL][FINGER_APP_FLOW_MAX+1] = 
{
    /* ����������� */
    {EM_FINGER_APP_FLOW0_CLEAR,\
    EM_FINGER_FLOW5_EMPTY,      EM_FINGER_FLOW7_AESKEY,     EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* ����ָ������ */
    {EM_FINGER_APP_FLOW1_ADD,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,\
    EM_FINGER_FLOW6_MERGE,      EM_FINGER_FLOW3_REG,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* ɾ��ָ������ */
    {EM_FINGER_APP_FLOW2_DEL,\
    EM_FINGER_FLOW4_DEL,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* ����ָ������ */
    {EM_FINGER_APP_FLOW3_SEARCH,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW1_CONVERT,    EM_FINGER_FLOW2_SEARCH,     EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* �����ͨ�û� */
    {EM_FINGER_APP_FLOW4_CLEAR_COMMON,\
    EM_FINGER_FLOW4_DEL,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
        
    /* ָ����¼�� */
    {EM_FINGER_APP_FLOW5_SLIDE_ADD,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW9_SLIDE,      EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW9_SLIDE,\
    EM_FINGER_FLOW0_GET,        EM_FINGER_FLOW9_SLIDE,      EM_FINGER_FLOW3_REG,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* �ϻ����� */
    {EM_FINGER_APP_FLOW6_AGING,\
    EM_FINGER_FLOW10_AGING,     EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},

    /* 0x68 ����˯������ */
    {EM_FINGER_APP_FLOW7_68_SLEEP,\
    EM_FINGER_FLOW_EX0_GET,     EM_FINGER_FLOW_EX1_SLEEP,   EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
        
    /* �����������(����0x60 ��ʽ) */
    {EM_FINGER_APP_FLOW8_CLEAR_V2,\
    EM_FINGER_FLOW5_EMPTY,      EM_FINGER_FLOW11_AESKEY_V2, EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
    
    /* ɾ��ָ������(ͨ��customID ɾ����ָ) */
    {EM_FINGER_APP_FLOW9_DEL_CUSTOM,\
    EM_FINGER_FLOW4_DEL,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,\
    EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE,        EM_FINGER_FLOW_NONE},
};



/***************************** 2  ��̬����*****************************/

/****************************************************************************** 
* ��������app_finger_cfg_reset
* �� �ܣ�ָ��ҵ��λ/ �ָ�
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_cfg_reset(void)
{
    /* ��EE �ָ� */
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
* ��������app_finger_cfg_reset
* �� �ܣ�ָ����ͨ�û�ָ�Ƹ�λ/ �ָ�
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_cfg_reset_common(void)
{
    /* ��EE �ָ� */
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
* ��������app_finger_cfg_get_nullnum
* �� �ܣ�ָ��ҵ�����ã��õ���Ӧ����δʹ�õĿյ����
* �� �룺void
* �� ����uint16_t* _pu16Num ���ؿյ����
* �� �أ�bool
*/
static bool app_finger_cfg_get_nullnum(FINGER_APP_USER_TYPE_E _em, uint16_t* _pu16Num)
{
    FingerAppCfg_S stCfg;
    *_pu16Num = 0;
    uint8_t i = (EM_FINGER_APP_TYPE_COMMON == _em && ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister)? M_FINGER_MAX_ADMIN_NUM : 0;
    for(; i < MSG_FINGER_NUM_RESERVED; i++)
    {
        /* ����Ա��֧�����M_FINGER_MAX_ADMIN_NUM �� */
        if(EM_FINGER_APP_TYPE_ADMIN == _em && M_FINGER_MAX_ADMIN_NUM == i) { break;}
        /* ����0xFF��0x00 �������  */
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
* ��������app_finger_cfg_write
* �� �ܣ�ָ��ҵ�����ã�д����Ӧָ����������
* �� �룺FingerAppCfg_S _stCfg
* �� ����void
* �� �أ�bool
*/
static bool app_finger_cfg_write(FingerAppCfg_S _stCfg)
{
    /* ��������������һ�£��Ȼ�ȡ����� */
    uint16_t u16Num = (_stCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] << 8) + _stCfg.acOffset[EM_FINGER_APP_CFG_NUM_L];

    /* �ж���ŵ���Ч�� */
    if(u16Num >= MSG_FINGER_NUM_RESERVED) {return false;}
    
    /* ����У��� */
    uint16_t u16CheckSum = 0;
    for(uint8_t i = 0; i < FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK; i++) {u16CheckSum += _stCfg.acOffset[i];}
    _stCfg.acOffset[EM_FINGER_APP_CFG_CHECK_H] = (uint8_t)(u16CheckSum>>8);
    _stCfg.acOffset[EM_FINGER_APP_CFG_CHECK_L] = (uint8_t)(u16CheckSum);
    _stCfg.acOffset[EM_FINGER_APP_CFG_EN] = FINGER_APP_CFG_EN;
    
    FingerAppCfg_S stCfg = {0};
    if((HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)&stCfg, sizeof(stCfg)) > 0))
    {
        /* ����ǿյ�д�� */
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
        /* ����ǿյģ����� */
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
* ��������app_finger_cfg_read
* �� �ܣ�ָ��ҵ�����ã���ȡ��Ӧָ����������
* �� �룺uint16 _u16Num
* �� ����void
* �� �أ�bool
*/
static bool app_finger_cfg_read(uint16_t _u16Num, FingerAppCfg_S *_pstCfg)
{
    /* �ж���ŵ���Ч�� */
    if(_u16Num >= MSG_FINGER_NUM_RESERVED) {return false;}
    
    if(HAL_EEPROM_ReadBytes(FINGER_APP_CFG_REG_START+_u16Num*MSG_FINGER_ONE_SIZE, (uint8_t *)_pstCfg, sizeof(_pstCfg)) > 0)
    {
        return true;
    }
    
    return false;
}

/****************************************************************************** 
* ��������app_finger_cfg_write_null
* �� �ܣ�ָ��ҵ�����ã������Ӧָ����������
* �� �룺uint16 _u16Num
* �� ����void
* �� �أ�bool
*/
static bool app_finger_cfg_write_null(uint16_t _u16Num)
{
    /* �ж���ŵ���Ч�� */
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
* ��������app_finger_cfg_get_registid
* �� �ܣ�������ȡ�ѱ�ע���ID��
* �� �룺uint8_t _u8Len  ����_pu8ID ��ʵ�ʴ�С
* �� ����uint8_t *_pu8ID �����ַ, uint8_t *_pu8Len ʵ��д��ĳ���
* �� �أ�bool
*/
static bool app_finger_cfg_get_registid(uint8_t _u8Len, uint8_t *_pu8ID, uint8_t *_pu8Len)
{
    /* �ж���ŵ���Ч�� */
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
* ��������app_finger_cfg_get_customid
* �� �ܣ�������ӦID�ŵ�custom ID
* �� �룺uint16_t u16CustomID, uint16_t *pu16PageID
* �� ����bool
* �� �أ�bool
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
* ��������app_finger_cfg_customid_check
* �� �ܣ�custom ID ����
* �� �룺uint16_t u16CustomID, uint16_t *pu16PageID
* �� ����bool   true - customID ����ʹ��   false - customID �ظ�
* �� �أ�bool
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
* ��������app_finger_work_cnt
* �� �ܣ��ṩ���ⲿ��ʱ��(ÿ10�������һ��)������ҵ���߼��ӳ�
* �� �룺void 
* �� ����void
* �� �أ�uint8_t ���ؼ���ֵ
*/ 
static uint32_t app_finger_work_cnt(void)
{
    return (s_u32AppWorkCnt > 0) ? (s_u32AppWorkCnt--) : 0;
}

/****************************************************************************** 
* ��������app_finger_set_countdown
* �� �ܣ�����ִ��ҵ����ʱʱ��
* �� �룺uint8_t 
* �� ����void
* �� �أ�bool �����Ƿ�ɹ�
*/ 
static bool app_finger_work_set_10ms(uint32_t _u32AppWorkCnt)
{
    /* ҵ��������ת��ʱ�����˹��󣬷ŵ�10ms��ʱ������Ҫ*100  */
    if(_u32AppWorkCnt > 5000) { return false; }

    s_u32AppWorkCnt = _u32AppWorkCnt;
    return true;
}

/****************************************************************************** 
* ��������app_finger_get_countdown
* �� �ܣ���ȡִ��ҵ����ʱʱ��
* �� �룺void 
* �� ����void
* �� �أ�uint8_t ���ؼ���ֵ
*/ 
static uint32_t app_finger_work_get(void)
{
    return s_u32AppWorkCnt;
}

/****************************************************************************** 
* ��������app_finger_img_cnt
* �� �ܣ��ṩ���ⲿ��ʱ��(ÿ10�������һ��)��ÿһ�γ��Ի�ȡͼ�񣬼���һ�μ���
* �� �룺void 
* �� ����void
* �� �أ�uint8_t ���ؼ���ֵ
*/ 
static uint32_t app_finger_img_cnt(void)
{
    return (s_u32GetImageCnt > 0) ? (s_u32GetImageCnt--) : 0;
}

/****************************************************************************** 
* ��������app_finger_img_reset
* �� �ܣ�����ͼ������ʱ����λ�� 10ms
* �� �룺void 
* �� ����void
* �� �أ�void
*/ 
static void app_finger_img_reset( uint32_t _u32AppWorkCnt)
{
    /* �ŵ�10ms��ʱ������Ҫ*10   ,���˹��� */
    s_u32GetImageCnt = _u32AppWorkCnt;
    return;
}

/****************************************************************************** 
* ��������app_finger_img_get
* �� �ܣ���ȡ��ǰͼ���Դ����ļ���ֵ
* �� �룺void 
* �� ����void
* �� �أ�uint8_t ���ؼ���ֵ
*/ 
static uint32_t app_finger_img_get(void)
{
    return s_u32GetImageCnt;
}

/****************************************************************************** 
* ��������app_finger_led
* �� �ܣ�ָ��¼��ư���Ѳ����
* �� �룺uint8_t _u8Led
* �� ����void
* �� �أ�void
*/
static void app_finger_get_led(uint8_t _u8Led)
{
    if(EM_FINGER_APP_FLOW1_ADD != s_stFingerAppParam.emAppFlow && EM_FINGER_APP_FLOW5_SLIDE_ADD != s_stFingerAppParam.emAppFlow) { return; }
    
    /* ���� */
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
* ��������app_finger_check_time
* �� �ܣ� У����ָ�Ƿ���п���Ȩ���Լ�ID��ʱЧ��
* �� �룺void
* �� ����void
* �� �أ�bool
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
        
    /* ʱЧ���δ�򿪣�ֱ�ӳɹ� */
    if(0 == stFingermeg.acOffset[EM_FINGER_APP_CFG_TIME_EN])
    {
        my_printf("# FINGER # Open ID(%d) without check time \n", (uint32_t)s_stFingerAppParam.u16OutPad);
        ret = true;
    }
    /* ��ȡ��ӦID �� ���ã���ȡʱЧ��Ǻ�ʱ�� ��
                        ʱЧ��Ǵ򿪵�����²���ʱЧ��Χ�ж�*/
    else if( true == app_finger_cfg_read(s_stFingerAppParam.u16OutPad, &stFingermeg))
    {
        RTCType start = {stFingermeg.acOffset[EM_FINGER_APP_CFG_START_SEC], stFingermeg.acOffset[EM_FINGER_APP_CFG_START_MIN],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_START_HOUR], 0,
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_START_DAY],stFingermeg.acOffset[EM_FINGER_APP_CFG_START_MONTH],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_START_YEAR], 0};
        RTCType end = {stFingermeg.acOffset[EM_FINGER_APP_CFG_END_SEC], stFingermeg.acOffset[EM_FINGER_APP_CFG_END_MIN],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_END_HOUR], 0,
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_END_DAY],stFingermeg.acOffset[EM_FINGER_APP_CFG_END_MONTH],
                        stFingermeg.acOffset[EM_FINGER_APP_CFG_END_YEAR], 0};    //����ʱ��

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
* ��������app_finger_report
* �� �ܣ�������������ͳһ��װ�ӿ�
* �� �룺VoiceType_E _em
* �� ����void
* �� �أ�void
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
        /* �������飬�ȴ����һ����������ʱ���ٲ���������  */
        (void)HAL_Voice_PlayingVoice(EM_VOICE_BREAK_CMD, 0);
        PUBLIC_Delayms(300);

        (void)HAL_Voice_PlayingVoice(_em, holdTimeMs);
        /* ����1��ʱ�䲥�� */
        (void)app_finger_work_set_10ms(holdTimeMs/10);
        /* ��λָ��ģ�鳢�Բ�����ȡͼ��ļ���ֵ */
        (void)app_finger_img_reset(holdTimeMs);
    }
    
    return;
}

/****************************************************************************** 
* ��������app_finger_reset_flag
* �� �ܣ�ָ��ҵ��ӿڣ��������flag��Ǳ���
* �� �룺FINGER_APP_FLOW_RESULT_E
* �� ����void
* �� �أ�void
*/
static void app_finger_reset_flag(FINGER_APP_FLOW_RESULT_E _em, bool _bForce)
{
    /*  1. ״̬��ת�����У��쳣�˳� 
            2. ״̬��ת�����У�����ת����Ѹ�λ
            3. ǿ�Ƹ�λ(˯��) */
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
        /* ������Ƶ������� */
        app_finger_report(EM_VOICE_BREAK_CMD, false);
    }
    return;
}

/****************************************************************************** 
* ��������app_finger_ack_ok
* �� �ܣ� ���̷���ֵOK��������дEE
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_ack_ok(void)
{
    my_printf("# FINGER # app_finger_ack_ok \n");
    bool bRet = false;
    /* ������ */
    if(EM_FINGER_FLOW3_REG == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        /* ����дEE ���� */
        uint8_t u8AdminEn = s_stFingerAppParam.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ];
        FINGER_APP_USER_TYPE_E _em = (MEM_USER_MASTER == u8AdminEn)? EM_FINGER_APP_TYPE_ADMIN : EM_FINGER_APP_TYPE_COMMON;
        
        if(true == app_finger_cfg_get_nullnum(_em, &s_stFingerAppParam.u16OutPad))
        {
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_ADMIN_EN] = u8AdminEn;
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] = (uint8_t)(s_stFingerAppParam.u16OutPad>>8);
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_L] = (uint8_t)(s_stFingerAppParam.u16OutPad);
            s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_WEEK] = 0xFE;

            /* �洢�ɹ��򲥱��Ǽǳɹ� */
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
        // ָ����ǿ�����껬��¼������е�����ʱ��
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
            /* ���ָ���������̵ĵư� */
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
* ��������app_finger_ack_err
* �� �ܣ� ���̷���ֵERROR ��������дEE
* �� �룺uint8_t ack
* �� ����void
* �� �أ�bool
*/
static bool app_finger_ack_err(uint8_t ack)
{
    my_printf("# FINGER # app_finger_ack_err \n");
    isPutUpFinger = true;
    
    // �������̲���Ҫ����������ָ����
    if(EM_FINGER_FLOW0_GET == s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag] && EM_FINGER_APP_FLOW3_SEARCH != s_stFingerAppParam.emAppFlow)
    {
        /* ���ָ���������̵ĵư� */
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
                // ¼��ָ��ʱ��ת������ʧ�ܣ��������²�ͼ
                s_u8AppFlowOrderFlag -= 2;
                return true;
            }
#else
            // ¼��ָ��ʱ��ת������ʧ�ܣ��������²�ͼ
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
* ��������app_finger_add
* �� �ܣ� ����EM_FINGER_APP_FLOW1_ADD �� EM_FINGER_APP_FLOW5_SLIDE_ADD ���� ��
                     ��Ի���¼��ָ������������ֵ
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_add(void)
{
    /* ��EM_FINGER_APP_FLOW5_SLIDE_ADD ������ */
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
                    /* ָ��ģ�������ָ���Ϸ���¼�벻�ɹ��������� */
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
* ��������app_finger_search
* �� �ܣ� ����EM_FINGER_APP_FLOW3_SEARCH ���� �� ���EM_FINGER_FLOW2_SEARCH ָ��ͼ���ȡ��ҵ���Դ���
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_search(void)
{
    /* ��EM_FINGER_APP_FLOW3_SEARCH ������ */
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
* ��������app_finger_delete
* �� �ܣ� ����EM_FINGER_APP_FLOW2_DEL ���� �� ���EM_FINGER_FLOW4_DEL ָ��ͼ���ȡ��ҵ���Դ���
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_delete(void)
{
    /* ��EM_FINGER_APP_FLOW2_DEL ������ */
    if(EM_FINGER_APP_FLOW2_DEL != s_stFingerAppParam.emAppFlow) {return true;}
    
    if(0 != s_u8AppFlowOrderFlag && EM_FINGER_FLOW_NONE != s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag])
    {
        switch(FINGER_GetAckCode())
        {
            case FINGER_ACK_OK:
                {
                    uint16_t u16Pad = (s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_H] << 8) + s_stFingerAppParam.stFingerAppCfg.acOffset[EM_FINGER_APP_CFG_NUM_L];
                    /* ģ���ѷ���ɾ���ɹ��ˣ�����EEд���Ƿ�ɹ����ж������� */
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
* ��������app_finger_clear
* �� �ܣ� ����EM_FINGER_APP_FLOW0_CLEAR ���� �� ���EM_FINGER_FLOW5_EMPTY ָ��ͼ���ȡ��ҵ���Դ���
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_clear(void)
{
    /* ��EM_FINGER_APP_FLOW0_CLEAR ������ */
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
* ��������app_finger_clear_common
* �� �ܣ� ����EM_FINGER_APP_FLOW4_CLEAR_COMMON ���� �� ���EM_FINGER_FLOW5_EMPTY ָ��ͼ���ȡ��ҵ���Դ���
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_clear_common(void)
{
    /* ��EM_FINGER_APP_FLOW4_CLEAR_COMMON ������ */
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
* ��������app_finger_ack_scan
* �� �ܣ� ��Э�鷵�ؽ������ 
* �� �룺void
* �� ����void
* �� �أ�bool
*/
static bool app_finger_ack_scan(void)
{
    return (!app_finger_add()|| !app_finger_search()|| !app_finger_delete()|| !app_finger_clear()|| !app_finger_clear_common()) ? false : true;
}

/****************************************************************************** 
* ��������app_finger_inparam_scan
* �� �ܣ� ״̬��ת��׼����κ�Ԥ�ƶ���
* �� �룺uint16_t *_pu16Pad
* �� ����void
* �� �أ�bool
*/
static bool app_finger_inparam_scan(uint32_t *_pu32Pad)
{
    /* ״̬��ת���Ƿ�������λ */
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
            /* ¼��ָ�ƻ�ָ����ʱ����һ��¼�벻���Ҳ����Ҫ̧����ָ���� */
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
            /* ����MAP s_u8AppFlowMap �����̷����õ��ļ��㷽ʽ���õ���u16PadΪʵ����Ҫд���Ӧbuffer �ı�� */
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


/***************************** 3 ���⺯��*****************************/

/****************************************************************************** 
* ��������APP_FINGER_Init
* �� �ܣ�ָ�ƶ���ɨ��
* �� �룺void
* �� ����void
* �� �أ�void
*/
void APP_FINGER_Init(void)
{
    /* ���ø�λָ��ҵ�����ڲ����� */
    (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false);
    
    /* ָ��Э������ʼ��������ָ��ģ�鹦�� */
    (void)FINGER_Init();
    
    return;
}

/****************************************************************************** 
* ��������APP_FINGER_Sleep
* �� �ܣ�����
* �� �룺void
* �� ����void
* �� �أ�void
*/
void APP_FINGER_Sleep(void)
{
    (void)app_finger_reset_flag(FINGER_APP_RESULT_IDLE, true);

    return;
}

/****************************************************************************** 
* ��������App_FINGER_Tim10Ms
* �� �ܣ�10MS ��ʱ�����ú���
* �� �룺void
* �� ����void
* �� �أ�void
*/
void App_FINGER_Tim10Ms(void)
{
    (void)app_finger_work_cnt();
    (void)app_finger_img_cnt();

    /*�ڲ�1S ��ʱ*/
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
* ��������APP_FINGER_Operate
* �� �ܣ�ָ�Ʋ����ϼ�
* �� �룺FingerAppParam_S _stFingerAppParam
* �� ����void
* �� �أ�bool
*/
bool APP_FINGER_Operate(FingerAppParam_S _stFingerAppParam)
{
    /* ҵ�������ж� */
    if(EM_FINGER_APP_FLOW_NONE != _stFingerAppParam.emAppFlow && _stFingerAppParam.emAppFlow >= EM_FINGER_APP_FLOW_ALL) { return false; }
    
    /* ������ֱ�ӷ��أ�������������ָ�� */
    uint16_t u16Pad = 0;
    uint8_t u8AdminEn = _stFingerAppParam.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ];
    FINGER_APP_USER_TYPE_E _em = (MEM_USER_MASTER == u8AdminEn)? EM_FINGER_APP_TYPE_ADMIN : EM_FINGER_APP_TYPE_COMMON;
    if(EM_FINGER_APP_FLOW1_ADD == _stFingerAppParam.emAppFlow && false == app_finger_cfg_get_nullnum(_em, &u16Pad)) { return false; }

#ifdef FINGER_VEIN_FUNCTION_ON
    /* ָ��������Ϊ����¼�� */
    if(EM_FINGER_APP_FLOW1_ADD == _stFingerAppParam.emAppFlow) {_stFingerAppParam.emAppFlow = EM_FINGER_APP_FLOW5_SLIDE_ADD;isPutUpFinger = false;}
#endif

    /* ����֧�ִ�ϣ������Ҫ��ģʽ˯�ߡ���λ */
    if(FINGER_APP_RESULT_RUNNING == s_stFingerAppParam.emFlowResult)
    {
        (void)FINGER_Sleep();
    }

    /* �������̲�����μ�� */
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

    /* ÿ�β���ָ��ģ����Ҫ����ģ�� */
    (void)FINGER_WakeUp();

    /* ��¼��ǰҵ������״ֵ̬*/
#ifdef FINGER_PROTOCAL_V2_SUPPORT
    s_u8FingerProtocalV2Cnt = 0;
#endif
    s_u8AppFlowOrderFlag = 0;
    s_stFingerAppParam.u16OutPad = 0;
    s_stFingerAppParam.emFlowResult = FINGER_APP_RESULT_RUNNING;
    s_stFingerAppParam.emAppFlow = _stFingerAppParam.emAppFlow;
    memcpy((void*)&s_stFingerAppParam.stFingerAppCfg, (void*)&_stFingerAppParam.stFingerAppCfg, sizeof(s_stFingerAppParam.stFingerAppCfg));
    /* ��λָ��ģ�鳢�Բ�����ȡͼ��ļ���ֵ */
    (void)app_finger_img_reset(100);
    
    return true;
}

/****************************************************************************** 
* ��������APP_FINGER_GetFlowResult
* �� �ܣ�����ָ��ҵ�����̽��
* �� �룺void
* �� ����uint16_t *pu16Pad
* �� �أ�FINGER_APP_FLOW_RESULT_E
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
* ��������APP_FINGER_GetFlowResult
* �� �ܣ����������ѱ�ע���ID��
* �� �룺uint8_t _u8Len  ����_pu8ID ��ʵ�ʴ�С
* �� ����uint8_t *_pu8ID �����ַ, uint8_t *_pu8Len ʵ��д��ĳ���
* �� �أ�void
*/ 
bool APP_FINGER_GetFingerID(uint8_t _u8Len, uint8_t *_pu8ID, uint8_t *_pu8Len)
{
    return app_finger_cfg_get_registid(_u8Len, _pu8ID, _pu8Len);
}

    
/****************************************************************************** 
* ��������APP_FINGER_GetPowerFlag
* �� �ܣ�����ָ��ģ����ϵ���
* �� �룺void
* �� ����void
* �� �أ�uint8_t
*/ 
uint8_t APP_FINGER_GetPowerFlag(void)
{
    return FINGER_GetPowerFlag();
}

/****************************************************************************** 
* ��������APP_FINGER_GetProtocalVersion
* �� �ܣ�����ָ��Э��汾
* �� �룺void
* �� ����void
* �� �أ�uint8_t
*/ 
uint8_t APP_FINGER_GetProtocalVersion(void)
{
    return FINGER_GetProtocalVersion();
}


/****************************************************************************** 
* ��������APP_FINGER_GetFlowResult
* �� �ܣ�����ָ��ҵ�����̽��
* �� �룺void
* �� ����void
* �� �أ�FINGER_APP_FLOW_RESULT_E
*/ 
FINGER_APP_ADD_FLOW_E APP_FINGER_GetAddFingerFlow(void)
{
    /* ��ǰ�����ADD���̣�������״̬�������ⲿ��ֹ���̽���¼��ָ�� */
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
* ��������APP_FINGER_Get_Breakdown
* �� �ܣ���ȡ�豸�������
* �� �룺void
* �� ����void
* �� �أ�bool - true - ���� false - ����
*/ 
bool APP_FINGER_Get_Breakdown(void)
{
    (void)APP_FINGER_Init();

    bool bPowerStatus = FINGER_GetPowerStatus();

    (void)APP_FINGER_Sleep();
    
    return bPowerStatus;
}


/****************************************************************************** 
* ��������APP_FINGER_Scan
* �� �ܣ�״̬��ѵ��ͨ��Ԥ��MAP����Ϊ�Զ�����FINGER�ӿڣ�ִ��ҵ������
* �� �룺void
* �� ����void
* �� �أ�void
*/
void APP_FINGER_Scan(void)
{
    /* ָ��ģ��Э��ִ����Ѳ */
    (void)FINGER_Scan();

    /* ����ģ�鳬ʱ */
    if(EM_FINGER_FLOW_NONE == FINGER_GetFlowStatus() && FINGER_GetTxRxFlag())  { (void)app_finger_reset_flag(FINGER_APP_RESULT_TIMEOUT, false); return; }

    /* ģ��δ�ϵ磬��ִ��ҵ�� */
    if(false == FINGER_GetPowerStatus() && EM_FINGER_FLOW_NONE == FINGER_GetFlowStatus()) 
    {
        return;
    }
 
    /* ҵ�������ж� */
    if(s_stFingerAppParam.emAppFlow >= EM_FINGER_APP_FLOW_ALL) { (void)app_finger_reset_flag(FINGER_APP_RESULT_FAIL, false); return; }

    /* Э�����̵�ǰ������״ִ̬��ʱ������ */
    if(EM_FINGER_FLOW_NONE != FINGER_GetFlowStatus() || FINGER_APP_RESULT_RUNNING != s_stFingerAppParam.emFlowResult) {return;}


    /* ҵ�����ʱ��ɺ���ִ�� */
    if(0 == app_finger_work_get())
    {
        /* ��Э�鷵�ؽ������ */
        if(false == app_finger_ack_scan()) {return;}

        /* ״̬��ת��׼����κ�Ԥ�ƶ��� */
        uint32_t u32Pad = 0;
        if(false == app_finger_inparam_scan(&u32Pad)) {return;}

        /* ִ��Э�鶯�� */
        (void)FINGER_Operate((FINGER_FLOW_TYPE_E)s_u8AppFlowMap[s_stFingerAppParam.emAppFlow][s_u8AppFlowOrderFlag], u32Pad);
    }
    
    return;
}

/****************************************************************************** 
* ��������APP_FINGER_CfgGetNullNum
* �� �ܣ�ָ��ҵ�����ã��õ���Ӧ����δʹ�õĿյ����
* �� �룺void
* �� ����uint16_t* _pu16Num ���ؿյ����
* �� �أ�bool
*/
bool APP_FINGER_CfgGetNullNum(FINGER_APP_USER_TYPE_E _em, uint16_t* _pu16Num)
{
    return app_finger_cfg_get_nullnum(_em, _pu16Num);
}

/****************************************************************************** 
* ��������APP_FINGER_CfgWrite
* �� �ܣ�ָ��ҵ�����ã�д����Ӧָ����������
* �� �룺FingerAppCfg_S _stCfg
* �� ����void
* �� �أ�bool
*/
bool APP_FINGER_CfgWrite(FingerAppCfg_S _stCfg)
{
    return app_finger_cfg_write(_stCfg);
}

/****************************************************************************** 
* ��������APP_FINGER_CfgRead
* �� �ܣ�ָ��ҵ�����ã���ȡ��Ӧָ����������
* �� �룺uint16 _u16Num
* �� ����void
* �� �أ�bool
*/
bool APP_FINGER_CfgRead(uint16_t _u16Num, FingerAppCfg_S *_pstCfg)
{
    return app_finger_cfg_read(_u16Num, _pstCfg);
}

/****************************************************************************** 
* ��������APP_FINGER_CfgCheck_CustomID
* �� �ܣ�ָ��customID ����
* �� �룺uint16 _u16Num
* �� ����void
* �� �أ�bool
*/
bool APP_FINGER_CfgCheck_CustomID(uint16_t _u16CustomID)
{
    return app_finger_cfg_customid_check(_u16CustomID);
}

