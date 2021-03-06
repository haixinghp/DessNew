/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: System.c 
* 作者：邓业豪
* 版本：V01
* 时间：20210729
* 内容简述：系统存储配置
****************************************************************************/
/* 标准头文件 */


/* 内部头文件 */
#include "System.h"
#include "Public.h"
#include "APP_BLE.h" 
#include "LockConfig.h"
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#include "APP_Finger.h"

uint8_t OpenDoorTimeCnt = 0; 
uint8_t LockConfigMode = 0; 

SYSTEM_SETTING SystemSeting ; //系统参数结构体
SYSTEM_FIX_SETTING SystemFixSeting;  //系统固化参数，恢复出厂不清空


/*********************************************************************************************************************
* Function Name :  SystemCfgVersionUpdate
* Description   :  配置版本兼容接口
* Para          :  bool isForce 是否强制恢复，恢复出厂的时候传入true , 其他场景传入flase
* Return        :  0失败  1成功
*********************************************************************************************************************/
uint8_t SystemCfgVersionUpdate(bool isForce)
{
    my_printf("# SystemCfgVersionUpdate # version is <%d> now\n", SystemSeting.SystemVersion);
    if(isForce || SystemSeting.SystemVersion < EEPROM_CFG_VERSION_2)
    {
        my_printf("cfg update to EEPROM_CFG_VERSION_2 \n");
        if(APP_FINGER_GetPowerFlag() == 0)
            return 0;

        SystemSeting.FingerFlag = APP_FINGER_GetPowerFlag();
        SystemSeting.SystemVersion = EEPROM_CFG_VERSION_2;
    }

    if(isForce || SystemSeting.SystemVersion < EEPROM_CFG_VERSION_3)
    {
        my_printf("cfg update to EEPROM_CFG_VERSION_3 \n");
        if(APP_FINGER_GetProtocalVersion() == 0)
            return 0;

        SystemSeting.FingerProtocalVersion = APP_FINGER_GetProtocalVersion();
        SystemSeting.SystemVersion = EEPROM_CFG_VERSION_3;
    }

    /* 写EEPROM */
    uint8_t tp1 = 0;
    for(uint8_t i=0; i<3; i++)
    {
        if( 1 == HAL_EEPROM_WriteBytes(MEM_FACT_START,(uint8_t *)(&SystemSeting),sizeof(SystemSeting)) )  //写入成功
        {
            tp1 = 1;
            break;
        }
    }
    if( tp1 == 0 )  //写入失败
        return 0;
    
    return 1;
}


 /*********************************************************************************************************************
* Function Name :  SystemInitFlash
* Description   :  恢复出厂设置，清空所有记录  本地管理员无注册
* Para          :  无
* Return        :  0失败  1成功
*********************************************************************************************************************/
uint8_t SystemInitFlash(void)  
{
	uint8_t clear[MEM_FACT_SIZE];
	memset(clear,0,sizeof(clear));
	HAL_EEPROM_WriteBytes(0,clear,sizeof(clear));//存储全部写0
	
    uint8_t aeskey[SYSTEM_AESKEY_LEN] = {0};
    memcpy((void*)aeskey, (void*)&SystemSeting.SysFingerAsekey, SYSTEM_AESKEY_LEN);

	//修改出厂配置，需要写值的以下处理，其余默认0
	memset(&SystemSeting,0,sizeof(SystemSeting));//结构体清零
    
    // 写回暂时不需要清理的数据
    memcpy((void*)&SystemSeting.SysFingerAsekey, (void*)aeskey, SYSTEM_AESKEY_LEN);
    
    SystemSeting.SysFactStartFig = MEM_FACT_MEM_FIG;           //0   //工厂数据起始标志  G
 
	OpenKind_U openKind ={0};
	openKind.bit.PwdMode = FUNCTION_ENABLE;            //密码解锁方式     0: 不支持  1:支持
	#ifdef FINGER_FUNCTION_ON
	openKind.bit.FingerMode = FUNCTION_ENABLE;         //指纹解锁方式     0: 不支持  1:支持
	#endif
	#ifdef SMART_KEY_FUNCTION_ON
	openKind.bit.SmartKeyMode = FUNCTION_DISABLE;      //智能钥匙解锁（蓝牙钥匙/蓝牙手环）     0: 不支持  1:支持
	#endif
	#ifdef FACE_FUNCTION_ON
	openKind.bit.FaceMode = FUNCTION_ENABLE;           //人脸解锁方式     0: 不支持  1:支持
	#endif
	#ifdef IC_CARD_FUNCTION_ON
	openKind.bit.IcCardMode = FUNCTION_ENABLE;         //IC卡解锁方式     0: 不支持  1:支持
	#endif
	#ifdef FINGER_VEIN_FUNCTION_ON
	openKind.bit.FingerVeinMode  = FUNCTION_DISABLE;   //指静脉解锁方式  0: 不支持  1:支持
	#endif
	#ifdef IRIS_FUNCTION_ON
	openKind.bit.IrisOpenMode    = FUNCTION_ENABLE;    //虹膜解锁方式 	 0: 不支持  1:支持
	#endif
	#ifdef HW_WALLET_FUNCTION_ON
	openKind.bit.HuaWeiWalletMode = FUNCTION_DISABLE;  //华为钱包解锁方式 0: 不支持  1:支持
	#endif
	SystemSeting.SysLockMode = openKind.data;                  //1   //开锁模式(指纹/密码蓝牙/蓝牙钥匙等)
	SystemSeting.SysFingerAllNum   = 0;  	   			 	   //2   //指纹总量 
	SystemSeting.SysFingerAdminNum = 0;  	   				   //3   //本地管理员指纹数量
	SystemSeting.SysFingerGuestNum = 0;  	   				   //4   //本地普通指纹数量
	SystemSeting.SysFaceAllNum   = 0;  	   		    		   //5   //人脸总数量
	SystemSeting.SysFaceAdminNum = 0;  	   		    		   //6   //本地管理员人脸数量
	SystemSeting.SysFaceGuestNum = 0;  	   		    		   //7   //本地普通人脸数量
	SystemSeting.SysCardAllNum   = 0;  	   		    		   //8   //卡总数量
	SystemSeting.SysCardAdminNum = 0;  	   		    		   //9   //本地管理员卡数量
	SystemSeting.SysCardGuestNum = 0;  	   		    		   //10  //本地普通卡数量
	SystemSeting.SysPwdAllNum   = 0;  	   		    		   //11  //密码总数量
	SystemSeting.SysPwdAdminNum = 0;  	   		    		   //12  //本地管理员密码数量
	SystemSeting.SysPwdGuestNum = 0;  	   		    		   //13  //本地普通密码数量
	SystemSeting.SysSmartKeyNum = 0;  	   		    		   //14  //全部蓝牙钥匙数量h （20个）
	SystemSeting.SysWifiMainSw  = FUNCTION_DISABLE;  	       //15  //wifi主开关
	SystemSeting.SystemAdminRegister = ADMIN_NONE_REGISTERED;  //16  //是否注册标记 
	#if defined LOCK_BODY_216_MOTOR || defined LOCK_BODY_218_MOTOR || defined LOCK_BODY_AUTO_MOTOR
	SystemSeting.DoorUnlockWarmSw  = FUNCTION_ENABLE;          //18  //门未关报警设置  0= 关闭报警 1= 开启报警   
	#endif

	#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
	SystemSeting.FaceCheckEnable = 0x55;            		   //19  //人脸验证开关  0x55= 开启  0x66= 关闭
	#else
	SystemSeting.FaceCheckEnable = 0x66;            		   //19  //人脸验证开关  0x55= 开启  0x66= 关闭
	#endif
	
	#if defined LOCK_BODY_211_MOTOR  
	SystemSeting.LockBodyMode   = LOCK_BODY_212;  	   	 	   //20  //锁体类型  0x55= 218锁体  0x66= 212锁体 
	#elif defined LOCK_BODY_216_MOTOR
	SystemSeting.LockBodyMode   = LOCK_BODY_216;  	   	 	   //20  //锁体类型  0x55= 218锁体  0x66= 212锁体 
	#elif defined LOCK_BODY_218_MOTOR
	SystemSeting.LockBodyMode   = LOCK_BODY_218;  	   	 	   //20  //锁体类型  0x55= 218锁体  0x66= 212锁体 
	#elif defined LOCK_BODY_AUTO_MOTOR
	SystemSeting.LockBodyMode   = LOCK_BODY_212;  	   	 	   //20  //锁体类型  0x55= 218锁体  0x66= 212锁体 
	#endif
	LockConfigMode = SystemSeting.LockBodyMode; 
	
	SystemSeting.SysWifiLogSw   = FUNCTION_DISABLE;  	   	   //22  //wifi开门信号上传开关
	SystemSeting.SysVoice = HIGH_VOICE_VOL;              	   //23  //语音开关默认最高
	SystemSeting.SysKeyDef = FUNCTION_DISABLE;                 //24  //一键布防开关    默认关闭
	SystemSeting.SysHumanIrDef = 0;         				   //25  //主动防御(工作时间)   0:关闭  (1-255秒)打开   
	SystemSeting.SysWifiSingle = FUNCTION_DISABLE;             //26   //门铃单双向
	SystemSeting.SysAutoLockTime = 0;         				   //27   //自动上锁时间
	SystemSeting.Sysprotect_lock = FUNCTION_DISABLE;           //28   //锁门防止误触,确认键
	SystemSeting.SysFingerFac = FUNCTION_DISABLE;              //29   //指纹厂家，通过厂家判断加密
	SystemSeting.SysFaceFac   = FUNCTION_DISABLE;              //30   //人脸厂家，通过厂家判断加密
	#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON  
	SystemSeting.SysDrawNear   = E_SENSE_HIGH;                 //31  //接近感应灵敏度  默认最高
	#else 
	SystemSeting.SysDrawNear   = E_SENSE_OFF;                  
	#endif

	SystemSeting.SysCompoundOpen = DOUBLE_CHECK_SW_OFF;        //64  //组合开门开关/双重认证开关   
	SystemSeting.SystemVersion  = EEPROM_CFG_DEFAULT;          //66  //版本记录(EEPROM变革跟踪)
	SystemSeting.CheckErrAllCnt = 0;         			       //68  //验证失败总计次   
	SystemSeting.CheckErrPwdCnt = 0;         			       //69  //密码验证失败计次   
	SystemSeting.TryForbitUtc   = 0;         			       //72-75//错误禁试发生时间 

	#if defined FACE_FUNCTION_ON
	SystemSeting.FaceOrIrisUnlockSuccessCnt = 0;			   //76-77人脸开锁成功次数
	SystemSeting.FaceOrIrisUnlockFailCnt = 0;					//78-79人脸开锁失败次数
	#endif
	SystemSeting.FingerOrVeinUnlockSuccessCnt = 0;				//80-81指纹开锁成功次数
	SystemSeting.FingerOrVeinUnlockFailCnt = 0;					//82-83指纹开锁失败次数
 
//	SystemSeting.SysFactDoneFig = 0;//MEM_FACT_MEM_FIG;   		   //65  //固定标记
   	
	uint8_t tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_WriteBytes(MEM_FACT_START,(uint8_t *)(&SystemSeting),sizeof(SystemSeting)) )  //写入成功
		{
			tp1 = 1;
			break;
		}
	}
	if( tp1 == 0 )  //写入失败
		return 0;

	//读取不可修改系统参数
	tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( HAL_EEPROM_ReadBytes(MEM_FIX_FACT_START,(uint8_t *)(&SystemFixSeting),sizeof(SystemFixSeting)) )
		{
			tp1 = 1;
			break;
		}
	}
	if( tp1 == 0 )  //读取失败
		return 0;
	
	if( MEM_FACT_MEM_FIG != SystemFixSeting.SysFixStartFig ) //写入标志未初始化
	{
		memset(clear,0,sizeof(clear));
		HAL_EEPROM_WriteBytes(MEM_FIX_FACT_START,clear,sizeof(clear));//存储全部写0
		
		SystemFixSeting.SysFixStartFig = MEM_FACT_MEM_FIG;
		SystemFixSeting.MotorTorque = LOW_TORQUE;         //默认低扭力
		SystemFixSeting.MotorDirection = RIGHT_HAND_DOOR; //默认右开
		tp1 = 0;
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes(MEM_FIX_FACT_START,(uint8_t *)(&SystemFixSeting),sizeof(SystemFixSeting)) )
			{
				tp1 = 1;
				break;
			}
		}
		if( tp1 == 0 )  //读取失败
			return 0;
	}
 	
