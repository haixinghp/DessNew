/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: APP_FACE.c 
* 作者：邓业豪
* 版本：V01
* 时间：20211101
* 内容简述：  人脸模组协议及应用
****************************************************************************/

/* 标准头文件 */
#include "stdint.h"
#include "string.h"

/* 内部头文件 */
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "..\HAL\HAL_Voice\HAL_Voice.h"
#include "LockConfig.h"
#include "Public.h"
#include "APP_FACE.h"
#include "APP_FACE_PRO.h"
#include "APP_BLE.h"
#include "App_LED.h" 
#include "System.h" 
#include "App_WIFI.h"
#include "App_GUI.h"  

#if defined OB_CAM_FUNCTION_ON ||  defined ST_CAM_FUNCTION_ON 
FaceProCloudData_T FaceProCloudData; //云端数据



/* 二代人脸功能

1. 下发阿里云密钥》完成断电
2. 配网》等待Note网络状态上报
		 联网成功后不断电，等待手机云端绑定配置
		 联网失败断电结束
3. 门铃呼叫》定时查询模组业务状态
			 空闲》正常休眠
			 错误》断电休眠》定时重连机制启动
4. 删除媒体文件》清空时执行
5. 消息透传》等待透传指令reply、等待模组联网note
6. 报警抓拍》等待抓拍指令reply、等待模组联网note
7. 固件升级》开始OTA，等待进度
8. 商汤模组可裁剪画面比例
9. 断网后重连机制
   网络状态错误MEDIA_STATE_ERROR
   下电FACE_CMD_POWERDOWN失败
10. WIFI断电不联网条件 电池低压/未配网/网络故障
*/


/***************************************************************************************
**函数名:       APP_FACE_PRO_Tim1s
**功能描述:     计时处理，用于应答包时间控制
**输入参数:     
**输出参数:    
**备注:         注意需要放在1秒定时器中断中
****************************************************************************************/
void APP_FACE_PRO_Tim1s (void)
{
	if(FaceWifiStatus.Timer)
	{
		FaceWifiStatus.Timer--;
	}
}





