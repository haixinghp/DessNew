/********************************************************************************************************************
 * @file:        APP_BLE.c
 * @author:      dengyehao
 * @version:     V0.1
 * @date:        2021-08-13
 * @Description: �ֻ���������
 * @ChangeList:  ����
*********************************************************************************************************************/
  
/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
#include "string.h"
#include "Public.h" 
#include "APP_BLE.h" 
#include "APP_FACE.h" 
#include "APP_FACE_PRO.h" 
#include "APP_Finger.h" 
#include "APP_GUI.h" 
#include "App_PWD.h" 
#include "App_LED.h" 
#include "App_WIFI.h" 
#include "App_Update.h" 
#include "SystemInit.h" 
#include "System.h" 
#include "LockConfig.h" 
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "..\HAL\HAL_Voice\HAL_Voice.h"
#include "..\HAL\HAL_ADC\HAL_ADC.h"
#include "APP_SmartKey.h" 
#include "APP_ID2.h"
#include "Encrypto.h"
#include "App_Touch.h" 
#include "App_Key.h" 
#include "APP_CAM.h"
#include "App_HumanSensor.h"
#include "App_NB.h"
/*-------------------------------------------------�궨��-----------------------------------------------------------*/ 

 //Э������
#define BLE_APP_HEAD            0xFE   //ͷ
#define BLE_APP_TYPE_CMD		0x01   //����
#define BLE_APP_TYPE_RSP		0x09   //����Ӧ��
//�˺ż��ܸ���
#define AES_ROOT_KEY  5      //Root_key�±�
/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/
typedef enum
{
	E_CMD_DEFAULT,   
    E_CMD_UNREGIST_DEVICE,    //ע���豸
	E_CMD_ROUTER_CFG,         //·��������
    E_CMD_SERVER_CFG,         //����������
	E_CMD_NEAR_SENSE_CFG,     //�ӽ���Ӧ����
    E_CMD_SERVER_TEST,    	  //����������
	E_CMD_DELETE_FINGER,      //ɾ��ָ��
	E_CMD_EXCHANGE_FINGER,    //�޸�ָ��
	E_CMD_CAM_GET_SN,		  //��ȡè�����к�
	E_CMD_REG_PIN,             //ע�����
	E_CMD_CAM_LINKKEY,        //�·���������Կ
	E_CMD_CAM_POWER_SAVING,   //ʡ��ģʽ
	E_CMD_CAM_GET_IP,         //��ȡè��IP
	E_CMD_FACE_PRO_OTA,       //è������
	E_CMD_WIFI_TEST,
	E_CMD_WIFI_FACTORY_TEST,
	E_CMD_GET_WIFI_SIGNAL_INTENSITY, //��ȡWIFI�ź�ǿ��
	E_CMD_SET_FOV_PARAM,     // ����FOV �ӳ���
	E_CMD_WIFI_MAIN_SW_POW_OFF,
	E_CMD_BLE_LOCK_CASE      //�¼���¼��ȡ
	
}BLE_RX_CMD_E;  //����������Ҫ���������� ����������Ҫ�ֲ������

static BLE_RX_CMD_E  BleRxCmdHandlePro = E_CMD_DEFAULT;
/*-------------------------------------------------��������---------------------------------------------------------*/
//����
const uint8_t  Root_key[16][8]={
{0x18,0xd1,0x84,0x4a,0xf3,0x60,0x94,0xcc},/*( 1)*/   
{0x22,0x06,0x5f,0x9e,0x9d,0xc8,0x9b,0x5c},/*( 2)*/
{0x30,0x07,0x19,0xb0,0xb2,0x2d,0x98,0x0b},/*( 3)*/
{0x6b,0xec,0xc9,0x58,0xa1,0x63,0x9a,0x7b},/*( 4)*/
{0x7e,0x43,0x01,0x95,0x01,0x6c,0x92,0xaa},/*( 5)*/
{0x91,0xd6,0x22,0x84,0xad,0x7d,0x97,0x6a},/*( 6)*/
{0xc8,0x62,0xf9,0xab,0x0f,0xba,0x94,0x49},/*( 7)*/
{0xa1,0x08,0xf0,0xf8,0x93,0x93,0x81,0x89},/*( 8)*/
{0xb1,0x93,0x88,0x88,0x51,0x83,0x81,0x28},/*( 9)*/
{0x7e,0x77,0x72,0xc7,0x8d,0xaf,0x82,0x18},/*(10)*/
{0x22,0x91,0x58,0x0d,0x6f,0xb4,0x74,0x27},/*(11)*/
{0x17,0x73,0x0c,0xb1,0x75,0x2f,0x7f,0x27},/*(12)*/
{0x38,0x68,0x7b,0xb6,0xbe,0xb7,0x75,0xa6},/*(13)*/
{0x41,0x5a,0x20,0x81,0x95,0x13,0x7c,0xd6},/*(14)*/
{0x5d,0x90,0xcd,0xd2,0x61,0x84,0x7a,0x13},/*(15)*/
{0x76,0x80,0x1a,0x79,0xe2,0xf5,0x7d,0x43},/*(16)*/
};

/*-------------------------------------------------ȫ�ֱ�������-----------------------------------------------------*/         


/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/
APP_BLE_TYPE  AppBleType;       //BLE�ṹ��

static uint8_t  ComDataByte1  = 0;
static uint16_t ComDataByte2 = 0;

static uint8_t  BleHandleProStep  = 0;
/*-------------------------------------------------��������---------------------------------------------------------*/
 

/*-------------------------------------------------��������---------------------------------------------------------*/
/*********************************************************************************************************************
* Function Name :  APP_BleInit()
* Description   :  BLE ����Init�ӿڷ�װ
* Para          :  DRV_BLE_ADV_MODE mode  �㲥������
				   DRV_BLE_ADV_FLAGS flags  �㲥����	
* Return        :  void
*********************************************************************************************************************/
void APP_BleInit(DRV_BLE_ADV_MODE mode ,DRV_BLE_ADV_FLAGS flags)
{
    DRV_BleInit(mode, flags, APP_UpdateDataHandler);
    return;
}

/***************************************************************************************
**������:       APP_BleDelPhone
**��������:     ɾ���ֻ���
**�������:      
**�������:     
**��ע:        
****************************************************************************************/
void APP_BleDelPhone(void )
{
	uint8_t temp[13];
	memset(temp,0xff,13);
	HAL_EEPROM_WriteBytes(MEM_PHONE_START,temp,13); 
}

/***************************************************************************************
**������:       BLE_Respond_Ready_DATA
**��������:     ��������������֡
**�������:     
**�������:     
**��ע:         
****************************************************************************************/
uint8_t AppBleRespondData (uint8_t cmd)
{
    uint16_t crc=0;

    AppBleType.Respond.TxDataBuf[L_HEAD] = BLE_APP_HEAD; //ͷ
    AppBleType.Respond.TxDataBuf[L_TYPE] = BLE_APP_TYPE_RSP; //Ӧ��
    AppBleType.Respond.TxDataBuf[L_CMD]  = cmd;       //����
    AppBleType.Respond.TxDataBuf[L_LENH] = AppBleType.Respond.TxLen>>8;  //����
	AppBleType.Respond.TxDataBuf[L_LENL] = AppBleType.Respond.TxLen&0xff;  //����
	//ID2����
	if(App_Id2_Data.ID2_EN)
	{
		if(BLE_LOCK_CASE_LOG!=cmd) //�¼���¼����̫�࣬ʹ������
		{
			PUBLIC_PrintHex("ID2_EncDATA",AppBleType.Respond.TxDataBuf,AppBleType.Respond.TxLen+7);
			uint8_t	EncDATA[256]={0};		
			uint16_t AllLen=((AppBleType.Respond.TxLen+3)%16)? (16*(1+(AppBleType.Respond.TxLen+3)/16)):(AppBleType.Respond.TxLen+3);//��������ܳ���16����
			EncDATA[0]=(AppBleType.Respond.TxLen+1)>>8;  //��Ч���ȵ���ԭʼ����+1����
			EncDATA[1]=(AppBleType.Respond.TxLen+1)&0xff;  //��Ч���ȵ���ԭʼ����+1����
			EncDATA[2]=cmd; //��������		
			memcpy(&EncDATA[3],&AppBleType.Respond.TxDataBuf[L_DATA],AppBleType.Respond.TxLen); //����������Ч����			
			Encrypto_my_aes(ENCRYPTION, EncDATA, AllLen, App_Id2_Data.AESKEY); //��������	
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA],EncDATA,AllLen); //�������ܺ������
			AppBleType.Respond.TxLen=AllLen ; //�ܳ��ȸı�
			AppBleType.Respond.TxDataBuf[L_LENH] = AppBleType.Respond.TxLen>>8;  //����
			AppBleType.Respond.TxDataBuf[L_LENL] = AppBleType.Respond.TxLen&0xff;  //����
		}
	}
	//У��
    for(uint8_t i=1; i<(5+AppBleType.Respond.TxLen); i++  )
    {
        crc += AppBleType.Respond.TxDataBuf[i];
    }	
    AppBleType.Respond.TxDataBuf[AppBleType.Respond.TxLen+5]=crc>>8;
    AppBleType.Respond.TxDataBuf[AppBleType.Respond.TxLen+6]=crc&0xff;
	uint8_t result = 1;
	#if LOCK_PROJECT_CHIP ==LOCK_PROJECT_RTL8762 
    result=ble_tx_event_handle(AppBleType.Respond.TxDataBuf,AppBleType.Respond.TxLen+7);
	#else
	ble_tx_event_handle(AppBleType.Respond.TxDataBuf,AppBleType.Respond.TxLen+7);
	#endif
	memset(&AppBleType.Respond,0,sizeof(AppBleType.Respond));//������ɺ�����ذ�����
	return result;
}

/***************************************************************************************
**������:       AppBleInit
**��������:     ���г�ʼ��,�ŵ������ȡ
**�������:     
**�������:     
**��ע:         
****************************************************************************************/		
void SystemAppBleInit(void)
{ 
	if(SystemSeting.SystemAdminRegister==ADMIN_APP_REGISTERED)
	{
		APP_BleInit(REGISTERED,GENERAL_FLAGS);
	}
	else
	{
		APP_BleInit(NONE_REGISTERED,GENERAL_FLAGS);
	}	
	
	BleRxCmdHandlePro = E_CMD_DEFAULT;
	BleHandleProStep = 0;
	
	//�ŵ����룬MAC��4λȡ��
	uint8_t mac[6];
	PUBLIC_GetMacAdd(mac);
	AppBleType.ChannelPwd[0]=~mac[5];
	AppBleType.ChannelPwd[1]=~mac[4];
	AppBleType.ChannelPwd[2]=~mac[3];
	AppBleType.ChannelPwd[3]=~mac[2];
}

void AppBleUserDataDecode(uint8_t mode , uint8_t *data, uint8_t len)
{
	uint8_t my_aes_key[16]={0}; // AES������Կ
	memcpy(&my_aes_key[0],AppBleType.RandomNum,8); //���������
	for(uint8_t i=0; i<8; i++)
	{
		my_aes_key[8+i]=~my_aes_key[i]; //��8�ֽ�
	}
	for(uint8_t i=0; i<8; i++)
	{
		my_aes_key[i]^=Root_key[AES_ROOT_KEY][i]; //������
		my_aes_key[i+8]^=Root_key[AES_ROOT_KEY][i];
	}  
	if(mode)//����
	{
		Encrypto_my_aes(ENCRYPTION, data, len, my_aes_key);
	}
	else
	{
		//ԭʼ�˺Ż�ԭ
		Encrypto_my_aes(DECRYPTION, data, len, my_aes_key);
	}
}


