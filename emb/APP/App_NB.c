/********************************************************************************************************************
 * @file:        App_NB.c
 * @author:      Haixing.Huang
 * @version:     V01.00
 * @date:        2022年3月29日
 * @Description: NBIOT接口功能函数文件,基于BC28编写
 * @ChangeList:  01. 初版
 *********************************************************************************************************************/

/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "LockConfig.h"
#include "Public.h"

#include "DRV_GPIO.h"
#include "../DRV/DRV_EXPORT/DRV_74HC4052.h"

#include "../HAL/HAL_EXPORT/HAL_EXPORT.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#include "..\HAL\HAL_ADC\HAL_ADC.h"
#include "..\HAL\HAL_UART\HAL_UART.h"

//#include "../server/Face.h"

//#include "APP_FACE_PRO.h"
//#include "APP_CAM.h"
#include "App_WIFI.h"
#include "System.h"
#include "SystemTim.h"
#include "App_NB.h"
#if defined NB_FUNCTION
/*-------------------------------------------------宏定义-----------------------------------------------------------*/

#define NB_RST_GPIO_PIN M_WIFI_CT_GPIO_PIN
#define NB_PSM_GPIO_PIN M_WIFI_RT_GPIO_PIN
#define NB_POW_GPIO_PIN M_WIFI_POW_GPIO_PIN
#define NB_TX_GPIO_PIN M_WIFI_TX_GPIO_PIN
#define NB_RX_GPIO_PIN M_WIFI_RX_GPIO_PIN

#define NB_BAUDRATE UART_BAUD_RATE_9600

#define NB_TX_POW_OFF() DRV_GpioOut0(NB_TX_GPIO_PIN)
#define NB_RX_POW_OFF() DRV_GpioOut0(NB_RX_GPIO_PIN)

#define NB_RST_INIT() DRV_GpioInputPullnull(NB_RST_GPIO_PIN)
#define NB_PSM_INIT() DRV_GpioInputPullnull(NB_PSM_GPIO_PIN)

#define NB_RST_READ() DRV_GpioRead(NB_RST_GPIO_PIN)
#define NB_PSM_READ() DRV_GpioRead(NB_PSM_GPIO_PIN)

#define NB_POW_OFF() (HW_ACTIVE_LEVEL_LOW == M_POW_NB_ARSTIVE_LEVEL) ? (DRV_GpioOut1(NB_POW_GPIO_PIN)) : (DRV_GpioOut0(NB_POW_GPIO_PIN))
#define NB_POW_ON() (HW_ACTIVE_LEVEL_LOW == M_POW_NB_ARSTIVE_LEVEL) ? (DRV_GpioOut0(NB_POW_GPIO_PIN)) : (DRV_GpioOut1(NB_POW_GPIO_PIN))
#define NB_RST_OFF() DRV_GpioOut0(NB_RST_GPIO_PIN)
#define NB_RST_ON() DRV_GpioOut1(NB_RST_GPIO_PIN)

#define NB_PSM_OFF() DRV_GpioOut0(NB_PSM_GPIO_PIN)
#define NB_PSM_ON() DRV_GpioOut0(NB_PSM_GPIO_PIN)

#define NB_TX_BUFF_SIZE (256)
#define NB_RX_BUFF_SIZE (200)

#define NB_SERVICE_IP (106.15.134.146) // NB服务器IP
#define NB_SERVICE_PORT_UDP (8899)     // NB服务器UDP端口

#define RUN_TIME_MAX (3000) //最大运行时间
/*-------------------------------------------------枚举定义---------------------------------------------------------*/

/*-------------------------------------------------常量定义---------------------------------------------------------*/
typedef enum
{
    eNBAtStatusIdle,        //空闲
    eNBAtStatusSend,        //发送数据
    eNBAtStatusRecWait,     //接收等待
    eNBAtStatusRecOverTime, //接收超时
    eNBAtStatusRecSuccee,   //接收成功
    eNBAtStatusRecFail,     //接收失败
    eNBAtStatusRecError,    //接收错误
    eNBAtStatusRetry,       //重试
} TYPEe_NBAtStatus;         // AT指令状态
typedef enum
{
    eNBRunStatusIdle,        //空闲
    eNBRunStatusPowerUp,     //上电
    eNBRunStatusStartingUp,  //开机
    eNBRunStatusReset,       //复位
    eNBRunStatusWakeup,      //唤醒
    eNBRunStatusAutoGatt,    //自动附着流程
    eNBRunStatusReady,       //开机后的准备工作
    eNBRunStatusQueryState,  //查询连接状态
    eNBRunStatusCGATT,       //附着
    eNBRunStatusSetupSocket, //创建Socket
    eNBRunStatusCheckUE,     //获取UE等数据
    eNBRunStatusDataSend,    //数据发送
    eNBRunStatusSetPSM,      // PSM设置
    eNBRunStatusExit,        //退出系统
} TYPEe_NBRunStatus;         // NB运行状态
typedef enum
{
    eNBRstSoft,                //软件
    eNBRstSoftSucceed,         //软件复位成功
    eNBRstHardWare,            //硬件
    eNBRstHardWareWait,        //等待完成
    eNBRstHardWareStartupWait, //等待重启完成
    eNBRstHardWareCheck,       //检测时候复位成功
    eNBRstHardWareSucceed,     //硬件复位成功
    eNBRstFail,                //复位失败
} TYPEe_NBRst;                 //复位状态

typedef enum
{
    eNBCfunSet,        //设置
    eNBCfunCheck,      //检查
    eNBCfunWait,       //等待
    eNBCfunFunSucceed, //成功
    eNBCfunFunFail,    //失败
} TYPEe_NBCfun;        //复位状态

typedef struct
{
    uint8_t pAtCmd[NB_TX_BUFF_SIZE]; //发送的指令
    TYPEe_NBAtStatus status;         // AT指令工作状态
    uint32_t overTime;               //超时时间
    uint32_t overTimeCnt;            //超时计时器
    uint16_t rxCnt;                  //接收计数器
    uint8_t rxBuff[NB_RX_BUFF_SIZE]; //接收缓冲区
    TYPEe_NBRunStatus NBRunStatus;   // NB运行状态
    uint8_t NBRunStatusSecond;       // NB次运行状态
    uint8_t tmpStep;                 // NB临时运行转债
    uint8_t retry;                   //重试次数
    TYPEe_NBRst NBRst;               //复位状态
    TYPEe_NBCfun NBCfun;             // CFUN控制
    uint8_t connectFlag;             //连接标志
    uint8_t socketNum;               // socket 编号
    int16_t signalPower;             // Signal power
    int16_t txPower;                 // TX power
    uint16_t SNR;                    //信噪比
    uint16_t cellID;                 //小区
    uint8_t *updata;                 //上传数据的指针
    uint8_t updataLen;               //上传数据的长度
    uint8_t firstPowerUp;            //首次上电,0首次上电；1非首次上电
    uint8_t deadFlag;                //模块死机标志
    uint32_t runOverTime;            //模块整体运行时间
    uint8_t CSQGetFlag;              //获取信号标志
    uint8_t CSQ;                     //信号大小取绝对值范围在-113dBm~-53dBm.0XFF表示获取错误，0，表示获取中
} TYPEs_NBCommCtrl;                  //通讯控制器
TYPEs_NBCommCtrl NBCommCtrl = {
    0,
};
/*-------------------------------------------------局部变量定义-----------------------------------------------------*/

