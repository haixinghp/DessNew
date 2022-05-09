/*********************************************************************************************************************
 * @file:        App_Finger.h
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-07-26
 * @Description: ָ��/ָ����ҵ��ģ��
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_FINGER_H_
#define  _APP_FINGER_H_

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "LockConfig.h"
#include "System.h"
#include "../SERVER/Finger.h"

/*--------------------------------------------------�궨��-----------------------------------------------------------*/
/* ָ��ģ�黺������ */
#define FINGER_BUFFER_NUM_1 0x01
#define FINGER_BUFFER_NUM_2 0x02
#define FINGER_BUFFER_NUM_3 0x03
#define FINGER_BUFFER_NUM_4 0x04
#define FINGER_BUFFER_NUM_5 0x05
#define FINGER_BUFFER_NUM_6 0x06

/* ҵ������- ����ָ��ģ��Э���������� */
#define FINGER_APP_FLOW_MAX 16

/* ҵ������ - ��N�ν�����������Ӧ������ */
#define FINGER_APP_CONVERT_1 1
#define FINGER_APP_CONVERT_2 2
#define FINGER_APP_CONVERT_3 3
#define FINGER_APP_CONVERT_4 4
#define FINGER_APP_CONVERT_5 5
#define FINGER_APP_CONVERT_6 6

/* ָ��ģ����EE �Ϸ������ʼ��ַ */
#ifndef MSG_FINGER_REG_START
#define FINGER_APP_CFG_REG_START 0x400
#else
#define FINGER_APP_CFG_REG_START MSG_FINGER_REG_START
#endif

/* ָ��ģ�鵥��ָ�����õ���������С  */
#ifndef MSG_FINGER_ONE_SIZE
#define FINGER_APP_CFG_ONE_SIZE 64
#else
#define FINGER_APP_CFG_ONE_SIZE MSG_FINGER_ONE_SIZE
#endif

/* ������У�������Ч����λ�� */
#define FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK (FINGER_APP_CFG_ONE_SIZE - 2)

#define FINGER_APP_CFG_EN       'W'     // ָ��ʹ��ʹ�ܱ��
#define FINGER_APP_CFG_MANAGER  'M'     // ����Ա�û����
#define FINGER_APP_CFG_GUESS    'G'     // ��ͨ�û����


/*--------------------------------------------------ö������---------------------------------------------------------*/

/* ָ��ģ��ҵ�������� */
typedef enum
{
    EM_FINGER_APP_FLOW0_CLEAR = 0,      // ���ָ��
    EM_FINGER_APP_FLOW1_ADD,            // ����ָ��
    EM_FINGER_APP_FLOW2_DEL,            // ɾ��ָ��
    EM_FINGER_APP_FLOW3_SEARCH,         // ����ƥ��
    EM_FINGER_APP_FLOW4_CLEAR_COMMON,   // �����ͨ�û�
    EM_FINGER_APP_FLOW5_SLIDE_ADD,      // ��������ָ����
    EM_FINGER_APP_FLOW6_AGING,          // ָ�����ϻ�����
    EM_FINGER_APP_FLOW7_68_SLEEP,       // 0x68ָ��ģ����������
    EM_FINGER_APP_FLOW8_CLEAR_V2,       // ���ָ��(����0x60 ��ʽ,��Ӧ FINGER_PROTOCAL_V3)
    EM_FINGER_APP_FLOW9_DEL_CUSTOM,     // ɾ��ָ��(ͨ��customID ɾ����ָ)
    EM_FINGER_APP_FLOW_ALL,             // ���������ޣ������ֵ

    EM_FINGER_APP_FLOW_NONE = 0xFF,     // ��Ч���̣�Ĭ��ֵ
}FINGER_APP_FLOW_TYPE_E; 