void BLEParse (void)
{
	//ID2����
	//��ͷ(1byte)+������(1byte)+ָ��(1byte)+����(2byte)+���ݰ�(n byte)+У���(2byte)
	//���ݰ�(n byte)=��Ч����(2byte)+ָ��(1byte)+ ����(n-3 byte)+����16�ֽ��貹��
	
	 //�������˵�,���Ȩ��֧�����Ļ�����
	#ifdef BLE_ID2_AES_ENC
	if( AppBleType.Admin!=E_KEY_ADMIN) //�����߼���
	{
		switch (AppBleType.RxDataBuf[L_CMD])
		{
			case 0x3C:
			case 0x3E:
			case 0x3B:
			case 0x65:
			case 0x66:
			case 0x67:
			case 0x68:
			case 0x69:
			case 0x6A:break;//��������	
			default://����ָ������
				if(App_Id2_Data.ID2_EN==0)
				{
					AppBleType.Respond.TxDataBuf[L_DATA]=7; //����
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(AppBleType.RxDataBuf[L_CMD]);
					return;
				}
		}
	}
	#endif
	if(App_Id2_Data.ID2_EN)//����ģʽ
	{
		uint16_t AllLen=  AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL] ; //�ܳ���
		if((AllLen>=16) && (AllLen%16==0)) //16����
		{
			Encrypto_my_aes(DECRYPTION,&AppBleType.RxDataBuf[L_DATA], AllLen, App_Id2_Data.AESKEY); //��������
			PUBLIC_PrintHex("ID2_DECRYPTION_DATA =", &AppBleType.RxDataBuf[L_DATA],AppBleType.RxDataBuf[L_LENL]);
			if(AppBleType.RxDataBuf[L_DATA+2]==AppBleType.RxDataBuf[L_CMD]) //ȷ�Ͻ��ܺ��ָ��
			{
				//�ѽ��ܺ�������滻Ϊԭʼ������ʽ
				uint16_t ValidLen=AppBleType.RxDataBuf[L_DATA]<<8 |AppBleType.RxDataBuf[L_DATA+1] ; //��Ч����
				memcpy(&AppBleType.RxDataBuf[L_DATA],&AppBleType.RxDataBuf[L_DATA+3],ValidLen); //������Ч���ȵ����ݰ�
				AppBleType.RxDataBuf[L_LENH]=(ValidLen-1)>>8;   //���ݳ����滻Ϊ��Ч����-1
				AppBleType.RxDataBuf[L_LENL]=(ValidLen-1)&0xff; //���ݳ����滻Ϊ��Ч����-1
			}
			else
			{
				my_printf("DEC CMD ERR--------------- \n");
				AppBleType.Respond.TxDataBuf[L_DATA]=9; //����
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(AppBleType.RxDataBuf[L_CMD]);
				return;
			}
		}
		else
		{
			my_printf("ENC LEN <16----------------- \n");
			AppBleType.Respond.TxDataBuf[L_DATA]=8; //����
			AppBleType.Respond.TxLen =1;  
			AppBleRespondData(AppBleType.RxDataBuf[L_CMD]);
			return;
		}
	}
	AppBleType.RxCdm=AppBleType.RxDataBuf[L_CMD];//��¼��ǰ����	
    uint8_t led_num[10]={EM_LED_0,EM_LED_1,EM_LED_2,EM_LED_3,EM_LED_4,EM_LED_5,EM_LED_6,EM_LED_7,EM_LED_8,EM_LED_9};
	switch (AppBleType.RxCdm) 
	{
		//ֱ�Ӵ�����
		case BLE_CONFIGURATION_LOCK: //���������ϱ�
		{
            AppBleType.Respond.TxDataBuf[L_DATA]= 0x0;  //ACK�ɹ�
		
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],SystemFixSeting.LockNameType,8);//���������ͺ�
		
			PUBLIC_GetMacAdd(&AppBleType.Respond.TxDataBuf[L_DATA+1+8]); //��ȡ����MAC
		
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1+8+6],AppBleType.ChannelPwd,4); //�����ŵ�����
		
			AppBleType.Respond.TxDataBuf[L_DATA+19]=0x02;  //ID2����
		
			AppBleType.Respond.TxDataBuf[L_DATA+20]=0x02;  //AES���� ���뼰�����˺Ŵ���
		
			AppBleType.Respond.TxDataBuf[L_DATA+21]=0x05;  //Xƽ̨+¼ָ����Э��53+54
			#if defined WIFI_FUNCTION_ON
			AppBleType.Respond.TxDataBuf[L_DATA+22]=0x00;  //����ģ��0>WIFI   1>NB
			#endif
			#if defined NB_FUNCTION
			AppBleType.Respond.TxDataBuf[L_DATA+22]=0x01;  //����ģ��0>WIFI   1>NB
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+23]=AES_ROOT_KEY; //AES���ܸ���

            //--������ʽ����----------------2byte-------------------
		    OpenKind_U openKind;
			openKind.data = 0;
		    openKind.bit.PwdMode = FUNCTION_ENABLE;            //���������ʽ     0: ��֧��  1:֧��
			#ifdef FINGER_FUNCTION_ON
			openKind.bit.FingerMode = FUNCTION_ENABLE;         //ָ�ƽ�����ʽ     0: ��֧��  1:֧��
			#endif
			#ifdef SMART_KEY_FUNCTION_ON
			openKind.bit.SmartKeyMode = FUNCTION_ENABLE;      //����Կ�׽���������Կ��/�����ֻ���     0: ��֧��  1:֧��
			#endif
			#ifdef FACE_FUNCTION_ON
			openKind.bit.FaceMode = FUNCTION_ENABLE;           //����������ʽ     0: ��֧��  1:֧��
			#elif defined IRIS_FUNCTION_ON
