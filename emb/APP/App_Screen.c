/*********************************************************************************************************************
 * @file:        APP_Screen.c
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-09-02
 * @Description: ����������
 * @ChangeList:  01. ����
*/


/* ��׼ͷ�ļ� */

/* �ڲ�ͷ�ļ� */
#include "APP_Screen.h"
#include "..\Server\Screen.h"


/* �ⲿͷ�ļ� */
#include "Public.h"
#include "LockConfig.h"
#include "System.h" 
#include "../HAL/HAL_EXPORT/HAL_EXPORT.h"
#include "../HAL/HAL_Voice/HAL_Voice.h"
#include "../HAL/HAL_RTC/HAL_RTC.h"
#include "DRV_UART.h"
#include "../DRV/DRV_EXPORT/DRV_74HC4052.h"

#ifdef SMART_SCREEN_ON 

/*********************** �ⲿ���������ͺ���************************/
/**************    Ϊ�˼�����ϣ�������ȥ���ⲿ����**********/


/*************************  0 �ڲ�ʹ�ú궨��*************************/

/*****************************  1 ��̬����*****************************/
static ScreenAppParam_S s_stScreenAppParam = {EM_SCREEN_APP_FLOW_NONE, 0xFF, SCREEN_APP_RESULT_IDLE, 0};
static uint32_t s_u32ScreenAppCnt = 0;
static uint8_t s_u8ScreenAppFlowOrderFlag  = 0;       // ��¼��ǰҵ��������תֵ��Ӧ��ָ����תֵ����Ӧs_u8AppFlowMap


/***************************** 2  ��̬����*****************************/

/****************************************************************************** 
* ��������app_screen_cnt
* �� �ܣ��ṩ���ⲿ��ʱ��(ÿ10�������һ��)��ÿһ�γ��Ի�ȡͼ�񣬼���һ�μ���
* �� �룺void 
* �� ����void
* �� �أ�uint8_t ���ؼ���ֵ
*/ 
static uint32_t app_screen_cnt(void)
{
    return (s_u32ScreenAppCnt > 0) ? (s_u32ScreenAppCnt--) : 0;
}

/****************************************************************************** 
* ��������app_screen_reset
* �� �ܣ�����ҵ�����̳��Դ���,�ŵ�10ms��ʱ�� 
* �� �룺void 
* �� ����void
* �� �أ�void
*/ 
static void app_screen_reset(uint32_t _u32Cnt)
{
    if(_u32Cnt > 3000){ _u32Cnt = 3000; }

    s_u32ScreenAppCnt = _u32Cnt;
    return;
}

/****************************************************************************** 
* ��������app_screen_get
* �� �ܣ���ȡ��ǰҵ�����̳��Դ����ļ���ֵ
* �� �룺void 
* �� ����void
* �� �أ�uint8_t ���ؼ���ֵ
*/ 
static uint32_t app_screen_get(void)
{
    return s_u32ScreenAppCnt;
}

/****************************************************************************** 
* ��������app_screen_reset_flag
* �� �ܣ��������flag��Ǳ���
* �� �룺SCREEN_APP_FLOW_RESULT_E
* �� ����void
* �� �أ�void
*/
static void app_screen_reset_flag(SCREEN_APP_FLOW_RESULT_E _em, bool _bForce)
{
    /*  1. ״̬��ת�����У��쳣�˳� 
            2. ״̬��ת�����У�����ת����Ѹ�λ
            3. ǿ�Ƹ�λ(˯��) */
    if((0 != s_u8ScreenAppFlowOrderFlag && SCREEN_APP_RESULT_RUNNING == s_stScreenAppParam.emFlowResult)
        || (0 != s_u8ScreenAppFlowOrderFlag && SCREEN_APP_RESULT_IDLE == s_stScreenAppParam.emFlowResult)
        || true == _bForce)
    {
        (void)SCREEN_PowerDown();
        s_stScreenAppParam.u32EmCfg = 0xFF;
        s_stScreenAppParam.emAppFlow = EM_SCREEN_APP_FLOW_NONE;
        s_stScreenAppParam.emFlowResult = _bForce?SCREEN_APP_RESULT_IDLE:_em;
        s_u8ScreenAppFlowOrderFlag = 0;
    }
    return;
}