/* ָ��ҵ������״ִ̬�н�� */
typedef enum
{
    FINGER_APP_RESULT_IDLE = 0,         // ����
    FINGER_APP_RESULT_RUNNING,          // ������    
    
    FINGER_APP_RESULT_SUC,              // ִ�гɹ�
    FINGER_APP_RESULT_FAIL,             // ִ��ʧ��
    FINGER_APP_RESULT_TIMEOUT,          // ��ʱ
    FINGER_APP_PROTOCAL_ERR,            // Э������쳣
    FINGER_APP_RESULT_ALL,              //�������ֵ
}FINGER_APP_FLOW_RESULT_E; 

/* ָ��ҵ��¼������״̬ö�� */
typedef enum
{
    FINGER_APP_ADD_NONE = 0,            // ��״̬
    FINGER_APP_ADD_1ST,                 // ��һ��¼��
    FINGER_APP_ADD_2ND,                 // �ڶ���¼��
    FINGER_APP_ADD_3RD,                 // ������¼��
    FINGER_APP_ADD_4TH,                 // ���Ĵ�¼��
    FINGER_APP_ADD_5TH,                 // �����¼��
    FINGER_APP_ADD_6TH,                 // ������¼��
    FINGER_APP_ADD_OVER,                // ģ����ָ��
    FINGER_APP_ADD_ALL,                 //�������ֵ
}FINGER_APP_ADD_FLOW_E; 

/* ָ���������� */
typedef enum
{
    EM_FINGER_APP_TYPE_ADMIN = 0,       // ����Ա�û�
    EM_FINGER_APP_TYPE_COMMON,          // ��ͨ�û�
    EM_FINGER_APP_TYPE_ALL              // �����û�
}FINGER_APP_USER_TYPE_E;


/* ָ��ģ�鵥�������ڴ���� 

0xX00                       0xX01                       0xX02                       0xX03                       0xX04                       0xX05                       0xX06                       0xX07
ָ��ʹ��        ģ����H       ģ����L       ����Ա���      ������        SOSв�ȱ��     ʱЧ���                ��
0xX08                       0xX09                       0xX0A                       0xX0B                      0xX0C                       0xX0D                       0xX0E                       0xX0F
��ʼ��                  ��                          ��                          ʱ                          ��                          ��                          ������                  ��

0xX10                       0xX11                       0xX12                       0xX13                       0xX14                       0xX15                       0xX16                       0xX17
��                             ʱ                          ��                          ��                         Ԥ��                      Ԥ��                    Ԥ��                     Ԥ��
0xX18                       0xX19                       0xX1A                       0xX1B                      0xX1C                       0xX1D                       0xX1E                       0xX1F
Ԥ��                      Ԥ��                     Ԥ��                    Ԥ��                     Ԥ��                     Ԥ��                       Ԥ��                    Ԥ��

0xX20                       0xX21                       0xX22                       0xX23                       0xX24                       0xX25                       0xX26                       0xX27
Ԥ��                      Ԥ��                     Ԥ��                    Ԥ��                     Ԥ��                     Ԥ��                       Ԥ��                    Ԥ��
0xX28                       0xX29                       0xX2A                       0xX2B                      0xX2C                       0xX2D                       0xX2E                       0xX2F
Ԥ��                      Ԥ��                     Ԥ��                    Ԥ��                     Ԥ��                     Ԥ��                       Ԥ��                    Ԥ��

0xX30                       0xX31                       0xX32                       0xX33                       0xX34                       0xX35                       0xX36                       0xX37
Ԥ��                      Ԥ��                     Ԥ��                    Ԥ��                     Ԥ��                     Ԥ��                       Ԥ��                    Ԥ��
0xX38                       0xX39                       0xX3A                       0xX3B                      0xX3C                       0xX3D                       0xX3E                       0xX3F
Ԥ��                      Ԥ��                     Ԥ��                    Ԥ��                     Ԥ��                     Ԥ��                    У��λH            У��λL


*/
/* ָ��ģ�鵥�������ڴ���� */
typedef enum
{
    EM_FINGER_APP_CFG_EN = 0x00,            // ָ��ʹ��
    EM_FINGER_APP_CFG_NUM_H,                // ģ����H
    EM_FINGER_APP_CFG_NUM_L,                // ģ����L
    EM_FINGER_APP_CFG_ADMIN_EN,             // ����Ա���
    EM_FINGER_APP_CFG_FAMILY_EN,            // ������
    EM_FINGER_APP_CFG_SOS_EN,               // SOSв�ȱ�� 
    EM_FINGER_APP_CFG_TIME_EN,              // ʱЧ���  
    EM_FINGER_APP_CFG_WEEK,                 // ��

    
    EM_FINGER_APP_CFG_START_YEAR = 0x08,    // ��ʼʱ��: ��
    EM_FINGER_APP_CFG_START_MONTH,          // ��ʼʱ��: ��
    EM_FINGER_APP_CFG_START_DAY,            // ��ʼʱ��: ��
    EM_FINGER_APP_CFG_START_HOUR,           // ��ʼʱ��: ʱ
    EM_FINGER_APP_CFG_START_MIN,            // ��ʼʱ��: ��
    EM_FINGER_APP_CFG_START_SEC,            // ��ʼʱ��: ��
    EM_FINGER_APP_CFG_END_YEAR,             // ����ʱ��: ��
    EM_FINGER_APP_CFG_END_MONTH,            // ����ʱ��: ��
    
    EM_FINGER_APP_CFG_END_DAY = 0x10,       // ����ʱ��: ��
    EM_FINGER_APP_CFG_END_HOUR,             // ����ʱ��: ʱ
    EM_FINGER_APP_CFG_END_MIN,              // ����ʱ��: ��
    EM_FINGER_APP_CFG_END_SEC,              // ����ʱ��: ��
    EM_FINGER_APP_CFG_CUSTOM_ID_H,          // �û��Զ���ID ��λH
    EM_FINGER_APP_CFG_CUSTOM_ID_L,          // �û��Զ���ID ��λL
    
    EM_FINGER_APP_CFG_VALID_MAX,            // ��Ч���ֵ
    
    EM_FINGER_APP_CFG_CHECK_H = FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK,      // У���H
    EM_FINGER_APP_CFG_CHECK_L = FINGER_APP_CFG_ONE_SIZE_WITHOUT_CHECK+1,    // У���L

}FINGER_APP_CFG_OFFSET_E; 