/*-------------------------------------------------局部变量定义-----------------------------------------------------*/

/*-------------------------------------------------全变量定义-----------------------------------------------------*/

/*-------------------------------------------------函数声明---------------------------------------------------------*/

static void App_NB_TxMessage(uint8_t *p_data, uint16_t length);

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
 * Function Name :  my_str_2_int()
 * Description   :  字符转换成整型
 * Para          :  char *cmd at指令 ,int32_t overtime 超时时间
 * Return        :  none
 *********************************************************************************************************************/
int my_str_2_int(char *string, char start_char)
{
    int data = 0, i = 0;
    char *str = string;
    uint8_t is_negtive = 0;
    while (*str != start_char)
    {
        str++;
        i++;
        if (i > 256)
            return 0;
    }
    str++;
    if (*str >= '0' && *str <= '9')
    {
        data = data * 10 + *str - '0';
    }
    else if (*str == '-')
    {
        is_negtive = 1;
    }
    str++;
    i = 0;
    while (*str >= '0' && *str <= '9')
    {

        data = data * 10 + *str - '0';
        str++;
        i++;
        if (i > 256)
            return 0;
    }
    if (is_negtive)
    {
        data = -data;
    }
    return data;
}
/*********************************************************************************************************************
 * Function Name :  App_NB_ATSend()
 * Description   :  NB AT指令发送
 * Para          :  char *cmd at指令 ,int32_t overtime 超时时间
 * Return        :  none
 *********************************************************************************************************************/
void App_NB_ATSend(char *cmd, int32_t overtime)
{
    uint32_t len = strlen(cmd); //获取发送的长度
    if (len > NB_TX_BUFF_SIZE)  //长度过长
    {
        my_printf("error: NB data too long \n");
        return;
    }
    if (eNBAtStatusIdle == NBCommCtrl.status)
    {
        NBCommCtrl.status = eNBAtStatusSend;                     //新的指令直接发送
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd)); //清空命令缓存
        memcpy(NBCommCtrl.pAtCmd, cmd, len);                     //拷贝需要发送的指令
        NBCommCtrl.overTime = overtime;
    }
    //如果有不同的指令，就重新开始发送
}

/*********************************************************************************************************************
 * Function Name :  App_NB_ATCmdClear()
 * Description   :  NB AT指令清除
 * Para          :  无
 * Return        :  无
 *********************************************************************************************************************/
void App_NB_ATCmdClear(void)
{
    NBCommCtrl.status = eNBAtStatusIdle;
}

/*********************************************************************************************************************
 * Function Name :  App_NB_PowerOn()
 * Description   :  NB上电
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_PowerOn(void)
{

    my_printf("NB_PowerOn\n");
#if defined NB_FUNCTION
    NB_POW_ON();

    //串口初始化
    UartCfg_S uartCfg = {0};
    uartCfg.BaudRate = NB_BAUDRATE;
    uartCfg.DataBit = DATA_8_BIT;
    uartCfg.StopBit = STOP_1_BIT;
    uartCfg.ParityType = PARITY_NONE;
    uartCfg.RxInerruptEn = INT_ENABLE;
    uartCfg.TxInerruptEn = INT_DISENABLE;
    HAL_Uart_ConfigInit(E_NB_UART, uartCfg);
    NBCommCtrl.NBRunStatusSecond = 0;
#endif
}

/*********************************************************************************************************************
 * Function Name :  App_NB_PowerOff()
 * Description   :  NB下电
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_PowerOff(void)
{
    my_printf("NB_PowerOff\n");
    if (E_NB_UART == HAL_Uart_GetCurDeviceType(NB_UART_COM))
    {
        HAL_Uart_DeInit(E_NB_UART);
    }
    NB_POW_OFF();
    NB_TX_POW_OFF();
    NB_RX_POW_OFF();
    NB_RST_OFF();
    NB_PSM_OFF();
}

/*********************************************************************************************************************
 * Function Name :  App_NB_CloseUart()
 * Description   :  NB串口关闭
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_CloseUart(void)
{
    my_printf("NB_CloseUart\n");
    if (E_NB_UART == HAL_Uart_GetCurDeviceType(NB_UART_COM))
    {
        my_printf("NB_CloseUart ing\n");
        HAL_Uart_DeInit(E_NB_UART);
    }
    //	NB_POW_OFF();
    //	NB_TX_POW_OFF();
    //    NB_RX_POW_OFF();
    NB_RST_OFF();
    NB_PSM_OFF();
}

/*********************************************************************************************************************
 * Function Name :  App_NB_Init()
 * Description   :  NB初始化
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_Init(void)
{
    //	App_NB_PowerOff();
    memset((uint8_t *)NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl));
    NB_POW_OFF();
    NB_TX_POW_OFF();
    NB_RX_POW_OFF();
    NB_RST_OFF();
    NB_PSM_OFF();
}

/*********************************************************************************************************************
 * Function Name :  App_NB_SleepInit()
 * Description   :  NB休眠配置,本处理需要放置到uart统一处理后面如HAL_Uart_SleepInit();
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_SleepInit(void)
{
    my_printf("App_NB_SleepInit\n");
    DRV_GpioInputPullnull(NB_RX_GPIO_PIN);
    DRV_GpioInputPullnull(NB_TX_GPIO_PIN); //正常状态下休眠配置
    NB_RST_OFF();
    NB_PSM_OFF();
}
/*********************************************************************************************************************
 * Function Name :  App_NB_SleepInitStart()
 * Description   :  获取NB信号强度
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_GetCSQStart(void)
{
    NBCommCtrl.CSQ = 0;
    NBCommCtrl.CSQGetFlag = 1;
    App_NB_Start();
}
/*********************************************************************************************************************
 * Function Name :  App_NB_SleepInitStart()
 * Description   :  获取NB信号强度
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
uint8_t App_NB_GetCSQ(void)
{
    if (2 == NBCommCtrl.CSQGetFlag)
    {
        NBCommCtrl.CSQGetFlag = 0;
    }
    return NBCommCtrl.CSQ;
}
/*********************************************************************************************************************
 * Function Name :  App_Wifi_TxMessage()
 * Description   :  uart 发送数据
 * Para   Input  :  p_data-待发送数据指针   length- 待发送数据长度
 * Para   Output :  none
 * Return        :  void
 * ChangeList    :  新增 2022-01-18 by gushengchi
 *********************************************************************************************************************/
