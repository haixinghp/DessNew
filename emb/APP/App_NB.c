/********************************************************************************************************************
 * @file:        App_NB.c
 * @author:      Haixing.Huang
 * @version:     V01.00
 * @date:        2022��3��29��
 * @Description: NBIOT�ӿڹ��ܺ����ļ�,����BC28��д
 * @ChangeList:  01. ����
 *********************************************************************************************************************/

/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
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
/*-------------------------------------------------�궨��-----------------------------------------------------------*/

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

#define NB_SERVICE_IP (106.15.134.146) // NB������IP
#define NB_SERVICE_PORT_UDP (8899)     // NB������UDP�˿�

#define RUN_TIME_MAX (3000) //�������ʱ��
/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/

/*-------------------------------------------------��������---------------------------------------------------------*/
typedef enum
{
    eNBAtStatusIdle,        //����
    eNBAtStatusSend,        //��������
    eNBAtStatusRecWait,     //���յȴ�
    eNBAtStatusRecOverTime, //���ճ�ʱ
    eNBAtStatusRecSuccee,   //���ճɹ�
    eNBAtStatusRecFail,     //����ʧ��
    eNBAtStatusRecError,    //���մ���
    eNBAtStatusRetry,       //����
} TYPEe_NBAtStatus;         // ATָ��״̬
typedef enum
{
    eNBRunStatusIdle,        //����
    eNBRunStatusPowerUp,     //�ϵ�
    eNBRunStatusStartingUp,  //����
    eNBRunStatusReset,       //��λ
    eNBRunStatusWakeup,      //����
    eNBRunStatusAutoGatt,    //�Զ���������
    eNBRunStatusReady,       //�������׼������
    eNBRunStatusQueryState,  //��ѯ����״̬
    eNBRunStatusCGATT,       //����
    eNBRunStatusSetupSocket, //����Socket
    eNBRunStatusCheckUE,     //��ȡUE������
    eNBRunStatusDataSend,    //���ݷ���
    eNBRunStatusSetPSM,      // PSM����
    eNBRunStatusExit,        //�˳�ϵͳ
} TYPEe_NBRunStatus;         // NB����״̬
typedef enum
{
    eNBRstSoft,                //����
    eNBRstSoftSucceed,         //������λ�ɹ�
    eNBRstHardWare,            //Ӳ��
    eNBRstHardWareWait,        //�ȴ����
    eNBRstHardWareStartupWait, //�ȴ��������
    eNBRstHardWareCheck,       //���ʱ��λ�ɹ�
    eNBRstHardWareSucceed,     //Ӳ����λ�ɹ�
    eNBRstFail,                //��λʧ��
} TYPEe_NBRst;                 //��λ״̬

typedef enum
{
    eNBCfunSet,        //����
    eNBCfunCheck,      //���
    eNBCfunWait,       //�ȴ�
    eNBCfunFunSucceed, //�ɹ�
    eNBCfunFunFail,    //ʧ��
} TYPEe_NBCfun;        //��λ״̬

typedef struct
{
    uint8_t pAtCmd[NB_TX_BUFF_SIZE]; //���͵�ָ��
    TYPEe_NBAtStatus status;         // ATָ���״̬
    uint32_t overTime;               //��ʱʱ��
    uint32_t overTimeCnt;            //��ʱ��ʱ��
    uint16_t rxCnt;                  //���ռ�����
    uint8_t rxBuff[NB_RX_BUFF_SIZE]; //���ջ�����
    TYPEe_NBRunStatus NBRunStatus;   // NB����״̬
    uint8_t NBRunStatusSecond;       // NB������״̬
    uint8_t tmpStep;                 // NB��ʱ����תծ
    uint8_t retry;                   //���Դ���
    TYPEe_NBRst NBRst;               //��λ״̬
    TYPEe_NBCfun NBCfun;             // CFUN����
    uint8_t connectFlag;             //���ӱ�־
    uint8_t socketNum;               // socket ���
    int16_t signalPower;             // Signal power
    int16_t txPower;                 // TX power
    uint16_t SNR;                    //�����
    uint16_t cellID;                 //С��
    uint8_t *updata;                 //�ϴ����ݵ�ָ��
    uint8_t updataLen;               //�ϴ����ݵĳ���
    uint8_t firstPowerUp;            //�״��ϵ�,0�״��ϵ磻1���״��ϵ�
    uint8_t deadFlag;                //ģ��������־
    uint32_t runOverTime;            //ģ����������ʱ��
    uint8_t CSQGetFlag;              //��ȡ�źű�־
    uint8_t CSQ;                     //�źŴ�Сȡ����ֵ��Χ��-113dBm~-53dBm.0XFF��ʾ��ȡ����0����ʾ��ȡ��
} TYPEs_NBCommCtrl;                  //ͨѶ������
TYPEs_NBCommCtrl NBCommCtrl = {
    0,
};
/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/

