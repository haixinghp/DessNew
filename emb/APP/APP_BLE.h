#ifndef _APP_BLE_H_
#define _APP_BLE_H_
	
#ifdef __cplusplus
	extern "C" {
#endif

/* ��׼ͷ�ļ� */
#include "stdint.h"

#include "DRV_BLE.h" 
/*-------------------------------------------------�궨��-----------------------------------------------------------*/ 
//Э���ʽ
#define L_HEAD				0   //ͷ
#define L_TYPE				1   //�����ֻ�01 ��09
#define L_CMD				2   //����
#define L_LENH				3	//���ݳ��ȸ��ֽ�
#define L_LENL				4   //���ݳ��ȵ��ֽ�
#define L_DATA				5   //���ݰ�

typedef enum
{
	E_DEFAULT,   				//��ͨģʽ����Ҫ�߼���
	E_KEY_ADMIN, 				//����������Աģʽ����������
	E_BLE_ADMIN,  			//�������ѹ���Աģʽ,��Ҫ�߼���
	E_VIDEO_ADMIN,			//����Կ������ָ���Ҫ�߼��ܣ��������ã�
	E_PHONE_CHECK_ERR,
	E_PHONE_CHECK_OK,
	E_SMARTKEKY_CHECK_ERR,
	E_SMARTKEKY_CHECK_OK,	
}ADMIN_STATE;

typedef struct
{
	struct
	{
		uint8_t  TxCmd;  //����֡����
		uint16_t TxLen;  //�����м����ݰ����ȣ�������ͷβ
		uint8_t  TxDataBuf[256];  //���ݷ���BUF
	}Respond; //�����ذ�
	
	ADMIN_STATE Admin; //��ǰ��������Ȩ��
	uint8_t RxDataBuf[256];  //���ݽ���BUF

	uint8_t RxPos;       //��ǰ����λ��
	uint8_t RxCdm;   //��ǰ�ֻ�����ָ��

	uint8_t ChannelPwd[4]; //�ŵ�����
	uint8_t RandomNum[8]; //������Ự��Կ
	
}APP_BLE_TYPE;


typedef enum{

	BLE_TIMEUPDATA		= 0x23,    //ʱ��ͬ��
	BLE_ADDFINGER		= 0x24,    //����ָ��
	
	BLE_PHONE_GETKEY	= 0x3A,    //��ȡ��Կ
	BLE_AES_ADDPHONE	= 0x7C,    //�����ֻ���AES���ܰ汾
	BLE_DELPHONE		= 0x29,    //ɾ���ֻ���
	BLE_PHONE_ENLOCK	= 0x39,    //�ֻ���������
	
	
	BLE_ADDFINGER_NEW   = 0x53,    //��ʼ����ָ��
	BLE_ADDFINGER_ACK   = 0x54,    //ȷ��ָ��¼��ɹ�
	BLE_GET_FINGERID    = 0x55,    //��ȡ��ע�������ָ�ƻ���ָID
	BLE_DELFINGER		= 0x28,    //ɾ��ָ��
	BLE_FINGER_WRTITE	= 0x2B,    //�޸�ָ������
		
	BLE_SMARTKEY_GETKEY1 = 0x3C,   //��ȡ��Կ
	BLE_SMARTKEY_GETKEY2 = 0x3E,   //��ȡ�����Կ ��3A,3Cһ��.
	BLE_ADDSMARTKEY_REQ  = 0x26,   //��������Կ����Կ����
	BLE_DELSMARTKEY		 = 0x2A,   //ɾ������Կ��
	BLE_SMARTKEY_ENLOCK  = 0x3B,   //����Կ�׿���
	
	BLE_AES_ADD_PASSWORD =0X79,    //������������
	BLE_AES_ADD_SOS_PASSWORD=0X7A, //SOS������������
    BLE_AES_ADD_TMP_PASSWORD=0X7B, //��ʱ��������
	
	
	BLE_REG_CHECK_CODE  = 0X42,      //ע��ȷ����
	BLE_DEVICE_REG		= 0x34,      //�豸ע��
	BLE_CLEAR_USERMEM	= 0x35,      //����û�
	BLE_ADMIN_PWD_CHECK = 0x36,      //�� ����������֤
	BLE_GET_HWIFO		= 0x37,      //��������汾�Ż�ȡ
	BLE_CMD_DISCONNECT	= 0xF0,      // �Ͽ���������
	BLE_CMD_OTAMODE		= 0xF4,      //���³��� ģʽ����Ӧ��	
	
	BLE_WIFI_CONNECTION_TEST = 0x40,   //���� ����������״̬
	BLE_WIFI_MAIN_SW         = 0x41,   //WiFi���ܿ����ر�
	BLE_SET_LOGUPWITHWIFI    = 0xF9,   //������¼�ϴ�����
	BLE_CMD_SET_WIFI	     = 0xF5,   // wifi��������  �ֻ�->��
	BLE_CMD_SET_WIFI_IP   	 = 0xF7,   // WIFI�������������á�
	BLE_GET_WIFI_SIGNAL_INTENSITY = 0x8D, // ��ȡWIFI�ź�ǿ��
	

	BLE_SE_VERIFY_TOKEN      = 0x65,   //ID2��֤TOKEN
	BLE_SE_CHALLENGE         = 0x66,   //ID2���¼���
	BLE_SE_SETMODE           = 0x67,   //ID2���أ�����Ҫ
	BLE_SE_GET_ID2               =0x6A,
	BLE_SE_ID2_TOKEN_NEW         =0x68,
	BLE_SE_ID2_BOTHWAY_VERIFY    =0x69,
	
	BLE_AUTO_LOCK_TIME       = 0x93,   //�Զ�����ʱ��
	BLE_DIRECTION            = 0x92,   //���ҿ� 
	BLE_MOTOR_TORQUE         = 0X91,   //���Ť��
	BLE_SPEED                = 0X72,   //�����ǣ����ٶ��Զ�����
	
	BLE_CONFIGURATION_LOCK   = 0X94,   //���������ϱ�
	BLE_VIOCE                = 0X71,   //��������
	BLE_IR_SET               = 0X88,   //��������
	BLE_LOCK_MODE            = 0X89,   //����ģʽ
	BLE_FACE_CHECK_EN        = 0X8A,   //������֤����
	BLE_KEY_DEF              = 0X95,   //һ������
	BLE_IR_DEF               = 0X96,   //��������
	BLE_LOCK_CHECK           = 0XA8 ,  //����ȷ�ϼ�
	BLE_RGB_MODE             = 0X8B ,  //��Χ��ģʽ����

	BLE_FACEADD              = 0X85,    //��ʼ��������
	BLE_FACESUCSSFUL         = 0X86,	//ȷ������¼��ɹ�
	BLE_FACE_DEL             = 0X87,    //ɾ������
	BLE_FACE_WRTITE	         = 0x9D,    //�޸���������
	BLE_FACE_VERSION         = 0X9A,    //�����汾��ѯ
	BLE_FACE_OTA             = 0X9C,    //����OTA
	BLE_FACE_HIJACK          = 0XA9,    //�������ٳ�

	BLE_FACE_THRESHOLD_LEVEL = 0xED,    //ʶ��ȫ�ȼ�����
	BLE_FACE_SET_LINKKEY     = 0xE5,    //�·���������Կ
	BLE_FACE_PRO_OTA         = 0XE7,    //è������
	
	BLE_CAM_GET_IP           = 0XE1,    //��ȡè��IP
	BLE_CAM_BELL_ONE_WAY     = 0XA4,    //è�۵�˫���л�
	BLE_CAM_GET_SN			 = 0XA3,    //��ȡè�����к�
	BLE_DOOR_UNLOCK_WARM	 = 0XAA,    //��δ�ر�������
	
	BLE_LOCK_CASE_LOG        = 0X9B,    //�¼���¼	
	BLE_CLEAR_LCOK_LOG       = 0xD3,    //����¼���¼



	
    //��װ
	BLE_WIFI_TEST            = 0xD2,         //WIFI����ָ������
	BLE_MODEL_WRITE_TEST     = 0xD5,         //BLE��������ͺ� (8��������,T86=54 38 36 00 00 00 00 00 (���油0)(ascii))
	BLE_MODEL_READ_TEST      = 0xD6,         //BLE  �ͺ���֤ 
	BLE_EEPROM_WRITE_TEST    = 0xDB,         //BLE����дEEPROM,2�������ݣ���λ�����ͣ� 
    BLE_EEPROM_READ_TEST     = 0xDC,         //BLE���Զ�EEPROM,��д��ĶԱ�
    BLE_LED_RED_GREEN_TEST   = 0xDD,         //BLE����ָʾ��ȫ��
    BLE_READ_ADC_TEST        = 0xDE,         //BLE���Զ�ȡ��ѹֵ 
	BLE_READ_TIME_TEST       = 0xDF,         //BLE���Զ�ȡʱ��   
	BLE_READ_ADC2_TEST       = 0xE2,         //�ڶ�·ADC
	BLE_GET_LOCK_WIFI_MAC    = 0xE3,         //������ָʾ��
	BLE_WIFI_FACTORY_TEST = 0xEB,        //WIFI����ɨ��ָ��·����
	BLE_CMD_SET_FOV_PARAM = 0xFA,         // ����FOV �ӳ���
	

}BLEPROTYPE;


/*--------------------------------------------------��������---------------------------------------------------------*/             
typedef union
{
	uint8_t data;
	struct
	{
		uint8_t PwdCheckEnable          :1;   //�������
		uint8_t FingerCheckEnable       :1;   //ָ�ƽ���
		uint8_t SmartgKeyCheckEn        :1;   //����Կ�׽���������Կ��/�����ֻ���
		uint8_t FaceCheckEnable         :1;   //��������
		uint8_t IcCardCheckEnable       :1;   //IC������
		uint8_t FingerVeinCheckEn       :1;   //ָ��������
//		uint8_t PhysicalKeyCheckEn      :1;   //���ӻ�еԿ�׽��� (�ѷϳ�) 20210909
		uint8_t IrisOpenCheckEn         :1;   //��Ĥ������ʽ
		uint8_t HuaWeiWalletEn          :1;   //��ΪǮ������
	}bit;
}DoubleCheckType_U;  //��Ͽ��ŷ�ʽ

typedef union
{
	uint16_t data;
	struct
	{
		uint16_t PwdMode           :1;    //���������ʽ     0: ��֧��  1:֧��
		uint16_t FingerMode        :1;    //ָ�ƽ�����ʽ     0: ��֧��  1:֧��
		uint16_t SmartKeyMode      :1;    //����Կ�׽���������Կ��/�����ֻ��� 0: ��֧��  1:֧��
		uint16_t FaceMode          :1;    //����������ʽ	 0: ��֧��  1:֧��
		uint16_t IcCardMode        :1;    //IC��������ʽ	 0: ��֧��  1:֧��
		uint16_t FingerVeinMode    :1;    //ָ����������ʽ	 0: ��֧��  1:֧��
	  //uint16_t PhysicalKeyMode   :1;    //���ӻ�еԿ�׽�����ʽ 0: ��֧��  1:֧��  �ϳ�
		uint16_t IrisOpenMode      :1;    //��Ĥ������ʽ     0: ��֧��  1:֧��
		uint16_t HuaWeiWalletMode  :1;    //��ΪǮ��������ʽ 0: ��֧��  1:֧��

		uint16_t Unknow  		   :8;    //Ԥ��
	}bit;
}OpenKind_U;  //֧�ֿ�����ʽ

typedef union
{
	uint16_t data;
	struct
	{
		//�ù��ܿ���״̬
		uint16_t GyroscopeModuleSts      :1;    //������ģ��             0: �ر�   1:����
		uint16_t MotoDirectAdjustSts     :1;    //���ŷ�����������ң�   0: �ҿ�   1:��  
		uint16_t MotoTorsionAdjustSts    :1;    //���Ť������ 		     0: ��Ť�� 1:��Ť��    
		uint16_t AutoLockTimSetPara      :5;    //�Զ�����ʱ�����(0-32)*5��      
 
		//���޸ù���
		uint16_t GyroscopeModuleEn   	 :1;    //������ģ��             0: ��  1:��
		uint16_t MotoDirectAdjustEn  	 :1;    //���ŷ�����������ң�   0: ��  1:��
		uint16_t MotoTorsionAdjustEn 	 :1;    //���Ť������ 		     0: ��  1:��
		uint16_t AutoLockTimAdjustEn 	 :1;    //�Զ�����ʱ�����       0: ��  1:��
		uint16_t TouchLockConfirmEn  	 :1;    //ȫ�Զ�����ȷ�Ϲ��ܣ�������������ȷ�ϣ� 0: ��  1:��
		uint16_t Unknow1		   		 :1;    //Ԥ��
		uint16_t Unknow2  		  		 :1;    //Ԥ��
		uint16_t Unknow3 		   		 :1;    //Ԥ��
	}bit;
}AutoLockCfg_U;  //ȫ�Զ�������  

typedef union
{
	uint8_t tab[6];
	struct
	{
		//���޸ù���   ��������1 
		uint8_t OneKeyDeployEn  	    :1;    //һ����������           0: ��  1:��
		uint8_t VolGradeAdjustEn  		:1;    //��������  			    0: ��  1:��
		uint8_t ActiveDefenseEn 		:1;    //��������(����30����) 0: ��  1:��
		uint8_t NfcFuncEn      			:1;    //NFC����                0: ��  1:��
		uint8_t OpenKindMoreEn    		:1;    //���ŷ�ʽ��ѡ����  	    0: ��  1:��
		uint8_t BellTakePictureEn	    :1;    //����ץ�Ĺ���			0: ��  1:��
		uint8_t NearSenseCheckEn  	    :1;    //������빦��           0: ��  1:��
		uint8_t EventRecordEn 		    :1;    //�¼���¼����		    0: ��  1:��
		
		//�ù��ܿ���״̬
		uint8_t OneKeyDeploySts         :1;    //һ����������״̬       0: �ر�   1:����
		uint8_t UnknowSts01       		:1;    //Ԥ��
		uint8_t UnknowSts02        		:1;    //Ԥ��
		uint8_t NfcFuncSts              :1;    //NFC���ܿ���״̬        0: �ر�   1:����  
		uint8_t UnknowSts03     	    :1;    //Ԥ��
		uint8_t BellTakePictureSts      :1;    //����ץ�Ĺ��ܿ���״̬   0: �ر�   1:����  
		uint8_t UnknowSts04       		:1;    //Ԥ��
		uint8_t EventRecordSts          :1;    //�¼���¼���ܿ���״̬   0: �ر�   1:����  
		
		//���޸ù���  ��������2 
		uint8_t OemBraceletEn  	        :1;    //С��OEM���ֻ�          0: ��  1:��
		uint8_t CameraTypeEn     	    :1;    //è��(�������±ȡ���ŵ��)���A3ָ��   0: ��  1:��
		uint8_t STCameraEn 				:1;    //����è��               0: ��  1:��
		uint8_t ABCameraEn      	    :1;    //�±�è��               0: ��  1:��
		uint8_t IcCardEn    		    :1;    //IC������    	  	    0: ��  1:��
		uint8_t R8NearSenseAdjustEn	    :1;    //R8�������ɵ�(Զ/��/�ر�) 0: ��  1:��
		uint8_t NewRemoteControlEn      :1;    //��ң����               0: ��  1:��
		uint8_t FaceForceEn 		    :1;    //����Ю�ֿ���		    0: ��  1:��
		
		//�ù��ܿ���״̬
		uint8_t UnknowSts1              :1;    //Ԥ��
		uint8_t UnknowSts2      		:1;    //Ԥ��
		uint8_t STCameraSts		        :1;    //����è�۵�˫��   		0: ����   1:˫��   
		uint8_t UnlockWarmEn            :1;    //��δ�ر�������         0: ��  1:��
		uint8_t UnknowSts4       		:1;    //�������������� 		0: ��  1:��
		uint8_t FingerGetID      		:1;    //ָ�ƻ���ָIDͬ������   0: ��  1:��
		uint8_t FaceCheckEn      		:1;    //������֤���� 			0: ��  1:��	
		uint8_t GetWifiSignalIntensity  :1;    //��ȡWIFI�ź�ǿ��
		
		//���޸ù���  ��������3 
		uint8_t ScreenIndoor 	        :1;    //��������               0: ��  1:��
		uint8_t RTLMcu		     	    :1;    //RTLϵ�е�Ƭ��
		uint8_t UnknowSts06 			:1;    //Ԥ��
		uint8_t UnknowSts07      	    :1;    //Ԥ��
		uint8_t UnknowSts08    		    :1;    //Ԥ��
		uint8_t UnknowSts09	    		:1;    //Ԥ��
		uint8_t UnknowSts10      		:1;    //Ԥ��
		uint8_t UnknowSts11 		    :1;    //Ԥ��
		
		//�ù��ܿ���״̬
		uint8_t UnknowSts12             :1;    //Ԥ��
		uint8_t UnknowSts13      		:1;    //Ԥ��
		uint8_t UnknowSts14		        :1;    //Ԥ��
		uint8_t UnknowSts15             :1;    //Ԥ��
		uint8_t UnknowSts16       		:1;    //Ԥ��
		uint8_t UnknowSts17      		:1;    //Ԥ��
		uint8_t UnknowSts18      		:1;    //Ԥ��	
		uint8_t UnknowSts19  			:1;    //Ԥ��

	}bit;
}FunctionCfg_U;  //��������   

typedef union
{
	uint8_t tab[8];
	struct
	{
		uint8_t NfcCheckEn    	   		:1;    //NFC���           0: ��  1:��
		uint8_t VersionCheckEn  		:1;    //�汾��ѯ  		   0: ��  1:��
		uint8_t ReadMacAddrEn  			:1;    //��MAC��ַ		   0: ��  1:��
		uint8_t CheckBatVol1En      	:1;    //��ѹ1             0: ��  1:��
		uint8_t CheckBatVol2En          :1;    //��ѹ2             0: ��  1:��
		uint8_t PirTestEn	   			:1;    //PIR����(��)	   0: ��  1:��
		uint8_t WriteFlashEn      	    :1;    //дflash           0: ��  1:��
		uint8_t ReadFlashEn 		    :1;    //��flash		   0: ��  1:��
		
		uint8_t ScreenTestEn    	   	:1;    //��Ļ����          0: ��  1:��
		uint8_t FingerInputEn   		:1;    //¼��ָ��  		   0: ��  1:��
		uint8_t FaceCommiteEn  			:1;    //����ͨ��		   0: ��  1:��
		uint8_t WifiSelfcheckEn      	:1;    //WIFI�Լ�          0: ��  1:��
		uint8_t R8SecretKeyLoadEn     	:1;    //R8��Կ�·�        0: ��  1:��
		uint8_t InnerBatNormalEn        :1;    //����﮵��(3.0V-4.3V)��Ϊ����   0: ��  1:��
		uint8_t ClearAllUserEn	   		:1;    //����û� 		   0: ��  1:��
		uint8_t Q5M_V9RealTimeVideoEn   :1;    //Q5M/V9ʵʱ��Ƶ    0: ��  1:��
 
		uint8_t PinCodeLoadEn    	   	:1;    //��ΪPIN�·�       0: ��  1:��
		uint8_t BleDisconnectEn   		:1;    //�����Ͽ�  		   0: ��  1:��
		uint8_t R8RealTimeVideoEn  	    :1;    //R8ʵʱ��Ƶ		   0: ��  1:��
		uint8_t WifiSignalIntensityEn   :1;    //WIFI����ɨ��·�����ź�ǿ��      0: ��  1:��
		uint8_t IcCardCheckEn	     	:1;    //IC�����          0: ��  1:��
		uint8_t LockModelLoadEn         :1;    //ȫ�Զ������ͺ��·�0: ��  1:��
		uint8_t Unknow    		   		:1;    //Ԥ��
		uint8_t SnCodeLoadEn        	:1;    //��ΪSN�·�        0: ��  1:��
		
		uint8_t EventRecordClearEn 	   	:1;    //С�ι�װ������¼���¼  0: ��  1:��
		uint8_t EncodeChipCheckEn  		:1;    //����оƬ���            0: ��  1:��
		uint8_t ButtonOpenModeCfgEn  	:1;    //���ż�����/˫���������� 0: ��  1:��
		uint8_t Unknow1 			    :5;    //Ԥ��
 
		uint32_t Unknow2; 			           //Ԥ��
 
	}bit;
}FactoryTestCfg_U;  //���⹤װ����   

typedef union
{
	uint8_t data;
	struct
	{
        uint8_t FaceAndFinger       :1;    //���� + ָ��            0: ��  1:��
        uint8_t FaceAndPwd          :1;    //���� + ����            0: ��  1:��
        uint8_t FingerAndPwd        :1;    //ָ�� + ����            0: ��  1:��     
        uint8_t IrisAndFinger       :1;    //��Ĥ + ָ��            0: ��  1:�� 
        uint8_t IrisAndPwd          :1;    //��Ĥ + ����            0: ��  1:��
        uint8_t VeinAndPwd          :1;    //ָ���� + ����          0: ��  1:��
        uint8_t Unknow1             :2;    //Ԥ��
 
	}bit;
}CombineOpenMode_U;  //��Ͽ��ŷ�ʽ  


extern APP_BLE_TYPE AppBleType;
void APP_BleInit(DRV_BLE_ADV_MODE mode,DRV_BLE_ADV_FLAGS flags);
uint8_t AppBleRespondData (uint8_t cmd);
void APP_BleSetAdminState(ADMIN_STATE mode);
ADMIN_STATE APP_BleGetAdminState(void);
void SystemAppBleInit(void);
void BleFaceCasePro(void);
void BleFingerCasePro(void);
void APP_BleServerProcess(void );
void App_BleProHandleThread( void );	
void APP_BleDelPhone(void );	
#ifdef __cplusplus
	}
#endif
#endif
//.end of the file.
