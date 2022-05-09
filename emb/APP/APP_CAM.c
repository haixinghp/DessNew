/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: APP_CAM.c 
* 作者：邓业豪
* 版本：V01
* 时间：20210804
* 内容简述：单猫眼应用文件
****************************************************************************/

/* 标准头文件 */
#include "stdint.h"
#include "string.h"

/* 内部头文件 */
#include "..\DRV\DRV_EXPORT\DRV_74HC4052.h"
#include "..\HAL\HAL_EXPORT\HAL_EXPORT.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#include "..\HAL\HAL_VOICE\HAL_Voice.h"
#include "..\HAL\HAL_UART\HAL_UART.h"
#include "LockConfig.h"
#include "Public.h"
#include "APP_CAM.h"
#include "App_WIFI.h" 
#ifdef  XM_CAM_FUNCTION_ON
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
//硬件
#define CAM_RX_PIN       M_UART_RX_GPIO_PIN     //RX引脚
#define CAM_TX_PIN       M_UART_TX_GPIO_PIN     //TX引脚
#define CAM_IO_INIT      HAL_EXPORT_PinInit(EM_CAMERA_IRQ,DIR_OUT,POLO_RETTAIN)
#define CAM_IO_WAKEUP    HAL_EXPORT_PinSet( EM_CAMERA_IRQ, OUT_LOW )
#define CAM_IO_RELEASE   HAL_EXPORT_PinSet( EM_CAMERA_IRQ, OUT_HIGH )
#define CAM_BAUDRATE     UART_BAUD_RATE_115200   //波特率
#define CAM_UTRT_SWITCH_ON       DRV_74HC0452_UartSelect(HW_74HC4052_UART_CAMERA)    //模拟开关切换 
#define CAM_UTRT_SWITCH_OFF      DRV_74HC0452_UartSelect(HW_74HC4052_UART_OFF)


//协议格式
#define  CAM_PID_HEAD         0
#define  CAM_PID_LEN          1
#define  CAM_PID_CDM          2
#define  CAM_PID_DATA         3


/*-------------------------------------------------结构体-----------------------------------------------------------*/
CamQueue_T astCamCtrl = {0, 0, {0}};


static uint8_t CaptureType=0;

static bool s_bSendCmdFlag = false;

/*********************************************************************************************************************
* Function Name :  cam_clear_queue
* Description   :  清消息队列
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void cam_clear_queue(void)
{
    my_printf("cam_clear_queue\n");
    memset((void*)&astCamCtrl, 0, sizeof(astCamCtrl));
    return;
} 

/*********************************************************************************************************************
* Function Name :  cam_get_queue_len
* Description   :  获取队列长度
* Para          :  无
* Return        :  DataLength
*********************************************************************************************************************/
static uint8_t cam_get_queue_len(void)
{
    uint8_t DataLength;
    if((astCamCtrl.HeadPtr >= CAM_QUEUE_SIZE)\
        || (astCamCtrl.TailPtr >= CAM_QUEUE_SIZE))//超过数组长度则清零
    {
        cam_clear_queue();
    }
    if(astCamCtrl.TailPtr >= astCamCtrl.HeadPtr)//如果尾指针大于头指针 则长度正常取 (尾位置 - 头位置)
    {
        DataLength = astCamCtrl.TailPtr - astCamCtrl.HeadPtr;
    }
    else //否则若头位置大于尾位置 则表明数据二次写入 头指针靠近消息队列出口 尾指针靠近消息队列入口 长度取(尾位置 + 数组深度 - 头位置)
    {
        DataLength = astCamCtrl.TailPtr + CAM_QUEUE_SIZE - astCamCtrl.HeadPtr;
    }
    return DataLength;
}

/*********************************************************************************************************************
* Function Name :  cam_reset_queue_data
* Description   :  完成队列的读取，head向后移动，使用过的队列数据清空
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void cam_reset_queue_data(void)
{
    my_printf("cam_reset_queue_data\n");

    //memset((void*)&astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf, 0, CAM_QUEUE_BUF_MAX);
    memset((void*)&astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxDataBuf, 0, CAM_QUEUE_BUF_MAX);
    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Txlen = 0;
    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxCdm = 0;
    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos = 0;
    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick = 0;

    if(astCamCtrl.TailPtr != astCamCtrl.HeadPtr)
    {
        if(astCamCtrl.HeadPtr == (CAM_QUEUE_SIZE-1))
        {
            astCamCtrl.HeadPtr = 0;
        }
        else
        {
            astCamCtrl.HeadPtr++;
        }
    }

    return;
}


/***************************************************************************************
**函数名:       App_CAM_Tim10Ms
**功能描述:     计时处理，用于应答包时间控制
**输入参数:     
**输出参数:    
**备注:         注意需要放在10ms定时器中断中
****************************************************************************************/
void App_CAM_Tim10Ms (void)
{
    if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick)
    {
        astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick--;
    }
}

