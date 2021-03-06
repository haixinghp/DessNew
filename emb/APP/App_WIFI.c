/********************************************************************************************************************
 * @file:        App_WIFI.h
 * @author:      lixiqun
 * @version:     V01.00
 * @date:        2021-08-011
 * @Description: wifi接口功能函数文件
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

#include "../server/Face.h"

#include "APP_FACE_PRO.h"
#include "APP_CAM.h"
#include "App_WIFI.h" 
#include "System.h" 
#include "SystemTim.h"
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define	WIFI_CT_GPIO_PIN	M_WIFI_CT_GPIO_PIN
#define	WIFI_RT_GPIO_PIN	M_WIFI_RT_GPIO_PIN
#define	WIFI_POW_GPIO_PIN	M_WIFI_POW_GPIO_PIN
#define WIFI_TX_GPIO_PIN	M_WIFI_TX_GPIO_PIN
#define WIFI_RX_GPIO_PIN	M_WIFI_RX_GPIO_PIN

#define WIFI_BAUDRATE		         UART_BAUD_RATE_19200
#define WIFI_SEND_PHOTOS_BAUDRATE	 UART_BAUD_RATE_115200

#define WIFI_TX_POW_OFF()	DRV_GpioOut0(WIFI_TX_GPIO_PIN)
#define WIFI_RX_POW_OFF()	DRV_GpioOut0(WIFI_RX_GPIO_PIN)

#define WIFI_CT_INIT()		DRV_GpioInputPullnull(WIFI_CT_GPIO_PIN)
#define WIFI_RT_INIT()		DRV_GpioOutPP(WIFI_RT_GPIO_PIN)

#define WIFI_CT_READ()      DRV_GpioRead(WIFI_CT_GPIO_PIN) 
#define WIFI_RT_POW_OFF()   DRV_GpioOut0(WIFI_RT_GPIO_PIN) 
#define WIFI_CT_POW_OFF()   DRV_GpioOut0(WIFI_CT_GPIO_PIN) 
#define WIFI_POW_OFF()      (HW_ACTIVE_LEVEL_LOW == M_POW_WIFI_ACTIVE_LEVEL)?(DRV_GpioOut1( WIFI_POW_GPIO_PIN )):(DRV_GpioOut0( WIFI_POW_GPIO_PIN ))
#define WIFI_POW_ON()       (HW_ACTIVE_LEVEL_LOW == M_POW_WIFI_ACTIVE_LEVEL)?(DRV_GpioOut0( WIFI_POW_GPIO_PIN )):(DRV_GpioOut1( WIFI_POW_GPIO_PIN ))

/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/


/*-------------------------------------------------局部变量定义-----------------------------------------------------*/         
WifiLockUploadMeg_T WifiLockMeg; 
WifiRxMeg_T WifiRxData;
WIFI_TX_STATE_E OB_CAM_TxState;
/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
static uint16_t WifiTxTimout;
static uint16_t WifiRxTimout;
typedef struct
{
	uint8_t UploadData[WIFI_UPLOAD_DATA_LENGTH_MAX];
	uint16_t UploadDataLength;
}WifiEmail_T;
typedef struct
{
	uint8_t HeadPtr;	//头指针
	uint8_t TailPtr;	//尾指针
	WifiEmail_T pEmail[QUEUE_SIZE];//wifi上行存储数据
}WifiQueue_T;

WifiQueue_T WifiUploadMsg = {0, 0, {0}};

/*-------------------------------------------------全变量定义-----------------------------------------------------*/
WifiRxMeg_T WifiRxData;
WifiLockUploadMeg_T WifiLockMeg;
WifiTx WifiTxTemp;
/*-------------------------------------------------函数声明---------------------------------------------------------*/
static void App_Wifi_TxMessage( uint8_t *p_data, uint16_t length );

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  WIFI_UploadDataClear(void)
* Description   :  清消息队列
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void WIFI_UploadDataClear(void)
{
	WifiUploadMsg.HeadPtr = WifiUploadMsg.TailPtr = 0;
} 

/*********************************************************************************************************************
* Function Name :  WIFI_UploadDataGetLength(void)
* Description   :  获取队列长度
* Para          :  无
* Return        :  DataLength
*********************************************************************************************************************/
extern uint8_t WIFI_UploadDataGetLength(void)
{
	uint8_t DataLength;
	if((WifiUploadMsg.HeadPtr >= QUEUE_SIZE)\
		|| (WifiUploadMsg.TailPtr >= QUEUE_SIZE))//超过数组长度则清零
	{
		WIFI_UploadDataClear();
	}
	if(WifiUploadMsg.TailPtr >= WifiUploadMsg.HeadPtr)//如果尾指针大于头指针 则长度正常取 (尾位置 - 头位置)
	{
		DataLength = WifiUploadMsg.TailPtr - WifiUploadMsg.HeadPtr;
	}
	else //否则若头位置大于尾位置 则表明数据二次写入 头指针靠近消息队列出口 尾指针靠近消息队列入口 长度取(尾位置 + 数组深度 - 头位置)
	{
		DataLength = WifiUploadMsg.TailPtr + QUEUE_SIZE - WifiUploadMsg.HeadPtr;
	}
	return DataLength;
}

/*********************************************************************************************************************
* Function Name :  WIFI_UploadDataFill(uint8_t buf)
* Description   :  往队列里添加消息 返回0表示队列信息已满 无法再存储
* Para          :  无
* Return        :  0 : 队列信息已满  1 : 数据成功存入队列
*********************************************************************************************************************/
extern uint8_t WIFI_UploadDataFill(const uint8_t *buf, uint16_t length)
{
	my_printf("WIFI_UploadDataFill\n");
	uint16_t Length = 0;
	Length = WIFI_UploadDataGetLength();
	if(Length >= QUEUE_SIZE)//消息队列已满 
	{
		return QUEUE_FULL;
	}
	WifiUploadMsg.pEmail[WifiUploadMsg.TailPtr].UploadDataLength = length;
	for(uint8_t i = 0; i < length; i++)
	{
		WifiUploadMsg.pEmail[WifiUploadMsg.TailPtr].UploadData[i] = buf[i];
	}
	WifiUploadMsg.TailPtr++;//尾指针位置下移 表示数据多了一字节
	if(WifiUploadMsg.TailPtr >= QUEUE_SIZE)//若尾指针位置超出队列长度 则重新置零
	{
		WifiUploadMsg.TailPtr = 0;
	}
	return QUEUE_OK;
}