/****************************************************************************** 
* ��������app_screen_flow_factory_reset
* �� �ܣ��ָ�����
* �� �룺uint8_t _u8Ex
* �� ����void
* �� �أ�void
*/
static void app_screen_flow_factory_reset()
{
    (void)SCREEN_FactoryReset((uint8_t)s_stScreenAppParam.u32EmCfg);
        
    return;
}

/****************************************************************************** 
* ��������app_screen_flow_show_motion
* �� �ܣ���ͼչʾ�������
* �� �룺bool isScreenShow  - true:�ֲ�չʾ  false:����չʾ
                    bool isSwitchBase - true �л�Ϊ��������  false ���л���������
* �� ����void
* �� �أ�void
*/
static void app_screen_flow_show_motion(bool isScreenShow, bool isSwitchBase)
{
    SCREEN_SHOW_MOTION_TYPE_E em = (SCREEN_SHOW_MOTION_TYPE_E)s_stScreenAppParam.u32EmCfg;

    uint8_t u8WrapCnt = 0;
    uint32_t u32Ex = 0;

    switch(em)
    {
        /* 0x0100 ���� */
        //case EM_SCREEN_FLOW0_SMILE_FACE: u32Ex = 0x01000000; break;
        //case EM_SCREEN_FLOW0_SORRY_FACE: u32Ex = 0x02000000; break;
        //case EM_SCREEN_FLOW0_CRY_FACE: u32Ex = 0x03000000; break;
        //case EM_SCREEN_FLOW0_BLINK_FACE: u32Ex = 0x04000000; break;

        /* 0x0001 ����ʾ */
        case EM_SCREEN_FLOW0_SAFETY_TIP: u32Ex = 0x00010001; break;
        case EM_SCREEN_FLOW0_NOLOCK: u32Ex = 0x00010002; break;
        case EM_SCREEN_FLOW0_LOW_POWER: u32Ex = 0x00010003; break;
            
        /* 0x0002 �������� */
#if 1   //��Ʒ��� ��Ҫ�������
        case EM_SCREEN_FLOW0_FACE_SUC:
        case EM_SCREEN_FLOW0_FINGER_SUC:
            if(isScreenShow)
            {
                switch(em)
                {
                    case EM_SCREEN_FLOW0_FACE_SUC: u32Ex = 0x00020001; break;
                    case EM_SCREEN_FLOW0_FINGER_SUC: u32Ex = 0x00020002; break;
                    default: u32Ex = 0x00020001; break;
                }
            }
            else
            {
                switch(Rtc_Real_Time.second%2)
                {
                    case 0: u32Ex = 0x00020001; break;
                    case 1: u32Ex = 0x00020002; break;
                    default: u32Ex = 0x00020001; break;
                }
            }
            break;
        case EM_SCREEN_FLOW0_FACE_FAIL:
        case EM_SCREEN_FLOW0_FINGER_FAIL:
        case EM_SCREEN_FLOW0_PWD_FAIL:
            if(isScreenShow)
            {
                switch(em)
                {
                    case EM_SCREEN_FLOW0_FACE_FAIL: u32Ex = 0x00020101; break;
                    case EM_SCREEN_FLOW0_FINGER_FAIL: u32Ex = 0x00020201; break;
                    case EM_SCREEN_FLOW0_PWD_FAIL: u32Ex = 0x00020501; break;
                    default: u32Ex = 0x00020101; break;
                }
            }
            else
            {
                switch(Rtc_Real_Time.second%3)
                {
                    case 0: u32Ex = 0x00020101; break;
                    case 1: u32Ex = 0x00020201; break;
                    case 2: u32Ex = 0x00020501; break;
                    default: u32Ex = 0x00020101; break;
                }
            }
            break;
#else
        case EM_SCREEN_FLOW0_FACE_SUC: u32Ex = 0x00020001; break;
        case EM_SCREEN_FLOW0_FINGER_SUC: u32Ex = 0x00020002; break;
        //case EM_SCREEN_FLOW0_VEIN_SUC: u32Ex = 0x00020003; break;
        //case EM_SCREEN_FLOW0_IRIS_SUC: u32Ex = 0x00020004; break;
        case EM_SCREEN_FLOW0_PWD_SUC: u32Ex = 0x00020005; break;
        //case EM_SCREEN_FLOW0_CARD_SUC: u32Ex = 0x00020006; break;
        //case EM_SCREEN_FLOW0_BLE_SUC: u32Ex = 0x00020007; break;
        case EM_SCREEN_FLOW0_FACE_FAIL: u32Ex = 0x00020101; break;
        //case EM_SCREEN_FLOW0_FACE_TOOCLOSE: u32Ex = 0x00020102; break;
        //case EM_SCREEN_FLOW0_FACE_TOOFARAWAY: u32Ex = 0x00020103; break;
        //case EM_SCREEN_FLOW0_FACE_KEEPOUT: u32Ex = 0x00020104; break;
        //case EM_SCREEN_FLOW0_FACE_MISS: u32Ex = 0x00020105; break;
        case EM_SCREEN_FLOW0_FINGER_FAIL: u32Ex = 0x00020201; break;
        //case EM_SCREEN_FLOW0_FINGER_MISS: u32Ex = 0x00020202; break;
        //case EM_SCREEN_FLOW0_VEIN_FAIL: u32Ex = 0x00020301; break;
        //case EM_SCREEN_FLOW0_VEIN_MISS: u32Ex = 0x00020302; break;
        //case EM_SCREEN_FLOW0_IRIS_FAIL: u32Ex = 0x00020401; break;
        //case EM_SCREEN_FLOW0_IRIS_MISS: u32Ex = 0x00020402; break;
        case EM_SCREEN_FLOW0_PWD_FAIL: u32Ex = 0x00020501; break;
        //case EM_SCREEN_FLOW0_CARD_FAIL: u32Ex = 0x00020601; break;
#endif

        /* 0x0003 ���� */
        case EM_SCREEN_FLOW0_SUNNY_DAY: u32Ex = 0x00030001; break;
        case EM_SCREEN_FLOW0_CLOUDY_DAY: u32Ex = 0x00030002; break;
        case EM_SCREEN_FLOW0_OVERCAST_DAY: u32Ex = 0x00030003; break;
        case EM_SCREEN_FLOW0_RAINY_DAY: u32Ex = 0x00030004; break;
        case EM_SCREEN_FLOW0_SNOWY_DAY: u32Ex = 0x00030005; break;
        //case EM_SCREEN_FLOW0_BLUE_ALERT: u32Ex = 0x00030101; break;
        //case EM_SCREEN_FLOW0_YELLOW_ALERT: u32Ex = 0x00030102; break;
        //case EM_SCREEN_FLOW0_ORANGE_ALERT: u32Ex = 0x00030103; break;
        //case EM_SCREEN_FLOW0_RED_ALERT: u32Ex = 0x00030104; break;

        /* 0x0004 ���� */
        case EM_SCREEN_FLOW0_HOLIDAY_0101: u32Ex = 0x00040001; break;
        case EM_SCREEN_FLOW0_HOLIDAY_0214: u32Ex = 0x00040002; break;
        case EM_SCREEN_FLOW0_HOLIDAY_0501: u32Ex = 0x00040003; break;
        case EM_SCREEN_FLOW0_HOLIDAY_0601: u32Ex = 0x00040004; break;
        case EM_SCREEN_FLOW0_HOLIDAY_1001: u32Ex = 0x00040005; break;
        case EM_SCREEN_FLOW0_HOLIDAY_1225: u32Ex = 0x00040006; break;
        case EM_SCREEN_FLOW0_LUNAR_1230: u32Ex = 0x00040101; break;
        case EM_SCREEN_FLOW0_LUNAR_0115: u32Ex = 0x00040102; break;
        case EM_SCREEN_FLOW0_LUNAR_0505: u32Ex = 0x00040103; break;
        case EM_SCREEN_FLOW0_LUNAR_0815: u32Ex = 0x00040104; break;
        case EM_SCREEN_FLOW0_LUNAR_0707: u32Ex = 0x00040105; break;
        
        /* 0x0005 ���鶯�� */
        case EM_SCREEN_FLOW0_ACT_BASE: u32Ex = 0x00050001; break;
        case EM_SCREEN_FLOW0_ACT_GEAR: u32Ex = 0x00050002; break;
        case EM_SCREEN_FLOW0_ACT_FACTORY: u32Ex = 0x00050003; break;
        case EM_SCREEN_FLOW0_ACT_GREET: u32Ex = 0x00050004; break;
        case EM_SCREEN_FLOW0_ACT_CLAP: u32Ex = 0x00050005; break;
        case EM_SCREEN_FLOW0_ACT_FIREWORKS: u32Ex = 0x00050006; break;

        /* �����ڵı��鲻�·� */
        default: return;
    }
    
    if(!isSwitchBase){(void)SCREEN_ShowMotionPic(u8WrapCnt, u32Ex); }
    else {(void)SCREEN_SwitchBase(u8WrapCnt, u32Ex); }
        
    return;
}

