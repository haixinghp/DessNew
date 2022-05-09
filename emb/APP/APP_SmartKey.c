/********************************************************************************************************************
 * @file:        APP_SmartKey.c
 * @author:      dengyehao
 * @version:     V0.1
 * @date:        2021-08-17
 * @Description: 蓝牙遥控器
 * @ChangeList:  初版
*********************************************************************************************************************/

/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include "APP_SmartKey.h" 
#include "System.h" 
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "Public.h" 
#include "Encrypto.h"

/*-------------------------------------------------宏定义-----------------------------------------------------------*/ 
#define SMARTKEY_LOG   //日志打印




/***************************************************************************************
**函数名:       SmartkeyPageidAddr
**功能描述:     根据pageid计算EE存储地址
**输入参数:     
**输出参数:     uint32_t地址
**备注:         
****************************************************************************************/	
static uint32_t SmartkeyPageidAddr(uint16_t pageid) 
{
	return MSG_SMARTKEY_REG_START + pageid * MSG_SMARTKEY_ONE_SIZE;
}



/***************************************************************************************
**函数名:       SmartkeySearchEmptyReg
**功能描述:     找空的存储空间
**输入参数:     
**输出参数:     pageid
**备注:         
****************************************************************************************/	
static uint32_t SmartkeySearchEmptyReg(void) //蓝牙钥匙寻址
{
    uint8_t pageid = 0;
	uint8_t head = I2C_FALSE;
	for( pageid = 0; pageid < MSG_SMARTKEY_NUM; pageid++ )
    {
        uint32_t addr =  SmartkeyPageidAddr(pageid); //存储地址
        HAL_EEPROM_ReadBytes(addr,&head,1);	//读取标记
        if( head != MEM_FACT_MEM_FIG )
        {
			return pageid; //找到pageid
        }
    }
	return pageid;
}

/***************************************************************************************
**函数名:       SmartkeyCheckRegId
**功能描述:     根据ID寻找智能钥匙
**输入参数:     mode  加密类型
				*CheakId  钥匙ID
**输出参数:     处理结果
**备注:         APP注册时使用简单异或，钥匙开锁时使用AES 
****************************************************************************************/	
uint16_t SmartkeyCheckRegId (SMARTKEY_ENC_TYPE mode, const uint8_t *CheakId)   
{
    uint16_t pageid = 0;
    uint16_t u16_Check_Loop;
    uint32_t addr = 0 ;
	uint8_t flash[32];
    for(pageid = 0; pageid < MSG_SMARTKEY_NUM; pageid++) //轮询
    {
        addr = SmartkeyPageidAddr(pageid);
		HAL_EEPROM_ReadBytes(addr,&flash[0],1);
        if(flash[0] != MEM_FACT_MEM_FIG)
        {
           continue;  //地址无效
        }
        if(AES_ENC==mode) //AES加密比对
        {
            memset(flash,0,32); //16字节清0
            memcpy(flash,&AppBleType.ChannelPwd[0],4); //拷贝信道密码
			HAL_EEPROM_ReadBytes(addr+1,&flash[4], SMARTKEY_ID_LEN);	//读ID
			//memcpy(id.phoneID,&old_data[4],13); //拷贝
			#ifdef SMARTKEY_LOG
            PUBLIC_PrintHex("channel key + 13 ID =",flash,32);
            PUBLIC_PrintHex("RandomNum =",AppBleType.RandomNum,8);
			#endif
			
			#ifdef SMARTKEY_XRO_ENC_ON  //验证白色对称加密钥匙
			//----------------------------先验证蓝牙钥匙
			uint8_t ble_data[24];
			memcpy(ble_data,CheakId,19); //拷贝蓝牙加密数据
			for(uint8_t i=0;i<8;i++)//异或加密  有效部分13+4
			{
				ble_data[i]^=AppBleType.RandomNum[i];
				ble_data[i+8]^=AppBleType.RandomNum[i];
				ble_data[i+8+8]^=AppBleType.RandomNum[i];
			}
			for(u16_Check_Loop = 0; u16_Check_Loop < 17; u16_Check_Loop++)
			{
				if(flash[u16_Check_Loop] != ble_data[u16_Check_Loop])
				{
					break;
				}
				else
				{
					if ( 12 == u16_Check_Loop)
					{
						return pageid;
					}
				}
			}
			#endif
			
            //AES加密密钥
            uint8_t temp_key[16]= {0x90,0x88,0x42,0x53,0x58,0xA9,0x78,0x1F,0x70,0x18,0x32,0x63,0x98,0xB9,0xC8,0xDF};
            for(uint8_t i=0; i<5; i++)      //根秘钥下标2/4/6/8/10位异或
            {
                temp_key[i]^=flash[4+(i+1)*2];
            }
            for(uint8_t i=0; i<8; i++)      //8字节随机数异或
            {
                temp_key[i]^=AppBleType.RandomNum[i];
                temp_key[i+8]^=AppBleType.RandomNum[i];
            }
			Encrypto_my_aes(ENCRYPTION, &flash[0], 32, temp_key); //加密数据		
			#ifdef SMARTKEY_LOG
            PUBLIC_PrintHex("SMARTKEY ID =",flash,32);
			#endif
            for(u16_Check_Loop = 0; u16_Check_Loop < (SMARTKEY_ID_LEN+4); u16_Check_Loop++) //信道密码+ID对比
            {
                if(flash[u16_Check_Loop] != CheakId[u16_Check_Loop])//比较
                {
                    break;
                }
                else
                {
                    if ( (SMARTKEY_ID_LEN+4-1) == u16_Check_Loop)
                    {
                        return pageid;
                    }
                }
            }
        }
        else
        {
			HAL_EEPROM_ReadBytes(addr+1,&flash[0], SMARTKEY_ID_LEN);	//读ID
            for(u16_Check_Loop = 0; u16_Check_Loop < SMARTKEY_ID_LEN; u16_Check_Loop++)//ID对比
            {
                if(flash[u16_Check_Loop] != CheakId[u16_Check_Loop])
                {
                    break;
                }
                else
                {
                    if ( (SMARTKEY_ID_LEN-1) == u16_Check_Loop)
                    {
						return pageid;
                    }
                }
            }

        }
    }
    return PAGEID_NULL;    //fail
}