void App_NB_TxMessage(uint8_t *p_data, uint16_t length)
{
#if defined NB_FUNCTION
    HAL_Uart_TxMessage(NB_UART_COM, p_data, length);
#endif
}
/*********************************************************************************************************************
 * Function Name :  App_NB_Reset()
 * Description   :  NB复位  软件复位：发送AT+NRB,模块回复REBOOTING，模块重启完成会发送“OK”
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
TYPEe_NBRst App_NB_Reset(void)
{
    switch (NBCommCtrl.NBRst)
    {
    case eNBRstSoft:                       //软件
        App_NB_ATSend("AT+NRB\r\n", 1000); //发送软件复位指令
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //模块能正常相应
            App_NB_ATCmdClear();
            NBCommCtrl.NBRst = eNBRstSoftSucceed;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            my_printf("ERROR: Reset no ack\n");
            NBCommCtrl.NBRst = eNBRstHardWare; //尝试使用硬件复位
        }
        break;
    case eNBRstSoftSucceed: //软件复位成功
        break;
    case eNBRstHardWare: //硬件
        NB_RST_ON();
        NBCommCtrl.overTimeCnt = 20; //延时需要等待
        NBCommCtrl.NBRst = eNBRstHardWareWait;
        break;
    case eNBRstHardWareWait: //等待完成
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NB_RST_OFF();
            NBCommCtrl.overTimeCnt = 420; //延时需要等待
            NBCommCtrl.NBRst = eNBRstHardWareStartupWait;
        }

        break;
    case eNBRstHardWareStartupWait: //等待重启完成
        if (0 == NBCommCtrl.overTimeCnt)
        {
            memset(NBCommCtrl.rxBuff, 0, sizeof(NBCommCtrl.rxBuff)); //清空接收缓存
            NBCommCtrl.NBRst = eNBRstHardWareCheck;
        }
        break;
    case eNBRstHardWareCheck:         //检测时候复位成功
        App_NB_ATSend("AT\r\n", 500); //发送软件复位指令
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //模块能正常相应
            App_NB_ATCmdClear();
            NBCommCtrl.NBRst = eNBRstHardWareSucceed;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            my_printf("ERROR: Reset no ack\n");
            NBCommCtrl.NBRunStatus = eNBRunStatusExit;
            NBCommCtrl.NBRst = eNBRstFail; //硬件复位失败
        }
        break;
    case eNBRstHardWareSucceed: //硬件复位成功
        break;
    case eNBRstFail: //复位失败
    default:
        break;
    }
    return NBCommCtrl.NBRst;
}

/*********************************************************************************************************************
* Function Name :  App_NB_FunSet()
* Description   :  NB CFUN设置
* Para          :  type 0
* Return        :  void

AT+CFUN=?
+CFUN:(0,1),(0,1)
OK
AT+CFUN=1 OK
AT+CFUN?
+CFUN:1
OK

*********************************************************************************************************************/
uint8_t App_NB_FunSet(uint8_t type)
{
    char *strx = NULL;
    switch (NBCommCtrl.NBCfun)
    {
    case eNBCfunSet:
        if (0 == type)
        {
            App_NB_ATSend("AT+CFUN=0\r\n", 2000); //发送软件复位指令
        }
        else
        {
            App_NB_ATSend("AT+CFUN=1\r\n", 2000);
        }

        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //模块能正常相应
            App_NB_ATCmdClear();
            NBCommCtrl.NBCfun = eNBCfunCheck;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            my_printf("FunSet no ack\n");
            if (0 != NBCommCtrl.retry)
            {
                NBCommCtrl.overTimeCnt = 50; //延时需要等待
                NBCommCtrl.NBCfun = eNBCfunWait;
            }
            else
            {
                NBCommCtrl.NBCfun = eNBCfunFunFail; //失败
            }
        }
        break;
    case eNBCfunCheck:
        App_NB_ATSend("AT+CFUN?\r\n", 2000); //
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //模块能正常相应
            if (0 == type)
            {

                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CFUN:0");
            }
            else
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CFUN:1");
            }
            App_NB_ATCmdClear();
            if (NULL != strx)
            {
                NBCommCtrl.NBCfun = eNBCfunFunSucceed;
            }
            else
            {
                if (0 != NBCommCtrl.retry)
                {
                    NBCommCtrl.overTimeCnt = 50; //延时需要等待
                    NBCommCtrl.NBCfun = eNBCfunWait;
                }
                else
                {
                    NBCommCtrl.NBCfun = eNBCfunFunFail; //失败
                }
            }
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            my_printf("FunSet? no ack\n");
            if (0 != NBCommCtrl.retry)
            {
                NBCommCtrl.overTimeCnt = 50; //延时需要等待
                NBCommCtrl.NBCfun = eNBCfunWait;
            }
            else
            {
                NBCommCtrl.NBCfun = eNBCfunFunFail; //失败
            }
        }

        break;
    case eNBCfunWait:
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NBCommCtrl.NBCfun = eNBCfunSet;
        }

        break;
    default:
        break;
    }
    return NBCommCtrl.NBCfun;
}

/*********************************************************************************************************************
 * Function Name :  App_NB_Time10Ms()
 * Description   :  NB 定时器处理
 * Para          :  none
 * Return        :  none
 *********************************************************************************************************************/
void App_NB_Time10Ms(void)
{
    if (NBCommCtrl.overTimeCnt > 0)
    {
        NBCommCtrl.overTimeCnt--;
    }
    if (NBCommCtrl.runOverTime > 0)
    {
        NBCommCtrl.runOverTime--;
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NB_ATDael()
 * Description   :  NB AT指令处理
 * Para          :  无
 * Return        :  TYPEe_NBAtStatus AT指令的状态
 *********************************************************************************************************************/
TYPEe_NBAtStatus App_NB_ATDael(void)
{
#if defined NB_FUNCTION
    char *strx = NULL;
    uint8_t tmp;
    if (E_NB_UART != HAL_Uart_GetCurDeviceType(NB_UART_COM)) //保护串口
    {
        return eNBAtStatusIdle;
    }
    switch (NBCommCtrl.status)
    {
    case eNBAtStatusIdle: //空闲
        break;
    case eNBAtStatusSend:
        memset(NBCommCtrl.rxBuff, 0, sizeof(NBCommCtrl.rxBuff));                      //清空接收缓存
        App_NB_TxMessage(NBCommCtrl.pAtCmd, strlen((const char *)NBCommCtrl.pAtCmd)); //发送数据
        my_printf("NB Send:%s", NBCommCtrl.pAtCmd);                                   //打印日志
        NBCommCtrl.overTimeCnt = NBCommCtrl.overTime;                                 //添加超时时间
        NBCommCtrl.status = eNBAtStatusRecWait;
        NBCommCtrl.rxCnt = 0;
        break;
    case eNBAtStatusRecWait:                                                 //接收等待
        while (UART_SUCCESS == HAL_Uart_PopByteFromQueue(NB_UART_COM, &tmp)) //从接收缓冲区获取数据
        {
            NBCommCtrl.rxBuff[NBCommCtrl.rxCnt] = tmp;
            NBCommCtrl.rxCnt++;
            if (NBCommCtrl.rxCnt > NB_RX_BUFF_SIZE) //防止溢出
            {
                NBCommCtrl.rxCnt = NB_RX_BUFF_SIZE - 1;
            }
        }
        strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"OK");
        if (NULL != strx) //返回OK
        {
            my_printf("NB rec ok:%s\n", NBCommCtrl.rxBuff); //打印日志
            NBCommCtrl.status = eNBAtStatusRecSuccee;       //接收成功
        }
        strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"ERROR");
        if (NULL != strx) //返回ERROR
        {
            my_printf("NB rec error:%s\n", NBCommCtrl.rxBuff); //打印日志
            NBCommCtrl.status = eNBAtStatusRecError;           //接收错误
        }
        if (0 == NBCommCtrl.overTimeCnt) //超时的情况下
        {
            NBCommCtrl.status = eNBAtStatusRecOverTime; //接收超时
        }
        break;
    case eNBAtStatusRecOverTime: //接收超时

        break;
    case eNBAtStatusRecSuccee: //接收成功
        /*继续接收获取一些芯片主动发送的命令*/
        while (UART_SUCCESS == HAL_Uart_PopByteFromQueue(NB_UART_COM, &tmp)) //从接收缓冲区获取数据
        {
            NBCommCtrl.rxBuff[NBCommCtrl.rxCnt] = tmp;
            NBCommCtrl.rxCnt++;
            if (NBCommCtrl.rxCnt > NB_RX_BUFF_SIZE) //防止溢出
            {
                NBCommCtrl.rxCnt = NB_RX_BUFF_SIZE - 1;
            }
        }
        break;
    case eNBAtStatusRecFail: //接收失败

        break;
    case eNBAtStatusRecError: //接收错误

        break;
    case eNBAtStatusRetry: //重试

    default:
        break;
    }
    return NBCommCtrl.status;
#else
    return NBCommCtrl.status;
#endif
}

