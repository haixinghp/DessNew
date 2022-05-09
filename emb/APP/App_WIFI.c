/********************************************************************************************************************
 * @file:        App_WIFI.h
 * @author:      lixiqun
 * @version:     V01.00
 * @date:        2021-08-011
 * @Description: wifi�ӿڹ��ܺ����ļ�
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

#include "../server/Face.h"

#include "APP_FACE_PRO.h"
#include "APP_CAM.h"
#include "App_WIFI.h" 
#include "System.h" 
#include "SystemTim.h"
/*-------------------------------------------------�궨��-----------------------------------------------------------*/
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

/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/


/*-------------------------------------------------��������---------------------------------------------------------*/


/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/         
WifiLockUploadMeg_T WifiLockMeg; 
WifiRxMeg_T WifiRxData;
WIFI_TX_STATE_E OB_CAM_TxState;
/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/
static uint16_t WifiTxTimout;
static uint16_t WifiRxTimout;
typedef struct
{
	uint8_t UploadData[WIFI_UPLOAD_DATA_LENGTH_MAX];
	uint16_t UploadDataLength;
}WifiEmail_T;
typedef struct
{
	uint8_t HeadPtr;	//ͷָ��
	uint8_t TailPtr;	//βָ��
	WifiEmail_T pEmail[QUEUE_SIZE];//wifi���д洢����
}WifiQueue_T;

WifiQueue_T WifiUploadMsg = {0, 0, {0}};

/*-------------------------------------------------ȫ��������-----------------------------------------------------*/
WifiRxMeg_T WifiRxData;
WifiLockUploadMeg_T WifiLockMeg;
WifiTx WifiTxTemp;
/*-------------------------------------------------��������---------------------------------------------------------*/
static void App_Wifi_TxMessage( uint8_t *p_data, uint16_t length );

/*-------------------------------------------------��������---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  WIFI_UploadDataClear(void)
* Description   :  ����Ϣ����
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void WIFI_UploadDataClear(void)
{
	WifiUploadMsg.HeadPtr = WifiUploadMsg.TailPtr = 0;
} 

/*********************************************************************************************************************
* Function Name :  WIFI_UploadDataGetLength(void)
* Description   :  ��ȡ���г���
* Para          :  ��
* Return        :  DataLength
*********************************************************************************************************************/
extern uint8_t WIFI_UploadDataGetLength(void)
{
	uint8_t DataLength;
	if((WifiUploadMsg.HeadPtr >= QUEUE_SIZE)\
		|| (WifiUploadMsg.TailPtr >= QUEUE_SIZE))//�������鳤��������
	{
		WIFI_UploadDataClear();
	}
	if(WifiUploadMsg.TailPtr >= WifiUploadMsg.HeadPtr)//���βָ�����ͷָ�� �򳤶�����ȡ (βλ�� - ͷλ��)
	{
		DataLength = WifiUploadMsg.TailPtr - WifiUploadMsg.HeadPtr;
	}
	else //������ͷλ�ô���βλ�� ��������ݶ���д�� ͷָ�뿿����Ϣ���г��� βָ�뿿����Ϣ������� ����ȡ(βλ�� + ������� - ͷλ��)
	{
		DataLength = WifiUploadMsg.TailPtr + QUEUE_SIZE - WifiUploadMsg.HeadPtr;
	}
	return DataLength;
}

/*********************************************************************************************************************
* Function Name :  WIFI_UploadDataFill(uint8_t buf)
* Description   :  ��������������Ϣ ����0��ʾ������Ϣ���� �޷��ٴ洢
* Para          :  ��
* Return        :  0 : ������Ϣ����  1 : ���ݳɹ��������
*********************************************************************************************************************/
extern uint8_t WIFI_UploadDataFill(const uint8_t *buf, uint16_t length)
{
	my_printf("WIFI_UploadDataFill\n");
	uint16_t Length = 0;
	Length = WIFI_UploadDataGetLength();
	if(Length >= QUEUE_SIZE)//��Ϣ�������� 
	{
		return QUEUE_FULL;
	}
	WifiUploadMsg.pEmail[WifiUploadMsg.TailPtr].UploadDataLength = length;
	for(uint8_t i = 0; i < length; i++)
	{
		WifiUploadMsg.pEmail[WifiUploadMsg.TailPtr].UploadData[i] = buf[i];
	}
	WifiUploadMsg.TailPtr++;//βָ��λ������ ��ʾ���ݶ���һ�ֽ�
	if(WifiUploadMsg.TailPtr >= QUEUE_SIZE)//��βָ��λ�ó������г��� ����������
	{
		WifiUploadMsg.TailPtr = 0;
	}
	return QUEUE_OK;
}

