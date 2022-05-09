#ifndef _APP_BLE_H_
#define _APP_BLE_H_
	
#ifdef __cplusplus
	extern "C" {
#endif

/* 标准头文件 */
#include "stdint.h"

#include "DRV_BLE.h" 
/*-------------------------------------------------宏定义-----------------------------------------------------------*/ 
//协议格式
#define L_HEAD				0   //头
#define L_TYPE				1   //类型手机01 锁09
#define L_CMD				2   //命令
#define L_LENH				3	//数据长度高字节
#define L_LENL				4   //数据长度低字节
#define L_DATA				5   //数据包

typedef enum
{
	E_DEFAULT,   				//普通模式，需要走加密
	E_KEY_ADMIN, 				//按键进管理员模式，可以明文
	E_BLE_ADMIN,  			//蓝牙唤醒管理员模式,需要走加密
	E_VIDEO_ADMIN,			//蓝牙钥匙连接指令，需要走加密（分离屏用）
	E_PHONE_CHECK_ERR,
	E_PHONE_CHECK_OK,
	E_SMARTKEKY_CHECK_ERR,
	E_SMARTKEKY_CHECK_OK,	
}ADMIN_STATE;

typedef struct
{
	struct
	{
		uint8_t  TxCmd;  //发送帧数据
		uint16_t TxLen;  //发送中间数据包长度，不包含头尾
		uint8_t  TxDataBuf[256];  //数据发送BUF
	}Respond; //蓝牙回包
	
	ADMIN_STATE Admin; //当前蓝牙连接权限
	uint8_t RxDataBuf[256];  //数据接收BUF

	uint8_t RxPos;       //当前接受位置
	uint8_t RxCdm;   //当前手机接收指令

	uint8_t ChannelPwd[4]; //信道密码
	uint8_t RandomNum[8]; //随机数会话密钥
	
}APP_BLE_TYPE;


typedef enum{

	BLE_TIMEUPDATA		= 0x23,    //时间同步
	BLE_ADDFINGER		= 0x24,    //增加指纹
	
	BLE_PHONE_GETKEY	= 0x3A,    //获取密钥
	BLE_AES_ADDPHONE	= 0x7C,    //增加手机号AES加密版本
	BLE_DELPHONE		= 0x29,    //删除手机号
	BLE_PHONE_ENLOCK	= 0x39,    //手机蓝牙开门
	
	
	BLE_ADDFINGER_NEW   = 0x53,    //开始增加指纹
	BLE_ADDFINGER_ACK   = 0x54,    //确认指纹录入成功
	BLE_GET_FINGERID    = 0x55,    //获取已注册的所有指纹或手指ID
	BLE_DELFINGER		= 0x28,    //删除指纹
	BLE_FINGER_WRTITE	= 0x2B,    //修改指纹属性
		
	BLE_SMARTKEY_GETKEY1 = 0x3C,   //获取密钥
	BLE_SMARTKEY_GETKEY2 = 0x3E,   //获取随机密钥 跟3A,3C一样.
	BLE_ADDSMARTKEY_REQ  = 0x26,   //增加智能钥匙密钥到锁
	BLE_DELSMARTKEY		 = 0x2A,   //删除智能钥匙
	BLE_SMARTKEY_ENLOCK  = 0x3B,   //智能钥匙开锁
	
	BLE_AES_ADD_PASSWORD =0X79,    //开锁密码设置
	BLE_AES_ADD_SOS_PASSWORD=0X7A, //SOS开锁密码设置
    BLE_AES_ADD_TMP_PASSWORD=0X7B, //临时密码设置
	
	
	BLE_REG_CHECK_CODE  = 0X42,      //注册确认码
	BLE_DEVICE_REG		= 0x34,      //设备注册
	BLE_CLEAR_USERMEM	= 0x35,      //清空用户
	BLE_ADMIN_PWD_CHECK = 0x36,      //锁 管理密码验证
	BLE_GET_HWIFO		= 0x37,      //锁具软件版本号获取
	BLE_CMD_DISCONNECT	= 0xF0,      // 断开蓝牙连接
	BLE_CMD_OTAMODE		= 0xF4,      //更新程序 模式，无应答	
	
	BLE_WIFI_CONNECTION_TEST = 0x40,   //测试 服务器连接状态
	BLE_WIFI_MAIN_SW         = 0x41,   //WiFi功能开启关闭
	BLE_SET_LOGUPWITHWIFI    = 0xF9,   //开锁记录上传设置
	BLE_CMD_SET_WIFI	     = 0xF5,   // wifi参数设置  手机->锁
	BLE_CMD_SET_WIFI_IP   	 = 0xF7,   // WIFI上增加域名设置。
	BLE_GET_WIFI_SIGNAL_INTENSITY = 0x8D, // 获取WIFI信号强度
	

	BLE_SE_VERIFY_TOKEN      = 0x65,   //ID2验证TOKEN
	BLE_SE_CHALLENGE         = 0x66,   //ID2重新激活
	BLE_SE_SETMODE           = 0x67,   //ID2开关，不需要
	BLE_SE_GET_ID2               =0x6A,
	BLE_SE_ID2_TOKEN_NEW         =0x68,
	BLE_SE_ID2_BOTHWAY_VERIFY    =0x69,
	
	BLE_AUTO_LOCK_TIME       = 0x93,   //自动上锁时间
	BLE_DIRECTION            = 0x92,   //左右开 
	BLE_MOTOR_TORQUE         = 0X91,   //电机扭力
	BLE_SPEED                = 0X72,   //陀螺仪，加速度自动上锁
	
	BLE_CONFIGURATION_LOCK   = 0X94,   //锁功能项上报
	BLE_VIOCE                = 0X71,   //音量设置
	BLE_IR_SET               = 0X88,   //红外设置
	BLE_LOCK_MODE            = 0X89,   //开门模式
	BLE_FACE_CHECK_EN        = 0X8A,   //人脸验证开关
	BLE_KEY_DEF              = 0X95,   //一键布防
	BLE_IR_DEF               = 0X96,   //主动防御
	BLE_LOCK_CHECK           = 0XA8 ,  //锁门确认键
	BLE_RGB_MODE             = 0X8B ,  //氛围灯模式配置

	BLE_FACEADD              = 0X85,    //开始增加人脸
	BLE_FACESUCSSFUL         = 0X86,	//确认人脸录入成功
	BLE_FACE_DEL             = 0X87,    //删除人脸
	BLE_FACE_WRTITE	         = 0x9D,    //修改人脸属性
	BLE_FACE_VERSION         = 0X9A,    //人脸版本查询
	BLE_FACE_OTA             = 0X9C,    //人脸OTA
	BLE_FACE_HIJACK          = 0XA9,    //人脸防劫持

	BLE_FACE_THRESHOLD_LEVEL = 0xED,    //识别安全等级调节
	BLE_FACE_SET_LINKKEY     = 0xE5,    //下发阿里云密钥
	BLE_FACE_PRO_OTA         = 0XE7,    //猫眼升级
	
	BLE_CAM_GET_IP           = 0XE1,    //获取猫眼IP
	BLE_CAM_BELL_ONE_WAY     = 0XA4,    //猫眼单双休切换
	BLE_CAM_GET_SN			 = 0XA3,    //获取猫眼序列号
	BLE_DOOR_UNLOCK_WARM	 = 0XAA,    //门未关报警设置
	
	BLE_LOCK_CASE_LOG        = 0X9B,    //事件记录	
	BLE_CLEAR_LCOK_LOG       = 0xD3,    //清除事件记录



	
    //工装
	BLE_WIFI_TEST            = 0xD2,         //WIFI推送指定号码
	BLE_MODEL_WRITE_TEST     = 0xD5,         //BLE添加锁具型号 (8节字数据,T86=54 38 36 00 00 00 00 00 (后面补0)(ascii))
	BLE_MODEL_READ_TEST      = 0xD6,         //BLE  型号验证 
	BLE_EEPROM_WRITE_TEST    = 0xDB,         //BLE测试写EEPROM,2节字数据（上位机发送） 
    BLE_EEPROM_READ_TEST     = 0xDC,         //BLE测试读EEPROM,与写入的对比
    BLE_LED_RED_GREEN_TEST   = 0xDD,         //BLE测试指示灯全亮
    BLE_READ_ADC_TEST        = 0xDE,         //BLE测试读取电压值 
	BLE_READ_TIME_TEST       = 0xDF,         //BLE测试读取时间   
	BLE_READ_ADC2_TEST       = 0xE2,         //第二路ADC
	BLE_GET_LOCK_WIFI_MAC    = 0xE3,         //产测亮指示灯
	BLE_WIFI_FACTORY_TEST = 0xEB,        //WIFI产测扫描指定路由器
	BLE_CMD_SET_FOV_PARAM = 0xFA,         // 设置FOV 视场角
	

}BLEPROTYPE;


/*--------------------------------------------------变量声明---------------------------------------------------------*/             
typedef union
{
	uint8_t data;
	struct
	{
		uint8_t PwdCheckEnable          :1;   //密码解锁
		uint8_t FingerCheckEnable       :1;   //指纹解锁
		uint8_t SmartgKeyCheckEn        :1;   //智能钥匙解锁（蓝牙钥匙/蓝牙手环）
		uint8_t FaceCheckEnable         :1;   //人脸解锁
		uint8_t IcCardCheckEnable       :1;   //IC卡解锁
		uint8_t FingerVeinCheckEn       :1;   //指静脉解锁
//		uint8_t PhysicalKeyCheckEn      :1;   //电子机械钥匙解锁 (已废除) 20210909
		uint8_t IrisOpenCheckEn         :1;   //虹膜解锁方式
		uint8_t HuaWeiWalletEn          :1;   //华为钱包解锁
	}bit;
}DoubleCheckType_U;  //组合开门方式

typedef union
{
	uint16_t data;
	struct
	{
		uint16_t PwdMode           :1;    //密码解锁方式     0: 不支持  1:支持
		uint16_t FingerMode        :1;    //指纹解锁方式     0: 不支持  1:支持
		uint16_t SmartKeyMode      :1;    //智能钥匙解锁（蓝牙钥匙/蓝牙手环） 0: 不支持  1:支持
		uint16_t FaceMode          :1;    //人脸解锁方式	 0: 不支持  1:支持
		uint16_t IcCardMode        :1;    //IC卡解锁方式	 0: 不支持  1:支持
		uint16_t FingerVeinMode    :1;    //指静脉解锁方式	 0: 不支持  1:支持
	  //uint16_t PhysicalKeyMode   :1;    //电子机械钥匙解锁方式 0: 不支持  1:支持  废除
		uint16_t IrisOpenMode      :1;    //虹膜解锁方式     0: 不支持  1:支持
		uint16_t HuaWeiWalletMode  :1;    //华为钱包解锁方式 0: 不支持  1:支持

		uint16_t Unknow  		   :8;    //预留
	}bit;
}OpenKind_U;  //支持开锁方式

typedef union
{
	uint16_t data;
	struct
	{
		//该功能开启状态
		uint16_t GyroscopeModuleSts      :1;    //陀螺仪模块             0: 关闭   1:开启
		uint16_t MotoDirectAdjustSts     :1;    //开门方向调整（左右）   0: 右开   1:左开  
		uint16_t MotoTorsionAdjustSts    :1;    //电机扭力调节 		     0: 低扭力 1:高扭力    
		uint16_t AutoLockTimSetPara      :5;    //自动上锁时间调节(0-32)*5秒      
 
		//有无该功能
		uint16_t GyroscopeModuleEn   	 :1;    //陀螺仪模块             0: 无  1:有
		uint16_t MotoDirectAdjustEn  	 :1;    //开门方向调整（左右）   0: 无  1:有
		uint16_t MotoTorsionAdjustEn 	 :1;    //电机扭力调节 		     0: 无  1:有
		uint16_t AutoLockTimAdjustEn 	 :1;    //自动上锁时间调节       0: 无  1:有
		uint16_t TouchLockConfirmEn  	 :1;    //全自动锁门确认功能（触摸按键上锁确认） 0: 无  1:有
		uint16_t Unknow1		   		 :1;    //预留
		uint16_t Unknow2  		  		 :1;    //预留
		uint16_t Unknow3 		   		 :1;    //预留
	}bit;
}AutoLockCfg_U;  //全自动锁配置  

typedef union
{
	uint8_t tab[6];
	struct
	{
		//有无该功能   功能配置1 
		uint8_t OneKeyDeployEn  	    :1;    //一键布防功能           0: 无  1:有
		uint8_t VolGradeAdjustEn  		:1;    //音量调节  			    0: 无  1:有
		uint8_t ActiveDefenseEn 		:1;    //主动防御(红外30秒检测) 0: 无  1:有
		uint8_t NfcFuncEn      			:1;    //NFC功能                0: 无  1:有
		uint8_t OpenKindMoreEn    		:1;    //开门方式可选功能  	    0: 无  1:有
		uint8_t BellTakePictureEn	    :1;    //门铃抓拍功能			0: 无  1:有
		uint8_t NearSenseCheckEn  	    :1;    //红外距离功能           0: 无  1:有
		uint8_t EventRecordEn 		    :1;    //事件记录功能		    0: 无  1:有
		
		//该功能开启状态
		uint8_t OneKeyDeploySts         :1;    //一键布防开关状态       0: 关闭   1:开启
		uint8_t UnknowSts01       		:1;    //预留
		uint8_t UnknowSts02        		:1;    //预留
		uint8_t NfcFuncSts              :1;    //NFC功能开启状态        0: 关闭   1:开启  
		uint8_t UnknowSts03     	    :1;    //预留
		uint8_t BellTakePictureSts      :1;    //门铃抓拍功能开启状态   0: 关闭   1:开启  
		uint8_t UnknowSts04       		:1;    //预留
		uint8_t EventRecordSts          :1;    //事件记录功能开启状态   0: 关闭   1:开启  
		
		//有无该功能  功能配置2 
		uint8_t OemBraceletEn  	        :1;    //小滴OEM新手环          0: 无  1:有
		uint8_t CameraTypeEn     	    :1;    //猫眼(雄迈、奥比、智诺等)结合A3指令   0: 无  1:有
		uint8_t STCameraEn 				:1;    //商汤猫眼               0: 无  1:有
		uint8_t ABCameraEn      	    :1;    //奥比猫眼               0: 无  1:有
		uint8_t IcCardEn    		    :1;    //IC卡功能    	  	    0: 无  1:有
		uint8_t R8NearSenseAdjustEn	    :1;    //R8红外距离可调(远/近/关闭) 0: 无  1:有
		uint8_t NewRemoteControlEn      :1;    //新遥控器               0: 无  1:有
		uint8_t FaceForceEn 		    :1;    //人脸挟持开关		    0: 无  1:有
		
		//该功能开启状态
		uint8_t UnknowSts1              :1;    //预留
		uint8_t UnknowSts2      		:1;    //预留
		uint8_t STCameraSts		        :1;    //商汤猫眼单双向   		0: 单向   1:双向   
		uint8_t UnlockWarmEn            :1;    //门未关报警功能         0: 无  1:有
		uint8_t UnknowSts4       		:1;    //语音包在线升级 		0: 无  1:有
		uint8_t FingerGetID      		:1;    //指纹或手指ID同步功能   0: 无  1:有
		uint8_t FaceCheckEn      		:1;    //人脸验证开关 			0: 无  1:有	
		uint8_t GetWifiSignalIntensity  :1;    //获取WIFI信号强度
		
		//有无该功能  功能配置3 
		uint8_t ScreenIndoor 	        :1;    //后屏功能               0: 无  1:有
		uint8_t RTLMcu		     	    :1;    //RTL系列单片机
		uint8_t UnknowSts06 			:1;    //预留
		uint8_t UnknowSts07      	    :1;    //预留
		uint8_t UnknowSts08    		    :1;    //预留
		uint8_t UnknowSts09	    		:1;    //预留
		uint8_t UnknowSts10      		:1;    //预留
		uint8_t UnknowSts11 		    :1;    //预留
		
		//该功能开启状态
		uint8_t UnknowSts12             :1;    //预留
		uint8_t UnknowSts13      		:1;    //预留
		uint8_t UnknowSts14		        :1;    //预留
		uint8_t UnknowSts15             :1;    //预留
		uint8_t UnknowSts16       		:1;    //预留
		uint8_t UnknowSts17      		:1;    //预留
		uint8_t UnknowSts18      		:1;    //预留	
		uint8_t UnknowSts19  			:1;    //预留

	}bit;
}FunctionCfg_U;  //功能配置   

typedef union
{
	uint8_t tab[8];
	struct
	{
		uint8_t NfcCheckEn    	   		:1;    //NFC检测           0: 无  1:有
		uint8_t VersionCheckEn  		:1;    //版本查询  		   0: 无  1:有
		uint8_t ReadMacAddrEn  			:1;    //读MAC地址		   0: 无  1:有
		uint8_t CheckBatVol1En      	:1;    //电压1             0: 无  1:有
		uint8_t CheckBatVol2En          :1;    //电压2             0: 无  1:有
		uint8_t PirTestEn	   			:1;    //PIR测试(无)	   0: 无  1:有
		uint8_t WriteFlashEn      	    :1;    //写flash           0: 无  1:有
		uint8_t ReadFlashEn 		    :1;    //读flash		   0: 无  1:有
		
		uint8_t ScreenTestEn    	   	:1;    //屏幕测试          0: 无  1:有
		uint8_t FingerInputEn   		:1;    //录入指纹  		   0: 无  1:有
		uint8_t FaceCommiteEn  			:1;    //人脸通信		   0: 无  1:有
		uint8_t WifiSelfcheckEn      	:1;    //WIFI自检          0: 无  1:有
		uint8_t R8SecretKeyLoadEn     	:1;    //R8密钥下发        0: 无  1:有
		uint8_t InnerBatNormalEn        :1;    //内置锂电池(3.0V-4.3V)都为正常   0: 无  1:有
		uint8_t ClearAllUserEn	   		:1;    //清空用户 		   0: 无  1:有
		uint8_t Q5M_V9RealTimeVideoEn   :1;    //Q5M/V9实时视频    0: 无  1:有
 
		uint8_t PinCodeLoadEn    	   	:1;    //华为PIN下发       0: 无  1:有
		uint8_t BleDisconnectEn   		:1;    //蓝牙断开  		   0: 无  1:有
		uint8_t R8RealTimeVideoEn  	    :1;    //R8实时视频		   0: 无  1:有
		uint8_t WifiSignalIntensityEn   :1;    //WIFI产测扫描路由器信号强度      0: 无  1:有
		uint8_t IcCardCheckEn	     	:1;    //IC卡检测          0: 无  1:有
		uint8_t LockModelLoadEn         :1;    //全自动锁体型号下发0: 无  1:有
		uint8_t Unknow    		   		:1;    //预留
		uint8_t SnCodeLoadEn        	:1;    //华为SN下发        0: 无  1:有
		
		uint8_t EventRecordClearEn 	   	:1;    //小滴工装软清空事件记录  0: 无  1:有
		uint8_t EncodeChipCheckEn  		:1;    //加密芯片检测            0: 无  1:有
		uint8_t ButtonOpenModeCfgEn  	:1;    //开门键单击/双击开门设置 0: 无  1:有
		uint8_t Unknow1 			    :5;    //预留
 
		uint32_t Unknow2; 			           //预留
 
	}bit;
}FactoryTestCfg_U;  //厂测工装配置   

typedef union
{
	uint8_t data;
	struct
	{
        uint8_t FaceAndFinger       :1;    //人脸 + 指纹            0: 无  1:有
        uint8_t FaceAndPwd          :1;    //人脸 + 密码            0: 无  1:有
        uint8_t FingerAndPwd        :1;    //指纹 + 密码            0: 无  1:有     
        uint8_t IrisAndFinger       :1;    //虹膜 + 指纹            0: 无  1:有 
        uint8_t IrisAndPwd          :1;    //虹膜 + 密码            0: 无  1:有
        uint8_t VeinAndPwd          :1;    //指静脉 + 密码          0: 无  1:有
        uint8_t Unknow1             :2;    //预留
 
	}bit;
}CombineOpenMode_U;  //组合开门方式  


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