/*********************************************************************************************************************
 * Function Name :  App_NB_CheckBusy()
 * Description   :  NB 查询是否处于忙碌状态
 * Para          :  无
 * Return        :  1,空闲；-1 忙碌
 *********************************************************************************************************************/
int8_t App_NB_CheckBusy(void)
{
    if ((eNBRunStatusIdle == NBCommCtrl.NBRunStatus) && (0 == WIFI_UploadDataGetLength()))
    {

        return 1;
    }
    return -1;
}
/*********************************************************************************************************************
 * Function Name :  App_NB_Start()
 * Description   :  NB 启动 如果已经在运行则不需要再次启动
 * Para          :  无
 * Return        :
 *********************************************************************************************************************/
void App_NB_Start(void)
{
    //	if(1 == App_NB_CheckBusy())
    {
        NBCommCtrl.NBRunStatus = eNBRunStatusPowerUp;
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NB_Start()
 * Description   :  模组联网 等待联网30秒
 * Para          :  无
 * Return        :  联网中 0；联网成功 ：1； 联网失败 ： 2；
 *********************************************************************************************************************/
uint8_t App_NB_NetWorking(void)
{
    if (0 == NBCommCtrl.firstPowerUp)
    {
        return 0;
    }
    else if (1 == NBCommCtrl.firstPowerUp)
    {
        return 1;
    }
    else if (2 == NBCommCtrl.firstPowerUp)
    {
        return 2;
    }
    return 0;
}

/*********************************************************************************************************************
* Function Name :  App_NBRunStatusPowerUp()
* Description   :  case处理
* Para          :  无
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusPowerUp(void)
{
    App_NB_PowerOn();                      //给模块上电
    NBCommCtrl.runOverTime = RUN_TIME_MAX; //填写整体运行时间
    if (0 == NBCommCtrl.connectFlag)
    {
        /*未联网*/
        my_printf("nb PowerUp Starting\n");
        NBCommCtrl.NBRunStatus = eNBRunStatusStartingUp;
    }
    else
    {
        my_printf("nb wakeup\n");
        NBCommCtrl.NBRunStatus = eNBRunStatusWakeup;
    }
}
/*********************************************************************************************************************
* Function Name :  App_NBRunStatusStartingUp()
* Description   :  case处理
* Para          :  无
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusStartingUp(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.overTimeCnt = 500; //等待NB启动完成，4秒左右
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 1:
        if (0 == NBCommCtrl.overTimeCnt) //启动完成
        {
            NBCommCtrl.NBRunStatusSecond++;
            NBCommCtrl.retry = 2;
        }
        break;
    case 2:
        if (NBCommCtrl.retry > 0)
        {
            App_NB_ATSend("AT\r\n", 50); //发送AT，模块回复OK为正常
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                App_NB_ATCmdClear();
#if 0
				if(0 == NBCommCtrl.connectFlag) 
				{
					/*未联网*/
					NBCommCtrl.NBRunStatusSecond++;
				}
				else
				{
					NBCommCtrl.NBRunStatus = eNBRunStatusSetPSM;
					NBCommCtrl.NBRunStatusSecond =0;
				}
#else
                NBCommCtrl.NBRunStatusSecond++;