//			openKind.bit.IrisOpenMode    = FUNCTION_ENABLE;    //��Ĥ������ʽ 	 0: ��֧��  1:֧��
			openKind.bit.FaceMode    = FUNCTION_ENABLE;    //��Ĥ������ʽ 	 0: ��֧��  1:֧��
            #endif
			#ifdef IC_CARD_FUNCTION_ON
			openKind.bit.IcCardMode = FUNCTION_ENABLE;         //IC��������ʽ     0: ��֧��  1:֧��
			#endif
			#ifdef FINGER_VEIN_FUNCTION_ON
			openKind.bit.FingerVeinMode  = FUNCTION_ENABLE;   //ָ����������ʽ  0: ��֧��  1:֧��
			#endif
			#ifdef HW_WALLET_FUNCTION_ON
			openKind.bit.HuaWeiWalletMode = FUNCTION_ENABLE;  //��ΪǮ��������ʽ 0: ��֧��  1:֧��
			#endif
			
            AppBleType.Respond.TxDataBuf[L_DATA+24] = (uint8_t)(openKind.data >> 8);	      
            AppBleType.Respond.TxDataBuf[L_DATA+25] = (uint8_t)(openKind.data >> 0);

            AppBleType.Respond.TxDataBuf[L_DATA+26] = 0x00;	    //����ָ��0ö
            AppBleType.Respond.TxDataBuf[L_DATA+27] = M_FINGER_MAX_ADMIN_NUM;	    //����ָ�ƻ�ָ��������

            //--ȫ��������---------------2byte----------------------------------------------------
 
			AutoLockCfg_U  AutoCfg;
			AutoCfg.data = 0;
			//���޸ù���
			AutoCfg.bit.GyroscopeModuleEn   = FUNCTION_DISABLE;       //������ģ��             0: ��  1:��
			AutoCfg.bit.MotoDirectAdjustEn  = FUNCTION_ENABLE;        //���ŷ�����������ң�   0: ��  1:��
			AutoCfg.bit.MotoTorsionAdjustEn = FUNCTION_ENABLE;        //���Ť������  		   0: ��  1:��
			#ifdef LOCK_BODY_216_MOTOR   
			AutoCfg.bit.MotoDirectAdjustEn  = FUNCTION_DISABLE;       //���ŷ�����������ң�   0: ��  1:��
			AutoCfg.bit.MotoTorsionAdjustEn = FUNCTION_DISABLE;       //���Ť������  		   0: ��  1:��
			#endif
			#ifdef LOCK_BODY_212_MOTOR  //����
    		AutoCfg.bit.AutoLockTimAdjustEn = FUNCTION_ENABLE;        //�Զ�����ʱ�����  	   0: ��  1:��
    	    AutoCfg.bit.AutoLockTimSetPara  = SystemSeting.SysAutoLockTime/5;  //�Զ�����ʱ�����(0-32)*5��     
			#elif defined LOCK_BODY_AUTO_MOTOR
			if( LockConfigMode == LOCK_BODY_212 )
			{
				AutoCfg.bit.AutoLockTimAdjustEn = FUNCTION_ENABLE;    //�Զ�����ʱ�����  	   0: ��  1:��
			    AutoCfg.bit.AutoLockTimSetPara  = SystemSeting.SysAutoLockTime/5;  //�Զ�����ʱ�����(0-32)*5��     
			}
			#endif
			
            #ifdef LOCK_KEY_CONFIRM_ON			
			AutoCfg.bit.TouchLockConfirmEn  = FUNCTION_ENABLE;        //ȫ�Զ�����ȷ�Ϲ��ܣ�������������ȷ�ϣ� 0: ��  1:��	
			#endif
			//�ù��ܿ���״̬
			AutoCfg.bit.GyroscopeModuleSts = FUNCTION_DISABLE;     	  //������ģ��             0: �ر�   1:����
            if( SystemFixSeting.MotorDirection == LEFT_HAND_DOOR )   
            {
                AutoCfg.bit.MotoDirectAdjustSts = FUNCTION_ENABLE;    //���ŷ�����������ң�   0: �ҿ�   1: ��  
            }
            if( SystemFixSeting.MotorTorque == HIGH_TORQUE ) 
            {
                AutoCfg.bit.MotoTorsionAdjustSts = FUNCTION_ENABLE;   //���Ť������ 		   0: ��Ť�� 1: ��Ť��   
            }
 
			AppBleType.Respond.TxDataBuf[L_DATA+28] = (uint8_t)(AutoCfg.data >> 8);	
			AppBleType.Respond.TxDataBuf[L_DATA+29] = (uint8_t)(AutoCfg.data >> 0);	
			
			
			AppBleType.Respond.TxDataBuf[L_DATA+30] = 0x06;          //¼��ָ�ƴ���

			//--����������1---------------2byte--------------------------------------------------------------------
	        FunctionCfg_U funcCfg;
            (void)memset( funcCfg.tab, 0, sizeof funcCfg );
			//���޸ù���   ��������1 
			#ifdef KEY_DEFENSE_ON
            funcCfg.bit.OneKeyDeployEn   = FUNCTION_ENABLE;    //һ����������       0: ��  1:��
			#endif
			funcCfg.bit.VolGradeAdjustEn = FUNCTION_ENABLE;    //��������           0: ��  1:��
			#ifdef HUMAN_ACTIVE_DEF_ON  
			funcCfg.bit.ActiveDefenseEn  = FUNCTION_ENABLE;    //��������(����30����) 0: ��  1:��
			#endif	
			funcCfg.bit.NfcFuncEn        = FUNCTION_DISABLE;   //NFC����		    0: ��  1:��
			funcCfg.bit.OpenKindMoreEn   = FUNCTION_ENABLE;    //���ŷ�ʽ��ѡ����	0: ��  1:��
			#ifdef BELL_VIDEO_FUNC_ON
			funcCfg.bit.BellTakePictureEn= FUNCTION_DISABLE;   //����ץ�Ĺ���		0: ��  1:��
			#endif
			#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON  
			funcCfg.bit.NearSenseCheckEn = FUNCTION_DISABLE;   //������빦��		0: ��  1:�� 
            #endif
			funcCfg.bit.EventRecordEn    = FUNCTION_ENABLE;    //�¼���¼����		0: ��  1:��
			//�ù��ܿ���״̬
			if( SystemSeting.SysKeyDef )
			{
				funcCfg.bit.OneKeyDeploySts= FUNCTION_ENABLE;    //һ����������״̬       0: �ر�   1:����
			}
			funcCfg.bit.NfcFuncSts         = FUNCTION_DISABLE;   //NFC���ܿ���״̬        0: �ر�   1:����  
			funcCfg.bit.BellTakePictureSts = FUNCTION_DISABLE;   //����ץ�Ĺ��ܿ���״̬   0: �ر�   1:����  
			funcCfg.bit.EventRecordSts     = FUNCTION_ENABLE;    //�¼���¼���ܿ���״̬   0: �ر�   1:����  

			AppBleType.Respond.TxDataBuf[L_DATA+31] = funcCfg.tab[ 0 ];
			AppBleType.Respond.TxDataBuf[L_DATA+32] = funcCfg.tab[ 1 ];

            //--����������2---------------2byte--------------------------------------------------------------------
			//���޸ù���  ��������2 
			funcCfg.bit.OemBraceletEn = FUNCTION_DISABLE;    //С��OEM���ֻ�       0: ��  1:��
			#ifdef XM_CAM_FUNCTION_ON
			funcCfg.bit.CameraTypeEn  = FUNCTION_ENABLE;     //è��(�������±ȡ���ŵ��)���A3ָ��  0: ��  1:��
			#endif
			#ifdef ST_CAM_FUNCTION_ON
			funcCfg.bit.STCameraEn    = FUNCTION_ENABLE;     //����è��       0: ��  1:��
			#endif
			#ifdef OB_CAM_FUNCTION_ON
			funcCfg.bit.ABCameraEn    = FUNCTION_ENABLE;     //�±�è��       0: ��  1:��	
			#endif
			#ifdef IC_CARD_FUNCTION_ON
			funcCfg.bit.IcCardEn      = FUNCTION_ENABLE;     //IC������       0: ��  1:��	
			#endif
			#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON  
			funcCfg.bit.R8NearSenseAdjustEn = FUNCTION_ENABLE;   //R8�������ɵ�(Զ/��/�ر�) 0: ��  1:��  
			#endif
			funcCfg.bit.NewRemoteControlEn = FUNCTION_ENABLE;    //��ң����  0: ��  1:��  
			#ifdef  OB_CAM_FUNCTION_ON
			funcCfg.bit.FaceForceEn = FUNCTION_ENABLE;            //����Ю�ֿ���  0: ��  1:��  
			#endif
			
			//�ù��ܿ���״̬
			funcCfg.bit.STCameraSts = FUNCTION_DISABLE;           //����è�۵�˫�� 0: ����   1:˫��

			#if defined LOCK_BODY_216_MOTOR|| defined LOCK_BODY_218_MOTOR 
			funcCfg.bit.UnlockWarmEn = FUNCTION_ENABLE;          //��δ�ر������� 0: ��  1:��	
			#elif defined LOCK_BODY_AUTO_MOTOR 
			if( LockConfigMode == LOCK_BODY_218 )
			{
				funcCfg.bit.UnlockWarmEn = FUNCTION_ENABLE;          //��δ�ر������� 0: ��  1:��	
			}
			#endif
			funcCfg.bit.FingerGetID = FUNCTION_ENABLE;            //ָ��ID��ѯ
			#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
            funcCfg.bit.FaceCheckEn = FUNCTION_ENABLE;            //������֤����  0: ��  1:��	
			#endif
			#if defined GET_WIFI_SIGNAL_INTENSITY_ON
			funcCfg.bit.GetWifiSignalIntensity = FUNCTION_ENABLE; //��ȡWIFI�ź�ǿ�ȹ���  0: ��  1:��
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+33] = funcCfg.tab[ 2 ];
			AppBleType.Respond.TxDataBuf[L_DATA+34] = funcCfg.tab[ 3 ];
 
            AppBleType.Respond.TxDataBuf[L_DATA+35]	= SystemSeting.SysVoice;       //��ǰ����
            AppBleType.Respond.TxDataBuf[L_DATA+36] = SystemSeting.SysHumanIrDef;  //��ǰ��������ʱ��
			AppBleType.Respond.TxDataBuf[L_DATA+37] = SystemSeting.SysLockMode;    //��ǰ����ģʽ
			AppBleType.Respond.TxDataBuf[L_DATA+38] = SystemSeting.SysDrawNear;    //��ǰ�ӽ���Ӧ����

			//--��װ����---------------8byte--------------------
			FactoryTestCfg_U factoryTest;
			(void)memset( factoryTest.tab, 0, sizeof factoryTest );
			factoryTest.bit.NfcCheckEn     = FUNCTION_DISABLE;       //NFC���      0: ��  1:��
			factoryTest.bit.VersionCheckEn = FUNCTION_ENABLE;        //�汾��ѯ     0: ��  1:��
			factoryTest.bit.ReadMacAddrEn  = FUNCTION_ENABLE;        //��MAC��ַ    0: ��  1:��
			factoryTest.bit.CheckBatVol1En = FUNCTION_ENABLE;        //��ѹ1        0: ��  1:��
			factoryTest.bit.CheckBatVol2En = FUNCTION_ENABLE;        //��ѹ2        0: ��  1:��
			factoryTest.bit.PirTestEn      = FUNCTION_DISABLE;       //PIR����      0: ��  1:��
			factoryTest.bit.WriteFlashEn   = FUNCTION_DISABLE;        //дflash      0: ��  1:��
			factoryTest.bit.ReadFlashEn    = FUNCTION_DISABLE;        //��flash      0: ��  1:��

			factoryTest.bit.ScreenTestEn   = FUNCTION_ENABLE;        //��Ļ����     0: ��  1:��
			factoryTest.bit.FingerInputEn  = FUNCTION_DISABLE;       //¼��ָ��     0: ��  1:��
			factoryTest.bit.FaceCommiteEn  = FUNCTION_DISABLE;       //����ͨ��     0: ��  1:��
			#if defined WIFI_FUNCTION_ON
			factoryTest.bit.WifiSelfcheckEn= FUNCTION_ENABLE;        //WIFI�Լ�     0: ��  1:��
			#endif
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
			factoryTest.bit.R8SecretKeyLoadEn=FUNCTION_ENABLE;      //R8��Կ�·�   0: ��  1:��
			#else
			factoryTest.bit.R8SecretKeyLoadEn=FUNCTION_DISABLE;
			#endif
			factoryTest.bit.InnerBatNormalEn=FUNCTION_DISABLE;       //����﮵��(3.0V-4.3V)��Ϊ����  0: ��  1:��
			factoryTest.bit.ClearAllUserEn = FUNCTION_ENABLE;        //����û�       0: ��  1:��
            #ifdef XM_CAM_FUNCTION_ON
            factoryTest.bit.Q5M_V9RealTimeVideoEn = FUNCTION_ENABLE;//Q5M/V9ʵʱ��Ƶ 0: ��  1:��
            #else
            factoryTest.bit.Q5M_V9RealTimeVideoEn = FUNCTION_DISABLE;//Q5M/V9ʵʱ��Ƶ 0: ��  1:��
            #endif
 
			factoryTest.bit.PinCodeLoadEn   = FUNCTION_DISABLE;      //��ΪPIN�·�    0: ��  1:��
			factoryTest.bit.BleDisconnectEn = FUNCTION_DISABLE;       //�����Ͽ�       0: ��  1:��
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
			factoryTest.bit.R8RealTimeVideoEn = FUNCTION_ENABLE;    //R8ʵʱ��Ƶ     0: ��  1:��
			#else
			factoryTest.bit.R8RealTimeVideoEn = FUNCTION_DISABLE;
			#endif
			factoryTest.bit.WifiSignalIntensityEn = FUNCTION_DISABLE; //WIFI����ɨ��·�����ź�ǿ��  0: ��  1:��
			factoryTest.bit.IcCardCheckEn   = FUNCTION_DISABLE;      //IC�����             0: ��  1:��
			factoryTest.bit.LockModelLoadEn = FUNCTION_DISABLE;       //ȫ�Զ������ͺ��·�   0: ��  1:��
			factoryTest.bit.SnCodeLoadEn    = FUNCTION_DISABLE;      //��ΪSN�·�           0: ��  1:��
 
			factoryTest.bit.EventRecordClearEn = FUNCTION_ENABLE;    //С�ι�װ������¼���¼  0: ��  1:��
			factoryTest.bit.EncodeChipCheckEn  = FUNCTION_DISABLE;   //����оƬ���            0: ��  1:��
			factoryTest.bit.ButtonOpenModeCfgEn= FUNCTION_DISABLE;   //���ż�����/˫���������� 0: ��  1:��
		
			AppBleType.Respond.TxDataBuf[L_DATA+39] = factoryTest.tab[ 0 ];
			AppBleType.Respond.TxDataBuf[L_DATA+40] = factoryTest.tab[ 1 ];
			AppBleType.Respond.TxDataBuf[L_DATA+41] = factoryTest.tab[ 2 ];
			AppBleType.Respond.TxDataBuf[L_DATA+42] = factoryTest.tab[ 3 ];
			AppBleType.Respond.TxDataBuf[L_DATA+43] = factoryTest.tab[ 4 ];
			AppBleType.Respond.TxDataBuf[L_DATA+44] = factoryTest.tab[ 5 ];
			AppBleType.Respond.TxDataBuf[L_DATA+45] = factoryTest.tab[ 6 ];
			AppBleType.Respond.TxDataBuf[L_DATA+46] = factoryTest.tab[ 7 ];
			
			CombineOpenMode_U combineMode;
			combineMode.data = 0;
			#ifdef  FACE_FUNCTION_ON
			combineMode.bit.FaceAndFinger = FUNCTION_ENABLE;   //���� + ָ��    0: ��  1:��
			combineMode.bit.FaceAndPwd    = FUNCTION_ENABLE;   //���� + ����    0: ��  1:��
            #elif defined IRIS_FUNCTION_ON
            combineMode.bit.IrisAndFinger = FUNCTION_ENABLE;   //��Ĥ + ָ��    0: ��  1:��
            combineMode.bit.IrisAndPwd    = FUNCTION_ENABLE;   //��Ĥ + ����    0: ��  1:��
            #elif defined FINGER_VEIN_FUNCTION_ON
            combineMode.bit.VeinAndPwd = FUNCTION_ENABLE;      //ָ���� + ����      0: ��  1:��
            #else
			combineMode.bit.FingerAndPwd  = FUNCTION_ENABLE;   //ָ�� + ����    0: ��  1:��    
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+47] = combineMode.data;//��Ͽ���  

			
            //--����������3---------------2byte--------------------------------------------------------------------
			//���޸ù���  ��������3 
			#ifdef XM_CAM_SCREEN_FUNCTION_ON
			funcCfg.bit.ScreenIndoor = FUNCTION_ENABLE;    //��������  0: ��  1:��     
			#endif
			#if LOCK_PROJECT_CHIP == LOCK_PROJECT_RTL8762
			funcCfg.bit.RTLMcu = FUNCTION_ENABLE;    //�Ƿ���RTLƽ̨  0: ����  1:��     
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+48] = funcCfg.tab[ 4 ];
			AppBleType.Respond.TxDataBuf[L_DATA+49] = funcCfg.tab[ 5 ];
			
			AppBleType.Respond.TxLen=50; //ֱ�ӻظ�
		}
		break;
        case BLE_CMD_OTAMODE: //�Ͽ�
        {
            AppBleType.Respond.TxLen=1;
            AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ� ����D
            AppBleRespondData(BLE_CMD_OTAMODE);//ͳһ�ذ�
            PUBLIC_Delayms(500);
			DRV_InterGenerateStartOta();
            break;
        }
		case BLE_TIMEUPDATA:         //ʱ��ͬ��
		{
			RTCType rtc_w;
			rtc_w.year = AppBleType.RxDataBuf[L_DATA];
			rtc_w.month = AppBleType.RxDataBuf[L_DATA+1];
			rtc_w.day =  AppBleType.RxDataBuf[L_DATA+2];
			rtc_w.week = AppBleType.RxDataBuf[L_DATA+3];
			rtc_w.hour = AppBleType.RxDataBuf[L_DATA+4];
			rtc_w.minuter = AppBleType.RxDataBuf[L_DATA+5];
			rtc_w.second = AppBleType.RxDataBuf[L_DATA+6];
			AppBleType.Respond.TxLen=1; //�ذ���
			//����RTC�����룬�ɹ�RTC_Successfully
			AppBleType.Respond.TxDataBuf[L_DATA]=HAL_RTC_WriteTime(&rtc_w); 
			break;
		}
		case BLE_SMARTKEY_GETKEY1:
			 APP_BleSetAdminState(E_VIDEO_ADMIN); //����������Ƶ�������û������˳�
		case BLE_PHONE_GETKEY:       //��ȡ�����
		case BLE_SMARTKEY_GETKEY2:
		{
			AppBleType.Respond.TxLen=9; //�ذ���
		    PUBLIC_GenerateRandVec(AppBleType.RandomNum,8);//���������
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],AppBleType.RandomNum,8);//��������
			break;
		}
		case BLE_AES_ADDPHONE:    	 //�����ֻ��˺�
		{
			//ԭʼ�˺Ż�ԭ
			AppBleUserDataDecode(0,&AppBleType.RxDataBuf[L_DATA],16);
			//ֱ�Ӵ洢�˺ţ�С�ֲ�Ʒ��һ��
			PUBLIC_PrintHex ( "BLE_AES_ADDPHONE",&AppBleType.RxDataBuf[L_DATA], 16 );
			HAL_EEPROM_WriteBytes(MEM_PHONE_START,&AppBleType.RxDataBuf[L_DATA],13); 
			AppBleType.Respond.TxLen=1; //�ذ���
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
		    SystemEventLogSave( ADD_BLE, 0 ); 
		    break;
		}
		case BLE_DELPHONE:        	 //ɾ���ֻ���
		{   
			//�������ֻ��������˺ţ�ֱ��ɾ������һ��
			APP_BleDelPhone();
			AppBleType.Respond.TxLen=1; //�ذ���
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
		    SystemEventLogSave( DELETE_BLE, 0 ); 
			break;
		}
		case BLE_ADDSMARTKEY_REQ: 	 //��������Կ����Կ����
		{
			for(uint8_t i=0;i<8;i++)//������
			{
				AppBleType.RxDataBuf[L_DATA+i]^=AppBleType.RandomNum[i];
				AppBleType.RxDataBuf[L_DATA+i+8]^=AppBleType.RandomNum[i];
			}
		    SystemEventLogSave( ADD_SMART_KEY, 0 ); 
			SmartKeyWriteId(&AppBleType.RxDataBuf[L_DATA]); //��ID
			AppBleType.Respond.TxLen=1; //�ذ���
			AppBleType.Respond.TxDataBuf[L_DATA]=0;
			break;
		}
		case BLE_DELSMARTKEY: //ɾ������Կ��
			SmartKeyDeleteId(&AppBleType.RxDataBuf[L_DATA]);
			AppBleType.Respond.TxLen=1; //�ذ���
			AppBleType.Respond.TxDataBuf[L_DATA]=0;
		    SystemEventLogSave( DELETE_SMART_KEY, 0 ); 
			break;
		case BLE_SMARTKEY_ENLOCK:
		{
			//�������������ܱȶ�
			uint16_t pageid=SmartkeyCheckRegId(AES_ENC,(const uint8_t*)&AppBleType.RxDataBuf[L_DATA]);
			if(PAGEID_NULL !=pageid)
			{
				uint8_t id[13]={0};
				SmartKeyReadId(pageid,id);//��ȡԭʼID
				//���ųɹ�
				if( true == App_GUI_GetWifiUploadSwSts() )
				{ 	
					WifiLockMeg.UnlockMode = SMARTKEY;
					WifiLockMeg.PageID.way1 = id[3];
					WifiLockMeg.PageID.id1 = id[4];
					WifiLockMeg.PageID.way2 = id[5];
					WifiLockMeg.PageID.id2 = id[6];
					WifiLockMeg.Attribute = NONE;		
//					App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
					UploadUnlockDoorMegEnable = 1;
				}
				SystemEventLogSave( SMART_KEY_OPEN, id[3] );
				APP_BleSetAdminState(E_SMARTKEKY_CHECK_OK);
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleType.Respond.TxDataBuf[L_DATA]=0;
				
			}
			else//����ʧ�ܼǽ���
			{				
				APP_BleSetAdminState(E_PHONE_CHECK_ERR);//��֤ʧ��	
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleType.Respond.TxDataBuf[L_DATA]=1;
			}
			break;	
		}
		case BLE_AES_ADD_PASSWORD  :     //������������
		{
			//ԭʼ�˺Ż�ԭ
			AppBleUserDataDecode(0,&AppBleType.RxDataBuf[L_DATA],16);
			uint8_t pwdbuf[MSG_PWD_BYTE_SIZE] = {0};
			memcpy( pwdbuf, &AppBleType.RxDataBuf[L_DATA], MSG_PWD_BYTE_SIZE );
			
			PwdMeg_T pwdmeg = {0}; 
			pwdmeg.UserValue  = MEM_PWD_VALID_FLG;
			pwdmeg.Privileges = PWD_LIMIT_ADMIN;
			pwdmeg.UserId     = PWD_USER_ID;
			memcpy( pwdmeg.Password, &AppBleType.RxDataBuf[L_DATA], MSG_PWD_BYTE_SIZE );
			uint8_t tp1 =0;
			for(uint8_t i=0; i<MSG_PWD_BYTE_SIZE; i++)
			{
				if( pwdbuf[i] == 0xFF )
				{
					tp1 = 1;
				    App_PWD_DelPwdIdFromEeprom( PWD_USER_ID );  
					SystemEventLogSave( DELETE_PASSWORD, PWD_USER_ID );  
					break;
				}
			}
			pwdmeg.Password[ MSG_PWD_BYTE_SIZE ] ='\0';
			if( tp1 == 0 )
			{
				if( SystemSeting.SysPwdAllNum )  //�޸�����
				{
					(void)App_PWD_ExchangePwdMegIntoEeprom( &pwdmeg );
					SystemEventLogSave( ADD_PASSWORD, PWD_USER_ID );  
				}
				else
				{
					(void)App_PWD_SaveOnePwdMegIntoEeprom( &pwdmeg );
					SystemEventLogSave( ADD_PASSWORD, PWD_USER_ID );  
				}
			}
			AppBleType.Respond.TxLen=1; //�ذ���
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			break;
		}
		case BLE_AES_ADD_SOS_PASSWORD  : //SOS������������
		{
			//ԭʼ�˺Ż�ԭ
			AppBleUserDataDecode(0,&AppBleType.RxDataBuf[L_DATA],16);
			SosPwdMeg_T sosPwd[SOS_PWD_ALL_NUM] ={0};
			uint8_t tp1 =0;
			for(uint8_t i=0; i<MSG_PWD_BYTE_SIZE; i++)
			{
				if( AppBleType.RxDataBuf[L_DATA+i] == 0xFF )
				{
					App_PWD_ClearAllSosPwdsFromEeprom();
					SystemEventLogSave( DELETE_SOS_PASSWORD, 0 );  
					tp1 =1;
					break;
				}
			}
			if( tp1 == 0 )
			{
				(void)memcpy( sosPwd[0].Password, &AppBleType.RxDataBuf[ L_DATA ], SOS_PWD_BYTE_SIZE );
				App_PWD_SaveSosPwdIntoEeprom( sosPwd );
				SystemEventLogSave( ADD_SOS_PASSWORD, 0 );  
			}
			AppBleType.Respond.TxLen=1; //�ذ���
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			break;
		}
		
		case BLE_AES_ADD_TMP_PASSWORD  : //��ʱ��������
		{
            TmpPwdMeg_T tmPwdmeg[TEMP_PWD_ALL_NUM] = {0};
			App_PWD_CreateTempPwds( tmPwdmeg );
			
			AppBleType.Respond.TxLen=0x3D;           //�ذ���
			AppBleType.Respond.TxDataBuf[L_DATA]=0;  //�ɹ�	
			
			for(uint8_t i=0; i<TEMP_PWD_ALL_NUM; i++)
			{
			    (void)memcpy( &AppBleType.Respond.TxDataBuf[L_DATA + 1+ i*TEMP_PWD_BYTE_SIZE], tmPwdmeg[i].Password, TEMP_PWD_BYTE_SIZE );
				PUBLIC_PrintHex("temp password:", tmPwdmeg[i].Password, TEMP_PWD_BYTE_SIZE);
			}
            AppBleUserDataDecode(1,&AppBleType.Respond.TxDataBuf[L_DATA+1],64);
 			AppBleType.Respond.TxLen=65;           //�ذ���
			
			App_PWD_SaveTempPwdsIntoEeprom( tmPwdmeg );
			
			break;
		}
		
		case BLE_PHONE_ENLOCK :  //�ֻ���������
		{
			for(uint8_t i=0;i<8;i++)//������  ��Ч����13+4
			{
				AppBleType.RxDataBuf[L_DATA+i]^=AppBleType.RandomNum[i];
				AppBleType.RxDataBuf[L_DATA+i+8]^=AppBleType.RandomNum[i];
				AppBleType.RxDataBuf[L_DATA+i+8+8]^=AppBleType.RandomNum[i];
			}
			//��֤�ŵ�����
			//��ȡ
			PUBLIC_PrintHex("BLE_PHONE_ENLOCK",&AppBleType.RxDataBuf[L_DATA],17);
			uint8_t PhoneID[17];
			memcpy(PhoneID,AppBleType.ChannelPwd,4); //�����ŵ�����
			HAL_EEPROM_ReadBytes(MEM_PHONE_START,&PhoneID[4],13); //��ȡID 
			PUBLIC_PrintHex("PhoneID",PhoneID,17);
			for(uint8_t CheckLoop = 0; CheckLoop < 17; CheckLoop++)//ID�Ա�
            {
                if(PhoneID[CheckLoop] != AppBleType.RxDataBuf[L_DATA+CheckLoop])
                {
					//��֤ʧ��
					AppBleType.Respond.TxLen=1; //�ذ���
					AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ��	
					APP_BleSetAdminState(E_PHONE_CHECK_ERR);
                    break;
                }
                else if ( 16 == CheckLoop)
				{
					//��֤�ɹ��ϱ�����
					APP_BleSetAdminState(E_PHONE_CHECK_OK);
					SystemEventLogSave( BLE_OPEN, 0 );  
					AppBleType.Respond.TxLen=3; //�ذ���
					AppBleType.Respond.TxDataBuf[L_DATA]=0; 
					uint32_t upper_bat_data  = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
					if(upper_bat_data>200) //��һ·��2V���ɵ���жϣ������ϲ�Ʒ
					{
						upper_bat_data-=200;
					}
					AppBleType.Respond.TxDataBuf[L_DATA+1] = (uint8_t)(upper_bat_data  >> 8);//������ʾ
					AppBleType.Respond.TxDataBuf[L_DATA+2] = (uint8_t)upper_bat_data;//������ʾ
					#ifdef UNDER_BAT_ADC_ON
					uint32_t under_bat_data = HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10;
					AppBleType.Respond.TxDataBuf[L_DATA+3] = (uint8_t)(under_bat_data  >> 8);//������ʾ
					AppBleType.Respond.TxDataBuf[L_DATA+4] = (uint8_t)under_bat_data;//������ʾ
					AppBleType.Respond.TxLen=5; //�ذ���
					#endif
					//��֤�ɹ�

					break;
                }
            }
			break;
		}
		
		case BLE_DEVICE_REG:	 //�豸ע��	
		{
			strcpy((char*)&AppBleType.Respond.TxDataBuf[L_DATA+1],"S70000000123456");
			PUBLIC_GetMacAdd(&AppBleType.Respond.TxDataBuf[L_DATA+16]);//����MAC
            memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+16+6],AppBleType.ChannelPwd,4);//�����ŵ�����
			AppBleType.Respond.TxDataBuf[L_DATA]=0;  
			AppBleType.Respond.TxLen=1+15+6+4; //�ذ���
			SystemSeting.SystemAdminRegister = ADMIN_APP_REGISTERED;
			SystemWriteSeting((uint8_t *)&SystemSeting.SystemAdminRegister,2); //���ע��
			break;
		}
		case BLE_CLEAR_USERMEM:  //����û�
		{
			if((AppBleType.RxDataBuf[L_DATA] == 0x25) && (AppBleType.RxDataBuf[L_DATA+1] == 0x87) &&\
                AppBleType.RxDataBuf[L_DATA+2] == 0x45 && AppBleType.RxDataBuf[L_DATA+3] == 0x10)
            {
				BleRxCmdHandlePro = E_CMD_UNREGIST_DEVICE;
				BleHandleProStep = 0;
            }
			break;
		}
		case BLE_REG_CHECK_CODE:
		{
			BleRxCmdHandlePro = E_CMD_REG_PIN;
			BleHandleProStep = 0;
			break;
		}
		case BLE_ADMIN_PWD_CHECK://�� ����������֤
		{
			//����Ҫ��֤Ĭ�����룬ֱ�ӻظ�
			if(SystemSeting.SystemAdminRegister!=ADMIN_NONE_REGISTERED)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //����ģʽ��APPע��󣬶���Ҫ��ʾӲ���
			}
			else
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //���������
			}
			AppBleType.Respond.TxLen=1; //�ذ���
			break;
   		}
		case BLE_GET_HWIFO:      //���������汾�Ż�ȡ
		{
			AppBleType.Respond.TxLen = strlen(LOCK_VERSION);//0x13;
			strcpy((char*)&AppBleType.Respond.TxDataBuf[L_DATA],LOCK_VERSION);
		    AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ� ����D
		    break;
		}
		case BLE_CMD_DISCONNECT: //�Ͽ�
		{
			AppBleType.Respond.TxLen=1;
		    AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ� ����D
			AppBleRespondData(AppBleType.RxCdm);//ͳһ�ذ�
			App_GUI_UpdateMenuQuitTime(1*100, true);
			break;
		}
		case BLE_SET_LOGUPWITHWIFI :   //������¼�ϴ����ÿ���
		{
			if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
				SystemSeting.SysWifiLogSw = FUNCTION_ENABLE;
			else 
				SystemSeting.SysWifiLogSw = FUNCTION_DISABLE;
			SystemWriteSeting(&SystemSeting.SysWifiLogSw,1);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			break;
		}
		#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 
			case BLE_FACE_PRO_OTA :
			{
				BleRxCmdHandlePro = E_CMD_FACE_PRO_OTA;
				BleHandleProStep = 0;
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //������
				AppBleType.Respond.TxLen=1; //�ذ���
				break;
			}
			case BLE_CAM_BELL_ONE_WAY :
			{
				if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
					SystemSeting.SysWifiSingle = FUNCTION_DISABLE;
				else 
					SystemSeting.SysWifiSingle = FUNCTION_ENABLE;
				SystemWriteSeting(&SystemSeting.SysWifiSingle,1);
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
				AppBleType.Respond.TxLen=1; //�ذ���
				break;
			}
			case BLE_WIFI_MAIN_SW :        //WIFI������
			{
				BleRxCmdHandlePro = E_CMD_WIFI_MAIN_SW_POW_OFF;
				if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
				SystemSeting.SysWifiMainSw = FUNCTION_ENABLE;
				else 
					SystemSeting.SysWifiMainSw = FUNCTION_DISABLE;
				SystemWriteSeting(&SystemSeting.SysWifiMainSw,1);
				BleHandleProStep = 0;
				break;
			}
		#else
		case BLE_WIFI_MAIN_SW :        //WIFI������
		{
			if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
				SystemSeting.SysWifiMainSw = FUNCTION_ENABLE;
			else 
				SystemSeting.SysWifiMainSw = FUNCTION_DISABLE;
			SystemWriteSeting(&SystemSeting.SysWifiMainSw,1);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			break;
		}
		case BLE_CMD_SET_WIFI :        //����
		{
			(void)memcpy( WifiLockMeg.Ssid, &AppBleType.RxDataBuf[L_DATA], 33 );
			(void)memcpy( WifiLockMeg.Passwd, &AppBleType.RxDataBuf[L_DATA+33], 65 );
			BleRxCmdHandlePro = E_CMD_ROUTER_CFG;
			BleHandleProStep = 0;
			break;
		}
		#endif
		case BLE_WIFI_CONNECTION_TEST ://���� ����������״̬
		{
			BleRxCmdHandlePro = E_CMD_SERVER_TEST;
			WifiLockMeg.UnlockMode = SERVER_TEST;
			WifiLockMeg.PageID.way1 = 0;
			WifiLockMeg.PageID.id1 = 0;
			WifiLockMeg.PageID.way2 = 0;
			WifiLockMeg.PageID.id2 = 0;
			WifiLockMeg.Attribute = NONE;
            BleHandleProStep = 0;
			break;
		}
		case BLE_WIFI_FACTORY_TEST:     //wifi����ɨ��ָ��·����
		{
			BleRxCmdHandlePro = E_CMD_WIFI_FACTORY_TEST;
			BleHandleProStep = 0;
			break;
		}
		case BLE_CMD_SET_WIFI_IP :     // WIFI�������������á�
		{
			(void)memcpy( WifiLockMeg.severname, &AppBleType.RxDataBuf[L_DATA], 30 );
			(void)memcpy( WifiLockMeg.portno, &AppBleType.RxDataBuf[L_DATA + 30], 2);
			BleRxCmdHandlePro = E_CMD_SERVER_CFG;
			BleHandleProStep = 0;
			break;
		}
		case BLE_GET_WIFI_SIGNAL_INTENSITY :     // ��ȡWIFI�ź�ǿ��
		{
			BleRxCmdHandlePro = E_CMD_GET_WIFI_SIGNAL_INTENSITY;
			BleHandleProStep = 0;
			break;
		}
		case BLE_VIOCE :  		  //��������
		{
			SystemSeting.SysVoice=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			break;	
		}
		case BLE_IR_SET : 		  //�������ã��ӽ���Ӧ��
		{
			ComDataByte1 = AppBleType.RxDataBuf[L_DATA+1]; //�ڶ����ֽ�
			BleRxCmdHandlePro = E_CMD_NEAR_SENSE_CFG;
			BleHandleProStep = 0;
			break;	
		}
		case BLE_MOTOR_TORQUE :   //���Ť��
		{
			SystemFixSeting.MotorTorque=AppBleType.RxDataBuf[L_DATA]; //��ֵ
			SystemWriteFixSeting(&SystemFixSeting.MotorTorque,1); //д��
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			break;
		}
		case BLE_DIRECTION :      //���ҿ� 
		{
			SystemFixSeting.MotorDirection=AppBleType.RxDataBuf[L_DATA]; //��ֵ
			SystemWriteFixSeting(&SystemFixSeting.MotorDirection,1); //д��
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			break;
		}
		case BLE_LOCK_MODE :	  //����ģʽ����Ͽ��ţ�
		{
			if(AppBleType.RxDataBuf[L_DATA]==DOUBLE_CHECK_SW_ON) //������ģʽ
			{
				SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_ON; 
			}
			else
			{
				SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_OFF; 
			}
			SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //д��Ͽ���
			
			SystemSeting.SysLockMode=AppBleType.RxDataBuf[L_DATA+1]; //�ڶ����ֽ���ģʽ
			SystemWriteSeting(&SystemSeting.SysLockMode,1);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			break;	
		}
		case BLE_AUTO_LOCK_TIME : //�Զ�����
		{
			SystemSeting.SysAutoLockTime=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysAutoLockTime,1);//д����
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1;  //�ذ���	
			App_GUI_DefaultDoorState();  //��ֹ�Զ�����
			break;
		}
		case BLE_FACE_CHECK_EN : //������֤����
		{
			SystemSeting.FaceCheckEnable=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.FaceCheckEnable,1);//д����
			if((0x66 == SystemSeting.FaceCheckEnable) && (SystemSeting.SysCompoundOpen == DOUBLE_CHECK_SW_ON))//�ر�������֤
			{
				SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_OFF; 
				SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //д��Ͽ���
			}
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1;  //�ذ���	
			break;
		}
		case BLE_KEY_DEF :   	  //һ������
		{
			SystemSeting.SysKeyDef=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysKeyDef,1);//д����
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���	
			break;
		}
		case BLE_IR_DEF :    	  //��������
		{
			SystemSeting.SysHumanIrDef=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysHumanIrDef,1);//д����
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���	
			break;
		}
		case BLE_RGB_MODE :    	  //��Χ��ģʽ����
		{
			SystemSeting.SysHumanIrDef=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.LedRGBMode,1);//д����
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���	
			break;
		}
		#if defined XM_CAM_FUNCTION_ON //è�ۻ�ȡ���к� �·���Կ
		case BLE_CAM_GET_SN :
		{
			BleRxCmdHandlePro = E_CMD_CAM_GET_SN;
			BleHandleProStep = 0;
			break;
		}
		case BLE_FACE_SET_LINKKEY :
		{
			BleRxCmdHandlePro = E_CMD_CAM_LINKKEY;
			BleHandleProStep = 0;
			break;
		}
		case BLE_CAM_BELL_ONE_WAY :
		{
			BleRxCmdHandlePro = E_CMD_CAM_POWER_SAVING;
			BleHandleProStep = 0;
			break;
		}
		case BLE_CAM_GET_IP :
		{
			BleRxCmdHandlePro = E_CMD_CAM_GET_IP;
			BleHandleProStep = 0;
			break;
		}
		#endif
		case BLE_DOOR_UNLOCK_WARM: //��δ�ر�������
		{
			SystemSeting.DoorUnlockWarmSw=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.DoorUnlockWarmSw,1);//д����
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���				
			break;
		}
		case BLE_LOCK_CHECK	:     //����ȷ�ϼ�
		{
			SystemSeting.Sysprotect_lock=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.Sysprotect_lock,1);//д����
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���				
			break;
		}
		case BLE_LOCK_CASE_LOG:   //�¼���¼
		{
			BleRxCmdHandlePro = E_CMD_BLE_LOCK_CASE;
			BleHandleProStep = 0;
			break;
		}
		case BLE_CLEAR_LCOK_LOG:
			SystemEventLogClear();
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���	
			break;
		case BLE_WIFI_TEST :      //WIFI����ָ������
		{
			memcpy(WifiLockMeg.WifiFactoryTestNum,&AppBleType.RxDataBuf[L_DATA],19);//��������
			BleRxCmdHandlePro = E_CMD_WIFI_TEST;
			break;
		}	
		case BLE_MODEL_WRITE_TEST :  //BLE���������ͺ� 
		{
			memcpy(SystemFixSeting.LockNameType,&AppBleType.RxDataBuf[L_DATA],8);//�����ͺ�
			SystemWriteFixSeting(SystemFixSeting.LockNameType,8); //�洢
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���					
			break;
		}	
		
		case BLE_GET_LOCK_WIFI_MAC: //��ȡMAC����ʾ����Ա���
			
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );      //����
			App_LED_OutputCtrl((LED_TYPE_E)led_num[0x0F & AppBleType.RxDataBuf[L_DATA]],EM_LED_ON);
			App_LED_OutputCtrl((LED_TYPE_E)led_num[0x0F & AppBleType.RxDataBuf[L_DATA+1]],EM_LED_ON);
			App_LED_OutputCtrl((LED_TYPE_E)led_num[0x0F & AppBleType.RxDataBuf[L_DATA+2]],EM_LED_ON);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxDataBuf[L_DATA+1]=6; 
			PUBLIC_GetMacAdd(&AppBleType.Respond.TxDataBuf[L_DATA+2]); //��ȡ����MAC
			AppBleType.Respond.TxDataBuf[L_DATA+2+6]=6; 
			memset(&AppBleType.Respond.TxDataBuf[L_DATA+2+6+1],0,6); //WIFI MAC��
			AppBleType.Respond.TxLen=15; //�ذ���				
		    break;
		case BLE_MODEL_READ_TEST :   //��ȡ�ͺ�
		{
			SystemReadFixSeting(SystemFixSeting.LockNameType,8); //���洢
			AppBleType.Respond.TxLen=9; //�ذ���		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],SystemFixSeting.LockNameType,8);//�����ͺ�
			break;
		}	
		
		case BLE_EEPROM_WRITE_TEST : //���������
		case BLE_EEPROM_READ_TEST:
		{
			AppBleType.Respond.TxLen=1; //�ذ���		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			break;
		}
		case BLE_READ_ADC_TEST :
		{
			uint32_t upper_bat_data  = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxDataBuf[L_DATA+1] = (uint8_t)(upper_bat_data  >> 8);//������ʾ
			AppBleType.Respond.TxDataBuf[L_DATA+2] = (uint8_t)upper_bat_data;//������ʾ
			AppBleType.Respond.TxLen=3; //�ذ���	
			break;
		}
		case BLE_READ_ADC2_TEST :
		{
			uint32_t upper_bat_data  = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxDataBuf[L_DATA+1] = (uint8_t)(upper_bat_data  >> 8);//������ʾ
			AppBleType.Respond.TxDataBuf[L_DATA+2] = (uint8_t)upper_bat_data;//������ʾ
			AppBleType.Respond.TxLen=3; //�ذ���	
			break;
		}
		case BLE_LED_RED_GREEN_TEST :
			App_LED_OutputCtrl(EM_LED_ALL,EM_LED_ON);			
			AppBleType.Respond.TxLen=1; //�ذ���		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			break;
		case BLE_READ_TIME_TEST:
			HAL_RTC_ReadTime();
			AppBleType.Respond.TxLen = 8;
			AppBleType.Respond.TxDataBuf[L_DATA+0] = 0;
			AppBleType.Respond.TxDataBuf[L_DATA+1] = 0x20;
			AppBleType.Respond.TxDataBuf[L_DATA+2] = Math_Bcd2Bin ( Rtc_Real_Time.year );
			AppBleType.Respond.TxDataBuf[L_DATA+3] = Math_Bcd2Bin ( Rtc_Real_Time.month );
			AppBleType.Respond.TxDataBuf[L_DATA+4] = Math_Bcd2Bin ( Rtc_Real_Time.day );
			AppBleType.Respond.TxDataBuf[L_DATA+5] = Math_Bcd2Bin ( Rtc_Real_Time.hour );
			AppBleType.Respond.TxDataBuf[L_DATA+6] = Math_Bcd2Bin ( Rtc_Real_Time.minuter );
			AppBleType.Respond.TxDataBuf[L_DATA+7] = Math_Bcd2Bin ( Rtc_Real_Time.second );
			break;
		case BLE_ADDFINGER_ACK :     //ȷ��ָ��¼��ɹ�
		{
			break;
		}
		case BLE_DELFINGER	:        //ɾ��ָ��
		{
			ComDataByte2 = ((uint16_t)AppBleType.RxDataBuf[L_DATA+2]<<8) + (uint16_t)AppBleType.RxDataBuf[L_DATA+3];  
			BleRxCmdHandlePro = E_CMD_DELETE_FINGER;
			BleHandleProStep = 0;
			break;
		}
		case BLE_FINGER_WRTITE :     //�޸�ָ������
		{
			FingerAppCfg_S fingerMeg = {0};
			// ָ��ʹ��
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_EN ] = MEM_FACT_MEM_FIG;   
			// ģ����H
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_NUM_H ] = AppBleType.RxDataBuf[L_DATA+2];   
			// ģ����L
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_NUM_L ] = AppBleType.RxDataBuf[L_DATA+3];  
			// ����Ա���
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ] = MEM_USER_MASTER;  
			// ������
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_FAMILY_EN ] = AppBleType.RxDataBuf[L_DATA+12]&0xf0;  
			// SOSв�ȱ��
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_SOS_EN ] = AppBleType.RxDataBuf[L_DATA+12]&0x0f;  
			// ʱЧ���
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_TIME_EN ] = AppBleType.RxDataBuf[L_DATA+13];  
			// ��ʼʱ��: ������ʱ����   0x08
			(void)memcpy( &fingerMeg.acOffset[ EM_FINGER_APP_CFG_START_YEAR ], &AppBleType.RxDataBuf[L_DATA+14], 6 );
			// ����ʱ��: ����    
			(void)memcpy( &fingerMeg.acOffset[ EM_FINGER_APP_CFG_END_YEAR ], &AppBleType.RxDataBuf[L_DATA+20], 2 );
			// ����ʱ��: ��ʱ����       0x10
			(void)memcpy( &fingerMeg.acOffset[ EM_FINGER_APP_CFG_END_DAY ], &AppBleType.RxDataBuf[L_DATA+22], 4 );
			// ��
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_WEEK ] = AppBleType.RxDataBuf[L_DATA+26];  
 
            APP_FINGER_CfgWrite( fingerMeg );
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���		
			break;
		}
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
		case BLE_CMD_SET_FOV_PARAM :     // ��ȡWIFI�ź�ǿ��
		{
			BleRxCmdHandlePro = E_CMD_SET_FOV_PARAM;
			BleHandleProStep = 0;
			break;
		}
