/********************************************************************************************************************
 * @file:        APP_ID2.c
 * @author:      dengyehao
 * @version:     V0.1
 * @date:        2021-09-13
 * @Description: 手机蓝牙交互ID2相关流程
 * @ChangeList:  初版
*********************************************************************************************************************/
  
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "string.h"
#include "DRV_GPIO.h"
#include "APP_BLE.h"
#include "Encrypto.h"
#include "APP_ID2.h"
#include "APP_BLE.h"
#include "id2_client.h"

/*-------------------------------------------------宏定义-----------------------------------------------------------*/         
#define ID2_POW_PIN_INIT        DRV_GpioOut1(M_SE_POW_GPIO_PIN)
#define ID2_POW_ON       		DRV_GpioOut0(M_SE_POW_GPIO_PIN)
#define ID2_POW_OFF             DRV_GpioOut1(M_SE_POW_GPIO_PIN)



/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/

/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         


/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
APP_ID2_DATA   App_Id2_Data;
/*-------------------------------------------------函数声明---------------------------------------------------------*/
 

/*-------------------------------------------------函数定义---------------------------------------------------------*/
/***************************************************************************************
**函数名:       APP_ID2Init
**功能描述:     
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void APP_ID2Init(void)
{
	ID2_POW_PIN_INIT;
	PUBLIC_Delayms(2);
	ID2_POW_ON;
	PUBLIC_Delayms(2);
	id2_client_init();
	memset(&App_Id2_Data,0,sizeof(App_Id2_Data));
}

/***************************************************************************************
**函数名:       APP_ID2HWSleep
**功能描述:     断电休眠
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void APP_ID2HWSleep(void)
{
	ID2_POW_PIN_INIT;
	ID2_POW_OFF;
	memset(&App_Id2_Data,0,sizeof(App_Id2_Data));
}


/***************************************************************************************
**函数名:       APP_BleID2Pro
**功能描述:     手指模组蓝牙数据处理
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void BleID2CasePro(void)
{
   if(BLE_SE_VERIFY_TOKEN == AppBleType.RxCdm)
    {
        int ret = 0;
        uint32_t dec_len = 256;
        uint8_t dec_out[256] = {0};
        uint8_t dec_in[256] = {0};
        uint32_t dec_in_len = 0;
        dec_len = AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL];
        Encrypto_my_base64(&AppBleType.RxDataBuf[L_DATA], dec_len, dec_in,(int*)&dec_in_len);
        ret = id2_client_decrypt(dec_in, dec_in_len, dec_out, &dec_len);
		if(ret==0)
        {
			my_printf("id2_client_decrypt  = %s\n", dec_out);
			AppBleType.Respond.TxLen=1;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		}
        else
        {
			my_printf("id2_client_decrypt is ret= %d\n", ret);
            //token invalid,get ID2
            uint32_t len = ID2_ID_LEN;
            uint8_t id2[ID2_ID_LEN+1 ] = {0};
            ret = id2_client_get_id(id2, &len);//获取ID2
			PUBLIC_PrintHex("id2=",id2,len);	
            AppBleType.Respond.TxLen=25;
			AppBleType.Respond.TxDataBuf[L_DATA]=1;//成功
            memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1], id2, ID2_ID_LEN);//复制ID2
        }
		AppBleRespondData(BLE_SE_VERIFY_TOKEN);//回复
		AppBleType.RxCdm=0; //直接结束		
    }
    //服务器下发CHALLENGE
	if(BLE_SE_CHALLENGE == AppBleType.RxCdm)
    {

        int ret = -1;
        uint32_t len = 256;
        uint8_t auth_code[256] = {0};
        char challenge[256] = {0}; /* get from ID2 server! */
		len = AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL];
        memcpy(challenge,&AppBleType.RxDataBuf[L_DATA], len);//复制challenge
        len=256;
        /* test without extra data */
        ret = id2_client_get_challenge_auth_code((const char *)challenge, NULL, 0, auth_code, &len);
        if (ret != 0)
        {
            AppBleType.Respond.TxLen=0;
			AppBleType.Respond.TxDataBuf[L_DATA]=1;//失败
        }
		else
		{
			AppBleType.Respond.TxLen = 1+len;
			AppBleType.Respond.TxDataBuf[L_DATA] = 0x0; //成功
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1], auth_code, len);
		}
		AppBleRespondData(BLE_SE_CHALLENGE);//回复
		AppBleType.RxCdm=0; //直接结束		
    }
    //设置SE开启或者禁用
	if(BLE_SE_SETMODE == AppBleType.RxCdm)	
    {
		AppBleType.Respond.TxLen=1;
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		AppBleRespondData(BLE_SE_SETMODE);//回复
		AppBleType.RxCdm=0; //直接结束	
    }
	else if(BLE_SE_GET_ID2== AppBleType.RxCdm)	
	{
		uint32_t len = ID2_ID_LEN;
		uint8_t id2[ID2_ID_LEN] = {0};
		(void)id2_client_get_id(id2, &len);//获取ID2
		PUBLIC_PrintHex("id2=",id2,len);	
		AppBleType.Respond.TxLen=ID2_ID_LEN+1;
		AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功
		memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1], id2, ID2_ID_LEN);//复制ID2	
		AppBleRespondData(BLE_SE_GET_ID2);//回复
		AppBleType.RxCdm=0; //直接结束		
	}
	else if(BLE_SE_ID2_TOKEN_NEW== AppBleType.RxCdm)	
	{
        uint32_t dec_len = 256;
        uint8_t dec_out[256] = {0};
        uint8_t dec_in[256] = {0};
        uint32_t dec_in_len = 0;
        dec_len = (AppBleType.RxDataBuf[L_LENH]<<8 |AppBleType.RxDataBuf[L_LENL])-16;
        Encrypto_my_base64(&AppBleType.RxDataBuf[L_DATA+16], dec_len, dec_in,(int*)&dec_in_len); //数据=R1+TOKEN
        if(id2_client_decrypt(dec_in, dec_in_len, dec_out, &dec_len) == 0)
        {
            my_printf("id2_client_decrypt  = %s\n", dec_out); //MAC_时间戳，补零后32字节
			// 0> id2_client_decrypt = EE:46:F5:35:DE:75_1631517892
			memcpy(&App_Id2_Data.R1,&AppBleType.RxDataBuf[L_DATA],16); //拷贝R1
			PUBLIC_PrintHex("App_Id2_Data.R1 =", App_Id2_Data.R1,16);
			uint8_t temp[32]={0};
			memcpy(&temp,&dec_out,strlen((char *)dec_out)); //拷贝原文，长度28字节，
			PUBLIC_PrintHex("temp = ", (uint8_t *)temp,strlen((char *)dec_out));
			for(uint8_t i=0;i<16;i++)  //异或R1
			{
				temp[i]^=App_Id2_Data.R1[i];
				temp[i+16]^=App_Id2_Data.R1[i]; 
			}
			PUBLIC_PrintHex("temp ^ R1 =", temp,strlen((char *)dec_out));
			uint32_t  hash_len=32;
			Encrypto_my_sha256(temp,strlen((char *)dec_out), temp, &hash_len); //哈希256
			PUBLIC_PrintHex("sha256 =", temp,32);
			for(uint8_t i=0;i<16;i++)
			{
				App_Id2_Data.AESKEY[i]=temp[i*2] ;  //抽取偶数项
			}
			PUBLIC_PrintHex("App_Id2_Data.AESKEY =", App_Id2_Data.AESKEY,16);
			Encrypto_my_aes(ENCRYPTION, &AppBleType.RxDataBuf[L_DATA], 16, App_Id2_Data.AESKEY); //加密R1
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1],&AppBleType.RxDataBuf[L_DATA],16); //拷贝发送
			PUBLIC_GenerateRandVec(App_Id2_Data.R2,16);//随机数生成
			PUBLIC_PrintHex("App_Id2_Data.R2 =", App_Id2_Data.R2,16);
			memcpy(&AppBleType.Respond.TxDataBuf[L_DATA+1+16],App_Id2_Data.R2,16);
			
			AppBleType.Respond.TxLen=1+16+16;
			AppBleType.Respond.TxDataBuf[L_DATA]=0;//成功		
		}
        else //失败
        {
			AppBleType.Respond.TxLen = 1;
			AppBleType.Respond.TxDataBuf[L_DATA] = 0x1; //失败
        }
		AppBleRespondData(BLE_SE_ID2_TOKEN_NEW);//回复
		AppBleType.RxCdm=0; //直接结束		
	}
	else if(BLE_SE_ID2_BOTHWAY_VERIFY== AppBleType.RxCdm)	
	{
		Encrypto_my_aes(DECRYPTION, &AppBleType.RxDataBuf[L_DATA], 16, App_Id2_Data.AESKEY); //解密R2
		PUBLIC_PrintHex("App_Id2_Data.R2 =", App_Id2_Data.R2,16);
		for(uint8_t i=0;i<16;i++)
		{
			if(AppBleType.RxDataBuf[L_DATA+i]==App_Id2_Data.R2[i])
			{
				if(i==15)
				{
					AppBleType.Respond.TxLen = 1;
					AppBleType.Respond.TxDataBuf[L_DATA] = 0x0; //成功
					AppBleRespondData(BLE_SE_ID2_BOTHWAY_VERIFY);//回复
					AppBleType.RxCdm=0; //直接结束	
					App_Id2_Data.ID2_EN=1; //加密使能
					return;
				}		
			}
		}
		AppBleType.Respond.TxLen = 1;
		AppBleType.Respond.TxDataBuf[L_DATA] = 0x1; //失败
		AppBleRespondData(BLE_SE_ID2_BOTHWAY_VERIFY);//回复
		AppBleType.RxCdm=0; //直接结束	
		my_printf("-----------------ID2 OVER-----------------------\n");
	}
}
/***************************************************************************************
**函数名:       APP_ID2Check
**功能描述:     
**输入参数:     
**输出参数:     ret 错误码
**备注:         返回值  成功 0 ; 失败 -1
****************************************************************************************/
uint8_t APP_ID2Check(void)
{
	int ret=0;
	ID2_POW_PIN_INIT;
	PUBLIC_Delayms(2);
	ID2_POW_ON;
	PUBLIC_Delayms(2);
	ret=id2_client_init();
	ID2_POW_PIN_INIT;
	ID2_POW_OFF;
	memset(&App_Id2_Data,0,sizeof(App_Id2_Data));
    return ret;
}





//.end of the file.