#endif
            }
            else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                     (eNBAtStatusRecFail == NBCommCtrl.status) ||
                     (eNBAtStatusRecError == NBCommCtrl.status))
            {
                if (NBCommCtrl.retry > 0)
                {
                    NBCommCtrl.retry--;
                }
                App_NB_ATCmdClear();
                if (0 == NBCommCtrl.retry)
                {
                    my_printf("ERROR: power up AT no ack\n");
                    NBCommCtrl.NBRunStatus = eNBRunStatusExit;
                    NBCommCtrl.NBRunStatusSecond = 0;
                    NBCommCtrl.deadFlag = 1;
                }
            }
        }
        break;
    case 3:
        NBCommCtrl.retry = 3;
        NBCommCtrl.tmpStep = 0; //临时步骤
        if (0 == NBCommCtrl.firstPowerUp)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        else
        {
            NBCommCtrl.NBRunStatusSecond = 9; //直接进入设置CFUN
        }
        break;
    case 4:
        if (0 == NBCommCtrl.tmpStep)
        {
            App_NB_ATSend("AT+QREGSWT=2\r\n", 2000); //关闭BC28周期性1h自注册电信IOT平台
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                App_NB_ATCmdClear();
                NBCommCtrl.tmpStep++;
            }
            else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                     (eNBAtStatusRecFail == NBCommCtrl.status) ||
                     (eNBAtStatusRecError == NBCommCtrl.status))
            {
                NBCommCtrl.retry--;
                App_NB_ATCmdClear();
                if (0 == NBCommCtrl.retry)
                {
                    my_printf("ERROR: power up AT+QREGSWT=2 no ack\n");
                    NBCommCtrl.NBRunStatus = eNBRunStatusExit;
                    //                    App_NB_PowerOff();
                }
            }
        }
        else if (1 == NBCommCtrl.tmpStep)
        {
            App_NB_ATSend("AT+QREGSWT?\r\n", 2000);
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                App_NB_ATCmdClear();
                NBCommCtrl.NBRunStatusSecond++;
            }
            else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                     (eNBAtStatusRecFail == NBCommCtrl.status) ||
                     (eNBAtStatusRecError == NBCommCtrl.status))
            {
                NBCommCtrl.retry--;
                App_NB_ATCmdClear();
                NBCommCtrl.tmpStep++;
                NBCommCtrl.overTimeCnt = 50; //等待一下再发指令
                if (0 == NBCommCtrl.retry)
                {
                    my_printf("ERROR: power up AT+QREGSWT? no ack\n");
                    NBCommCtrl.NBRunStatus = eNBRunStatusExit;
                    //                    App_NB_PowerOff();
                }
            }
        }
        else
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.tmpStep = 0;
            }
        }
        break;
    case 5:
        NBCommCtrl.tmpStep = 0; //临时步骤
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 6:
        App_NB_ATSend("AT+CPSMS=1,,,\"01011111\",\"00000001\"\r\n", 100); //休眠配置,重启或者休眠可以保存
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            my_printf("ERROR:AT+CPSMS\n");
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 7:
        NBCommCtrl.tmpStep = 0;      //临时步骤
        NBCommCtrl.overTimeCnt = 10; //稍微延迟
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 8:
        if (0 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.NBRst = eNBRstSoft; //先用软件复位的方式
                NBCommCtrl.tmpStep++;
            }
        }
        else if (1 == NBCommCtrl.tmpStep)
        {
            App_NB_Reset(); //复位
            if ((eNBRstSoftSucceed == NBCommCtrl.NBRst) ||
                (eNBRstHardWareSucceed == NBCommCtrl.NBRst) ||
                (eNBRstFail == NBCommCtrl.NBRst))
            {
                NBCommCtrl.tmpStep = 0;
                NBCommCtrl.NBRunStatusSecond++;
            }
        }

        break;
    case 9:
        NBCommCtrl.tmpStep = 0; //临时步骤
        NBCommCtrl.overTimeCnt = 300;
        NBCommCtrl.NBRunStatusSecond++;

        break;
    case 10:
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 11:
        NBCommCtrl.NBCfun = eNBCfunSet;
        NBCommCtrl.retry = 10; // cfun设置重试
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 12:
        App_NB_FunSet(1);
        if ((eNBCfunFunSucceed == NBCommCtrl.NBCfun) ||
            (eNBCfunFunFail == NBCommCtrl.NBCfun))
        {
            NBCommCtrl.retry = 3;
            NBCommCtrl.overTimeCnt = 30;
            NBCommCtrl.NBCfun = eNBCfunSet;
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 13:
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 14:
        App_NB_ATSend("AT+CGATT=1\r\n", 100);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            App_NB_ATCmdClear();
            NBCommCtrl.overTimeCnt = 100; //需要延迟一段时间
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            NBCommCtrl.retry--;
            App_NB_ATCmdClear();
            if (0 == NBCommCtrl.retry)
            {
                my_printf("ERROR: power up AT+CGATT no ack\n");
                NBCommCtrl.NBRunStatusSecond++;
            }
        }
        break;
    case 15:
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 16:
        App_NB_ATSend("AT+CSQ\r\n", 100);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            if (NULL != strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CSQ:99,99")) //无信号
            {
                my_printf("ERROR: power up AT+CSQ 99,99\n");
                NBCommCtrl.NBRunStatus = eNBRunStatusExit;
                App_NB_PowerOff();
            }
            else
            {

                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusReady;
            }
            App_NB_ATCmdClear();
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            NBCommCtrl.retry--;
            App_NB_ATCmdClear();
            if (0 == NBCommCtrl.retry)
            {
                my_printf("ERROR: power up AT+CSQ no ack\n");
                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusExit;
                App_NB_PowerOff();
            }
        }
        break;

    default:
        break;
    }
}

