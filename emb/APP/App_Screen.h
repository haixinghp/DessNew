/*********************************************************************************************************************
 * @file:        APP_Screen.h
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-09-02
 * @Description: ����������
 * @ChangeList:  01. ����
**********************************************************************************************************************/
  
#ifndef  _APP_SCREEN_H_
#define  _APP_SCREEN_H_

/*--------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "LockConfig.h"

/*--------------------------------------------------�궨��-----------------------------------------------------------*/

/* ҵ������- ����������ģ��Э���������� */
#define SCREEN_APP_FLOW_MAX 6

/*--------------------------------------------------ö������---------------------------------------------------------*/

/* ģ��ҵ�������� */
typedef enum
{
    EM_SCREEN_APP_FLOW0_SHOW_MOTION,    // չʾ��ͼ
    EM_SCREEN_APP_FLOW1_SHOW_COVER,     // չʾͼ��
    EM_SCREEN_APP_FLOW2_SWITCH_VER,     // �л���ͼ�汾
    EM_SCREEN_APP_FLOW3_GET_VER,        // ��ȡ��ͼ�汾
    EM_SCREEN_APP_FLOW4_SWITCH_BASE,    // �л��������Զ���
    EM_SCREEN_APP_FLOW5_UPDATE,         // ������������
    EM_SCREEN_APP_FLOW6_UPDATE_DATA,    // ��������������
    EM_SCREEN_APP_FLOW7_FACTORY_RESET,  // ��������������
    EM_SCREEN_APP_FLOW8_SCREEN_SHOW,    // ����չʾ
    EM_SCREEN_APP_FLOW_ALL,             // ���������ޣ������ֵ

    EM_SCREEN_APP_FLOW_NONE = 0xFF,     // ��Ч���̣�Ĭ��ֵ
}SCREEN_APP_FLOW_TYPE_E;

/* EM_SCREEN_APP_FLOW0_SHOW_MOTION - չʾ��ͼ������ϸö�� */
typedef enum
{
    /* 0x0100 ���� */
    //EM_SCREEN_FLOW0_SMILE_FACE,         // Ц�� 01 00 00 00
    //EM_SCREEN_FLOW0_SORRY_FACE,         // ���� 02 00 00 00
    //EM_SCREEN_FLOW0_CRY_FACE,           // ���� 03 00 00 00
    //EM_SCREEN_FLOW0_BLINK_FACE,         // գ�� 04 00 00 00

    /* 0x0001 ����ʾ */
    EM_SCREEN_FLOW0_SAFETY_TIP,         // ��ȫ��ʾ00 01 00 01
    EM_SCREEN_FLOW0_NOLOCK,             // ��δ����00 01 00 02
    EM_SCREEN_FLOW0_LOW_POWER,          // ��ص�ѹ00 01 00 03
        
    /* 0x0002 �������� */
    EM_SCREEN_FLOW0_FACE_SUC,           // ���������ɹ� 00 02 00 01
    EM_SCREEN_FLOW0_FINGER_SUC,         // ָ�ƽ����ɹ� 00 02 00 02
    //EM_SCREEN_FLOW0_VEIN_SUC,           // ��ָ�����ɹ� 00 02 00 03
    //EM_SCREEN_FLOW0_IRIS_SUC,           // ��Ĥ�����ɹ� 00 02 00 04
    EM_SCREEN_FLOW0_PWD_SUC,            // ��������ɹ� 00 02 00 05
    //EM_SCREEN_FLOW0_CARD_SUC,           // �ſ������ɹ� 00 02 00 06
    //EM_SCREEN_FLOW0_BLE_SUC,            // ���������ɹ� 00 02 00 07
    EM_SCREEN_FLOW0_FACE_FAIL,          // ��������ʧ�� 00 02 01 01
    //EM_SCREEN_FLOW0_FACE_TOOCLOSE,      // ���������       00 02 01 02
    //EM_SCREEN_FLOW0_FACE_TOOFARAWAY,    // ��������Զ       00 02 01 03
    //EM_SCREEN_FLOW0_FACE_KEEPOUT,       // �������ڵ�       00 02 01 04
    //EM_SCREEN_FLOW0_FACE_MISS,          // ����δ��⵽ 00 02 01 05
    EM_SCREEN_FLOW0_FINGER_FAIL,        // ָ�ƽ���ʧ�� 00 02 02 01
    //EM_SCREEN_FLOW0_FINGER_MISS,        // ָ��δ��⵽ 00 02 02 02
    //EM_SCREEN_FLOW0_VEIN_FAIL,          // ��ָ����ʧ�� 00 02 03 01
    //EM_SCREEN_FLOW0_VEIN_MISS,          // ��ָδ��⵽ 00 02 03 02
    //EM_SCREEN_FLOW0_IRIS_FAIL,          // ��Ĥ����ʧ�� 00 02 04 01
    //EM_SCREEN_FLOW0_IRIS_MISS,          // ��Ĥδ��⵽ 00 02 04 02
    EM_SCREEN_FLOW0_PWD_FAIL,           // ��������ɹ� 00 02 05 01
    //EM_SCREEN_FLOW0_CARD_FAIL,          // �ſ������ɹ� 00 02 06 01

    /* 0x0003 ���� */
    EM_SCREEN_FLOW0_SUNNY_DAY,          // ���� 00 03 00 01
    EM_SCREEN_FLOW0_CLOUDY_DAY,         // ���� 00 03 00 02
    EM_SCREEN_FLOW0_OVERCAST_DAY,       // ���� 00 03 00 03
    EM_SCREEN_FLOW0_RAINY_DAY,          // ���� 00 03 00 04
    EM_SCREEN_FLOW0_SNOWY_DAY,          // ѩ�� 00 03 00 05
    //EM_SCREEN_FLOW0_BLUE_ALERT,         // ������ɫԤ�� 00 03 01 01
    //EM_SCREEN_FLOW0_YELLOW_ALERT,       // ������ɫԤ�� 00 03 01 02
    //EM_SCREEN_FLOW0_ORANGE_ALERT,       // ������ɫԤ�� 00 03 01 03
    //EM_SCREEN_FLOW0_RED_ALERT,          // ������ɫԤ�� 00 03 01 04
        
    /* 0x0004 ���� */
    EM_SCREEN_FLOW0_HOLIDAY_0101,       // Ԫ��       00 04 00 01
    EM_SCREEN_FLOW0_HOLIDAY_0214,       // ���˽� 00 04 00 02
    EM_SCREEN_FLOW0_HOLIDAY_0501,       // �Ͷ��� 00 04 00 03
    EM_SCREEN_FLOW0_HOLIDAY_0601,       // ��ͯ�� 00 04 00 04
    EM_SCREEN_FLOW0_HOLIDAY_1001,       // ����� 00 04 00 05
    EM_SCREEN_FLOW0_HOLIDAY_1225,       // ʥ��       00 04 00 06
    EM_SCREEN_FLOW0_LUNAR_1230,         // ��Ϧ       00 04 01 01
    EM_SCREEN_FLOW0_LUNAR_0115,         // Ԫ���� 00 04 01 02
    EM_SCREEN_FLOW0_LUNAR_0505,         // ����� 00 04 01 03
    EM_SCREEN_FLOW0_LUNAR_0815,         // ����� 00 04 01 04
    EM_SCREEN_FLOW0_LUNAR_0707,         // ��Ϧ       00 04 01 05
        
    /* 0x0005 ���鶯�� */
    EM_SCREEN_FLOW0_ACT_BASE,           // ��������(��������) 00 05 00 01
    EM_SCREEN_FLOW0_ACT_GEAR,           // ���� 00 05 00 02
    EM_SCREEN_FLOW0_ACT_FACTORY,		// �ָ����� 00 05 00 03
    EM_SCREEN_FLOW0_ACT_GREET,		    // ���к� 00 05 00 04
    EM_SCREEN_FLOW0_ACT_CLAP,		    // ���� 00 05 00 05
    EM_SCREEN_FLOW0_ACT_FIREWORKS,      // �� 00 05 00 06
	
    EM_SCREEN_FLOW0_MAX = EM_SCREEN_FLOW0_ACT_FIREWORKS,
    EM_SCREEN_FLOW0_ALL,

    EM_SCREEN_FLOW0_NONE = 0xFF,     // ��Ч���̣�Ĭ��ֵ
}SCREEN_SHOW_MOTION_TYPE_E; 

