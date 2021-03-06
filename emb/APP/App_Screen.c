/*********************************************************************************************************************
 * @file:        APP_Screen.c
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-09-02
 * @Description: 智能屏交互
 * @ChangeList:  01. 初版
*/


/* 标准头文件 */

/* 内部头文件 */
#include "APP_Screen.h"
#include "..\Server\Screen.h"


/* 外部头文件 */
#include "Public.h"
#include "LockConfig.h"
#include "System.h" 
#include "../HAL/HAL_EXPORT/HAL_EXPORT.h"
#include "../HAL/HAL_Voice/HAL_Voice.h"
#include "../HAL/HAL_RTC/HAL_RTC.h"
#include "DRV_UART.h"
#include "../DRV/DRV_EXPORT/DRV_74HC4052.h"

#ifdef SMART_SCREEN_ON 

/*********************** 外部声明变量和函数************************/
/**************    为了减少耦合，后续逐步去除外部声音**********/


/*************************  0 内部使用宏定义*************************/

/*****************************  1 静态变量*****************************/
static ScreenAppParam_S s_stScreenAppParam = {EM_SCREEN_APP_FLOW_NONE, 0xFF, SCREEN_APP_RESULT_IDLE, 0};
static uint32_t s_u32ScreenAppCnt = 0;
static uint8_t s_u8ScreenAppFlowOrderFlag  = 0;       // 记录当前业务流程运转值对应的指令运转值，对应s_u8AppFlowMap


/***************************** 2  静态函数*****************************/

/****************************************************************************** 
* 函数名：app_screen_cnt
* 功 能：提供给外部定时器(每10毫秒调用一次)，每一次尝试获取图像，减少一次计数
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
static uint32_t app_screen_cnt(void)
{
    return (s_u32ScreenAppCnt > 0) ? (s_u32ScreenAppCnt--) : 0;
}

/****************************************************************************** 
* 函数名：app_screen_reset
* 功 能：重置业务流程尝试次数,放到10ms定时器 
* 输 入：void 
* 输 出：void
* 返 回：void
*/ 
static void app_screen_reset(uint32_t _u32Cnt)
{
    if(_u32Cnt > 3000){ _u32Cnt = 3000; }

    s_u32ScreenAppCnt = _u32Cnt;
    return;
}

