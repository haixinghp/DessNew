/********************************************************************************************************************
 * @file:      App_PWD.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2021-08-09
 * @brief:     密码功能函数  
 * @Description:   
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "App_PWD.h" 
#include "Public.h"
#include "System.h"
#include "App_BLE.h"
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"
/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#define  MSG_PWD_ALL_NUM            MEM_BOARDPWD_ALL       //可用密码总组数
#define  MSG_PWD_ONE_VALUE_SIZE     24                     //单个密码的有效长度 
#define  MSG_PWD_MEG_SIZE           32                     //单个密码分配的信息长度
#define  MEM_PWD_START_ADDR         MEM_BOARDPWD_START     //密码在EEPROM中的起始地址  

#define  MEM_TMP_PWD_START_ADDR     MEM_BOARDTEMPPWD_START //临时密码在EEPROM中的起始地址  
#define  MSG_TMP_PWD_MEG_SIZE       16                     //单个临时密码分配的信息长度
#define  MSG_TMP_PWD_VALUE_SIZE     8                      //单个临时密码的有效长度

#define  MEM_SOS_PWD_START_ADDR     MEM_BOARDSOSPWD_START  //报警密码在EEPROM中的起始地址  
#define  MSG_SOS_PWD_MEG_SIZE       32                     //单个临时密码分配的信息长度
#define  MSG_SOS_PWD_VALUE_SIZE     8                      //单个临时密码的有效长度

/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/


/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         


/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
typedef struct
{
     uint8_t   UserKind;    	//用户类型 0x01指纹，0x02密码，0x04人脸， 0x03智卡，
     uint8_t   Limitkind;       //用户属性  
	 uint8_t   Weekday;         //星期
	 uint16_t  UserId;          //用户编号
	 uint32_t  StartTim;        //生效时间
	 uint32_t  StopTim;         //失效时间	 
	
}UserMegTab_T;

static uint32_t PwdTickTimMs = 0;
/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  App_PWD_FileInit()
* Description   :  相关初始化
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_PWD_FileInit( void )
{
	
}

/*********************************************************************************************************************
* Function Name :  App_PWD_Tim10Ms()
* Description   :  相关定时器  10ms执行一次
* Para          :  无
* Return        :  void
*********************************************************************************************************************/
void App_PWD_Tim10Ms( void )
{
	if( PwdTickTimMs > 0 )
		PwdTickTimMs--;
}
 
/*********************************************************************************************************************
* Function Name :  App_PWD_ChangePwdPageidToEepromAddr
* Description   :  获取密码存储与EEPROM中的地址
* Para          :  pageid - 存储的页码
* Return        :  对应eeprom中的地址
*********************************************************************************************************************/
static uint32_t App_PWD_ChangePwdPageidToEepromAddr( uint16_t pageid )   
{
    return MEM_PWD_START_ADDR + pageid * MSG_PWD_MEG_SIZE;
}

/*********************************************************************************************************************
* Function Name :  App_PWD_SearchEmptyPwdEepromAddr
* Description   :  寻找空的地址.
* Para          :  none
* Return        :  对应EEPROM地址
*********************************************************************************************************************/
static uint32_t App_PWD_SearchEmptyPwdEepromAddr( void )
{
    uint16_t pageid;
    uint32_t addr;
	PwdMeg_T pwdMeg={0};
	
    for(pageid=0; pageid<MSG_PWD_ALL_NUM; pageid++)
    {
        addr = App_PWD_ChangePwdPageidToEepromAddr( pageid );

        for(uint8_t i=0; i<3; i++)
		{
		    if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), 1 ) )  
				break;
		}
	
        if( pwdMeg.UserValue != MEM_PWD_VALID_FLG )//指纹存在标志
        {
            return addr;					 
        }
		else 
		{
			continue;
		}
    }
    return addr;
}