/* EM_SCREEN_APP_FLOW1_SHOW_COVER -  չʾͼ��������ϸö�� */
typedef enum
{
    EM_SCREEN_FLOW1_1,
    EM_SCREEN_FLOW1_ALL,

    EM_SCREEN_FLOW1_NONE = 0xFF,     // ��Ч���̣�Ĭ��ֵ
}SCREEN_SHOW_COVER_TYPE_E; 

/* EM_SCREEN_APP_FLOW2_SWITCH_VER - �л���ͼ�汾������ϸö�� */
typedef enum
{
    EM_SCREEN_FLOW2_1,
    EM_SCREEN_FLOW2_ALL,

    EM_SCREEN_FLOW2_NONE = 0xFF,     // ��Ч���̣�Ĭ��ֵ
}SCREEN_SWITCH_VER_TYPE_E; 


/* EM_SCREEN_APP_FLOW3_GET_VER - ��ȡ��ͼ�汾������ϸö�� */
typedef enum
{
    EM_SCREEN_FLOW3_1,
    EM_SCREEN_FLOW3_ALL,

    EM_SCREEN_FLOW3_NONE = 0xFF,     // ��Ч���̣�Ĭ��ֵ
}SCREEN_GET_VER_TYPE_E; 


/* ����״ִ̬�н�� */
typedef enum
{
    SCREEN_APP_RESULT_IDLE = 0,         // ����
    SCREEN_APP_RESULT_RUNNING,          // ������    
    
    SCREEN_APP_RESULT_SUC,              // ִ�гɹ�
    SCREEN_APP_RESULT_FAIL,             // ִ��ʧ��
    SCREEN_APP_RESULT_ALL,              //�������ֵ
}SCREEN_APP_FLOW_RESULT_E; 


#ifdef SMART_SCREEN_ON 


/*--------------------------------------------------��������---------------------------------------------------------*/             

/* ģ��ҵ�������̼���νṹ�� */
typedef struct
{
    /* ��� */
    SCREEN_APP_FLOW_TYPE_E      emAppFlow;      // ��¼��ǰҵ��������תֵ
    uint32_t                    u32EmCfg;       // ��ζ���������emAppFlowȷ��ʹ�þ���ö��
    
    /* ���� */
    SCREEN_APP_FLOW_RESULT_E    emFlowResult;   // ���н��,��Ϊ�����Ч
    uint16_t                    u16OutPad;      // �������Σ�Ŀǰ����֧����ο���Ľ�ADD��DELָ�ƣ����ڴ���ģ���
    
}ScreenAppParam_S; 

#endif


/*--------------------------------------------------��������---------------------------------------------------------*/

void App_SCREEN_Tim10Ms(void);
void APP_SCREEN_WakeUp(void);
void APP_SCREEN_Sleep(void);
bool APP_SCREEN_Operate(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg);
uint8_t APP_SCREEN_GetResult(void);
bool APP_SCREEN_Update(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint8_t *_pu8Data, uint32_t _u32Len);
void APP_SCREEN_Scan(void);


#endif




