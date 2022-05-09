/********************************************************************************************************************
 * @file:        APP_SmartKey.c
 * @author:      dengyehao
 * @version:     V0.1
 * @date:        2021-08-17
 * @Description: ����ң����
 * @ChangeList:  ����
*********************************************************************************************************************/

/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
#include "APP_SmartKey.h" 
#include "System.h" 
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "Public.h" 
#include "Encrypto.h"

/*-------------------------------------------------�궨��-----------------------------------------------------------*/ 
#define SMARTKEY_LOG   //��־��ӡ




/***************************************************************************************
**������:       SmartkeyPageidAddr
**��������:     ����pageid����EE�洢��ַ
**�������:     
**�������:     uint32_t��ַ
**��ע:         
****************************************************************************************/	
static uint32_t SmartkeyPageidAddr(uint16_t pageid) 
{
	return MSG_SMARTKEY_REG_START + pageid * MSG_SMARTKEY_ONE_SIZE;
}



/***************************************************************************************
**������:       SmartkeySearchEmptyReg
**��������:     �ҿյĴ洢�ռ�
**�������:     
**�������:     pageid
**��ע:         
****************************************************************************************/	
static uint32_t SmartkeySearchEmptyReg(void) //����Կ��Ѱַ
{
    uint8_t pageid = 0;
	uint8_t head = I2C_FALSE;
	for( pageid = 0; pageid < MSG_SMARTKEY_NUM; pageid++ )
    {
        uint32_t addr =  SmartkeyPageidAddr(pageid); //�洢��ַ
        HAL_EEPROM_ReadBytes(addr,&head,1);	//��ȡ���
        if( head != MEM_FACT_MEM_FIG )
        {
			return pageid; //�ҵ�pageid
        }
    }
	return pageid;
}

