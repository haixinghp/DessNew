/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: APP_FACE.c 
* 作者：邓业豪
* 版本：V01
* 时间：20210818
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
#include "App_LED.h" 
#include "System.h" 
#include "..\Server\Face.h"
#if defined FACE_FUNCTION_ON  || defined IRIS_FUNCTION_ON

/*-------------------------------------------------结构体-----------------------------------------------------------*/
FACE_ATTRIBUTE FaceAttribute; //人脸存储
uint8_t FaceTimer = 0;
/***************************************************************************************
**函数名:       APP_FACE_Tim1s
**功能描述:     计时处理，用于应答包时间控制
**输入参数:     
**输出参数:    
**备注:         注意需要放在1秒定时器中断中
****************************************************************************************/
void APP_FACE_Tim1s (void)
{
	if(FaceTimer)
	{
		FaceTimer--;
	}
}

/***************************************************************************************
**函数名:       FacePageidAddr
**功能描述:     根据Pageid找存储地址
**输入参数:     地址块
**输出参数:     实际地址
**备注:      
****************************************************************************************/
static uint32_t FacePageidAddr(uint8_t Pageid)
{
	return   MSG_FACE_REG_START + Pageid * MSG_FACE_ONE_SIZE;
}

/***************************************************************************************
**函数名:       FaceEepromVerify
**功能描述:     轮询存储搜索ID
**输入参数:     模组返回人脸ID
**输出参数:     成功1  失败0
**备注:         结构体直接读取
****************************************************************************************/
static uint16_t FaceEepromVerify (uint16_t UserId)
{
    for(uint16_t pageid = 0; pageid < MSG_FACE_USER_NUM; pageid++)
    {
		HAL_EEPROM_ReadBytes(FacePageidAddr(pageid),(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));	//直接读结构体
		if((FaceAttribute.FaceVaild==MEM_FACT_MEM_FIG)&& (FaceAttribute.FaceId == UserId))
		{				
			return 1;					
		}
    }
    return 0;    //fail
}

/***************************************************************************************
**函数名:       FaceEepromSave
**功能描述:     保存人脸信息ID
**输入参数:     
**输出参数:     
**备注:         结构体直接写入
****************************************************************************************/
static void FaceEepromSave(void)
{
	//找空地址
	uint16_t pageid;
	uint32_t addr;
	uint8_t SaveFlag=0;;
	for( pageid = 0; pageid < MSG_FACE_USER_NUM; pageid++ ) //轮询
	{
		addr =FacePageidAddr(pageid);
		HAL_EEPROM_ReadBytes(addr,&SaveFlag,1);	//read Reg MSG fig
		if( SaveFlag != MEM_FACT_MEM_FIG )//指纹存在标志.  
		{
			break;				// empty pageid	
		}
	}
	//直接存储结构体
	
	FaceAttribute.FaceVaild=MEM_FACT_MEM_FIG;//写标记
	FaceAttribute.FacePageId=pageid;//序号
	HAL_EEPROM_WriteBytes(addr,(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));
	
	//数量增加
	SystemSeting.SysFaceAllNum++;
	SystemWriteSeting(&SystemSeting.SysFaceAllNum,1);
	if(FaceAttribute.FaceLim ==MEM_USER_MASTER)
	{
		SystemSeting.SysFaceAdminNum++;
		SystemWriteSeting(&SystemSeting.SysFaceAdminNum,1); //管理员
	}
	else
	{
		SystemSeting.SysFaceGuestNum++;
		SystemWriteSeting(&SystemSeting.SysFaceGuestNum,1);
	}
}