/****************************************************************************** 
* ��������app_screen_flow_show_cover
* �� �ܣ�ͼ��չʾ�������
* �� �룺void
* �� ����void
* �� �أ�void
*/
static void app_screen_flow_show_cover(void)
{
    SCREEN_SHOW_COVER_TYPE_E em = (SCREEN_SHOW_COVER_TYPE_E)s_stScreenAppParam.u32EmCfg;
    
    uint8_t u8WrapCnt = 0;
    uint32_t u8Scene = 0;
    uint32_t u32Ex = 0;

    switch(em)
    {
        default:
            break;
    }
    (void)SCREEN_ShowCoverPic(u8WrapCnt, u8Scene, u32Ex);
        
    return;
}

/****************************************************************************** 
* ��������app_screen_flow_switch_ver
* �� �ܣ��汾ת���������
* �� �룺void
* �� ����void
* �� �أ�void
*/
static void app_screen_flow_switch_ver(void)
{
    SCREEN_SWITCH_VER_TYPE_E em = (SCREEN_SWITCH_VER_TYPE_E)s_stScreenAppParam.u32EmCfg;

    uint32_t u32Ex = 0;

    switch(em)
    {
        case EM_SCREEN_FLOW2_1:
            break;
        default:
            break;
    }
    (void)SCREEN_SwitchVersion(u32Ex);
        
    return;
}