#endif

		default :
//			AppBleType.Respond.TxLen=1; //�ذ���		
//			AppBleType.Respond.TxDataBuf[L_DATA]=0x33; //δָ֪��
			break;
	}
	if(AppBleType.Respond.TxLen)
	{
		AppBleRespondData(AppBleType.RxCdm);//ͳһ�ذ�
	}
}


/***************************************************************************************
**������:       BleFaceCasePro
**��������:     ����ģ���������ݴ���
**�������:     
**�������:     
**��ע:         
****************************************************************************************/
void BleFaceCasePro(void)
{
	#ifdef  FACE_FUNCTION_ON
	static uint8_t result = 0;
	if (BLE_FACEADD==AppBleType.RxCdm)//��ǰ�����¼�������������
	{
		if(AppFaceWorkPro.Register==FACE_ADD_FRONT) //��һ�ν���
		{
			if(SystemSeting.SysFaceAllNum>=MSG_FACE_USER_NUM)
			{
				AppBleType.Respond.TxLen=1; //�ذ���		
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ�ܣ���������
				AppBleRespondData(AppBleType.RxCdm);//�ذ�
				AppBleType.RxCdm=0; //����
				return;
			}
			else 
			{
				AppBleType.Respond.TxLen=2; //�ذ���		
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //�Ǽ�������
				AppBleType.Respond.TxDataBuf[L_DATA+1]=0; //��ʼע��	
				AppBleRespondData(AppBleType.RxCdm);//�ذ�	
			}	
	    }		
		uint8_t sta=FaceEnrollPro(MEM_USER_MASTER);//��������
		static uint8_t sta_copy=0;
		if(sta!=sta_copy) //ָ��ֻ��һ��
		{
			sta_copy=sta;
			switch(sta)
			{
			case FACE_ADD_UP: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=1;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;
			case FACE_ADD_DOWN: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=2;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;
			case FACE_ADD_LEFT: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=4;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;
			
			case FACE_ADD_SUCCESSFUL: //�Ǽ�������һ������ �洢
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=5;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);	
				result = 1;//�ɹ�
				break;
			
			case FACE_ADD_RIGHT: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=3;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;	
			
			case FACE_ADD_ERROR:
				result = 2;//ʧ��
				break;
			
			case FACE_ADD_OVER: //�µ����
				if(result == 1)//�ɹ�
				{
					AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
					AppBleType.Respond.TxDataBuf[L_DATA+1]=FaceAttribute.FacePageId<<8;//��ID
					AppBleType.Respond.TxDataBuf[L_DATA+2]=FaceAttribute.FacePageId&0xff;//��ID
					AppBleType.Respond.TxLen=3; //�ذ���
					AppBleRespondData(BLE_FACEADD);
				}
				else if(result == 2)//ʧ��
				{
					AppBleType.Respond.TxDataBuf[L_DATA]=1;//ʧ��
					AppBleType.Respond.TxLen=1; //�ذ���
					AppBleRespondData(BLE_FACEADD);
					App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
				}
				result = 0;
				AppFaceWorkPro.Register = FACE_ADD_FRONT;
				AppBleType.RxCdm=0;//ֱ�ӽ���
				break;
			default:
				break;
			}
		}
	}
	else if(BLE_FACESUCSSFUL==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
		AppBleType.Respond.TxLen=1; //�ذ���
		AppBleType.RxCdm=0; //ֱ�ӽ���
		AppBleRespondData(BLE_FACESUCSSFUL);//�ظ�
	}
	else if(BLE_FACE_WRTITE==AppBleType.RxCdm)
	{
		uint16_t FaceId = 0;
		FaceId = AppBleType.RxDataBuf[L_DATA];
		FaceId <<= 8;
		FaceId |= AppBleType.RxDataBuf[L_DATA + 1];
		FaceRead(FaceAttribute, FaceId);
		my_printf("FaceId = %d\n", FaceId);
		FaceAttribute.tm_vaild.fig = AppBleType.RxDataBuf[L_DATA + 3];
		my_printf("FaceAttribute.tm_vaild.fig = %d\n", FaceAttribute.tm_vaild.fig);
		//ʱЧ��ʼ
		FaceAttribute.tm_vaild.start.year = AppBleType.RxDataBuf[L_DATA + 4];
		FaceAttribute.tm_vaild.start.month = AppBleType.RxDataBuf[L_DATA + 5];
		FaceAttribute.tm_vaild.start.day = AppBleType.RxDataBuf[L_DATA + 6];
		FaceAttribute.tm_vaild.start.hour = AppBleType.RxDataBuf[L_DATA + 7];
		FaceAttribute.tm_vaild.start.minuter = AppBleType.RxDataBuf[L_DATA + 8];
		FaceAttribute.tm_vaild.start.second = AppBleType.RxDataBuf[L_DATA + 9];
		//ʱЧ��ʼ
		FaceAttribute.tm_vaild.stop.year = AppBleType.RxDataBuf[L_DATA + 10];
		FaceAttribute.tm_vaild.stop.month = AppBleType.RxDataBuf[L_DATA + 11];
		FaceAttribute.tm_vaild.stop.day = AppBleType.RxDataBuf[L_DATA + 12];
		FaceAttribute.tm_vaild.stop.hour = AppBleType.RxDataBuf[L_DATA + 13];
		FaceAttribute.tm_vaild.stop.minuter = AppBleType.RxDataBuf[L_DATA + 14];
		FaceAttribute.tm_vaild.stop.second = AppBleType.RxDataBuf[L_DATA + 15];
		//��ʱЧ BIT1 -BIT 7��ʾ��һ������
		FaceAttribute.tm_vaild.wday = AppBleType.RxDataBuf[L_DATA + 16];
		
		FaceWrite(FaceAttribute, FaceId);
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
		AppBleType.Respond.TxLen=1; //�ذ���
		AppBleType.RxCdm=0; //ֱ�ӽ���
		AppBleRespondData(BLE_FACE_WRTITE);//�ظ�
	}	
	else if(BLE_FACE_DEL==AppBleType.RxCdm)
	{
		if(FaceDeleteAppUser(AppBleType.RxDataBuf[L_DATA+1]))//ɾ�������û����, �����ֽ�
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			AppBleType.RxCdm=0; //ֱ�ӽ���
			AppBleRespondData(BLE_FACE_DEL);//�ظ�
		}
	}
	else if(BLE_ADDFINGER_ACK==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
		AppBleType.Respond.TxLen=1; //�ذ���
		AppBleType.RxCdm=0; //ֱ�ӽ���
		AppBleRespondData(BLE_ADDFINGER_ACK);//�ظ�
	}
	else if(BLE_FACE_VERSION==AppBleType.RxCdm) //��ȡ�汾��
	{
		uint8_t Task=FaceGneralTaskFlow(FACE_CMD_GETVERSION,0,0,FACE_DEFAULT_TIMEOUT_1S);
		if(Task==TASK_SUCCESS)//��ȡ�ɹ�
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&FaceMsgType.Reply.DataPack.Version,sizeof(FaceMsgType.Reply.DataPack.Version));
			AppBleType.Respond.TxLen=sizeof(FaceMsgType.Reply.DataPack.Version)+1; //�ذ���
		}
		else if(Task==TASK_POWERDOWN) //�µ����
		{
			AppBleRespondData(BLE_FACE_VERSION);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���			
		}
	}
	else if(BLE_FACE_HIJACK==AppBleType.RxCdm) //���ٳֿ��أ�����û��
	{
		//ֱ�ӵ��µ����
		if(FaceGneralTaskFlow(FACE_CMD_HIJACK_MODE,&AppBleType.RxDataBuf[L_DATA],1,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			AppBleRespondData(BLE_FACE_HIJACK);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���		
		}
	}
	else if(BLE_FACE_THRESHOLD_LEVEL==AppBleType.RxCdm)//ʶ��ȫ�ȼ�����
	{
		if(FaceGneralTaskFlow(FACE_CMD_SET_THRESHOLD_LEVEL,&AppBleType.RxDataBuf[L_DATA],2,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			AppBleRespondData(BLE_FACE_THRESHOLD_LEVEL);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���	
		}
	}	
	
	#elif defined IRIS_FUNCTION_ON
	if (BLE_FACEADD==AppBleType.RxCdm)//��ǰ�����¼�������������
	{
		if(AppFaceWorkPro.Register==FACE_ADD_FRONT) //��һ�ν���
		{
			if(SystemSeting.SysFaceAllNum>=MSG_FACE_USER_NUM)
			{
				AppBleType.Respond.TxLen=1; //�ذ���		
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ�ܣ���������
				AppBleRespondData(AppBleType.RxCdm);//�ذ�
				AppBleType.RxCdm=0; //����
				return;
			}
			else 
			{
				AppBleType.Respond.TxLen=2; //�ذ���		
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //�Ǽ�������
				AppBleType.Respond.TxDataBuf[L_DATA+1]=0; //��ʼע��	
				AppBleRespondData(AppBleType.RxCdm);//�ذ�	
			}	
	    }		
		uint8_t sta=FaceEnrollPro(MEM_USER_MASTER);//��������
		static uint8_t sta_copy=0;
		if(sta!=sta_copy) //ָ��ֻ��һ��
		{
			sta_copy=sta;
			switch(sta)
			{
			case FACE_ADD_LEFT: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=3;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;	
			
			case FACE_ADD_SUCCESSFUL: //�Ǽ�������һ������ �洢
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//ִ����
				AppBleType.Respond.TxDataBuf[L_DATA+1]=5;//���һ������
				AppBleType.Respond.TxLen=2; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;
			
			case FACE_ADD_ERROR:
				App_LED_OutputCtrl( EM_LED_CFG_NET, EM_LED_ON ); //����25846
				AppBleType.Respond.TxDataBuf[L_DATA]=1;//ʧ��
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				break;
			
			case FACE_ADD_OVER: //�µ����
				AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
				AppBleType.Respond.TxDataBuf[L_DATA+1]=FaceAttribute.FacePageId<<8;//��ID
				AppBleType.Respond.TxDataBuf[L_DATA+2]=FaceAttribute.FacePageId&0xff;//��ID
				AppBleType.Respond.TxLen=3; //�ذ���
				AppBleRespondData(BLE_FACEADD);
				AppBleType.RxCdm=0;//ֱ�ӽ���
				break;

			default:
				break;
			}
		}
	}
	else if(BLE_FACESUCSSFUL==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
		AppBleType.Respond.TxLen=1; //�ذ���
		AppBleType.RxCdm=0; //ֱ�ӽ���
		AppBleRespondData(BLE_FACESUCSSFUL);//�ظ�
	}
	else if(BLE_FACE_WRTITE==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
		AppBleType.Respond.TxLen=1; //�ذ���
		AppBleType.RxCdm=0; //ֱ�ӽ���
		AppBleRespondData(BLE_FACE_WRTITE);//�ظ�
	}	
	else if(BLE_FACE_DEL==AppBleType.RxCdm)
	{
		if(FaceDeleteAppUser(AppBleType.RxDataBuf[L_DATA+1]))//ɾ�������û����, �����ֽ�
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			AppBleType.Respond.TxLen=1; //�ذ���
			AppBleType.RxCdm=0; //ֱ�ӽ���
			AppBleRespondData(BLE_FACE_DEL);//�ظ�
		}
	}
	else if(BLE_ADDFINGER_ACK==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
		AppBleType.Respond.TxLen=1; //�ذ���
		AppBleType.RxCdm=0; //ֱ�ӽ���
		AppBleRespondData(BLE_ADDFINGER_ACK);//�ظ�
	}
	else if(BLE_FACE_VERSION==AppBleType.RxCdm) //��ȡ�汾��
	{
		uint8_t Task=FaceGneralTaskFlow(FACE_CMD_GETVERSION,0,0,FACE_DEFAULT_TIMEOUT_1S);
		if(Task==TASK_SUCCESS)//��ȡ�ɹ�
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&FaceMsgType.Reply.DataPack.Version,sizeof(FaceMsgType.Reply.DataPack.Version));
			AppBleType.Respond.TxLen=sizeof(FaceMsgType.Reply.DataPack.Version)+1; //�ذ���
		}
		else if(Task==TASK_POWERDOWN) //�µ����
		{
			AppBleRespondData(BLE_FACE_VERSION);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���			
		}
	}
	else if(BLE_FACE_HIJACK==AppBleType.RxCdm) //���ٳֿ��أ�����û��
	{
		//ֱ�ӵ��µ����
		if(FaceGneralTaskFlow(FACE_CMD_HIJACK_MODE,&AppBleType.RxDataBuf[L_DATA],1,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			AppBleRespondData(BLE_FACE_HIJACK);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���		
		}
	}
	else if(BLE_FACE_THRESHOLD_LEVEL==AppBleType.RxCdm)//�����������ù���
	{
		
	}	
	#endif
		
	#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 	
	else if(BLE_FACE_SET_LINKKEY==AppBleType.RxCdm)//���ð�������Կ
	{
		if(FaceGneralTaskFlow(FACE_CMD_SET_LINKKEY,&AppBleType.RxDataBuf[L_DATA],96*2,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			AppBleRespondData(BLE_FACE_SET_LINKKEY);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���	
		}
	}	
	else if(BLE_CAM_GET_SN==AppBleType.RxCdm)//��ȡè�����к�
	{
		uint8_t Task=FaceGneralTaskFlow(FACE_CMD_DEVICENAME,0,0,FACE_PASS_DATA_TIMEOUT_5S);
		if(Task==TASK_SUCCESS)
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//�ɹ�
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&FaceMsgType.Reply.DataPack.Data,32);
			AppBleType.Respond.TxLen=33; //�ذ���
		}
		else if(Task==TASK_POWERDOWN)
		{
			AppBleRespondData(BLE_CAM_GET_SN);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���	
		}
	}	
	else if(BLE_CMD_SET_WIFI==AppBleType.RxCdm)//����
	{
		int8_t Result=FaceProSetSSID(&AppBleType.RxDataBuf[L_DATA],98);
		if( Result != -1)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=Result;//���
			AppBleRespondData(BLE_CMD_SET_WIFI);//�ظ�
			AppBleType.RxCdm=0; //ֱ�ӽ���	
		}
	}	
	#endif

	return;
}

