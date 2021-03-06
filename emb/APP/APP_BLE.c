/********************************************************************************************************************
 * @file:        APP_BLE.c
 * @author:      dengyehao
 * @version:     V0.1
 * @date:        2021-08-13
 * @Description: 手机蓝牙交互
 * @ChangeList:  初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
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
/*-------------------------------------------------宏定义-----------------------------------------------------------*/ 

 //协议命令
#define BLE_APP_HEAD            0xFE   //头
#define BLE_APP_TYPE_CMD		0x01   //请求
#define BLE_APP_TYPE_RSP		0x09   //属性应答
//账号加密根密
#define AES_ROOT_KEY  5      //Root_key下标
/*-------------------------------------------------枚举定义---------------------------------------------------------*/
typedef enum
{
	E_CMD_DEFAULT,   
    E_CMD_UNREGIST_DEVICE,    //注销设备
	E_CMD_ROUTER_CFG,         //路由器配置
    E_CMD_SERVER_CFG,         //服务器配置
	E_CMD_NEAR_SENSE_CFG,     //接近感应配置
    E_CMD_SERVER_TEST,    	  //服务器测试
	E_CMD_DELETE_FINGER,      //删除指纹
	E_CMD_EXCHANGE_FINGER,    //修改指纹
	E_CMD_CAM_GET_SN,		  //获取猫眼序列号
	E_CMD_REG_PIN,             //注册配对
	E_CMD_CAM_LINKKEY,        //下发阿里云密钥
	E_CMD_CAM_POWER_SAVING,   //省电模式
	E_CMD_CAM_GET_IP,         //获取猫眼IP
	E_CMD_FACE_PRO_OTA,       //猫眼升级
	E_CMD_WIFI_TEST,
	E_CMD_WIFI_FACTORY_TEST,
	E_CMD_GET_WIFI_SIGNAL_INTENSITY, //获取WIFI信号强度
	E_CMD_SET_FOV_PARAM,     // 设置FOV 视场角
	E_CMD_WIFI_MAIN_SW_POW_OFF,
	E_CMD_BLE_LOCK_CASE      //事件记录获取
	
}BLE_RX_CMD_E;  //蓝牙接收需要动作的命令 动作流程需要分步骤完成

static BLE_RX_CMD_E  BleRxCmdHandlePro = E_CMD_DEFAULT;
/*-------------------------------------------------常量定义---------------------------------------------------------*/
//根密
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

/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         


/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
APP_BLE_TYPE  AppBleType;       //BLE结构体

static uint8_t  ComDataByte1  = 0;
static uint16_t ComDataByte2 = 0;

static uint8_t  BleHandleProStep  = 0;
/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/
/*********************************************************************************************************************
* Function Name :  APP_BleInit()
* Description   :  BLE 驱动Init接口封装
* Para          :  DRV_BLE_ADV_MODE mode  广播名类型
				   DRV_BLE_ADV_FLAGS flags  广播类型	
* Return        :  void
*********************************************************************************************************************/
void APP_BleInit(DRV_BLE_ADV_MODE mode ,DRV_BLE_ADV_FLAGS flags)
{
    DRV_BleInit(mode, flags, APP_UpdateDataHandler);
    return;
}

