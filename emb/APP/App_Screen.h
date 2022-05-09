/*********************************************************************************************************************
 * @file:        APP_Screen.h
 * @author:      fanshuyu
 * @version:     V01.00
 * @date:        2021-09-02
 * @Description: 智能屏交互
 * @ChangeList:  01. 初版
**********************************************************************************************************************/
  
#ifndef  _APP_SCREEN_H_
#define  _APP_SCREEN_H_

/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "LockConfig.h"

/*--------------------------------------------------宏定义-----------------------------------------------------------*/

/* 业务流程- 调用智能屏模组协议的最大上限 */
#define SCREEN_APP_FLOW_MAX 6

/*--------------------------------------------------枚举声明---------------------------------------------------------*/

/* 模组业务功能流程 */
typedef enum
{
    EM_SCREEN_APP_FLOW0_SHOW_MOTION,    // 展示动图
    EM_SCREEN_APP_FLOW1_SHOW_COVER,     // 展示图层
    EM_SCREEN_APP_FLOW2_SWITCH_VER,     // 切换动图版本
    EM_SCREEN_APP_FLOW3_GET_VER,        // 获取动图版本
    EM_SCREEN_APP_FLOW4_SWITCH_BASE,    // 切换基础屏显动画
    EM_SCREEN_APP_FLOW5_UPDATE,         // 发起升级动作
    EM_SCREEN_APP_FLOW6_UPDATE_DATA,    // 升级包数据推送
    EM_SCREEN_APP_FLOW7_FACTORY_RESET,  // 升级包数据推送
    EM_SCREEN_APP_FLOW8_SCREEN_SHOW,    // 表情展示
    EM_SCREEN_APP_FLOW_ALL,             // 主流程上限，即最大值

    EM_SCREEN_APP_FLOW_NONE = 0xFF,     // 无效流程，默认值
}SCREEN_APP_FLOW_TYPE_E;

/* EM_SCREEN_APP_FLOW0_SHOW_MOTION - 展示动图流程详细枚举 */
typedef enum
{
    /* 0x0100 表情 */
    //EM_SCREEN_FLOW0_SMILE_FACE,         // 笑脸 01 00 00 00
    //EM_SCREEN_FLOW0_SORRY_FACE,         // 囧脸 02 00 00 00
    //EM_SCREEN_FLOW0_CRY_FACE,           // 哭脸 03 00 00 00
    //EM_SCREEN_FLOW0_BLINK_FACE,         // 眨眼 04 00 00 00

    /* 0x0001 锁提示 */
    EM_SCREEN_FLOW0_SAFETY_TIP,         // 安全提示00 01 00 01
    EM_SCREEN_FLOW0_NOLOCK,             // 门未上锁00 01 00 02
    EM_SCREEN_FLOW0_LOW_POWER,          // 电池低压00 01 00 03
        
    /* 0x0002 解锁动作 */
    EM_SCREEN_FLOW0_FACE_SUC,           // 人脸解锁成功 00 02 00 01
    EM_SCREEN_FLOW0_FINGER_SUC,         // 指纹解锁成功 00 02 00 02
    //EM_SCREEN_FLOW0_VEIN_SUC,           // 手指解锁成功 00 02 00 03
    //EM_SCREEN_FLOW0_IRIS_SUC,           // 虹膜解锁成功 00 02 00 04
    EM_SCREEN_FLOW0_PWD_SUC,            // 密码解锁成功 00 02 00 05
    //EM_SCREEN_FLOW0_CARD_SUC,           // 门卡解锁成功 00 02 00 06
    //EM_SCREEN_FLOW0_BLE_SUC,            // 蓝牙解锁成功 00 02 00 07
    EM_SCREEN_FLOW0_FACE_FAIL,          // 人脸解锁失败 00 02 01 01
    //EM_SCREEN_FLOW0_FACE_TOOCLOSE,      // 人脸距离近       00 02 01 02
    //EM_SCREEN_FLOW0_FACE_TOOFARAWAY,    // 人脸距离远       00 02 01 03
    //EM_SCREEN_FLOW0_FACE_KEEPOUT,       // 人脸有遮挡       00 02 01 04
    //EM_SCREEN_FLOW0_FACE_MISS,          // 人脸未检测到 00 02 01 05
    EM_SCREEN_FLOW0_FINGER_FAIL,        // 指纹解锁失败 00 02 02 01
    //EM_SCREEN_FLOW0_FINGER_MISS,        // 指纹未检测到 00 02 02 02
    //EM_SCREEN_FLOW0_VEIN_FAIL,          // 手指解锁失败 00 02 03 01
    //EM_SCREEN_FLOW0_VEIN_MISS,          // 手指未检测到 00 02 03 02
    //EM_SCREEN_FLOW0_IRIS_FAIL,          // 虹膜解锁失败 00 02 04 01
    //EM_SCREEN_FLOW0_IRIS_MISS,          // 虹膜未检测到 00 02 04 02
    EM_SCREEN_FLOW0_PWD_FAIL,           // 密码解锁成功 00 02 05 01
    //EM_SCREEN_FLOW0_CARD_FAIL,          // 门卡解锁成功 00 02 06 01

    /* 0x0003 天气 */
    EM_SCREEN_FLOW0_SUNNY_DAY,          // 晴天 00 03 00 01
    EM_SCREEN_FLOW0_CLOUDY_DAY,         // 多云 00 03 00 02
    EM_SCREEN_FLOW0_OVERCAST_DAY,       // 阴天 00 03 00 03
    EM_SCREEN_FLOW0_RAINY_DAY,          // 雨天 00 03 00 04
    EM_SCREEN_FLOW0_SNOWY_DAY,          // 雪天 00 03 00 05
    //EM_SCREEN_FLOW0_BLUE_ALERT,         // 天气蓝色预警 00 03 01 01
    //EM_SCREEN_FLOW0_YELLOW_ALERT,       // 天气黄色预警 00 03 01 02
    //EM_SCREEN_FLOW0_ORANGE_ALERT,       // 天气橙色预警 00 03 01 03
    //EM_SCREEN_FLOW0_RED_ALERT,          // 天气红色预警 00 03 01 04
        
    /* 0x0004 节日 */
    EM_SCREEN_FLOW0_HOLIDAY_0101,       // 元旦       00 04 00 01
    EM_SCREEN_FLOW0_HOLIDAY_0214,       // 情人节 00 04 00 02
    EM_SCREEN_FLOW0_HOLIDAY_0501,       // 劳动节 00 04 00 03
    EM_SCREEN_FLOW0_HOLIDAY_0601,       // 儿童节 00 04 00 04
    EM_SCREEN_FLOW0_HOLIDAY_1001,       // 国庆节 00 04 00 05
    EM_SCREEN_FLOW0_HOLIDAY_1225,       // 圣诞       00 04 00 06
    EM_SCREEN_FLOW0_LUNAR_1230,         // 除夕       00 04 01 01
    EM_SCREEN_FLOW0_LUNAR_0115,         // 元宵节 00 04 01 02
    EM_SCREEN_FLOW0_LUNAR_0505,         // 端午节 00 04 01 03
    EM_SCREEN_FLOW0_LUNAR_0815,         // 中秋节 00 04 01 04
    EM_SCREEN_FLOW0_LUNAR_0707,         // 七夕       00 04 01 05
        
    /* 0x0005 表情动作 */
    EM_SCREEN_FLOW0_ACT_BASE,           // 基础动作(基础表情) 00 05 00 01
    EM_SCREEN_FLOW0_ACT_GEAR,           // 齿轮 00 05 00 02
    EM_SCREEN_FLOW0_ACT_FACTORY,		// 恢复设置 00 05 00 03
    EM_SCREEN_FLOW0_ACT_GREET,		    // 打招呼 00 05 00 04
    EM_SCREEN_FLOW0_ACT_CLAP,		    // 鼓掌 00 05 00 05
    EM_SCREEN_FLOW0_ACT_FIREWORKS,      // 礼花 00 05 00 06
	
    EM_SCREEN_FLOW0_MAX = EM_SCREEN_FLOW0_ACT_FIREWORKS,
    EM_SCREEN_FLOW0_ALL,

    EM_SCREEN_FLOW0_NONE = 0xFF,     // 无效流程，默认值
}SCREEN_SHOW_MOTION_TYPE_E; 