/***************************************************************************************
**函数名:       FaceDelete
**功能描述:     删除指定PageId位置人脸
**输入参数:     
**输出参数:     返回1完成 返回0等待
**备注:         循环读取状态
****************************************************************************************/
uint8_t FaceDeleteAppUser(uint8_t PageId)
{
	if(AppFaceWorkPro.DeleteUser==0)
	{
		AppFaceWorkPro.DeleteUser=1;
		uint32_t addr;
		addr=FacePageidAddr(PageId);//计算地址
		HAL_EEPROM_ReadBytes(addr,(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));	//直接读结构体		
		if(FaceAttribute.FaceVaild!=MEM_FACT_MEM_FIG)
		{
			AppFaceWorkPro.DeleteUser=0;
			return 1; //无存储，一般不会有这种情况
		}
		else
		{
			FaceAttribute.FaceVaild=0;
			HAL_EEPROM_WriteBytes(addr,(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));	//直接写结构体	
			my_printf("Delete face id = %x  ,FacePageId = %x\n",FaceAttribute.FaceId,FaceAttribute.FacePageId);
			
			//数量减少
			SystemSeting.SysFaceAllNum--;
			SystemWriteSeting(&SystemSeting.SysFaceAllNum,1);
			if(FaceAttribute.FaceLim ==MEM_USER_MASTER)
			{
				SystemSeting.SysFaceAdminNum--;
				SystemWriteSeting(&SystemSeting.SysFaceAdminNum,1); //管理员
			}
			else
			{
				my_printf("FaceDeleteAppUser SystemSeting.SysFaceGuestNum = %d\n", SystemSeting.SysFaceGuestNum);
				SystemSeting.SysFaceGuestNum--;
				SystemWriteSeting(&SystemSeting.SysFaceGuestNum,1);
			}
		}
	}
	//删除人脸ID
	if(FaceGneralTaskFlow(FACE_CMD_DEL_USER,(uint8_t *)&FaceAttribute.FaceId,2,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)
	{
		AppFaceWorkPro.DeleteUser=0;
		return 1;
	}
	return 0;
}

/***************************************************************************************
**函数名:       FaceWrite
**功能描述:     人脸业务配置，写入相应人脸配置区域
**输入参数:     
**输出参数:     返回1完成 返回0等待
**备注:         循环读取状态
****************************************************************************************/
void FaceWrite(FACE_ATTRIBUTE FACE_MEG, uint16_t PageId)
{
	uint32_t addr = 0;
	addr=FacePageidAddr(PageId);//计算地址
	HAL_EEPROM_WriteBytes(addr,(uint8_t *)&FACE_MEG,sizeof(FACE_MEG));	//直接写结构体	
    return;
}

/***************************************************************************************
**函数名:       FaceRead
**功能描述:     人脸业务配置，写入相应人脸配置区域
**输入参数:     
**输出参数:     返回1完成 返回0等待
**备注:         循环读取状态
****************************************************************************************/
void FaceRead(FACE_ATTRIBUTE FACE_MEG, uint16_t PageId)
{
	uint32_t addr = 0;;
	addr=FacePageidAddr(PageId);//计算地址	
	HAL_EEPROM_ReadBytes( addr, (uint8_t *)&FACE_MEG, sizeof(FACE_MEG) ) ;
    return;
}
/***************************************************************************************
**函数名:       FaceDelete
**功能描述:     删除所有普通用户人脸
**输入参数:     
**输出参数:     
**备注:         阻塞进行
****************************************************************************************/
void FaceDeleteGuestUser(void)
{
    uint16_t pageid = 0;
    uint32_t addr = FacePageidAddr(pageid);
    for(pageid = 0; pageid < MSG_FACE_USER_NUM; pageid++)
    {
		addr = FacePageidAddr(pageid);
		HAL_EEPROM_ReadBytes(addr,(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));	//直接读结构体
		if((FaceAttribute.FaceLim==MEM_USER_GUEST)&&(FaceAttribute.FaceVaild==MEM_FACT_MEM_FIG))//判断普通用户
		{
			while(1)
			{
				FaceServerProcess();
				uint8_t result=FaceGneralTaskFlow(FACE_CMD_DEL_USER,(uint8_t *)&FaceAttribute.FaceId,2,FACE_DEFAULT_TIMEOUT_1S);//开始删除人脸
				if((result==TASK_SUCCESS)||(result==TASK_FAIL))//直到完成
				{
					FaceClearTask();//清除本次任务
					break;
				}
			}
			memset((uint8_t *)&FaceAttribute,0,sizeof(FaceAttribute));
			HAL_EEPROM_WriteBytes(addr,(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));	//直接写结构体
		}
	}
	while(1)
	{
		SystemSeting.SysFaceGuestNum = 0;
		SystemWriteSeting(&SystemSeting.SysFaceGuestNum,1);
		FaceServerProcess();
		if(FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)//清空
		{
			break;
		}
	}
	
	
}



/***************************************************************************************
**函数名:       FaceGetVeifyState
**功能描述:     验证人脸流程
**输入参数:     TimeEn  时效使能 开门时需要，进菜单不需要
**输出参数:     当前状态
**备注:         
****************************************************************************************/
uint8_t FaceGetVeifyState(uint8_t AdminEn,uint16_t *Pageid, uint8_t *unlockStatus)
{
	#if defined FACE_FUNCTION_ON
	static uint8_t result=0;
	switch (AppFaceWorkPro.Verify)
    {
    	case FACE_VERIFY_CHECK_DEMO:
			result=0; //清除结果
			#if defined OB_CAM_FUNCTION_ON// || defined OB_CAM_FUNCTION_ON //二代人脸格式
				AppFaceWorkPro.Verify++;				
			#else
			if(SystemSeting.SysFaceAllNum==0) //无人脸，发体验模式指令
			{
				if(	FaceGneralTaskFlow(FACE_CMD_DEMO,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_SUCCESS)//DEMO模式指令
				{
					AppFaceWorkPro.Verify++;		   
				}
			}
			else
			{
				AppFaceWorkPro.Verify++;				
			}
			#endif
    		break;
    	case FACE_VERIFY_CHECK: 
			FaceGneralTaskFlow(FACE_CMD_VERIFY,0,0,FACE_VERIFY_TIMEOUT_10S);//验证指令
			if(AppFaceWorkPro.TaskFlow == TASK_WORKING)//执行中
			{
				if(FaceMsgType.NidFaceState.NoFace>90)
				{
					FaceMsgType.NidFaceState.NoFace=0;
					HAL_Voice_PlayingVoice(EM_CHECK_FACE_NONE_MP3,150); //未检测到人脸
					AppFaceWorkPro.Verify=FACE_VERIFY_POWERDOWN;	
					result= FACE_VERIFY_NOFACE;
				}
				else if(FaceMsgType.NidFaceState.FaceFar>30)
				{
					FaceMsgType.NidFaceState.FaceFar=0;
					HAL_Voice_PlayingVoice(EM_FACE_TOO_FAR_MP3,150); //人脸距离太远  
					return FACE_VERIFY_FAR;
				}
				else if(FaceMsgType.NidFaceState.FaceNear>30)
				{
					FaceMsgType.NidFaceState.FaceNear=0;
					HAL_Voice_PlayingVoice(EM_FACE_TOO_NEARLY_MP3,150); //人脸距离太近 
					return FACE_VERIFY_NEAR;			
				}
				else if(FaceMsgType.NidFaceState.FaceOcc>30)
				{
					FaceMsgType.NidFaceState.FaceOcc=0;
					HAL_Voice_PlayingVoice(EM_FACE_SHELTED_MP3,150); //脸部有遮挡  	
					return FACE_VERIFY_OCCLUSION;
				}
			}
			else if(AppFaceWorkPro.TaskFlow == TASK_SUCCESS)//验证指令执行完成
			{
				if(FACE_CMD_VERIFY == FaceMsgType.Reply.MsgMid) //接收成功
				{
					if( MR_SUCCESS == FaceMsgType.Reply.Msgresult) //验证成功
					{
						FaceAttribute.FaceId=FaceMsgType.Reply.DataPack.Verify.UserId; //拷贝ID
						my_printf("FaceMsgType.Reply.DataPack.Verify.unlockStatus = %x\n", FaceMsgType.Reply.DataPack.Verify.unlockStatus);
						*unlockStatus = FaceMsgType.Reply.DataPack.Verify.unlockStatus;//解锁过程中睁闭眼状态
						if(FaceEepromVerify(FaceAttribute.FaceId))//核对ID成功
						{		
							if(((AdminEn)&&(FaceAttribute.FaceLim==MEM_USER_MASTER))  || (AdminEn==0) ) //是否验证管理员
							{
								*Pageid=FaceAttribute.FacePageId; //这个ID不需要转换大小端
								if(FaceAttribute.tm_vaild.fig) //属性时效使能
								{
									if(RTC_Successfully==HAL_RTC_TimeIsTimesize(&FaceAttribute.tm_vaild.start,
																				&FaceAttribute.tm_vaild.stop,
																				FaceAttribute.tm_vaild.wday))
									{
										result= FACE_VERIFY_SUCCESS;//时效验证成功
									}
									else
									{
										result= FACE_VERIFY_TIME_FAIL; //时效验证失败
									}
								}
								else
								{
									result= FACE_VERIFY_SUCCESS;//无时效验证成功
								}
							}
							else
							{
								result=FACE_VERIFY_ADMIN_FAIL; //非管理员
							}
						}
						else
						{
							result=FACE_VERIFY_EE_FAIL; //EE比对失败
						}
					}
					else //验证失败
					{
						result= FACE_VERIFY_MODULE_FAIL; //模组验证失败
					}
				}
			}
			else if(AppFaceWorkPro.TaskFlow == TASK_POWERDOWN) //验证结束后自动下电
			{
				return result;
			}
			break;
		case FACE_VERIFY_POWERDOWN: //下电后再返回结果
			if(TASK_POWERDOWN == FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S))//提前下电
			{
				return result;
			}
			break;
    	default:
    		break;
    }
	if(AppFaceWorkPro.TaskFlow == TASK_FAIL)
	{
		return FACE_VERIFY_MODULE_ERR;
	}
	else
	{
		return FACE_VERIFY_IDLE;
	}

	#elif defined IRIS_FUNCTION_ON
	static uint8_t result=0;
	switch (AppFaceWorkPro.Verify)
    {
    	case FACE_VERIFY_CHECK_DEMO:
			result=0; //清除结果
			AppFaceWorkPro.Verify++;
    		break;
    	case FACE_VERIFY_CHECK: 
			FaceGneralTaskFlow(IRIS_CMD_VERIFY,0,0,14000);//验证指令
			if(AppFaceWorkPro.TaskFlow == TASK_WORKING)//执行中
			{
				if(FaceMsgType.NidFaceState.NoFace>60)
				{
					FaceMsgType.NidFaceState.NoFace=0;
					HAL_Voice_PlayingVoice(EM_CHECK_FACE_NONE_MP3,150); //未检测到人脸
					AppFaceWorkPro.Verify=FACE_VERIFY_POWERDOWN;	
					result= FACE_VERIFY_NOFACE;
				}
				else if(FaceMsgType.NidFaceState.FaceFar>60)
				{
					FaceMsgType.NidFaceState.FaceFar=0;
					HAL_Voice_PlayingVoice(EM_FACE_TOO_FAR_MP3,150); //人脸距离太远  
					return FACE_VERIFY_FAR;
				}
				else if(FaceMsgType.NidFaceState.FaceNear>60)
				{
					FaceMsgType.NidFaceState.FaceNear=0;
					HAL_Voice_PlayingVoice(EM_FACE_TOO_NEARLY_MP3,150); //人脸距离太近 
					return FACE_VERIFY_NEAR;			
				}
				else if(FaceMsgType.NidFaceState.FaceOcc>60)
				{
					FaceMsgType.NidFaceState.FaceOcc=0;
					HAL_Voice_PlayingVoice(EM_FACE_SHELTED_MP3,150); //脸部有遮挡  	
					return FACE_VERIFY_OCCLUSION;
				}
			}
			else if(AppFaceWorkPro.TaskFlow == TASK_SUCCESS)//验证指令执行完成
			{
				my_printf("FaceMsgType.Reply.MsgMid = %x\n", FaceMsgType.Reply.MsgMid);
				if(IRIS_CMD_VERIFY == FaceMsgType.Reply.MsgMid) //接收成功
				{
					if( MR_SUCCESS == FaceMsgType.Reply.Msgresult) //验证成功
					{
						FaceAttribute.FaceId=FaceMsgType.Reply.DataPack.Verify.UserId; //拷贝ID
						if(FaceEepromVerify(FaceAttribute.FaceId))//核对ID成功
						{		
							if(((AdminEn)&&(FaceAttribute.FaceLim==MEM_USER_MASTER))  || (AdminEn==0) ) //是否验证管理员
							{
								*Pageid=FaceAttribute.FacePageId; //这个ID不需要转换大小端
								my_printf("Register face id = %x  ,FacePageId = %x\n",FaceAttribute.FaceId,FaceAttribute.FacePageId);
								if(FaceAttribute.tm_vaild.fig) //属性时效使能
								{
									if(RTC_Successfully==HAL_RTC_TimeIsTimesize(&FaceAttribute.tm_vaild.start,
																				&FaceAttribute.tm_vaild.stop,
																				FaceAttribute.tm_vaild.wday))
									{
										result= FACE_VERIFY_SUCCESS;//时效验证成功
									}
									else
									{
										result= FACE_VERIFY_TIME_FAIL; //时效验证失败
									}
								}
								result= FACE_VERIFY_SUCCESS;//无时效验证成功
							}
							else
							{
								result=FACE_VERIFY_ADMIN_FAIL; //非管理员
							}
						}
						else
						{
							result=FACE_VERIFY_EE_FAIL; //EE比对失败
						}
					}
					else //验证失败
					{
						my_printf("FACE_VERIFY_MODULE_FAIL\n");
						result= FACE_VERIFY_MODULE_FAIL; //模组验证失败
					}
				}
			}
			else if(AppFaceWorkPro.TaskFlow == TASK_POWERDOWN) //验证结束后自动下电
			{
				return result;
			}
			break;
		case FACE_VERIFY_POWERDOWN: //下电后再返回结果
			if(TASK_POWERDOWN == FaceGneralTaskFlow(FACE_CMD_POWERDOWN,0,0,FACE_DEFAULT_TIMEOUT_1S))//提前下电
			{
				return result;
			}
			break;
    	default:
    		break;
    }
	if(AppFaceWorkPro.TaskFlow == TASK_FAIL)
	{
		return FACE_VERIFY_MODULE_ERR;
	}
	else
	{
		return FACE_VERIFY_IDLE;
	}
	#endif
}













/***************************************************************************************
**函数名:       FaceEnrollPro
**功能描述:     注册人脸流程
**输入参数:     管理员或普通用户
**输出参数:     当前状态
**备注:         #define MEM_USER_MASTER                          'M'
				#define MEM_USER_GUEST                           'G'
****************************************************************************************/
uint8_t FaceEnrollPro(uint8_t FaceLim)
{
	#if defined FACE_FUNCTION_ON
	typedef struct
	{
		uint8_t admin;                    //是否设置为管理员yes:1 no:0
		uint8_t userName[32]; //录入用户姓名
		uint8_t faceDirection;      //用户需要录入的方向
		uint8_t timeOut;                  //录入超时时间 单位:s
	}FACE_ENROLL_DATA;     //注册发送结构体   
	
	static FACE_ENROLL_DATA FaceEnrollData;
	FaceEnrollData.timeOut=20; //超时固定输入20秒
	switch(AppFaceWorkPro.Register)
	{
	case FACE_ADD_FRONT:
		memset(&FaceEnrollData,0,sizeof(FaceEnrollData));
		HAL_Voice_PlayingVoice(EM_CAPTURE_FACE_FRONT_MP3,100);//请录入正脸  
		FaceEnrollData.faceDirection=0x01;//正脸  
		AppFaceWorkPro.Register++;
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_5, EM_LED_ON ); //亮灯
		FaceTimer = 20;
		break;
	case FACE_ADD_UP:
		HAL_Voice_PlayingVoice(EM_MOVE_HEAD_UP_MP3,150);//请微微抬头  
		FaceEnrollData.faceDirection=0x10;//抬头
		AppFaceWorkPro.Register++;
		App_LED_OutputCtrl( EM_LED_2, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_5, EM_LED_ON ); //亮灯	
		FaceTimer = 20;	
		break;	
	case FACE_ADD_DOWN:
		HAL_Voice_PlayingVoice(EM_MOVE_HEAD_DOWN_MP3,150);//请微微低头  
		FaceEnrollData.faceDirection=0x08;//低头  
		AppFaceWorkPro.Register++;
		App_LED_OutputCtrl( EM_LED_8, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_2, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_5, EM_LED_ON ); //亮灯
		FaceTimer = 20;
		break;
	case FACE_ADD_RIGHT:
		HAL_Voice_PlayingVoice(EM_TURN_FACE_RIGHT_MP3,150);//请把脸偏向右手边  
		FaceEnrollData.faceDirection=0x02;//右手边  
		AppFaceWorkPro.Register++;		
		App_LED_OutputCtrl( EM_LED_6, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_8, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_2, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_5, EM_LED_ON ); //亮灯
		FaceTimer = 19;
		break;
	case FACE_ADD_LEFT:
		FaceEnrollData.faceDirection=0x04;//左手边 
		AppFaceWorkPro.Register++;		
		HAL_Voice_PlayingVoice(EM_TURN_FACE_LEFT_MP3,150); //请把脸偏向左手边 
		App_LED_OutputCtrl( EM_LED_6, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_4, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_8, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_2, EM_LED_ON ); //亮灯
		App_LED_OutputCtrl( EM_LED_5, EM_LED_ON ); //亮灯	
		FaceTimer = 20;
		break;
	case FACE_WAIT_ADD_FRONT: 
	case FACE_WAIT_ADD_UP:
	case FACE_WAIT_ADD_DOWN:
	case FACE_WAIT_ADD_RIGHT:
	case FACE_WAIT_ADD_LEFT:	
		if(HAL_Voice_GetBusyState()!=0)//语音播报完成
		{
			break;
		}
		FaceGneralTaskFlow(FACE_CMD_ENROLL,(uint8_t *)&FaceEnrollData,35,FACE_ENROLL_TIMEOUT_22S); // 登记
		if(AppFaceWorkPro.TaskFlow == TASK_SUCCESS)//指令执行完成,
		{
			if(FaceMsgType.Reply.MsgMid==FACE_CMD_ENROLL) //接收成功
			{
				if( MR_SUCCESS == FaceMsgType.Reply.Msgresult) //成功
				{				
					if(AppFaceWorkPro.Register==FACE_WAIT_ADD_LEFT)//最后一步完成
					{
						HAL_Voice_PlayingVoice(EM_REGISTER_SUCCESS_MP3,200); //登记成功	 
						App_LED_OutputCtrl( EM_LED_CFG_NET, EM_LED_ON ); //亮灯25846
						//存储
						memset(&FaceAttribute,0,sizeof(FaceAttribute));
						//if(FaceMsgType.Reply.DataPack.Enroll.Direction ==0x1F )// 5个方向完成						
						FaceAttribute.FaceId=FaceMsgType.Reply.DataPack.Enroll.UserId; //拷贝ID
						FaceAttribute.FaceLim=FaceLim;//权限	
						FaceEepromSave();
						my_printf("Register face id = %x  ,FacePageId = %x\n",FaceAttribute.FaceId,FaceAttribute.FacePageId);
						SystemEventLogSave( ADD_FACE, FaceAttribute.FacePageId ); 
						if(SystemSeting.SystemAdminRegister==ADMIN_NONE_REGISTERED) //无注册改为本地注册
						{
							SystemSeting.SystemAdminRegister=ADMIN_LOCAL_REGISTERED;
							SystemWriteSeting((uint8_t *)&SystemSeting.SystemAdminRegister,2); //标记注册
						}						
						//完成
						AppFaceWorkPro.Register=FACE_ADD_SUCCESSFUL;
						return AppFaceWorkPro.Register;
					}
					AppFaceWorkPro.Register++;	//开始下个方向
					FaceClearTask(); //重新发送新指令
				}
				else //失败流程结束（超时或活检失败）
				{
					HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );//登记失败	
					AppFaceWorkPro.Register=FACE_ADD_ERROR;
				}
			}
		}
		else if(AppFaceWorkPro.TaskFlow == TASK_FAIL)//执行出错
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );//登记失败	
			AppFaceWorkPro.Register=FACE_ADD_ERROR;
		}
		break;
	case FACE_ADD_SUCCESSFUL:
		if(FaceGneralTaskFlow(FACE_CMD_ENROLL,(uint8_t *)&FaceEnrollData,35,FACE_ENROLL_TIMEOUT_22S)==TASK_POWERDOWN)
		{
			return FACE_ADD_OVER;
		}
	case FACE_ADD_ERROR	:
		//结束后执行下电
		if(FaceGneralTaskFlow(FACE_CMD_ENROLL,(uint8_t *)&FaceEnrollData,35,FACE_ENROLL_TIMEOUT_22S)==TASK_POWERDOWN)
		{
			return FACE_ADD_OVER;
		}
		break;
	case FACE_ADD_OVER:
		break;
	default:
		break;
	}
	
	switch(AppFaceWorkPro.Register)
	{
		case FACE_WAIT_ADD_FRONT: 
			if((FaceTimer%7 == 0) && (HAL_Voice_GetBusyState()==0) && (FaceTimer > 0 && FaceTimer < 20))
			{
				HAL_Voice_PlayingVoice(EM_CAPTURE_FACE_FRONT_MP3,100);//请录入正脸
			}
		break;
		case FACE_WAIT_ADD_UP:
			if((FaceTimer%7 == 0) && (HAL_Voice_GetBusyState()==0) && (FaceTimer > 0 && FaceTimer < 20))
			{
				HAL_Voice_PlayingVoice(EM_MOVE_HEAD_UP_MP3,150);//请微微抬头 
			}
		break;
		case FACE_WAIT_ADD_DOWN:
			if((FaceTimer%7 == 0) && (HAL_Voice_GetBusyState()==0) && (FaceTimer > 0 && FaceTimer < 20))
			{
				HAL_Voice_PlayingVoice(EM_MOVE_HEAD_DOWN_MP3,150);//请微微低头 
			}
		break;
		case FACE_WAIT_ADD_RIGHT:
			if((FaceTimer%7 == 0) && (HAL_Voice_GetBusyState()==0) && (FaceTimer > 0 && FaceTimer < 20))
			{
				HAL_Voice_PlayingVoice(EM_TURN_FACE_RIGHT_MP3,150);//请把脸偏向右手边
			}
		break;
		case FACE_WAIT_ADD_LEFT:
			if((FaceTimer%7 == 0) && (HAL_Voice_GetBusyState()==0) && (FaceTimer > 0 && FaceTimer < 20))
			{
				HAL_Voice_PlayingVoice(EM_TURN_FACE_LEFT_MP3,150); //请把脸偏向左手边 
			}
		break;
		default:
		break;
	}
	return AppFaceWorkPro.Register;
	#elif defined IRIS_FUNCTION_ON
	typedef struct
	{
		uint8_t left_right;		//1 ：双眼 2：左眼 3：右眼
		uint8_t admin;                    //是否设置为管理员yes:1 no:0
		uint8_t user_id_flag;//0:不自动生成id 1：自动生成id
		uint8_t user_id[2];//用户id 
		uint8_t user_name_flag;//0:不录入姓名 1：录入姓名
		uint8_t userName[32]; //录入用户姓名
		uint8_t timeOut;                  //录入超时时间 单位:s
	}IRIS_ENROLL_DATA;     //注册发送结构体 
	static IRIS_ENROLL_DATA IRISEnrollData;
	switch(AppFaceWorkPro.Register)
	{
	case FACE_ADD_FRONT:
		memset(&IRISEnrollData,0,sizeof(IRISEnrollData));
		HAL_Voice_PlayingVoice(EM_INPUT_IRIS_MP3,200);//请录入正脸  
		IRISEnrollData.left_right = 0x01;//双眼 
		IRISEnrollData.user_id_flag = 0x01;//自动分配id
		IRISEnrollData.timeOut = 15; 
		AppFaceWorkPro.Register++;
		App_LED_OutputCtrl( EM_LED_ALL, EM_LED_OFF );
		App_LED_OutputCtrl( EM_LED_1, EM_LED_ON ); //亮灯
		break;
	case FACE_WAIT_ADD_FRONT: 
	case FACE_WAIT_ADD_UP:
	case FACE_WAIT_ADD_DOWN:
	case FACE_WAIT_ADD_RIGHT:
	case FACE_WAIT_ADD_LEFT:	
		if(HAL_Voice_GetBusyState()!=0)//语音播报完成
		{
			break;
		}
		FaceGneralTaskFlow(IRIS_CMD_ENROLL,(uint8_t *)&IRISEnrollData,39,16000); // 登记
		if(AppFaceWorkPro.TaskFlow == TASK_SUCCESS)//指令执行完成,
		{
			if(FaceMsgType.Reply.MsgMid==IRIS_CMD_ENROLL) //接收成功
			{
				if( MR_SUCCESS == FaceMsgType.Reply.Msgresult) //成功
				{				
					if(AppFaceWorkPro.Register==1 )//最后一步完成
					{
						HAL_Voice_PlayingVoice(EM_REGISTER_SUCCESS_MP3,200); //登记成功	 
						App_LED_OutputCtrl( EM_LED_ALL, EM_LED_ON ); //亮灯25846
						//存储
						memset(&FaceAttribute,0,sizeof(FaceAttribute));
						//if(FaceMsgType.Reply.DataPack.Enroll.Direction ==0x1F )// 5个方向完成						
						FaceAttribute.FaceId=FaceMsgType.Reply.DataPack.Enroll.UserId; //拷贝ID
						FaceAttribute.FaceLim=FaceLim;//权限	
						FaceEepromSave();
						my_printf("Register face id = %x  ,FacePageId = %x\n",FaceAttribute.FaceId,FaceAttribute.FacePageId);
						SystemEventLogSave( ADD_IRIS, FaceAttribute.FacePageId ); 
						if(SystemSeting.SystemAdminRegister==ADMIN_NONE_REGISTERED) //无注册改为本地注册
						{
							SystemSeting.SystemAdminRegister=ADMIN_LOCAL_REGISTERED;
							SystemWriteSeting((uint8_t *)&SystemSeting.SystemAdminRegister,2); //标记注册
						}						
						//完成
						AppFaceWorkPro.Register=FACE_ADD_SUCCESSFUL;
						return AppFaceWorkPro.Register;
					}
//					AppFaceWorkPro.Register++;	//开始下个方向
					FaceClearTask(); //重新发送新指令
				}
				else //失败流程结束（超时或活检失败）
				{
					HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );//登记失败	
					AppFaceWorkPro.Register=FACE_ADD_ERROR;
				}
			}
		}
		else if(AppFaceWorkPro.TaskFlow == TASK_FAIL)//执行出错
		{
			HAL_Voice_PlayingVoice( EM_REGISTER_FAIL_MP3, 150 );//登记失败	
			AppFaceWorkPro.Register=FACE_ADD_ERROR;
		}
		break;
	case FACE_ADD_SUCCESSFUL:
		if(FaceGneralTaskFlow(IRIS_CMD_ENROLL,(uint8_t *)&IRISEnrollData,39,16000)==TASK_POWERDOWN)
			{
				return FACE_ADD_OVER;
			}
	case FACE_ADD_ERROR	:
		//结束后执行下电
		if(FaceGneralTaskFlow(IRIS_CMD_ENROLL,(uint8_t *)&IRISEnrollData,39,16000)==TASK_POWERDOWN)
		{
			return FACE_ADD_OVER;
		}
		break;
	case FACE_ADD_OVER:

		break;
	default:
		break;
	}
	return AppFaceWorkPro.Register;
	#endif
}


/***************************************************************************************
**函数名:       FaceEepromEmpty
**功能描述:     清空人脸存储
**输入参数:     
**输出参数:     
**备注:         该函数阻塞
****************************************************************************************/
void FaceEepromEmpty (void)
{
    uint16_t pageid = 0;
    uint32_t addr = FacePageidAddr(pageid);
	TimeoutSystick = 1000;//10s
	memset(&FaceAttribute,0,sizeof(FaceAttribute));
    for(pageid = 0; pageid < MSG_FACE_USER_NUM; pageid++)
    {
		addr = FacePageidAddr(pageid);
		HAL_EEPROM_WriteBytes(addr,(uint8_t *)&FaceAttribute,sizeof(FaceAttribute));	//直接读结构体
    }
	while(1)
	{
		FaceServerProcess();
		if(FaceGneralTaskFlow(FACE_CMD_DEL_ALL,0,0,FACE_DEFAULT_TIMEOUT_1S)==TASK_POWERDOWN)//清空
		{
			SystemSeting.SysFaceAdminNum = 0;
			break;
		}
		else if(TimeoutSystick == 0)
		{
			FacePowerDown();
			break;
		}
	}
}


#endif


//.end of the file.