/****************************************************************************** 
* ��������app_screen_flow_get_ver
* �� �ܣ��汾��ȡ���
* �� �룺void
* �� ����void
* �� �أ�void
*/
static void app_screen_flow_get_ver(void)
{
    SCREEN_GET_VER_TYPE_E em = (SCREEN_GET_VER_TYPE_E)s_stScreenAppParam.u32EmCfg;

    switch(em)
    {
        case EM_SCREEN_FLOW3_1:
            break;
        default:
            break;
    }
    (void)SCREEN_GetVersion();
        
    return;
}


/****************************************************************************** 
* ��������app_screen_flow_operate
* �� �ܣ�����������顢ƥ��ѡ�񣬲�ƥ���������
* �� �룺void
* �� ����void
* �� �أ�void
*/
static void app_screen_flow_operate(void)
{
    if(SCREEN_APP_RESULT_RUNNING == s_stScreenAppParam.emFlowResult && s_stScreenAppParam.emAppFlow < EM_SCREEN_APP_FLOW_ALL)
    {
        switch(s_stScreenAppParam.emAppFlow)
        {
            case EM_SCREEN_APP_FLOW0_SHOW_MOTION:
                (void)app_screen_flow_show_motion(false, false);
                break;
            case EM_SCREEN_APP_FLOW1_SHOW_COVER:
                (void)app_screen_flow_show_cover();
                break;
            case EM_SCREEN_APP_FLOW2_SWITCH_VER:
                (void)app_screen_flow_switch_ver();
                break;
            case EM_SCREEN_APP_FLOW3_GET_VER:
                (void)app_screen_flow_get_ver();
                break;
            case EM_SCREEN_APP_FLOW4_SWITCH_BASE:
                (void)app_screen_flow_show_motion(false, true);
                break;
            case EM_SCREEN_APP_FLOW7_FACTORY_RESET:
                (void)app_screen_flow_factory_reset();
                break;
            case EM_SCREEN_APP_FLOW8_SCREEN_SHOW:
                (void)app_screen_flow_show_motion(true, false);
                break;
            default:
                break;
        }

        s_stScreenAppParam.emAppFlow = EM_SCREEN_APP_FLOW_NONE;
    }
    
    return;
}