/***************************************************************************************
**函数名:       xor_inverted_check
**功能描述:     数据校验
**输入参数:     *inBuf输入数组指针    inLen长度
**输出参数:     check  校验结果
**备注:        
****************************************************************************************/
static uint8_t xor_inverted_check(const uint8_t *inBuf, uint8_t inLen)
{
    uint8_t check = 0,i;
    for(i = 0; i < inLen; i++)
    {
        check ^= inBuf[i];
    }
    check=~check;
    return check;
}


/***************************************************************************************
**函数名:       CAM_SendCommand
**功能描述:     串口开启、切换模拟开关
**输入参数:     
**输出参数:    
**备注:        
****************************************************************************************/
static void CAM_SendCommand (uint8_t cmd , uint8_t *data , uint8_t len)
{
    uint8_t SendData[256];
    
    SendData[CAM_PID_HEAD]=CAM_CMD_HEAD;  //包头
    SendData[CAM_PID_LEN]= len+4;  //数据长度+4（头+长度+命令+校验）
    SendData[CAM_PID_CDM]=cmd;
    memcpy(&SendData[CAM_PID_DATA],data,len); //拷贝数据
    
    //计算末尾字节校验
    SendData[SendData[CAM_PID_LEN]-1]=xor_inverted_check((const uint8_t *)SendData,SendData[CAM_PID_LEN]-1);
    
    //串口发送
    HAL_Uart_TxMessage(CAMERA_UART_COM,SendData,SendData[CAM_PID_LEN]); //串口发送
    PUBLIC_PrintHex("CAM_UartTx",SendData,SendData[CAM_PID_LEN]);
}




/***************************************************************************************
**函数名:       CAM_UartWakeUp
**功能描述:     串口开启、切换模拟开关
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
static void CAM_UartWakeUp (void)
{
    my_printf("CAM_UartWakeUp();\n");
    //串口模拟开关切换
    CAM_UTRT_SWITCH_ON;

    //串口初始化
    UartCfg_S uartCfg={0};
    uartCfg.BaudRate = CAM_BAUDRATE;
    uartCfg.DataBit = DATA_8_BIT;
    uartCfg.StopBit = STOP_1_BIT;
    uartCfg.ParityType = PARITY_NONE;
    uartCfg.RxInerruptEn = INT_ENABLE;
    uartCfg.TxInerruptEn = INT_DISENABLE;
    HAL_Uart_ConfigInit( E_CAMERA_UART, uartCfg );
    
    //拉唤醒脚
    CAM_IO_INIT;
    CAM_IO_WAKEUP;
    
    //发送指令查询模组是否唤醒状态
    uint8_t reserve=0;//保留码
    CAM_SendCommand(CAM_CMD_SN_GET,&reserve,1); //发送查询序列号，数据仅保留码0

    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick=500; //唤醒5秒超时
}  


/***************************************************************************************
**函数名:       CAM_UartRelease
**功能描述:     串口释放、切换模拟开关
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
void CAM_UartRelease (void)
{
    // 使用过的队列释放
    cam_reset_queue_data();
    
    //关串口
    HAL_Uart_DeInit( E_CAMERA_UART );
    
    //拉唤醒脚
    CAM_IO_RELEASE;
    
    //串口模拟开关切换
    CAM_UTRT_SWITCH_OFF;
}
/***************************************************************************************
**函数名:       CAM_ReadFifoData
**功能描述:     串口数据解码
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
static uint8_t CAM_ReadFifoData (void)
{
    if(HAL_Uart_GetCurDeviceType( CAMERA_UART_COM ) !=E_CAMERA_UART) //检查串口状态
    {
        return 0;
    }
    uint8_t pdata=0;
    do  //取数据直到队列为空 或者 取到完整一包
    {
        if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos==0) //无数据
        {
            if(UART_SUCCESS == HAL_Uart_PopByteFromQueue( CAMERA_UART_COM, &pdata )) //串口队列取数据
            {
                if(CAM_CMD_HEAD==pdata) //包头
                {
                    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos++]=pdata; //开始接受
                }                
            }
            else
            {
                return 0; //无数据
            }
        }
        else
        {
            //一包接完
            if((astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos>CAM_PID_DATA) && astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[CAM_PID_LEN]== astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos)
            {
                PUBLIC_PrintHex("CAM_RX_DATA",astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf,astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos);
                astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos=0;
                return 1;
            }
            else
            {
                if(UART_SUCCESS==HAL_Uart_PopByteFromQueue( CAMERA_UART_COM, &pdata )) //队列取数据
                {
                    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxPos++]=pdata;//继续接收
                }
                else
                {
                    return 0; //无数据
                }
            }
        }
    }while(1);
}  

/***************************************************************************************
**函数名:       CAM_GetServerState
**功能描述:     猫眼任务状态
**输入参数:     uint8_t _i8TaskId   CAM_SendCommandStart操作后返回的非错误的正值索引
**输出参数:     DrvIrControl.State 当前状态
**备注:         
****************************************************************************************/
CAM_STATE_E CAM_GetServerState(uint8_t _i8TaskId)
{
    return astCamCtrl.astCamCtrl[_i8TaskId].State;
}