/*********************************************************************************************************************
* Function Name :  App_PWD_SaveOnePwdMegIntoEeprom
* Description   :  将单个密码信息保存于EEPROM中
* Para          :  ppwdmeg - 密码信息指针
* Return        :  none
*********************************************************************************************************************/
void App_PWD_SaveOnePwdMegIntoEeprom( PwdMeg_T *ppwdmeg )
{
	my_printf( "App_PWD_SaveOnePwdMegIntoEeprom()\n" );   
	uint32_t addr;
	addr = App_PWD_SearchEmptyPwdEepromAddr();
	for(uint8_t i=0; i<MSG_PWD_BYTE_SIZE; i++)
	{
		ppwdmeg->Password[i] ^= AppBleType.ChannelPwd[i%4];
	}
	uint8_t tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_WriteBytes( addr, (uint8_t *)ppwdmeg, MSG_PWD_ONE_VALUE_SIZE ) )  
		{
			tp1 = 1;
			break;
		}
	}
	if( tp1 == 0 )
		return;
	
	if( ppwdmeg->Privileges == PWD_LIMIT_ADMIN )           //管理员用户
	{
		SystemSeting.SysPwdAdminNum++;
		(void)SystemWriteSeting( &SystemSeting.SysPwdAdminNum, sizeof SystemSeting.SysPwdAdminNum );   	
	}
	else if( ppwdmeg->Privileges == PWD_LIMIT_GUEST )      //普通用户
	{
	    SystemSeting.SysPwdGuestNum++;
		(void)SystemWriteSeting( &SystemSeting.SysPwdGuestNum, sizeof SystemSeting.SysPwdGuestNum );   
	}
	
	SystemSeting.SysPwdAllNum++;
	(void)SystemWriteSeting( &SystemSeting.SysPwdAllNum, sizeof SystemSeting.SysPwdAllNum );   
	
}

/*********************************************************************************************************************
* Function Name :  App_PWD_QueryPwdByStringFromEeprom
* Description   :  通过输入的密码串，查询EEPROM中是否有对应的密码  
* Para          :  userType - 用户类型 'M'-管理员 'G'-普通用户  'A'-所有用户    
                   ppwdmeg - 读取出密码信息指针  pdata-待验证的虚位密码指针
* Return        :  0-未找到  1- 已找到
*********************************************************************************************************************/
uint8_t App_PWD_QueryPwdByStringFromEeprom( uint8_t userType, PwdMeg_T *ppwdmeg, char *pdata )
{
	my_printf( "App_PWD_QueryPwdByStringFromEeprom()\n" );   
	uint8_t   retval = 0;
	uint16_t  pageid;
	uint32_t  addr;
	PwdMeg_T  pwdMeg={0};

	if( (userType == PWD_LIMIT_ADMIN) || (userType == PWD_LIMIT_GUEST) )     //管理员验证/普通用户验证
	{
		for(pageid=0; pageid<MSG_PWD_ALL_NUM; pageid++)
		{
			addr = App_PWD_ChangePwdPageidToEepromAddr(pageid);
			for(uint8_t i=0; i<3; i++)
			{
				if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), 1 ) )  
					break;
			}
			
			if( pwdMeg.UserValue == MEM_PWD_VALID_FLG )//存在标志 
			{
				for(uint8_t i=0; i<3; i++)
				{
					if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE ) )  
						break;
				}
				
				if( pwdMeg.Privileges == userType )     
				{
					 for(uint8_t i=0; i<MSG_PWD_BYTE_SIZE; i++)
					 {
						 pwdMeg.Password[i]^=AppBleType.ChannelPwd[i%4];
					 }
					 if( strstr( pdata, (char*)pwdMeg.Password ) != NULL )  //比对成功
					 {
						 memcpy( (uint8_t *)ppwdmeg, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE );
						 retval = 1;
						 break;
					 }
					 else
					 {
						  continue;
					 }
				}
				else 
				{
					 continue;
				}
			}
			else
			{
				continue;
			}
		}
		return retval;
	}
	else if( userType == PWD_LIMIT_ALL )  
	{
		for(pageid=0; pageid<MSG_PWD_ALL_NUM; pageid++)
		{
			addr = App_PWD_ChangePwdPageidToEepromAddr(pageid);
			for(uint8_t i=0; i<3; i++)
			{
				if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), 1 ) )  
					break;
			}
			
			if( pwdMeg.UserValue == MEM_PWD_VALID_FLG )//存在标志 
			{
				for(uint8_t i=0; i<3; i++)
				{
					if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE ) )  
						break;
				}
				for(uint8_t i=0; i<MSG_PWD_BYTE_SIZE; i++)
				{
					pwdMeg.Password[i]^=AppBleType.ChannelPwd[i%4];
				}
				 if( strstr( pdata,  (char*)pwdMeg.Password ) != NULL )  //比对成功
				{
					memcpy( (uint8_t *)ppwdmeg, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE );
				    retval = 1;
					break;
				}
				else
				{
				    continue;
				}
			}
			else
			{
				continue;
			}
		}
		return retval;
	}

	return retval;
}
 