/***************************************************************************************
**函数名:       SmartKeyReadId
**功能描述:     读指定地址钥匙ID
**输入参数:     pageid
				*id  钥匙ID
**输出参数:     处理结果
**备注:         
****************************************************************************************/	
void SmartKeyReadId (uint16_t pageid , uint8_t *id )   
{
	uint32_t addr = SmartkeyPageidAddr(pageid);
	HAL_EEPROM_ReadBytes(addr+1,id, SMARTKEY_ID_LEN);	//读ID
}

/***************************************************************************************
**函数名:       SmartKeyWriteId
**功能描述:     注册指定钥匙
**输入参数:     *id  钥匙ID
**输出参数:     处理结果
**备注:         
****************************************************************************************/	
uint8_t SmartKeyWriteId (uint8_t *id )   
{
	uint8_t pageid;
	uint8_t flash[16];
	uint32_t addr;
	//判断数量
	if(SystemSeting.SysSmartKeyNum >MSG_SMARTKEY_NUM) 
	{
		return 0;
	}	
	//找空地址
	pageid=SmartkeySearchEmptyReg();

    //存储ID
	flash[0]=MEM_FACT_MEM_FIG;
	memcpy(&flash[1],id,SMARTKEY_ID_LEN);
	addr=SmartkeyPageidAddr(pageid);
	HAL_EEPROM_WriteBytes(addr,flash,14);
	
	//更新存储数量
	SystemSeting.SysSmartKeyNum++;
	SystemWriteSeting(&SystemSeting.SysSmartKeyNum,1); 	
	return 1;
}


/***************************************************************************************
**函数名:       SmartKeyDeleteId
**功能描述:     删除指定钥匙
**输入参数:     *id  钥匙ID
				len  ID长度，默认13
**输出参数:     处理结果
**备注:         
****************************************************************************************/	
uint8_t SmartKeyDeleteId (uint8_t *id )   
{
	uint32_t addr;
	uint16_t pageid;
	uint8_t clean;
	//判断数量
	if(SystemSeting.SysSmartKeyNum == 0 ) 
	{
		return 0;
	}	
	//找ID
	pageid=SmartkeyCheckRegId(XRO_ENC,(const uint8_t*)id); 
	if(PAGEID_NULL==pageid) //没找到
	{
		return 0;
	}
	//计算地址
	addr=SmartkeyPageidAddr(pageid); //存储地址
	
	//清存储
	clean=0xff;
	HAL_EEPROM_WriteBytes(addr,&clean,1);
	
	//
	SystemSeting.SysSmartKeyNum--;
	SystemWriteSeting(&SystemSeting.SysSmartKeyNum,1); 	
	return 1;
}


 /*********************************************************************************************************************
* Function Name :  SmartKeyDeleteClear
* Description   :  清空
* Para          :  
* Return        :  无
* note          :  
* example       ： 
*********************************************************************************************************************/
void SmartKeyDeleteClear( void )
{
	uint8_t flash[MSG_SMARTKEY_ONE_SIZE]={0};
	for(uint32_t i=0;i<MSG_SMARTKEY_NUM;i++)
	{
		uint32_t add= MSG_SMARTKEY_REG_START + i * MSG_SMARTKEY_ONE_SIZE;
		HAL_EEPROM_WriteBytes(add,flash,MSG_SMARTKEY_ONE_SIZE);
	}
}










//.end of the file.