/***************************************************************************************
**������:       BleFingerCasePro
**��������:     ��ָģ���������ݴ���
**�������:     
**�������:     
**��ע:         
****************************************************************************************/
void BleFingerCasePro(void)
{
    /* ��ǰ�ӿ�ֻ������APP ģʽ��¼��ָ�� */
    //if(ADMIN_APP_REGISTERED != App_GUI_GetRegisterSts()) { return; }

    static FINGER_APP_ADD_FLOW_E emFlag = FINGER_APP_ADD_OVER;
    static uint16_t u16ID = 0;
    
    /* ��ǰ�����¼������������� */
	if(BLE_ADDFINGER_NEW == AppBleType.RxCdm)//
	{
        if( 0 == DRV_GetBleConnect())
        {
            AppBleType.RxCdm = 0; //����
            (void)APP_FINGER_Sleep();
            emFlag = FINGER_APP_ADD_OVER;
            (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
            (void)HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );	
            return;
        }
        
        uint16_t u16Pad = 0;
        FINGER_APP_FLOW_RESULT_E emRet = APP_FINGER_GetFlowResult(&u16Pad);
	    FINGER_APP_ADD_FLOW_E em = APP_FINGER_GetAddFingerFlow();

	    /* ָ��ģ����״̬����¼������״̬ */
        if(FINGER_APP_RESULT_IDLE == emRet)
        {
            (void)App_LED_OutputCtrl(EM_LED_ALL, EM_LED_OFF);
            emFlag = em;
            /* ������ֱ�ӷ��أ�������������ָ�� */
            if(false == APP_FINGER_CfgGetNullNum(EM_FINGER_APP_TYPE_ADMIN,&u16ID))
            {
				AppBleType.Respond.TxLen = 1; //�ذ���
				AppBleType.Respond.TxDataBuf[L_DATA] = 1; //ʧ�ܣ���������
				AppBleRespondData(AppBleType.RxCdm);//�ذ�
				AppBleType.RxCdm = 0; //����
                (void)APP_FINGER_Sleep();
				emFlag = FINGER_APP_ADD_OVER;
                (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
                (void)HAL_Voice_PlayingVoice( EM_REGIST_CNT_FULL_MP3, 150 );	
				return;
            }

            FingerAppParam_S fingerPara;
    		memset( (void*)&fingerPara, 0, sizeof fingerPara );
            fingerPara.emAppFlow = EM_FINGER_APP_FLOW1_ADD;
    		fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ] = MEM_USER_MASTER;
            /* ��������ָ������ ,����ʧ�ܣ�������������ָ��*/
    		if(false == APP_FINGER_Operate( fingerPara ))
            {
				AppBleType.Respond.TxLen = 1; //�ذ���
				AppBleType.Respond.TxDataBuf[L_DATA] = 1; //ʧ�ܣ���������
				AppBleRespondData(AppBleType.RxCdm);//�ذ�
				AppBleType.RxCdm = 0; //����
                (void)APP_FINGER_Sleep();
				emFlag = FINGER_APP_ADD_OVER;
                (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
                (void)HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );	
				return;
            }

            /* APP_FINGER_Operate ����true�����̿�ʼ��ת */
			AppBleType.Respond.TxLen = 2; //�ذ���
			AppBleType.Respond.TxDataBuf[L_DATA] = 2; //�Ǽ�������
			AppBleType.Respond.TxDataBuf[L_DATA+1] = 0; //��ʼע��	
			AppBleRespondData(BLE_ADDFINGER_NEW);//�ذ�
        }
        /* ¼��ָ��1 ~ 6  */
        else if(em >= FINGER_APP_ADD_1ST && em <= FINGER_APP_ADD_6TH && emFlag != em)
        {
            emFlag = em;
			AppBleType.Respond.TxDataBuf[L_DATA] = 2;//ִ����
			AppBleType.Respond.TxDataBuf[L_DATA+1] = em;//���һ������
			AppBleType.Respond.TxLen = 2; //�ذ���
			AppBleRespondData(BLE_ADDFINGER_NEW);

            if(FINGER_APP_ADD_2ND == em || FINGER_APP_ADD_4TH== em)
            {
    			AppBleType.Respond.TxDataBuf[L_DATA] = 0;
    			AppBleType.Respond.TxDataBuf[L_DATA+1] = 0;//PageID - 1st byte
    			AppBleType.Respond.TxDataBuf[L_DATA+2] = 0;//PageID - 2nd byte
    			AppBleType.Respond.TxDataBuf[L_DATA+3] = 0;//PageID - 3rd byte
    			AppBleType.Respond.TxDataBuf[L_DATA+4] = u16ID;//PageID - 4th byte
    			AppBleType.Respond.TxDataBuf[L_DATA+5] = em;//���һ������
    			AppBleType.Respond.TxLen = 6; //�ذ���
    			AppBleRespondData(BLE_ADDFINGER_NEW);
            }
        }
        /* ���¼��  */
        else if(FINGER_APP_RESULT_SUC == emRet || FINGER_APP_RESULT_FAIL == emRet || FINGER_APP_RESULT_TIMEOUT == emRet)
        {
            emFlag = em;
            if(FINGER_APP_RESULT_SUC == emRet)
            {
    			AppBleType.Respond.TxDataBuf[L_DATA] = 0;
    			AppBleType.Respond.TxDataBuf[L_DATA+1] = 0;//PageID - 1st byte
    			AppBleType.Respond.TxDataBuf[L_DATA+2] = 0;//PageID - 2nd byte
    			AppBleType.Respond.TxDataBuf[L_DATA+3] = 0;//PageID - 3rd byte
    			AppBleType.Respond.TxDataBuf[L_DATA+4] = u16ID;//PageID - 4th byte
    			AppBleType.Respond.TxDataBuf[L_DATA+5] = FINGER_APP_ADD_6TH;//���һ������
    			AppBleType.Respond.TxLen = 6; //�ذ���
    			AppBleRespondData(BLE_ADDFINGER_NEW);
            }
            else
            {
    			AppBleType.Respond.TxDataBuf[L_DATA] = 1;
    			AppBleType.Respond.TxLen = 1; //�ذ���
    			AppBleRespondData(BLE_ADDFINGER_NEW);
                (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
                (void)HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );	
            }
            AppBleType.RxCdm = 0; //ֱ�ӽ���
#ifdef FINGER_VEIN_FUNCTION_ON
            SystemEventLogSave( ADD_VEIN, u16ID );
#else
            SystemEventLogSave( ADD_FINGER, u16ID );
#endif
            (void)APP_FINGER_Sleep();
            
        }
	}
	else if(BLE_ADDFINGER_ACK == AppBleType.RxCdm)
	{
        AppBleType.Respond.TxDataBuf[L_DATA] = 0;//�ɹ�
        AppBleType.Respond.TxDataBuf[L_DATA+1] = 0;
	    AppBleType.Respond.TxLen = 2; //�ذ���
        AppBleRespondData(AppBleType.RxCdm);
        AppBleType.RxCdm = 0; //ֱ�ӽ���
        (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
        HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, 150 );	
	}
	else if(BLE_GET_FINGERID == AppBleType.RxCdm)
	{
	    uint8_t u8IdBuf[MSG_FINGER_NUM_RESERVED] = {0};
        uint8_t len = 0;

        if(APP_FINGER_GetFingerID(MSG_FINGER_NUM_RESERVED, u8IdBuf, &len))
        {
            AppBleType.Respond.TxDataBuf[L_DATA] = 0;//�ɹ�
            for(uint8_t i = 0 ; i < len; i++) {AppBleType.Respond.TxDataBuf[L_DATA+i+1] = u8IdBuf[i];}
    	    AppBleType.Respond.TxLen = len+1; //�ذ���
        }
        else
        {
            AppBleType.Respond.TxDataBuf[L_DATA] = 1;//ʧ��
    	    AppBleType.Respond.TxLen = 1; //�ذ���
        }
	
        AppBleRespondData(AppBleType.RxCdm);
        AppBleType.RxCdm = 0; //ֱ�ӽ���
	}

    return;
}
 