/***************************************************************************************
**函数名:       APP_BleDelPhone
**功能描述:     删除手机号
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
void APP_BleDelPhone(void )
{
	uint8_t temp[13];
	memset(temp,0xff,13);
	HAL_EEPROM_WriteBytes(MEM_PHONE_START,temp,13); 
}

/***************************************************************************************
**函数名:       BLE_Respond_Ready_DATA
**功能描述:     蓝牙发送数据组帧
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
uint8_t AppBleRespondData (uint8_t cmd)
{
    uint16_t crc=0;

    AppBleType.Respond.TxDataBuf[L_HEAD] = BLE_APP_HEAD; //头
    AppBleType.Respond.TxDataBuf[L_TYPE] = BLE_APP_TYPE_RSP; //应答
    AppBleType.Respond.TxDataBuf[L_CMD]  = cmd;       //命令
    AppBleType.Respond.TxDataBuf[L_LENH] = AppBleType.Respond.TxLen>>8;  //长度
	AppBleType.Respond.TxDataBuf[L_LENL] = AppBleType.Respond.TxLen&0xff;  //长度
	//ID2加密
	if(App_Id2_Data.ID2_EN)
	{
		if(BLE_LOCK_CASE_LOG!=cmd) //事件记录数据太多，使用明文
		{
			PUBLIC_PrintHex("ID2_EncDATA",AppBleType.Respond.TxDataBuf,AppBleType.Respond.TxLen+7);
			uint8_t	EncDATA[256]={0};		
			uint16_t AllLen=((AppBleType.Respond.TxLen+3)%16)? (16*(1+(AppBleType.Respond.TxLen+3)/16)):(AppBleType.Respond.TxLen+3);//计算加密总长度16倍数
			EncDATA[0]=(AppBleType.Respond.TxLen+1)>>8;  //有效长度等于原始长度+1命令
			EncDATA[1]=(AppBleType.Respond.TxLen+1)&0xff;  //有效长度等于原始长度+1命令
			EncDATA[2]=cmd; //拷贝命令		
			memcpy(&EncDATA[3],&AppBleType.Respond.TxDataBuf[L_DATA],AppBleType.Respond.TxLen); //拷贝明文有效数据			
			Encrypto_my_aes(ENCRYPTION, EncDATA, AllLen, App_Id2_Data.AESKEY); //加密数据	
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA],EncDATA,AllLen); //拷贝加密后的数据
			AppBleType.Respond.TxLen=AllLen ; //总长度改变
			AppBleType.Respond.TxDataBuf[L_LENH] = AppBleType.Respond.TxLen>>8;  //长度
			AppBleType.Respond.TxDataBuf[L_LENL] = AppBleType.Respond.TxLen&0xff;  //长度
		}
	}
	//校验
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
	memset(&AppBleType.Respond,0,sizeof(AppBleType.Respond));//发送完成后清除回包数据
	return result;
}

/***************************************************************************************
**函数名:       AppBleInit
**功能描述:     队列初始化,信道密码获取
**输入参数:     
**输出参数:     
**备注:         
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
	
	//信道密码，MAC高4位取反
	uint8_t mac[6];
	PUBLIC_GetMacAdd(mac);
	AppBleType.ChannelPwd[0]=~mac[5];
	AppBleType.ChannelPwd[1]=~mac[4];
	AppBleType.ChannelPwd[2]=~mac[3];
	AppBleType.ChannelPwd[3]=~mac[2];
}

void AppBleUserDataDecode(uint8_t mode , uint8_t *data, uint8_t len)
{
	uint8_t my_aes_key[16]={0}; // AES加密秘钥
	memcpy(&my_aes_key[0],AppBleType.RandomNum,8); //拷贝随机数
	for(uint8_t i=0; i<8; i++)
	{
		my_aes_key[8+i]=~my_aes_key[i]; //后8字节
	}
	for(uint8_t i=0; i<8; i++)
	{
		my_aes_key[i]^=Root_key[AES_ROOT_KEY][i]; //异或根密
		my_aes_key[i+8]^=Root_key[AES_ROOT_KEY][i];
	}  
	if(mode)//加密
	{
		Encrypto_my_aes(ENCRYPTION, data, len, my_aes_key);
	}
	else
	{
		//原始账号还原
		Encrypto_my_aes(DECRYPTION, data, len, my_aes_key);
	}
}


void BLEParse (void)
{
	//ID2加密
	//包头(1byte)+包属性(1byte)+指令(1byte)+长度(2byte)+数据包(n byte)+校验和(2byte)
	//数据包(n byte)=有效长度(2byte)+指令(1byte)+ 内容(n-3 byte)+不足16字节需补零
	
	 //按键进菜单,最高权限支持密文或明文
	#ifdef BLE_ID2_AES_ENC
	if( AppBleType.Admin!=E_KEY_ADMIN) //必须走加密
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
			case 0x6A:break;//允许明文	
			default://其余指令密文
				if(App_Id2_Data.ID2_EN==0)
				{
					AppBleType.Respond.TxDataBuf[L_DATA]=7; //错误
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(AppBleType.RxDataBuf[L_CMD]);
					return;
				}
		}
	}
	#endif
	if(App_Id2_Data.ID2_EN)//加密模式
	{
		uint16_t AllLen=  AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL] ; //总长度
		if((AllLen>=16) && (AllLen%16==0)) //16倍数
		{
			Encrypto_my_aes(DECRYPTION,&AppBleType.RxDataBuf[L_DATA], AllLen, App_Id2_Data.AESKEY); //解密数据
			PUBLIC_PrintHex("ID2_DECRYPTION_DATA =", &AppBleType.RxDataBuf[L_DATA],AppBleType.RxDataBuf[L_LENL]);
			if(AppBleType.RxDataBuf[L_DATA+2]==AppBleType.RxDataBuf[L_CMD]) //确认解密后的指令
			{
				//把解密后的类容替换为原始明文形式
				uint16_t ValidLen=AppBleType.RxDataBuf[L_DATA]<<8 |AppBleType.RxDataBuf[L_DATA+1] ; //有效长度
				memcpy(&AppBleType.RxDataBuf[L_DATA],&AppBleType.RxDataBuf[L_DATA+3],ValidLen); //拷贝有效长度的数据包
				AppBleType.RxDataBuf[L_LENH]=(ValidLen-1)>>8;   //数据长度替换为有效长度-1
				AppBleType.RxDataBuf[L_LENL]=(ValidLen-1)&0xff; //数据长度替换为有效长度-1
			}
			else
			{
				my_printf("DEC CMD ERR--------------- \n");
				AppBleType.Respond.TxDataBuf[L_DATA]=9; //错误
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(AppBleType.RxDataBuf[L_CMD]);
				return;
			}
		}
		else
		{
			my_printf("ENC LEN <16----------------- \n");
			AppBleType.Respond.TxDataBuf[L_DATA]=8; //错误
			AppBleType.Respond.TxLen =1;  
			AppBleRespondData(AppBleType.RxDataBuf[L_CMD]);
			return;
		}
	}
	AppBleType.RxCdm=AppBleType.RxDataBuf[L_CMD];//记录当前任务	
    uint8_t led_num[10]={EM_LED_0,EM_LED_1,EM_LED_2,EM_LED_3,EM_LED_4,EM_LED_5,EM_LED_6,EM_LED_7,EM_LED_8,EM_LED_9};
	switch (AppBleType.RxCdm) 
	{
		//直接处理类
		case BLE_CONFIGURATION_LOCK: //锁功能项上报
		{
            AppBleType.Respond.TxDataBuf[L_DATA]= 0x0;  //ACK成功
		
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],SystemFixSeting.LockNameType,8);//拷贝锁具型号
		
			PUBLIC_GetMacAdd(&AppBleType.Respond.TxDataBuf[L_DATA+1+8]); //获取锁具MAC
		
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1+8+6],AppBleType.ChannelPwd,4); //拷贝信道密码
		
			AppBleType.Respond.TxDataBuf[L_DATA+19]=0x02;  //ID2加密
		
			AppBleType.Respond.TxDataBuf[L_DATA+20]=0x02;  //AES加密 密码及添加账号传输
		
			AppBleType.Respond.TxDataBuf[L_DATA+21]=0x05;  //X平台+录指纹新协议53+54
			#if defined WIFI_FUNCTION_ON
			AppBleType.Respond.TxDataBuf[L_DATA+22]=0x00;  //无线模块0>WIFI   1>NB
			#endif
			#if defined NB_FUNCTION
			AppBleType.Respond.TxDataBuf[L_DATA+22]=0x01;  //无线模块0>WIFI   1>NB
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+23]=AES_ROOT_KEY; //AES加密根密

            //--解锁方式类型----------------2byte-------------------
		    OpenKind_U openKind;
			openKind.data = 0;
		    openKind.bit.PwdMode = FUNCTION_ENABLE;            //密码解锁方式     0: 不支持  1:支持
			#ifdef FINGER_FUNCTION_ON
			openKind.bit.FingerMode = FUNCTION_ENABLE;         //指纹解锁方式     0: 不支持  1:支持
			#endif
			#ifdef SMART_KEY_FUNCTION_ON
			openKind.bit.SmartKeyMode = FUNCTION_ENABLE;      //智能钥匙解锁（蓝牙钥匙/蓝牙手环）     0: 不支持  1:支持
			#endif
			#ifdef FACE_FUNCTION_ON
			openKind.bit.FaceMode = FUNCTION_ENABLE;           //人脸解锁方式     0: 不支持  1:支持
			#elif defined IRIS_FUNCTION_ON
//			openKind.bit.IrisOpenMode    = FUNCTION_ENABLE;    //虹膜解锁方式 	 0: 不支持  1:支持
			openKind.bit.FaceMode    = FUNCTION_ENABLE;    //虹膜解锁方式 	 0: 不支持  1:支持
            #endif
			#ifdef IC_CARD_FUNCTION_ON
			openKind.bit.IcCardMode = FUNCTION_ENABLE;         //IC卡解锁方式     0: 不支持  1:支持
			#endif
			#ifdef FINGER_VEIN_FUNCTION_ON
			openKind.bit.FingerVeinMode  = FUNCTION_ENABLE;   //指静脉解锁方式  0: 不支持  1:支持
			#endif
			#ifdef HW_WALLET_FUNCTION_ON
			openKind.bit.HuaWeiWalletMode = FUNCTION_ENABLE;  //华为钱包解锁方式 0: 不支持  1:支持
			#endif
			
            AppBleType.Respond.TxDataBuf[L_DATA+24] = (uint8_t)(openKind.data >> 8);	      
            AppBleType.Respond.TxDataBuf[L_DATA+25] = (uint8_t)(openKind.data >> 0);

            AppBleType.Respond.TxDataBuf[L_DATA+26] = 0x00;	    //离线指纹0枚
            AppBleType.Respond.TxDataBuf[L_DATA+27] = M_FINGER_MAX_ADMIN_NUM;	    //在线指纹或指静脉数量

            //--全动锁配置---------------2byte----------------------------------------------------
 
			AutoLockCfg_U  AutoCfg;
			AutoCfg.data = 0;
			//有无该功能
			AutoCfg.bit.GyroscopeModuleEn   = FUNCTION_DISABLE;       //陀螺仪模块             0: 无  1:有
			AutoCfg.bit.MotoDirectAdjustEn  = FUNCTION_ENABLE;        //开门方向调整（左右）   0: 无  1:有
			AutoCfg.bit.MotoTorsionAdjustEn = FUNCTION_ENABLE;        //电机扭力调节  		   0: 无  1:有
			#ifdef LOCK_BODY_216_MOTOR   
			AutoCfg.bit.MotoDirectAdjustEn  = FUNCTION_DISABLE;       //开门方向调整（左右）   0: 无  1:有
			AutoCfg.bit.MotoTorsionAdjustEn = FUNCTION_DISABLE;       //电机扭力调节  		   0: 无  1:有
			#endif
			#ifdef LOCK_BODY_212_MOTOR  //大电机
    		AutoCfg.bit.AutoLockTimAdjustEn = FUNCTION_ENABLE;        //自动上锁时间调节  	   0: 无  1:有
    	    AutoCfg.bit.AutoLockTimSetPara  = SystemSeting.SysAutoLockTime/5;  //自动上锁时间调节(0-32)*5秒     
			#elif defined LOCK_BODY_AUTO_MOTOR
			if( LockConfigMode == LOCK_BODY_212 )
			{
				AutoCfg.bit.AutoLockTimAdjustEn = FUNCTION_ENABLE;    //自动上锁时间调节  	   0: 无  1:有
			    AutoCfg.bit.AutoLockTimSetPara  = SystemSeting.SysAutoLockTime/5;  //自动上锁时间调节(0-32)*5秒     
			}
			#endif
			
            #ifdef LOCK_KEY_CONFIRM_ON			
			AutoCfg.bit.TouchLockConfirmEn  = FUNCTION_ENABLE;        //全自动锁门确认功能（触摸按键上锁确认） 0: 无  1:有	
			#endif
			//该功能开启状态
			AutoCfg.bit.GyroscopeModuleSts = FUNCTION_DISABLE;     	  //陀螺仪模块             0: 关闭   1:开启
            if( SystemFixSeting.MotorDirection == LEFT_HAND_DOOR )   
            {
                AutoCfg.bit.MotoDirectAdjustSts = FUNCTION_ENABLE;    //开门方向调整（左右）   0: 右开   1: 左开  
            }
            if( SystemFixSeting.MotorTorque == HIGH_TORQUE ) 
            {
                AutoCfg.bit.MotoTorsionAdjustSts = FUNCTION_ENABLE;   //电机扭力调节 		   0: 低扭力 1: 高扭力   
            }
 
			AppBleType.Respond.TxDataBuf[L_DATA+28] = (uint8_t)(AutoCfg.data >> 8);	
			AppBleType.Respond.TxDataBuf[L_DATA+29] = (uint8_t)(AutoCfg.data >> 0);	
			
			
			AppBleType.Respond.TxDataBuf[L_DATA+30] = 0x06;          //录入指纹次数

			//--锁功能配置1---------------2byte--------------------------------------------------------------------
	        FunctionCfg_U funcCfg;
            (void)memset( funcCfg.tab, 0, sizeof funcCfg );
			//有无该功能   功能配置1 
			#ifdef KEY_DEFENSE_ON
            funcCfg.bit.OneKeyDeployEn   = FUNCTION_ENABLE;    //一键布防功能       0: 无  1:有
			#endif
			funcCfg.bit.VolGradeAdjustEn = FUNCTION_ENABLE;    //音量调节           0: 无  1:有
			#ifdef HUMAN_ACTIVE_DEF_ON  
			funcCfg.bit.ActiveDefenseEn  = FUNCTION_ENABLE;    //主动防御(红外30秒检测) 0: 无  1:有
			#endif	
			funcCfg.bit.NfcFuncEn        = FUNCTION_DISABLE;   //NFC功能		    0: 无  1:有
			funcCfg.bit.OpenKindMoreEn   = FUNCTION_ENABLE;    //开门方式可选功能	0: 无  1:有
			#ifdef BELL_VIDEO_FUNC_ON
			funcCfg.bit.BellTakePictureEn= FUNCTION_DISABLE;   //门铃抓拍功能		0: 无  1:有
			#endif
			#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON  
			funcCfg.bit.NearSenseCheckEn = FUNCTION_DISABLE;   //红外距离功能		0: 无  1:有 
            #endif
			funcCfg.bit.EventRecordEn    = FUNCTION_ENABLE;    //事件记录功能		0: 无  1:有
			//该功能开启状态
			if( SystemSeting.SysKeyDef )
			{
				funcCfg.bit.OneKeyDeploySts= FUNCTION_ENABLE;    //一键布防开关状态       0: 关闭   1:开启
			}
			funcCfg.bit.NfcFuncSts         = FUNCTION_DISABLE;   //NFC功能开启状态        0: 关闭   1:开启  
			funcCfg.bit.BellTakePictureSts = FUNCTION_DISABLE;   //门铃抓拍功能开启状态   0: 关闭   1:开启  
			funcCfg.bit.EventRecordSts     = FUNCTION_ENABLE;    //事件记录功能开启状态   0: 关闭   1:开启  

			AppBleType.Respond.TxDataBuf[L_DATA+31] = funcCfg.tab[ 0 ];
			AppBleType.Respond.TxDataBuf[L_DATA+32] = funcCfg.tab[ 1 ];

            //--锁功能配置2---------------2byte--------------------------------------------------------------------
			//有无该功能  功能配置2 
			funcCfg.bit.OemBraceletEn = FUNCTION_DISABLE;    //小滴OEM新手环       0: 无  1:有
			#ifdef XM_CAM_FUNCTION_ON
			funcCfg.bit.CameraTypeEn  = FUNCTION_ENABLE;     //猫眼(雄迈、奥比、智诺等)结合A3指令  0: 无  1:有
			#endif
			#ifdef ST_CAM_FUNCTION_ON
			funcCfg.bit.STCameraEn    = FUNCTION_ENABLE;     //商汤猫眼       0: 无  1:有
			#endif
			#ifdef OB_CAM_FUNCTION_ON
			funcCfg.bit.ABCameraEn    = FUNCTION_ENABLE;     //奥比猫眼       0: 无  1:有	
			#endif
			#ifdef IC_CARD_FUNCTION_ON
			funcCfg.bit.IcCardEn      = FUNCTION_ENABLE;     //IC卡功能       0: 无  1:有	
			#endif
			#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON  
			funcCfg.bit.R8NearSenseAdjustEn = FUNCTION_ENABLE;   //R8红外距离可调(远/近/关闭) 0: 无  1:有  
			#endif
			funcCfg.bit.NewRemoteControlEn = FUNCTION_ENABLE;    //新遥控器  0: 无  1:有  
			#ifdef  OB_CAM_FUNCTION_ON
			funcCfg.bit.FaceForceEn = FUNCTION_ENABLE;            //人脸挟持开关  0: 无  1:有  
			#endif
			
			//该功能开启状态
			funcCfg.bit.STCameraSts = FUNCTION_DISABLE;           //商汤猫眼单双向 0: 单向   1:双向

			#if defined LOCK_BODY_216_MOTOR|| defined LOCK_BODY_218_MOTOR 
			funcCfg.bit.UnlockWarmEn = FUNCTION_ENABLE;          //门未关报警功能 0: 无  1:有	
			#elif defined LOCK_BODY_AUTO_MOTOR 
			if( LockConfigMode == LOCK_BODY_218 )
			{
				funcCfg.bit.UnlockWarmEn = FUNCTION_ENABLE;          //门未关报警功能 0: 无  1:有	
			}
			#endif
			funcCfg.bit.FingerGetID = FUNCTION_ENABLE;            //指纹ID查询
			#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
            funcCfg.bit.FaceCheckEn = FUNCTION_ENABLE;            //人脸验证开关  0: 无  1:有	
			#endif
			#if defined GET_WIFI_SIGNAL_INTENSITY_ON
			funcCfg.bit.GetWifiSignalIntensity = FUNCTION_ENABLE; //获取WIFI信号强度功能  0: 无  1:有
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+33] = funcCfg.tab[ 2 ];
			AppBleType.Respond.TxDataBuf[L_DATA+34] = funcCfg.tab[ 3 ];
 
            AppBleType.Respond.TxDataBuf[L_DATA+35]	= SystemSeting.SysVoice;       //当前音量
            AppBleType.Respond.TxDataBuf[L_DATA+36] = SystemSeting.SysHumanIrDef;  //当前主动防御时间
			AppBleType.Respond.TxDataBuf[L_DATA+37] = SystemSeting.SysLockMode;    //当前开门模式
			AppBleType.Respond.TxDataBuf[L_DATA+38] = SystemSeting.SysDrawNear;    //当前接近感应距离

			//--工装配置---------------8byte--------------------
			FactoryTestCfg_U factoryTest;
			(void)memset( factoryTest.tab, 0, sizeof factoryTest );
			factoryTest.bit.NfcCheckEn     = FUNCTION_DISABLE;       //NFC检测      0: 无  1:有
			factoryTest.bit.VersionCheckEn = FUNCTION_ENABLE;        //版本查询     0: 无  1:有
			factoryTest.bit.ReadMacAddrEn  = FUNCTION_ENABLE;        //读MAC地址    0: 无  1:有
			factoryTest.bit.CheckBatVol1En = FUNCTION_ENABLE;        //电压1        0: 无  1:有
			factoryTest.bit.CheckBatVol2En = FUNCTION_ENABLE;        //电压2        0: 无  1:有
			factoryTest.bit.PirTestEn      = FUNCTION_DISABLE;       //PIR测试      0: 无  1:有
			factoryTest.bit.WriteFlashEn   = FUNCTION_DISABLE;        //写flash      0: 无  1:有
			factoryTest.bit.ReadFlashEn    = FUNCTION_DISABLE;        //读flash      0: 无  1:有

			factoryTest.bit.ScreenTestEn   = FUNCTION_ENABLE;        //屏幕测试     0: 无  1:有
			factoryTest.bit.FingerInputEn  = FUNCTION_DISABLE;       //录入指纹     0: 无  1:有
			factoryTest.bit.FaceCommiteEn  = FUNCTION_DISABLE;       //人脸通信     0: 无  1:有
			#if defined WIFI_FUNCTION_ON
			factoryTest.bit.WifiSelfcheckEn= FUNCTION_ENABLE;        //WIFI自检     0: 无  1:有
			#endif
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
			factoryTest.bit.R8SecretKeyLoadEn=FUNCTION_ENABLE;      //R8密钥下发   0: 无  1:有
			#else
			factoryTest.bit.R8SecretKeyLoadEn=FUNCTION_DISABLE;
			#endif
			factoryTest.bit.InnerBatNormalEn=FUNCTION_DISABLE;       //内置锂电池(3.0V-4.3V)都为正常  0: 无  1:有
			factoryTest.bit.ClearAllUserEn = FUNCTION_ENABLE;        //清空用户       0: 无  1:有
            #ifdef XM_CAM_FUNCTION_ON
            factoryTest.bit.Q5M_V9RealTimeVideoEn = FUNCTION_ENABLE;//Q5M/V9实时视频 0: 无  1:有
            #else
            factoryTest.bit.Q5M_V9RealTimeVideoEn = FUNCTION_DISABLE;//Q5M/V9实时视频 0: 无  1:有
            #endif
 
			factoryTest.bit.PinCodeLoadEn   = FUNCTION_DISABLE;      //华为PIN下发    0: 无  1:有
			factoryTest.bit.BleDisconnectEn = FUNCTION_DISABLE;       //蓝牙断开       0: 无  1:有
			#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON
			factoryTest.bit.R8RealTimeVideoEn = FUNCTION_ENABLE;    //R8实时视频     0: 无  1:有
			#else
			factoryTest.bit.R8RealTimeVideoEn = FUNCTION_DISABLE;
			#endif
			factoryTest.bit.WifiSignalIntensityEn = FUNCTION_DISABLE; //WIFI产测扫描路由器信号强度  0: 无  1:有
			factoryTest.bit.IcCardCheckEn   = FUNCTION_DISABLE;      //IC卡检测             0: 无  1:有
			factoryTest.bit.LockModelLoadEn = FUNCTION_DISABLE;       //全自动锁体型号下发   0: 无  1:有
			factoryTest.bit.SnCodeLoadEn    = FUNCTION_DISABLE;      //华为SN下发           0: 无  1:有
 
			factoryTest.bit.EventRecordClearEn = FUNCTION_ENABLE;    //小滴工装软清空事件记录  0: 无  1:有
			factoryTest.bit.EncodeChipCheckEn  = FUNCTION_DISABLE;   //加密芯片检测            0: 无  1:有
			factoryTest.bit.ButtonOpenModeCfgEn= FUNCTION_DISABLE;   //开门键单击/双击开门设置 0: 无  1:有
		
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
			combineMode.bit.FaceAndFinger = FUNCTION_ENABLE;   //人脸 + 指纹    0: 无  1:有
			combineMode.bit.FaceAndPwd    = FUNCTION_ENABLE;   //人脸 + 密码    0: 无  1:有
            #elif defined IRIS_FUNCTION_ON
            combineMode.bit.IrisAndFinger = FUNCTION_ENABLE;   //虹膜 + 指纹    0: 无  1:有
            combineMode.bit.IrisAndPwd    = FUNCTION_ENABLE;   //虹膜 + 密码    0: 无  1:有
            #elif defined FINGER_VEIN_FUNCTION_ON
            combineMode.bit.VeinAndPwd = FUNCTION_ENABLE;      //指静脉 + 密码      0: 无  1:有
            #else
			combineMode.bit.FingerAndPwd  = FUNCTION_ENABLE;   //指纹 + 密码    0: 无  1:有    
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+47] = combineMode.data;//组合开门  

			
            //--锁功能配置3---------------2byte--------------------------------------------------------------------
			//有无该功能  功能配置3 
			#ifdef XM_CAM_SCREEN_FUNCTION_ON
			funcCfg.bit.ScreenIndoor = FUNCTION_ENABLE;    //后屏功能  0: 无  1:有     
			#endif
			#if LOCK_PROJECT_CHIP == LOCK_PROJECT_RTL8762
			funcCfg.bit.RTLMcu = FUNCTION_ENABLE;    //是否是RTL平台  0: 不是  1:是     
			#endif
			AppBleType.Respond.TxDataBuf[L_DATA+48] = funcCfg.tab[ 4 ];
			AppBleType.Respond.TxDataBuf[L_DATA+49] = funcCfg.tab[ 5 ];
			
			AppBleType.Respond.TxLen=50; //直接回复
		}
		break;
        case BLE_CMD_OTAMODE: //断开
        {
            AppBleType.Respond.TxLen=1;
            AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功 覆盖D
            AppBleRespondData(BLE_CMD_OTAMODE);//统一回包
            PUBLIC_Delayms(500);
			DRV_InterGenerateStartOta();
            break;
        }
		case BLE_TIMEUPDATA:         //时间同步
		{
			RTCType rtc_w;
			rtc_w.year = AppBleType.RxDataBuf[L_DATA];
			rtc_w.month = AppBleType.RxDataBuf[L_DATA+1];
			rtc_w.day =  AppBleType.RxDataBuf[L_DATA+2];
			rtc_w.week = AppBleType.RxDataBuf[L_DATA+3];
			rtc_w.hour = AppBleType.RxDataBuf[L_DATA+4];
			rtc_w.minuter = AppBleType.RxDataBuf[L_DATA+5];
			rtc_w.second = AppBleType.RxDataBuf[L_DATA+6];
			AppBleType.Respond.TxLen=1; //回包长
			//返回RTC错误码，成功RTC_Successfully
			AppBleType.Respond.TxDataBuf[L_DATA]=HAL_RTC_WriteTime(&rtc_w); 
			break;
		}
		case BLE_SMARTKEY_GETKEY1:
			 APP_BleSetAdminState(E_VIDEO_ADMIN); //分离屏看视频，允许用户返回退出
		case BLE_PHONE_GETKEY:       //获取随机数
		case BLE_SMARTKEY_GETKEY2:
		{
			AppBleType.Respond.TxLen=9; //回包长
		    PUBLIC_GenerateRandVec(AppBleType.RandomNum,8);//随机数生成
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],AppBleType.RandomNum,8);//拷贝发送
			break;
		}
		case BLE_AES_ADDPHONE:    	 //添加手机账号
		{
			//原始账号还原
			AppBleUserDataDecode(0,&AppBleType.RxDataBuf[L_DATA],16);
			//直接存储账号，小嘀产品仅一组
			PUBLIC_PrintHex ( "BLE_AES_ADDPHONE",&AppBleType.RxDataBuf[L_DATA], 16 );
			HAL_EEPROM_WriteBytes(MEM_PHONE_START,&AppBleType.RxDataBuf[L_DATA],13); 
			AppBleType.Respond.TxLen=1; //回包长
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
		    SystemEventLogSave( ADD_BLE, 0 ); 
		    break;
		}
		case BLE_DELPHONE:        	 //删除手机号
		{   
			//不解析手机发来的账号，直接删除本地一组
			APP_BleDelPhone();
			AppBleType.Respond.TxLen=1; //回包长
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
		    SystemEventLogSave( DELETE_BLE, 0 ); 
			break;
		}
		case BLE_ADDSMARTKEY_REQ: 	 //增加智能钥匙密钥到锁
		{
			for(uint8_t i=0;i<8;i++)//异或加密
			{
				AppBleType.RxDataBuf[L_DATA+i]^=AppBleType.RandomNum[i];
				AppBleType.RxDataBuf[L_DATA+i+8]^=AppBleType.RandomNum[i];
			}
		    SystemEventLogSave( ADD_SMART_KEY, 0 ); 
			SmartKeyWriteId(&AppBleType.RxDataBuf[L_DATA]); //存ID
			AppBleType.Respond.TxLen=1; //回包长
			AppBleType.Respond.TxDataBuf[L_DATA]=0;
			break;
		}
		case BLE_DELSMARTKEY: //删除智能钥匙
			SmartKeyDeleteId(&AppBleType.RxDataBuf[L_DATA]);
			AppBleType.Respond.TxLen=1; //回包长
			AppBleType.Respond.TxDataBuf[L_DATA]=0;
		    SystemEventLogSave( DELETE_SMART_KEY, 0 ); 
			break;
		case BLE_SMARTKEY_ENLOCK:
		{
			//蓝牙数据做加密比对
			uint16_t pageid=SmartkeyCheckRegId(AES_ENC,(const uint8_t*)&AppBleType.RxDataBuf[L_DATA]);
			if(PAGEID_NULL !=pageid)
			{
				uint8_t id[13]={0};
				SmartKeyReadId(pageid,id);//获取原始ID
				//开门成功
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
				AppBleType.Respond.TxLen=1; //回包长
				AppBleType.Respond.TxDataBuf[L_DATA]=0;
				
			}
			else//开门失败记禁试
			{				
				APP_BleSetAdminState(E_PHONE_CHECK_ERR);//验证失败	
				AppBleType.Respond.TxLen=1; //回包长
				AppBleType.Respond.TxDataBuf[L_DATA]=1;
			}
			break;	
		}
		case BLE_AES_ADD_PASSWORD  :     //开锁密码设置
		{
			//原始账号还原
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
				if( SystemSeting.SysPwdAllNum )  //修改密码
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
			AppBleType.Respond.TxLen=1; //回包长
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			break;
		}
		case BLE_AES_ADD_SOS_PASSWORD  : //SOS开锁密码设置
		{
			//原始账号还原
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
			AppBleType.Respond.TxLen=1; //回包长
		    AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			break;
		}
		
		case BLE_AES_ADD_TMP_PASSWORD  : //临时密码设置
		{
            TmpPwdMeg_T tmPwdmeg[TEMP_PWD_ALL_NUM] = {0};
			App_PWD_CreateTempPwds( tmPwdmeg );
			
			AppBleType.Respond.TxLen=0x3D;           //回包长
			AppBleType.Respond.TxDataBuf[L_DATA]=0;  //成功	
			
			for(uint8_t i=0; i<TEMP_PWD_ALL_NUM; i++)
			{
			    (void)memcpy( &AppBleType.Respond.TxDataBuf[L_DATA + 1+ i*TEMP_PWD_BYTE_SIZE], tmPwdmeg[i].Password, TEMP_PWD_BYTE_SIZE );
				PUBLIC_PrintHex("temp password:", tmPwdmeg[i].Password, TEMP_PWD_BYTE_SIZE);
			}
            AppBleUserDataDecode(1,&AppBleType.Respond.TxDataBuf[L_DATA+1],64);
 			AppBleType.Respond.TxLen=65;           //回包长
			
			App_PWD_SaveTempPwdsIntoEeprom( tmPwdmeg );
			
			break;
		}
		
		case BLE_PHONE_ENLOCK :  //手机蓝牙开门
		{
			for(uint8_t i=0;i<8;i++)//异或加密  有效部分13+4
			{
				AppBleType.RxDataBuf[L_DATA+i]^=AppBleType.RandomNum[i];
				AppBleType.RxDataBuf[L_DATA+i+8]^=AppBleType.RandomNum[i];
				AppBleType.RxDataBuf[L_DATA+i+8+8]^=AppBleType.RandomNum[i];
			}
			//验证信道密码
			//读取
			PUBLIC_PrintHex("BLE_PHONE_ENLOCK",&AppBleType.RxDataBuf[L_DATA],17);
			uint8_t PhoneID[17];
			memcpy(PhoneID,AppBleType.ChannelPwd,4); //拷贝信道密码
			HAL_EEPROM_ReadBytes(MEM_PHONE_START,&PhoneID[4],13); //读取ID 
			PUBLIC_PrintHex("PhoneID",PhoneID,17);
			for(uint8_t CheckLoop = 0; CheckLoop < 17; CheckLoop++)//ID对比
            {
                if(PhoneID[CheckLoop] != AppBleType.RxDataBuf[L_DATA+CheckLoop])
                {
					//验证失败
					AppBleType.Respond.TxLen=1; //回包长
					AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败	
					APP_BleSetAdminState(E_PHONE_CHECK_ERR);
                    break;
                }
                else if ( 16 == CheckLoop)
				{
					//验证成功上报电量
					APP_BleSetAdminState(E_PHONE_CHECK_OK);
					SystemEventLogSave( BLE_OPEN, 0 );  
					AppBleType.Respond.TxLen=3; //回包长
					AppBleType.Respond.TxDataBuf[L_DATA]=0; 
					uint32_t upper_bat_data  = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
					if(upper_bat_data>200) //第一路减2V当干电池判断，兼容老产品
					{
						upper_bat_data-=200;
					}
					AppBleType.Respond.TxDataBuf[L_DATA+1] = (uint8_t)(upper_bat_data  >> 8);//电量提示
					AppBleType.Respond.TxDataBuf[L_DATA+2] = (uint8_t)upper_bat_data;//电量提示
					#ifdef UNDER_BAT_ADC_ON
					uint32_t under_bat_data = HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10;
					AppBleType.Respond.TxDataBuf[L_DATA+3] = (uint8_t)(under_bat_data  >> 8);//电量提示
					AppBleType.Respond.TxDataBuf[L_DATA+4] = (uint8_t)under_bat_data;//电量提示
					AppBleType.Respond.TxLen=5; //回包长
					#endif
					//验证成功

					break;
                }
            }
			break;
		}
		
		case BLE_DEVICE_REG:	 //设备注册	
		{
			strcpy((char*)&AppBleType.Respond.TxDataBuf[L_DATA+1],"S70000000123456");
			PUBLIC_GetMacAdd(&AppBleType.Respond.TxDataBuf[L_DATA+16]);//拷贝MAC
            memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+16+6],AppBleType.ChannelPwd,4);//拷贝信道密码
			AppBleType.Respond.TxDataBuf[L_DATA]=0;  
			AppBleType.Respond.TxLen=1+15+6+4; //回包长
			SystemSeting.SystemAdminRegister = ADMIN_APP_REGISTERED;
			SystemWriteSeting((uint8_t *)&SystemSeting.SystemAdminRegister,2); //标记注册
			break;
		}
		case BLE_CLEAR_USERMEM:  //清空用户
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
		case BLE_ADMIN_PWD_CHECK://锁 管理密码验证
		{
			//不需要验证默认密码，直接回复
			if(SystemSeting.SystemAdminRegister!=ADMIN_NONE_REGISTERED)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //工程模式或APP注册后，都需要提示硬清空
			}
			else
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //数据已清空
			}
			AppBleType.Respond.TxLen=1; //回包长
			break;
   		}
		case BLE_GET_HWIFO:      //锁具软件版本号获取
		{
			AppBleType.Respond.TxLen = strlen(LOCK_VERSION);//0x13;
			strcpy((char*)&AppBleType.Respond.TxDataBuf[L_DATA],LOCK_VERSION);
		    AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功 覆盖D
		    break;
		}
		case BLE_CMD_DISCONNECT: //断开
		{
			AppBleType.Respond.TxLen=1;
		    AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功 覆盖D
			AppBleRespondData(AppBleType.RxCdm);//统一回包
			App_GUI_UpdateMenuQuitTime(1*100, true);
			break;
		}
		case BLE_SET_LOGUPWITHWIFI :   //开锁记录上传设置开关
		{
			if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
				SystemSeting.SysWifiLogSw = FUNCTION_ENABLE;
			else 
				SystemSeting.SysWifiLogSw = FUNCTION_DISABLE;
			SystemWriteSeting(&SystemSeting.SysWifiLogSw,1);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长
			break;
		}
		#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 
			case BLE_FACE_PRO_OTA :
			{
				BleRxCmdHandlePro = E_CMD_FACE_PRO_OTA;
				BleHandleProStep = 0;
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //工作中
				AppBleType.Respond.TxLen=1; //回包长
				break;
			}
			case BLE_CAM_BELL_ONE_WAY :
			{
				if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
					SystemSeting.SysWifiSingle = FUNCTION_DISABLE;
				else 
					SystemSeting.SysWifiSingle = FUNCTION_ENABLE;
				SystemWriteSeting(&SystemSeting.SysWifiSingle,1);
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
				AppBleType.Respond.TxLen=1; //回包长
				break;
			}
			case BLE_WIFI_MAIN_SW :        //WIFI主开关
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
		case BLE_WIFI_MAIN_SW :        //WIFI主开关
		{
			if( AppBleType.RxDataBuf[L_DATA] == 0x00 )
				SystemSeting.SysWifiMainSw = FUNCTION_ENABLE;
			else 
				SystemSeting.SysWifiMainSw = FUNCTION_DISABLE;
			SystemWriteSeting(&SystemSeting.SysWifiMainSw,1);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长
			break;
		}
		case BLE_CMD_SET_WIFI :        //配网
		{
			(void)memcpy( WifiLockMeg.Ssid, &AppBleType.RxDataBuf[L_DATA], 33 );
			(void)memcpy( WifiLockMeg.Passwd, &AppBleType.RxDataBuf[L_DATA+33], 65 );
			BleRxCmdHandlePro = E_CMD_ROUTER_CFG;
			BleHandleProStep = 0;
			break;
		}
		#endif
		case BLE_WIFI_CONNECTION_TEST ://测试 服务器连接状态
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
		case BLE_WIFI_FACTORY_TEST:     //wifi厂测扫描指定路由器
		{
			BleRxCmdHandlePro = E_CMD_WIFI_FACTORY_TEST;
			BleHandleProStep = 0;
			break;
		}
		case BLE_CMD_SET_WIFI_IP :     // WIFI上增加域名设置。
		{
			(void)memcpy( WifiLockMeg.severname, &AppBleType.RxDataBuf[L_DATA], 30 );
			(void)memcpy( WifiLockMeg.portno, &AppBleType.RxDataBuf[L_DATA + 30], 2);
			BleRxCmdHandlePro = E_CMD_SERVER_CFG;
			BleHandleProStep = 0;
			break;
		}
		case BLE_GET_WIFI_SIGNAL_INTENSITY :     // 获取WIFI信号强度
		{
			BleRxCmdHandlePro = E_CMD_GET_WIFI_SIGNAL_INTENSITY;
			BleHandleProStep = 0;
			break;
		}
		case BLE_VIOCE :  		  //音量设置
		{
			SystemSeting.SysVoice=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysVoice,1);
			HAL_Voice_VolumeSet( (VOL_GRADE_E)SystemSeting.SysVoice ); 
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长
			break;	
		}
		case BLE_IR_SET : 		  //红外设置（接近感应）
		{
			ComDataByte1 = AppBleType.RxDataBuf[L_DATA+1]; //第二个字节
			BleRxCmdHandlePro = E_CMD_NEAR_SENSE_CFG;
			BleHandleProStep = 0;
			break;	
		}
		case BLE_MOTOR_TORQUE :   //电机扭力
		{
			SystemFixSeting.MotorTorque=AppBleType.RxDataBuf[L_DATA]; //赋值
			SystemWriteFixSeting(&SystemFixSeting.MotorTorque,1); //写入
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长
			break;
		}
		case BLE_DIRECTION :      //左右开 
		{
			SystemFixSeting.MotorDirection=AppBleType.RxDataBuf[L_DATA]; //赋值
			SystemWriteFixSeting(&SystemFixSeting.MotorDirection,1); //写入
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长
			break;
		}
		case BLE_LOCK_MODE :	  //开门模式（组合开门）
		{
			if(AppBleType.RxDataBuf[L_DATA]==DOUBLE_CHECK_SW_ON) //开门与模式
			{
				SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_ON; 
			}
			else
			{
				SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_OFF; 
			}
			SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //写组合开门
			
			SystemSeting.SysLockMode=AppBleType.RxDataBuf[L_DATA+1]; //第二个字节是模式
			SystemWriteSeting(&SystemSeting.SysLockMode,1);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长
			break;	
		}
		case BLE_AUTO_LOCK_TIME : //自动上锁
		{
			SystemSeting.SysAutoLockTime=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysAutoLockTime,1);//写配置
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1;  //回包长	
			App_GUI_DefaultDoorState();  //防止自动上锁
			break;
		}
		case BLE_FACE_CHECK_EN : //人脸验证开关
		{
			SystemSeting.FaceCheckEnable=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.FaceCheckEnable,1);//写配置
			if((0x66 == SystemSeting.FaceCheckEnable) && (SystemSeting.SysCompoundOpen == DOUBLE_CHECK_SW_ON))//关闭人脸验证
			{
				SystemSeting.SysCompoundOpen=DOUBLE_CHECK_SW_OFF; 
				SystemWriteSeting(&SystemSeting.SysCompoundOpen,1); //写组合开门
			}
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1;  //回包长	
			break;
		}
		case BLE_KEY_DEF :   	  //一键布防
		{
			SystemSeting.SysKeyDef=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysKeyDef,1);//写配置
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长	
			break;
		}
		case BLE_IR_DEF :    	  //主动防御
		{
			SystemSeting.SysHumanIrDef=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.SysHumanIrDef,1);//写配置
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长	
			break;
		}
		case BLE_RGB_MODE :    	  //氛围灯模式配置
		{
			SystemSeting.SysHumanIrDef=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.LedRGBMode,1);//写配置
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长	
			break;
		}
		#if defined XM_CAM_FUNCTION_ON //猫眼获取序列号 下发密钥
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
		case BLE_DOOR_UNLOCK_WARM: //门未关报警设置
		{
			SystemSeting.DoorUnlockWarmSw=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.DoorUnlockWarmSw,1);//写配置
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长				
			break;
		}
		case BLE_LOCK_CHECK	:     //锁门确认键
		{
			SystemSeting.Sysprotect_lock=AppBleType.RxDataBuf[L_DATA]; 
			SystemWriteSeting(&SystemSeting.Sysprotect_lock,1);//写配置
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长				
			break;
		}
		case BLE_LOCK_CASE_LOG:   //事件记录
		{
			BleRxCmdHandlePro = E_CMD_BLE_LOCK_CASE;
			BleHandleProStep = 0;
			break;
		}
		case BLE_CLEAR_LCOK_LOG:
			SystemEventLogClear();
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长	
			break;
		case BLE_WIFI_TEST :      //WIFI推送指定号码
		{
			memcpy(WifiLockMeg.WifiFactoryTestNum,&AppBleType.RxDataBuf[L_DATA],19);//拷贝号码
			BleRxCmdHandlePro = E_CMD_WIFI_TEST;
			break;
		}	
		case BLE_MODEL_WRITE_TEST :  //BLE添加锁具型号 
		{
			memcpy(SystemFixSeting.LockNameType,&AppBleType.RxDataBuf[L_DATA],8);//拷贝型号
			SystemWriteFixSeting(SystemFixSeting.LockNameType,8); //存储
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长					
			break;
		}	
		
		case BLE_GET_LOCK_WIFI_MAC: //获取MAC，显示检验员编号
			
			App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );      //灭屏
			App_LED_OutputCtrl((LED_TYPE_E)led_num[0x0F & AppBleType.RxDataBuf[L_DATA]],EM_LED_ON);
			App_LED_OutputCtrl((LED_TYPE_E)led_num[0x0F & AppBleType.RxDataBuf[L_DATA+1]],EM_LED_ON);
			App_LED_OutputCtrl((LED_TYPE_E)led_num[0x0F & AppBleType.RxDataBuf[L_DATA+2]],EM_LED_ON);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxDataBuf[L_DATA+1]=6; 
			PUBLIC_GetMacAdd(&AppBleType.Respond.TxDataBuf[L_DATA+2]); //获取锁具MAC
			AppBleType.Respond.TxDataBuf[L_DATA+2+6]=6; 
			memset(&AppBleType.Respond.TxDataBuf[L_DATA+2+6+1],0,6); //WIFI MAC空
			AppBleType.Respond.TxLen=15; //回包长				
		    break;
		case BLE_MODEL_READ_TEST :   //获取型号
		{
			SystemReadFixSeting(SystemFixSeting.LockNameType,8); //读存储
			AppBleType.Respond.TxLen=9; //回包长		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],SystemFixSeting.LockNameType,8);//拷贝型号
			break;
		}	
		
		case BLE_EEPROM_WRITE_TEST : //无意义测试
		case BLE_EEPROM_READ_TEST:
		{
			AppBleType.Respond.TxLen=1; //回包长		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			break;
		}
		case BLE_READ_ADC_TEST :
		{
			uint32_t upper_bat_data  = (HAL_ADC_GetValidVal(EM_UPPER_BAT_DATA) / 10);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxDataBuf[L_DATA+1] = (uint8_t)(upper_bat_data  >> 8);//电量提示
			AppBleType.Respond.TxDataBuf[L_DATA+2] = (uint8_t)upper_bat_data;//电量提示
			AppBleType.Respond.TxLen=3; //回包长	
			break;
		}
		case BLE_READ_ADC2_TEST :
		{
			uint32_t upper_bat_data  = (HAL_ADC_GetValidVal(EM_UNDER_BAT_DATA) / 10);
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxDataBuf[L_DATA+1] = (uint8_t)(upper_bat_data  >> 8);//电量提示
			AppBleType.Respond.TxDataBuf[L_DATA+2] = (uint8_t)upper_bat_data;//电量提示
			AppBleType.Respond.TxLen=3; //回包长	
			break;
		}
		case BLE_LED_RED_GREEN_TEST :
			App_LED_OutputCtrl(EM_LED_ALL,EM_LED_ON);			
			AppBleType.Respond.TxLen=1; //回包长		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
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
		case BLE_ADDFINGER_ACK :     //确认指纹录入成功
		{
			break;
		}
		case BLE_DELFINGER	:        //删除指纹
		{
			ComDataByte2 = ((uint16_t)AppBleType.RxDataBuf[L_DATA+2]<<8) + (uint16_t)AppBleType.RxDataBuf[L_DATA+3];  
			BleRxCmdHandlePro = E_CMD_DELETE_FINGER;
			BleHandleProStep = 0;
			break;
		}
		case BLE_FINGER_WRTITE :     //修改指纹属性
		{
			FingerAppCfg_S fingerMeg = {0};
			// 指纹使能
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_EN ] = MEM_FACT_MEM_FIG;   
			// 模组编号H
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_NUM_H ] = AppBleType.RxDataBuf[L_DATA+2];   
			// 模组编号L
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_NUM_L ] = AppBleType.RxDataBuf[L_DATA+3];  
			// 管理员标记
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_ADMIN_EN ] = MEM_USER_MASTER;  
			// 亲情标记
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_FAMILY_EN ] = AppBleType.RxDataBuf[L_DATA+12]&0xf0;  
			// SOS胁迫标记
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_SOS_EN ] = AppBleType.RxDataBuf[L_DATA+12]&0x0f;  
			// 时效标记
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_TIME_EN ] = AppBleType.RxDataBuf[L_DATA+13];  
			// 开始时间: 年月日时分秒   0x08
			(void)memcpy( &fingerMeg.acOffset[ EM_FINGER_APP_CFG_START_YEAR ], &AppBleType.RxDataBuf[L_DATA+14], 6 );
			// 结束时间: 年月    
			(void)memcpy( &fingerMeg.acOffset[ EM_FINGER_APP_CFG_END_YEAR ], &AppBleType.RxDataBuf[L_DATA+20], 2 );
			// 结束时间: 日时分秒       0x10
			(void)memcpy( &fingerMeg.acOffset[ EM_FINGER_APP_CFG_END_DAY ], &AppBleType.RxDataBuf[L_DATA+22], 4 );
			// 周
			fingerMeg.acOffset[ EM_FINGER_APP_CFG_WEEK ] = AppBleType.RxDataBuf[L_DATA+26];  
 
            APP_FINGER_CfgWrite( fingerMeg );
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
			AppBleType.Respond.TxLen=1; //回包长		
			break;
		}
#ifdef XM_CAM_SCREEN_FUNCTION_PLUS_ON
		case BLE_CMD_SET_FOV_PARAM :     // 获取WIFI信号强度
		{
			BleRxCmdHandlePro = E_CMD_SET_FOV_PARAM;
			BleHandleProStep = 0;
			break;
		}
#endif

		default :
//			AppBleType.Respond.TxLen=1; //回包长		
//			AppBleType.Respond.TxDataBuf[L_DATA]=0x33; //未知指令
			break;
	}
	if(AppBleType.Respond.TxLen)
	{
		AppBleRespondData(AppBleType.RxCdm);//统一回包
	}
}


/***************************************************************************************
**函数名:       BleFaceCasePro
**功能描述:     人脸模组蓝牙数据处理
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void BleFaceCasePro(void)
{
	#ifdef  FACE_FUNCTION_ON
	static uint8_t result = 0;
	if (BLE_FACEADD==AppBleType.RxCdm)//当前蓝牙事件增加人脸流程
	{
		if(AppFaceWorkPro.Register==FACE_ADD_FRONT) //第一次进来
		{
			if(SystemSeting.SysFaceAllNum>=MSG_FACE_USER_NUM)
			{
				AppBleType.Respond.TxLen=1; //回包长		
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败，人脸已满
				AppBleRespondData(AppBleType.RxCdm);//回包
				AppBleType.RxCdm=0; //结束
				return;
			}
			else 
			{
				AppBleType.Respond.TxLen=2; //回包长		
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //登记流程中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=0; //开始注册	
				AppBleRespondData(AppBleType.RxCdm);//回包	
			}	
	    }		
		uint8_t sta=FaceEnrollPro(MEM_USER_MASTER);//增加人脸
		static uint8_t sta_copy=0;
		if(sta!=sta_copy) //指令只发一次
		{
			sta_copy=sta;
			switch(sta)
			{
			case FACE_ADD_UP: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=1;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;
			case FACE_ADD_DOWN: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=2;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;
			case FACE_ADD_LEFT: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=4;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;
			
			case FACE_ADD_SUCCESSFUL: //登记完成最后一个方向 存储
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=5;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);	
				result = 1;//成功
				break;
			
			case FACE_ADD_RIGHT: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=3;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;	
			
			case FACE_ADD_ERROR:
				result = 2;//失败
				break;
			
			case FACE_ADD_OVER: //下电完成
				if(result == 1)//成功
				{
					AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
					AppBleType.Respond.TxDataBuf[L_DATA+1]=FaceAttribute.FacePageId<<8;//回ID
					AppBleType.Respond.TxDataBuf[L_DATA+2]=FaceAttribute.FacePageId&0xff;//回ID
					AppBleType.Respond.TxLen=3; //回包长
					AppBleRespondData(BLE_FACEADD);
				}
				else if(result == 2)//失败
				{
					AppBleType.Respond.TxDataBuf[L_DATA]=1;//失败
					AppBleType.Respond.TxLen=1; //回包长
					AppBleRespondData(BLE_FACEADD);
					App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
				}
				result = 0;
				AppFaceWorkPro.Register = FACE_ADD_FRONT;
				AppBleType.RxCdm=0;//直接结束
				break;
			default:
				break;
			}
		}
	}
	else if(BLE_FACESUCSSFUL==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleType.Respond.TxLen=1; //回包长
		AppBleType.RxCdm=0; //直接结束
		AppBleRespondData(BLE_FACESUCSSFUL);//回复
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
		//时效开始
		FaceAttribute.tm_vaild.start.year = AppBleType.RxDataBuf[L_DATA + 4];
		FaceAttribute.tm_vaild.start.month = AppBleType.RxDataBuf[L_DATA + 5];
		FaceAttribute.tm_vaild.start.day = AppBleType.RxDataBuf[L_DATA + 6];
		FaceAttribute.tm_vaild.start.hour = AppBleType.RxDataBuf[L_DATA + 7];
		FaceAttribute.tm_vaild.start.minuter = AppBleType.RxDataBuf[L_DATA + 8];
		FaceAttribute.tm_vaild.start.second = AppBleType.RxDataBuf[L_DATA + 9];
		//时效开始
		FaceAttribute.tm_vaild.stop.year = AppBleType.RxDataBuf[L_DATA + 10];
		FaceAttribute.tm_vaild.stop.month = AppBleType.RxDataBuf[L_DATA + 11];
		FaceAttribute.tm_vaild.stop.day = AppBleType.RxDataBuf[L_DATA + 12];
		FaceAttribute.tm_vaild.stop.hour = AppBleType.RxDataBuf[L_DATA + 13];
		FaceAttribute.tm_vaild.stop.minuter = AppBleType.RxDataBuf[L_DATA + 14];
		FaceAttribute.tm_vaild.stop.second = AppBleType.RxDataBuf[L_DATA + 15];
		//周时效 BIT1 -BIT 7表示周一到周日
		FaceAttribute.tm_vaild.wday = AppBleType.RxDataBuf[L_DATA + 16];
		
		FaceWrite(FaceAttribute, FaceId);
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleType.Respond.TxLen=1; //回包长
		AppBleType.RxCdm=0; //直接结束
		AppBleRespondData(BLE_FACE_WRTITE);//回复
	}	
	else if(BLE_FACE_DEL==AppBleType.RxCdm)
	{
		if(FaceDeleteAppUser(AppBleType.RxDataBuf[L_DATA+1]))//删除单个用户完成, 传低字节
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			AppBleType.Respond.TxLen=1; //回包长
			AppBleType.RxCdm=0; //直接结束
			AppBleRespondData(BLE_FACE_DEL);//回复
		}
	}
	else if(BLE_ADDFINGER_ACK==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleType.Respond.TxLen=1; //回包长
		AppBleType.RxCdm=0; //直接结束
		AppBleRespondData(BLE_ADDFINGER_ACK);//回复
	}
	else if(BLE_FACE_VERSION==AppBleType.RxCdm) //获取版本号
	{
		uint8_t Task=FaceGneralTaskFlow(FACE_CMD_GETVERSION,0,0,FACE_DEFAULT_TIMEOUT_1S);
		if(Task==TASK_SUCCESS)//获取成功
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&FaceMsgType.Reply.DataPack.Version,sizeof(FaceMsgType.Reply.DataPack.Version));
			AppBleType.Respond.TxLen=sizeof(FaceMsgType.Reply.DataPack.Version)+1; //回包长
		}
		else if(Task==TASK_POWERDOWN) //下电完成
		{
			AppBleRespondData(BLE_FACE_VERSION);//回复
			AppBleType.RxCdm=0; //直接结束			
		}
	}
	else if(BLE_FACE_HIJACK==AppBleType.RxCdm) //防劫持开关，商汤没有
	{
		//直接等下电完成
		if(FaceGneralTaskFlow(FACE_CMD_HIJACK_MODE,&AppBleType.RxDataBuf[L_DATA],1,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			AppBleRespondData(BLE_FACE_HIJACK);//回复
			AppBleType.RxCdm=0; //直接结束		
		}
	}
	else if(BLE_FACE_THRESHOLD_LEVEL==AppBleType.RxCdm)//识别安全等级调节
	{
		if(FaceGneralTaskFlow(FACE_CMD_SET_THRESHOLD_LEVEL,&AppBleType.RxDataBuf[L_DATA],2,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			AppBleRespondData(BLE_FACE_THRESHOLD_LEVEL);//回复
			AppBleType.RxCdm=0; //直接结束	
		}
	}	
	
	#elif defined IRIS_FUNCTION_ON
	if (BLE_FACEADD==AppBleType.RxCdm)//当前蓝牙事件增加人脸流程
	{
		if(AppFaceWorkPro.Register==FACE_ADD_FRONT) //第一次进来
		{
			if(SystemSeting.SysFaceAllNum>=MSG_FACE_USER_NUM)
			{
				AppBleType.Respond.TxLen=1; //回包长		
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败，人脸已满
				AppBleRespondData(AppBleType.RxCdm);//回包
				AppBleType.RxCdm=0; //结束
				return;
			}
			else 
			{
				AppBleType.Respond.TxLen=2; //回包长		
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //登记流程中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=0; //开始注册	
				AppBleRespondData(AppBleType.RxCdm);//回包	
			}	
	    }		
		uint8_t sta=FaceEnrollPro(MEM_USER_MASTER);//增加人脸
		static uint8_t sta_copy=0;
		if(sta!=sta_copy) //指令只发一次
		{
			sta_copy=sta;
			switch(sta)
			{
			case FACE_ADD_LEFT: 
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=3;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;	
			
			case FACE_ADD_SUCCESSFUL: //登记完成最后一个方向 存储
				AppBleType.Respond.TxDataBuf[L_DATA]=2;//执行中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=5;//完成一个反向
				AppBleType.Respond.TxLen=2; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;
			
			case FACE_ADD_ERROR:
				App_LED_OutputCtrl( EM_LED_CFG_NET, EM_LED_ON ); //亮灯25846
				AppBleType.Respond.TxDataBuf[L_DATA]=1;//失败
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData(BLE_FACEADD);
				break;
			
			case FACE_ADD_OVER: //下电完成
				AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
				AppBleType.Respond.TxDataBuf[L_DATA+1]=FaceAttribute.FacePageId<<8;//回ID
				AppBleType.Respond.TxDataBuf[L_DATA+2]=FaceAttribute.FacePageId&0xff;//回ID
				AppBleType.Respond.TxLen=3; //回包长
				AppBleRespondData(BLE_FACEADD);
				AppBleType.RxCdm=0;//直接结束
				break;

			default:
				break;
			}
		}
	}
	else if(BLE_FACESUCSSFUL==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleType.Respond.TxLen=1; //回包长
		AppBleType.RxCdm=0; //直接结束
		AppBleRespondData(BLE_FACESUCSSFUL);//回复
	}
	else if(BLE_FACE_WRTITE==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleType.Respond.TxLen=1; //回包长
		AppBleType.RxCdm=0; //直接结束
		AppBleRespondData(BLE_FACE_WRTITE);//回复
	}	
	else if(BLE_FACE_DEL==AppBleType.RxCdm)
	{
		if(FaceDeleteAppUser(AppBleType.RxDataBuf[L_DATA+1]))//删除单个用户完成, 传低字节
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			AppBleType.Respond.TxLen=1; //回包长
			AppBleType.RxCdm=0; //直接结束
			AppBleRespondData(BLE_FACE_DEL);//回复
		}
	}
	else if(BLE_ADDFINGER_ACK==AppBleType.RxCdm)
	{
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleType.Respond.TxLen=1; //回包长
		AppBleType.RxCdm=0; //直接结束
		AppBleRespondData(BLE_ADDFINGER_ACK);//回复
	}
	else if(BLE_FACE_VERSION==AppBleType.RxCdm) //获取版本号
	{
		uint8_t Task=FaceGneralTaskFlow(FACE_CMD_GETVERSION,0,0,FACE_DEFAULT_TIMEOUT_1S);
		if(Task==TASK_SUCCESS)//获取成功
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&FaceMsgType.Reply.DataPack.Version,sizeof(FaceMsgType.Reply.DataPack.Version));
			AppBleType.Respond.TxLen=sizeof(FaceMsgType.Reply.DataPack.Version)+1; //回包长
		}
		else if(Task==TASK_POWERDOWN) //下电完成
		{
			AppBleRespondData(BLE_FACE_VERSION);//回复
			AppBleType.RxCdm=0; //直接结束			
		}
	}
	else if(BLE_FACE_HIJACK==AppBleType.RxCdm) //防劫持开关，商汤没有
	{
		//直接等下电完成
		if(FaceGneralTaskFlow(FACE_CMD_HIJACK_MODE,&AppBleType.RxDataBuf[L_DATA],1,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			AppBleRespondData(BLE_FACE_HIJACK);//回复
			AppBleType.RxCdm=0; //直接结束		
		}
	}
	else if(BLE_FACE_THRESHOLD_LEVEL==AppBleType.RxCdm)//二代人脸设置光敏
	{
		
	}	
	#endif
		
	#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 	
	else if(BLE_FACE_SET_LINKKEY==AppBleType.RxCdm)//设置阿里云密钥
	{
		if(FaceGneralTaskFlow(FACE_CMD_SET_LINKKEY,&AppBleType.RxDataBuf[L_DATA],96*2,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			AppBleRespondData(BLE_FACE_SET_LINKKEY);//回复
			AppBleType.RxCdm=0; //直接结束	
		}
	}	
	else if(BLE_CAM_GET_SN==AppBleType.RxCdm)//获取猫眼序列号
	{
		uint8_t Task=FaceGneralTaskFlow(FACE_CMD_DEVICENAME,0,0,FACE_PASS_DATA_TIMEOUT_5S);
		if(Task==TASK_SUCCESS)
		{
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&FaceMsgType.Reply.DataPack.Data,32);
			AppBleType.Respond.TxLen=33; //回包长
		}
		else if(Task==TASK_POWERDOWN)
		{
			AppBleRespondData(BLE_CAM_GET_SN);//回复
			AppBleType.RxCdm=0; //直接结束	
		}
	}	
	else if(BLE_CMD_SET_WIFI==AppBleType.RxCdm)//配网
	{
		int8_t Result=FaceProSetSSID(&AppBleType.RxDataBuf[L_DATA],98);
		if( Result != -1)
		{
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=Result;//结果
			AppBleRespondData(BLE_CMD_SET_WIFI);//回复
			AppBleType.RxCdm=0; //直接结束	
		}
	}	
	#endif

	return;
}

/***************************************************************************************
**函数名:       BleFingerCasePro
**功能描述:     手指模组蓝牙数据处理
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void BleFingerCasePro(void)
{
    /* 当前接口只允许在APP 模式下录入指纹 */
    //if(ADMIN_APP_REGISTERED != App_GUI_GetRegisterSts()) { return; }

    static FINGER_APP_ADD_FLOW_E emFlag = FINGER_APP_ADD_OVER;
    static uint16_t u16ID = 0;
    
    /* 当前蓝牙事件增加人脸流程 */
	if(BLE_ADDFINGER_NEW == AppBleType.RxCdm)//
	{
        if( 0 == DRV_GetBleConnect())
        {
            AppBleType.RxCdm = 0; //结束
            (void)APP_FINGER_Sleep();
            emFlag = FINGER_APP_ADD_OVER;
            (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
            (void)HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );	
            return;
        }
        
        uint16_t u16Pad = 0;
        FINGER_APP_FLOW_RESULT_E emRet = APP_FINGER_GetFlowResult(&u16Pad);
	    FINGER_APP_ADD_FLOW_E em = APP_FINGER_GetAddFingerFlow();

	    /* 指纹模组无状态或不在录入人脸状态 */
        if(FINGER_APP_RESULT_IDLE == emRet)
        {
            (void)App_LED_OutputCtrl(EM_LED_ALL, EM_LED_OFF);
            emFlag = em;
            /* 满了则直接返回，不再允许添加指纹 */
            if(false == APP_FINGER_CfgGetNullNum(EM_FINGER_APP_TYPE_ADMIN,&u16ID))
            {
				AppBleType.Respond.TxLen = 1; //回包长
				AppBleType.Respond.TxDataBuf[L_DATA] = 1; //失败，人脸已满
				AppBleRespondData(AppBleType.RxCdm);//回包
				AppBleType.RxCdm = 0; //结束
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
            /* 启动增加指纹流程 ,启动失败，则不再允许添加指纹*/
    		if(false == APP_FINGER_Operate( fingerPara ))
            {
				AppBleType.Respond.TxLen = 1; //回包长
				AppBleType.Respond.TxDataBuf[L_DATA] = 1; //失败，人脸已满
				AppBleRespondData(AppBleType.RxCdm);//回包
				AppBleType.RxCdm = 0; //结束
                (void)APP_FINGER_Sleep();
				emFlag = FINGER_APP_ADD_OVER;
                (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
                (void)HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );	
				return;
            }

            /* APP_FINGER_Operate 返回true，流程开始运转 */
			AppBleType.Respond.TxLen = 2; //回包长
			AppBleType.Respond.TxDataBuf[L_DATA] = 2; //登记流程中
			AppBleType.Respond.TxDataBuf[L_DATA+1] = 0; //开始注册	
			AppBleRespondData(BLE_ADDFINGER_NEW);//回包
        }
        /* 录入指纹1 ~ 6  */
        else if(em >= FINGER_APP_ADD_1ST && em <= FINGER_APP_ADD_6TH && emFlag != em)
        {
            emFlag = em;
			AppBleType.Respond.TxDataBuf[L_DATA] = 2;//执行中
			AppBleType.Respond.TxDataBuf[L_DATA+1] = em;//完成一个反向
			AppBleType.Respond.TxLen = 2; //回包长
			AppBleRespondData(BLE_ADDFINGER_NEW);

            if(FINGER_APP_ADD_2ND == em || FINGER_APP_ADD_4TH== em)
            {
    			AppBleType.Respond.TxDataBuf[L_DATA] = 0;
    			AppBleType.Respond.TxDataBuf[L_DATA+1] = 0;//PageID - 1st byte
    			AppBleType.Respond.TxDataBuf[L_DATA+2] = 0;//PageID - 2nd byte
    			AppBleType.Respond.TxDataBuf[L_DATA+3] = 0;//PageID - 3rd byte
    			AppBleType.Respond.TxDataBuf[L_DATA+4] = u16ID;//PageID - 4th byte
    			AppBleType.Respond.TxDataBuf[L_DATA+5] = em;//完成一个反向
    			AppBleType.Respond.TxLen = 6; //回包长
    			AppBleRespondData(BLE_ADDFINGER_NEW);
            }
        }
        /* 完成录入  */
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
    			AppBleType.Respond.TxDataBuf[L_DATA+5] = FINGER_APP_ADD_6TH;//完成一个反向
    			AppBleType.Respond.TxLen = 6; //回包长
    			AppBleRespondData(BLE_ADDFINGER_NEW);
            }
            else
            {
    			AppBleType.Respond.TxDataBuf[L_DATA] = 1;
    			AppBleType.Respond.TxLen = 1; //回包长
    			AppBleRespondData(BLE_ADDFINGER_NEW);
                (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
                (void)HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );	
            }
            AppBleType.RxCdm = 0; //直接结束
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
        AppBleType.Respond.TxDataBuf[L_DATA] = 0;//成功
        AppBleType.Respond.TxDataBuf[L_DATA+1] = 0;
	    AppBleType.Respond.TxLen = 2; //回包长
        AppBleRespondData(AppBleType.RxCdm);
        AppBleType.RxCdm = 0; //直接结束
        (void)App_LED_OutputCtrl(EM_LED_CFG_NET, EM_LED_ON);
        HAL_Voice_PlayingVoice( EM_REGISTER_SUCCESS_MP3, 150 );	
	}
	else if(BLE_GET_FINGERID == AppBleType.RxCdm)
	{
	    uint8_t u8IdBuf[MSG_FINGER_NUM_RESERVED] = {0};
        uint8_t len = 0;

        if(APP_FINGER_GetFingerID(MSG_FINGER_NUM_RESERVED, u8IdBuf, &len))
        {
            AppBleType.Respond.TxDataBuf[L_DATA] = 0;//成功
            for(uint8_t i = 0 ; i < len; i++) {AppBleType.Respond.TxDataBuf[L_DATA+i+1] = u8IdBuf[i];}
    	    AppBleType.Respond.TxLen = len+1; //回包长
        }
        else
        {
            AppBleType.Respond.TxDataBuf[L_DATA] = 1;//失败
    	    AppBleType.Respond.TxLen = 1; //回包长
        }
	
        AppBleRespondData(AppBleType.RxCdm);
        AppBleType.RxCdm = 0; //直接结束
	}

    return;
}
 