/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/

/*-------------------------------------------------ȫ��������-----------------------------------------------------*/

/*-------------------------------------------------��������---------------------------------------------------------*/

static void App_NB_TxMessage(uint8_t *p_data, uint16_t length);

/*-------------------------------------------------��������---------------------------------------------------------*/

/*********************************************************************************************************************
 * Function Name :  my_str_2_int()
 * Description   :  �ַ�ת��������
 * Para          :  char *cmd atָ�� ,int32_t overtime ��ʱʱ��
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
 * Description   :  NB ATָ���
 * Para          :  char *cmd atָ�� ,int32_t overtime ��ʱʱ��
 * Return        :  none
 *********************************************************************************************************************/
void App_NB_ATSend(char *cmd, int32_t overtime)
{
    uint32_t len = strlen(cmd); //��ȡ���͵ĳ���
    if (len > NB_TX_BUFF_SIZE)  //���ȹ���
    {
        my_printf("error: NB data too long \n");
        return;
    }
    if (eNBAtStatusIdle == NBCommCtrl.status)
    {
        NBCommCtrl.status = eNBAtStatusSend;                     //�µ�ָ��ֱ�ӷ���
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd)); //��������
        memcpy(NBCommCtrl.pAtCmd, cmd, len);                     //������Ҫ���͵�ָ��
        NBCommCtrl.overTime = overtime;
    }
    //����в�ͬ��ָ������¿�ʼ����
}

/*********************************************************************************************************************
 * Function Name :  App_NB_ATCmdClear()
 * Description   :  NB ATָ�����
 * Para          :  ��
 * Return        :  ��
 *********************************************************************************************************************/
void App_NB_ATCmdClear(void)
{
    NBCommCtrl.status = eNBAtStatusIdle;
}

/*********************************************************************************************************************
 * Function Name :  App_NB_PowerOn()
 * Description   :  NB�ϵ�
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_PowerOn(void)
{

    my_printf("NB_PowerOn\n");
#if defined NB_FUNCTION
    NB_POW_ON();

    //���ڳ�ʼ��
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
 * Description   :  NB�µ�
 * Para          :  ��
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
 * Description   :  NB���ڹر�
 * Para          :  ��
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
 * Description   :  NB��ʼ��
 * Para          :  ��
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
 * Description   :  NB��������,��������Ҫ���õ�uartͳһ����������HAL_Uart_SleepInit();
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_SleepInit(void)
{
    my_printf("App_NB_SleepInit\n");
    DRV_GpioInputPullnull(NB_RX_GPIO_PIN);
    DRV_GpioInputPullnull(NB_TX_GPIO_PIN); //����״̬����������
    NB_RST_OFF();
    NB_PSM_OFF();
}
/*********************************************************************************************************************
 * Function Name :  App_NB_SleepInitStart()
 * Description   :  ��ȡNB�ź�ǿ��
 * Para          :  ��
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
 * Description   :  ��ȡNB�ź�ǿ��
 * Para          :  ��
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
 * Description   :  uart ��������
 * Para   Input  :  p_data-����������ָ��   length- ���������ݳ���
 * Para   Output :  none
 * Return        :  void
 * ChangeList    :  ���� 2022-01-18 by gushengchi
 *********************************************************************************************************************/