/***************************************************************************************
**������:       BLE_GetFifoData
**��������:     �������ݽ���
**�������:      
**�������:     ���1���ɹ��հ�
**��ע:        
****************************************************************************************/
uint8_t BLE_GetFifoData (void)
{
	uint8_t pdata=0;
	uint16_t len; //����
	do  //ȡ����ֱ������Ϊ�� ���� ȡ������һ��
	{
		if(AppBleType.RxPos==0) //������
		{
			if( app_fifo_get(&AppBleFifo, &pdata ) == FIFO_SUCCESS) //����ȡ����
			{
				if(BLE_APP_HEAD==pdata) //��ͷ
				{
					AppBleType.RxDataBuf[AppBleType.RxPos++]=pdata; //��ʼ����
				}				
			}
			else
			{
				return 0; //������
			}
		}
		else
		{
			if(AppBleType.RxPos>=L_DATA)//�жϰ���
			{
				len=AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL];
			}			
			//һ������(L_HEAD+L_TYPE+L_CMD+L_LENH+L_LENL) +len +(CRCH+CRCL)
			if((5+len+2) ==AppBleType.RxPos)
			{
				AppBleType.RxPos=0;
				App_GUI_UpdateMenuQuitTime(60*100, true); //��������ʱ��
				BLEParse();				
				return 1;
			}
			else
			{
				if( app_fifo_get(&AppBleFifo, &pdata ) == FIFO_SUCCESS) //����ȡ����
				{
					AppBleType.RxDataBuf[AppBleType.RxPos++]=pdata;//��������
				} 
				else
				{
					return 0; //������
				}
			}
		}
    }while(1);
}  

