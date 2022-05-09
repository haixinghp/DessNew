/**************************************************************************** 
* Copyright (C), 2008-2021,��ʩ�����磨�й������޹�˾ 
* �ļ���: System.c 
* ���ߣ���ҵ��
* �汾��V01
* ʱ�䣺20210729
* ���ݼ�����ϵͳ�洢����
****************************************************************************/
/* ��׼ͷ�ļ� */


/* �ڲ�ͷ�ļ� */
#include "System.h"
#include "Public.h"
#include "APP_BLE.h" 
#include "LockConfig.h"
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#include "APP_Finger.h"

uint8_t OpenDoorTimeCnt = 0; 
uint8_t LockConfigMode = 0; 

SYSTEM_SETTING SystemSeting ; //ϵͳ�����ṹ��
SYSTEM_FIX_SETTING SystemFixSeting;  //ϵͳ�̻��������ָ����������


/*********************************************************************************************************************
* Function Name :  SystemCfgVersionUpdate
* Description   :  ���ð汾���ݽӿ�
* Para          :  bool isForce �Ƿ�ǿ�ƻָ����ָ�������ʱ����true , ������������flase
* Return        :  0ʧ��  1�ɹ�
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

    /* дEEPROM */
    uint8_t tp1 = 0;
    for(uint8_t i=0; i<3; i++)
    {
        if( 1 == HAL_EEPROM_WriteBytes(MEM_FACT_START,(uint8_t *)(&SystemSeting),sizeof(SystemSeting)) )  //д��ɹ�
        {
            tp1 = 1;
            break;
        }
    }
    if( tp1 == 0 )  //д��ʧ��
        return 0;
    
    return 1;
}


 /*********************************************************************************************************************
* Function Name :  SystemInitFlash
* Description   :  �ָ��������ã�������м�¼  ���ع���Ա��ע��
* Para          :  ��
* Return        :  0ʧ��  1�ɹ�
*********************************************************************************************************************/
uint8_t SystemInitFlash(void)  
{
	uint8_t clear[MEM_FACT_SIZE];
	memset(clear,0,sizeof(clear));
	HAL_EEPROM_WriteBytes(0,clear,sizeof(clear));//�洢ȫ��д0
	
    uint8_t aeskey[SYSTEM_AESKEY_LEN] = {0};
    memcpy((void*)aeskey, (void*)&SystemSeting.SysFingerAsekey, SYSTEM_AESKEY_LEN);

	//�޸ĳ������ã���Ҫдֵ�����´���������Ĭ��0
	memset(&SystemSeting,0,sizeof(SystemSeting));//�ṹ������
    
    // д����ʱ����Ҫ����������
    memcpy((void*)&SystemSeting.SysFingerAsekey, (void*)aeskey, SYSTEM_AESKEY_LEN);
    
    SystemSeting.SysFactStartFig = MEM_FACT_MEM_FIG;           //0   //����������ʼ��־  G
 
	OpenKind_U openKind ={0};
	openKind.bit.PwdMode = FUNCTION_ENABLE;            //���������ʽ     0: ��֧��  1:֧��
	#ifdef FINGER_FUNCTION_ON
	openKind.bit.FingerMode = FUNCTION_ENABLE;         //ָ�ƽ�����ʽ     0: ��֧��  1:֧��
	#endif
	#ifdef SMART_KEY_FUNCTION_ON
	openKind.bit.SmartKeyMode = FUNCTION_DISABLE;      //����Կ�׽���������Կ��/�����ֻ���     0: ��֧��  1:֧��
	#endif
	#ifdef FACE_FUNCTION_ON
	openKind.bit.FaceMode = FUNCTION_ENABLE;           //����������ʽ     0: ��֧��  1:֧��
	#endif
	#ifdef IC_CARD_FUNCTION_ON
	openKind.bit.IcCardMode = FUNCTION_ENABLE;         //IC��������ʽ     0: ��֧��  1:֧��
	#endif
	#ifdef FINGER_VEIN_FUNCTION_ON
	openKind.bit.FingerVeinMode  = FUNCTION_DISABLE;   //ָ����������ʽ  0: ��֧��  1:֧��
	#endif
	#ifdef IRIS_FUNCTION_ON
	openKind.bit.IrisOpenMode    = FUNCTION_ENABLE;    //��Ĥ������ʽ 	 0: ��֧��  1:֧��
	#endif
	#ifdef HW_WALLET_FUNCTION_ON
	openKind.bit.HuaWeiWalletMode = FUNCTION_DISABLE;  //��ΪǮ��������ʽ 0: ��֧��  1:֧��
	#endif
	SystemSeting.SysLockMode = openKind.data;                  //1   //����ģʽ(ָ��/��������/����Կ�׵�)
	SystemSeting.SysFingerAllNum   = 0;  	   			 	   //2   //ָ������ 
	SystemSeting.SysFingerAdminNum = 0;  	   				   //3   //���ع���Աָ������
	SystemSeting.SysFingerGuestNum = 0;  	   				   //4   //������ָͨ������
	SystemSeting.SysFaceAllNum   = 0;  	   		    		   //5   //����������
	SystemSeting.SysFaceAdminNum = 0;  	   		    		   //6   //���ع���Ա��������
	SystemSeting.SysFaceGuestNum = 0;  	   		    		   //7   //������ͨ��������
	SystemSeting.SysCardAllNum   = 0;  	   		    		   //8   //��������
	SystemSeting.SysCardAdminNum = 0;  	   		    		   //9   //���ع���Ա������
	SystemSeting.SysCardGuestNum = 0;  	   		    		   //10  //������ͨ������
	SystemSeting.SysPwdAllNum   = 0;  	   		    		   //11  //����������
	SystemSeting.SysPwdAdminNum = 0;  	   		    		   //12  //���ع���Ա��������
	SystemSeting.SysPwdGuestNum = 0;  	   		    		   //13  //������ͨ��������
	SystemSeting.SysSmartKeyNum = 0;  	   		    		   //14  //ȫ������Կ������h ��20����
	SystemSeting.SysWifiMainSw  = FUNCTION_DISABLE;  	       //15  //wifi������
	SystemSeting.SystemAdminRegister = ADMIN_NONE_REGISTERED;  //16  //�Ƿ�ע���� 
	#if defined LOCK_BODY_216_MOTOR || defined LOCK_BODY_218_MOTOR || defined LOCK_BODY_AUTO_MOTOR
	SystemSeting.DoorUnlockWarmSw  = FUNCTION_ENABLE;          //18  //��δ�ر�������  0= �رձ��� 1= ��������   
	#endif

	#if defined FACE_FUNCTION_ON || defined IRIS_FUNCTION_ON
	SystemSeting.FaceCheckEnable = 0x55;            		   //19  //������֤����  0x55= ����  0x66= �ر�
	#else
	SystemSeting.FaceCheckEnable = 0x66;            		   //19  //������֤����  0x55= ����  0x66= �ر�
	#endif
	
	#if defined LOCK_BODY_211_MOTOR  
	SystemSeting.LockBodyMode   = LOCK_BODY_212;  	   	 	   //20  //��������  0x55= 218����  0x66= 212���� 
	#elif defined LOCK_BODY_216_MOTOR
	SystemSeting.LockBodyMode   = LOCK_BODY_216;  	   	 	   //20  //��������  0x55= 218����  0x66= 212���� 
	#elif defined LOCK_BODY_218_MOTOR
	SystemSeting.LockBodyMode   = LOCK_BODY_218;  	   	 	   //20  //��������  0x55= 218����  0x66= 212���� 
	#elif defined LOCK_BODY_AUTO_MOTOR
	SystemSeting.LockBodyMode   = LOCK_BODY_212;  	   	 	   //20  //��������  0x55= 218����  0x66= 212���� 
	#endif
	LockConfigMode = SystemSeting.LockBodyMode; 
	
	SystemSeting.SysWifiLogSw   = FUNCTION_DISABLE;  	   	   //22  //wifi�����ź��ϴ�����
	SystemSeting.SysVoice = HIGH_VOICE_VOL;              	   //23  //��������Ĭ�����
	SystemSeting.SysKeyDef = FUNCTION_DISABLE;                 //24  //һ����������    Ĭ�Ϲر�
	SystemSeting.SysHumanIrDef = 0;         				   //25  //��������(����ʱ��)   0:�ر�  (1-255��)��   
	SystemSeting.SysWifiSingle = FUNCTION_DISABLE;             //26   //���嵥˫��
	SystemSeting.SysAutoLockTime = 0;         				   //27   //�Զ�����ʱ��
	SystemSeting.Sysprotect_lock = FUNCTION_DISABLE;           //28   //���ŷ�ֹ��,ȷ�ϼ�
	SystemSeting.SysFingerFac = FUNCTION_DISABLE;              //29   //ָ�Ƴ��ң�ͨ�������жϼ���
	SystemSeting.SysFaceFac   = FUNCTION_DISABLE;              //30   //�������ң�ͨ�������жϼ���
	#if defined IR_FUNCTION_ON || defined RADAR_FUNCTION_ON  
	SystemSeting.SysDrawNear   = E_SENSE_HIGH;                 //31  //�ӽ���Ӧ������  Ĭ�����
	#else 
	SystemSeting.SysDrawNear   = E_SENSE_OFF;                  
	#endif

	SystemSeting.SysCompoundOpen = DOUBLE_CHECK_SW_OFF;        //64  //��Ͽ��ſ���/˫����֤����   
	SystemSeting.SystemVersion  = EEPROM_CFG_DEFAULT;          //66  //�汾��¼(EEPROM������)
	SystemSeting.CheckErrAllCnt = 0;         			       //68  //��֤ʧ���ܼƴ�   
	SystemSeting.CheckErrPwdCnt = 0;         			       //69  //������֤ʧ�ܼƴ�   
	SystemSeting.TryForbitUtc   = 0;         			       //72-75//������Է���ʱ�� 

	#if defined FACE_FUNCTION_ON
	SystemSeting.FaceOrIrisUnlockSuccessCnt = 0;			   //76-77���������ɹ�����
	SystemSeting.FaceOrIrisUnlockFailCnt = 0;					//78-79��������ʧ�ܴ���
	#endif
	SystemSeting.FingerOrVeinUnlockSuccessCnt = 0;				//80-81ָ�ƿ����ɹ�����
	SystemSeting.FingerOrVeinUnlockFailCnt = 0;					//82-83ָ�ƿ���ʧ�ܴ���
 
//	SystemSeting.SysFactDoneFig = 0;//MEM_FACT_MEM_FIG;   		   //65  //�̶����
   	
	uint8_t tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_WriteBytes(MEM_FACT_START,(uint8_t *)(&SystemSeting),sizeof(SystemSeting)) )  //д��ɹ�
		{
			tp1 = 1;
			break;
		}
	}
	if( tp1 == 0 )  //д��ʧ��
		return 0;

	//��ȡ�����޸�ϵͳ����
	tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( HAL_EEPROM_ReadBytes(MEM_FIX_FACT_START,(uint8_t *)(&SystemFixSeting),sizeof(SystemFixSeting)) )
		{
			tp1 = 1;
			break;
		}
	}
	if( tp1 == 0 )  //��ȡʧ��
		return 0;
	
	if( MEM_FACT_MEM_FIG != SystemFixSeting.SysFixStartFig ) //д���־δ��ʼ��
	{
		memset(clear,0,sizeof(clear));
		HAL_EEPROM_WriteBytes(MEM_FIX_FACT_START,clear,sizeof(clear));//�洢ȫ��д0
		
		SystemFixSeting.SysFixStartFig = MEM_FACT_MEM_FIG;
		SystemFixSeting.MotorTorque = LOW_TORQUE;         //Ĭ�ϵ�Ť��
		SystemFixSeting.MotorDirection = RIGHT_HAND_DOOR; //Ĭ���ҿ�
		tp1 = 0;
		for(uint8_t i=0; i<3; i++)
		{
			if( 1 == HAL_EEPROM_WriteBytes(MEM_FIX_FACT_START,(uint8_t *)(&SystemFixSeting),sizeof(SystemFixSeting)) )
			{
				tp1 = 1;
				break;
			}
		}
		if( tp1 == 0 )  //��ȡʧ��
			return 0;
	}
 	