/*********************************************************************************************************************
* Function Name :  WIFI_UploadData_Get(uint8_t *buf)
* Description   :  往队列里添加消息 返回0表示队列信息已满 无法再存储
* Para          :  无
* Return        :  0 : 队列信息为空  1 : 数据成功取出队列
*********************************************************************************************************************/
extern uint8_t WIFI_UploadData_Get(uint8_t *buf, uint16_t *length)
{
	my_printf("WIFI_UploadData_Get\n");
	uint16_t Length = 0;
	Length = WIFI_UploadDataGetLength();
	if(0 == Length)//队列为空
	{
		return QUEUE_EMPTY;
	}
	for(uint8_t i = 0; i < WifiUploadMsg.pEmail[WifiUploadMsg.HeadPtr].UploadDataLength; i++)
	{
		buf[i] = WifiUploadMsg.pEmail[WifiUploadMsg.HeadPtr].UploadData[i];
	}
	*length = WifiUploadMsg.pEmail[WifiUploadMsg.HeadPtr].UploadDataLength;
	WifiUploadMsg.HeadPtr++;//头指针位置下移 表示数据少了一帧
	if(WifiUploadMsg.HeadPtr >= QUEUE_SIZE)
	{
		WifiUploadMsg.HeadPtr = 0;
	}
	return QUEUE_OK;
}
/*********************************************************************************************************************
* Function Name :  App_Key_Tim10Ms()
* Description   :  功能相关定时器   10ms触发一次
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_Wifi_Tim10Ms(void)
{
	if( WifiTxTimout > 0 )
        WifiTxTimout--;
	if( WifiRxTimout > 0 )
        WifiRxTimout--;
} 

/*********************************************************************************************************************
* Function Name :  App_WIFI_PowerOff()
* Description   :  wifi下电
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void WIFI_PowerOff(void)
{
	my_printf("WIFI_PowerOff\n");
	if( E_WIFI_UART == HAL_Uart_GetCurDeviceType( WIFI_UART_COM ))
	{
		HAL_Uart_DeInit( E_WIFI_UART );
	}
	WIFI_POW_OFF();
}
 
/*********************************************************************************************************************
* Function Name :  App_WIFI_PowerOn()
* Description   :  wifi上电
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void WIFI_PowerOn(void)
{
	my_printf("WIFI_PowerOn\n");
	WIFI_POW_ON();
	//串口初始化
	UartCfg_S uartCfg={0};
	uartCfg.BaudRate = WIFI_BAUDRATE;
	uartCfg.DataBit = DATA_8_BIT;
	uartCfg.StopBit = STOP_1_BIT;
	uartCfg.ParityType = PARITY_NONE;
	uartCfg.RxInerruptEn = INT_ENABLE;
	uartCfg.TxInerruptEn = INT_DISENABLE;
	HAL_Uart_ConfigInit( E_WIFI_UART, uartCfg );
}

/*********************************************************************************************************************
* Function Name :  WIFI_GetCheckSumWord
* Description   :  计算校验和
* Para          :  pdata-待校验的数据  len-数据长度
* Return        :  校验和
*********************************************************************************************************************/
static uint16_t WIFI_GetCheckSumWord(const uint8_t *pdata, uint16_t len)
{
    uint16_t sum =0;
    for(uint16_t i = 0; i<len; i++ )
    {
        sum += pdata[i];
    }
    return( sum );
}
/*********************************************************************************************************************
* Function Name :  App_WIFI_CommomTx(uint8_t cmd)
* Description   :  主进程
* Para          :  cmd
* Return        :  void
*********************************************************************************************************************/
uint8_t App_WIFI_CommomTx(uint8_t cmd)
{
	uint8_t txbuf[1024] = {0};
	uint16_t tp2, len;
	uint32_t under_bat_data = 0;
	uint32_t upper_bat_data = 0;
	txbuf[0] = 0xFE;//帧头
	txbuf[1] = 0x02;//属性  02-无后续帧   03-有后续帧(服务器有应答)
	RTC_TimeUpdate(RTC_TIME_CLOCK_BCD);
	WifiLockMeg.DevType = 0x0b;
    uint8_t ret = 0;
	switch(cmd)
	{
		case WIFI_CMD_CONF_ROUTER://设置路由器(0XF6)
			txbuf[2] = cmd;//命令字
			tp2 = 0x62;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
		    for(uint16_t i = 0; i < sizeof(WifiLockMeg.Ssid); i++ )
			{
				txbuf[5 + i] = WifiLockMeg.Ssid[i];//路由器SSID 33字节
			}
			for(uint16_t i = 0; i < sizeof(WifiLockMeg.Passwd); i++ )
			{
				txbuf[5 + 33 + i] = WifiLockMeg.Passwd[i];//路由器密码 65字节  不足全部补零
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			App_Wifi_TxMessage(txbuf, len + 7);
			WifiLockMeg.ConfigState = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_CONF_SERVER://设置服务器域名(0XF7)
			txbuf[2] = cmd;//命令字
			tp2 = 0x20;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
		    for(uint16_t i = 0; i < sizeof(WifiLockMeg.severname); i++ )
			{
				txbuf[5 + i] = WifiLockMeg.severname[i];//服务器主域名   30byte  不足补零
			}
			txbuf[ 5 + 30 ] = WifiLockMeg.portno[0];
			txbuf[ 5 + 31 ] = WifiLockMeg.portno[1];
			len = txbuf[3]*256 + txbuf[4];//校验和
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			App_Wifi_TxMessage(txbuf, len + 7);
//			PUBLIC_PrintHex("WifiTxData",txbuf,len + 7);
			WifiLockMeg.ConfigState = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_CLOUD_DATA://主动获取云端数据(0X0A)
			txbuf[2] = cmd;//命令字
			tp2 = 0x11;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			txbuf[5] = 0x01;//获取天气信息
			PUBLIC_GetMacAdd(&txbuf[6]);//MAC地址
			txbuf[12] = Rtc_Real_Time.year;//锁具当前时间
			txbuf[13] = Rtc_Real_Time.month;
			txbuf[14] = Rtc_Real_Time.day;
			txbuf[15] = Rtc_Real_Time.hour;
			txbuf[16] = Rtc_Real_Time.minuter;
			txbuf[17] = Rtc_Real_Time.second;//锁具当前时间
			upper_bat_data = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
			txbuf[18] = (uint8_t)(upper_bat_data  >> 8);//电量提示
			txbuf[19] = (uint8_t)upper_bat_data;//电量提示
			under_bat_data = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
			txbuf[20] = (uint8_t)((under_bat_data) >> 8);//电量提示
			txbuf[21] = (uint8_t)(under_bat_data);//电量提示
			len = txbuf[3]*256 + txbuf[4];//校验和
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			#if defined XM_CAM_FUNCTION_ON //开门上报
				ret = CAM_SendCommandStart(CAM_CMD_DATA_SEND, txbuf, len + 7);
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
				memcpy(WifiTxTemp.data, txbuf, len + 7);
				WifiTxTemp.length = len + 7;
			#else
				ret = WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			#endif
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_UPLOAD_UNLOCK_MEG://开锁信息上传(0x04)
			txbuf[2] = cmd;//命令字
			if(WifiLockMeg.WifiFactoryTest == 1)
			{
				WifiLockMeg.WifiFactoryTest = 0;
				tp2 = 25;//数据长度
				txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
				txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
				PUBLIC_GetMacAdd(&txbuf[5]);//MAC地址
				memcpy(&txbuf[11],WifiLockMeg.WifiFactoryTestNum,19);
				len = txbuf[3]*256 + txbuf[4];//校验和
				tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
				txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
				txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			}
			else
			{
				if(WifiLockMeg.UnlockMode <= 0x0C)//开锁方式对应方式二本次开门没有为空
				{
					tp2 = 39;//数据长度
				}
				else
				{
					tp2 = 43;//数据长度
					txbuf[44] = (uint8_t)WifiLockMeg.UnlockWay2SuccessTime[0];//开锁方式对应方式二历史成功次数（本次开门没有为空）
					txbuf[45] = (uint8_t)WifiLockMeg.UnlockWay2SuccessTime[1];
					txbuf[46] = (uint8_t)WifiLockMeg.UnlockWay2FailTime[0];//开锁方式对应方式二历史失败次数（本次开门没有为空）
					txbuf[47] = (uint8_t)WifiLockMeg.UnlockWay2FailTime[1];
				}
				txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
				txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
				//开锁实时上传数据包
				txbuf[5] = WifiLockMeg.DevType;//设备类型
				PUBLIC_GetMacAdd(&txbuf[6]);//MAC地址
				#if defined LOCK_BODY_212_MOTOR
					txbuf[12] = LOCK_BODY_212;//锁体信息
				#elif defined LOCK_BODY_216_MOTOR
					txbuf[12] = LOCK_BODY_216;//锁体信息
				#elif defined LOCK_BODY_218_MOTOR
					txbuf[12] = LOCK_BODY_218;//锁体信息
				#elif defined LOCK_BODY_AUTO_MOTOR
					if(LockConfigMode == LOCK_BODY_212)
					{
						txbuf[12] = LOCK_BODY_212;//锁体信息
					}
					else if(LockConfigMode == LOCK_BODY_218)
					{
						txbuf[12] = LOCK_BODY_218;//锁体信息
					}
				#endif
				txbuf[13] = WifiLockMeg.DevInfor.FactoryInfo;//OEM/ODM厂家信息
				txbuf[14] = WifiLockMeg.DevInfor.FingerModuleInfo;//指纹模块厂家信息
				txbuf[15] = WifiLockMeg.DevInfor.MotorUnlokCount;//电机开门，工作次数
				txbuf[16] = WifiLockMeg.DevInfor.MotorUnlokTime[0];//开门时间电机工作时间
				txbuf[17] = WifiLockMeg.DevInfor.MotorUnlokTime[1];//开门时间电机工作时间
				txbuf[18] = WifiLockMeg.DevInfor.MotorUnlokCurrentVal[0];//开门电机电流平均值
				txbuf[19] = WifiLockMeg.DevInfor.MotorUnlokCurrentVal[1];//开门电机电流平均值
				for(uint8_t i= 0; i < 4; i++)
				{
					txbuf[20 + i] = WifiLockMeg.DevInfor.MotorUnlokAdc[i];//MAC地址
				}
				upper_bat_data = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
				txbuf[24] = (uint8_t)(upper_bat_data  >> 8);//电量提示
				txbuf[25] = (uint8_t)upper_bat_data;//电量提示
				txbuf[26] = WifiLockMeg.UnlockMode;//开锁方式
				txbuf[27] = WifiLockMeg.PageID.way1;//PageID
				txbuf[28] = WifiLockMeg.PageID.id1;//PageID
				txbuf[29] = WifiLockMeg.PageID.way2;//PageID
				txbuf[30] = WifiLockMeg.PageID.id2;//PageID
				txbuf[31] = WifiLockMeg.Attribute;//属性
				txbuf[32] = Rtc_Real_Time.year;//开锁时间
				txbuf[33] = Rtc_Real_Time.month;
				txbuf[34] = Rtc_Real_Time.day;
				txbuf[35] = Rtc_Real_Time.hour;
				txbuf[36] = Rtc_Real_Time.minuter;
				txbuf[37] = Rtc_Real_Time.second;//开锁时间
				#if defined WIZARD_UPPER_BAT_ADC_ON && defined WIZARD_UNDER_BAT_ADC_ON
					under_bat_data = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
					tp2 = 35;//数据长度
					txbuf[38] = (uint8_t)((under_bat_data) >> 8);//电量提示
					txbuf[39] = (uint8_t)(under_bat_data);//电量提示
				#endif
				txbuf[40] = (uint8_t)WifiLockMeg.UnlockWay1SuccessTime[0];//开锁方式对应方式一历史成功次数
				txbuf[41] = (uint8_t)WifiLockMeg.UnlockWay1SuccessTime[1];
				txbuf[42] = (uint8_t)WifiLockMeg.UnlockWay1FailTime[0];//开锁方式对应方式一历史失败次数
				txbuf[43] = (uint8_t)WifiLockMeg.UnlockWay1FailTime[1];
				len = txbuf[3]*256 + txbuf[4];//校验和
				tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
				txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
				txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			}
			#if defined XM_CAM_FUNCTION_ON //开门上报
				ret = CAM_SendCommandStart(CAM_CMD_DATA_SEND, txbuf, len + 7);
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
				memcpy(WifiTxTemp.data, txbuf, len + 7);
				WifiTxTemp.length = len + 7;
				if(WifiLockMeg.UnlockMode == SERVER_TEST)//测试wifi的数据没有存队列直接可发
				{
				
				}
				else//发送开锁信息
				{
					OB_CAM_TxState = WIFI_TX_PREPARE;
				}
			#else
				WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			#endif
			if(WifiLockMeg.State == WIFI_TX_OVER)
			{
				WifiLockMeg.State = WIFI_TX_PREPARE;
			}
			break;
		case WIFI_CMD_UPLOAD_ERROR_MEG://故障信息上传(0x07)
			txbuf[2] = cmd;//命令字
			#if defined WIZARD_UPPER_BAT_ADC_ON && defined WIZARD_UNDER_BAT_ADC_ON
				under_bat_data = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
				tp2 = 24;//数据长度
				txbuf[27] = (uint8_t)((under_bat_data) >> 8);//电量提示
				txbuf[28] = (uint8_t)(under_bat_data);//电量提示
			#else
				tp2 = 22;//数据长度
			#endif
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
		    txbuf[5] = WifiLockMeg.DevType;//设备类型
			PUBLIC_GetMacAdd(&txbuf[6]);//MAC地址
			txbuf[12] = Rtc_Real_Time.year;//开锁时间
			txbuf[13] = Rtc_Real_Time.month;
			txbuf[14] = Rtc_Real_Time.day;
			txbuf[15] = Rtc_Real_Time.hour;
			txbuf[16] = Rtc_Real_Time.minuter;
			txbuf[17] = Rtc_Real_Time.second;//开锁时间
			txbuf[18] = WifiLockMeg.DevInfor.Reserve;//预留补0
			txbuf[19] = WifiLockMeg.DevInfor.Reserve;//预留补0
			txbuf[20] = WifiLockMeg.DevInfor.Reserve;//预留补0
			txbuf[21] = WifiLockMeg.DevInfor.Reserve;//预留补0
			txbuf[22] = WifiLockMeg.DevInfor.Reserve;//预留补0
			txbuf[23] = WifiLockMeg.DevInfor.FactoryInfo;//OEM/ODM厂家信息
			upper_bat_data = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
			txbuf[24] = (uint8_t)(upper_bat_data >> 8);//电量提示
			txbuf[25] = (uint8_t)upper_bat_data;//电量提示
			txbuf[26] = WifiLockMeg.AlarmMeg;//报警信息
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			#ifdef XM_CAM_FUNCTION_ON  //报警透传
				ret = CAM_SendCommandStart(CAM_CMD_DATA_SEND, txbuf, len + 7);
				CAM_SetCapture(WifiLockMeg.AlarmMeg); //执行抓拍
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
				memcpy(WifiTxTemp.data, txbuf, len + 7);
				WifiTxTemp.length = len + 7;
				if((WifiLockMeg.AlarmMeg == LOCKPICKING) || (WifiLockMeg.AlarmMeg == FORBID_TRY) || (WifiLockMeg.AlarmMeg == EPLOYMENT) || (WifiLockMeg.AlarmMeg == DEFENSE))//逗留保护告警直接可发
				{
				
				}
				else//发送开锁信息从队列里取数据再发送
				{
					OB_CAM_TxState = WIFI_TX_PREPARE;
				}
			#else
				WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			#endif
			if(WifiLockMeg.State == WIFI_TX_OVER)
			{
				WifiLockMeg.State = WIFI_TX_PREPARE;
			}
			break;
		case WIFI_CMD_UPLOAD_IMAGE_MEG://图片上传(0x09)
			txbuf[1] = 0x03;//03代表有后续数据
//			if(WifiLockMeg.PhotoPakageSum == WifiLockMeg.PhotoPackageNum)
//			{
//				txbuf[1] = 0x02;//02代表无后续数据
//			}
			txbuf[2] = cmd;//命令字
			tp2 = 26 + WifiLockMeg.DataLenth + 2;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			txbuf[5] =	WifiLockMeg.PhotoPakageSum;//包数
			txbuf[6] =	WifiLockMeg.PhotoPackageNum;//包序
		    txbuf[7] = WifiLockMeg.DevType;//设备类型
			PUBLIC_GetMacAdd(&txbuf[8]);//MAC地址
			txbuf[14] = WifiLockMeg.PhotoUploadType;//图片类型
			memcpy(&txbuf[15],WifiLockMeg.PhotoAES,16);
			memcpy(&txbuf[31], WifiLockMeg.PhotoData, (WifiLockMeg.DataLenth + 2));
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			my_printf("App_Wifi_TxMessage(txbuf, len + 7) len + 7 = %d\n", len + 7);
			App_Wifi_TxMessage(txbuf, len + 7);
//			PUBLIC_PrintHex("WifiTxData",txbuf,len + 7);
//			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_SET_OTA_UPDATA://OTA更新(0x10)
			txbuf[2] = cmd;//命令字
			tp2 = WifiLockMeg.DataLenth;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
		    for(uint16_t i = 0; i < WifiLockMeg.DataLenth; i++ )
			{
				txbuf[5 + i] = WifiLockMeg.OtaUpdateDataAddr[i];//下载链接
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_OTA_UPDATA://模组自主查询OTA更新(0x11)
			txbuf[2] = cmd;//命令字
			tp2 = WifiLockMeg.DataLenth;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
		    for(uint16_t i = 0; i < WifiLockMeg.DataLenth; i++ )
			{
				txbuf[5 + i] = WifiLockMeg.OtaQueryUpdateData[i];//下载链接
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_SET_BAUDRATE://通讯波特率设置(0xFA)
			txbuf[2] = cmd;//命令字
			tp2 = 1;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			txbuf[5] = WifiLockMeg.BaudrateSet;//通讯波特率(33表示19200，55表示115200(默认19200),EE表示57600,66表示9600)
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			App_Wifi_TxMessage(txbuf, len + 7);
			PUBLIC_PrintHex("WifiTxData",txbuf,len + 7);
//			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
//			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_MAC_ADDR:
			txbuf[2] = cmd;//命令字
			tp2 = 1;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_FACTORY_CHECK://产测扫描路由器(0XFC)
			txbuf[2] = cmd;//命令字
			tp2 = 33;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			for(uint8_t i = 0; i < 33; i++)
			{
				txbuf[5 + i] = WifiLockMeg.Ssid[i];//路由器SSID
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			App_Wifi_TxMessage(txbuf, len + 7);
//			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
//			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_SOFT_VERSION://查询WIFI模块软件版本号(0xFD)
			txbuf[2] = cmd;//命令字
			tp2 = 1;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_PASSWORD://查询当前模块内PSK密码(0xFE)
			txbuf[2] = cmd;//命令字
			tp2 = 1;//数据长度
			txbuf[3] = (uint8_t)(tp2 >> 8);//数据长度
			txbuf[4] = (uint8_t)(tp2 >> 0);//数据长度
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//校验和
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//校验和
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		default:
			break;
	}

    return ret;
}
 

/*********************************************************************************************************************
* Function Name :  App_Wifi_ComRxHandle
* Description   :  接受数据处理
* Para          :  para-接收到的数据
* Return        :  0:未接收到数据
*********************************************************************************************************************/
#ifdef WIFI_FUNCTION_ON
#if !defined(XM_CAM_FUNCTION_ON) && !defined(OB_CAM_FUNCTION_ON) && !defined(ST_CAM_FUNCTION_ON)
static uint8_t WIFI_ComRxHandle(void)
{
    uint8_t  rxdata;
    uint16_t sumcheck = 0, len;
    static uint16_t step;
    static uint8_t rxbuf[600];
    uint8_t cmd;

	if( E_WIFI_UART != HAL_Uart_GetCurDeviceType( WIFI_UART_COM ))
	{
		step = 0;
		return 0;
	}

	if(WifiRxTimout == 0)//1s还未能接收到新的数据认为数据传输结束
	{
        step = 0;
	}
 
	while(UART_SUCCESS == HAL_Uart_PopByteFromQueue( WIFI_UART_COM, &rxdata ))
	{
		WifiRxTimout = 100;
		if( step == 0 )    //帧头
		{
			if(( rxdata != 0xFE ) && ( rxdata != 0x1E ))
				step = 0;
			else
				rxbuf[ step++ ] = rxdata;
		}
		else if( step == 1 ) //属性
		{
			if(( rxdata != 0x02 ) && ( rxdata != 0x03 ))
				step = 0;
			else
				rxbuf[ step++ ] = rxdata;
		}
		else if( step == 2 ) //命令字
		{
			rxbuf[ step++ ] = rxdata;
		}
		else if( (step == 3)||(step == 4) ) //数据长度
		{
			rxbuf[ step++ ] = rxdata;
		}
		else if( step >= 600 )
		{
			step = 0;
		}
		else
		{
			if( step >= (rxbuf[3]*256+ rxbuf[4] + 6) )  //数据接收完毕
			{
				rxbuf[ step ] = rxdata;
				len = rxbuf[3]*256+ rxbuf[4];
				step = 0;
				for(uint16_t i=0; i < len + 4; i++)
					sumcheck += rxbuf[ 1 + i ];
				if( sumcheck == rxbuf[ len + 5 ]*256 + rxbuf[ len + 6 ] ) //校验通过
				{
					cmd = rxbuf[ 2 ];
					
					switch(cmd)
					{
						case WIFI_CMD_CONF_ROUTER:
							WifiRxData.AckResult = rxbuf[5];
							break;
						case WIFI_CMD_CONF_SERVER:
							WifiRxData.AckResult = rxbuf[5];
							break;
						case WIFI_CMD_UPLOAD_UNLOCK_MEG:
							WifiRxData.AckResult = rxbuf[5];
							break;
						case WIFI_CMD_UPLOAD_ERROR_MEG:
							WifiRxData.AckResult = rxbuf[5];
							break;
						case WIFI_CMD_UPLOAD_IMAGE_MEG:
							WifiRxData.PhotoPakageSum = rxbuf[5];
							WifiRxData.PhotoPackageNum = rxbuf[6];
							WifiRxData.AckResult = rxbuf[7];
							break;
						case WIFI_CMD_SET_OTA_UPDATA:
							WifiRxData.AckResult = rxbuf[5];
							break;
						case WIFI_CMD_QUERY_OTA_UPDATA:
							//回复数据还未处理
							break;
						case WIFI_CMD_SET_BAUDRATE:
							WifiRxData.AckResult = rxbuf[5];
							break;
						case WIFI_CMD_QUERY_MAC_ADDR:
							for(uint8_t i = 0; i < 6; i++)
							{
								WifiRxData.WifiMac[i] = rxbuf[5 + i];
							}
							break;
						case WIFI_CMD_FACTORY_CHECK:
							WifiRxData.AckResult = rxbuf[5];
							WifiRxData.SignalIntensity = rxbuf[6];
							break;
						case WIFI_CMD_QUERY_SOFT_VERSION:
							for(uint8_t i = 0; i < 5; i++)
							{
								WifiRxData.SoftwareVersion[i] = rxbuf[5 + i];
							}
							break;
						case WIFI_CMD_QUERY_PASSWORD:
							for(uint8_t i = 0; i < len; i++)
							{
								WifiRxData.WifiPassword[i] = rxbuf[5 + i];
							}
							break;
						default:
							break;
					}
				}
				return 1;
			}
			else
			{
				rxbuf[ step++ ] = rxdata;
			}
		}

	}
	return 0;
}
#endif
#endif
	
/*********************************************************************************************************************
* Function Name :  App_WIFI_UartInit()
* Description   :  串口初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
static void App_WIFI_UartInit(void)//串口初始化
{
	UartCfg_S uartCfg={0};
	uartCfg.BaudRate = WIFI_BAUDRATE;
	uartCfg.DataBit = DATA_8_BIT;
	uartCfg.StopBit = STOP_1_BIT;
	uartCfg.ParityType = PARITY_NONE;
	uartCfg.RxInerruptEn = INT_ENABLE;
	uartCfg.TxInerruptEn = INT_DISENABLE;
	HAL_Uart_ConfigInit( E_WIFI_UART, uartCfg );
}
/*********************************************************************************************************************
* Function Name :  WIFI_DataUploadOperate(uint8_t cmd)
* Description   :  wifi上传数据发送处理
* Para          :  无
* Return        :  0 : wifi无上传数据
*********************************************************************************************************************/
uint8_t WIFI_DataUploadOperate(void)
{
	uint8_t txbuf[WIFI_UPLOAD_DATA_LENGTH_MAX] = {0};
	uint16_t len;
	if(APP_WIFI_TxState() == 1)
		return 0;
	if(((HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_FACE_UART) || 
		(HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_FINGER_UART) ||
		(HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_CAMERA_UART))//串口被占用
		&& (WifiLockMeg.State < WIFI_TX_START))//上电未发送时串口被切走重发
	{
		WifiLockMeg.State = WIFI_TX_PREPARE;
		WIFI_POW_OFF();
		return WifiLockMeg.State;
	}
	
	switch(WifiLockMeg.State)
		
	{
		case WIFI_TX_PREPARE://wifi准备推送
			WIFI_PowerOn();
			WifiTxTimout = 2;
			WifiLockMeg.State = WIFI_POWER_ON;
			break;
		case WIFI_POWER_ON://wifi模块上电延时等待稳定
			if(WifiTxTimout == 0)
			{
				WifiLockMeg.State = WIFI_TX_START;
				WifiTxTimout = 6000;
			}
			break;
		case WIFI_TX_START://开始发送wifi数据
			if((WIFI_CT_READ() == 1))//CT=1,空闲，可以发送数据
			{
				if((HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) != E_WIFI_UART)
					&& HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_NO_UART)//发送wifi数据前确保串口切到wifi
				{
					App_WIFI_UartInit();
				}
				if(HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_WIFI_UART)//发送wifi数据前确保串口切到wifi
				{
					WifiLockMeg.State = WIFI_TX_ING;
					if(WIFI_UploadData_Get(txbuf, &len) == QUEUE_OK)
					{
						App_Wifi_TxMessage(txbuf, len);
						PUBLIC_PrintHex("WifiTxData",txbuf,len);
						my_printf("WIFI_UploadDataGetLength = %d\n", WIFI_UploadDataGetLength());
					}
					WifiTxTimout = 2000;
				}
			}
			else if((WIFI_CT_READ() == 0) && (WifiTxTimout < 5000))//CT=0,一直被占用10s就超时失败退出
			{
				WIFI_UploadData_Get(txbuf, &len);
				WifiLockMeg.State = WIFI_TX_FAIL;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				WIFI_UploadData_Get(txbuf, &len);
				WifiLockMeg.State = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_ING://wifi数据发送中	
			if((WIFI_CT_READ() == 0) && (WifiTxTimout != 0))//CT=0,完成发送
			{
				WifiLockMeg.State = WIFI_TX_SUCCESS;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				WifiLockMeg.State = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_SUCCESS://wifi数据发送成功
			WIFI_PowerOff();
			if(0 != WIFI_UploadDataGetLength())
			{
				WifiLockMeg.State = WIFI_POWEROFF_DELAY;
				WifiTxTimout = 100;
			}
			else
			{
				WifiLockMeg.State = WIFI_TX_OVER;//wifi推送结束
				WifiTxTimout = 100;
			}
			break;
		case WIFI_TX_FAIL://wifi数据发送失败
			WIFI_PowerOff();
			if(0 != WIFI_UploadDataGetLength())
			{
				WifiLockMeg.State = WIFI_POWEROFF_DELAY;
				WifiTxTimout = 100;
			}
			else
			{
				WifiLockMeg.State = WIFI_TX_OVER;//wifi推送结束
				WifiTxTimout = 100;
			}
			break;
		case WIFI_POWEROFF_DELAY:
			if(WifiTxTimout == 0)//超时退出
			{
				WifiLockMeg.State = WIFI_TX_PREPARE;
			}
			break;
		default:
			break;
	}
	return WifiLockMeg.State;//发送状态
}
/*********************************************************************************************************************
* Function Name :  App_WIFI_PhotoUpload()
* Description   :  wifi初始化
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_PhotoUpload(void)
{
	WifiLockMeg.PhotoState = WIFI_TX_PREPARE;
}

/*********************************************************************************************************************
* Function Name :  WIFI_PhotoUploadOperate()
* Description   :  wifi初始化
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
#if defined FACE_FUNCTION_ON
uint8_t WIFI_PhotoUploadOperate(void)
{
	static uint8_t first_set_baut = 0;
	static uint8_t first_tx = 0;
	if(WifiLockMeg.PhotoState == WIFI_TX_OVER)
		return WifiLockMeg.PhotoState;
	static uint8_t wifi_tx_cn;
	switch(WifiLockMeg.PhotoState)
	{
		case WIFI_TX_PREPARE://wifi准备推送
			my_printf("PhotoUploadOperate WIFI_PowerOn\n");
			WIFI_PowerOn();
			WifiTxTimout = 1000;
			WifiLockMeg.PhotoState = WIFI_POWER_ON;
			WifiRxData.AckResult = 0x99;
			break;
		case WIFI_POWER_ON://wifi模块上电延时等待稳定
			if((WIFI_CT_READ() == 1) && (WifiTxTimout != 0) && (first_set_baut == 0))//CT=1,空闲，可以发送数据
			{
				first_set_baut = 1;
				WifiLockMeg.BaudrateSet = VALUE115200;
				App_WIFI_CommomTx(WIFI_CMD_SET_BAUDRATE);
				WifiTxTimout = 100;
			}
			if((first_set_baut == 1) && (WifiTxTimout < 50))//CT=0,完成发送
			{
				first_set_baut = 0;
				WifiLockMeg.PhotoState = WIFI_TX_START;
				WifiTxTimout = 1000;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				WifiLockMeg.PhotoState = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_START://开始发送wifi数据
			if(WifiLockMeg.DataLenth != 0)//人脸模块已获取该次上传图片的包内容
			{
				WifiRxData.AckResult = 0x99;
				//串口初始化
				App_WIFI_PhotoUartInit();
				first_tx = 1;
				WifiLockMeg.PhotoPackageNum++;
				memcpy(&WifiLockMeg.PhotoData,&FaceMsgType.Image.Data,(FaceMsgType.Image.MsgLen + 2));
				App_WIFI_CommomTx(WIFI_CMD_UPLOAD_IMAGE_MEG);
				my_printf("PhotoUploadOperate WIFI_TX_START\n");
				FaceMsgType.Image.MsgLen = 0;
				memset(FaceMsgType.Image.Data,0,1024);
				wifi_tx_cn = 0;
				WifiTxTimout = 4;
				WifiLockMeg.PhotoState = WIFI_TX_START_WAIT;
			}
			else if(WifiTxTimout == 0)//40ms未收到回复
			{
				WifiLockMeg.PhotoState = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_START_WAIT://开始发送wifi数据
			if(WifiTxTimout == 0)//10s未收到回复
			{
				WifiTxTimout = 1000;
				WifiLockMeg.PhotoState = WIFI_TX_ING;
			}
		break;
		case WIFI_TX_ING://wifi数据发送中			
			if(WifiRxData.AckResult == WIFI_ACK_UPLOAD_PHOTO_OK)//发送成功
			{
				if(WifiLockMeg.PhotoPackageNum == WifiLockMeg.PhotoPakageSum)//发完图片
				{
					my_printf("WIFI_ACK_UPLOAD_PHOTO_OK WifiLockMeg.PhotoState = WIFI_TX_SUCCESS;\n");
					WifiLockMeg.PhotoState = WIFI_TX_SUCCESS;
				}
				else//未发完图片
				{
					memcpy(&WifiLockMeg.PhotoData,&FaceMsgType.Image.Data,(FaceMsgType.Image.MsgLen + 2));
					my_printf("WIFI_ACK_UPLOAD_PHOTO_OK WifiLockMeg.PhotoState = WIFI_TX_START;\n");
					WifiLockMeg.PhotoState = WIFI_TX_START;
					WifiTxTimout = 800;
				}
			}
			else if(WifiTxTimout == 0)//1000ms未收到回复
			{
				WifiTxTimout = 100;
				WifiRxData.AckResult = 0x99;
				App_WIFI_CommomTx(WIFI_CMD_UPLOAD_IMAGE_MEG);
				if(wifi_tx_cn == 2)//3次重发失败后发送失败
				{
					WifiLockMeg.PhotoState = WIFI_TX_FAIL;
				}
				wifi_tx_cn++;
			}
			break;
		case WIFI_TX_SUCCESS://wifi数据发送成功
			my_printf("WIFI_TX_SUCCESS WIFI_PowerOff\n");
			WIFI_PowerOff();
			WifiLockMeg.PhotoState = WIFI_TX_OVER;//wifi推送结束
			break;
		case WIFI_TX_FAIL://wifi数据发送失败
			WIFI_PowerOff();
			WifiLockMeg.PhotoState = WIFI_TX_OVER;//wifi推送结束
			break;
		default:
			break;
	}
	return WifiLockMeg.State;
}
#endif
/*********************************************************************************************************************
* Function Name :  int8_t App_WIFI_ConfigOperate(uint8_t cmd, uint8_t *first_flag)
* Description   :  配网指令处理函数
* Para          :  cmd : WIFI_CMD_CONF_ROUTER / WIFI_CMD_CONF_SERVER
* Return        :  WifiLockMeg.Result : WIFI_CONFIG_FAIL / WIFI_CONFIG_ING / WIFI_CONFIG_SUCESS
*********************************************************************************************************************/
int8_t App_WIFI_ConfigThread(uint8_t cmd, uint8_t *first_flag)
{
	WifiLockMeg.Result = WIFI_CONFIG_ING;
	if(WifiLockMeg.State != WIFI_TX_OVER)//等待锁具开门等信息上传完成后再进行配网操作
		return WifiLockMeg.Result;
	static uint8_t step;
	if(!*first_flag)
	{
		*first_flag = 1;
		step = WIFI_TX_PREPARE;
	}
	switch(step)
	{
		case 0://wifi准备推送
			WIFI_PowerOn();
			WifiTxTimout = 50;
			step = WIFI_POWER_ON;
			break;
		case WIFI_POWER_ON://wifi模块上电延时等待稳定
			if(WifiTxTimout == 0)
			{
				step = WIFI_TX_START;
				WifiTxTimout = 1000;
			}
			break;
		case WIFI_TX_START://开始发送wifi数据
			if((WIFI_CT_READ() == 1) && (WifiTxTimout != 0))//CT=1,空闲，可以发送数据
			{
				step = WIFI_TX_ING;
				App_WIFI_CommomTx(cmd);
				WifiTxTimout = 2000;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_ING://wifi数据发送中
			if((WIFI_CT_READ() == 0) && (WifiTxTimout != 0)&& (WifiRxData.AckResult == WIFI_ACK_OK))//CT=0,完成发送
			{
				step = WIFI_TX_SUCCESS;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_SUCCESS://wifi数据发送成功
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi推送结束
			WifiLockMeg.Result = WIFI_CONFIG_SUCESS;
			break;
		case WIFI_TX_FAIL://wifi数据发送失败
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi推送结束
			WifiLockMeg.Result = WIFI_CONFIG_FAIL;
			break;
		default:
			break;
	}
	return WifiLockMeg.Result;
}

/*********************************************************************************************************************
* Function Name :  int8_t App_WIFI_ConfigOperate(uint8_t cmd, uint8_t *first_flag)
* Description   :  需等待ack指令的网络处理函数
* Para          :  cmd : WIFI_CMD_CONF_ROUTER / WIFI_CMD_CONF_SERVER
* Return        :  WifiLockMeg.Result : WIFI_CONFIG_FAIL / WIFI_CONFIG_ING / WIFI_CONFIG_SUCESS
*********************************************************************************************************************/
#ifdef WIFI_FUNCTION_ON
#if !defined(XM_CAM_FUNCTION_ON) && !defined(OB_CAM_FUNCTION_ON) && !defined(ST_CAM_FUNCTION_ON)
int8_t App_WIFI_ACK_Thread(uint8_t cmd, uint8_t *first_flag)
{
	WifiLockMeg.Result = WIFI_CONFIG_ING;
	if(WifiLockMeg.State != WIFI_TX_OVER)//等待锁具开门等信息上传完成后再进行配网操作
		return WifiLockMeg.Result;
	static uint8_t step;
	if(!*first_flag)
	{
		*first_flag = 1;
		step = WIFI_TX_PREPARE;
	}
	switch(step)
	{
		case 0://wifi准备推送
			WIFI_PowerOn();
			WifiTxTimout = 50;
			step = WIFI_POWER_ON;
			break;
		case WIFI_POWER_ON://wifi模块上电延时等待稳定
			if(WifiTxTimout == 0)
			{
				step = WIFI_TX_START;
				WifiTxTimout = 1000;
			}
			break;
		case WIFI_TX_START://开始发送wifi数据
			if((WIFI_CT_READ() == 1) && (WifiTxTimout != 0))//CT=1,空闲，可以发送数据
			{
				WifiRxData.AckResult = 0xFF;
				step = WIFI_TX_ING;
				App_WIFI_CommomTx(cmd);
				WifiTxTimout = 600;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_ING://wifi数据发送中
			if((WIFI_CT_READ() == 0) && (WifiTxTimout != 0) && (WifiRxData.AckResult != 0xFF))//CT=0,完成发送
			{
				step = WIFI_TX_SUCCESS;
			}
			if(WifiTxTimout == 0)//超时退出
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_SUCCESS://wifi数据发送成功
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi推送结束
			WifiLockMeg.Result = WIFI_CONFIG_SUCESS;
			break;
		case WIFI_TX_FAIL://wifi数据发送失败
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi推送结束
			WifiLockMeg.Result = WIFI_CONFIG_FAIL;
			break;
		default:
			break;
	}
	return WifiLockMeg.Result;
}
#endif
#endif
/*********************************************************************************************************************
* Function Name :  APP_WIFI_TxState()
* Description   :  wifi发送状态
* Para          :  无
* Return        :  发送完成：1； 发送中：-1；
*********************************************************************************************************************/
int8_t APP_WIFI_TxState(void)
{
	#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 
		if(OB_CAM_TxState == WIFI_TX_OVER)
		{
			return 1;
		}
	#elif defined XM_CAM_FUNCTION_ON
		if(CAM_GetQueueClearState()) //等待透传抓拍完成
		{
			return 1;
		}
	#else
		if((WifiLockMeg.State == WIFI_TX_OVER)
			&& (WIFI_UploadDataGetLength() == 0))
		{
			return 1;
		}
	#endif
		return -1;
}

/*********************************************************************************************************************
* Function Name :  App_WIFI_Init()
* Description   :  wifi初始化
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_Init(void)
{
	#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
	memset(WifiTxTemp.data, 0, 128);
	OB_CAM_TxState = WIFI_TX_OVER;
	#elif defined XM_CAM_FUNCTION_ON
	WifiLockMeg.State = WIFI_TX_OVER;
	WifiLockMeg.Result = WIFI_CONFIG_ING;
	WifiLockMeg.WifiFactoryTest = 0;
	#else
	WIFI_POW_OFF();
	WIFI_CT_INIT();
	WIFI_RT_INIT();
	WIFI_RT_POW_OFF();
	WifiLockMeg.State = WIFI_TX_OVER;
	WifiLockMeg.Result = WIFI_CONFIG_ING;
	WifiLockMeg.PhotoState = WIFI_TX_OVER;
	WifiLockMeg.WifiFactoryTest = 0;
	
	WifiRxTimout = 0;
	WifiTxTimout = 0;
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_WIFI_WakeupInit()
* Description   :  wifi唤醒初始化
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_WakeupInit(void)
{
	#if defined XM_CAM_FUNCTION_ON
	
	#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
	
	#else
	WIFI_POW_OFF();
	WIFI_CT_INIT();
	WIFI_RT_INIT();
	WIFI_RT_POW_OFF();
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_WIFI_ScanProcess()
* Description   :  wifi动作扫描进程
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_ScanProcess(void)
{
	#if defined XM_CAM_FUNCTION_ON
		CAM_ServerScan();
	#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
		FaceProScanProcess();
	#elif defined WIFI_FUNCTION_ON
		WIFI_ComRxHandle();//接受数据处理函数
		WIFI_DataUploadOperate();//发送数据处理函数
		#if defined FACE_FUNCTION_ON
		WIFI_PhotoUploadOperate();//发送图片数据处理函数
		#endif
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_WIFI_ScanProcess()
* Description   :  wifi动作扫描进程
* Para          :  无
* Return        :  none
*********************************************************************************************************************/
void APP_Wifi_Sleep()
{
	#if defined XM_CAM_FUNCTION_ON
	
	#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
	
	#else
	#if defined WIFI_FUNCTION_ON
	WIFI_PowerOff();
	WIFI_TX_POW_OFF();
    WIFI_RX_POW_OFF();
	WIFI_RT_POW_OFF();
	WIFI_CT_POW_OFF();
	#endif
	#endif
}

void App_WIFI_UartPowerOn(void)
{
	WIFI_POW_ON();
}
/*********************************************************************************************************************
* Function Name :  App_Wifi_TxMessage()
* Description   :  uart 发送数据
* Para   Input  :  p_data-待发送数据指针   length- 待发送数据长度
* Para   Output :  none
* Return        :  void
* ChangeList    :  新增 2022-01-18 by gushengchi
*********************************************************************************************************************/
void App_Wifi_TxMessage( uint8_t *p_data, uint16_t length )
{
	HAL_Uart_TxMessage( WIFI_UART_COM, p_data, length );
}

void App_WIFI_PhotoUartInit(void)//串口初始化
{
	UartCfg_S uartCfg={0};
	uartCfg.BaudRate = WIFI_SEND_PHOTOS_BAUDRATE;
	uartCfg.DataBit = DATA_8_BIT;
	uartCfg.StopBit = STOP_1_BIT;
	uartCfg.ParityType = PARITY_NONE;
	uartCfg.RxInerruptEn = INT_ENABLE;
	uartCfg.TxInerruptEn = INT_DISENABLE;
	HAL_Uart_ConfigInit( E_WIFI_UART, uartCfg );
}

/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