/***************************************************************************************
**������:       APP_BleSetAdminState
**��������:     ���õ�ǰȨ��״̬
**�������:      
**�������:     
**��ע:        
****************************************************************************************/
void APP_BleSetAdminState(ADMIN_STATE mode)
{
	AppBleType.Admin=mode;
}

/***************************************************************************************
**������:       APP_BleGetAdminState
**��������:     ��ȡ��ǰȨ��״̬
**�������:      
**�������:     
**��ע:        
****************************************************************************************/
ADMIN_STATE APP_BleGetAdminState(void)
{
	return AppBleType.Admin;
}

/*********************************************************************************************************************
* Function Name :  App_BleDelFingerPro()
* Description   :  ɾ����Ӧ�ı�ŵ�ָ������   
* Para  Input   :  pageId: ��ɾ����ָ�Ʊ��
* Return        :  -1= ɾ��ʧ��  0= ִ����  1= ɾ���ɹ�
*********************************************************************************************************************/
static int8_t App_BleDelFingerPro( uint16_t pageId, uint8_t *pfisrtFlg )
{
    FingerAppParam_S fingerPara;
	FINGER_APP_FLOW_RESULT_E ret; 
	uint16_t pageid;
	static uint8_t step;
	if( *pfisrtFlg == 0 )
	{
		*pfisrtFlg = 1;
		step = 0;
	}
	switch( step )
	{
		case 0:   //��������
				memset( (void*)&fingerPara, 0, sizeof fingerPara );
				fingerPara.emAppFlow = EM_FINGER_APP_FLOW2_DEL;
				fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_NUM_H ] = (uint8_t)(pageId >> 8);
		        fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_NUM_L ] = (uint8_t)(pageId >> 0);
				APP_FINGER_Operate( fingerPara );
				step = 1;	
		break;
		
		case 1:   //�ȴ����
			    ret = APP_FINGER_GetFlowResult( &pageid );
				if( FINGER_APP_RESULT_SUC == ret )         //���ӳɹ�
				{
					APP_FINGER_Sleep();  //�ر�ָ��ģ��
					step = 0;
					return 1; 	
				}
				else if( FINGER_APP_RESULT_FAIL == ret )   //����ʧ��
				{
					APP_FINGER_Sleep();  //�ر�ָ��ģ��
					step = 0;
					return -1; 
				}
		break;
		
		default: break;
	}
	
	return 0;
}

/*********************************************************************************************************************
* Function Name :  App_BleDeviceRegThread
* Description   :  APPע�����������
* Para          :  none
* Return        :  none
*********************************************************************************************************************/
static int8_t App_BleDeviceRegThread(  uint8_t *pfisrtFlg )
{
	if( *pfisrtFlg == 0 )
	{
		*pfisrtFlg = 1;
		if(SystemSeting.SystemAdminRegister!=ADMIN_NONE_REGISTERED)
		{
			return 2;//����ģʽ��APPע��󣬶���Ҫ��ʾӲ���
		}
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );      //ɨ�����ּ�
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );    //��ȫ��		
	}	
	if( App_Touch_GetCurrentKeyIndex() >= 4 )//��ȡ��ֵ����
	{
		uint8_t  buflen = 0;
		uint8_t inputBuf[4]={0};
		App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
		PUBLIC_ChangeDecToString( inputBuf, inputBuf, 4 );	 //ת�ַ���
		
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );      //����
		App_LED_OutputCtrl( EM_LED_CFG_NET, EM_LED_ON ); 
		App_LED_OutputCtrl( EM_LED_LOCK_B, EM_LED_OFF ); 
		
		if(AppBleType.RxDataBuf[L_DATA]==inputBuf[0] &&
			AppBleType.RxDataBuf[L_DATA+1]==inputBuf[1] &&
			AppBleType.RxDataBuf[L_DATA+2]==inputBuf[2] &&
			AppBleType.RxDataBuf[L_DATA+3]==inputBuf[3] )
		{
			return 0;
		}
		else
		{
			return 1;//ʧ��
		}
	}
	return -1;
}


/*********************************************************************************************************************
* Function Name :  App_BleProHandleThread
* Description   :  ����ִ��ָ���   
* Para          :  none
* Return        :  none
*********************************************************************************************************************/
void App_BleProHandleThread( void )
{
	int8_t  tp1=0;
    static uint8_t taskid = 0;
	switch( BleRxCmdHandlePro )
	{
		case E_CMD_REG_PIN:
		{
			tp1 = App_BleDeviceRegThread( &BleHandleProStep );
			if( -1 != tp1 )   //���
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=tp1; 
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_REG_CHECK_CODE);
				AppBleType.RxCdm=0; 
			}
		}
		break;
		case E_CMD_UNREGIST_DEVICE: //ע���豸
		{
			 static uint8_t tryflg = 0;
			 if( BleHandleProStep == 0 )
				 tryflg =0;
			 if( tryflg == 0  )
			 {
				 tp1 = SystemParaBackFactoryIntoFlash( &BleHandleProStep );
				 if( 1 == tp1 )         //success
				 {
					 if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
					 {
						HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
					 }
					 tryflg = 1;
				 }
				 else if( -1 == tp1 )   //fail
				 {
					 if( EM_VOL_GRADE_OFF == SystemSeting.SysVoice )
					 {
						HAL_Voice_VolumeSet( EM_VOL_GRADE_HIGH );
					 }
					 tryflg = 2;
				 } 
			}
			 
			if( 1 == tryflg )      //�趨�ɹ�
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0;  
				AppBleType.Respond.TxLen=1;  
				AppBleRespondData(BLE_CLEAR_USERMEM);
				BleRxCmdHandlePro = E_CMD_DEFAULT;
				HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, 0 );
				App_GUI_UpdateMenuQuitTime(1*100, true); //��������ʱ��
				tryflg =0;
			}
			else if( 2 == tryflg ) //�趨ʧ��
			{
                App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
                App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
                App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
				HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 300 );
				tryflg = 3;
			}
			else if( 3 == tryflg )  //�ȴ������������
			{
				if( 0 == HAL_Voice_GetBusyState() )
				{
					tryflg = 4;
                    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
				}
			}
			else if( 4 == tryflg )  //�ȴ������������
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1;  
				AppBleType.Respond.TxLen=1;  
				AppBleRespondData(BLE_CLEAR_USERMEM);
				BleRxCmdHandlePro = E_CMD_DEFAULT;
				HAL_Voice_PlayingVoice( EM_SET_FAIL_MP3, 0 );
				App_GUI_UpdateMenuQuitTime(1*100, true); //��������ʱ��
				tryflg = 0;
			}
		}
		break;
		
		case E_CMD_ROUTER_CFG:      //·��������
		{
			#if defined XM_CAM_FUNCTION_ON //è������
				if(BleHandleProStep==0)
				{
					BleHandleProStep=1;
					taskid = CAM_SendCommandStart(CAM_CMD_SSID_SEND,&AppBleType.RxDataBuf[L_DATA],98);
				}
				else
				{
					tp1=CAM_GetServerState(taskid);
					if( CAM_SUCCESSFUL == tp1 )
					{
						AppBleType.Respond.TxDataBuf[L_DATA] =0;  
						AppBleType.Respond.TxLen =1;  
						AppBleRespondData(BLE_CMD_SET_WIFI);
						BleRxCmdHandlePro = E_CMD_DEFAULT;
					}
					else if( CAM_FAIL == tp1 ) 
					{
						AppBleType.Respond.TxDataBuf[L_DATA] =1;  
						AppBleType.Respond.TxLen =1;  
						AppBleRespondData(BLE_CMD_SET_WIFI);
						BleRxCmdHandlePro = E_CMD_DEFAULT;
					}
				}
			#else
             tp1 = App_WIFI_ConfigThread( WIFI_CMD_CONF_ROUTER, &BleHandleProStep );
				if( WIFI_CONFIG_SUCESS == tp1 )         //�ɹ�
				 {
					AppBleType.Respond.TxDataBuf[L_DATA] =0;  
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(BLE_CMD_SET_WIFI);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				 }
				 else if( WIFI_CONFIG_FAIL == tp1 )      //ʧ��
				 {
					AppBleType.Respond.TxDataBuf[L_DATA] =1;  
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(BLE_CMD_SET_WIFI);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				 }
			#endif
		}
		break;
		
		case E_CMD_SERVER_CFG:      //���������ã���WIFIģ��֧��
		{
             tp1 = App_WIFI_ConfigThread( WIFI_CMD_CONF_SERVER, &BleHandleProStep );
			 if( WIFI_CONFIG_SUCESS == tp1 )         //�ɹ�
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =0;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_CMD_SET_WIFI_IP);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			 }
			 else if( WIFI_CONFIG_FAIL == tp1 )      //ʧ��
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =1;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_CMD_SET_WIFI_IP);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT; 
			 }
		}
		break;
		
		case E_CMD_WIFI_FACTORY_TEST:      ///����ɨ��·����
		{
			//����Ҫʹ�ú꣬94�����Ƿ�ִ��
			 if(( WIFI_CONFIG_SUCESS == tp1 ) && (WifiRxData.AckResult == WIFI_ACK_OK))         //�ɹ�
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =0;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_WIFI_FACTORY_TEST);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			 }
			 else if(( WIFI_CONFIG_FAIL == tp1 ) || (WifiRxData.AckResult == WIFI_ACK_FAIL))     //ʧ��
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =1;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_WIFI_FACTORY_TEST);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT; 
			 }
		}
		break;
		
		case E_CMD_NEAR_SENSE_CFG:  //�ӽ���Ӧ����
		{
		     tp1 = App_HumanSensorSet( ComDataByte1, &BleHandleProStep );
		     if( tp1 == 1 )
			 {
				SystemSeting.SysDrawNear = ComDataByte1;
			    SystemWriteSeting(&SystemSeting.SysDrawNear,1);	
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData(BLE_IR_SET);
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			 }
			 else if( tp1 == -1 )
			 {
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ��
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData(BLE_IR_SET);
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			 } 
		}
		break; 
		
		case E_CMD_SERVER_TEST:     //����������
		{
			if(0 == BleHandleProStep)
            {
			    taskid = App_WIFI_CommomTx( WIFI_CMD_UPLOAD_UNLOCK_MEG );
				#if defined NB_FUNCTION
				App_NB_Start();
				#endif
                BleHandleProStep = 1;
            }
            else
            {
			    #if defined XM_CAM_FUNCTION_ON  //è��͸��
    			tp1 = CAM_GetServerState(taskid);
    			if( tp1 == CAM_SUCCESSFUL )
    			 {
    				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
    				AppBleType.Respond.TxLen=1; //�ذ���
    				AppBleRespondData(BLE_WIFI_CONNECTION_TEST);
    				BleRxCmdHandlePro = E_CMD_DEFAULT;  
    			 }
    			 else if( tp1 == CAM_FAIL) 
    			 {
    				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ��
    				AppBleType.Respond.TxLen=1; //�ذ���
    				AppBleRespondData(BLE_WIFI_CONNECTION_TEST);
    				BleRxCmdHandlePro = E_CMD_DEFAULT;  
    			 }
    			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
    			if(FaceProAlarm(UNLOCK_MEG, WifiTxTemp.data, WifiTxTemp.length) == 1)
    			{
    				AppBleType.Respond.TxDataBuf[L_DATA] =0;  
    				AppBleType.Respond.TxLen =1;  
    				AppBleRespondData(BLE_WIFI_CONNECTION_TEST);
    				//ble ack
    				BleRxCmdHandlePro = E_CMD_DEFAULT;
    			}
    			#else
    			if( 1 == APP_WIFI_TxState() )
    			{
    				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
    				AppBleType.Respond.TxLen=1; //�ذ���
    				AppBleRespondData(BLE_WIFI_CONNECTION_TEST);
    				BleRxCmdHandlePro = E_CMD_DEFAULT;  
    			}
    			#endif
            }
		}
		break; 
 
		case E_CMD_GET_WIFI_SIGNAL_INTENSITY:     //��ȡWIFI�ź�ǿ��
		{
			#if (defined XM_CAM_FUNCTION_ON) && (defined XM_CAM_SCREEN_FUNCTION_PLUS_ON)  //è��͸��
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_GetWifiSignalStrength();
			}
			else
			{
				tp1=CAM_GetServerState(taskid);
				if( CAM_SUCCESSFUL == tp1 )
				{
				    uint8_t u8Data = 0;
				    if(CAM_AnalysisWifiSignalStrength(&u8Data, taskid))
                    {
					    AppBleType.Respond.TxDataBuf[L_DATA] = 0;  
                    }
                    else
                    {
					    AppBleType.Respond.TxDataBuf[L_DATA] = 1;  
                    }
                    AppBleType.Respond.TxDataBuf[L_DATA + 1] = ~u8Data + 1;//ȥ������
                    AppBleType.Respond.TxLen = 2;
					AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				}
				else if( CAM_FAIL == tp1 ) 
				{
					AppBleType.Respond.TxDataBuf[L_DATA] = 1;  
                    AppBleType.Respond.TxDataBuf[L_DATA + 1] = 0;
					AppBleType.Respond.TxLen = 2;  
					AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				}
			}
            
			#elif defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
			static uint8_t intensity = 0;
			int8_t temp = FaceProGetWifiIntensity(&intensity);
			if(temp == 1)
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 0;  //�ɹ�
				AppBleType.Respond.TxLen = 2;  
				AppBleType.Respond.TxDataBuf[L_DATA + 1] = ~intensity + 1;//ȥ������
				AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(temp == -1)
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 1;//ʧ��  
				AppBleType.Respond.TxLen = 2;  
				AppBleType.Respond.TxDataBuf[L_DATA + 1] = 0;
				AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			#elif defined WIFI_FUNCTION_ON
			tp1 = App_WIFI_ConfigThread( WIFI_CMD_CONF_SERVER, &BleHandleProStep );
			if(tp1 == WIFI_CONFIG_SUCESS)
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 0;  //�ɹ�
				AppBleType.Respond.TxLen = 2;  
				AppBleType.Respond.TxDataBuf[L_DATA + 1] = WifiRxData.SignalIntensity;
				AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT; 
			}
			else if(tp1 == WIFI_CONFIG_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 1;//ʧ��  
				AppBleType.Respond.TxLen = 2;  
				AppBleType.Respond.TxDataBuf[L_DATA + 1] = 0;
				AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			#endif
			
			#if (defined NB_FUNCTION)
			uint8_t csqTmp =0;
			if(BleHandleProStep==0)
			{
				BleHandleProStep = 1;
				App_NB_GetCSQStart();
			}
			else if(1 == BleHandleProStep)
			{
				csqTmp = App_NB_GetCSQ();
				if(0 != csqTmp)
				{
					if(0Xff == csqTmp)
					{
						AppBleType.Respond.TxDataBuf[L_DATA] = 1;//ʧ��  
						AppBleType.Respond.TxLen = 2;  
						AppBleType.Respond.TxDataBuf[L_DATA + 1] = 0;
						AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
						BleRxCmdHandlePro = E_CMD_DEFAULT;
					}
					else 
					{
						AppBleType.Respond.TxDataBuf[L_DATA] = 0;  //�ɹ�
						AppBleType.Respond.TxLen = 2;  
						AppBleType.Respond.TxDataBuf[L_DATA + 1] = csqTmp;
						AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
						BleRxCmdHandlePro = E_CMD_DEFAULT;
					}
				}
			}
			#endif
		}
		break; 
        