#endif


/***************************** 3 ���⺯��*****************************/

/****************************************************************************** 
* ��������App_SCREEN_Tim10Ms
* �� �ܣ�10MS ��ʱ�����ú���
* �� �룺void
* �� ����void
* �� �أ�void
*/
void App_SCREEN_Tim10Ms(void)
{
#ifdef SMART_SCREEN_ON 
    (void)app_screen_cnt();
#endif
    return;
}

/****************************************************************************** 
* ��������APP_SCREEN_WakeUp
* �� �ܣ�����
* �� �룺void
* �� ����void
* �� �أ�void
*/
void APP_SCREEN_WakeUp(void)
{
#ifdef SMART_SCREEN_ON 
    /* ���ø�λָ��ҵ�����ڲ����� */
    s_stScreenAppParam.u32EmCfg = 0xFF;
    s_stScreenAppParam.emAppFlow = EM_SCREEN_APP_FLOW_NONE;
    s_stScreenAppParam.emFlowResult = SCREEN_APP_RESULT_IDLE;
    s_u8ScreenAppFlowOrderFlag = 0;
    
    /* ������Э������ʼ�� */
    (void)SCREEN_PowerUp();
#endif
    return;
}

/****************************************************************************** 
* ��������APP_SCREEN_Sleep
* �� �ܣ�����
* �� �룺void
* �� ����void
* �� �أ�void
*/
void APP_SCREEN_Sleep(void)
{
#ifdef SMART_SCREEN_ON 
    (void)app_screen_reset_flag(SCREEN_APP_RESULT_IDLE, true);
#endif
    return;
}

/****************************************************************************** 
* ��������APP_SCREEN_Operate
* �� �ܣ������������ϼ�
* �� �룺SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg
* �� ����void
* �� �أ�bool
*/
bool APP_SCREEN_Operate(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg)
{    
#ifdef SMART_SCREEN_ON 
    /* ҵ�������ж� */
    if((EM_SCREEN_APP_FLOW_NONE == _emAppFlow)) {return false;}

    switch(_emAppFlow)
    {
        case EM_SCREEN_APP_FLOW0_SHOW_MOTION:
        case EM_SCREEN_APP_FLOW4_SWITCH_BASE:
        case EM_SCREEN_APP_FLOW8_SCREEN_SHOW:
            if(_u32EmCfg >= EM_SCREEN_FLOW0_ALL) { return false; }
            break;
        case EM_SCREEN_APP_FLOW1_SHOW_COVER:
            if(_u32EmCfg >= EM_SCREEN_FLOW1_ALL) { return false; }
            break;
        case EM_SCREEN_APP_FLOW2_SWITCH_VER:
            if(_u32EmCfg >= EM_SCREEN_FLOW2_ALL) { return false; }
            break;
        case EM_SCREEN_APP_FLOW3_GET_VER:
            if(_u32EmCfg >= EM_SCREEN_FLOW3_ALL) { return false; }
            break;
        case EM_SCREEN_APP_FLOW7_FACTORY_RESET:
            if(_u32EmCfg >= 0xFF) { return false; }
            break;
        default:
            return false;;
    }

    /* ÿ�β�����Ҫ����ģ�� */
    (void)SCREEN_OperateReady();

    /* ��¼��ǰҵ������״ֵ̬*/
    s_u8ScreenAppFlowOrderFlag = 0;
    s_stScreenAppParam.u16OutPad = 0;
    s_stScreenAppParam.u32EmCfg = _u32EmCfg;
    s_stScreenAppParam.emAppFlow = _emAppFlow;
    s_stScreenAppParam.emFlowResult = SCREEN_APP_RESULT_RUNNING;

    /* ��λ����ֵ */
    if(_emAppFlow == EM_SCREEN_APP_FLOW7_FACTORY_RESET)
        (void)app_screen_reset(2500);
    else
        (void)app_screen_reset(50);

    (void)app_screen_flow_operate();

    while(app_screen_get() > 0)
    {
        (void)SCREEN_Scan();
        if(SCREEN_ACK_NONE != SCREEN_GetAckCode()) {break;}
    }
#endif
    return true;
}