/******************ģ�����*****************************/



/******************ģ�����*****************************/
	return 1;
}


 /*********************************************************************************************************************
* Function Name :  SystemReadFlash
* Description   :  ��ȡϵͳ����
* Para          :  ��
* Return        :  0ʧ��  1�ɹ�
*********************************************************************************************************************/
uint8_t SystemReadFlash ( void ) 
{
	//ϵͳ��ַ1
	uint8_t	tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_ReadBytes(MEM_FACT_START, (uint8_t *)(&SystemSeting), sizeof(SystemSeting)) )
		{
			tp1 = 1;
			break;   //��ȡ�ɹ�
		}
	}
	if( tp1 == 0 )  //��ȡʧ��
		return 0;
 
	if( (SystemSeting.SysFactDoneFig != MEM_FACT_MEM_FIG) || (SystemSeting.SysFactStartFig != MEM_FACT_MEM_FIG) )
	{
		return 0;   //������־����
	}
	
	#ifdef LOCK_BODY_AUTO_MOTOR
	if( OpenDoorTimeCnt == 0 )
	{
	   if( SystemSeting.LockBodyMode != LOCK_BODY_212 )
	   {
		   SystemSeting.LockBodyMode = LOCK_BODY_212;
		   SystemWriteSeting(&SystemSeting.LockBodyMode,1);//д����
	   }
	}
    LockConfigMode = SystemSeting.LockBodyMode; 
	if( OpenDoorTimeCnt < 2 )  //ǰ2��ǿ�Ƹĳ�218
	{
		LockConfigMode = LOCK_BODY_218;
	}
		
	#endif
	//ϵͳ��ַ2
	tp1 = 0;
	for(uint8_t i=0; i<3; i++)
	{
		if( 1 == HAL_EEPROM_ReadBytes(MEM_FIX_FACT_START, (uint8_t *)(&SystemFixSeting), sizeof(SystemFixSeting)) )
		{
			tp1 = 1;
			break;   //��ȡ�ɹ�
		}
	}
	if( tp1 == 0 )  //��ȡʧ��
		return 0;
 
	if( SystemFixSeting.SysFixStartFig != MEM_FACT_MEM_FIG )
	{
		return 0;   //������־����
	}

	return 1;  //�ɹ�
}

 /*********************************************************************************************************************
* Function Name :  SystemReadSeting
* Description   :  ������ȡϵͳ�����ṹ���Ա
* Para          :  ��
* Return        :  0ʧ��  1�ɹ�
*********************************************************************************************************************/
uint8_t SystemReadSeting ( uint8_t *setting, uint8_t len )   
{
	uint32_t add= setting - (uint8_t *)( &SystemSeting); //�ṹ��ƫ��ֵ��ӦEE��ַ����0��ʼ
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
* Description   :  д�뵥ϵͳ����ֵ
* Para          :  *setting       ָ��SystemSeting�ṹ���Ա��ַ
                    data          ��ֵ
* Return        :  0ʧ��  1�ɹ�
* note          :  ��������У��data��Ч������ͬʱ�����Ա����
* example       �� д�����Աע��ɹ����־
				   SystemSeting.SystemAdminRegister=ADMIN_LOCAL_REGISTERED;
                   SystemWriteSeting((uint8_t *)&SystemSeting.SystemAdminRegister);
*********************************************************************************************************************/
uint8_t SystemWriteSeting( uint8_t *setting, uint8_t len )   
{
	if( len == 0 )
		return 0;
	
	if(setting == (uint8_t *)(&SystemSeting.SystemAdminRegister)) //ע���־
	{
		if((ADMIN_LOCAL_REGISTERED != SystemSeting.SystemAdminRegister)
		 &&(ADMIN_APP_REGISTERED != SystemSeting.SystemAdminRegister)) //�����Ϸ��ж�
		{
			return 0;
		}
	}
	else if(setting ==&SystemSeting.SysWifiMainSw)//WIFI����
	{
		if( (SystemSeting.SysWifiMainSw !=0) && (SystemSeting.SysWifiMainSw !=1) )
		{
			return 0;
		}
	}
	else if(setting ==SystemSeting.SysFingerAsekey)
	{

	}

	uint32_t add= MEM_FACT_START + (setting - (uint8_t *)(&SystemSeting)); //�ṹ��ƫ��ֵ��ӦEE��ַ����0��ʼ
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
* Description   :  ������ȡϵͳ�����ṹ���Ա
* Para          :  ��
* Return        :  0ʧ��  1�ɹ�
*********************************************************************************************************************/
uint8_t SystemReadFixSeting( uint8_t *setting, uint8_t len )   
{
	uint32_t add= MEM_FACT_START + (setting - (uint8_t *)( &SystemFixSeting)); //�ṹ��ƫ��ֵ��ӦEE��ַ����0��ʼ
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
* Description   :  д�뵥ϵͳ����ֵ
* Para          :  *setting       SystemFixSeting
                    len           ����
* Return        :  0ʧ��  1�ɹ�
* note          :  
*********************************************************************************************************************/
uint8_t SystemWriteFixSeting( uint8_t *setting, uint8_t len )   
{
	if( len == 0 )
		return 0;
	
	uint32_t add= MEM_FIX_FACT_START+ (setting - (uint8_t *)(&SystemFixSeting)); //�ṹ��ƫ��ֵ��ӦEE��ַ����0��ʼ
	
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
* Description   :  д�����¼���¼
* Para          :  event_type      �¼�����
                   event_id        �¼��û�ID��¼
* Return        :  ��
* note          :  �¼���¼��ز���������Ӳ���
* example       �� 
*********************************************************************************************************************/
void SystemEventLogSave( LOCK_EVENT_LOG event_type, uint16_t pageid )
{ 
	uint8_t temp[MSG_LOG_RECORD_REG_ONE_SIZE];
	memset ( temp, 0, MSG_LOG_RECORD_REG_ONE_SIZE );
	temp[2] = (uint8_t)(pageid >> 8);//�ȿ���ID����ID�����ֱ�Ӹ���
	temp[3] = (uint8_t)pageid;
	switch (event_type)
    {
    	case BAC_OPEN_IN_DOOR :     memcpy ( temp, "BCBopen ", 8 );break;//���ڰ��ֿ�
    	case BAC_CLOSE_IN_DOOR :    memcpy ( temp, "BCBclose", 8 );break;//���ڰ��ֹ�
		case KEY_OPEN_IN_DOOR :     memcpy ( temp, "KOKopen ", 8 );break;//���ڰ�����
		case KEY_CLOSE_IN_DOOR :    memcpy ( temp, "KCKclose", 8 );break;//���ڰ�����		
		case FALSE_LOCK_ALARM :     memcpy ( temp, "JLJLOCK ", 8 );break;//��������
		case CLOSE_OUT_DOOR :       memcpy ( temp, "WCWclose", 8 );break;//��������
		case AUTO_CLOSE_DOOR :      memcpy ( temp, "OCOclose", 8 );break;//�Զ�����
		case EMPTY_LOCK :           memcpy ( temp, "EEEMPTY ", 8 );break;//�������
		case TEMP_PASSWORD_OPEN :   memcpy ( temp, " KTEMP  ", 8 );break;//��ʱ���뿪��
		case TRY_OPRN_ALARM :       memcpy ( temp, " TTEST  ", 8 );break;//���Ա���
		case PICK_OPRN_ALARM :      memcpy ( temp, " AALARM ", 8 );break;//��������
		case DELETE_SOS_PASSWORD :  memcpy ( temp, "-KSOSPWD", 8 );break;//��������ɾ��
		case ADD_SOS_PASSWORD :     memcpy ( temp, "+KSOSPWD", 8 );break;//���ӱ�������
		case SOS_PASSWORD_OPEN :    memcpy ( temp, " KSOSPWD", 8 );break;//�������뿪��	
		case DELETE_PASSWORD :      memcpy ( temp, "-KPWD   ", 8 );break;//����ɾ��
		case ADD_PASSWORD :         memcpy ( temp, "+KPWD   ", 8 );break;//��������
		case PASSWORD_OPEN :        memcpy ( temp, " KPWD   ", 8 );break;//���뿪��		
		case ADD_SMART_KEY :        memcpy ( temp, "+S", 2 );break;//���ӵ���Կ��
		case DELETE_SMART_KEY :     memcpy ( temp, "-S", 2 );break;//ɾ������Կ��
		case SMART_KEY_OPEN :       memcpy ( temp, " S", 2 );
									memcpy ( &temp[2], (uint8_t*)&pageid, 4 ); //��¼ID
									break;//����Կ�׿���
		case ADD_FACE :             memcpy ( temp, "+R", 2 );break;//��������
		case DELETE_FACE :          memcpy ( temp, "-R", 2 );break;//ɾ������
        case FACE_OPEN :            memcpy ( temp, " R", 2 );break;//��������
		case ADD_BLE :              memcpy ( temp, "+P", 2 );break;//���������˺�
		case DELETE_BLE :           memcpy ( temp, "-P", 2 );break;//ɾ�������˺�
        case BLE_OPEN :             memcpy ( temp, " P", 2 );break;//�����˺ſ���
		case ADD_CARD :             memcpy ( temp, "+C", 2 );break;//���ӿ�
		case DELETE_CARD :          memcpy ( temp, "-C", 2 );break;//ɾ����
        case CARD_OPEN :            memcpy ( temp, " C", 2 );break;//������
		case ADD_FINGER :           memcpy ( temp, "+F", 2 );break;//����ָ��
		case DELETE_FINGER :        memcpy ( temp, "-F", 2 );break;//ɾ��ָ��
        case FINGER_OPEN :          memcpy ( temp, " F", 2 );break;//ָ�ƿ���
		case ADD_VEIN :            	memcpy ( temp, "+V", 2 );break;//����ָ����
		case DELETE_VEIN :          memcpy ( temp, "-V", 2 );break;//ɾ��ָ����
		case VEIN_OPEN :           	memcpy ( temp, " V", 2 );break;//ָ��������
		case ADD_IRIS :            	memcpy ( temp, "+I", 2 );break;//���Ӻ�Ĥ
		case DELETE_IRIS :          memcpy ( temp, "-I", 2 );break;//ɾ����Ĥ
		case IRIS_OPEN :           	memcpy ( temp, " I", 2 );break;//��Ĥ����
		case NOTHING_CASE :         memcpy ( temp, "AU", 2 );break;//�������޶�������Դ
		case FACE_ADMIN_CHECK :     memcpy ( temp, "AR", 2 );break;//��������˵���¼
		case FINGER_ADMIN_CHECK :   memcpy ( temp, "AF", 2 );break;//ָ�ƽ���˵���¼
		case PWD_ADMIN_CHECK :      memcpy ( temp, "AK", 2 );break;//�������˵���¼
		case VEIN_ADMIN_CHECK :     memcpy ( temp, "AV", 2 );break;//ָ��������˵���¼
        case IRIS_ADMIN_CHECK :     memcpy ( temp, "AI", 2 );break;//��Ĥ����˵���¼
    	default:
    		break;
    }
  
	RTC_TimeUpdate(RTC_TIME_CLOCK_BCD); //��ȡ����ʱ��
    temp[8] = Rtc_Real_Time.year;
    temp[9] = Rtc_Real_Time.month ;
    temp[10] = Rtc_Real_Time.day ;
    temp[11] = Rtc_Real_Time.hour ;
    temp[12] = Rtc_Real_Time.minuter ;
    temp[13] = Rtc_Real_Time.second ;	
	
//	if(event_type==EMPTY_LOCK)
//	{
//		memcpy(SystemFixSeting.SysClearCase,temp,14);
//		SystemWriteFixSeting(SystemFixSeting.SysClearCase,14); //Ӳ��մ洢λ�����֣����Ǵ���
//		return;
//	}

    uint32_t addr = MSG_LOG_RECORD_START + SystemFixSeting.SysLockLogSn * MSG_LOG_RECORD_ONE_SIZE; //����洢��ַ
	HAL_EEPROM_WriteBytes(addr,temp,sizeof(temp)); //14�ֽڼ�¼д��
	
	
	SystemFixSeting.SysLockLogSn ++; 
	if ( SystemFixSeting.SysLockLogSn >= MSG_LOG_RECORD_NUM )
    {
        SystemFixSeting.SysLockLogSn = 0;
    }
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogSn,2);//��ǰ�洢λ��
	
	
	SystemFixSeting.SysLockLogAll++;
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogAll,2);//�洢��¼�ܴ���
	my_printf( "SysLockLogSn=%d    SysLockLogAll=%d  \n", SystemFixSeting.SysLockLogSn, SystemFixSeting.SysLockLogAll);
	return;
}
 /*********************************************************************************************************************
* Function Name :  SystemEventLogClear
* Description   :  ����¼���¼
* Para          :  
* Return        :  ��
* note          :  
* example       �� 
*********************************************************************************************************************/
void SystemEventLogClear( void )
{
	uint8_t flash[MSG_LOG_RECORD_ONE_SIZE]={0};
	SystemFixSeting.SysLockLogSn =0;  
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogSn,2);//��ǰ�洢λ��
	SystemFixSeting.SysLockLogAll =0;
	SystemWriteFixSeting((uint8_t *)&SystemFixSeting.SysLockLogAll,2);//�洢��¼�ܴ���
	for(uint32_t i=0;i<MSG_LOG_RECORD_NUM;i++)
	{
		uint32_t add= MSG_LOG_RECORD_START + i * MSG_LOG_RECORD_ONE_SIZE;
		HAL_EEPROM_WriteBytes(add,flash,MSG_LOG_RECORD_ONE_SIZE);
	}
}


//.end of the file.