#if (defined XM_CAM_FUNCTION_ON) && (defined XM_CAM_SCREEN_FUNCTION_PLUS_ON)
        case E_CMD_SET_FOV_PARAM:
        {
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SetFovParam(AppBleType.RxDataBuf[L_DATA]);
			}
			else
			{
				tp1=CAM_GetServerState(taskid);
				if( CAM_SUCCESSFUL == tp1 )
				{
					AppBleType.Respond.TxDataBuf[L_DATA] = 0;    
                    AppBleType.Respond.TxLen = 1;
					AppBleRespondData(E_CMD_SET_FOV_PARAM);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				}
				else if( CAM_FAIL == tp1 ) 
				{
					AppBleType.Respond.TxDataBuf[L_DATA] =1;  
					AppBleType.Respond.TxLen = 1;  
					AppBleRespondData(E_CMD_SET_FOV_PARAM);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				}
			}
        }
        break;
#endif
		
		case E_CMD_DELETE_FINGER: 	//ɾ��ָ��
		{
            tp1 = App_BleDelFingerPro( ComDataByte2, &BleHandleProStep );
			if( tp1 == 1 )         //success
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 0;  
				AppBleType.Respond.TxLen=1;  
			    AppBleRespondData( BLE_DELFINGER );
			    BleRxCmdHandlePro = E_CMD_DEFAULT;
                #ifdef FINGER_VEIN_FUNCTION_ON
                SystemEventLogSave( DELETE_VEIN, ComDataByte2 );
                #else
                SystemEventLogSave( DELETE_FINGER, ComDataByte2 );
                #endif
			}
			else if( tp1 == -1 )   //fail
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 1; 
				AppBleType.Respond.TxLen=1;  				
			    AppBleRespondData( BLE_DELFINGER );
			    BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		}
		break; 
		
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
		case E_CMD_FACE_PRO_OTA:       //è������
		{
			tp1 = FaceProOta();
			if( tp1 == 1 )         //success
			{
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			break;
		}
		#endif
		
		#if defined XM_CAM_FUNCTION_ON //è�ۻ�ȡ���к� �·���Կ
		case E_CMD_CAM_GET_SN:	  //��ȡè�����к�
		{ 
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_SN_GET,NULL,0);
			}
			else
			{
				tp1=CAM_GetServerState(taskid);
				if( CAM_SUCCESSFUL == tp1 )
				{
					AppBleType.Respond.TxDataBuf[L_DATA] =0;  
					AppBleType.Respond.TxLen=CAM_GetCameraData(CAM_CMD_SN_GET, &AppBleType.Respond.TxDataBuf[L_DATA + 1], taskid);
					AppBleType.Respond.TxLen +=1;  
					AppBleRespondData(BLE_CAM_GET_SN);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				}
				else if( CAM_FAIL == tp1 ) 
				{
					AppBleType.Respond.TxDataBuf[L_DATA] =1;  
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(BLE_CAM_GET_SN);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				}
			}
		}
		break;

		case E_CMD_CAM_POWER_SAVING:  //è�۵�˫���л�
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_SINGLE_MODE_SET,&AppBleType.RxDataBuf[L_DATA],1);
			}
			else if(CAM_GetServerState(taskid)==CAM_SUCCESSFUL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData( BLE_CAM_BELL_ONE_WAY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(CAM_GetServerState(taskid)==CAM_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ��
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData( BLE_CAM_BELL_ONE_WAY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		
		break;
		case E_CMD_CAM_GET_IP:  //��ȡè��IP
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_GET_IP,0,0);
			}
			else if(CAM_GetServerState(taskid)==CAM_SUCCESSFUL)
			{
				AppBleType.Respond.TxLen=CAM_GetCameraData(CAM_CMD_GET_IP,&AppBleType.Respond.TxDataBuf[L_DATA], taskid); //��L_DATA��ʼ����
				AppBleType.Respond.TxDataBuf[L_DATA] =0;
				AppBleType.Respond.TxLen +=1;  
				AppBleRespondData(BLE_CAM_GET_IP);
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(CAM_GetServerState(taskid)==CAM_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ��
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData( BLE_CAM_GET_IP );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		break;
		case E_CMD_CAM_LINKKEY://�·���������Կ
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_LINKKEY_SEND,&AppBleType.RxDataBuf[L_DATA],96*2);
			}
			else if(CAM_GetServerState(taskid)==CAM_SUCCESSFUL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData( BLE_FACE_SET_LINKKEY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(CAM_GetServerState(taskid)==CAM_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //ʧ��
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData( BLE_FACE_SET_LINKKEY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		break;
		#endif
		
		case E_CMD_WIFI_TEST:     //WIFI����ָ������
		{
			//���⹦��è�ۿ��Ҫ
			AppBleType.Respond.TxLen=1; //�ذ���		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
			WifiLockMeg.WifiFactoryTest = 1;
			App_WIFI_CommomTx(WIFI_CMD_UPLOAD_UNLOCK_MEG);
			#if defined NB_FUNCTION
            App_NB_Start();
            #endif
			AppBleRespondData(BLE_WIFI_TEST);
			BleRxCmdHandlePro = E_CMD_DEFAULT;  
		}
		break;
		
		#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
		case E_CMD_WIFI_MAIN_SW_POW_OFF:     //�ص�������Դ
		{
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)
			{
				my_printf("E_CMD_WIFI_MAIN_SW_POW_OFF\n");
				//���⹦��è�ۿ��Ҫ
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
				AppBleType.Respond.TxLen=1; //�ذ���
				AppBleRespondData(BLE_WIFI_MAIN_SW);
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			}
		}
		break;
		#endif
		
		case E_CMD_BLE_LOCK_CASE://��ȡ�¼���¼
		{
			static uint16_t LogPlace=0;
			uint32_t addr;
			if(BleHandleProStep==0)
			{
				LogPlace=1;	//λ������	
				if(SystemFixSeting.SysLockLogAll>MSG_LOG_RECORD_NUM) //�Ѿ�������
				{
					BleHandleProStep=1;
				}
				else
				{
					BleHandleProStep=2;
				}
			}
			else if(BleHandleProStep==1)//�Ѿ�������
			{
				if(LogPlace<=MSG_LOG_RECORD_NUM) //����600��
				{
					if(SystemFixSeting.SysLockLogSn>=LogPlace) //ǰ���
					{			
						addr= MSG_LOG_RECORD_START + (SystemFixSeting.SysLockLogSn-LogPlace) * MSG_LOG_RECORD_ONE_SIZE; //����洢��ַ
					}
					else //�����
					{
						addr= MSG_LOG_RECORD_START + (MSG_LOG_RECORD_NUM-(LogPlace-SystemFixSeting.SysLockLogSn)) * MSG_LOG_RECORD_ONE_SIZE; //����洢��ַ
					}
					//������
					HAL_EEPROM_ReadBytes(addr,&AppBleType.Respond.TxDataBuf[L_DATA+3],MSG_LOG_RECORD_REG_ONE_SIZE);			
					AppBleType.Respond.TxLen=17; //�ذ���
					AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�		
					AppBleType.Respond.TxDataBuf[L_DATA+1]=(MSG_LOG_RECORD_NUM )>>8; //�ܳ���
					AppBleType.Respond.TxDataBuf[L_DATA+2]=(MSG_LOG_RECORD_NUM )&0xff; //�ܳ���
					if(AppBleRespondData(BLE_LOCK_CASE_LOG))
					{
						LogPlace++;
					}
				}
				else
				{
					BleHandleProStep=3;
				}
			}
			else if(BleHandleProStep==2)//��û����
			{
				if(LogPlace<=SystemFixSeting.SysLockLogSn)
				{
					addr= MSG_LOG_RECORD_START + (SystemFixSeting.SysLockLogSn-LogPlace) * MSG_LOG_RECORD_ONE_SIZE; //����洢��ַ
					//������
					HAL_EEPROM_ReadBytes(addr,&AppBleType.Respond.TxDataBuf[L_DATA+3],MSG_LOG_RECORD_REG_ONE_SIZE);				
					AppBleType.Respond.TxLen=17; //�ذ���
					AppBleType.Respond.TxDataBuf[L_DATA+1]= (SystemFixSeting.SysLockLogSn )>>8; //��ǰ����
					AppBleType.Respond.TxDataBuf[L_DATA+2]= (SystemFixSeting.SysLockLogSn )&0xff; //��ǰ����	
					if(AppBleRespondData(BLE_LOCK_CASE_LOG))
					{
						LogPlace++;
					}				
				}
				else
				{
					BleHandleProStep=3;
				}
			}
			else if(BleHandleProStep==3)
			{
				BleHandleProStep=0;
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //�ɹ�
				AppBleType.Respond.TxLen=1; //�ذ���		
				AppBleRespondData(BLE_LOCK_CASE_LOG);
				App_GUI_UpdateMenuQuitTime(60*100, true); //��������ʱ��
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			}	
		}
		break;	
        default:
			     BleHandleProStep = 0;
		break;	
	}
}
/***************************************************************************************
**������:       APP_BleServerProcess
**��������:     �������ݽ���
**�������:      
**�������:     
**��ע:        
****************************************************************************************/
void APP_BleServerProcess(void )
{
	BLE_GetFifoData();  //�����հ�
	BleID2CasePro();    //ID2����
	
	BleFingerCasePro(); //ָ����������
	BleFaceCasePro();   //������������
	App_BleProHandleThread();  
	
}


 
//.end of the file.