/***************************************************************************************
**函数名:       CAM_ServerScan
**功能描述:     猫眼任务状态
**输入参数:     void
**输出参数:     void
**备注:         
****************************************************************************************/
void CAM_ServerScan(void)
{
    uint8_t CdmData=0;//临时数据包
    if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxCdm==0)
    {
        return;
    }
    
    if((HAL_Uart_GetCurDeviceType( CAMERA_UART_COM ) != E_CAMERA_UART)
        &&(HAL_Uart_GetCurDeviceType( CAMERA_UART_COM ) != E_NO_UART))
    {
        return;
    }

    switch (astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State) 
    {
        case CAM_WAKE_UP: 
            my_printf("CAM - taskid-%d  \n", astCamCtrl.HeadPtr, astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick);
            CAM_UartWakeUp();
            astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_READY;
            break;
        case CAM_READY:
            if(CAM_ReadFifoData()) //收到ready或者应答
            {
                if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[CAM_PID_CDM]==CAM_CMD_SN_GET)
                {
                    my_printf("Cam_is_working   ! \n"); //模组未休眠在工作状态
                    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_COMMAND; //立即发送
                }
                else if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[CAM_PID_CDM]==CAM_CMD_READY)
                {
                    my_printf("Cam_is_Ready  ! \n"); //回复就绪
                    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_ACK_READY;
                    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick = 3; //20毫秒后在发送，防止模组未完全启动
                }
                    
            }
            else if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick==0) //超时无响应
            {
                 astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_FAIL;
                 CAM_UartRelease();
                 my_printf("Cam_no_Reply  ! \n");
            }
            break;    
        case CAM_ACK_READY    : //回复模组就绪指令
            if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick==0)
            {
                CdmData=0x1b; //回复猫眼就绪
                CAM_SendCommand(CAM_CMD_READY_ACK,&CdmData,1);
                astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_COMMAND;
            }
            break;    
        case CAM_COMMAND://发送命令
            CAM_SendCommand(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxCdm, &astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxDataBuf[0], astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Txlen);
            astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_RESPONSE;
            astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick = 500; //1秒超时
            break;    
        case CAM_RESPONSE://等待应答
            if(CAM_ReadFifoData()) //收到ready或者应答
            {
                uint8_t RxCmd = astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[CAM_PID_CDM];
                uint8_t actionCmd = astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxCdm;
                my_printf("CAM_RESPONSE - RxCmd = 0x%x actionCmd = 0x%x \n", RxCmd, actionCmd);
                /* 确认命令字,发送和接收是对应关系
                                特殊情况1: PIR测试，发送休眠后，模组回复0x80
                                */
                if(RxCmd == actionCmd
                    || (actionCmd == CAM_CMD_FAST_SLEEP_SET && RxCmd == CAM_CMD_NET_SLEEP_STATE))
                {
                    my_printf("Cam_Has_Response  ! State is CAM_SUCCESSFUL\n");
                    astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_SUCCESSFUL;
                    CAM_UartRelease();
                }
            }
            else if(astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].Systick==0) //超时无响应
            {
                 my_printf("Cam_No_Response  ! State is CAM_FAIL\n");
                 astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].State = CAM_FAIL;
                 CAM_UartRelease();
            }
            break;    
        default:
            break;
    }    
    
    return;
}