/*********************************************************************************************************************
* Function Name :  App_PWD_QueryByIdFromEeprom
* Description   :  通过用户编号，查询EEPROM中是否有对应的密码用户编号
* Para          :  pwdId - 待查找的用户编号   paddr - 此编号对应的EEPROM地址  ppwdmeg-对应编号密码的信息指针
* Return        :  0-未找到  1- 已找到
*********************************************************************************************************************/
uint8_t App_PWD_QueryByIdFromEeprom( uint16_t pwdId, uint32_t *paddr, PwdMeg_T *ppwdmeg )   
{
	uint8_t  retval = 0;
	PwdMeg_T  pwdMeg={0};
	
	for(uint16_t pageid=0; pageid<MSG_PWD_ALL_NUM; pageid++)
	{
		uint32_t addr = App_PWD_ChangePwdPageidToEepromAddr(pageid);
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), 1 ) )  
				break;
		}
		if( pwdMeg.UserValue == MEM_PWD_VALID_FLG )//指纹存在标志 
		{
			for(uint8_t i=0; i<3; i++)
			{
				if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE ) )  
					break;
			}
			if( pwdId == pwdMeg.UserId )  //找到了对应的ID
			{
				*paddr = addr;
				 for(uint8_t i=0; i<MSG_PWD_BYTE_SIZE; i++)
				 {
					pwdMeg.Password[i] ^= AppBleType.ChannelPwd[i%4];
				 }
				 memcpy( (uint8_t *)ppwdmeg, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE );
				 retval = 1;
				 break;
			}	  
			else 
			{
				 continue;
			}							
		}
		else
		{
			continue;
		}
	}
	return retval;    
}

/*********************************************************************************************************************
* Function Name :  App_PWD_DelPwdIdFromEeprom
* Description   :  通过用户编号，删除EEPROM中对应编号的密码
* Para          :  pwdId - 待删除的密码ID   
* Return        :  0-删除失败  1- 删除成功
*********************************************************************************************************************/
uint8_t App_PWD_DelPwdIdFromEeprom( uint16_t pwdId )
{
	my_printf( "App_PWD_DelPwdIdFromEeprom()\n" );   
	uint8_t  retval = 0;
	uint32_t addr;
	PwdMeg_T pwdMeg={0};
	
	if( 1 == App_PWD_QueryByIdFromEeprom(pwdId, &addr, &pwdMeg) )   
	{
		pwdMeg.UserValue = 0xff;

		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, (uint8_t *)(&pwdMeg), 1 ) )  
				break;
		}
		if( pwdMeg.Privileges == PWD_LIMIT_ADMIN )           //管理员用户
		{
			if( SystemSeting.SysPwdAdminNum > 0)
			    SystemSeting.SysPwdAdminNum--; 
			(void)SystemWriteSeting( &SystemSeting.SysPwdAdminNum, sizeof SystemSeting.SysPwdAdminNum );   	
			
		}
		else if( pwdMeg.Privileges == PWD_LIMIT_GUEST )      //普通用户
		{
			if( SystemSeting.SysPwdGuestNum > 0)
			    SystemSeting.SysPwdGuestNum--; 
			(void)SystemWriteSeting( &SystemSeting.SysPwdGuestNum, sizeof SystemSeting.SysPwdGuestNum );   	
		}
		
		if( SystemSeting.SysPwdAllNum > 0)
			SystemSeting.SysPwdAllNum--; 
		(void)SystemWriteSeting( &SystemSeting.SysPwdAllNum, sizeof SystemSeting.SysPwdAllNum );   
		
		retval = 1;
	}

	return retval; 	
}