/*********************************************************************************************************************
* Function Name :  App_NBRunStatusReset()
* Description   :  case处理
* Para          :  无
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusReset(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        //判断是否出现模块无应答状态有的话直接退出
        NBCommCtrl.NBRunStatusSecond++;
        NBCommCtrl.NBRst = eNBRstSoft; //先用软件复位的方式
        break;
    case 1:
        App_NB_Reset(); //复位
        if ((eNBRstSoftSucceed == NBCommCtrl.NBRst) ||
            (eNBRstHardWareSucceed == NBCommCtrl.NBRst))
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        if (eNBRstFail == NBCommCtrl.NBRst) //复位失败，模块停止工作。
        {
            my_printf("ERROR:NB IS DEAD!!!\n");
            NBCommCtrl.NBRunStatus = eNBRunStatusExit;
            NBCommCtrl.NBRunStatusSecond = 0;
            NBCommCtrl.connectFlag = 0;
            //			NBCommCtrl.deadFlag = 1;
        }
        break;
    case 2:
        NBCommCtrl.NBRunStatusSecond = 0;
        NBCommCtrl.NBRunStatus = eNBRunStatusReady;
        NBCommCtrl.connectFlag = 0;
        my_printf("nb reset \n");
        break;
    default:
        break;
    }
}

/*********************************************************************************************************************
* Function Name :  App_NBRunStatusWakeup()
* Description   :  case处理
* Para          :  无
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusWakeup(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 1:
        App_NB_ATSend("AT\r\n", 100);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            App_NB_ATCmdClear();
            NBCommCtrl.overTimeCnt = 4;
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            my_printf("nb wakeup overtime\n");
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 2:
        if (0 == NBCommCtrl.overTimeCnt)
        {
            if (0 == NBCommCtrl.connectFlag)
            {
                my_printf("nb starting up\n");
                NBCommCtrl.NBRunStatus = eNBRunStatusStartingUp;
            }
            else
            {
                my_printf("nb QueryState\n");
                NBCommCtrl.NBRunStatus = eNBRunStatusQueryState;
            }
            NBCommCtrl.NBRunStatusSecond = 0;
        }
        break;
    default:
        break;
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusReady()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusReady(void)
{
    static uint8_t step = 0; //临时步骤
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        App_NB_ATSend("ATE1\r\n", 50);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 1:
        App_NB_ATSend("AT+CSCON=1\r\n", 50);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 2:
        step = 0;
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 3:
        if (0 == NBCommCtrl.connectFlag)
        {
            if (0 == step)
            {
                App_NB_ATSend("AT+QLEDMODE=0\r\n", 50); //关闭指示灯功能
                if (eNBAtStatusRecSuccee == NBCommCtrl.status)
                {
                    App_NB_ATCmdClear();
                    step++;
                }
                else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                         (eNBAtStatusRecFail == NBCommCtrl.status) ||
                         (eNBAtStatusRecError == NBCommCtrl.status))
                {
                    App_NB_ATCmdClear();
                    step++;
                }
            }
            else if (1 == step)
            {
                App_NB_ATSend("AT+NPSMR=1\r\n", 50); //省电模式状态上报
                if (eNBAtStatusRecSuccee == NBCommCtrl.status)
                {
                    App_NB_ATCmdClear();
                    step++;
                }
                else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                         (eNBAtStatusRecFail == NBCommCtrl.status) ||
                         (eNBAtStatusRecError == NBCommCtrl.status))
                {
                    App_NB_ATCmdClear();
                    step++;
                }
            }
            else if (2 == step)
            {
                App_NB_ATSend("AT+NCCID\r\n", 50); //识别 USIM 卡
                if (eNBAtStatusRecSuccee == NBCommCtrl.status)
                {
                    App_NB_ATCmdClear();
                    NBCommCtrl.NBRunStatusSecond++;
                }
                else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                         (eNBAtStatusRecFail == NBCommCtrl.status) ||
                         (eNBAtStatusRecError == NBCommCtrl.status))
                {
                    App_NB_ATCmdClear();
                    NBCommCtrl.NBRunStatusSecond++;
                }
            }
        }
        else
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 4:
        NBCommCtrl.NBRunStatus = eNBRunStatusQueryState;
        NBCommCtrl.NBRunStatusSecond = 0;
        break;
    default:
        break;
    }
}
/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusQueryStat()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusQueryStat(void)
{
    char *strx = NULL;
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 1:
        //电量低关机
        NBCommCtrl.NBRunStatusSecond++;
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.retry = 3;
        break;
    case 2:
        if (0 == NBCommCtrl.tmpStep)
        {
            App_NB_ATSend("AT+CEREG?\r\n", 50); // EPS 网络注册状态
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CEREG:");
                if (NULL == strx)
                {
                    my_printf("ERROR:AT+CEREG? no ack\n");
                }
                else
                {
                    if (('1' == *(strx + 9)) || ('5' == *(strx + 9))) //已注册，本地网络；已注册，漫游网络
                    {
                        if (0 == NBCommCtrl.connectFlag)
                        {
                            NBCommCtrl.tmpStep = 1;
                            NBCommCtrl.overTimeCnt = 100;
                        }
                        else
                        {
                            NBCommCtrl.NBRunStatus = eNBRunStatusCGATT;
                        }
                    }
                    else
                    {
                        if (NBCommCtrl.retry > 0)
                        {
                            NBCommCtrl.retry--;
                            NBCommCtrl.tmpStep = 20;
                            NBCommCtrl.overTimeCnt = 500; //延迟一下，等待网络注册成功
                        }
                        else
                        {
                            NBCommCtrl.tmpStep = 10;
                            NBCommCtrl.overTimeCnt = 100;
                            NBCommCtrl.NBRunStatusSecond++;
                        }
                    }
                }
                App_NB_ATCmdClear();
            }
            else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                     (eNBAtStatusRecFail == NBCommCtrl.status) ||
                     (eNBAtStatusRecError == NBCommCtrl.status))
            {
                App_NB_ATCmdClear();
                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusReset;
            }
        }
        else if (1 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusCGATT;
            }
        }
        else if (10 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusReset;
            }
        }
        else if (20 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.tmpStep = 0;
            }
        }
        break;

    default:
        break;
    }
}
/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusCGATT()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusCGATT(void)
{
    char *strx = NULL;
    App_NB_ATSend("AT+CGATT?\r\n", 50); // EPS 网络注册状态
    if (eNBAtStatusRecSuccee == NBCommCtrl.status)
    {
        strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CGATT:");
        if (NULL == strx)
        {
            my_printf("ERROR:AT+CGATT?? no ack\n");
        }
        else
        {
            if ('1' == *(strx + 7)) //附着成功
            {
                my_printf("AT+CGATT OK\n");
                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusSetupSocket;
            }
            else
            {
                my_printf("ERROR:AT+CGATT?? no CGATT");
                NBCommCtrl.NBRunStatusSecond = 0;
                NBCommCtrl.NBRunStatus = eNBRunStatusReset;
            }
        }
        App_NB_ATCmdClear();
    }
    else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
             (eNBAtStatusRecFail == NBCommCtrl.status) ||
             (eNBAtStatusRecError == NBCommCtrl.status))
    {
        my_printf("AT+CGATT error\n");
        App_NB_ATCmdClear();
        NBCommCtrl.NBRunStatusSecond = 0;
        NBCommCtrl.NBRunStatus = eNBRunStatusReset;
    }
}
/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusSetupSocket()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusSetupSocket(void)
{
    char *strx = NULL;
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.NBRunStatusSecond++;
        App_NB_ATCmdClear();
        if (0 == NBCommCtrl.connectFlag)
        {
            NBCommCtrl.connectFlag = 1;
        }
        else
        {
            NBCommCtrl.NBRunStatus = eNBRunStatusCheckUE;
            break;
        }

        break;
    case 1:

        App_NB_ATSend("AT+NSOCR=DGRAM,17,0,1\r\n", 200); //创建 Socket
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"AT+NSOCR=DGRAM,17,0,1");
            if (NULL == strx)
            {
                my_printf("ERROR:AT+NSOCR=DGRAM,17,0,1 no ack\n");
                NBCommCtrl.NBRunStatus = eNBRunStatusExit;
                App_NB_ATCmdClear();
            }
            else
            {
                for (int i = 21; i < 50; i++)
                {
                    if (*(strx + i) >= '0' && *(strx + i) <= '9')
                    {
                        NBCommCtrl.socketNum = *(strx + i);
                        break;
                    }
                }
                NBCommCtrl.NBRunStatus = eNBRunStatusCheckUE;
            }
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
        }
        else if ((eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond++;
            my_printf("eNBAtStatusRecFail\n");
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
            NBCommCtrl.NBRunStatus = eNBRunStatusReset;
            my_printf("eNBAtStatusRecOverTime\n");
        }

        break;
    case 2:
        NBCommCtrl.connectFlag = 0;
        App_NB_ATSend("AT+NSOCL=1\r\n", 200); //关闭 Socket
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
            NBCommCtrl.NBRunStatus = eNBRunStatusReset;
        }
        break;
    default:
        break;
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusCheckUE()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusCheckUE(void)
{
    char *strx = NULL;
    char CSQBuff[2] = {
        0,
    };
    uint8_t csqTmp = 0;
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.NBRunStatusSecond++;
        App_NB_ATCmdClear();
        break;
    case 1:
        if (0 == NBCommCtrl.tmpStep)
        {
            App_NB_ATSend("AT+NUESTATS\r\n", 200); //查询 UE 统计信息
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"Signal power:"); //返回OK
                if (strx != NULL)
                {
                    NBCommCtrl.signalPower = my_str_2_int(strx, ':');
                    strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"TX power:"); //返回OK
                    NBCommCtrl.txPower = my_str_2_int(strx, ':');
                    strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"SNR:"); //返回OK
                    NBCommCtrl.SNR = my_str_2_int(strx, ':');
                    strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"Cell ID:"); //返回OK
                    NBCommCtrl.cellID = my_str_2_int(strx, ':');
                }
                App_NB_ATCmdClear();
                NBCommCtrl.NBRunStatusSecond++;
            }
            else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                     (eNBAtStatusRecFail == NBCommCtrl.status) ||
                     (eNBAtStatusRecError == NBCommCtrl.status))
            {
                App_NB_ATCmdClear();
                NBCommCtrl.tmpStep = 1;
                NBCommCtrl.overTimeCnt = 10;
            }
        }
        else if (1 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.tmpStep = 0;
                NBCommCtrl.NBRunStatusSecond++;
            }
        }
        break;
    case 2:
        App_NB_ATSend("AT+CSQ\r\n", 35);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //            strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"Cell ID:"); //返回OK
            //            if (NULL != strx)
            //            {
            //            }
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
            if (1 == NBCommCtrl.CSQGetFlag)
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CSQ:"); //返回OK
                if (NULL != strx)
                {
                    memcpy(CSQBuff, strx + (sizeof("+CSQ:") - 1), 2);
                    my_printf("Get CSQ IS:%s\n", CSQBuff);
                    if (('9' == CSQBuff[0]) && ('9' == CSQBuff[1])) //未知
                    {
                        NBCommCtrl.CSQ = 0XFF; //失败
                    }
                    else if ((('0' <= CSQBuff[0]) && (CSQBuff[0] <= '9')) &&
                             (('0' <= CSQBuff[1]) && (CSQBuff[1] <= '9')))
                    {
                        NBCommCtrl.CSQ = (113 - ((CSQBuff[0] - '0') * 10 + (CSQBuff[1] - '0')) * 2);
                    }
                    else if (('0' <= CSQBuff[0]) && (CSQBuff[0] <= '9'))
                    {
                        NBCommCtrl.CSQ = (113 - (CSQBuff[0] - '0') * 2);
                    }
                    else
                    {
                        NBCommCtrl.CSQ = 0XFF; //失败
                    }
                    my_printf("Get CSQ2 IS:%d\n", NBCommCtrl.CSQ);
                    NBCommCtrl.CSQGetFlag = 2;                 //获取到结果
                    NBCommCtrl.NBRunStatus = eNBRunStatusExit; //进行数据发送
                }
            }
            else
            {
                NBCommCtrl.NBRunStatus = eNBRunStatusDataSend; //进行数据发送
                my_printf("NBRunStatusDataSend\n");
            }
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
            NBCommCtrl.NBRunStatus = eNBRunStatusReset;
        }
        break;
    default:
        break;
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusDataSend()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusDataSend(void)
{
#define SEND_OVER_TIME (400)
    uint8_t txbuf[WIFI_UPLOAD_DATA_LENGTH_MAX] = {0}; //发送缓存
    uint16_t len;                                     //发送长度
    char dataBuff[20] = {0};
    uint16_t strIndex = 0;
    uint32_t i = 0;
    char *strx = NULL;
    char *strx1 = NULL;
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.NBRunStatusSecond++;
        //        if (0 == NBCommCtrl.firstPowerUp)
        //        {
        //            /*首次上电退出*/
        //            NBCommCtrl.firstPowerUp = 1;
        //            NBCommCtrl.NBRunStatus = eNBRunStatusExit;
        //            my_printf("power up first ok\n");
        //        }
        //        else
        //        {
        //            NBCommCtrl.NBRunStatusSecond++;
        //        }
        break;
    case 1:
        NBCommCtrl.status = eNBAtStatusSend;                                         //新的指令直接发送
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd));                     //清空命令缓存
        memcpy(NBCommCtrl.pAtCmd, "AT+NSORF=1,80\r\n", sizeof("AT+NSORF=1,80\r\n")); //读取数据，用于清空回复
        NBCommCtrl.pAtCmd[9] = NBCommCtrl.socketNum;                                 //填写socket号
        NBCommCtrl.overTime = 100;
        NBCommCtrl.NBRunStatusSecond++;
        NBCommCtrl.tmpStep = 0;
        break;
    case 2:
        if (0 == NBCommCtrl.tmpStep)
        {
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                NBCommCtrl.overTimeCnt = 300;
                NBCommCtrl.tmpStep = 1;
            }
            else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                     (eNBAtStatusRecFail == NBCommCtrl.status) ||
                     (eNBAtStatusRecError == NBCommCtrl.status))
            {
                NBCommCtrl.overTimeCnt = 0;
                NBCommCtrl.tmpStep = 1;
            }
        }
        else if (1 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.tmpStep = 0;
                NBCommCtrl.NBRunStatusSecond++;
            }
        }
        break;
    case 3:
        NBCommCtrl.status = eNBAtStatusSend;                     //新的指令直接发送
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd)); //清空命令缓存
        memcpy(NBCommCtrl.pAtCmd,
               "AT+NSOSTF=1,106.15.134.146,8899,0x400,",
               sizeof("AT+NSOSTF=1,106.15.134.146,8899,0x400,")); //新测试服 UDP  等待接收到下行数据再休眠
	
		if (WIFI_UploadData_Get(txbuf, &len) == QUEUE_OK) //获取发送的数据和长度
		{
			NBCommCtrl.updata = txbuf;
			NBCommCtrl.updataLen = len;
		}
		
		if(0 == NBCommCtrl.updataLen)
		{
			/*没有需要发送的数据*/
			if (0 == NBCommCtrl.firstPowerUp) 
			{
				/*第一次上电，填写一个数据0，加快功耗下降*/
				len = 1;
				NBCommCtrl.updata = txbuf;
				NBCommCtrl.updataLen = len;
			}
			else
			{
				/*非第一次上电，直接结束*/
				App_NB_ATCmdClear();
				NBCommCtrl.NBRunStatusSecond = 5;
			}
			
		}
		else
		{
			NBCommCtrl.pAtCmd[10] = NBCommCtrl.socketNum;  //填写socket号
			sprintf(dataBuff, "%d", NBCommCtrl.updataLen); //将待发数据长度转成字符串

			strcat((char *)NBCommCtrl.pAtCmd, dataBuff);  //追加长度
			strcat((char *)NBCommCtrl.pAtCmd, ",\0\0");   //追加","
			strIndex = strlen((char *)NBCommCtrl.pAtCmd); //获取长度
			for (i = 0; i < NBCommCtrl.updataLen; i++)    //待发送数据
			{
				sprintf(((char *)NBCommCtrl.pAtCmd + strIndex) + i * 2, "%02X", NBCommCtrl.updata[i]);
			}
			strcat((char *)NBCommCtrl.pAtCmd, "\r\n"); //追加回车
			NBCommCtrl.overTime = SEND_OVER_TIME;
			NBCommCtrl.retry = 3;
			NBCommCtrl.NBRunStatusSecond++;
		}
		
        break;
    case 4:
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+NSONMI");   //返回SEND OK
            strx1 = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CSCON:0"); //返回进入IDLE模式
            if ((NULL == strx) && (NULL == strx1))                                     //要求在规定时间累收到指定字符
            {
                if (0 == NBCommCtrl.overTimeCnt)
                {
                    if (NBCommCtrl.retry > 0)
                    {
                        /*重发*/
                        NBCommCtrl.retry--;
                        NBCommCtrl.overTime = SEND_OVER_TIME;
                        NBCommCtrl.status = eNBAtStatusSend; //新的指令直接发送
                    }
                    else
                    {
                        NBCommCtrl.NBRunStatusSecond++;
                    }
                }
            }
            else
            {
                my_printf("data send succed!\n");
                if (0 != WIFI_UploadDataGetLength()) //检查剩余的数据
                {
                    NBCommCtrl.NBRunStatusSecond = 3; //重发
                }
                else
                {
                    NBCommCtrl.NBRunStatusSecond++;
                }
            }
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            if (NBCommCtrl.retry > 0)
            {
                NBCommCtrl.retry--;
                NBCommCtrl.NBRunStatusSecond = 3; //重发
            }
            else
            {
                NBCommCtrl.NBRunStatusSecond++;
            }
        }
        break;

    case 5:
        NBCommCtrl.status = eNBAtStatusSend;                                         //新的指令直接发送
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd));                     //清空命令缓存
        memcpy(NBCommCtrl.pAtCmd, "AT+NSORF=1,80\r\n", sizeof("AT+NSORF=1,80\r\n")); //读取数据，用于清空回复
        NBCommCtrl.pAtCmd[9] = NBCommCtrl.socketNum;                                 //填写socket号
        NBCommCtrl.overTime = 100;
        NBCommCtrl.NBRunStatusSecond++;
        NBCommCtrl.tmpStep = 0;
        break;
    case 6:
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            NBCommCtrl.NBRunStatusSecond++;
            App_NB_ATCmdClear();
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            NBCommCtrl.NBRunStatusSecond++;
            App_NB_ATCmdClear();
        }
        break;
    case 7:
        if (0 == NBCommCtrl.firstPowerUp)
        {
            NBCommCtrl.firstPowerUp = 1;
            my_printf("power up first ok\n");
        }
        NBCommCtrl.NBRunStatusSecond = 0;
        NBCommCtrl.NBRunStatus = eNBRunStatusExit; //退出推送
        break;
    default:
        break;
    }