/*--------------------------------------------------��������---------------------------------------------------------*/

/* ָ��ģ�鵥�����÷��� */
typedef struct
{
    uint8_t  acOffset[FINGER_APP_CFG_ONE_SIZE];  // ָ�Ƶ������ô�С��offset����ƫ�Ƹ���ö��FINGER_APP_CFG_OFFSET_Eȷ��
}FingerAppCfg_S; 

/* ָ��ģ��ҵ�������̼���νṹ�� */
typedef struct
{
    /* ��� */
    FINGER_APP_FLOW_TYPE_E  emAppFlow;          // ��¼��ǰҵ��������תֵ
    FingerAppCfg_S          stFingerAppCfg;     // ¼�롢�޸�ָ�����ø�ֵ����
    
    /* ���� */
    FINGER_APP_FLOW_RESULT_E    emFlowResult;   // ���н��,��Ϊ�����Ч
    uint16_t                    u16OutPad;      // �������Σ�Ŀǰ����֧����ο���Ľ�ADD��DELָ�ƣ����ڴ���ģ���
    
}FingerAppParam_S; 

/*--------------------------------------------------��������---------------------------------------------------------*/

/* ҵ��ӿ� */
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

/* ҵ��:  ָ��ҵ����ʱ�����ӿ�*/
void App_FINGER_Tim10Ms(void);

/*  ����ҵ��:  �����ӿ�*/
bool APP_FINGER_CfgWrite(FingerAppCfg_S _stCfg);
bool APP_FINGER_CfgRead(uint16_t _u16Num, FingerAppCfg_S *_pstCfg);
bool APP_FINGER_CfgGetNullNum(FINGER_APP_USER_TYPE_E _em, uint16_t* _pu16Num);
bool APP_FINGER_CfgCheck_CustomID(uint16_t _u16CustomID);


#endif