void App_NB_TxMessage(uint8_t *p_data, uint16_t length)
{
#if defined NB_FUNCTION
    HAL_Uart_TxMessage(NB_UART_COM, p_data, length);
#endif
}
/*********************************************************************************************************************
 * Function Name :  App_NB_Reset()
 * Description   :  NB��λ  ������λ������AT+NRB,ģ��ظ�REBOOTING��ģ��������ɻᷢ�͡�OK��
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
TYPEe_NBRst App_NB_Reset(void)
{
    switch (NBCommCtrl.NBRst)
    {
    case eNBRstSoft:                       //����
        App_NB_ATSend("AT+NRB\r\n", 1000); //����������λָ��
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //ģ����������Ӧ
            App_NB_ATCmdClear();
            NBCommCtrl.NBRst = eNBRstSoftSucceed;
        }
        else if ((eNBAtStatusRecOverTime == NBCommCtrl.status) ||
                 (eNBAtStatusRecFail == NBCommCtrl.status) ||
                 (eNBAtStatusRecError == NBCommCtrl.status))
        {
            App_NB_ATCmdClear();
            my_printf("ERROR: Reset no ack\n");
            NBCommCtrl.NBRst = eNBRstHardWare; //����ʹ��Ӳ����λ
        }
        break;
    case eNBRstSoftSucceed: //������λ�ɹ�
        break;
    case eNBRstHardWare: //Ӳ��
        NB_RST_ON();
        NBCommCtrl.overTimeCnt = 20; //��ʱ��Ҫ�ȴ�
        NBCommCtrl.NBRst = eNBRstHardWareWait;
        break;
    case eNBRstHardWareWait: //�ȴ����
        if (0 == NBCommCtrl.overTimeCnt)
        {
            NB_RST_OFF();
            NBCommCtrl.overTimeCnt = 420; //��ʱ��Ҫ�ȴ�
            NBCommCtrl.NBRst = eNBRstHardWareStartupWait;
        }

        break;
    case eNBRstHardWareStartupWait: //�ȴ��������
        if (0 == NBCommCtrl.overTimeCnt)
        {
            memset(NBCommCtrl.rxBuff, 0, sizeof(NBCommCtrl.rxBuff)); //��ս��ջ���
            NBCommCtrl.NBRst = eNBRstHardWareCheck;
        }
        break;
    case eNBRstHardWareCheck:         //���ʱ��λ�ɹ�
        App_NB_ATSend("AT\r\n", 500); //����������λָ��
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //ģ����������Ӧ
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
            NBCommCtrl.NBRst = eNBRstFail; //Ӳ����λʧ��
        }
        break;
    case eNBRstHardWareSucceed: //Ӳ����λ�ɹ�
        break;
    case eNBRstFail: //��λʧ��
    default:
        break;
    }
    return NBCommCtrl.NBRst;
}

/*********************************************************************************************************************
* Function Name :  App_NB_FunSet()
* Description   :  NB CFUN����
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
            App_NB_ATSend("AT+CFUN=0\r\n", 2000); //����������λָ��
        }
        else
        {
            App_NB_ATSend("AT+CFUN=1\r\n", 2000);
        }

        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //ģ����������Ӧ
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
                NBCommCtrl.overTimeCnt = 50; //��ʱ��Ҫ�ȴ�
                NBCommCtrl.NBCfun = eNBCfunWait;
            }
            else
            {
                NBCommCtrl.NBCfun = eNBCfunFunFail; //ʧ��
            }
        }
        break;
    case eNBCfunCheck:
        App_NB_ATSend("AT+CFUN?\r\n", 2000); //
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            //ģ����������Ӧ
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
                    NBCommCtrl.overTimeCnt = 50; //��ʱ��Ҫ�ȴ�
                    NBCommCtrl.NBCfun = eNBCfunWait;
                }
                else
                {
                    NBCommCtrl.NBCfun = eNBCfunFunFail; //ʧ��
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
                NBCommCtrl.overTimeCnt = 50; //��ʱ��Ҫ�ȴ�
                NBCommCtrl.NBCfun = eNBCfunWait;
            }
            else
            {
                NBCommCtrl.NBCfun = eNBCfunFunFail; //ʧ��
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
 * Description   :  NB ��ʱ������
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
 * Description   :  NB ATָ���
 * Para          :  ��
 * Return        :  TYPEe_NBAtStatus ATָ���״̬
 *********************************************************************************************************************/