/***************************************************************************************
**函数名:       CAM_SendCommandStart
**功能描述:     猫眼配网
**输入参数:     *pdata 数据指针   len 数据长度
**输出参数:     int8_t 返回队列id，返回-1 为队列满, -2为当前正忙
**备注:        // 清空、睡眠、PIR开关、获取序列号、WIFI透传、抓拍、门铃、阿里云密钥下发、SSID设置 、单双向切换
****************************************************************************************/
int8_t CAM_SendCommandStart(uint8_t cmd , uint8_t *pdata ,uint8_t len)
{
    if(s_bSendCmdFlag)
    {
        my_printf("CAM is busy!  - 0x%x is running!\n", astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].TxCdm);
        return -2;
    }
    s_bSendCmdFlag = true;

    // 检查队列是否已满
    if((astCamCtrl.TailPtr > astCamCtrl.HeadPtr && astCamCtrl.TailPtr == (CAM_QUEUE_SIZE-1) && astCamCtrl.HeadPtr == 0)
        || (astCamCtrl.TailPtr < astCamCtrl.HeadPtr && astCamCtrl.TailPtr +1 == astCamCtrl.HeadPtr))
    {
        s_bSendCmdFlag = false;
        return -1;
    }

    my_printf("CAM - CAM_SendCommandStart CMD-0x%x \n", cmd);

    memset((void*)&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr], 0, sizeof(CAM_CONTROL));
    astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxCdm=cmd;//拷贝命令字
    
    switch (cmd)
    {
        case CAM_CMD_SSID_SEND:  // 设置SSID
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //保留码
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=len+1;//数据包+保留码
            memcpy(&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1],pdata,len); //拷贝有效数据
            break;
        case CAM_CMD_SINGLE_MODE_SET: //单向模式设置
        case CAM_CMD_PIR:    //PIR开关
        case CAM_CMD_LINKKEY_SEND:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //保留码
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=len+1;//数据包+保留码
            memcpy(&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1],pdata,len); //拷贝有效数据
            break;    
        case CAM_CMD_DATA_SEND: //透传数据
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //保留码
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1]=0x00; //TCP模式
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[2]=14;   //域名长度
            strcpy((char *)&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3],"106.15.134.146"); //拷贝正式服域名
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3+14]=0x22;
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3+15]=0xC2;
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3+16]=len;   //透传包长度
            memcpy(&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3+14+2+1],pdata,len); //拷贝有效数据
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=20+len;
            break;
            
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
        case CAM_CMD_GET_WIFI_STATE:
#endif
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Systick=5000; //唤醒5秒超时
        //无有效数据包命令
        case CAM_CMD_FAST_SLEEP_SET:
        case CAM_CMD_SN_GET:
        case CAM_CMD_GET_IP:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //保留码
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=len+1;//数据包+保留码
            break;    
        case CAM_CMD_BELL:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxCdm=CAM_CMD_ALARM_SEND; //改命令
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x01; //抓拍
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1]=99;   //电量
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[2]=0;    //冻结方式无
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3]=0;    //报警方式无
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[4]=1;    //门铃事件
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=5;
            break;
        case CAM_CMD_ALARM:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxCdm=CAM_CMD_ALARM_SEND; //改命令
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x01;  //抓拍
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1]=100;   //电量
            if(CaptureType==LOCKPICKING)        //撬锁
            {
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[2]=0;   //冻结方式无
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3]=1;   //报警方式 0无报警 1防翘报警 2非法闯入 3钥匙报警 4一键布防 6镜头遮挡
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[4]=0;   //事件     1门铃 6清空 8试错
            }
            else if(CaptureType==EPLOYMENT)     //布防
            {
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[2]=0;
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3]=4;
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[4]=0;
            }
            else                                //禁试
            {
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[2]=0;
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3]=0;
                astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[4]=8;
            }

            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=5;
            break;
        case CAM_CMD_CLEAR:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Systick=5000;
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxCdm=CAM_CMD_ALARM_SEND; //改命令
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //系统配置
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1]=100;  //电量
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[2]=0;    //冻结方式无
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[3]=0;    //报警方式
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[4]=6;    //清空事件
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=5;    
            break;
            
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
        case CAM_CMD_SET_FOV:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Systick = 5000;
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //保留码
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=len+1;//数据包+保留码
            memcpy(&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1],pdata,len); //拷贝有效数据
            break;