/***************************************************************************************
**函数名:       FaceProDeleteFile
**功能描述:     删除媒体问题
**输入参数:     
**输出参数:    
**备注:         奥比版本需跟猫眼通信
****************************************************************************************/
void FaceProDeleteFile (void)
{
	TimeoutSystick = 1000;//10s
	while(1)
	{
		FaceServerProcess();
		uint8_t File[2]={0,1};//媒体类型0 强制删除
		if(FaceGneralTaskFlow(FACE_CMD_DELETE_FILE,File,2,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)//清空
		{
			break;
		}
		else if(TimeoutSystick == 0)
		{
			FacePowerDown();
			break;
		}
	}
}

/***************************************************************************************
**函数名:       FaceProDeleteFile
**功能描述:     删除媒体问题
**输入参数:     
**输出参数:    
**备注:         需跟猫眼通信
****************************************************************************************/
void FaceProSetLightSensor (void)
{
	TimeoutSystick = 500;//5s
	while(1)
	{
		FaceServerProcess();
		uint8_t File[2]={0,1};//媒体类型0 强制删除
		if(FaceGneralTaskFlow(FACE_CMD_SET_LIGHTSENSOR,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)//清空
		{
			break;
		}
		else if(TimeoutSystick == 0)
		{
			FacePowerDown();
			break;
		}
	}
}
/***************************************************************************************
**函数名:       FaceProSetSSID
**功能描述:     配网
**输入参数:     ssid 
**输出参数:    
**备注:         需等待30秒模组联网以及手机云端绑定
****************************************************************************************/
int8_t FaceProSetSSID (uint8_t *ssid ,uint8_t len)
{
	switch (AppFaceWorkPro.SetWifi)
    {
    	case 0: //配网指令
			if(FaceGneralTaskFlow(FACE_CMD_SET_SSID,ssid,len,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)
			{
				AppFaceWorkPro.SetWifi++;
				FaceWifiStatus.Timer=30;//30秒联网
			}
			break;
		case 1:	//等待联网note
			if(FaceWifiStatus.Timer==0) //超时
			{
				AppFaceWorkPro.SetWifi++;
			}
			if(FaceWifiStatus.media_state==MEDIA_STATE_WAITING) //联网成功，不断电
			{
				return 0;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
			{
				AppFaceWorkPro.SetWifi++;
			}
			break;
		case 2: //失败断电
			if(FaceGneralTaskFlow(FACE_CMD_SET_SSID,ssid,len,FACE_DEFAULT_TIMEOUT_3S)==TASK_POWERDOWN)
			{
				return 1;
			}
    	default:
    		break;
	}	
	return -1;	
}


/***************************************************************************************
**函数名:       FaceProCallBell
**功能描述:     Bell
**输入参数:     
**输出参数:    
**备注:         主动呼叫需发指令
				中断唤醒接听上电
				需反复查询模组业务状态待手机挂断空闲退出
****************************************************************************************/
uint8_t FaceProCallBell(uint8_t Bell)
{
	switch (AppFaceWorkPro.CallBell)
    {
		case 0:
			FaceWifiStatus.Timer=10;
			AppFaceWorkPro.CallBell++;
    	case 1:
			if(Bell) //主动按门铃
			{
				uint8_t BellData[8]={0,0,0,8,1,0,0,0};//抓拍8秒使能云存
				if(FaceGneralTaskFlow(FACE_CMD_BELL,BellData,8,FACE_VERIFY_TIMEOUT_10S)==TASK_SUCCESS) //主动呼叫
				{
					AppFaceWorkPro.CallBell++;
					FaceWifiStatus.Timer=5;
				}
			}
			else if(FaceGneralTaskFlow(FACE_CMD_MODULE_STATUS,0,0,FACE_VERIFY_TIMEOUT_10S)==TASK_SUCCESS)//中断唤醒
			{
				FaceClearTask();//清任务，下次执行
				AppFaceWorkPro.CallBell++;
				FaceWifiStatus.Timer=5;
			}
			if(FaceWifiStatus.Timer == 0)
			{
				my_printf("FaceProCallBell timeout\n");
				return 1;//休眠
			}
    		break;
    	case 2: //检查模组云业务状态
			{
				if(FaceWifiStatus.Timer) //10秒查询一次
				{
					break;
				}FACE_GNERAL_TASK_E tp1 = (FACE_GNERAL_TASK_E)FaceGneralTaskFlow(FACE_CMD_MODULE_STATUS,0,0,FACE_VERIFY_TIMEOUT_10S);
				if(tp1 == TASK_SUCCESS) //主动呼叫
				{
					FaceWifiStatus.media_state=FaceMsgType.Reply.DataPack.Data[1];//云业务状态
					FaceWifiStatus.media_error=FaceMsgType.Reply.DataPack.Data[2];//错误类型
					if(FaceWifiStatus.media_state==MEDIA_STATE_IDLE)//空闲退出
					{
						AppFaceWorkPro.CallBell++;
						FaceWifiStatus.Timer=10;
					}
					else if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
					{				
						AppFaceWorkPro.CallBell++;
						FaceWifiStatus.Timer=10;
					}
					else //工作中，继续等待查询
					{
						FaceWifiStatus.Timer=10;
						FaceClearTask();//清任务，下次执行
					}
				}
				else if(tp1 == TASK_FAIL)
				{
					my_printf("FaceProCallBell timeout\n");
					return 1;//休眠
				}
			}
    		break;
		case 3: 
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) //断电
			{
				return 1;//休眠
			}
			if(FaceWifiStatus.Timer == 0) //10秒未回复断电
			{
				return 1;//休眠
			}
    	default:
    		break;
    }
	return 0;
}

/***************************************************************************************
**函数名:       FaceProAlarm
**功能描述:     报警抓拍
**输入参数:     
**输出参数:    
**备注:         部分指令仅需透传
				部分指令透传+抓拍
****************************************************************************************/
uint8_t FaceProAlarm(uint8_t type ,uint8_t *data ,uint8_t len)
{
	switch (AppFaceWorkPro.Alarm)
    {
		case 0:
			FaceWifiStatus.Timer=10;
			AppFaceWorkPro.Alarm++;
		case 1://透传
			if(FaceGneralTaskFlow(FACE_CMD_PASS_DATA,data,len,FACE_VERIFY_TIMEOUT_10S)==TASK_SUCCESS) 
			{
				AppFaceWorkPro.Alarm++;
				FaceWifiStatus.Timer=10;
			}
			else if(FaceWifiStatus.Timer == 0)
			{
				return 1;
			}
			break;
		case 2://等待联网
			if(FaceWifiStatus.Timer==0) //超时
			{
				AppFaceWorkPro.Alarm=5;
			}
			if(FaceWifiStatus.media_state==MEDIA_STATE_WAITING) //联网成功
			{
				AppFaceWorkPro.Alarm++;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
			{
				AppFaceWorkPro.Alarm=5;
			}
			break;
		case 3://抓拍
			if((type==1) ||(type==3)||(type==4)||(type==5))
			{
				uint8_t CapData[8]={0,0,0,0,15,1,type,0};//抓拍视频15秒使能云存
				if(FaceGneralTaskFlow(FACE_CMD_MID_CAPTURE,CapData,8,FACE_VERIFY_TIMEOUT_10S)==TASK_SUCCESS) 
				{
					AppFaceWorkPro.Alarm++;
					FaceWifiStatus.Timer=40; 
				}				
			}
			else
			{
				AppFaceWorkPro.Alarm=5; //下电
			}
			break;
		case 4://等待空闲
			if(FaceWifiStatus.Timer==0) //超时
			{
				AppFaceWorkPro.Alarm=5;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_IDLE) //任务空闲
			{
				AppFaceWorkPro.Alarm=5;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
			{
				AppFaceWorkPro.Alarm=5;
			}
			break;
		case 5:	//下电
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				return 1;
			}
			break;
    	default:
    		break;
    }
	return 0;
}


/***************************************************************************************
**函数名:       FaceProNetworking
**功能描述:     模组联网
**输入参数:     
**输出参数:    联网成功 ：1； 联网失败 ： 2；
**备注:         等待联网30秒
****************************************************************************************/
uint8_t FaceProNetworking(void)
{
	switch (AppFaceWorkPro.Networking)
    {
		case 0:
			FaceWifiStatus.Timer=10;
			AppFaceWorkPro.Networking++;
			break;
		case 1://等待上电
			if(FaceGneralTaskFlow(FACE_CMD_GET_WIFISTATUS,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS) 
			{
				AppFaceWorkPro.Networking++;
				FaceWifiStatus.Timer=30;
			}
			else if(FaceWifiStatus.Timer == 0)
			{
				return 2;
			}
			break;
		case 2://接收note
			if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
			{
				AppFaceWorkPro.Networking++;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_WAITING)//联网成功
			{
				AppFaceWorkPro.Networking = 4;
			}
			else if(FaceWifiStatus.Timer==0)
			{
				AppFaceWorkPro.Networking++;
			}
			break;
		case 3:	
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				return 2;	
			}
			break;
		case 4:	
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				return 1;	
			}
			break;
    	default:
    		break;
    }
	return 0;	
	
}

/***************************************************************************************
**函数名:       FaceProGetWifiIntensity
**功能描述:     获取wifi信号强度
**输入参数:     
**输出参数:    获取wifi信号强度成功 ：1； 失败 ： -1；
**备注:         等待联网30秒
****************************************************************************************/
int8_t FaceProGetWifiIntensity(uint8_t *intensity)
{
	static uint8_t IntensityTemp = 0;
	switch (AppFaceWorkPro.WifiIntensity)
    {
		case 0:
			FaceWifiStatus.Timer=10;
			AppFaceWorkPro.WifiIntensity++;
			break;
		case 1://等待上电
			if(FaceGneralTaskFlow(FACE_CMD_GET_WIFISTATUS,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS) 
			{
				AppFaceWorkPro.WifiIntensity++;
				FaceWifiStatus.Timer=30;
			}
			else if(FaceWifiStatus.Timer == 0)
			{
				return -1;
			}
			break;
		case 2:
			if(FaceGneralTaskFlow(FACE_CMD_DEVICENAME,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS)  
			{
				AppFaceWorkPro.WifiIntensity++;
				FaceWifiStatus.Timer=30;
			}
			else if(FaceWifiStatus.Timer == 0)
			{
				return -1;
			}
			break;
		case 3://接收note
			if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
			{
				AppFaceWorkPro.WifiIntensity++;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_WAITING)//联网成功
			{
				if(FaceGneralTaskFlow(FACE_CMD_GET_WIFISTATUS,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS) 
				{
					*intensity = FaceMsgType.Reply.DataPack.Data[1];
					AppFaceWorkPro.WifiIntensity = 5;
					my_printf("wifi signal intensity = %x\n", intensity);
				}
			}
			if(FaceWifiStatus.Timer==0)
			{
				AppFaceWorkPro.WifiIntensity++;
			}
			break;
		case 4:	
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				return -1;	
			}
			break;
		case 5:	
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				return 1;	
			}
			break;
    	default:
    		break;
    }
	return 0;
}
/***************************************************************************************
**函数名:       FaceProOta
**功能描述:     在线升级
**输入参数:     
**输出参数:    
**备注:         发送OTA START指令，上报升级进度至手机
****************************************************************************************/
int8_t FaceProOta(void)
{
	switch (AppFaceWorkPro.Upgrade)
    {
		case 0://开始升级
			if(FaceGneralTaskFlow(FACE_CMD_START_OTA,0,0,FACE_DEFAULT_TIMEOUT_3S)==TASK_SUCCESS) 
			{
				AppFaceWorkPro.Upgrade++;
				FaceWifiStatus.Timer=60;
			}
			break;
		case 1:
			if(FaceWifiStatus.ota_state==1)//上报进度，刷新时间
			{
				App_GUI_UpdateMenuQuitTime(60*100, true); //跟新休眠时间
				FaceWifiStatus.Timer=60;
				AppBleType.Respond.TxDataBuf[L_DATA]=2; //工作中
				AppBleType.Respond.TxDataBuf[L_DATA+1]=FaceWifiStatus.ota_state;
				AppBleType.Respond.TxDataBuf[L_DATA+2]=FaceWifiStatus.ota_error;
				AppBleType.Respond.TxDataBuf[L_DATA+3]=FaceWifiStatus.ota_progress;
				AppBleType.Respond.TxLen=4; //回包长
				AppBleRespondData(BLE_FACE_PRO_OTA);
				FaceWifiStatus.ota_state=0; //清除
			}
			else if(FaceWifiStatus.ota_state==3) //猫眼报错退出
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=1; //出错
				AppBleType.Respond.TxDataBuf[L_DATA+1]=FaceWifiStatus.ota_state;
				AppBleType.Respond.TxDataBuf[L_DATA+2]=FaceWifiStatus.ota_error;
				AppBleType.Respond.TxDataBuf[L_DATA+3]=FaceWifiStatus.ota_progress;
				AppBleType.Respond.TxLen=4; //回包长
				AppBleRespondData(BLE_FACE_PRO_OTA);
				AppFaceWorkPro.Upgrade++;
			}
			
			if(FaceWifiStatus.ota_progress==100)//完成
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=3; 
				AppBleType.Respond.TxLen=1; //回包长
				AppBleRespondData(BLE_FACE_PRO_OTA);
				AppFaceWorkPro.Upgrade++;
			}
		
		    if(FaceWifiStatus.Timer==0) //超时
			{
				AppBleType.Respond.TxDataBuf[L_DATA]=4;
				AppBleType.Respond.TxDataBuf[L_DATA+1]=0;
				AppBleType.Respond.TxDataBuf[L_DATA+2]=0;
				AppBleType.Respond.TxDataBuf[L_DATA+3]=0;
				AppBleType.Respond.TxLen=4; //回包长
				AppBleRespondData(BLE_FACE_PRO_OTA);	
				AppFaceWorkPro.Upgrade++;
			}
			break;
		case 2:	 //下电
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				#if defined OB_CAM_FUNCTION_ON || defined ST_CAM_FUNCTION_ON //二代人脸格式
				AppFaceWorkPro.Upgrade = 3;
				FaceWifiStatus.Timer=1;
				#else
				AppBleType.RxCdm=0; //直接结束
				#endif
			}
			break;
		case 3:	//等待联网	
			if(FaceProNetworking() == 1)
			{
				return 1;
			}
    	default:
    		break;
	}		
	return 0;
}

/***************************************************************************************
**函数名:       FaceProQueryCloudData
**功能描述:     主动获取云端数据（0x0A）
**输入参数:     
**输出参数:     发送并响应完成：1；响应失败 ： 2；
**备注:         
				
****************************************************************************************/
uint8_t FaceProQueryCloudData(uint8_t *data ,uint8_t len)
{
	uint8_t temp = 0;
	switch (AppFaceWorkPro.QueryCloudData)
    {
		case 0:
			FaceWifiStatus.Timer=10;
			AppFaceWorkPro.QueryCloudData++;
			break;
		case 1://透传
			if(FaceGneralTaskFlow(FACE_CMD_PASS_DATA,data,len,FACE_VERIFY_TIMEOUT_10S)==TASK_SUCCESS) 
			{
//				FaceMsgType.Reply.DataPack.Data[6] = 05;//天气
//				FaceMsgType.Reply.DataPack.Data[7] = 11;//节日
				FaceProCloudData.Weather = FaceMsgType.Reply.DataPack.Data[6];
				FaceProCloudData.Festival = FaceMsgType.Reply.DataPack.Data[7];
				FaceProCloudData.Minimun_Temperature = FaceMsgType.Reply.DataPack.Data[8];
				FaceProCloudData.Maximun_Temperature = FaceMsgType.Reply.DataPack.Data[9];
				memcpy(&FaceProCloudData.Date,&FaceMsgType.Reply.DataPack.Data[9],6);
				AppFaceWorkPro.QueryCloudData++;
				FaceWifiStatus.Timer=10;
			}
			else if(0 == FaceWifiStatus.Timer)
			{
				if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
				{
					return 2;	
				}
			}
			break;
		case 2://等待空闲
			temp = FaceProNetworking();
			if(temp == 1)
			{
				return 1;
			}
			else if(temp == 2)
			{
				return 2;
			}
			break;
    	default:
    		break;
    }
	return 0;
}




/***************************************************************************************
**函数名:       FaceProScanProcess
**功能描述:     开锁信息扫描函数
**输入参数:     
**输出参数:    1:wifi推送结束
**备注:         
****************************************************************************************/
int8_t FaceProScanProcess(void)
{
	if(OB_CAM_TxState == WIFI_TX_OVER)
		return 1;
	switch (AppFaceWorkPro.ScanState)
    {
		case 0:
			FaceWifiStatus.Timer=10;
			AppFaceWorkPro.ScanState++;
			break;
		case 1://透传
			if(FaceGneralTaskFlow(FACE_CMD_PASS_DATA, WifiTxTemp.data, WifiTxTemp.length, FACE_PASS_DATA_TIMEOUT_5S)==TASK_SUCCESS) 
			{
				AppFaceWorkPro.ScanState++;
				FaceWifiStatus.Timer=10;
			}
			else if(FaceWifiStatus.Timer == 0)
			{
				OB_CAM_TxState = WIFI_TX_OVER;
			}
			break;
		case 2://接收note
			if(FaceWifiStatus.media_state==MEDIA_STATE_ERROR) //网络故障
			{
				AppFaceWorkPro.ScanState++;
			}
			else if(FaceWifiStatus.media_state==MEDIA_STATE_WAITING)//联网成功
			{
				AppFaceWorkPro.ScanState = 3;
			}
			else if(FaceWifiStatus.Timer==0)
			{
				AppFaceWorkPro.ScanState++;
			}
			break;
		case 3:	//下电
			if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN) 
			{
				memset(WifiTxTemp.data, 0, 128);
				OB_CAM_TxState = WIFI_TX_OVER;
				return 1;
			}
			break;
    	default:
    		break;
    }
	return 0;
}

///***************************************************************************************
//**函数名:       FaceProWifiPush
//**功能描述:     OB wifi推送函数
//**输入参数:     
//**输出参数:    
//**备注:         该函数阻塞
//****************************************************************************************/
//void FaceProWifiPush(void)
//{
//	uint8_t tp1;
//	TimeoutSystick = 1000;//10s
//	while(1)
//	{
//		tp1 = FaceServerProcess();
//		if(1 == tp1)
//		{
//			break;
//		}
//		else if(TimeoutSystick == 0)
//		{
//			FacePowerDown();
//			break;
//		}
//	}
//}
#endif


//.end of the file.