/***************************************************************************************
**函数名:       BLE_GetFifoData
**功能描述:     蓝牙数据解析
**输入参数:      
**输出参数:     结果1：成功收包
**备注:        
****************************************************************************************/
uint8_t BLE_GetFifoData (void)
{
	uint8_t pdata=0;
	uint16_t len; //包长
	do  //取数据直到队列为空 或者 取到完整一包
	{
		if(AppBleType.RxPos==0) //无数据
		{
			if( app_fifo_get(&AppBleFifo, &pdata ) == FIFO_SUCCESS) //队列取数据
			{
				if(BLE_APP_HEAD==pdata) //包头
				{
					AppBleType.RxDataBuf[AppBleType.RxPos++]=pdata; //开始接受
				}				
			}
			else
			{
				return 0; //无数据
			}
		}
		else
		{
			if(AppBleType.RxPos>=L_DATA)//判断包长
			{
				len=AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL];
			}			
			//一包接完(L_HEAD+L_TYPE+L_CMD+L_LENH+L_LENL) +len +(CRCH+CRCL)
			if((5+len+2) ==AppBleType.RxPos)
			{
				AppBleType.RxPos=0;
				App_GUI_UpdateMenuQuitTime(60*100, true); //跟新休眠时间
				BLEParse();				
				return 1;
			}
			else
			{
				if( app_fifo_get(&AppBleFifo, &pdata ) == FIFO_SUCCESS) //队列取数据
				{
					AppBleType.RxDataBuf[AppBleType.RxPos++]=pdata;//继续接收
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
**函数名:       APP_BleSetAdminState
**功能描述:     设置当前权限状态
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
void APP_BleSetAdminState(ADMIN_STATE mode)
{
	AppBleType.Admin=mode;
}

/***************************************************************************************
**函数名:       APP_BleGetAdminState
**功能描述:     获取当前权限状态
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
ADMIN_STATE APP_BleGetAdminState(void)
{
	return AppBleType.Admin;
}

/*********************************************************************************************************************
* Function Name :  App_BleDelFingerPro()
* Description   :  删除对应的编号的指纹流程   
* Para  Input   :  pageId: 待删除的指纹编号
* Return        :  -1= 删除失败  0= 执行中  1= 删除成功
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
		case 0:   //启动流程
				memset( (void*)&fingerPara, 0, sizeof fingerPara );
				fingerPara.emAppFlow = EM_FINGER_APP_FLOW2_DEL;
				fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_NUM_H ] = (uint8_t)(pageId >> 8);
		        fingerPara.stFingerAppCfg.acOffset[ EM_FINGER_APP_CFG_NUM_L ] = (uint8_t)(pageId >> 0);
				APP_FINGER_Operate( fingerPara );
				step = 1;	
		break;
		
		case 1:   //等待结果
			    ret = APP_FINGER_GetFlowResult( &pageid );
				if( FINGER_APP_RESULT_SUC == ret )         //添加成功
				{
					APP_FINGER_Sleep();  //关闭指纹模组
					step = 0;
					return 1; 	
				}
				else if( FINGER_APP_RESULT_FAIL == ret )   //添加失败
				{
					APP_FINGER_Sleep();  //关闭指纹模组
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
* Description   :  APP注册配对码输入
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
			return 2;//工程模式或APP注册后，都需要提示硬清空
		}
		App_Touch_FuncEnCtrl( EM_SCAN_NONE_LOCK );      //扫描数字键
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON );    //灯全量		
	}	
	if( App_Touch_GetCurrentKeyIndex() >= 4 )//获取键值个数
	{
		uint8_t  buflen = 0;
		uint8_t inputBuf[4]={0};
		App_Touch_GetCurrentKeyValBuf( inputBuf, &buflen ); 
		PUBLIC_ChangeDecToString( inputBuf, inputBuf, 4 );	 //转字符串
		
	    App_Touch_FuncEnCtrl( EM_SCAN_OFF );  
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );      //灭屏
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
			return 1;//失败
		}
	}
	return -1;
}


/*********************************************************************************************************************
* Function Name :  App_BleProHandleThread
* Description   :  蓝牙执行指令动作   
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
			if( -1 != tp1 )   //完成
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=tp1; 
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_REG_CHECK_CODE);
				AppBleType.RxCdm=0; 
			}
		}
		break;
		case E_CMD_UNREGIST_DEVICE: //注销设备
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
			 
			if( 1 == tryflg )      //设定成功
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0;  
				AppBleType.Respond.TxLen=1;  
				AppBleRespondData(BLE_CLEAR_USERMEM);
				BleRxCmdHandlePro = E_CMD_DEFAULT;
				HAL_Voice_PlayingVoice( EM_SET_SUCCESS_MP3, 0 );
				App_GUI_UpdateMenuQuitTime(1*100, true); //更新休眠时间
				tryflg =0;
			}
			else if( 2 == tryflg ) //设定失败
			{
                App_LED_OutputCtrl( EM_LED_LOCK_R, EM_LED_ON ); 
                App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
                App_LED_OutputCtrl( EM_LED_X, EM_LED_ON );
				HAL_Voice_PlayingVoice( EM_WARM_ALARM_MP3, 300 );
				tryflg = 3;
			}
			else if( 3 == tryflg )  //等带语音播报完毕
			{
				if( 0 == HAL_Voice_GetBusyState() )
				{
					tryflg = 4;
                    HAL_Voice_PlayingVoice( EM_VOICE_BREAK_CMD, 0 ); 
				}
			}
			else if( 4 == tryflg )  //等带语音播报完毕
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1;  
				AppBleType.Respond.TxLen=1;  
				AppBleRespondData(BLE_CLEAR_USERMEM);
				BleRxCmdHandlePro = E_CMD_DEFAULT;
				HAL_Voice_PlayingVoice( EM_SET_FAIL_MP3, 0 );
				App_GUI_UpdateMenuQuitTime(1*100, true); //更新休眠时间
				tryflg = 0;
			}
		}
		break;
		
		case E_CMD_ROUTER_CFG:      //路由器配置
		{
			#if defined XM_CAM_FUNCTION_ON //猫眼配网
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
				if( WIFI_CONFIG_SUCESS == tp1 )         //成功
				 {
					AppBleType.Respond.TxDataBuf[L_DATA] =0;  
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(BLE_CMD_SET_WIFI);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				 }
				 else if( WIFI_CONFIG_FAIL == tp1 )      //失败
				 {
					AppBleType.Respond.TxDataBuf[L_DATA] =1;  
					AppBleType.Respond.TxLen =1;  
					AppBleRespondData(BLE_CMD_SET_WIFI);
					BleRxCmdHandlePro = E_CMD_DEFAULT;
				 }
			#endif
		}
		break;
		
		case E_CMD_SERVER_CFG:      //服务器配置，仅WIFI模组支持
		{
             tp1 = App_WIFI_ConfigThread( WIFI_CMD_CONF_SERVER, &BleHandleProStep );
			 if( WIFI_CONFIG_SUCESS == tp1 )         //成功
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =0;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_CMD_SET_WIFI_IP);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			 }
			 else if( WIFI_CONFIG_FAIL == tp1 )      //失败
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =1;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_CMD_SET_WIFI_IP);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT; 
			 }
		}
		break;
		
		case E_CMD_WIFI_FACTORY_TEST:      ///产测扫描路由器
		{
			//不需要使用宏，94决定是否执行
			 if(( WIFI_CONFIG_SUCESS == tp1 ) && (WifiRxData.AckResult == WIFI_ACK_OK))         //成功
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =0;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_WIFI_FACTORY_TEST);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			 }
			 else if(( WIFI_CONFIG_FAIL == tp1 ) || (WifiRxData.AckResult == WIFI_ACK_FAIL))     //失败
			 {
				AppBleType.Respond.TxDataBuf[L_DATA] =1;  
				AppBleType.Respond.TxLen =1;  
				AppBleRespondData(BLE_WIFI_FACTORY_TEST);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT; 
			 }
		}
		break;
		
		case E_CMD_NEAR_SENSE_CFG:  //接近感应配置
		{
		     tp1 = App_HumanSensorSet( ComDataByte1, &BleHandleProStep );
		     if( tp1 == 1 )
			 {
				SystemSeting.SysDrawNear = ComDataByte1;
			    SystemWriteSeting(&SystemSeting.SysDrawNear,1);	
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData(BLE_IR_SET);
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			 }
			 else if( tp1 == -1 )
			 {
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData(BLE_IR_SET);
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			 } 
		}
		break; 
		
		case E_CMD_SERVER_TEST:     //服务器测试
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
			    #if defined XM_CAM_FUNCTION_ON  //猫眼透传
    			tp1 = CAM_GetServerState(taskid);
    			if( tp1 == CAM_SUCCESSFUL )
    			 {
    				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
    				AppBleType.Respond.TxLen=1; //回包长
    				AppBleRespondData(BLE_WIFI_CONNECTION_TEST);
    				BleRxCmdHandlePro = E_CMD_DEFAULT;  
    			 }
    			 else if( tp1 == CAM_FAIL) 
    			 {
    				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败
    				AppBleType.Respond.TxLen=1; //回包长
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
    				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
    				AppBleType.Respond.TxLen=1; //回包长
    				AppBleRespondData(BLE_WIFI_CONNECTION_TEST);
    				BleRxCmdHandlePro = E_CMD_DEFAULT;  
    			}
    			#endif
            }
		}
		break; 
 
		case E_CMD_GET_WIFI_SIGNAL_INTENSITY:     //获取WIFI信号强度
		{
			#if (defined XM_CAM_FUNCTION_ON) && (defined XM_CAM_SCREEN_FUNCTION_PLUS_ON)  //猫眼透传
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
                    AppBleType.Respond.TxDataBuf[L_DATA + 1] = ~u8Data + 1;//去掉负号
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
				AppBleType.Respond.TxDataBuf[L_DATA] = 0;  //成功
				AppBleType.Respond.TxLen = 2;  
				AppBleType.Respond.TxDataBuf[L_DATA + 1] = ~intensity + 1;//去掉负号
				AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(temp == -1)
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 1;//失败  
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
				AppBleType.Respond.TxDataBuf[L_DATA] = 0;  //成功
				AppBleType.Respond.TxLen = 2;  
				AppBleType.Respond.TxDataBuf[L_DATA + 1] = WifiRxData.SignalIntensity;
				AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
				//ble ack
				BleRxCmdHandlePro = E_CMD_DEFAULT; 
			}
			else if(tp1 == WIFI_CONFIG_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA] = 1;//失败  
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
						AppBleType.Respond.TxDataBuf[L_DATA] = 1;//失败  
						AppBleType.Respond.TxLen = 2;  
						AppBleType.Respond.TxDataBuf[L_DATA + 1] = 0;
						AppBleRespondData(BLE_GET_WIFI_SIGNAL_INTENSITY);
						BleRxCmdHandlePro = E_CMD_DEFAULT;
					}
					else 
					{
						AppBleType.Respond.TxDataBuf[L_DATA] = 0;  //成功
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
		
		case E_CMD_DELETE_FINGER: 	//删除指纹
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
		case E_CMD_FACE_PRO_OTA:       //猫眼升级
		{
			tp1 = FaceProOta();
			if( tp1 == 1 )         //success
			{
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			break;
		}
		#endif
		
		#if defined XM_CAM_FUNCTION_ON //猫眼获取序列号 下发密钥
		case E_CMD_CAM_GET_SN:	  //获取猫眼序列号
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

		case E_CMD_CAM_POWER_SAVING:  //猫眼单双休切换
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_SINGLE_MODE_SET,&AppBleType.RxDataBuf[L_DATA],1);
			}
			else if(CAM_GetServerState(taskid)==CAM_SUCCESSFUL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData( BLE_CAM_BELL_ONE_WAY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(CAM_GetServerState(taskid)==CAM_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData( BLE_CAM_BELL_ONE_WAY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		
		break;
		case E_CMD_CAM_GET_IP:  //获取猫眼IP
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_GET_IP,0,0);
			}
			else if(CAM_GetServerState(taskid)==CAM_SUCCESSFUL)
			{
				AppBleType.Respond.TxLen=CAM_GetCameraData(CAM_CMD_GET_IP,&AppBleType.Respond.TxDataBuf[L_DATA], taskid); //从L_DATA开始拷贝
				AppBleType.Respond.TxDataBuf[L_DATA] =0;
				AppBleType.Respond.TxLen +=1;  
				AppBleRespondData(BLE_CAM_GET_IP);
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(CAM_GetServerState(taskid)==CAM_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData( BLE_CAM_GET_IP );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		break;
		case E_CMD_CAM_LINKKEY://下发阿里云密钥
			if(BleHandleProStep==0)
			{
				BleHandleProStep=1;
				taskid = CAM_SendCommandStart(CAM_CMD_LINKKEY_SEND,&AppBleType.RxDataBuf[L_DATA],96*2);
			}
			else if(CAM_GetServerState(taskid)==CAM_SUCCESSFUL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData( BLE_FACE_SET_LINKKEY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
			else if(CAM_GetServerState(taskid)==CAM_FAIL)
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //失败
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData( BLE_FACE_SET_LINKKEY );
				BleRxCmdHandlePro = E_CMD_DEFAULT;
			}
		break;
		#endif
		
		case E_CMD_WIFI_TEST:     //WIFI推送指定号码
		{
			//产测功能猫眼款不需要
			AppBleType.Respond.TxLen=1; //回包长		
			AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
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
		case E_CMD_WIFI_MAIN_SW_POW_OFF:     //关掉人脸电源
		{
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)
			{
				my_printf("E_CMD_WIFI_MAIN_SW_POW_OFF\n");
				//产测功能猫眼款不需要
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData(BLE_WIFI_MAIN_SW);
				BleRxCmdHandlePro = E_CMD_DEFAULT;  
			}
		}
		break;
		#endif
		
		case E_CMD_BLE_LOCK_CASE://获取事件记录
		{
			static uint16_t LogPlace=0;
			uint32_t addr;
			if(BleHandleProStep==0)
			{
				LogPlace=1;	//位置索引	
				if(SystemFixSeting.SysLockLogAll>MSG_LOG_RECORD_NUM) //已经存满了
				{
					BleHandleProStep=1;
				}
				else
				{
					BleHandleProStep=2;
				}
			}
			else if(BleHandleProStep==1)//已经存满了
			{
				if(LogPlace<=MSG_LOG_RECORD_NUM) //发送600条
				{
					if(SystemFixSeting.SysLockLogSn>=LogPlace) //前面的
					{			
						addr= MSG_LOG_RECORD_START + (SystemFixSeting.SysLockLogSn-LogPlace) * MSG_LOG_RECORD_ONE_SIZE; //计算存储地址
					}
					else //后面的
					{
						addr= MSG_LOG_RECORD_START + (MSG_LOG_RECORD_NUM-(LogPlace-SystemFixSeting.SysLockLogSn)) * MSG_LOG_RECORD_ONE_SIZE; //计算存储地址
					}
					//读出来
					HAL_EEPROM_ReadBytes(addr,&AppBleType.Respond.TxDataBuf[L_DATA+3],MSG_LOG_RECORD_REG_ONE_SIZE);			
					AppBleType.Respond.TxLen=17; //回包长
					AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功		
					AppBleType.Respond.TxDataBuf[L_DATA+1]=(MSG_LOG_RECORD_NUM )>>8; //总长度
					AppBleType.Respond.TxDataBuf[L_DATA+2]=(MSG_LOG_RECORD_NUM )&0xff; //总长度
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
			else if(BleHandleProStep==2)//还没存满
			{
				if(LogPlace<=SystemFixSeting.SysLockLogSn)
				{
					addr= MSG_LOG_RECORD_START + (SystemFixSeting.SysLockLogSn-LogPlace) * MSG_LOG_RECORD_ONE_SIZE; //计算存储地址
					//读出来
					HAL_EEPROM_ReadBytes(addr,&AppBleType.Respond.TxDataBuf[L_DATA+3],MSG_LOG_RECORD_REG_ONE_SIZE);				
					AppBleType.Respond.TxLen=17; //回包长
					AppBleType.Respond.TxDataBuf[L_DATA+1]= (SystemFixSeting.SysLockLogSn )>>8; //当前长度
					AppBleType.Respond.TxDataBuf[L_DATA+2]= (SystemFixSeting.SysLockLogSn )&0xff; //当前长度	
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
				AppBleType.Respond.TxDataBuf[L_DATA]=0; //成功
				AppBleType.Respond.TxLen=1; //回包长		
				AppBleRespondData(BLE_LOCK_CASE_LOG);
				App_GUI_UpdateMenuQuitTime(60*100, true); //跟新休眠时间
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
**函数名:       APP_BleServerProcess
**功能描述:     蓝牙数据解析
**输入参数:      
**输出参数:     
**备注:        
****************************************************************************************/
void APP_BleServerProcess(void )
{
	BLE_GetFifoData();  //蓝牙收包
	BleID2CasePro();    //ID2流程
	
	BleFingerCasePro(); //指纹数据流程
	BleFaceCasePro();   //人脸数据流程
	App_BleProHandleThread();  
	
}


 
//.end of the file.