/*********************************************************************************************************************
* Function Name :  App_PWD_ExchangePwdMegIntoEeprom
* Description   :  修改密码的属性
* Para          :    ppwdmeg - 待修改密码的信息指针
* Return        :  0-修改成功  1-修改失败 
*********************************************************************************************************************/
uint8_t App_PWD_ExchangePwdMegIntoEeprom( PwdMeg_T *ppwdmeg )
{
	my_printf( "App_PWD_ExchangePwdMegIntoEeprom()\n" );   
	uint32_t addr =0;
	PwdMeg_T pwdMeg={0};

	uint8_t ret = App_PWD_QueryByIdFromEeprom( ppwdmeg->UserId, &addr, &pwdMeg );   
	if( ret == 1 )  //找到了
	{
		for( uint8_t i = 0; i<MSG_PWD_BYTE_SIZE; i++)
		{
			ppwdmeg->Password[i] ^= AppBleType.ChannelPwd[i%4];
		}
        memcpy( (uint8_t *)(&pwdMeg), (uint8_t *)ppwdmeg, MSG_PWD_ONE_VALUE_SIZE );
		uint8_t tp1 =0;
		for(uint8_t i = 0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE ) )  
			{
				tp1 = 1;
				break;
			}	
		}
		if( tp1 == 0 )
		    return 0;
		
		return 1;
	}
	else
	{
		return 0;
	}	
}

#if 0
/*********************************************************************************************************************
* Function Name :  App_PWD_GetAllPwdMegFromEeprom
* Description   :  获取EEPROM中所有的密码信息
* Para          :  ptab - 读取后的密码信息   psize- 用户的数量
* Return        :  none
*********************************************************************************************************************/
static void App_PWD_GetAllPwdMegFromEeprom( UserMegTab_T *ptab, uint8_t *psize )
{
	uint8_t  userNum = 0;
    uint16_t pageid;
    uint32_t addr;
	PwdMeg_T pwdMeg = {0};
 
    for(pageid = 0; pageid < MSG_PWD_ALL_NUM; pageid++)//找指纹
    {
		addr = App_PWD_ChangePwdPageidToEepromAddr(pageid);
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), 1 ) )  
				break;
		}

        if( pwdMeg.UserValue != MEM_PWD_VALID_FLG )
        {
            continue;  
        }
		
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&pwdMeg), MSG_PWD_ONE_VALUE_SIZE ) )  
				break;
		}
		ptab->UserKind = 0;
		ptab->UserId   = pwdMeg.UserId;
		ptab->StartTim = pwdMeg.StartTim;
		ptab->StopTim  = pwdMeg.StopTim;
		ptab->Weekday  = pwdMeg.Weekday;

		if( pwdMeg.Privileges == PWD_LIMIT_ADMIN )
		{
			ptab->Limitkind = 0;
		}
		else if( pwdMeg.Privileges == PWD_LIMIT_GUEST )
		{
			ptab->Limitkind = 0;
		}
		memset( (uint8_t *)(&pwdMeg), 0, MSG_PWD_ONE_VALUE_SIZE );
		ptab++;
		userNum++;
		*psize = userNum;	
    }
		 
}
#endif

/*********************************************************************************************************************
* Function Name :  App_PWD_ClearAllPwdMegFromEeprom
* Description   :  清空EEPROM中的所有密码  临时密码除外
* Para          :  none
* Return        :  none
*********************************************************************************************************************/
void App_PWD_ClearAllPwdMegFromEeprom( void )
{
	uint8_t cleanbuf[ MSG_PWD_ONE_VALUE_SIZE ] = {0};
	
	for(uint16_t pageid=0; pageid<MSG_PWD_ALL_NUM; pageid++)//找指纹
	{
		uint32_t addr = App_PWD_ChangePwdPageidToEepromAddr(pageid);
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, cleanbuf, MSG_PWD_ONE_VALUE_SIZE ) )  
			{
				break;
			}	
		}
	}
	SystemSeting.SysPwdAdminNum = 0; 
	(void)SystemWriteSeting( &SystemSeting.SysPwdAdminNum, sizeof SystemSeting.SysPwdAdminNum );  
	SystemSeting.SysPwdGuestNum = 0; 
	(void)SystemWriteSeting( &SystemSeting.SysPwdGuestNum, sizeof SystemSeting.SysPwdGuestNum );  
	SystemSeting.SysPwdAllNum = 0; 
	(void)SystemWriteSeting( &SystemSeting.SysPwdAllNum, sizeof SystemSeting.SysPwdAllNum );  

}