#undef SEND_OVER_TIME
}

/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusSetPSM()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusSetPSM(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.NBRunStatusSecond = 0;
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 1:
        App_NB_ATSend("AT+QLEDMODE=0\r\n", 35);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 2:
        App_NB_ATSend("AT+NPSMR?\r\n", 100);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 3:
        App_NB_ATSend("AT+CPSMS=1,,,\"01011111\",\"00000001\"\r\n", 100);
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {

            NBCommCtrl.NBRunStatusSecond++;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            my_printf("ERROR:AT+CPSMS\n");
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 4:
        NBCommCtrl.NBRunStatusSecond = 0;
        NBCommCtrl.NBRunStatus = eNBRunStatusExit;
        break;
    default:
        break;
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusExit()
 * Description   :  case处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusExit(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        if (0 == NBCommCtrl.firstPowerUp)
        {
            NBCommCtrl.firstPowerUp = 2; //为正常联网
        }
        if (0 == NBCommCtrl.connectFlag)
        {
            /*关机模组，节省功耗*/
            NBCommCtrl.NBCfun = eNBCfunSet;
            NBCommCtrl.retry = 10;
            NBCommCtrl.NBRunStatusSecond++;
        }
        else
        {
            App_NB_CloseUart(); //关闭串口
            NBCommCtrl.NBRunStatus = eNBRunStatusIdle;
            NBCommCtrl.NBRunStatusSecond = 0;
        }
        NBCommCtrl.tmpStep = 0;
        break;
    case 1:
        App_NB_FunSet(0);
        if ((eNBCfunFunSucceed == NBCommCtrl.NBCfun) ||
            (eNBCfunFunFail == NBCommCtrl.NBCfun))
        {
            NBCommCtrl.overTimeCnt = 30;
            NBCommCtrl.NBCfun = eNBCfunSet;
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 2:
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        break;
    case 3:
        my_printf("test power Off\n");

        NBCommCtrl.NBRunStatus = eNBRunStatusIdle;
        NBCommCtrl.NBRunStatusSecond = 0;
        break;
    default:
        break;
    }
}
/*********************************************************************************************************************
 * Function Name :  App_NB_Dael()
 * Description   :  NB 处理
 * Para          :  无
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_Dael(void)
{
    uint8_t txbuf[WIFI_UPLOAD_DATA_LENGTH_MAX] = {0}; //发送缓存
    uint16_t len;
	/*空闲*/
    if (1 == App_NB_CheckBusy()) 
    {
        return;
    }
	/*串口被占用*/
    if (((HAL_Uart_GetCurDeviceType(NB_UART_COM) == E_FACE_UART) ||
         (HAL_Uart_GetCurDeviceType(NB_UART_COM) == E_FINGER_UART) ||
         (HAL_Uart_GetCurDeviceType(NB_UART_COM) == E_CAMERA_UART)) 
		)
//        && (NBCommCtrl.NBRunStatus < eNBRunStatusDataSend))         //上电未发送时串口被切走重发
    {
        NBCommCtrl.NBRunStatus = eNBRunStatusPowerUp;
        return;
    }

    if (1 == NBCommCtrl.deadFlag) //如果硬件出现死机的情况
    {
        NBCommCtrl.deadFlag = 0;
        my_printf("NB IS DEAD!!!\n");
        NBCommCtrl.CSQ = 0XFF;     //失败
        NBCommCtrl.CSQGetFlag = 2; //获取到结果
        App_NB_PowerOff();
        NBCommCtrl.NBRunStatus = eNBRunStatusIdle; //清除工作状态
        /*清空队列*/
        while (1)
        {
            if (QUEUE_EMPTY == WIFI_UploadData_Get(txbuf, &len))
            {
                break;
            }
        }
    }
    switch (NBCommCtrl.NBRunStatus)
    {
    case eNBRunStatusIdle: //空闲
        break;
    case eNBRunStatusPowerUp: //上电
        App_NBRunStatusPowerUp();
        break;
    case eNBRunStatusStartingUp: //开机
        App_NBRunStatusStartingUp();
        break;
    case eNBRunStatusReset: //复位
        App_NBRunStatusReset();
        break;
    case eNBRunStatusWakeup: //唤醒
        App_NBRunStatusWakeup();
        break;
    case eNBRunStatusAutoGatt: //自动附着流程
        App_NBRunStatusCGATT();
        break;
    case eNBRunStatusReady: //开机后的准备工作
        App_NBRunStatusReady();
        break;
    case eNBRunStatusQueryState: //查询连接状态
        App_NBRunStatusQueryStat();
        break;
    case eNBRunStatusCGATT: //附着
        App_NBRunStatusCGATT();
        break;
    case eNBRunStatusSetupSocket: //创建Socket
        App_NBRunStatusSetupSocket();
        break;
    case eNBRunStatusCheckUE: //获取UE等数据
        App_NBRunStatusCheckUE();
        break;
    case eNBRunStatusDataSend: //数据发送
        App_NBRunStatusDataSend();
        break;
    case eNBRunStatusSetPSM: // PSM设置
        App_NBRunStatusSetPSM();
        break;
    case eNBRunStatusExit: //退出
        App_NBRunStatusExit();
        break;
    default:
        break;
    }

    if (eNBRunStatusIdle != NBCommCtrl.NBRunStatus)
    {
        if (0 == NBCommCtrl.runOverTime)
        {
            NBCommCtrl.deadFlag = 1;
        }
    }
}

/*********************************************************************************************************************
 * Function Name :  App_NB_ScanProcess()
 * Description   :  NB动作扫描进程
 * Para          :  无
 * Return        :  none
 *********************************************************************************************************************/
void App_NB_ScanProcess(void)
{
    App_NB_Dael();   // NB工作流程处理
	App_NB_ATDael(); // at指令处理
}
#endif