/****************************************************************************** 
* ��������APP_SCREEN_Operate
* �� �ܣ�ָ�Ʋ����ϼ�
* �� �룺SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg
* �� ����void
* �� �أ�bool
*/
bool APP_SCREEN_Update(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint8_t *_pu8Data, uint32_t _u32Len)
{    
#ifdef SMART_SCREEN_ON 
    /* ҵ�������ж� */
    if((EM_SCREEN_APP_FLOW5_UPDATE != _emAppFlow
        || EM_SCREEN_APP_FLOW6_UPDATE_DATA != _emAppFlow)
        || NULL == _pu8Data)
    {
        return false;
    }

    /* ÿ�β�����Ҫ����ģ�� */
    (void)SCREEN_OperateReady();

    /* ��¼��ǰҵ������״ֵ̬*/
    s_u8ScreenAppFlowOrderFlag = 0;
    s_stScreenAppParam.u16OutPad = 0;
    //s_stScreenAppParam.u32EmCfg = _u32EmCfg;
    s_stScreenAppParam.emAppFlow = _emAppFlow;
    s_stScreenAppParam.emFlowResult = SCREEN_APP_RESULT_RUNNING;

    /* ��λ����ֵ */
    (void)app_screen_reset(50);

    switch(_emAppFlow)
    {
        case EM_SCREEN_APP_FLOW5_UPDATE:
            SCREEN_ApplyForUpdate(_pu8Data);
            break;
        case EM_SCREEN_APP_FLOW6_UPDATE_DATA:
            SCREEN_StartToUpdate(_pu8Data, _u32Len);
            break;
        default:
            break;
    }

    while(app_screen_get() > 0)
    {
        (void)SCREEN_Scan();
        if(SCREEN_ACK_NONE != SCREEN_GetAckCode()) {break;}
    }
#endif
    return true;
}

uint8_t APP_SCREEN_GetResult(void)
{
    uint8_t ret = 0;
#ifdef SMART_SCREEN_ON 
    ret = SCREEN_GetAckCode();
#endif
    return ret;
}

/****************************************************************************** 
* ��������APP_SCREEN_Scan
* �� �ܣ�״̬��ѵ��ͨ��Ԥ��MAP����Ϊ�Զ�����FINGER�ӿڣ�ִ��ҵ������
* �� �룺void
* �� ����void
* �� �أ�void
*/
void APP_SCREEN_Scan(void)
{
#ifdef SMART_SCREEN_ON 
    /* ������ģ��Э��ִ����Ѳ */
    (void)SCREEN_Scan();

    /* ����������顢ƥ��ѡ�񣬲�ƥ���������,��SCREENЭ��ģ�鷢�����Զ��� */
    (void)app_screen_flow_operate();
#endif
    return;
}