/*********************************************************************************************************************
* Function Name :  WIFI_UploadData_Get(uint8_t *buf)
* Description   :  ��������������Ϣ ����0��ʾ������Ϣ���� �޷��ٴ洢
* Para          :  ��
* Return        :  0 : ������ϢΪ��  1 : ���ݳɹ�ȡ������
*********************************************************************************************************************/
extern uint8_t WIFI_UploadData_Get(uint8_t *buf, uint16_t *length)
{
	my_printf("WIFI_UploadData_Get\n");
	uint16_t Length = 0;
	Length = WIFI_UploadDataGetLength();
	if(0 == Length)//����Ϊ��
	{
		return QUEUE_EMPTY;
	}
	for(uint8_t i = 0; i < WifiUploadMsg.pEmail[WifiUploadMsg.HeadPtr].UploadDataLength; i++)
	{
		buf[i] = WifiUploadMsg.pEmail[WifiUploadMsg.HeadPtr].UploadData[i];
	}
	*length = WifiUploadMsg.pEmail[WifiUploadMsg.HeadPtr].UploadDataLength;
	WifiUploadMsg.HeadPtr++;//ͷָ��λ������ ��ʾ��������һ֡
	if(WifiUploadMsg.HeadPtr >= QUEUE_SIZE)
	{
		WifiUploadMsg.HeadPtr = 0;
	}
	return QUEUE_OK;
}
/*********************************************************************************************************************
* Function Name :  App_Key_Tim10Ms()
* Description   :  ������ض�ʱ��   10ms����һ��
* Para          :  ��
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
* Description   :  wifi�µ�
* Para          :  ��
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
* Description   :  wifi�ϵ�
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
void WIFI_PowerOn(void)
{
	my_printf("WIFI_PowerOn\n");
	WIFI_POW_ON();
	//���ڳ�ʼ��
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
* Description   :  ����У���
* Para          :  pdata-��У�������  len-���ݳ���
* Return        :  У���
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
* Description   :  ������
* Para          :  cmd
* Return        :  void
*********************************************************************************************************************/
uint8_t App_WIFI_CommomTx(uint8_t cmd)
{
	uint8_t txbuf[1024] = {0};
	uint16_t tp2, len;
	uint32_t under_bat_data = 0;
	uint32_t upper_bat_data = 0;
	txbuf[0] = 0xFE;//֡ͷ
	txbuf[1] = 0x02;//����  02-�޺���֡   03-�к���֡(��������Ӧ��)
	RTC_TimeUpdate(RTC_TIME_CLOCK_BCD);
	WifiLockMeg.DevType = 0x0b;
    uint8_t ret = 0;
	switch(cmd)
	{
		case WIFI_CMD_CONF_ROUTER://����·����(0XF6)
			txbuf[2] = cmd;//������
			tp2 = 0x62;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
		    for(uint16_t i = 0; i < sizeof(WifiLockMeg.Ssid); i++ )
			{
				txbuf[5 + i] = WifiLockMeg.Ssid[i];//·����SSID 33�ֽ�
			}
			for(uint16_t i = 0; i < sizeof(WifiLockMeg.Passwd); i++ )
			{
				txbuf[5 + 33 + i] = WifiLockMeg.Passwd[i];//·�������� 65�ֽ�  ����ȫ������
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			App_Wifi_TxMessage(txbuf, len + 7);
			WifiLockMeg.ConfigState = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_CONF_SERVER://���÷���������(0XF7)
			txbuf[2] = cmd;//������
			tp2 = 0x20;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
		    for(uint16_t i = 0; i < sizeof(WifiLockMeg.severname); i++ )
			{
				txbuf[5 + i] = WifiLockMeg.severname[i];//������������   30byte  ���㲹��
			}
			txbuf[ 5 + 30 ] = WifiLockMeg.portno[0];
			txbuf[ 5 + 31 ] = WifiLockMeg.portno[1];
			len = txbuf[3]*256 + txbuf[4];//У���
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			App_Wifi_TxMessage(txbuf, len + 7);
//			PUBLIC_PrintHex("WifiTxData",txbuf,len + 7);
			WifiLockMeg.ConfigState = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_CLOUD_DATA://������ȡ�ƶ�����(0X0A)
			txbuf[2] = cmd;//������
			tp2 = 0x11;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			txbuf[5] = 0x01;//��ȡ������Ϣ
			PUBLIC_GetMacAdd(&txbuf[6]);//MAC��ַ
			txbuf[12] = Rtc_Real_Time.year;//���ߵ�ǰʱ��
			txbuf[13] = Rtc_Real_Time.month;
			txbuf[14] = Rtc_Real_Time.day;
			txbuf[15] = Rtc_Real_Time.hour;
			txbuf[16] = Rtc_Real_Time.minuter;
			txbuf[17] = Rtc_Real_Time.second;//���ߵ�ǰʱ��
			upper_bat_data = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
			txbuf[18] = (uint8_t)(upper_bat_data  >> 8);//������ʾ
			txbuf[19] = (uint8_t)upper_bat_data;//������ʾ
			under_bat_data = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
			txbuf[20] = (uint8_t)((under_bat_data) >> 8);//������ʾ
			txbuf[21] = (uint8_t)(under_bat_data);//������ʾ
			len = txbuf[3]*256 + txbuf[4];//У���
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			#if defined XM_CAM_FUNCTION_ON //�����ϱ�
				ret = CAM_SendCommandStart(CAM_CMD_DATA_SEND, txbuf, len + 7);
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
				memcpy(WifiTxTemp.data, txbuf, len + 7);
				WifiTxTemp.length = len + 7;
			#else
				ret = WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			#endif
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_UPLOAD_UNLOCK_MEG://������Ϣ�ϴ�(0x04)
			txbuf[2] = cmd;//������
			if(WifiLockMeg.WifiFactoryTest == 1)
			{
				WifiLockMeg.WifiFactoryTest = 0;
				tp2 = 25;//���ݳ���
				txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
				txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
				PUBLIC_GetMacAdd(&txbuf[5]);//MAC��ַ
				memcpy(&txbuf[11],WifiLockMeg.WifiFactoryTestNum,19);
				len = txbuf[3]*256 + txbuf[4];//У���
				tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
				txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
				txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			}
			else
			{
				if(WifiLockMeg.UnlockMode <= 0x0C)//������ʽ��Ӧ��ʽ�����ο���û��Ϊ��
				{
					tp2 = 39;//���ݳ���
				}
				else
				{
					tp2 = 43;//���ݳ���
					txbuf[44] = (uint8_t)WifiLockMeg.UnlockWay2SuccessTime[0];//������ʽ��Ӧ��ʽ����ʷ�ɹ����������ο���û��Ϊ�գ�
					txbuf[45] = (uint8_t)WifiLockMeg.UnlockWay2SuccessTime[1];
					txbuf[46] = (uint8_t)WifiLockMeg.UnlockWay2FailTime[0];//������ʽ��Ӧ��ʽ����ʷʧ�ܴ��������ο���û��Ϊ�գ�
					txbuf[47] = (uint8_t)WifiLockMeg.UnlockWay2FailTime[1];
				}
				txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
				txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
				//����ʵʱ�ϴ����ݰ�
				txbuf[5] = WifiLockMeg.DevType;//�豸����
				PUBLIC_GetMacAdd(&txbuf[6]);//MAC��ַ
				#if defined LOCK_BODY_212_MOTOR
					txbuf[12] = LOCK_BODY_212;//������Ϣ
				#elif defined LOCK_BODY_216_MOTOR
					txbuf[12] = LOCK_BODY_216;//������Ϣ
				#elif defined LOCK_BODY_218_MOTOR
					txbuf[12] = LOCK_BODY_218;//������Ϣ
				#elif defined LOCK_BODY_AUTO_MOTOR
					if(LockConfigMode == LOCK_BODY_212)
					{
						txbuf[12] = LOCK_BODY_212;//������Ϣ
					}
					else if(LockConfigMode == LOCK_BODY_218)
					{
						txbuf[12] = LOCK_BODY_218;//������Ϣ
					}
				#endif
				txbuf[13] = WifiLockMeg.DevInfor.FactoryInfo;//OEM/ODM������Ϣ
				txbuf[14] = WifiLockMeg.DevInfor.FingerModuleInfo;//ָ��ģ�鳧����Ϣ
				txbuf[15] = WifiLockMeg.DevInfor.MotorUnlokCount;//������ţ���������
				txbuf[16] = WifiLockMeg.DevInfor.MotorUnlokTime[0];//����ʱ��������ʱ��
				txbuf[17] = WifiLockMeg.DevInfor.MotorUnlokTime[1];//����ʱ��������ʱ��
				txbuf[18] = WifiLockMeg.DevInfor.MotorUnlokCurrentVal[0];//���ŵ������ƽ��ֵ
				txbuf[19] = WifiLockMeg.DevInfor.MotorUnlokCurrentVal[1];//���ŵ������ƽ��ֵ
				for(uint8_t i= 0; i < 4; i++)
				{
					txbuf[20 + i] = WifiLockMeg.DevInfor.MotorUnlokAdc[i];//MAC��ַ
				}
				upper_bat_data = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
				txbuf[24] = (uint8_t)(upper_bat_data  >> 8);//������ʾ
				txbuf[25] = (uint8_t)upper_bat_data;//������ʾ
				txbuf[26] = WifiLockMeg.UnlockMode;//������ʽ
				txbuf[27] = WifiLockMeg.PageID.way1;//PageID
				txbuf[28] = WifiLockMeg.PageID.id1;//PageID
				txbuf[29] = WifiLockMeg.PageID.way2;//PageID
				txbuf[30] = WifiLockMeg.PageID.id2;//PageID
				txbuf[31] = WifiLockMeg.Attribute;//����
				txbuf[32] = Rtc_Real_Time.year;//����ʱ��
				txbuf[33] = Rtc_Real_Time.month;
				txbuf[34] = Rtc_Real_Time.day;
				txbuf[35] = Rtc_Real_Time.hour;
				txbuf[36] = Rtc_Real_Time.minuter;
				txbuf[37] = Rtc_Real_Time.second;//����ʱ��
				#if defined WIZARD_UPPER_BAT_ADC_ON && defined WIZARD_UNDER_BAT_ADC_ON
					under_bat_data = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
					tp2 = 35;//���ݳ���
					txbuf[38] = (uint8_t)((under_bat_data) >> 8);//������ʾ
					txbuf[39] = (uint8_t)(under_bat_data);//������ʾ
				#endif
				txbuf[40] = (uint8_t)WifiLockMeg.UnlockWay1SuccessTime[0];//������ʽ��Ӧ��ʽһ��ʷ�ɹ�����
				txbuf[41] = (uint8_t)WifiLockMeg.UnlockWay1SuccessTime[1];
				txbuf[42] = (uint8_t)WifiLockMeg.UnlockWay1FailTime[0];//������ʽ��Ӧ��ʽһ��ʷʧ�ܴ���
				txbuf[43] = (uint8_t)WifiLockMeg.UnlockWay1FailTime[1];
				len = txbuf[3]*256 + txbuf[4];//У���
				tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
				txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
				txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			}
			#if defined XM_CAM_FUNCTION_ON //�����ϱ�
				ret = CAM_SendCommandStart(CAM_CMD_DATA_SEND, txbuf, len + 7);
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
				memcpy(WifiTxTemp.data, txbuf, len + 7);
				WifiTxTemp.length = len + 7;
				if(WifiLockMeg.UnlockMode == SERVER_TEST)//����wifi������û�д����ֱ�ӿɷ�
				{
				
				}
				else//���Ϳ�����Ϣ
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
		case WIFI_CMD_UPLOAD_ERROR_MEG://������Ϣ�ϴ�(0x07)
			txbuf[2] = cmd;//������
			#if defined WIZARD_UPPER_BAT_ADC_ON && defined WIZARD_UNDER_BAT_ADC_ON
				under_bat_data = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
				tp2 = 24;//���ݳ���
				txbuf[27] = (uint8_t)((under_bat_data) >> 8);//������ʾ
				txbuf[28] = (uint8_t)(under_bat_data);//������ʾ
			#else
				tp2 = 22;//���ݳ���
			#endif
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
		    txbuf[5] = WifiLockMeg.DevType;//�豸����
			PUBLIC_GetMacAdd(&txbuf[6]);//MAC��ַ
			txbuf[12] = Rtc_Real_Time.year;//����ʱ��
			txbuf[13] = Rtc_Real_Time.month;
			txbuf[14] = Rtc_Real_Time.day;
			txbuf[15] = Rtc_Real_Time.hour;
			txbuf[16] = Rtc_Real_Time.minuter;
			txbuf[17] = Rtc_Real_Time.second;//����ʱ��
			txbuf[18] = WifiLockMeg.DevInfor.Reserve;//Ԥ����0
			txbuf[19] = WifiLockMeg.DevInfor.Reserve;//Ԥ����0
			txbuf[20] = WifiLockMeg.DevInfor.Reserve;//Ԥ����0
			txbuf[21] = WifiLockMeg.DevInfor.Reserve;//Ԥ����0
			txbuf[22] = WifiLockMeg.DevInfor.Reserve;//Ԥ����0
			txbuf[23] = WifiLockMeg.DevInfor.FactoryInfo;//OEM/ODM������Ϣ
			upper_bat_data = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
			txbuf[24] = (uint8_t)(upper_bat_data >> 8);//������ʾ
			txbuf[25] = (uint8_t)upper_bat_data;//������ʾ
			txbuf[26] = WifiLockMeg.AlarmMeg;//������Ϣ
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			#ifdef XM_CAM_FUNCTION_ON  //����͸��
				ret = CAM_SendCommandStart(CAM_CMD_DATA_SEND, txbuf, len + 7);
				CAM_SetCapture(WifiLockMeg.AlarmMeg); //ִ��ץ��
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
				memcpy(WifiTxTemp.data, txbuf, len + 7);
				WifiTxTemp.length = len + 7;
				if((WifiLockMeg.AlarmMeg == LOCKPICKING) || (WifiLockMeg.AlarmMeg == FORBID_TRY) || (WifiLockMeg.AlarmMeg == EPLOYMENT) || (WifiLockMeg.AlarmMeg == DEFENSE))//���������澯ֱ�ӿɷ�
				{
				
				}
				else//���Ϳ�����Ϣ�Ӷ�����ȡ�����ٷ���
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
		case WIFI_CMD_UPLOAD_IMAGE_MEG://ͼƬ�ϴ�(0x09)
			txbuf[1] = 0x03;//03�����к�������
//			if(WifiLockMeg.PhotoPakageSum == WifiLockMeg.PhotoPackageNum)
//			{
//				txbuf[1] = 0x02;//02�����޺�������
//			}
			txbuf[2] = cmd;//������
			tp2 = 26 + WifiLockMeg.DataLenth + 2;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			txbuf[5] =	WifiLockMeg.PhotoPakageSum;//����
			txbuf[6] =	WifiLockMeg.PhotoPackageNum;//����
		    txbuf[7] = WifiLockMeg.DevType;//�豸����
			PUBLIC_GetMacAdd(&txbuf[8]);//MAC��ַ
			txbuf[14] = WifiLockMeg.PhotoUploadType;//ͼƬ����
			memcpy(&txbuf[15],WifiLockMeg.PhotoAES,16);
			memcpy(&txbuf[31], WifiLockMeg.PhotoData, (WifiLockMeg.DataLenth + 2));
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			my_printf("App_Wifi_TxMessage(txbuf, len + 7) len + 7 = %d\n", len + 7);
			App_Wifi_TxMessage(txbuf, len + 7);
//			PUBLIC_PrintHex("WifiTxData",txbuf,len + 7);
//			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_SET_OTA_UPDATA://OTA����(0x10)
			txbuf[2] = cmd;//������
			tp2 = WifiLockMeg.DataLenth;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
		    for(uint16_t i = 0; i < WifiLockMeg.DataLenth; i++ )
			{
				txbuf[5 + i] = WifiLockMeg.OtaUpdateDataAddr[i];//��������
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_OTA_UPDATA://ģ��������ѯOTA����(0x11)
			txbuf[2] = cmd;//������
			tp2 = WifiLockMeg.DataLenth;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
		    for(uint16_t i = 0; i < WifiLockMeg.DataLenth; i++ )
			{
				txbuf[5 + i] = WifiLockMeg.OtaQueryUpdateData[i];//��������
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_SET_BAUDRATE://ͨѶ����������(0xFA)
			txbuf[2] = cmd;//������
			tp2 = 1;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			txbuf[5] = WifiLockMeg.BaudrateSet;//ͨѶ������(33��ʾ19200��55��ʾ115200(Ĭ��19200),EE��ʾ57600,66��ʾ9600)
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			App_Wifi_TxMessage(txbuf, len + 7);
			PUBLIC_PrintHex("WifiTxData",txbuf,len + 7);
//			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
//			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_MAC_ADDR:
			txbuf[2] = cmd;//������
			tp2 = 1;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_FACTORY_CHECK://����ɨ��·����(0XFC)
			txbuf[2] = cmd;//������
			tp2 = 33;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			for(uint8_t i = 0; i < 33; i++)
			{
				txbuf[5 + i] = WifiLockMeg.Ssid[i];//·����SSID
			}
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			App_Wifi_TxMessage(txbuf, len + 7);
//			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
//			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_SOFT_VERSION://��ѯWIFIģ�������汾��(0xFD)
			txbuf[2] = cmd;//������
			tp2 = 1;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
			WIFI_UploadDataFill((const uint8_t *)txbuf, len + 7);
			WifiLockMeg.State = WIFI_TX_PREPARE;
			break;
		case WIFI_CMD_QUERY_PASSWORD://��ѯ��ǰģ����PSK����(0xFE)
			txbuf[2] = cmd;//������
			tp2 = 1;//���ݳ���
			txbuf[3] = (uint8_t)(tp2 >> 8);//���ݳ���
			txbuf[4] = (uint8_t)(tp2 >> 0);//���ݳ���
			len = txbuf[3]*256 + txbuf[4];
			tp2 = WIFI_GetCheckSumWord( (const uint8_t *)&txbuf[1], len + 4 );
			txbuf[5 + len] = (uint8_t)( tp2 >> 8 );//У���
			txbuf[6 + len] = (uint8_t)( tp2 >> 0 );//У���
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
* Description   :  �������ݴ���
* Para          :  para-���յ�������
* Return        :  0:δ���յ�����
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

	if(WifiRxTimout == 0)//1s��δ�ܽ��յ��µ�������Ϊ���ݴ������
	{
        step = 0;
	}
 
	while(UART_SUCCESS == HAL_Uart_PopByteFromQueue( WIFI_UART_COM, &rxdata ))
	{
		WifiRxTimout = 100;
		if( step == 0 )    //֡ͷ
		{
			if(( rxdata != 0xFE ) && ( rxdata != 0x1E ))
				step = 0;
			else
				rxbuf[ step++ ] = rxdata;
		}
		else if( step == 1 ) //����
		{
			if(( rxdata != 0x02 ) && ( rxdata != 0x03 ))
				step = 0;
			else
				rxbuf[ step++ ] = rxdata;
		}
		else if( step == 2 ) //������
		{
			rxbuf[ step++ ] = rxdata;
		}
		else if( (step == 3)||(step == 4) ) //���ݳ���
		{
			rxbuf[ step++ ] = rxdata;
		}
		else if( step >= 600 )
		{
			step = 0;
		}
		else
		{
			if( step >= (rxbuf[3]*256+ rxbuf[4] + 6) )  //���ݽ������
			{
				rxbuf[ step ] = rxdata;
				len = rxbuf[3]*256+ rxbuf[4];
				step = 0;
				for(uint16_t i=0; i < len + 4; i++)
					sumcheck += rxbuf[ 1 + i ];
				if( sumcheck == rxbuf[ len + 5 ]*256 + rxbuf[ len + 6 ] ) //У��ͨ��
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
							//�ظ����ݻ�δ����
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
* Description   :  ���ڳ�ʼ��
* Para          :  ��
* Return        :  void
*********************************************************************************************************************/
static void App_WIFI_UartInit(void)//���ڳ�ʼ��
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
* Description   :  wifi�ϴ����ݷ��ʹ���
* Para          :  ��
* Return        :  0 : wifi���ϴ�����
*********************************************************************************************************************/
uint8_t WIFI_DataUploadOperate(void)
{
	uint8_t txbuf[WIFI_UPLOAD_DATA_LENGTH_MAX] = {0};
	uint16_t len;
	if(APP_WIFI_TxState() == 1)
		return 0;
	if(((HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_FACE_UART) || 
		(HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_FINGER_UART) ||
		(HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_CAMERA_UART))//���ڱ�ռ��
		&& (WifiLockMeg.State < WIFI_TX_START))//�ϵ�δ����ʱ���ڱ������ط�
	{
		WifiLockMeg.State = WIFI_TX_PREPARE;
		WIFI_POW_OFF();
		return WifiLockMeg.State;
	}
	
	switch(WifiLockMeg.State)
		
	{
		case WIFI_TX_PREPARE://wifi׼������
			WIFI_PowerOn();
			WifiTxTimout = 2;
			WifiLockMeg.State = WIFI_POWER_ON;
			break;
		case WIFI_POWER_ON://wifiģ���ϵ���ʱ�ȴ��ȶ�
			if(WifiTxTimout == 0)
			{
				WifiLockMeg.State = WIFI_TX_START;
				WifiTxTimout = 6000;
			}
			break;
		case WIFI_TX_START://��ʼ����wifi����
			if((WIFI_CT_READ() == 1))//CT=1,���У����Է�������
			{
				if((HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) != E_WIFI_UART)
					&& HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_NO_UART)//����wifi����ǰȷ�������е�wifi
				{
					App_WIFI_UartInit();
				}
				if(HAL_Uart_GetCurDeviceType( WIFI_UART_COM ) == E_WIFI_UART)//����wifi����ǰȷ�������е�wifi
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
			else if((WIFI_CT_READ() == 0) && (WifiTxTimout < 5000))//CT=0,һֱ��ռ��10s�ͳ�ʱʧ���˳�
			{
				WIFI_UploadData_Get(txbuf, &len);
				WifiLockMeg.State = WIFI_TX_FAIL;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				WIFI_UploadData_Get(txbuf, &len);
				WifiLockMeg.State = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_ING://wifi���ݷ�����	
			if((WIFI_CT_READ() == 0) && (WifiTxTimout != 0))//CT=0,��ɷ���
			{
				WifiLockMeg.State = WIFI_TX_SUCCESS;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				WifiLockMeg.State = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_SUCCESS://wifi���ݷ��ͳɹ�
			WIFI_PowerOff();
			if(0 != WIFI_UploadDataGetLength())
			{
				WifiLockMeg.State = WIFI_POWEROFF_DELAY;
				WifiTxTimout = 100;
			}
			else
			{
				WifiLockMeg.State = WIFI_TX_OVER;//wifi���ͽ���
				WifiTxTimout = 100;
			}
			break;
		case WIFI_TX_FAIL://wifi���ݷ���ʧ��
			WIFI_PowerOff();
			if(0 != WIFI_UploadDataGetLength())
			{
				WifiLockMeg.State = WIFI_POWEROFF_DELAY;
				WifiTxTimout = 100;
			}
			else
			{
				WifiLockMeg.State = WIFI_TX_OVER;//wifi���ͽ���
				WifiTxTimout = 100;
			}
			break;
		case WIFI_POWEROFF_DELAY:
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				WifiLockMeg.State = WIFI_TX_PREPARE;
			}
			break;
		default:
			break;
	}
	return WifiLockMeg.State;//����״̬
}
/*********************************************************************************************************************
* Function Name :  App_WIFI_PhotoUpload()
* Description   :  wifi��ʼ��
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_PhotoUpload(void)
{
	WifiLockMeg.PhotoState = WIFI_TX_PREPARE;
}

/*********************************************************************************************************************
* Function Name :  WIFI_PhotoUploadOperate()
* Description   :  wifi��ʼ��
* Para          :  ��
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
		case WIFI_TX_PREPARE://wifi׼������
			my_printf("PhotoUploadOperate WIFI_PowerOn\n");
			WIFI_PowerOn();
			WifiTxTimout = 1000;
			WifiLockMeg.PhotoState = WIFI_POWER_ON;
			WifiRxData.AckResult = 0x99;
			break;
		case WIFI_POWER_ON://wifiģ���ϵ���ʱ�ȴ��ȶ�
			if((WIFI_CT_READ() == 1) && (WifiTxTimout != 0) && (first_set_baut == 0))//CT=1,���У����Է�������
			{
				first_set_baut = 1;
				WifiLockMeg.BaudrateSet = VALUE115200;
				App_WIFI_CommomTx(WIFI_CMD_SET_BAUDRATE);
				WifiTxTimout = 100;
			}
			if((first_set_baut == 1) && (WifiTxTimout < 50))//CT=0,��ɷ���
			{
				first_set_baut = 0;
				WifiLockMeg.PhotoState = WIFI_TX_START;
				WifiTxTimout = 1000;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				WifiLockMeg.PhotoState = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_START://��ʼ����wifi����
			if(WifiLockMeg.DataLenth != 0)//����ģ���ѻ�ȡ�ô��ϴ�ͼƬ�İ�����
			{
				WifiRxData.AckResult = 0x99;
				//���ڳ�ʼ��
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
			else if(WifiTxTimout == 0)//40msδ�յ��ظ�
			{
				WifiLockMeg.PhotoState = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_START_WAIT://��ʼ����wifi����
			if(WifiTxTimout == 0)//10sδ�յ��ظ�
			{
				WifiTxTimout = 1000;
				WifiLockMeg.PhotoState = WIFI_TX_ING;
			}
		break;
		case WIFI_TX_ING://wifi���ݷ�����			
			if(WifiRxData.AckResult == WIFI_ACK_UPLOAD_PHOTO_OK)//���ͳɹ�
			{
				if(WifiLockMeg.PhotoPackageNum == WifiLockMeg.PhotoPakageSum)//����ͼƬ
				{
					my_printf("WIFI_ACK_UPLOAD_PHOTO_OK WifiLockMeg.PhotoState = WIFI_TX_SUCCESS;\n");
					WifiLockMeg.PhotoState = WIFI_TX_SUCCESS;
				}
				else//δ����ͼƬ
				{
					memcpy(&WifiLockMeg.PhotoData,&FaceMsgType.Image.Data,(FaceMsgType.Image.MsgLen + 2));
					my_printf("WIFI_ACK_UPLOAD_PHOTO_OK WifiLockMeg.PhotoState = WIFI_TX_START;\n");
					WifiLockMeg.PhotoState = WIFI_TX_START;
					WifiTxTimout = 800;
				}
			}
			else if(WifiTxTimout == 0)//1000msδ�յ��ظ�
			{
				WifiTxTimout = 100;
				WifiRxData.AckResult = 0x99;
				App_WIFI_CommomTx(WIFI_CMD_UPLOAD_IMAGE_MEG);
				if(wifi_tx_cn == 2)//3���ط�ʧ�ܺ���ʧ��
				{
					WifiLockMeg.PhotoState = WIFI_TX_FAIL;
				}
				wifi_tx_cn++;
			}
			break;
		case WIFI_TX_SUCCESS://wifi���ݷ��ͳɹ�
			my_printf("WIFI_TX_SUCCESS WIFI_PowerOff\n");
			WIFI_PowerOff();
			WifiLockMeg.PhotoState = WIFI_TX_OVER;//wifi���ͽ���
			break;
		case WIFI_TX_FAIL://wifi���ݷ���ʧ��
			WIFI_PowerOff();
			WifiLockMeg.PhotoState = WIFI_TX_OVER;//wifi���ͽ���
			break;
		default:
			break;
	}
	return WifiLockMeg.State;
}
#endif
/*********************************************************************************************************************
* Function Name :  int8_t App_WIFI_ConfigOperate(uint8_t cmd, uint8_t *first_flag)
* Description   :  ����ָ�������
* Para          :  cmd : WIFI_CMD_CONF_ROUTER / WIFI_CMD_CONF_SERVER
* Return        :  WifiLockMeg.Result : WIFI_CONFIG_FAIL / WIFI_CONFIG_ING / WIFI_CONFIG_SUCESS
*********************************************************************************************************************/
int8_t App_WIFI_ConfigThread(uint8_t cmd, uint8_t *first_flag)
{
	WifiLockMeg.Result = WIFI_CONFIG_ING;
	if(WifiLockMeg.State != WIFI_TX_OVER)//�ȴ����߿��ŵ���Ϣ�ϴ���ɺ��ٽ�����������
		return WifiLockMeg.Result;
	static uint8_t step;
	if(!*first_flag)
	{
		*first_flag = 1;
		step = WIFI_TX_PREPARE;
	}
	switch(step)
	{
		case 0://wifi׼������
			WIFI_PowerOn();
			WifiTxTimout = 50;
			step = WIFI_POWER_ON;
			break;
		case WIFI_POWER_ON://wifiģ���ϵ���ʱ�ȴ��ȶ�
			if(WifiTxTimout == 0)
			{
				step = WIFI_TX_START;
				WifiTxTimout = 1000;
			}
			break;
		case WIFI_TX_START://��ʼ����wifi����
			if((WIFI_CT_READ() == 1) && (WifiTxTimout != 0))//CT=1,���У����Է�������
			{
				step = WIFI_TX_ING;
				App_WIFI_CommomTx(cmd);
				WifiTxTimout = 2000;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_ING://wifi���ݷ�����
			if((WIFI_CT_READ() == 0) && (WifiTxTimout != 0)&& (WifiRxData.AckResult == WIFI_ACK_OK))//CT=0,��ɷ���
			{
				step = WIFI_TX_SUCCESS;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_SUCCESS://wifi���ݷ��ͳɹ�
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi���ͽ���
			WifiLockMeg.Result = WIFI_CONFIG_SUCESS;
			break;
		case WIFI_TX_FAIL://wifi���ݷ���ʧ��
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi���ͽ���
			WifiLockMeg.Result = WIFI_CONFIG_FAIL;
			break;
		default:
			break;
	}
	return WifiLockMeg.Result;
}

/*********************************************************************************************************************
* Function Name :  int8_t App_WIFI_ConfigOperate(uint8_t cmd, uint8_t *first_flag)
* Description   :  ��ȴ�ackָ������紦������
* Para          :  cmd : WIFI_CMD_CONF_ROUTER / WIFI_CMD_CONF_SERVER
* Return        :  WifiLockMeg.Result : WIFI_CONFIG_FAIL / WIFI_CONFIG_ING / WIFI_CONFIG_SUCESS
*********************************************************************************************************************/
#ifdef WIFI_FUNCTION_ON
#if !defined(XM_CAM_FUNCTION_ON) && !defined(OB_CAM_FUNCTION_ON) && !defined(ST_CAM_FUNCTION_ON)
int8_t App_WIFI_ACK_Thread(uint8_t cmd, uint8_t *first_flag)
{
	WifiLockMeg.Result = WIFI_CONFIG_ING;
	if(WifiLockMeg.State != WIFI_TX_OVER)//�ȴ����߿��ŵ���Ϣ�ϴ���ɺ��ٽ�����������
		return WifiLockMeg.Result;
	static uint8_t step;
	if(!*first_flag)
	{
		*first_flag = 1;
		step = WIFI_TX_PREPARE;
	}
	switch(step)
	{
		case 0://wifi׼������
			WIFI_PowerOn();
			WifiTxTimout = 50;
			step = WIFI_POWER_ON;
			break;
		case WIFI_POWER_ON://wifiģ���ϵ���ʱ�ȴ��ȶ�
			if(WifiTxTimout == 0)
			{
				step = WIFI_TX_START;
				WifiTxTimout = 1000;
			}
			break;
		case WIFI_TX_START://��ʼ����wifi����
			if((WIFI_CT_READ() == 1) && (WifiTxTimout != 0))//CT=1,���У����Է�������
			{
				WifiRxData.AckResult = 0xFF;
				step = WIFI_TX_ING;
				App_WIFI_CommomTx(cmd);
				WifiTxTimout = 600;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_ING://wifi���ݷ�����
			if((WIFI_CT_READ() == 0) && (WifiTxTimout != 0) && (WifiRxData.AckResult != 0xFF))//CT=0,��ɷ���
			{
				step = WIFI_TX_SUCCESS;
			}
			if(WifiTxTimout == 0)//��ʱ�˳�
			{
				step = WIFI_TX_FAIL;
			}
			break;
		case WIFI_TX_SUCCESS://wifi���ݷ��ͳɹ�
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi���ͽ���
			WifiLockMeg.Result = WIFI_CONFIG_SUCESS;
			break;
		case WIFI_TX_FAIL://wifi���ݷ���ʧ��
			WIFI_PowerOff();
			step = WIFI_TX_PREPARE;//wifi���ͽ���
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
* Description   :  wifi����״̬
* Para          :  ��
* Return        :  ������ɣ�1�� �����У�-1��
*********************************************************************************************************************/
int8_t APP_WIFI_TxState(void)
{
	#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 
		if(OB_CAM_TxState == WIFI_TX_OVER)
		{
			return 1;
		}
	#elif defined XM_CAM_FUNCTION_ON
		if(CAM_GetQueueClearState()) //�ȴ�͸��ץ�����
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
* Description   :  wifi��ʼ��
* Para          :  ��
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
* Description   :  wifi���ѳ�ʼ��
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_WakeupInit(void)
{
	#if defined XM_CAM_FUNCTION_ON
	
	#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
	
	#else
	WIFI_POW_OFF();
	WIFI_CT_INIT();
	WIFI_RT_INIT();
	WIFI_RT_POW_OFF();
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_WIFI_ScanProcess()
* Description   :  wifi����ɨ�����
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void App_WIFI_ScanProcess(void)
{
	#if defined XM_CAM_FUNCTION_ON
		CAM_ServerScan();
	#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
		FaceProScanProcess();
	#elif defined WIFI_FUNCTION_ON
		WIFI_ComRxHandle();//�������ݴ�������
		WIFI_DataUploadOperate();//�������ݴ�������
		#if defined FACE_FUNCTION_ON
		WIFI_PhotoUploadOperate();//����ͼƬ���ݴ�������
		#endif
	#endif
}

/*********************************************************************************************************************
* Function Name :  App_WIFI_ScanProcess()
* Description   :  wifi����ɨ�����
* Para          :  ��
* Return        :  none
*********************************************************************************************************************/
void APP_Wifi_Sleep()
{
	#if defined XM_CAM_FUNCTION_ON
	
	#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //����������ʽ
	
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
* Description   :  uart ��������
* Para   Input  :  p_data-����������ָ��   length- ���������ݳ���
* Para   Output :  none
* Return        :  void
* ChangeList    :  ���� 2022-01-18 by gushengchi
*********************************************************************************************************************/
void App_Wifi_TxMessage( uint8_t *p_data, uint16_t length )
{
	HAL_Uart_TxMessage( WIFI_UART_COM, p_data, length );
}

void App_WIFI_PhotoUartInit(void)//���ڳ�ʼ��
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