/**************************************************************************** 
* Copyright (C), 2008-2021,��ʩ�����磨�й������޹�˾ 
* �ļ���: APP_Update.c 
* ���ߣ�����ع
* �汾��V01
* ʱ�䣺20220105
* ���ݼ�����ҵ��ͨ����������(С��Э����գ�I2C����ģ��)
****************************************************************************/
/* ��׼ͷ�ļ� */
#include <stdint.h>   

/* �ڲ�ͷ�ļ� */

/* �ⲿͷ�ļ� */

#ifndef _APP_UPDATE_H_
#define _APP_UPDATE_H_

#define APP_UPDATE_PACK_CACHE_CNT   16               // ��������
#define APP_UPDATE_PACK_CACHE_LEN   512             // ���������С
#define APP_UPDATE_PACK_MAX (APP_UPDATE_PACK_CACHE_CNT*APP_UPDATE_PACK_CACHE_LEN)   // ��������С


/* ͨ��OTA ��������ö�� */
typedef enum
{
    EM_APP_UPDATE_NONE = 0,         //��Ч���̣�Ĭ��ֵ
    EM_APP_UPDATE_TYPE_VOICE,       // ����оƬOTA
    EM_APP_UPDATE_TYPE_SCREEN,      // ������������ OTA
    EM_APP_UPDATE_TYPE_EMOJI,       // ����������� OTA
    /* ���������� */
    EM_APP_UPDATE_ALL,              //���������ޣ������ֵ
}APP_UPDATE_TYPE_E; 


typedef struct
{
	uint8_t     Enabled;
	uint8_t     Step;
	uint8_t     I2cSpeed;
    uint8_t     SinglePackageSize;  //A15: ���ݰ������
	uint32_t    Package;
	
	uint32_t    FileSize;
	uint32_t    offset;
	uint32_t    FileChack;          // A7/A8/A9/A10: �̼�У���
	uint32_t    RxChack;            // A11/A12/A13/A14: �̼�У���
	
	uint16_t    stage;              // A16/A17: �׶ΰ�
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