#endif
#ifdef XM_CAM_SCREEN_FUNCTION_ON
        case CAM_CMD_GOOUT_TIP:
        case CAM_CMD_OPENDOOR_TIP:
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[0]=0x00; //保留码
            astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].Txlen=len+1;//数据包+保留码
            memcpy(&astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].TxDataBuf[1],pdata,len); //拷贝有效数据
            break;
#endif
        default:
            break;
    }

    uint8_t taskId = astCamCtrl.TailPtr;
    if(astCamCtrl.TailPtr == (CAM_QUEUE_SIZE-1))
    {
        astCamCtrl.TailPtr = 0;
    }
    else
    {
        astCamCtrl.TailPtr++;
    }
    
    astCamCtrl.astCamCtrl[astCamCtrl.TailPtr].State=CAM_WAKE_UP;//开始唤醒猫眼
    s_bSendCmdFlag = false;
    
    return taskId;
}

/***************************************************************************************
**函数名:       CAM_GetCameraSn
**功能描述:     猫眼序列号
**输入参数:     *pdata 数据指针  
**输出参数:     数据长度
**备注:        
****************************************************************************************/
uint8_t CAM_GetCameraData(uint8_t CMD, uint8_t *pdata , uint8_t taskId)
{
    if(astCamCtrl.astCamCtrl[taskId].RxDataBuf[CAM_PID_CDM]==CMD) //检查命令
    {
        memcpy(pdata,&astCamCtrl.astCamCtrl[taskId].RxDataBuf[CAM_PID_DATA+1],astCamCtrl.astCamCtrl[taskId].RxDataBuf[CAM_PID_LEN]-5);
        return astCamCtrl.astCamCtrl[taskId].RxDataBuf[CAM_PID_LEN]-5;
    }
    return 0;
}

/***************************************************************************************
**函数名:       CAM_AnalysisWifiSignalStrength
**功能描述:     解析WIFI信号强度数据
**输入参数:     *pdata 数据指针  
**输出参数:     数据长度
**备注:        
****************************************************************************************/
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
bool CAM_AnalysisWifiSignalStrength(uint8_t *pdata , uint8_t taskId)
{
    if(astCamCtrl.astCamCtrl[taskId].RxDataBuf[CAM_PID_CDM] == CAM_CMD_GET_WIFI_STATE) //检查命令
    {
        *pdata = astCamCtrl.astCamCtrl[taskId].RxDataBuf[CAM_PID_DATA + 2]; //0:保留码 +1:状态 +2:信号强度 +3:信噪比
        return true;
    }
    return false;
}
#endif


/***************************************************************************************
**函数名:       CAM_SetCapture
**功能描述:     猫眼推送抓拍类型
**输入参数:     Type 抓拍类型
**输出参数:     
**备注:        
****************************************************************************************/
void CAM_SetCapture(uint8_t Type)
{
    if(LOCKPICKING == Type || FORBID_TRY == Type || EPLOYMENT == Type || DEFENSE == Type)
    {
        CaptureType = Type;
        CAM_SendCommandStart(CAM_CMD_ALARM, NULL, 0 );
        CaptureType = 0;
    }
    return;
}

/***************************************************************************************
**函数名:       CAM_GetPushAlarmState
**功能描述:     猫眼推送
**输入参数:     
**输出参数:     是否成功bool
**备注:        
****************************************************************************************/
uint8_t CAM_GetPushAlarmState(void)
{
    return 1;
    if( 0 == CaptureType)
    {
        return 1;
    }
    static int8_t taskId = -1;
    if(taskId < 0)
    {
        taskId = CAM_SendCommandStart(CAM_CMD_ALARM, NULL, 0 ); //抓拍
        return 0;
    }
    else
    {
        if(CAM_GetServerState(taskId)==CAM_SUCCESSFUL || CAM_GetServerState(taskId)==CAM_FAIL)
        {
            CaptureType = 0;
            taskId = -1;
            return 1;
        }
    }
    
    return 0;
}



/***************************************************************************************
**函数名:       CAM_ClearCameraData
**功能描述:     猫眼清空
**输入参数:    void
**输出参数:    是否成功bool
**备注:        
****************************************************************************************/
int8_t CAM_ClearCameraData(void)
{
    return CAM_SendCommandStart(CAM_CMD_CLEAR, NULL, 0);
}