/******************模组清空*****************************/



/******************模组清空*****************************/
	return 1;
}


 /*********************************************************************************************************************
* Function Name :  SystemReadFlash
* Description   :  读取系统配置
* Para          :  无
* Return        :  0失败  1成功
*********************************************************************************************************************/
uint8_t SystemReadFlash ( void ) 
{
	//系统地址1
	uint8_t	tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_ReadBytes(MEM_FACT_START, (uint8_t *)(&SystemSeting), sizeof(SystemSeting)) )
		{
			tp1 = 1;
			break;   //读取成功
		}
	}
	if( tp1 == 0 )  //读取失败
		return 0;
 
	if( (SystemSeting.SysFactDoneFig != MEM_FACT_MEM_FIG) || (SystemSeting.SysFactStartFig != MEM_FACT_MEM_FIG) )
	{
		return 0;   //工厂标志错误
	}
	
	#ifdef LOCK_BODY_AUTO_MOTOR
	if( OpenDoorTimeCnt == 0 )
	{
	   if( SystemSeting.LockBodyMode != LOCK_BODY_212 )
	   {
		   SystemSeting.LockBodyMode = LOCK_BODY_212;
		   SystemWriteSeting(&SystemSeting.LockBodyMode,1);//写配置
	   }
	}
    LockConfigMode = SystemSeting.LockBodyMode; 
	if( OpenDoorTimeCnt < 2 )  //前2次强制改成218
	{
		LockConfigMode = LOCK_BODY_218;
	}
		
	#endif
	//系统地址2
	tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_ReadBytes(MEM_FIX_FACT_START, (uint8_t *)(&SystemFixSeting), sizeof(SystemFixSeting)) )
		{
			tp1 = 1;
			break;   //读取成功
		}
	}
	if( tp1 == 0 )  //读取失败
		return 0;
 
	if( SystemFixSeting.SysFixStartFig != MEM_FACT_MEM_FIG )
	{
		return 0;   //工厂标志错误
	}

	return 1;  //成功
}

 /*********************************************************************************************************************
* Function Name :  SystemReadSeting
* Description   :  单独读取系统变量结构体成员
* Para          :  无
* Return        :  0失败  1成功
*********************************************************************************************************************/
uint8_t SystemReadSeting ( uint8_t *setting, uint8_t len )   
{
	uint32_t add= setting - (uint8_t *)( &SystemSeting); //结构体偏移值对应EE地址，从0开始
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_ReadBytes(add, setting, len) )
		{
			return 1;
		}	
	}
	return 0;	
}

 /*********************************************************************************************************************
* Function Name :  SystemWriteSeting
* Description   :  写入单系统参数值
* Para          :  *setting       指定SystemSeting结构体成员地址
                    data          数值
* Return        :  0失败  1成功
* note          :  函数内需校验data有效参数，同时计算成员长度
* example       ： 写入管理员注册成功标标志
				   SystemSeting.SystemAdminRegister=ADMIN_LOCAL_REGISTERED;
                   SystemWriteSeting((uint8_t *)&SystemSeting.SystemAdminRegister);
*********************************************************************************************************************/
uint8_t SystemWriteSeting( uint8_t *setting, uint8_t len )   
{
	if( len == 0 )
		return 0;
	
	if(setting == (uint8_t *)(&SystemSeting.SystemAdminRegister)) //注册标志
	{
		if((ADMIN_LOCAL_REGISTERED != SystemSeting.SystemAdminRegister)
		 &&(ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister)) //参数合法判断
		{
			return 0;
		}
	}
	else if(setting ==&SystemSeting.SysWifiMainSw)//WIFI开关
	{
		if( (SystemSeting.SysWifiMainSw !=0) && (SystemSeting.SysWifiMainSw !=1) )
		{
			return 0;
		}
	}
	else if(setting ==SystemSeting.SysFingerAsekey)
	{

	}

	uint32_t add= MEM_FACT_START + (setting - (uint8_t *)(&SystemSeting)); //结构体偏移值对应EE地址，从0开始
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_WriteBytes(add, setting, len) )
		{
			return 1;
		}	
	}
 
	return 0;
}

 /*********************************************************************************************************************
* Function Name :  SystemReadFixSeting
* Description   :  单独读取系统变量结构体成员
* Para          :  无
* Return        :  0失败  1成功
*********************************************************************************************************************/
uint8_t SystemReadFixSeting( uint8_t *setting, uint8_t len )   
{
	uint32_t add= MEM_FACT_START + (setting - (uint8_t *)( &SystemFixSeting)); //结构体偏移值对应EE地址，从0开始
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_ReadBytes(add, setting, len) )
		{
			return 1;
		}	
	}
	return 0;
}
 /*********************************************************************************************************************
* Function Name :  SystemWriteFixSeting
* Description   :  写入单系统参数值
* Para          :  *setting       SystemFixSeting
                    len           长度
* Return        :  0失败  1成功
* note          :  
*********************************************************************************************************************/
uint8_t SystemWriteFixSeting( uint8_t *setting, uint8_t len )   
{
	if( len == 0 )
		return 0;
	
	uint32_t add= MEM_FIX_FACT_START+ (setting - (uint8_t *)(&SystemFixSeting)); //结构体偏移值对应EE地址，从0开始
	
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_WriteBytes(add,setting,len) )
		{
			return 1;
		}	
	}
	return 0;
}

 /*********************************************************************************************************************
* Function Name :  SystemEventLogSave
* Description   :  写锁具事件记录
* Para          :  event_type      事件类型
                   event_id        事件用户ID记录
* Return        :  无
* note          :  事件记录相关参数均不能硬清空
* example       ： 
*********************************************************************************************************************/
void SystemEventLogSave( LOCK_EVENT_LOG event_type, uint16_t pageid )
{ 
	uint8_t temp[MSG_LOG_RECORD_REG_ONE_SIZE];
	memset ( temp, 0, MSG_LOG_RECORD_REG_ONE_SIZE );
	temp[2] = (uint8_t)(pageid >> 8);//先拷贝ID，无ID下面可直接覆盖
	temp[3] = (uint8_t)pageid;
	switch (event_type)
    {
    	case BAC_OPEN_IN_DOOR :     memcpy ( temp, "BCBopen ", 8 );break;//门内把手开
    	case BAC_CLOSE_IN_DOOR :    memcpy ( temp, "BCBclose", 8 );break;//门内把手关
		case KEY_OPEN_IN_DOOR :     memcpy ( temp, "KOKopen ", 8 );break;//门内按键开
		case KEY_CLOSE_IN_DOOR :    memcpy ( temp, "KCKclose", 8 );break;//门内按键关		
		case FALSE_LOCK_ALARM :     memcpy ( temp, "JLJLOCK ", 8 );break;//假锁报警
		case CLOSE_OUT_DOOR :       memcpy ( temp, "WCWclose", 8 );break;//门外上锁
		case AUTO_CLOSE_DOOR :      memcpy ( temp, "OCOclose", 8 );break;//自动上锁
		case EMPTY_LOCK :           memcpy ( temp, "EEEMPTY ", 8 );break;//清空锁具
		case TEMP_PASSWORD_OPEN :   memcpy ( temp, " KTEMP  ", 8 );break;//临时密码开门
		case TRY_OPRN_ALARM :       memcpy ( temp, " TTEST  ", 8 );break;//禁试报警
		case PICK_OPRN_ALARM :      memcpy ( temp, " AALARM ", 8 );break;//撬锁报警
		case DELETE_SOS_PASSWORD :  memcpy ( temp, "-KSOSPWD", 8 );break;//报警密码删除
		case ADD_SOS_PASSWORD :     memcpy ( temp, "+KSOSPWD", 8 );break;//增加报警密码
		case SOS_PASSWORD_OPEN :    memcpy ( temp, " KSOSPWD", 8 );break;//报警密码开门	
		case DELETE_PASSWORD :      memcpy ( temp, "-KPWD   ", 8 );break;//密码删除
		case ADD_PASSWORD :         memcpy ( temp, "+KPWD   ", 8 );break;//增加密码
		case PASSWORD_OPEN :        memcpy ( temp, " KPWD   ", 8 );break;//密码开门		
		case ADD_SMART_KEY :        memcpy ( temp, "+S", 2 );break;//增加电子钥匙
		case DELETE_SMART_KEY :     memcpy ( temp, "-S", 2 );break;//删除电子钥匙
		case SMART_KEY_OPEN :       memcpy ( temp, " S", 2 );
									memcpy ( &temp[2], (uint8_t*)&pageid, 4 ); //记录ID
									break;//电子钥匙开门
		case ADD_FACE :             memcpy ( temp, "+R", 2 );break;//增加人脸
		case DELETE_FACE :          memcpy ( temp, "-R", 2 );break;//删除人脸
        case FACE_OPEN :            memcpy ( temp, " R", 2 );break;//人脸开门
		case ADD_BLE :              memcpy ( temp, "+P", 2 );break;//增加蓝牙账号
		case DELETE_BLE :           memcpy ( temp, "-P", 2 );break;//删除蓝牙账号
        case BLE_OPEN :             memcpy ( temp, " P", 2 );break;//蓝牙账号开门
		case ADD_CARD :             memcpy ( temp, "+C", 2 );break;//增加卡
		case DELETE_CARD :          memcpy ( temp, "-C", 2 );break;//删除卡
        case CARD_OPEN :            memcpy ( temp, " C", 2 );break;//卡开门
		case ADD_FINGER :           memcpy ( temp, "+F", 2 );break;//增加指纹
		case DELETE_FINGER :        memcpy ( temp, "-F", 2 );break;//删除指纹
        case FINGER_OPEN :          memcpy ( temp, " F", 2 );break;//指纹开门
		case ADD_VEIN :            	memcpy ( temp, "+V", 2 );break;//增加指静脉
		case DELETE_VEIN :          memcpy ( temp, "-V", 2 );break;//删除指静脉
		case VEIN_OPEN :           	memcpy ( temp, " V", 2 );break;//指静脉开门
		case ADD_IRIS :            	memcpy ( temp, "+I", 2 );break;//增加虹膜
		case DELETE_IRIS :          memcpy ( temp, "-I", 2 );break;//删除虹膜
		case IRIS_OPEN :           	memcpy ( temp, " I", 2 );break;//虹膜开门
		case NOTHING_CASE :         memcpy ( temp, "AU", 2 );break;//触发后无动作唤醒源
		case FACE_ADMIN_CHECK :     memcpy ( temp, "AR", 2 );break;//人脸进入菜单记录
		case FINGER_ADMIN_CHECK :   memcpy ( temp, "AF", 2 );break;//指纹进入菜单记录
		case PWD_ADMIN_CHECK :      memcpy ( temp, "AK", 2 );break;//密码进入菜单记录
		case VEIN_ADMIN_CHECK :     memcpy ( temp, "AV", 2 );break;//指静脉进入菜单记录
        case IRIS_ADMIN_CHECK :     memcpy ( temp, "AI", 2 );break;//虹膜进入菜单记录
    	default:
    		break;
    }
  
	RTC_TimeUpdate(RTC_TIME_CLOCK_BCD); //获取最新时间
    temp[8] = Rtc_Real_Time.year;
    temp[9] = Rtc_Real_Time.month ;
    temp[10] = Rtc_Real_Time.day ;
    temp[11] = Rtc_Real_Time.hour ;
    temp[12] = Rtc_Real_Time.minuter ;
    temp[13] = Rtc_Real_Time.second ;	
	
//	if(event_type==EMPTY_LOCK)
//	{
//		memcpy(SystemFixSeting.SysClearCase,temp,14);
//		SystemWriteFixSeting(SystemFixSeting.SysClearCase,14); //硬清空存储位置区分，不记次数
//		return;
//	}

    uint32_t addr = MSG_LOG_RECORD_START + SystemFixSeting.SysLockLogSn * MSG_LOG_RECORD_ONE_SIZE; //计算存储地址
	HAL_EEPROM_WriteBytes(addr,temp,sizeof(temp)); //14字节记录写入
	
	
	SystemFixSeting.SysLockLogSn ++; 
	if ( SystemFixSeting.SysLockLogSn >= MSG_LOG_RECORD_NUM )
    {
        SystemFixSeting.SysLockLogSn = 0;
    }
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogSn,2);//当前存储位置
	
	
	SystemFixSeting.SysLockLogAll++;
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogAll,2);//存储记录总次数
	my_printf( "SysLockLogSn=%d    SysLockLogAll=%d  \n", SystemFixSeting.SysLockLogSn, SystemFixSeting.SysLockLogAll);
	return;
}
 /*********************************************************************************************************************
* Function Name :  SystemEventLogClear
* Description   :  清空事件记录
* Para          :  
* Return        :  无
* note          :  
* example       ： 
*********************************************************************************************************************/
void SystemEventLogClear( void )
{
	uint8_t flash[MSG_LOG_RECORD_ONE_SIZE]={0};
	SystemFixSeting.SysLockLogSn =0;  
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogSn,2);//当前存储位置
	SystemFixSeting.SysLockLogAll =0;
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogAll,2);//存储记录总次数
	for(uint32_t i=0;i<MSG_LOG_RECORD_NUM;i++)
	{
		uint32_t add= MSG_LOG_RECORD_START + i * MSG_LOG_RECORD_ONE_SIZE;
		HAL_EEPROM_WriteBytes(add,flash,MSG_LOG_RECORD_ONE_SIZE);
	}
}


//.end of the file.