/*********************************************************************************************************************
* Function Name :  App_PWD_CheckTimeliness
* Description   :  验证时效性接口
* Para Input    :  curDate- 当前时间戳  startDate-生效时间戳  stopDate-失效时间戳  weekWork-生效周   BCD码  b0--b6：周1-周日
* Para Output   :  none
* Return        :  验证结果  0= 验证失败  1= 验证成功
*********************************************************************************************************************/
static uint8_t App_PWD_CheckTimeliness( uint32_t curDate, uint32_t startDate, uint32_t stopDate, uint8_t weekWork )
{
	my_printf("App_PWD_CheckTimeliness()\n");
	my_printf("/*---------------------time check start------------------------*/\r\n");
	RTCType dateClock = {0};
 /*-------------获取当前时间---------------*/
	uint32_t currentUtc;
	HAL_RTC_TimeToTm( curDate, &dateClock );  //将时间戳转换成时间
	uint8_t weekday = dateClock.week >> 1;
	uint32_t currentTims = dateClock.hour * 3600 + dateClock.minuter * 60 + dateClock.second;
	dateClock.hour = 0;
	dateClock.minuter = 0;
	dateClock.second = 0;
	currentUtc = HAL_RTC_TmToTime( &dateClock ); //获取当前日期时间戳
	
 /*-------------获取开始时间---------------*/
	uint32_t startUtc;
	HAL_RTC_TimeToTm( startDate, &dateClock );  //将时间戳转换成时间
	uint32_t startTims = dateClock.hour * 3600 + dateClock.minuter * 60 + dateClock.second;
	dateClock.hour = 0;
	dateClock.minuter = 0;
	dateClock.second = 0;
	startUtc = HAL_RTC_TmToTime( &dateClock ); //获取当前日期时间戳
	
 /*-------------获取结束时间---------------*/
	uint32_t stopUtc;
	HAL_RTC_TimeToTm( stopDate, &dateClock );
	uint32_t stopTims = dateClock.hour * 3600 + dateClock.minuter * 60 + dateClock.second;
	dateClock.hour = 0;
	dateClock.minuter = 0;
	dateClock.second = 0;
	stopUtc = HAL_RTC_TmToTime( &dateClock );
	
 /*-------------检验时效性-----------------*/
    my_printf("/*---------------------time check stop------------------------*/\r\n");
	if( 0 == (weekday & weekWork) )
	{
		 my_printf("key check error by weekday!\n");
		 return 0;
	}
	else if( (currentUtc < startUtc) || (currentUtc > stopUtc) )  //时效不对
	{
		 my_printf("key check error by date!\n");
		 return 0;
	} 
	else if( (currentTims < startTims) || (currentTims > stopTims) )  //时间不对
	{
		 my_printf("key check error by time!\n");
		 return 0;
	} 
    my_printf("key check all right!\n");
	return 1;	
}

/*********************************************************************************************************************
* Function Name :  App_PWD_VerifyUserPwd
* Description   :  验证用户密码
* Para Input    :  userType- 用户权限  pdata- 待匹配的密码数组
* Para Output   :  ppwdmeg- 匹配的密码信息
* Return        :  验证结果  -1= 验证失败  1= 验证成功
*********************************************************************************************************************/
int8_t App_PWD_VerifyUserPwd( uint8_t userType, PwdMeg_T *ppwdmeg, char *pdata )
{
	if( 1 == App_PWD_QueryPwdByStringFromEeprom( userType, ppwdmeg, pdata ) )  //匹配到了密码
	{
		if( ppwdmeg->Timeliness == 1 )  //时效性判断
		{
			uint8_t tp1 = App_PWD_CheckTimeliness( Rtc_Real_Time.timestamp, ppwdmeg->StartTim, ppwdmeg->StopTim, ppwdmeg->Weekday );
            if( tp1 == 1 )  //有效
			{
				return 1;
			}
		}
		else 
		{
			return 1;
		}
	}
 
	return -1;
}
 