/***************************************************************************************
**函数名:       CAM_GetQueueClearState
**功能描述:     获取猫眼队列清空状态
**输入参数:    void
**输出参数:    bool 清空返回 true 未清空返回false
**备注:        
****************************************************************************************/
bool CAM_GetQueueClearState(void)
{
    return cam_get_queue_len() > 0 ? false : true;
}
/***************************************************************************************
**函数名:       CAM_GoOutGiveNotice
**功能描述:     门外开门通知猫眼
**输入参数:    void
**输出参数:    参照 CAM_SendCommandStart 返回值
**备注:        
****************************************************************************************/
#ifdef XM_CAM_SCREEN_FUNCTION_ON
int8_t CAM_GoOutGiveNotice(void)
{
    uint8_t tmp[8] = {0};

    tmp[0] = (uint8_t)(Rtc_Real_Time.year >> 8);
    tmp[1] = (uint8_t)(Rtc_Real_Time.year);
    tmp[2] = Rtc_Real_Time.month;
    tmp[3] = Rtc_Real_Time.day;
    tmp[4] = Rtc_Real_Time.hour;
    tmp[5] = Rtc_Real_Time.minuter;
    tmp[6] = Rtc_Real_Time.second;
    
    return CAM_SendCommandStart(CAM_CMD_GOOUT_TIP, tmp, 7);
}
#endif

/***************************************************************************************
**函数名:       CAM_OpenDoorGiveNotice
**功能描述:     门外开门通知猫眼
**输入参数:    void
**输出参数:    参照 CAM_SendCommandStart 返回值
**备注:        
****************************************************************************************/
#ifdef XM_CAM_SCREEN_FUNCTION_ON
int8_t CAM_OpenDoorGiveNotice(LOCK_EVENT_LOG _em, uint16_t _id, LOCK_EVENT_LOG _em2, uint16_t _id2)
{
    /* 开门方式：0x01指纹，0x02密码，0x03卡，0x04人脸，0x05静脉，0x06机械钥匙，0x07虹膜，0x09电子钥匙
            非以上类型不做处理
       */
    if(FINGER_OPEN != _em && PASSWORD_OPEN != _em && FACE_OPEN != _em && VEIN_OPEN != _em && IRIS_OPEN != _em && BLE_OPEN != _em)
    {
        return -3;
    }

    uint8_t tmp[8] = {0};

    /*  第一个开门类型 */
    switch(_em)
    {
        case FINGER_OPEN:
            tmp[0] = 0x01;
            break;
        case PASSWORD_OPEN:
            tmp[0] = 0x02;
            break;
        case FACE_OPEN:
            tmp[0] = 0x04;
            break;
        case VEIN_OPEN:
            tmp[0] = 0x05;
            break;
        case IRIS_OPEN:
            tmp[0] = 0x07;
            break;
        case BLE_OPEN:
            tmp[0] = 0x09;
            break;
        default:
            return -3;
    }

    /*  第一个开门类型ID */
    tmp[1] = (uint8_t)(_id >> 8);
    tmp[2] = (uint8_t)(_id);

    if( NOTHING_CASE != _em2)
    {
        /*  第二个开门类型 */
        switch(_em2)
        {
            case FINGER_OPEN:
                tmp[3] = 0x01;
                break;
            case PASSWORD_OPEN:
                tmp[3] = 0x02;
                break;
            case FACE_OPEN:
                tmp[3] = 0x04;
                break;
            case VEIN_OPEN:
                tmp[3] = 0x05;
                break;
            case IRIS_OPEN:
                tmp[3] = 0x07;
                break;
            case BLE_OPEN:
                tmp[3] = 0x09;
                break;
            default:
                return -3;
        }

        /*  第二个开门类型ID */
        tmp[4] = (uint8_t)(_id2 >> 8);
        tmp[5] = (uint8_t)(_id2);
    }
    
    return CAM_SendCommandStart(CAM_CMD_OPENDOOR_TIP, tmp, 6);
}
#endif

/***************************************************************************************
**函数名:       CAM_GetWifiSignalStrength
**功能描述:     猫眼视场角设置
**输入参数:    void
**输出参数:    参照 CAM_SendCommandStart 返回值
**备注:        
****************************************************************************************/
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
int8_t CAM_GetWifiSignalStrength(void)
{
    return CAM_SendCommandStart(CAM_CMD_GET_WIFI_STATE, 0, 0);
}
#endif