TYPEe_NBAtStatus App_NB_ATDael(void)
{
#if defined NB_FUNCTION
    char *strx = NULL;
    uint8_t tmp;
    if (E_NB_UART != HAL_Uart_GetCurDeviceType(NB_UART_COM)) //��������
    {
        return eNBAtStatusIdle;
    }
    switch (NBCommCtrl.status)
    {
    case eNBAtStatusIdle: //����
        break;
    case eNBAtStatusSend:
        memset(NBCommCtrl.rxBuff, 0, sizeof(NBCommCtrl.rxBuff));                      //��ս��ջ���
        App_NB_TxMessage(NBCommCtrl.pAtCmd, strlen((const char *)NBCommCtrl.pAtCmd)); //��������
        my_printf("NB Send:%s", NBCommCtrl.pAtCmd);                                   //��ӡ��־
        NBCommCtrl.overTimeCnt = NBCommCtrl.overTime;                                 //���ӳ�ʱʱ��
        NBCommCtrl.status = eNBAtStatusRecWait;
        NBCommCtrl.rxCnt = 0;
        break;
    case eNBAtStatusRecWait:                                                 //���յȴ�
        while (UART_SUCCESS == HAL_Uart_PopByteFromQueue(NB_UART_COM, &tmp)) //�ӽ��ջ�������ȡ����
        {
            NBCommCtrl.rxBuff[NBCommCtrl.rxCnt] = tmp;
            NBCommCtrl.rxCnt++;
            if (NBCommCtrl.rxCnt > NB_RX_BUFF_SIZE) //��ֹ���
            {
                NBCommCtrl.rxCnt = NB_RX_BUFF_SIZE - 1;
            }
        }
        strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"OK");
        if (NULL != strx) //����OK
        {
            my_printf("NB rec ok:%s\n", NBCommCtrl.rxBuff); //��ӡ��־
            NBCommCtrl.status = eNBAtStatusRecSuccee;       //���ճɹ�
        }
        strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"ERROR");
        if (NULL != strx) //����ERROR
        {
            my_printf("NB rec error:%s\n", NBCommCtrl.rxBuff); //��ӡ��־
            NBCommCtrl.status = eNBAtStatusRecError;           //���մ���
        }
        if (0 == NBCommCtrl.overTimeCnt) //��ʱ�������
        {
            NBCommCtrl.status = eNBAtStatusRecOverTime; //���ճ�ʱ
        }
        break;
    case eNBAtStatusRecOverTime: //���ճ�ʱ

        break;
    case eNBAtStatusRecSuccee: //���ճɹ�
        /*�������ջ�ȡһЩоƬ�������͵�����*/
        while (UART_SUCCESS == HAL_Uart_PopByteFromQueue(NB_UART_COM, &tmp)) //�ӽ��ջ�������ȡ����
        {
            NBCommCtrl.rxBuff[NBCommCtrl.rxCnt] = tmp;
            NBCommCtrl.rxCnt++;
            if (NBCommCtrl.rxCnt > NB_RX_BUFF_SIZE) //��ֹ���
            {
                NBCommCtrl.rxCnt = NB_RX_BUFF_SIZE - 1;
            }
        }
        break;
    case eNBAtStatusRecFail: //����ʧ��

        break;
    case eNBAtStatusRecError: //���մ���

        break;
    case eNBAtStatusRetry: //����

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
 * Description   :  NB ��ѯ�Ƿ���æµ״̬
 * Para          :  ��
 * Return        :  1,���У�-1 æµ
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
 * Description   :  NB ���� ����Ѿ�����������Ҫ�ٴ�����
 * Para          :  ��
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
 * Description   :  ģ������ �ȴ�����30��
 * Para          :  ��
 * Return        :  ������ 0�������ɹ� ��1�� ����ʧ�� �� 2��
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
* Description   :  case����
* Para          :  ��
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusPowerUp(void)
{
    App_NB_PowerOn();                      //��ģ���ϵ�
    NBCommCtrl.runOverTime = RUN_TIME_MAX; //��д��������ʱ��
    if (0 == NBCommCtrl.connectFlag)
    {
        /*δ����*/
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
* Description   :  case����
* Para          :  ��
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusStartingUp(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        NBCommCtrl.overTimeCnt = 500; //�ȴ�NB������ɣ�4������
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 1:
        if (0 == NBCommCtrl.overTimeCnt) //�������
        {
            NBCommCtrl.NBRunStatusSecond++;
            NBCommCtrl.retry = 2;
        }
        break;
    case 2:
        if (NBCommCtrl.retry > 0)
        {
            App_NB_ATSend("AT\r\n", 50); //����AT��ģ��ظ�OKΪ����
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                App_NB_ATCmdClear();
#if 0
				if(0 == NBCommCtrl.connectFlag) 
				{
					/*δ����*/
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
        NBCommCtrl.tmpStep = 0; //��ʱ����
        if (0 == NBCommCtrl.firstPowerUp)
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        else
        {
            NBCommCtrl.NBRunStatusSecond = 9; //ֱ�ӽ�������CFUN
        }
        break;
    case 4:
        if (0 == NBCommCtrl.tmpStep)
        {
            App_NB_ATSend("AT+QREGSWT=2\r\n", 2000); //�ر�BC28������1h��ע�����IOTƽ̨
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
                NBCommCtrl.overTimeCnt = 50; //�ȴ�һ���ٷ�ָ��
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
        NBCommCtrl.tmpStep = 0; //��ʱ����
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 6:
        App_NB_ATSend("AT+CPSMS=1,,,\"01011111\",\"00000001\"\r\n", 100); //��������,�����������߿��Ա���
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
        NBCommCtrl.tmpStep = 0;      //��ʱ����
        NBCommCtrl.overTimeCnt = 10; //��΢�ӳ�
        NBCommCtrl.NBRunStatusSecond++;
        break;
    case 8:
        if (0 == NBCommCtrl.tmpStep)
        {
            if (0 == NBCommCtrl.overTimeCnt)
            {
                NBCommCtrl.NBRst = eNBRstSoft; //����������λ�ķ�ʽ
                NBCommCtrl.tmpStep++;
            }
        }
        else if (1 == NBCommCtrl.tmpStep)
        {
            App_NB_Reset(); //��λ
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
        NBCommCtrl.tmpStep = 0; //��ʱ����
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
        NBCommCtrl.retry = 10; // cfun��������
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
            NBCommCtrl.overTimeCnt = 100; //��Ҫ�ӳ�һ��ʱ��
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
            if (NULL != strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CSQ:99,99")) //���ź�
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
* Description   :  case����
* Para          :  ��
* Return        :  void


*********************************************************************************************************************/
void App_NBRunStatusReset(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        //�ж��Ƿ����ģ����Ӧ��״̬�еĻ�ֱ���˳�
        NBCommCtrl.NBRunStatusSecond++;
        NBCommCtrl.NBRst = eNBRstSoft; //����������λ�ķ�ʽ
        break;
    case 1:
        App_NB_Reset(); //��λ
        if ((eNBRstSoftSucceed == NBCommCtrl.NBRst) ||
            (eNBRstHardWareSucceed == NBCommCtrl.NBRst))
        {
            NBCommCtrl.NBRunStatusSecond++;
        }
        if (eNBRstFail == NBCommCtrl.NBRst) //��λʧ�ܣ�ģ��ֹͣ������
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
* Description   :  case����
* Para          :  ��
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
 * Description   :  case����
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusReady(void)
{
    static uint8_t step = 0; //��ʱ����
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
                App_NB_ATSend("AT+QLEDMODE=0\r\n", 50); //�ر�ָʾ�ƹ���
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
                App_NB_ATSend("AT+NPSMR=1\r\n", 50); //ʡ��ģʽ״̬�ϱ�
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
                App_NB_ATSend("AT+NCCID\r\n", 50); //ʶ�� USIM ��
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
 * Description   :  case����
 * Para          :  ��
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
        //�����͹ػ�
        NBCommCtrl.NBRunStatusSecond++;
        NBCommCtrl.tmpStep = 0;
        NBCommCtrl.retry = 3;
        break;
    case 2:
        if (0 == NBCommCtrl.tmpStep)
        {
            App_NB_ATSend("AT+CEREG?\r\n", 50); // EPS ����ע��״̬
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CEREG:");
                if (NULL == strx)
                {
                    my_printf("ERROR:AT+CEREG? no ack\n");
                }
                else
                {
                    if (('1' == *(strx + 9)) || ('5' == *(strx + 9))) //��ע�ᣬ�������磻��ע�ᣬ��������
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
                            NBCommCtrl.overTimeCnt = 500; //�ӳ�һ�£��ȴ�����ע��ɹ�
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
 * Description   :  case����
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusCGATT(void)
{
    char *strx = NULL;
    App_NB_ATSend("AT+CGATT?\r\n", 50); // EPS ����ע��״̬
    if (eNBAtStatusRecSuccee == NBCommCtrl.status)
    {
        strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CGATT:");
        if (NULL == strx)
        {
            my_printf("ERROR:AT+CGATT?? no ack\n");
        }
        else
        {
            if ('1' == *(strx + 7)) //���ųɹ�
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
 * Description   :  case����
 * Para          :  ��
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

        App_NB_ATSend("AT+NSOCR=DGRAM,17,0,1\r\n", 200); //���� Socket
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
        App_NB_ATSend("AT+NSOCL=1\r\n", 200); //�ر� Socket
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
 * Description   :  case����
 * Para          :  ��
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
            App_NB_ATSend("AT+NUESTATS\r\n", 200); //��ѯ UE ͳ����Ϣ
            if (eNBAtStatusRecSuccee == NBCommCtrl.status)
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"Signal power:"); //����OK
                if (strx != NULL)
                {
                    NBCommCtrl.signalPower = my_str_2_int(strx, ':');
                    strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"TX power:"); //����OK
                    NBCommCtrl.txPower = my_str_2_int(strx, ':');
                    strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"SNR:"); //����OK
                    NBCommCtrl.SNR = my_str_2_int(strx, ':');
                    strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"Cell ID:"); //����OK
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
            //            strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"Cell ID:"); //����OK
            //            if (NULL != strx)
            //            {
            //            }
            App_NB_ATCmdClear();
            NBCommCtrl.NBRunStatusSecond = 0;
            if (1 == NBCommCtrl.CSQGetFlag)
            {
                strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CSQ:"); //����OK
                if (NULL != strx)
                {
                    memcpy(CSQBuff, strx + (sizeof("+CSQ:") - 1), 2);
                    my_printf("Get CSQ IS:%s\n", CSQBuff);
                    if (('9' == CSQBuff[0]) && ('9' == CSQBuff[1])) //δ֪
                    {
                        NBCommCtrl.CSQ = 0XFF; //ʧ��
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
                        NBCommCtrl.CSQ = 0XFF; //ʧ��
                    }
                    my_printf("Get CSQ2 IS:%d\n", NBCommCtrl.CSQ);
                    NBCommCtrl.CSQGetFlag = 2;                 //��ȡ�����
                    NBCommCtrl.NBRunStatus = eNBRunStatusExit; //�������ݷ���
                }
            }
            else
            {
                NBCommCtrl.NBRunStatus = eNBRunStatusDataSend; //�������ݷ���
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
 * Description   :  case����
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusDataSend(void)
{
#define SEND_OVER_TIME (400)
    uint8_t txbuf[WIFI_UPLOAD_DATA_LENGTH_MAX] = {0}; //���ͻ���
    uint16_t len;                                     //���ͳ���
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
        //            /*�״��ϵ��˳�*/
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
        NBCommCtrl.status = eNBAtStatusSend;                                         //�µ�ָ��ֱ�ӷ���
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd));                     //��������
        memcpy(NBCommCtrl.pAtCmd, "AT+NSORF=1,80\r\n", sizeof("AT+NSORF=1,80\r\n")); //��ȡ���ݣ�������ջظ�
        NBCommCtrl.pAtCmd[9] = NBCommCtrl.socketNum;                                 //��дsocket��
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
        NBCommCtrl.status = eNBAtStatusSend;                     //�µ�ָ��ֱ�ӷ���
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd)); //��������
        memcpy(NBCommCtrl.pAtCmd,
               "AT+NSOSTF=1,106.15.134.146,8899,0x400,",
               sizeof("AT+NSOSTF=1,106.15.134.146,8899,0x400,")); //�²��Է� UDP  �ȴ����յ���������������
	
		if (WIFI_UploadData_Get(txbuf, &len) == QUEUE_OK) //��ȡ���͵����ݺͳ���
		{
			NBCommCtrl.updata = txbuf;
			NBCommCtrl.updataLen = len;
		}
		
		if(0 == NBCommCtrl.updataLen)
		{
			/*û����Ҫ���͵�����*/
			if (0 == NBCommCtrl.firstPowerUp) 
			{
				/*��һ���ϵ磬��дһ������0���ӿ칦���½�*/
				len = 1;
				NBCommCtrl.updata = txbuf;
				NBCommCtrl.updataLen = len;
			}
			else
			{
				/*�ǵ�һ���ϵ磬ֱ�ӽ���*/
				App_NB_ATCmdClear();
				NBCommCtrl.NBRunStatusSecond = 5;
			}
			
		}
		else
		{
			NBCommCtrl.pAtCmd[10] = NBCommCtrl.socketNum;  //��дsocket��
			sprintf(dataBuff, "%d", NBCommCtrl.updataLen); //���������ݳ���ת���ַ���

			strcat((char *)NBCommCtrl.pAtCmd, dataBuff);  //׷�ӳ���
			strcat((char *)NBCommCtrl.pAtCmd, ",\0\0");   //׷��","
			strIndex = strlen((char *)NBCommCtrl.pAtCmd); //��ȡ����
			for (i = 0; i < NBCommCtrl.updataLen; i++)    //����������
			{
				sprintf(((char *)NBCommCtrl.pAtCmd + strIndex) + i * 2, "%02X", NBCommCtrl.updata[i]);
			}
			strcat((char *)NBCommCtrl.pAtCmd, "\r\n"); //׷�ӻس�
			NBCommCtrl.overTime = SEND_OVER_TIME;
			NBCommCtrl.retry = 3;
			NBCommCtrl.NBRunStatusSecond++;
		}
		
        break;
    case 4:
        if (eNBAtStatusRecSuccee == NBCommCtrl.status)
        {
            strx = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+NSONMI");   //����SEND OK
            strx1 = strstr((const char *)NBCommCtrl.rxBuff, (const char *)"+CSCON:0"); //���ؽ���IDLEģʽ
            if ((NULL == strx) && (NULL == strx1))                                     //Ҫ���ڹ涨ʱ�����յ�ָ���ַ�
            {
                if (0 == NBCommCtrl.overTimeCnt)
                {
                    if (NBCommCtrl.retry > 0)
                    {
                        /*�ط�*/
                        NBCommCtrl.retry--;
                        NBCommCtrl.overTime = SEND_OVER_TIME;
                        NBCommCtrl.status = eNBAtStatusSend; //�µ�ָ��ֱ�ӷ���
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
                if (0 != WIFI_UploadDataGetLength()) //���ʣ�������
                {
                    NBCommCtrl.NBRunStatusSecond = 3; //�ط�
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
                NBCommCtrl.NBRunStatusSecond = 3; //�ط�
            }
            else
            {
                NBCommCtrl.NBRunStatusSecond++;
            }
        }
        break;

    case 5:
        NBCommCtrl.status = eNBAtStatusSend;                                         //�µ�ָ��ֱ�ӷ���
        memset(NBCommCtrl.pAtCmd, 0, sizeof(NBCommCtrl.pAtCmd));                     //��������
        memcpy(NBCommCtrl.pAtCmd, "AT+NSORF=1,80\r\n", sizeof("AT+NSORF=1,80\r\n")); //��ȡ���ݣ�������ջظ�
        NBCommCtrl.pAtCmd[9] = NBCommCtrl.socketNum;                                 //��дsocket��
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
        NBCommCtrl.NBRunStatus = eNBRunStatusExit; //�˳�����
        break;
    default:
        break;
    }
#undef SEND_OVER_TIME
}

/*********************************************************************************************************************
 * Function Name :  App_NBRunStatusSetPSM()
 * Description   :  case����
 * Para          :  ��
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
 * Description   :  case����
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NBRunStatusExit(void)
{
    switch (NBCommCtrl.NBRunStatusSecond)
    {
    case 0:
        if (0 == NBCommCtrl.firstPowerUp)
        {
            NBCommCtrl.firstPowerUp = 2; //Ϊ��������
        }
        if (0 == NBCommCtrl.connectFlag)
        {
            /*�ػ�ģ�飬��ʡ����*/
            NBCommCtrl.NBCfun = eNBCfunSet;
            NBCommCtrl.retry = 10;
            NBCommCtrl.NBRunStatusSecond++;
        }
        else
        {
            App_NB_CloseUart(); //�رմ���
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
 * Description   :  NB ����
 * Para          :  ��
 * Return        :  void
 *********************************************************************************************************************/
void App_NB_Dael(void)
{
    uint8_t txbuf[WIFI_UPLOAD_DATA_LENGTH_MAX] = {0}; //���ͻ���
    uint16_t len;
	/*����*/
    if (1 == App_NB_CheckBusy()) 
    {
        return;
    }
	/*���ڱ�ռ��*/
    if (((HAL_Uart_GetCurDeviceType(NB_UART_COM) == E_FACE_UART) ||
         (HAL_Uart_GetCurDeviceType(NB_UART_COM) == E_FINGER_UART) ||
         (HAL_Uart_GetCurDeviceType(NB_UART_COM) == E_CAMERA_UART)) 
		)
//        && (NBCommCtrl.NBRunStatus < eNBRunStatusDataSend))         //�ϵ�δ����ʱ���ڱ������ط�
    {
        NBCommCtrl.NBRunStatus = eNBRunStatusPowerUp;
        return;
    }

    if (1 == NBCommCtrl.deadFlag) //���Ӳ���������������
    {
        NBCommCtrl.deadFlag = 0;
        my_printf("NB IS DEAD!!!\n");
        NBCommCtrl.CSQ = 0XFF;     //ʧ��
        NBCommCtrl.CSQGetFlag = 2; //��ȡ�����
        App_NB_PowerOff();
        NBCommCtrl.NBRunStatus = eNBRunStatusIdle; //�������״̬
        /*��ն���*/
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
    case eNBRunStatusIdle: //����
        break;
    case eNBRunStatusPowerUp: //�ϵ�
        App_NBRunStatusPowerUp();
        break;
    case eNBRunStatusStartingUp: //����
        App_NBRunStatusStartingUp();
        break;
    case eNBRunStatusReset: //��λ
        App_NBRunStatusReset();
        break;
    case eNBRunStatusWakeup: //����
        App_NBRunStatusWakeup();
        break;
    case eNBRunStatusAutoGatt: //�Զ���������
        App_NBRunStatusCGATT();
        break;
    case eNBRunStatusReady: //�������׼������
        App_NBRunStatusReady();
        break;
    case eNBRunStatusQueryState: //��ѯ����״̬
        App_NBRunStatusQueryStat();
        break;
    case eNBRunStatusCGATT: //����
        App_NBRunStatusCGATT();
        break;
    case eNBRunStatusSetupSocket: //����Socket
        App_NBRunStatusSetupSocket();
        break;
    case eNBRunStatusCheckUE: //��ȡUE������
        App_NBRunStatusCheckUE();
        break;
    case eNBRunStatusDataSend: //���ݷ���
        App_NBRunStatusDataSend();
        break;
    case eNBRunStatusSetPSM: // PSM����
        App_NBRunStatusSetPSM();
        break;
    case eNBRunStatusExit: //�˳�
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
 * Description   :  NB����ɨ�����
 * Para          :  ��
 * Return        :  none
 *********************************************************************************************************************/
void App_NB_ScanProcess(void)
{
    App_NB_Dael();   // NB�������̴���
	App_NB_ATDael(); // atָ���
}
#endif