/*********************************************************************************************************************
* Function Name :  App_PWD_ChgTempPwdPageidToEepromAddr
* Description   :  获取临时密码存储与EEPROM中的地址
* Para          :  pageid - 存储的页码
* Return        :  对应eeprom中的地址
*********************************************************************************************************************/
static uint32_t App_PWD_ChgTempPwdPageidToEepromAddr( uint16_t pageid )   
{
    uint32_t addr;
    addr = MEM_TMP_PWD_START_ADDR + pageid * MSG_TMP_PWD_MEG_SIZE;
    return addr;
}
/*********************************************************************************************************************
* Function Name :  App_PWD_SaveTempPwdsIntoEeprom
* Description   :  保存临时密码于EEPROM中
* Para          :  pTmPwdmeg - 临时密码信息指针  
* Return        :  none
*********************************************************************************************************************/
void App_PWD_SaveTempPwdsIntoEeprom( TmpPwdMeg_T *pTmPwdmeg )
{
	my_printf( "App_PWD_SaveTempPwdsIntoEeprom()\n" );   
	
	for(uint8_t i=0; i<TEMP_PWD_ALL_NUM; i++)
	{
		(pTmPwdmeg+i)->UserValue = MEM_PWD_VALID_FLG;
		for(uint8_t j=0; j<TEMP_PWD_BYTE_SIZE; j++)
	    {	
		   (pTmPwdmeg+i)->Password[j] ^= AppBleType.ChannelPwd[j%4];
		}
		(pTmPwdmeg+i)->Password[TEMP_PWD_BYTE_SIZE] = '\0';
	}
	
	for(uint16_t pageid=0; pageid<TEMP_PWD_ALL_NUM; pageid++)
	{
		uint32_t addr = App_PWD_ChgTempPwdPageidToEepromAddr( pageid );
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, (uint8_t *)(&pTmPwdmeg[pageid]), MSG_TMP_PWD_VALUE_SIZE ) )  
				break;
		}
	}
}

/*********************************************************************************************************************
* Function Name :  App_PWD_CreateTempPwds
* Description   :  创建临时密码并保存于EEPROM中
* Para          :  pTmPwdmeg - 临时密码信息指针  
* Return        :  none
*********************************************************************************************************************/
void App_PWD_CreateTempPwds( TmpPwdMeg_T *pTmPwdmeg )
{
	my_printf( "App_PWD_CreateTempPwds()\n" );   
	
	RTC_TimeUpdate(RTC_TIME_CLOCK_BCD); //获取最新时间
	uint8_t rtc_tmp = Math_Bcd2Bin(Rtc_Real_Time.second);
 
	for(uint8_t row_index=0; row_index<TEMP_PWD_ALL_NUM; row_index++) // 10组 秘钥
	{
		(pTmPwdmeg+row_index)->UserValue = MEM_PWD_VALID_FLG;
		(pTmPwdmeg+row_index)->Password[0] = ((rtc_tmp | row_index) + 5) % 10 + 0x30;
		(pTmPwdmeg+row_index)->Password[1] = 0x39 - row_index;
		rtc_tmp = rtc_tmp + 0x03;
		(pTmPwdmeg+row_index)->Password[2] = (Math_Bcd2Bin(Rtc_Real_Time.minuter) * row_index + rtc_tmp) % 10 + 0x30;
		rtc_tmp = rtc_tmp * row_index % 10;
		(pTmPwdmeg+row_index)->Password[3] = rtc_tmp % 10 + 0x30;
		rtc_tmp = rtc_tmp + Math_Bcd2Bin(Rtc_Real_Time.minuter);
		(pTmPwdmeg+row_index)->Password[4] = ((rtc_tmp | row_index) + 7) % 10 + 0x30;
		rtc_tmp = rtc_tmp + row_index;
		(pTmPwdmeg+row_index)->Password[5] = (rtc_tmp ^ row_index + 4) % 10 + 0x30;
		rtc_tmp = rtc_tmp + Math_Bcd2Bin(Rtc_Real_Time.hour);
	}
}