/***************************************************************************************
**函数名:       CAM_SetFovParam
**功能描述:     猫眼视场角设置
**输入参数:    void
**输出参数:    参照 CAM_SendCommandStart 返回值
**备注:        
****************************************************************************************/
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
int8_t CAM_SetFovParam(uint8_t _u8Param)
{
    return CAM_SendCommandStart(CAM_CMD_SET_FOV, &_u8Param, 1);
}
#endif


/***************************************************************************************
**函数名:       CAM_PirWakeUpTest
**功能描述:     猫眼产测检验PIR
**输入参数:     void
**输出参数:     0执行中 1成功 2失败
**备注:         锁发送打开PIR>锁发送DE快速休眠>测试人员手晃动PIR>模组被唤醒上报唤醒源>确认唤醒源为PIR>锁发送关闭PIR指令>结束  
****************************************************************************************/
uint8_t CAM_PirWakeUpTest(void)
{
    static uint8_t TestPro=0;
    static uint32_t Time=0;
    static uint32_t testTaskId=0;
    uint8_t Pir=0;
    uint8_t tp1=0;
    switch (TestPro)
    {
        case 0:
            Pir=1;
            testTaskId = CAM_SendCommandStart(CAM_CMD_PIR,&Pir,1); //开启PIR
            TestPro++;
            break;
        case 1: 
            tp1=CAM_GetServerState(testTaskId);
            if( CAM_SUCCESSFUL == tp1) //开启成功
            {
                TestPro++ ;
            }
            else if(CAM_FAIL == tp1 ) 
            {
                TestPro=8 ;//失败
            }
            break;
        case 2:
            testTaskId = CAM_SendCommandStart(CAM_CMD_FAST_SLEEP_SET,NULL,0); //发送快速睡眠指令
            TestPro++;
            break;
        case 3: 
            tp1=CAM_GetServerState(testTaskId);
            if( CAM_SUCCESSFUL == tp1 || CAM_FAIL == tp1)//休眠成功(fail的原因在于海带模组回复80，CAM_GetServerState是失败)
            {
                HAL_Voice_PlayingVoice( EM_BUTTON_TIPS_MP3, 0 );  //提示音
                //串口初始化
                UartCfg_S uartCfg={0};
                uartCfg.BaudRate = CAM_BAUDRATE;
                uartCfg.DataBit = DATA_8_BIT;
                uartCfg.StopBit = STOP_1_BIT;
                uartCfg.ParityType = PARITY_NONE;
                uartCfg.RxInerruptEn = INT_ENABLE;
                uartCfg.TxInerruptEn = INT_DISENABLE;
                HAL_Uart_ConfigInit( E_CAMERA_UART, uartCfg );
                CAM_UTRT_SWITCH_ON;    
                Time=Rtc_Real_Time.timestamp; //记录当前时间
                TestPro++ ;
            }
            break;
        case 4:
            if(Rtc_Real_Time.timestamp -Time  >15) //10秒超时 等待PIR唤醒
            {
                TestPro=8 ;//失败
            }
            else if(CAM_ReadFifoData()) //收到ready或者应答
            {
                if((astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[CAM_PID_CDM]==CAM_CMD_WAKE_REASON)&& //DC唤醒源
                (astCamCtrl.astCamCtrl[astCamCtrl.HeadPtr].RxDataBuf[CAM_PID_DATA]==0x02))//PIR唤醒
                {
                    TestPro++; //唤醒成功
                }
            }
            break;
        case 5:
            Pir=0;
            testTaskId = CAM_SendCommandStart(CAM_CMD_PIR,&Pir,1); //关闭PIR
            TestPro++;
            break;
        case 6: 
            tp1=CAM_GetServerState(testTaskId);
            if( CAM_SUCCESSFUL == tp1) //开启成功
            {
                TestPro=7 ; //成功
            }
            else if(CAM_FAIL == tp1 ) 
            {
                TestPro=8 ;//失败
            }
            break;
        case 7: //成功
            TestPro =0 ;
            return 1;
        case 8: //失败
            Pir=0;
            CAM_SendCommandStart(CAM_CMD_PIR,&Pir,1); //关闭PIR
            TestPro =0 ;
            return 2;
        default:
            break;
    }
    return 0;
}
#endif



//.end of the file.