/***************************************************************************************
**������:       SmartkeyCheckRegId
**��������:     ����IDѰ������Կ��
**�������:     mode  ��������
				*CheakId  Կ��ID
**�������:     ������
**��ע:         APPע��ʱʹ�ü����Կ�׿���ʱʹ��AES 
****************************************************************************************/	
uint16_t SmartkeyCheckRegId (SMARTKEY_ENC_TYPE mode, const uint8_t *CheakId)   
{
    uint16_t pageid = 0;
    uint16_t u16_Check_Loop;
    uint32_t addr = 0 ;
	uint8_t flash[32];
    for(pageid = 0; pageid < MSG_SMARTKEY_NUM; pageid++) //��ѯ
    {
        addr = SmartkeyPageidAddr(pageid);
		HAL_EEPROM_ReadBytes(addr,&flash[0],1);
        if(flash[0] != MEM_FACT_MEM_FIG)
        {
           continue;  //��ַ��Ч
        }
        if(AES_ENC==mode) //AES���ܱȶ�
        {
            memset(flash,0,32); //16�ֽ���0
            memcpy(flash,&AppBleType.ChannelPwd[0],4); //�����ŵ�����
			HAL_EEPROM_ReadBytes(addr+1,&flash[4], SMARTKEY_ID_LEN);	//��ID
			//memcpy(id.phoneID,&old_data[4],13); //����
			#ifdef SMARTKEY_LOG
            PUBLIC_PrintHex("channel key + 13 ID =",flash,32);
            PUBLIC_PrintHex("RandomNum =",AppBleType.RandomNum,8);
			#endif
			
			#ifdef SMARTKEY_XRO_ENC_ON  //��֤��ɫ�ԳƼ���Կ��
			//----------------------------����֤����Կ��
			uint8_t ble_data[24];
			memcpy(ble_data,CheakId,19); //����������������
			for(uint8_t i=0;i<8;i++)//������  ��Ч����13+4
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
			
            //AES������Կ
            uint8_t temp_key[16]= {0x90,0x88,0x42,0x53,0x58,0xA9,0x78,0x1F,0x70,0x18,0x32,0x63,0x98,0xB9,0xC8,0xDF};
            for(uint8_t i=0; i<5; i++)      //����Կ�±�2/4/6/8/10λ���
            {
                temp_key[i]^=flash[4+(i+1)*2];
            }
            for(uint8_t i=0; i<8; i++)      //8�ֽ���������
            {
                temp_key[i]^=AppBleType.RandomNum[i];
                temp_key[i+8]^=AppBleType.RandomNum[i];
            }
			Encrypto_my_aes(ENCRYPTION, &flash[0], 32, temp_key); //��������		
			#ifdef SMARTKEY_LOG
            PUBLIC_PrintHex("SMARTKEY ID =",flash,32);
			#endif
            for(u16_Check_Loop = 0; u16_Check_Loop < (SMARTKEY_ID_LEN+4); u16_Check_Loop++) //�ŵ�����+ID�Ա�
            {
                if(flash[u16_Check_Loop] != CheakId[u16_Check_Loop])//�Ƚ�
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
			HAL_EEPROM_ReadBytes(addr+1,&flash[0], SMARTKEY_ID_LEN);	//��ID
            for(u16_Check_Loop = 0; u16_Check_Loop < SMARTKEY_ID_LEN; u16_Check_Loop++)//ID�Ա�
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
**������:       SmartKeyReadId
**��������:     ��ָ����ַԿ��ID
**�������:     pageid
				*id  Կ��ID
**�������:     ������
**��ע:         
****************************************************************************************/	
void SmartKeyReadId (uint16_t pageid , uint8_t *id )   
{
	uint32_t addr = SmartkeyPageidAddr(pageid);
	HAL_EEPROM_ReadBytes(addr+1,id, SMARTKEY_ID_LEN);	//��ID
}

/***************************************************************************************
**������:       SmartKeyWriteId
**��������:     ע��ָ��Կ��
**�������:     *id  Կ��ID
**�������:     ������
**��ע:         
****************************************************************************************/	
uint8_t SmartKeyWriteId (uint8_t *id )   
{
	uint8_t pageid;
	uint8_t flash[16];
	uint32_t addr;
	//�ж�����
	if(SystemSeting.SysSmartKeyNum >MSG_SMARTKEY_NUM) 
	{
		return 0;
	}	
	//�ҿյ�ַ
	pageid=SmartkeySearchEmptyReg();

    //�洢ID
	flash[0]=MEM_FACT_MEM_FIG;
	memcpy(&flash[1],id,SMARTKEY_ID_LEN);
	addr=SmartkeyPageidAddr(pageid);
	HAL_EEPROM_WriteBytes(addr,flash,14);
	
	//���´洢����
	SystemSeting.SysSmartKeyNum++;
	SystemWriteSeting(&SystemSeting.SysSmartKeyNum,1); 	
	return 1;
}


/***************************************************************************************
**������:       SmartKeyDeleteId
**��������:     ɾ��ָ��Կ��
**�������:     *id  Կ��ID
				len  ID���ȣ�Ĭ��13
**�������:     ������
**��ע:         
****************************************************************************************/	
uint8_t SmartKeyDeleteId (uint8_t *id )   
{
	uint32_t addr;
	uint16_t pageid;
	uint8_t clean;
	//�ж�����
	if(SystemSeting.SysSmartKeyNum == 0 ) 
	{
		return 0;
	}	
	//��ID
	pageid=SmartkeyCheckRegId(XRO_ENC,(const uint8_t*)id); 
	if(PAGEID_NULL==pageid) //û�ҵ�
	{
		return 0;
	}
	//�����ַ
	addr=SmartkeyPageidAddr(pageid); //�洢��ַ
	
	//��洢
	clean=0xff;
	HAL_EEPROM_WriteBytes(addr,&clean,1);
	
	//
	SystemSeting.SysSmartKeyNum--;
	SystemWriteSeting(&SystemSeting.SysSmartKeyNum,1); 	
	return 1;
}


 /*********************************************************************************************************************
* Function Name :  SmartKeyDeleteClear
* Description   :  ���
* Para          :  
* Return        :  ��
* note          :  
* example       �� 
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