/****************************************************************************** 
* 函数名：app_screen_get
* 功 能：获取当前业务流程尝试次数的计数值
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
static uint32_t app_screen_get(void)
{
    return s_u32ScreenAppCnt;
}

/****************************************************************************** 
* 函数名：app_screen_reset_flag
* 功 能：重置相关flag标记变量
* 输 入：SCREEN_APP_FLOW_RESULT_E
* 输 出：void
* 返 回：void
*/
static void app_screen_reset_flag(SCREEN_APP_FLOW_RESULT_E _em, bool _bForce)
{
    /*  1. 状态运转过程中，异常退出 
            2. 状态运转过程中，但运转结果已复位
            3. 强制复位(睡眠) */
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
* 函数名：app_screen_flow_factory_reset
* 功 能：恢复出厂
* 输 入：uint8_t _u8Ex
* 输 出：void
* 返 回：void
*/
static void app_screen_flow_factory_reset()
{
    (void)SCREEN_FactoryReset((uint8_t)s_stScreenAppParam.u32EmCfg);
        
    return;
}

/****************************************************************************** 
* 函数名：app_screen_flow_show_motion
* 功 能：动图展示填入入参
* 输 入：bool isScreenShow  - true:轮播展示  false:正常展示
                    bool isSwitchBase - true 切换为基础表情  false 不切换基础表情
* 输 出：void
* 返 回：void
*/
static void app_screen_flow_show_motion(bool isScreenShow, bool isSwitchBase)
{
    SCREEN_SHOW_MOTION_TYPE_E em = (SCREEN_SHOW_MOTION_TYPE_E)s_stScreenAppParam.u32EmCfg;

    uint8_t u8WrapCnt = 0;
    uint32_t u32Ex = 0;

    switch(em)
    {
        /* 0x0100 表情 */
        //case EM_SCREEN_FLOW0_SMILE_FACE: u32Ex = 0x01000000; break;
        //case EM_SCREEN_FLOW0_SORRY_FACE: u32Ex = 0x02000000; break;
        //case EM_SCREEN_FLOW0_CRY_FACE: u32Ex = 0x03000000; break;
        //case EM_SCREEN_FLOW0_BLINK_FACE: u32Ex = 0x04000000; break;

        /* 0x0001 锁提示 */
        case EM_SCREEN_FLOW0_SAFETY_TIP: u32Ex = 0x00010001; break;
        case EM_SCREEN_FLOW0_NOLOCK: u32Ex = 0x00010002; break;
        case EM_SCREEN_FLOW0_LOW_POWER: u32Ex = 0x00010003; break;
            
        /* 0x0002 解锁动作 */
#if 1   //产品变更 需要随机表情
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

        /* 0x0003 天气 */
        case EM_SCREEN_FLOW0_SUNNY_DAY: u32Ex = 0x00030001; break;
        case EM_SCREEN_FLOW0_CLOUDY_DAY: u32Ex = 0x00030002; break;
        case EM_SCREEN_FLOW0_OVERCAST_DAY: u32Ex = 0x00030003; break;
        case EM_SCREEN_FLOW0_RAINY_DAY: u32Ex = 0x00030004; break;
        case EM_SCREEN_FLOW0_SNOWY_DAY: u32Ex = 0x00030005; break;
        //case EM_SCREEN_FLOW0_BLUE_ALERT: u32Ex = 0x00030101; break;
        //case EM_SCREEN_FLOW0_YELLOW_ALERT: u32Ex = 0x00030102; break;
        //case EM_SCREEN_FLOW0_ORANGE_ALERT: u32Ex = 0x00030103; break;
        //case EM_SCREEN_FLOW0_RED_ALERT: u32Ex = 0x00030104; break;

        /* 0x0004 节日 */
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
        
        /* 0x0005 表情动作 */
        case EM_SCREEN_FLOW0_ACT_BASE: u32Ex = 0x00050001; break;
        case EM_SCREEN_FLOW0_ACT_GEAR: u32Ex = 0x00050002; break;
        case EM_SCREEN_FLOW0_ACT_FACTORY: u32Ex = 0x00050003; break;
        case EM_SCREEN_FLOW0_ACT_GREET: u32Ex = 0x00050004; break;
        case EM_SCREEN_FLOW0_ACT_CLAP: u32Ex = 0x00050005; break;
        case EM_SCREEN_FLOW0_ACT_FIREWORKS: u32Ex = 0x00050006; break;

        /* 不存在的表情不下发 */
        default: return;
    }
    
    if(!isSwitchBase){(void)SCREEN_ShowMotionPic(u8WrapCnt, u32Ex); }
    else {(void)SCREEN_SwitchBase(u8WrapCnt, u32Ex); }
        
    return;
}

/****************************************************************************** 
* 函数名：app_screen_flow_show_cover
* 功 能：图层展示填入入参
* 输 入：void
* 输 出：void
* 返 回：void
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
* 函数名：app_screen_flow_switch_ver
* 功 能：版本转换填入入参
* 输 入：void
* 输 出：void
* 返 回：void
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
* 函数名：app_screen_flow_get_ver
* 功 能：版本获取入参
* 输 入：void
* 输 出：void
* 返 回：void
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
* 函数名：app_screen_flow_operate
* 功 能：动作参数检查、匹配选择，并匹配填入入参
* 输 入：void
* 输 出：void
* 返 回：void
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


/***************************** 3 对外函数*****************************/

/****************************************************************************** 
* 函数名：App_SCREEN_Tim10Ms
* 功 能：10MS 定时器调用函数
* 输 入：void
* 输 出：void
* 返 回：void
*/
void App_SCREEN_Tim10Ms(void)
{
#ifdef SMART_SCREEN_ON 
    (void)app_screen_cnt();
#endif
    return;
}

/****************************************************************************** 
* 函数名：APP_SCREEN_WakeUp
* 功 能：唤醒
* 输 入：void
* 输 出：void
* 返 回：void
*/
void APP_SCREEN_WakeUp(void)
{
#ifdef SMART_SCREEN_ON 
    /* 重置复位指纹业务功能内部参数 */
    s_stScreenAppParam.u32EmCfg = 0xFF;
    s_stScreenAppParam.emAppFlow = EM_SCREEN_APP_FLOW_NONE;
    s_stScreenAppParam.emFlowResult = SCREEN_APP_RESULT_IDLE;
    s_u8ScreenAppFlowOrderFlag = 0;
    
    /* 智能屏协议服务初始化 */
    (void)SCREEN_PowerUp();
#endif
    return;
}

/****************************************************************************** 
* 函数名：APP_SCREEN_Sleep
* 功 能：休眠
* 输 入：void
* 输 出：void
* 返 回：void
*/
void APP_SCREEN_Sleep(void)
{
#ifdef SMART_SCREEN_ON 
    (void)app_screen_reset_flag(SCREEN_APP_RESULT_IDLE, true);
#endif
    return;
}

/****************************************************************************** 
* 函数名：APP_SCREEN_Operate
* 功 能：智能屏操作合集
* 输 入：SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg
* 输 出：void
* 返 回：bool
*/
bool APP_SCREEN_Operate(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg)
{    
#ifdef SMART_SCREEN_ON 
    /* 业务流程判定 */
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

    /* 每次操作需要唤醒模块 */
    (void)SCREEN_OperateReady();

    /* 记录当前业务流程状态值*/
    s_u8ScreenAppFlowOrderFlag = 0;
    s_stScreenAppParam.u16OutPad = 0;
    s_stScreenAppParam.u32EmCfg = _u32EmCfg;
    s_stScreenAppParam.emAppFlow = _emAppFlow;
    s_stScreenAppParam.emFlowResult = SCREEN_APP_RESULT_RUNNING;

    /* 复位计数值 */
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
* 函数名：APP_SCREEN_Operate
* 功 能：指纹操作合集
* 输 入：SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg
* 输 出：void
* 返 回：bool
*/
bool APP_SCREEN_Update(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint8_t *_pu8Data, uint32_t _u32Len)
{    
#ifdef SMART_SCREEN_ON 
    /* 业务流程判定 */
    if((EM_SCREEN_APP_FLOW5_UPDATE != _emAppFlow
        || EM_SCREEN_APP_FLOW6_UPDATE_DATA != _emAppFlow)
        || NULL == _pu8Data)
    {
        return false;
    }

    /* 每次操作需要唤醒模块 */
    (void)SCREEN_OperateReady();

    /* 记录当前业务流程状态值*/
    s_u8ScreenAppFlowOrderFlag = 0;
    s_stScreenAppParam.u16OutPad = 0;
    //s_stScreenAppParam.u32EmCfg = _u32EmCfg;
    s_stScreenAppParam.emAppFlow = _emAppFlow;
    s_stScreenAppParam.emFlowResult = SCREEN_APP_RESULT_RUNNING;

    /* 复位计数值 */
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
* 函数名：APP_SCREEN_Scan
* 功 能：状态轮训，通过预制MAP，改为自动调用FINGER接口，执行业务流程
* 输 入：void
* 输 出：void
* 返 回：void
*/
void APP_SCREEN_Scan(void)
{
#ifdef SMART_SCREEN_ON 
    /* 智能屏模组协议执行轮巡 */
    (void)SCREEN_Scan();

    /* 动作参数检查、匹配选择，并匹配填入入参,向SCREEN协议模组发起屏显动作 */
    (void)app_screen_flow_operate();
#endif
    return;
}