/* EM_SCREEN_APP_FLOW1_SHOW_COVER -  展示图层流程详细枚举 */
typedef enum
{
    EM_SCREEN_FLOW1_1,
    EM_SCREEN_FLOW1_ALL,

    EM_SCREEN_FLOW1_NONE = 0xFF,     // 无效流程，默认值
}SCREEN_SHOW_COVER_TYPE_E; 

/* EM_SCREEN_APP_FLOW2_SWITCH_VER - 切换动图版本流程详细枚举 */
typedef enum
{
    EM_SCREEN_FLOW2_1,
    EM_SCREEN_FLOW2_ALL,

    EM_SCREEN_FLOW2_NONE = 0xFF,     // 无效流程，默认值
}SCREEN_SWITCH_VER_TYPE_E; 


/* EM_SCREEN_APP_FLOW3_GET_VER - 获取动图版本流程详细枚举 */
typedef enum
{
    EM_SCREEN_FLOW3_1,
    EM_SCREEN_FLOW3_ALL,

    EM_SCREEN_FLOW3_NONE = 0xFF,     // 无效流程，默认值
}SCREEN_GET_VER_TYPE_E; 


/* 流程状态执行结果 */
typedef enum
{
    SCREEN_APP_RESULT_IDLE = 0,         // 空闲
    SCREEN_APP_RESULT_RUNNING,          // 运行中    
    
    SCREEN_APP_RESULT_SUC,              // 执行成功
    SCREEN_APP_RESULT_FAIL,             // 执行失败
    SCREEN_APP_RESULT_ALL,              //流程最大值
}SCREEN_APP_FLOW_RESULT_E; 


#ifdef SMART_SCREEN_ON 


/*--------------------------------------------------变量声明---------------------------------------------------------*/             

/* 模组业务功能流程及入参结构体 */
typedef struct
{
    /* 入参 */
    SCREEN_APP_FLOW_TYPE_E      emAppFlow;      // 记录当前业务流程运转值
    uint32_t                    u32EmCfg;       // 入参动作，根据emAppFlow确认使用具体枚举
    
    /* 出参 */
    SCREEN_APP_FLOW_RESULT_E    emFlowResult;   // 运行结果,作为入参无效
    uint16_t                    u16OutPad;      // 操作出参，目前对外支持入参可配的仅ADD和DEL指纹，用于传出模板号
    
}ScreenAppParam_S; 

#endif


/*--------------------------------------------------函数声明---------------------------------------------------------*/

void App_SCREEN_Tim10Ms(void);
void APP_SCREEN_WakeUp(void);
void APP_SCREEN_Sleep(void);
bool APP_SCREEN_Operate(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint32_t _u32EmCfg);
uint8_t APP_SCREEN_GetResult(void);
bool APP_SCREEN_Update(SCREEN_APP_FLOW_TYPE_E _emAppFlow, uint8_t *_pu8Data, uint32_t _u32Len);
void APP_SCREEN_Scan(void);


#endif