/*********************************************************************************************************************
* Function Name :  App_PWD_QueryPwdByStringFromEeprom
* Description   :  通过输入的密码串，查询EEPROM中是否有对应的密码    用完即删除
* Para Input    :  pdata-待验证的虚位密码指针
* Return        :  0-未找到  1- 已找到
*********************************************************************************************************************/
static uint8_t App_PWD_QueryTmpPwdFromEeprom( char *pdata )
{
	my_printf( "App_PWD_QueryTmpPwdFromEeprom()\n" );   
	uint8_t   retval = 0;
    TmpPwdMeg_T tmPwdmeg = {0};
	uint8_t cleanbuf[ MSG_TMP_PWD_VALUE_SIZE ] = {0};

	for(uint16_t pageid=0; pageid<TEMP_PWD_ALL_NUM; pageid++)
	{
		uint32_t addr = App_PWD_ChgTempPwdPageidToEepromAddr( pageid );
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&tmPwdmeg), 1 ) )  
				break;
		}
		
		if( tmPwdmeg.UserValue == MEM_PWD_VALID_FLG )//存在标志 
		{
			for(uint8_t i=0; i<3; i++)
			{
				if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&tmPwdmeg), MSG_TMP_PWD_VALUE_SIZE ) )  
					break;
			}
			for(uint8_t i=0; i<TEMP_PWD_BYTE_SIZE; i++)
			{
				tmPwdmeg.Password[i]^=AppBleType.ChannelPwd[i%4];
			}
			if( strstr( pdata, (char*)tmPwdmeg.Password ) != NULL )  //比对成功
			{
				(void)HAL_EEPROM_WriteBytes( addr, cleanbuf, MSG_TMP_PWD_VALUE_SIZE );
				retval = 1;
				break;
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	return retval;
}
 
/*********************************************************************************************************************
* Function Name :  App_PWD_ClearAllTmpPwdsFromEeprom
* Description   :  清空EEPROM中的所有临时密码   
* Para          :  none
* Return        :  none
*********************************************************************************************************************/
void App_PWD_ClearAllTmpPwdsFromEeprom( void )
{
	uint8_t cleanbuf[ MSG_TMP_PWD_VALUE_SIZE ] = {0};
	
	for(uint16_t pageid = 0; pageid < TEMP_PWD_ALL_NUM; pageid++)//找指纹
	{
		uint32_t addr = App_PWD_ChgTempPwdPageidToEepromAddr( pageid );
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, cleanbuf, MSG_TMP_PWD_VALUE_SIZE ) )  
			{
				break;
			}	
		}
	}
}

/*********************************************************************************************************************
* Function Name :  App_PWD_VerifyTempPwd
* Description   :  验证临时密码密码
* Para          :  none
* Return        :  验证结果  -1= 验证失败  1= 验证成功
*********************************************************************************************************************/
int8_t App_PWD_VerifyTempPwd( char *pdata )
{
	int8_t ret = -1;
    if( 1 == App_PWD_QueryTmpPwdFromEeprom(pdata) )
	{
	    ret = 1;
	}
 
	return ret;
}

/*********************************************************************************************************************
* Function Name :  App_PWD_ChgSosPwdPageidToEepromAddr
* Description   :  获取报警密码存储与EEPROM中的地址
* Para          :  pageid - 存储的页码
* Return        :  对应eeprom中的地址
*********************************************************************************************************************/
static uint32_t App_PWD_ChgSosPwdPageidToEepromAddr( uint16_t pageid )   
{
    uint32_t addr;
    addr = MEM_SOS_PWD_START_ADDR + pageid * MSG_SOS_PWD_MEG_SIZE;
    return addr;
}

/*********************************************************************************************************************
* Function Name :  App_PWD_SaveSosPwdIntoEeprom
* Description   :  保存报警密码于EEPROM中
* Para          :  pSosPwdmeg - 报警密码信息指针  
* Return        :  none
*********************************************************************************************************************/
void App_PWD_SaveSosPwdIntoEeprom( SosPwdMeg_T *pSosPwdmeg )
{
	my_printf( "App_PWD_SaveSosPwdIntoEeprom()\n" );   

	for(uint8_t i = 0; i < SOS_PWD_ALL_NUM; i++)
	{
		(pSosPwdmeg+i)->UserValue = MEM_PWD_VALID_FLG;
		for(uint8_t j=0; j<SOS_PWD_BYTE_SIZE; j++)
	    {	
		   (pSosPwdmeg+i)->Password[j] ^= AppBleType.ChannelPwd[j%4];
		}
		(pSosPwdmeg+i)->Password[SOS_PWD_BYTE_SIZE] = '\0';
	} 
	
	for(uint16_t pageid = 0; pageid < SOS_PWD_ALL_NUM; pageid++)
	{
		uint32_t addr = App_PWD_ChgSosPwdPageidToEepromAddr( pageid );
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, (uint8_t *)(pSosPwdmeg+pageid), MSG_SOS_PWD_VALUE_SIZE ) )  
				break;
		}
	}
}

/*********************************************************************************************************************
* Function Name :  App_PWD_QueryPwdByStringFromEeprom
* Description   :  通过输入的密码串，查询EEPROM中是否有对应的密码    用完即删除
* Para Input    :  pdata-待验证的虚位密码指针
* Return        :  0-未找到  1- 已找到
*********************************************************************************************************************/
static uint8_t App_PWD_QuerySosPwdFromEeprom( char *pdata )
{
	my_printf( "App_PWD_QuerySosPwdFromEeprom()\n" );   
	uint8_t   retval = 0;
    SosPwdMeg_T sosPwdmeg = {0};

	for(uint16_t pageid=0; pageid<SOS_PWD_ALL_NUM; pageid++)
	{
		uint32_t addr = App_PWD_ChgSosPwdPageidToEepromAddr( pageid );
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&sosPwdmeg), 1 ) )  
				break;
		}
		
		if( sosPwdmeg.UserValue == MEM_PWD_VALID_FLG )//存在标志 
		{
			for(uint8_t i=0; i<3; i++)
			{
				if( 1 == HAL_EEPROM_ReadBytes( addr, (uint8_t *)(&sosPwdmeg), MSG_SOS_PWD_VALUE_SIZE ) )  
					break;
			}
			for(uint8_t i=0; i<SOS_PWD_BYTE_SIZE; i++)
			{
				sosPwdmeg.Password[i]^=AppBleType.ChannelPwd[i%4];
			}
			if( strstr( pdata, (char*)sosPwdmeg.Password ) != NULL )  //比对成功
			{
				retval = 1;
				break;
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	return retval;
}

/*********************************************************************************************************************
* Function Name :  App_PWD_ClearAllSosPwdsFromEeprom
* Description   :  清空EEPROM中的所有报警密码   
* Para          :  none
* Return        :  none
*********************************************************************************************************************/
void App_PWD_ClearAllSosPwdsFromEeprom( void )
{
	uint8_t cleanbuf[ MSG_SOS_PWD_MEG_SIZE ] = {0};
	
	for(uint16_t pageid=0; pageid<SOS_PWD_ALL_NUM; pageid++)//找指纹
	{
		uint32_t addr = App_PWD_ChgSosPwdPageidToEepromAddr( pageid );
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes( addr, cleanbuf, MSG_SOS_PWD_VALUE_SIZE ) )  
			{
				break;
			}	
		}
	}
}

/*********************************************************************************************************************
* Function Name :  App_PWD_VerifySosPwd
* Description   :  验证报警密码
* Para          :  none
* Return        :  验证结果  -1= 验证失败  1= 验证成功
*********************************************************************************************************************/
int8_t App_PWD_VerifySosPwd( char *pdata )
{
	int8_t ret = -1;
    if( 1 == App_PWD_QuerySosPwdFromEeprom(pdata) )
	{
	    ret = 1;
	}
 
	return ret;
}

 
 
 
 
 
 
/*-------------------------------------------------THE FILE END-----------------------------------------------------*/
