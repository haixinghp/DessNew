/********************************************************************************************************************
 * @file:      App_CpuCard.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2020-11-14
 * @brief:     CPU�����ܺ���   ����΢��
 * @changlist:  2020-11-18  ʵ�ָ���΢�����俨���鿨����
*********************************************************************************************************************/



/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
#include "..\HAL\HAL_EEPROM\HAL_EEPROM.h"
#include "LockConfig.h"
#include "Public.h"

#ifdef IC_CARD_FUNCTION_ON


#include "App_CpuCard.h"
#include "my_des.h"
#include "../DRV/DRV_NFC/DRV_MFRC522.h"
#include "../HAL/HAL_Voice/HAL_Voice.h"

#include "iso14443a.h"
#include "nz3801-ab_com.h"
#include "nz3801-ab.h"
#include "errno.h"

/*-------------------------------------------------�궨��-----------------------------------------------------------*/


/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/


/*-------------------------------------------------��������---------------------------------------------------------*/
static const uint8_t APDUSelectMF[] = {0x00,0xA4,0x00, 0x00, 0x00}; // ѡ����Ŀ¼
static const uint8_t APDUGetRandom[] = {0x00,0x84,0x00,0x00,0x04};  // ��ȡ�����
static uint8_t APDUGetExternalAuthentication[] = {0x00,0x82,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // �ⲿ��֤
static const uint8_t APDUEmptyCard[] = {0x80,0x0E,0x00,0x00,0x00};  // ��տ�Ƭ
static const uint8_t APDUCreateMFKeyFile[] = {0x80,0xE0,0x00,0x00,0x07,0x3F,0x00,0xB0,0x01,0xFA,0xFF,0xFF}; // ����MF��Կ�ļ�
static const uint8_t APDUAddMFKeyFilePwd[] = {0x80,0xD4,0x01,0x00,0x0D,0x39,0xF0,0xF0,0xAA,0x55,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; // ��Կ�ļ������ø�������8�ֽ���Կ
static const uint8_t APDUCreateDF[] = {0x80,0xE0,0x3F,0x01,0x0D,0x38,0x05,0x20,0xFA,0xFA,0x95,0xFF,0xFF,0x44,0x44,0x46,0x30,0x31}; // ����Ŀ¼ DF�ļ�
static const uint8_t APDUSelectDF[] = {0x00,0xA4,0x00,0x00,0x02,0x3F,0x01}; // ѡ��DFĿ¼
static const uint8_t APDUCreateDFKeyFile[] = {0x80,0xE0,0x00,0x00,0x07,0x3F,0x01,0x8F,0x95,0xFA,0xFF,0xFF}; // ����DF�ļ��µ���Կ�ļ�
static uint8_t APDUAddDFExternalKey[] = {0x80,0xD4,0x01,0x00,0x0D,0x39,0xF0,0xFA,0xAA,0x55,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08}; // ����ⲿ��֤��Կ 8�ֽ�
static uint8_t APDUAddDFInsideKey[] = {0x80,0xD4,0x01,0x00,0x0D,0x30,0xF0,0xF0,0xAA,0x33,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08}; 	 // ����ڲ���֤��Կ 8�ֽ�

static const uint8_t APDUCreateBin[] = {0x80,0xE0,0x00,0x03,0x07,0x28,0x00,0x1E,0xFA,0xFA,0xFF,0xFF}; // �����������ļ�
static const uint8_t APDUSelectBin[] = {0x00,0xA4,0x00,0x00,0x02,0x00,0x03}; 													// ѡ��Ҫ�����Ķ������ļ�
static uint8_t APDUWriteBin[] = {0x00,0xD6,0x00,0x00,0x06,0x01,0x02,0x03,0x04,0x05,0x6}; 							// д�����������ļ�
static const uint8_t APDUReadBin[] = {0x00,0xB0,0x00,0x00,0x00,0x00}; 																// ��ȡ�������ļ�
static uint8_t APDUGetInsideAuthentication[] = {0x00,0x88,0x00,0x00,0x08,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88}; // �ڲ���֤8�ֽڣ������

static uint8_t MFFactorySetkey[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t DFExternalKey[ DF_EXTERN_KEY_SIZE ] = { 0 };  //�ⲿ��֤��Կ
static uint8_t DFInsideKey[ DF_INSIDE_KEY_SIZE ]   = { 0 };  //�ڲ���֤��Կ


static const uint8_t APDUFINDSIMCARD[] = {0x00,0xA4,0x04, 0x00, 0x00}; // ȷ�Ͽ�Ƭ����
static const uint8_t APDUGETSEID[] = {0x80,0xCA,0x00, 0x44, 0x00}; // ȷ�Ͽ�Ƭ����

/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/


/*-------------------------------------------------ȫ�ֱ�������-----------------------------------------------------*/
CARD_MEG_Def  CpuCardMessage;

/*-------------------------------------------------��������---------------------------------------------------------*/
uint8_t check_apdu_result( uint8_t *data );

/*-------------------------------------------------��������---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  Cloud_ChangeCpuCardPageidToEepromAddr
* Description   :  ��ö�Ӧ��EEPROM��ַ
* Para          :
* Return        :  ��Ӧ��Pageid
*********************************************************************************************************************/
static uint32_t Cloud_ChangeCpuCardPageidToEepromAddr( uint16_t pageid )
{
    uint32_t addr;
    addr = MSG_CPU_CARD_REG_START + pageid * MSG_CPU_CARD_ONE_SIZE;
    return addr;
}


/*********************************************************************************************************************
* Function Name :  CpuCard_QueryUserCpuCardMegFromEeprom
* Description   :  ��ѯEEPROM���Ƿ��ж�Ӧ��cpu���û����
* Para          :  type - ���������� USER_ID -�û����  PAGE_ID - �ֻ���EEPROM�ڲ����
                   numId - cpu���û���� / cpu����EEPROM���
                   paddr - �˱�Ŷ�Ӧ��EEPROM��ַ  pIphoneMeg-��Ӧ����ֻ���Ϣָ��
* Return        :  0-δ�ҵ�  1- ���ҵ�
*********************************************************************************************************************/
uint8_t CpuCard_QueryUserCpuCardMegFromEeprom( uint8_t type, uint32_t numId, uint32_t *paddr, CARD_MEG_Def *pCardMeg )
{
    uint8_t  retval = 0;
    uint16_t pageid;
    uint32_t addr = 0;
    my_printf( "CPU CARD ALL NUM: %d \n", SystemSeting.SysCardAllNum );

    for( pageid=0; pageid<MSG_CPU_CARD_USER_NUM; pageid++ )
    {
        addr = Cloud_ChangeCpuCardPageidToEepromAddr( pageid );
        HAL_EEPROM_ReadBytes(addr,(uint8_t *)&pCardMeg->tab[0], 1);
        if( pCardMeg->data.UserValue == MEM_FACT_MEM_FIG )
        {
            HAL_EEPROM_ReadBytes(addr,(uint8_t *)&pCardMeg->tab, MSG_CPU_CARD_REG_ONE_SIZE);
            my_printf("CARD TYPE = %c --> ",(char)pCardMeg->tab[1]);
            my_printf("CARD USER ID: %d\r\n", pCardMeg->data.UserId);

            if( type == USER_ID )       //����û�
            {
                if( pCardMeg->data.UserId == numId )  //�ҵ��˶�Ӧ��ID
                {
                    *paddr = addr;
                    retval = 1;
                    break;
                }
                else
                {
                    continue;
                }
            }

            if( type == CARD_ID )  //cpu��ID
            {
                if( pCardMeg->data.CardId == numId )  //�ҵ��˶�Ӧ��ID
                {
                    *paddr = addr;
                    retval = 1;
                    break;
                }
                else
                {
                    continue;
                }
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
* Function Name :  Cloud_SearchEmptyCpuCardPageIdFromEeprom
* Description   :  ��ѯEEPROM�пյ�cpu����Ӧ��Pageid
* Para          :      
* Return        :  ��Ӧ��Pageid
*********************************************************************************************************************/
static uint16_t Cloud_SearchEmptyCpuCardPageIdFromEeprom( void ) 
{
    uint16_t pageid;
    uint32_t addr;
    uint8_t SaveFlag=0;
    for( pageid = 0; pageid < MSG_CPU_CARD_USER_NUM; pageid++ )
    {
        addr = Cloud_ChangeCpuCardPageidToEepromAddr( pageid );
        HAL_EEPROM_ReadBytes(addr,&SaveFlag,1);	//read Reg MSG fig
        if( SaveFlag != MEM_FACT_MEM_FIG )//ָ�ƴ��ڱ�־.
        {
            return pageid; //return empty pageid
        }
    }
    return pageid;
}

/*********************************************************************************************************************
* Function Name :  Cloud_SaveOneCpuCardMegIntoEeprom
* Description   :  ������cpu����Ϣ������EEPROM��
* Para          :  pIphoneMeg - �ֻ��ǿ���Ϣָ��
* Return        :  none
*********************************************************************************************************************/
static void Cloud_SaveOneCpuCardMegIntoEeprom( CARD_MEG_Def *pCardMeg )
{
    my_printf("Cloud_SaveOneCpuCardMegIntoEeprom()\n");
    uint16_t pageid;
    uint32_t addr;
//    uint32_t cpuCardId=0;
    
    //CPU������¼
    SystemSeting.SysCardAllNum++;
    SystemWriteSeting(&SystemSeting.SysCardAllNum,1);
    
    pageid = Cloud_SearchEmptyCpuCardPageIdFromEeprom(); 

    addr = Cloud_ChangeCpuCardPageidToEepromAddr( pageid );
    HAL_EEPROM_WriteBytes(addr,(uint8_t *)&pCardMeg->tab, MSG_CPU_CARD_REG_ONE_SIZE); 

}

/*********************************************************************************************************************
* Function Name :  Cloud_DelcCpuCardegFromEeprom
* Description   :  ɾ��EEPROM�жԵ�ַ��cpu��
* Para          :  addr - ��ɾ���ֻ��ĵ�ַ   
* Return        :  none 
*********************************************************************************************************************/
static void Cloud_DelcCpuCardegFromEeprom( uint32_t addr )
{
    my_printf("Cloud_DelcCpuCardegFromEeprom()\n");
    CARD_MEG_Def  cpuCardMeg = {0}; 

    HAL_EEPROM_ReadBytes(addr, cpuCardMeg.tab, MSG_CPU_CARD_REG_ONE_SIZE);
    cpuCardMeg.data.UserValue = 0xff;
    HAL_EEPROM_WriteBytes(addr,(uint8_t *)&cpuCardMeg.data.UserValue, 1);

    if( SystemSeting.SysCardAllNum > 0 )
      SystemSeting.SysCardAllNum--;
    SystemWriteSeting(&SystemSeting.SysCardAllNum,1);

}

/*********************************************************************************************************************
* Function Name :  Cloud_ClearAllCpuCardMegFromEeprom
* Description   :  ���EEPROM�е�����cpu��
* Para          :  none
* Return        :  none
*********************************************************************************************************************/
static void Cloud_ClearAllCpuCardMegFromEeprom( void )
{
    
    uint32_t pageid = 0;
    uint32_t addr = 0;
    memset(&CpuCardMessage,0,sizeof(CpuCardMessage)); 
    
    for(pageid = 0; pageid < MSG_CARD_NUM; pageid++)
    {
        addr = Cloud_ChangeCpuCardPageidToEepromAddr(pageid);
        HAL_EEPROM_WriteBytes(addr, (uint8_t *)&CpuCardMessage, sizeof(CpuCardMessage));
    }
}





/*********************************************************************************************************************
* Function Name :  App_CpuCard_FlieInit()
* Description   :  ����ļ���ʼ��
* Para          :  none
* Return        :  none
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_CpuCard_FlieInit( void )
{
//	 uint8_t ret_value=1;

    uint8_t  err;
    uint8_t  buf[260];
    uint8_t  clength;
//  u16 ilength;
    iso14443AProximityCard_t gvCardA;

    iso14443AInitialize();
    err = nz3801ActivateField(false);
    if(err!=ERR_NONE)
    {
        SuccessRate.numFieldOffFail++;
        ISO_14443A_DBG(">Field OFF\r\n FAIL.\r\n");
        return err;
    }
    NZ_DelayMs(6);
//	NZ_DelayMs(55);
//	my_printf(">nz3801ActivateField\r\n");
    err = nz3801ActivateField(true);
    if(err!=ERR_NONE)
    {
        SuccessRate.numFieldOnFail++;
        ISO_14443A_DBG(">Field ON\r\n FAIL.\r\n");
        return err;
    }
    NZ_DelayMs(15);

//  NZ_DelayMs(105);//add by banglin.zhang@20191226
    SuccessRate.Totality++;
//	my_printf(">iso14443ASelect\r\n");
    // REQAorWUPA & ANTICOLLION
    err = iso14443ASelect(ISO14443A_CMD_WUPA, &gvCardA);
    if(err==ERR_NONE)
    {
        ISO_14443A_DBG(">REQA\r\n OK.\r\n");
        ISO_14443A_DBG(" YES Card.\r\n");
        ISO_14443A_DBG_EXT(gvCardA.atqa,sizeof(gvCardA.atqa)," ATQA:");
        ISO_14443A_DBG_EXT(gvCardA.uid,gvCardA.actlength," UID:");
        memcpy(&g_u8_CardInfo[2],gvCardA.uid,4);
        SuccessRate.numL3OK++;
    }
    else
    {
        ISO_14443A_DBG(">REQA\r\n FAIL.[%d]\r\n",err);
        ISO_14443A_DBG(" NO Card!!!\r\n");
    }

    // RATS
    if(err==ERR_NONE)
    {
        my_printf(">iso14443AEnterProtocolMode\r\n");
        gvCardA.fsdi = 8; // fsdi=8 -> FSD=256
        err = iso14443AEnterProtocolMode(&gvCardA,buf,&clength);
        if(err==ERR_NONE)
        {
            ISO_14443A_DBG(">RATS\r\n OK.\r\n");
            ISO_14443A_DBG_EXT(buf,clength," ATS:");
            ISO_14443A_DBG(" FSCI=%d\r\n FWI=%d\r\n TA=%02X\r\n",gvCardA.fsci,gvCardA.fwi,gvCardA.TA);
        }
        else
        {
            ISO_14443A_DBG(">RATS\r\n FAIL.[%d]\r\n",err);
            err =12;//����RTASʧ��
        }
    }


#if 1
    // PPS
    if((err==ERR_NONE) && (PPSEn))
    {
        uint8_t pps1 = 0x0; // 106K
//				TxRxSpeed = 212;
        if(TxRxSpeed==212) pps1 = 0x5;
        else if(TxRxSpeed==424) pps1 = 0xA;
        else if(TxRxSpeed==848) pps1 = 0xF;
        else /*if(TxRxSpeed==106)*/ pps1 = 0x0;
//        my_printf(">iso14443ASendProtocolAndParameterSelection\r\n");
        err = iso14443ASendProtocolAndParameterSelection(pps1);
        if(err==ERR_NONE)
        {
            ISO_14443A_DBG(">PPS\r\n OK.\r\n");
            // here set tx&rxbit speed
            if(pps1==0x5)
            {
                nzWriteReg(MODWIDTH, reg24h_MODWIDTH);
                nzSetBitMask(TXMODE, BFL_JBIT_212KBPS);
                nzSetBitMask(RXMODE, BFL_JBIT_212KBPS);
                ISO_14443A_DBG(" SPEED=212kBd.\r\n");
            }
            else if(pps1==0xA)
            {
                nzWriteReg(MODWIDTH, reg24h_MODWIDTH);
                nzSetBitMask(TXMODE, BFL_JBIT_424KBPS);
                nzSetBitMask(RXMODE, BFL_JBIT_424KBPS);
                ISO_14443A_DBG(" SPEED=424kBd.\r\n");
            }
            else if(pps1==0xF)
            {
                nzWriteReg(MODWIDTH, reg24h_MODWIDTH);
                nzSetBitMask(TXMODE, BFL_JBIT_848KBPS);
                nzSetBitMask(RXMODE, BFL_JBIT_848KBPS);
                ISO_14443A_DBG(" SPEED=848kBd.\r\n");
            }
            else
            {
                ISO_14443A_DBG(" SPEED=106kBd.\r\n");
            }
        }
        else
        {
            ISO_14443A_DBG(">PPS\r\n FAIL.[%d]\r\n",err);
            ISO_14443A_DBG(" SPEED=106kBd.\r\n");
        }
        err = ERR_NONE;
    }
#endif

    return err;
}

/*********************************************************************************************************************
* Function Name :  Api_Math_CompareBufByte()
* Description   :  �Ƚ����������Ƿ�һ��
* Para Input    :  pSbuf- ����1  pDbuf- ����2   length- ���Ƚϵ�������
* Para Output   :  none
* Return        :  0= ��һ��  1= һ��
* author				:  gushengchi  2021/11/16
*********************************************************************************************************************/
static uint8_t Api_Math_CompareBufByte( const uint8_t *pSbuf, const uint8_t *pDbuf, uint8_t length )
{
    if( !length )
        return 0;

    for(uint8_t i=0; i<length; length++)
    {
        if( pSbuf[i] != pDbuf[i] )
        {
            return 0;
        }
    }
    return 1;
}

/*********************************************************************************************************************
* Function Name :  App_CpuCard_ApduRegistProcess()
* Description   :  ��cpu����ע������
* Para Intput   :
* Para Output   :
* Return        :  0- �ɹ�  other- ʧ��
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_CpuCard_ApduRegistProcess( CARD_MEG_Def *pCardMeg )
{
    uint8_t  err = eAPDUSuccess;			//����״̬��
    uint8_t  buf[270]; 								//��ȡapdu����
    uint16_t ilength;  								//apdu��ȡ�����ݵ�������
    uint8_t  Random_buf[8] = {0}; 		//���������
    uint8_t  cipher_buf[24]= {0};
//	CARD_MEG_Def  carMeg = {0};

//    /*--------ȷ�������Ƿ�󶨸ÿ�-----*/
//	uint32 address =0;
//	CARD_MEG_Def  cardMeg={0};
//	uint32 cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
//	if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, &cardMeg ))
//	{
//			Cloud_DelcCpuCardegFromEeprom( address );
//	}
    my_printf("/*---------Add cpu card start!-----------*/\r\n");
    /*--------ѡ����Ŀ¼--------------*/
    err = ApduTransceive( (uint8_t*)APDUSelectMF, sizeof(APDUSelectMF), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-APDUSelectMF OK.\r\n");
        ISO_14443A_DBG_EXT((uint8_t*)APDUSelectMF,sizeof(APDUSelectMF)," TX:");
        ISO_14443A_DBG_EXT(buf,ilength," RX:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result )
        {
#ifdef DEBUG_LOG_ENABLE
            my_printf("CPU card>APDU-APDUSelectMF\r\n FAIL.[%d]\r\n",err);
#endif
            return eAPDUFailSelectMF; //ѡ��MF�ļ�����
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailSelectMF; //ѡ��MF�ļ�����;
    }

    /*--------��ȡ�����--------------*/
    err = ApduTransceive( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-GetRandom OK.\r\n");
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
            (void)memcpy( Random_buf, &buf[0], 4);
        }
        else
        {
            return eAPDUFailGetRandom;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-GetRandom\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailGetRandom;
    }

    /*--------�ⲿ��֤----------------*/
#ifdef DEBUG_LOG_ENABLE
    my_printf("DES ECB ENC:");
#endif

    uint8_t ret = des_ecb_encrypt( cipher_buf, Random_buf, 4, MFFactorySetkey );
#ifdef DEBUG_LOG_ENABLE
    for(int i = 0; i < ret; i++)
    {
        my_printf("%02X",cipher_buf[i]);
    }
    my_printf("\r\n");
#endif
    memcpy( &APDUGetExternalAuthentication[5], cipher_buf, 8 ); //��д��������
    err = ApduTransceive( (uint8_t*)APDUGetExternalAuthentication, sizeof(APDUGetExternalAuthentication), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-iAPDU_External_authenticate OK.\r\n");
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetExternalAuthentication,sizeof(APDUGetExternalAuthentication)," TX:" );
        my_printf( "RXlen=%d ",ilength );
        ISO_14443A_DBG_EXT( buf,ilength," RX:" );
#endif
        uint8_t result = check_apdu_result(&buf[ilength-2]);
        if( result == 0 )
        {
        }
        else
        {
#if 0
            return eAPDUFailGetExternalAuthentication; //ע��ʱ�ⲿ��֤���󣬿��Լ���
#endif
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-iAPDU_External_authenticate\r\n FAIL.[%d]\r\n",err);
#endif
#if 0
        return eAPDUFailGetExternalAuthentication;  //ע��ʱ�ⲿ��֤���󣬿��Լ���
#endif
    }

    /*--------��տ�Ƭ----------------*/
    err = ApduTransceive( (uint8_t*)APDUEmptyCard, sizeof(APDUEmptyCard), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUEmptyCard OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUEmptyCard, sizeof(APDUEmptyCard), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailEmptyCard;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUEmptyCard\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailEmptyCard;
    }

    /*--------����MF��Կ�ļ�----------*/
    err = ApduTransceive( (uint8_t*)APDUCreateMFKeyFile, sizeof(APDUCreateMFKeyFile), buf, &ilength );
    if(	err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUCreateMFKeyFile OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUCreateMFKeyFile, sizeof(APDUCreateMFKeyFile), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailCreateMFKeyFile;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUCreateMFKeyFile\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailCreateMFKeyFile;
    }

    /*--------��Կ�ļ�������Կ--------*/
    err = ApduTransceive( (uint8_t*)APDUAddMFKeyFilePwd, sizeof(APDUAddMFKeyFilePwd), buf, &ilength );
    if(	err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUAddMFKeyFilePwd OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUAddMFKeyFilePwd, sizeof(APDUAddMFKeyFilePwd), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailAddMFKeyFilePwd;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUAddMFKeyFilePwd\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailAddMFKeyFilePwd;
    }

    /*--------����Ŀ¼DF�ļ�----------*/
    err = ApduTransceive( (uint8_t*)APDUCreateDF, sizeof(APDUCreateDF), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUCreateDF OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUCreateDF, sizeof(APDUCreateDF), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailCreateDF;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUCreateDF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailCreateDF;
    }

    /*--------ѡ��DF�ļ�Ŀ¼----------*/
    err = ApduTransceive( (uint8_t*)APDUSelectDF, sizeof(APDUSelectDF), buf, &ilength );
    if(	err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUSelectDF OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUSelectDF, sizeof(APDUSelectDF), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailSelectDF;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUSelectDF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailSelectDF;
    }

    /*--------����DF�ļ��µ���Կ�ļ�--*/
    err = ApduTransceive( (uint8_t*)APDUCreateDFKeyFile, sizeof(APDUCreateDFKeyFile), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("APDUCreateDFKeyFile OK.\r\n");
        ISO_14443A_DBG_EXT((uint8_t*)APDUCreateDFKeyFile,sizeof(APDUCreateDFKeyFile)," TX:");
        ISO_14443A_DBG_EXT(buf,ilength," RX:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailCreateDFKeyFile;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUCreateDFKeyFile\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailCreateDFKeyFile;
    }

    /*--------����ⲿ��֤��Կ--------*/
    // (void)random_vector_generate( DFExternalKey, DF_EXTERN_KEY_SIZE );//�����8��
    DRV_InterGenerateRandVec(DFExternalKey, DF_EXTERN_KEY_SIZE);
    ISO_14443A_DBG_EXT( DFExternalKey, DF_EXTERN_KEY_SIZE, " DFExternalKey:" );
    memcpy( &APDUAddDFExternalKey[10], DFExternalKey, DF_EXTERN_KEY_SIZE );
    err = ApduTransceive( (uint8_t*)APDUAddDFExternalKey, sizeof(APDUAddDFExternalKey), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("APDUAddDFExternalKey OK.\r\n");
        ISO_14443A_DBG_EXT((uint8_t*)APDUAddDFExternalKey,sizeof(APDUAddDFExternalKey)," TX:");
        ISO_14443A_DBG_EXT(buf,ilength," RX:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailAddDFExternalKey;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUAddDFExternalKey\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailAddDFExternalKey;
    }

    /*--------����ڲ���֤��Կ--------*/
    // (void)random_vector_generate( DFInsideKey, DF_INSIDE_KEY_SIZE );//�����8��
    DRV_InterGenerateRandVec(DFInsideKey, DF_INSIDE_KEY_SIZE);
    ISO_14443A_DBG_EXT( DFInsideKey, DF_INSIDE_KEY_SIZE, " DFInsideKey:" );
    memcpy( &APDUAddDFInsideKey[10], DFInsideKey, DF_INSIDE_KEY_SIZE );
    err = ApduTransceive( (uint8_t*)APDUAddDFInsideKey, sizeof(APDUAddDFInsideKey), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUAddDFInsideKey OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUAddDFInsideKey, sizeof(APDUAddDFInsideKey), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailAddDFInsideKey;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUAddDFInsideKey\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailAddDFInsideKey;
    }

    /*--------�����������ļ�----------*/
    err = ApduTransceive( (uint8_t*)APDUCreateBin, sizeof(APDUCreateBin), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("APDUCreateBin OK.\r\n");
        ISO_14443A_DBG_EXT((uint8_t*)APDUCreateBin,sizeof(APDUCreateBin)," TX:");
        ISO_14443A_DBG_EXT(buf,ilength," RX:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailCreateBin;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUCreateBin\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailCreateBin;
    }

    /*--------ѡ��Ҫ�����Ķ������ļ�--*/
    err = ApduTransceive( (uint8_t*)APDUSelectBin, sizeof(APDUSelectBin), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUSelectBin OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUSelectBin, sizeof(APDUSelectBin), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailSelectBin;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDUSelectBin\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailSelectBin;
    }

    /*--------д�����������ļ���д������MAC��--------*/
    uint8_t lockMac[ 6 ] = {0};
    #if 0
    lockMac[ 0 ] = (uint8_t)(NRF_FICR->DEVICEADDR[1]>>8) | 0xC0;
    lockMac[ 1 ] = (uint8_t)NRF_FICR->DEVICEADDR[1];
    lockMac[ 2 ] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>24);
    lockMac[ 3 ] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>16);
    lockMac[ 4 ] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>8);
    lockMac[ 5 ] = (uint8_t)NRF_FICR->DEVICEADDR[0];
    #else
    PUBLIC_GetMacAdd(&lockMac[0]);
    #endif

    memcpy( &APDUWriteBin[5], lockMac, 6 );
    err = ApduTransceive( (uint8_t*)APDUWriteBin, sizeof(APDUWriteBin), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUWriteBin OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUWriteBin, sizeof(APDUWriteBin), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailWriteBin;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUWriteBin\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailWriteBin;
    }

    /*--------���˱��濨�������Ϣ----*/
    memcpy( pCardMeg->data.ExternAuthenPublicKey, DFExternalKey, DF_EXTERN_KEY_SIZE );
    memcpy( pCardMeg->data.InnerAuthenPublicKey, DFInsideKey, DF_INSIDE_KEY_SIZE );
//		pCardMeg->data.CardId = cpuCardId;

    /*--------THE END-----------------*/
    my_printf("/*---------Add cpu card stop!-----------*/\r\n");

    return err;
}

/*********************************************************************************************************************
* Function Name :  App_CpuCard_ApduRegistProcess()
* Description   :  ��cpu���Ľ����������
* Para Intput   :  cardId- ����
* Para Output   :  pCardMeg- �����Ƭ��Ϣ
* Return        :  0- �ɹ�  other- ʧ��
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_CpuCard_ApduIdentifyProcess( uint32_t cardId, CARD_MEG_Def *pCardMeg )
{
    uint8_t  err = eAPDUSuccess;			//����״̬��
    uint8_t  buf[270]; 								//��ȡapdu����
    uint16_t ilength;  								//apdu��ȡ�����ݵ�������
    uint8_t  Random_buf[ 8 ] = { 0 }; //���������
    uint8_t  cipher_buf[24]= {0};

    /*--------ȷ�������Ƿ�󶨸ÿ�------*/
    uint32_t address =0;
    CARD_MEG_Def  cardMeg= {0};
    if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cardId, &address, &cardMeg ))
    {
        if( cardMeg.data.TimelinessState == 1 )  //ʱЧ����
        {
            #if 0
            uint32_t currentUtc;
            Api_Wifi_LocalRtcToUtc( &currentUtc );
            if( 0 == Api_Math_CheckTimeliness( currentUtc, cardMeg.data.StartTim, cardMeg.data.StopTim, cardMeg.data.Weekday ))
            {
                return eAPDUMatchFail;
            }
            #else
            if(RTC_Successfully != HAL_RTC_TimeIsTimesize(&cardMeg.data.tm_vaild.start,
														&cardMeg.data.tm_vaild.stop,
														cardMeg.data.tm_vaild.wday))
            {
                return eAPDUMatchFail;
            }
            #endif
        }
    }
    else
    {
        return eAPDUMatchFail;
    }
    memcpy( DFExternalKey, cardMeg.data.ExternAuthenPublicKey, DF_EXTERN_KEY_SIZE );
    memcpy( DFInsideKey, cardMeg.data.InnerAuthenPublicKey, DF_INSIDE_KEY_SIZE );

    my_printf("/*---------Check cpu card start!-----------*/\r\n");
    /*--------ѡ��DF�ļ�--------------*/
    err = ApduTransceive( (uint8_t*)APDUSelectDF, sizeof(APDUSelectDF), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUSelectDF OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUSelectDF, sizeof(APDUSelectDF), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailSelectDF;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUSelectDF\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailSelectDF;
    }

    /*--------��ȡ�����--------------*/
    err = ApduTransceive( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDU-GetRandom OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
            (void)memcpy( Random_buf, &buf[0], 4 );
        }
        else
        {
            return eAPDUFailGetRandom;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDU-GetRandom\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailGetRandom;
    }

    /*--------�ⲿ��֤----------------*/
#ifdef DEBUG_LOG_ENABLE
    ISO_14443A_DBG_EXT((uint8_t*)Random_buf,sizeof(Random_buf)," Random_buf:");
    ISO_14443A_DBG_EXT( DFExternalKey, DF_EXTERN_KEY_SIZE, " DFExternalKey:");
    my_printf("DES ECB ENC:");
#endif
    uint8_t ret = des_ecb_encrypt( cipher_buf, Random_buf, 4, DFExternalKey );
#ifdef DEBUG_LOG_ENABLE
    for(int i = 0; i < ret; i++)
    {
        my_printf("%02X",cipher_buf[i]);
    }
    my_printf("\r\n");
#endif
    memcpy( &APDUGetExternalAuthentication[5], cipher_buf, 8 );
    err = ApduTransceive( (uint8_t*)APDUGetExternalAuthentication, sizeof(APDUGetExternalAuthentication), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDU-iAPDU_External_authenticate OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetExternalAuthentication, sizeof(APDUGetExternalAuthentication), " TX:" );
        my_printf( "RXlen=%d ",ilength );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result=check_apdu_result(&buf[ilength-2]);
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailGetExternalAuthentication;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDU-iAPDU_External_authenticate\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailGetExternalAuthentication;
    }

    /*--------�ڲ���֤�����----------*/
    uint8_t insideRadomData[8];
    // (void)random_vector_generate( insideRadomData, sizeof insideRadomData );//�����8��
    DRV_InterGenerateRandVec(insideRadomData, sizeof insideRadomData);
    memcpy( &APDUGetInsideAuthentication[5], insideRadomData, sizeof insideRadomData );
    err = ApduTransceive( (uint8_t*)APDUGetInsideAuthentication, sizeof(APDUGetInsideAuthentication), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "iAPDU_Inside_authenticate OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetInsideAuthentication, sizeof(APDUGetInsideAuthentication), " TX:");
        ISO_14443A_DBG_EXT( buf, ilength, " RX:");
        ISO_14443A_DBG_EXT( DFInsideKey, DF_INSIDE_KEY_SIZE, " DFInsideKey:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
            uint8_t tp1 = des_ecb_encrypt( cipher_buf, insideRadomData, 8, DFInsideKey );
            if( 0 == Api_Math_CompareBufByte( buf, cipher_buf, tp1 ) )  //fail
            {
                my_printf( "iAPDU_Inside_authenticate check fail!\r\n" );
                return eAPDUFailGetInsideAuthentication;
            }
        }
        else
        {
            return eAPDUFailGetInsideAuthentication;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUGetInsideAuthentication\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailGetInsideAuthentication;
    }

    /*--------ѡ��������ļ�----------*/
    err = ApduTransceive( (uint8_t*)APDUSelectBin, sizeof(APDUSelectBin), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUSelectBin OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUSelectBin, sizeof(APDUSelectBin), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
        }
        else
        {
            return eAPDUFailSelectDF;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUSelectBin\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailSelectDF;
    }

    /*--------��ȡ�������ļ�----------*/
    err = ApduTransceive( (uint8_t*)APDUReadBin, sizeof(APDUReadBin), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( "APDUReadBin OK.\r\n" );
        ISO_14443A_DBG_EXT( (uint8_t*)APDUReadBin, sizeof(APDUReadBin), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
            uint8_t lockMac[ 6 ] = {0};
            #if 0
            lockMac[ 0 ] = (uint8_t)(NRF_FICR->DEVICEADDR[1]>>8) | 0xC0;
            lockMac[ 1 ] = (uint8_t)NRF_FICR->DEVICEADDR[1];
            lockMac[ 2 ] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>24);
            lockMac[ 3 ] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>16);
            lockMac[ 4 ] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>8);
            lockMac[ 5 ] = (uint8_t)NRF_FICR->DEVICEADDR[0];
            #else
            PUBLIC_GetMacAdd(&lockMac[0]);
            #endif
            
            if( 0 == Api_Math_CompareBufByte( buf, lockMac, 6 ) )  // �Աȶ�ȡ�����ļ�����
            {
                my_printf( "Device MAC check fail!\r\n" );
                return eAPDUFailReadBin;
            }
        }
        else
        {
            return eAPDUFailReadBin;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf( ">APDUReadBin\r\n FAIL.[%d]\r\n", err );
#endif
        return eAPDUFailReadBin;
    }
    /*--------�����Ƭ��Ϣ-----------*/
    memcpy( pCardMeg->tab, cardMeg.tab, sizeof cardMeg.tab );

    /*--------THE END----------------*/
    my_printf("/*---------Check cpu card stop!-----------*/\r\n");
    return err;
}
/*********************************************************************************************************************
* Function Name :  check_apdu_result
* Description   :  ��ȡӦ��Ľ�����ж��Ƿ���ȷ
* Para          :  data- ���յ�������
* Return        :  0�������ɹ�  ����ֵ������ʧ��
*********************************************************************************************************************/
static uint8_t check_apdu_result( uint8_t *data )
{
#ifdef DEBUG_LOG_ENABLE
    my_print_hex("check_apdu_result input",data,2);
#endif

    uint8_t ret_value=0;
    if(data[0]==0x90&&data[1]==0x00)
    {
        ret_value=0;
    }
    else  if(data[0]==0x6A&&data[1]==0x82)
    {
        ret_value=1;
    }
    else {
        ret_value=2;
    }
    return ret_value;
}
///*********************************************************************************************************************
//* Function Name :  App_CpuCard_ApduUnregistProcess()
//* Description   :  ���CPU������
//* Para Intput   :
//* Para Output   :
//* Return        :  0- �ɹ�  other- ʧ��
//* author				:  gushengchi
//*********************************************************************************************************************/
//static uint8_t App_CpuCard_ApduUnregistProcess( uint8 userLimit, TimValueType_Enum timeValueType, CARD_MEG_Def *pCardMeg )
//{
//    uint8_t  err = eAPDUSuccess;			//����״̬��
//    uint8_t  buf[270]; 								//��ȡapdu����
//    uint16_t ilength;  								//apdu��ȡ�����ݵ�������
//	uint8_t  Random_buf[8] = {0}; 		//���������
//	uint8_t  cipher_buf[24]= {0};
//
//		my_printf("/*---------Empty cpu card start!-----------*/\r\n");
//		/*--------ѡ����Ŀ¼--------------*/
//		err = ApduTransceive( (uint8_t*)APDUSelectMF, sizeof(APDUSelectMF), buf, &ilength );
//		if( err == eAPDUSuccess )
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf("CPU card>APDU-iAPDU_SelectAID OK.\r\n");
//				ISO_14443A_DBG_EXT((uint8_t*)APDUSelectMF,sizeof(APDUSelectMF)," TX:");
//				ISO_14443A_DBG_EXT(buf,ilength," RX:");
//				#endif
//				uint8_t result = check_apdu_result( &buf[ilength-2] );
//				if( result )
//				{
//						#ifdef DEBUG_LOG_ENABLE
//						my_printf("CPU card>APDU-iAPDU_SelectAID\r\n FAIL.[%d]\r\n",err);
//						#endif
//						return eAPDUFailSelectMF; //ѡ��MF�ļ�����
//				}
//		}
//		else
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
//				#endif
//				return eAPDUFailSelectMF; //ѡ��MF�ļ�����;
//		}
//
//		/*--------��ȡ�����--------------*/
//		err = ApduTransceive( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), buf, &ilength );
//		if( err == eAPDUSuccess )
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf(">APDU-GetRandom OK.\r\n");
//				ISO_14443A_DBG_EXT( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), " TX:" );
//				ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
//				#endif
//				uint8_t result = check_apdu_result( &buf[ilength-2] );
//				if( result == 0 )
//				{
//					 (void)memcpy( Random_buf, &buf[0], 4);
//				}
//				else
//				{
//					 return eAPDUFailGetRandom;
//				}
//		}
//		else
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf(">APDU-GetRandom\r\n FAIL.[%d]\r\n",err);
//				#endif
//				return eAPDUFailGetRandom;
//		}
//
//		/*--------�ⲿ��֤----------------*/
//		#ifdef DEBUG_LOG_ENABLE
//		my_printf("DES ECB ENC:");
//		#endif
//		uint8 ret = des_ecb_encrypt( cipher_buf, Random_buf, 4, MFFactorySetkey );
//		#ifdef DEBUG_LOG_ENABLE
//		for(int i = 0; i < ret; i++)
//		{
//				my_printf("%02X",cipher_buf[i]);
//		}
//		my_printf("\r\n");
//		#endif
//    memcpy( &APDUGetExternalAuthentication[5], cipher_buf, 8 ); //��д��������
//		err = ApduTransceive( (uint8_t*)APDUGetExternalAuthentication, sizeof(APDUGetExternalAuthentication), buf, &ilength );
//		if( err == eAPDUSuccess )
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf( ">APDU-iAPDU_External_authenticate OK.\r\n" );
//				ISO_14443A_DBG_EXT( (uint8_t*)APDUGetExternalAuthentication, sizeof(APDUGetExternalAuthentication), " TX:" );
//				my_printf( "RXlen=%d ",ilength );
//				ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
//				#endif
//				uint8_t result = check_apdu_result( &buf[ilength-2] );
//				if( result == 0 )
//				{
//				}
//				else
//				{
//						#if 0
//						return eAPDUFailGetExternalAuthentication; //ע��ʱ�ⲿ��֤���󣬿��Լ���
//						#endif
//				}
//		}
//		else
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf(">APDU-iAPDU_External_authenticate\r\n FAIL.[%d]\r\n",err);
//				#endif
//				#if 0
//				return eAPDUFailGetExternalAuthentication;  //ע��ʱ�ⲿ��֤���󣬿��Լ���
//				#endif
//		}
//
//		/*--------��տ�Ƭ----------------*/
//		err = ApduTransceive( (uint8_t*)APDUEmptyCard, sizeof(APDUEmptyCard), buf, &ilength );
//		if( err == eAPDUSuccess )
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf( "APDUEmptyCard OK.\r\n" );
//				ISO_14443A_DBG_EXT( (uint8_t*)APDUEmptyCard, sizeof(APDUEmptyCard), " TX:" );
//				ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
//				#endif
//				uint8_t result = check_apdu_result( &buf[ilength-2] );
//				if( result == 0 )
//				{
//				}
//				else
//				{
//					 return eAPDUFailEmptyCard;
//				}
//		}
//		else
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf( ">APDUEmptyCard\r\n FAIL.[%d]\r\n", err );
//				#endif
//				return eAPDUFailEmptyCard;
//		}
//
//		/*--------THE END----------------*/
//
//		my_printf("/*---------Empty cpu card stop!-----------*/\r\n");
//
//    return err;
//}
/*********************************************************************************************************************
* Function Name :  App_CpuCard_ApduRegistProcess()
* Description   :  �ͳ���SIM����ע������
* Para Intput   :
* Para Output   :
* Return        :  0- �ɹ�  other- ʧ��
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_SIMCard_ApduRegistProcess(CARD_MEG_Def *pCardMeg )
{
    static uint8_t SEIDBUF[20]= {0};
    uint8_t  err = eAPDUSuccess;			//����״̬��
    uint8_t  buf[255]; 								//��ȡapdu����
    uint8_t  headbuf[5]= {0x00,0x44,0x0C,0x93,0x0A};
    uint16_t ilength;  		//apdu��ȡ�����ݵ�������
//	/*--------ȷ���������ް󶨸ÿ�--------------*/
//	uint32 address =0;
//	CARD_MEG_Def  cardMeg={0};
//	uint32 cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
//	if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, &cardMeg ))
//	{
//		Cloud_DelcCpuCardegFromEeprom( address );
//	}
    my_printf("/*---------Add SIM card start!-----------*/\r\n");
    /*--------ѡ����Ŀ¼--------------*/
    err = ApduTransceive( (uint8_t*)APDUFINDSIMCARD, sizeof(APDUFINDSIMCARD), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-APDUSelectMF OK.\r\n");
        ISO_14443A_DBG_EXT((uint8_t*)APDUSelectMF,sizeof(APDUSelectMF)," TX:");
        ISO_14443A_DBG_EXT(buf,ilength," RX:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result )
        {
#ifdef DEBUG_LOG_ENABLE
            my_printf("CPU card>APDU-APDUSelectMF\r\n FAIL.[%d]\r\n",err);
#endif
            return eAPDUMatchFail; //ѡ��MF�ļ�����
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUMatchFail; //ѡ��MF�ļ�����;
    }
    err = ApduTransceive( (uint8_t*)APDUGETSEID, sizeof(APDUGETSEID), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-GetRandom OK.\r\n");
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
            (void)memcpy( SEIDBUF, &buf[0], 15);
        }
        else
        {
            return eAPDUMatchFail;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-GetRandom\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUMatchFail;
    }

    for(int i=0; i<5; i++) //�ƶ�SIM�����ع̶����� 0440C930A+SEID
    {
        if(SEIDBUF[i]==headbuf[i])
        {
        }
        else
        {
            return eAPDUMatchFail;
        }
    }
//    memcpy(pCardMeg->data.SEID,&SEIDBUF[5],10);

    /*--------THE END-----------------*/
    my_printf("/*---------Add cpu card stop!-----------*/\r\n");

    return err;
}
/*********************************************************************************************************************
* Function Name :  App_CpuCard_ApduRegistProcess()
* Description   :  �ͳ���SIM������֤����
* Para Intput   :
* Para Output   :
* Return        :  0- �ɹ�  other- ʧ��
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_SIMCard_ApduIdentifyProcess( uint32_t cardId, CARD_MEG_Def *pCardMeg )
{
    static uint8_t SEIDBUF[20]= {0};
    uint8_t  err = eAPDUSuccess;			//����״̬��
    uint8_t  buf[255]; 								//��ȡapdu����
    uint8_t  headbuf[5]= {0x00,0x44,0x0C,0x93,0x0A};
    uint16_t ilength;  		//apdu��ȡ�����ݵ�������
    /*--------ȷ�������Ƿ�󶨸ÿ�------*/
    uint32_t address =0;
    CARD_MEG_Def  cardMeg= {0};
    if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cardId, &address, &cardMeg ))
    {
        if( cardMeg.data.TimelinessState == 1 )  //ʱЧ����
        {
            #if 0
            uint32_t currentUtc;
            Api_Wifi_LocalRtcToUtc( &currentUtc );
            if( 0 == Api_Math_CheckTimeliness( currentUtc, cardMeg.data.StartTim, cardMeg.data.StopTim, cardMeg.data.Weekday ))
            {
                return eAPDUMatchFail;
            }
            #else
            if(RTC_Successfully != HAL_RTC_TimeIsTimesize(&cardMeg.data.tm_vaild.start,
														&cardMeg.data.tm_vaild.stop,
														cardMeg.data.tm_vaild.wday))
            {
                return eAPDUMatchFail;
            }
            #endif
        }
    }
    else
    {
        return eAPDUMatchFail;
    }
    err = ApduTransceive( (uint8_t*)APDUFINDSIMCARD, sizeof(APDUFINDSIMCARD), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-APDUSelectMF OK.\r\n");
        ISO_14443A_DBG_EXT((uint8_t*)APDUSelectMF,sizeof(APDUSelectMF)," TX:");
        ISO_14443A_DBG_EXT(buf,ilength," RX:");
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result )
        {
#ifdef DEBUG_LOG_ENABLE
            my_printf("CPU card>APDU-APDUSelectMF\r\n FAIL.[%d]\r\n",err);
#endif
            return eAPDUMatchFail; //ѡ��MF�ļ�����
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUMatchFail; //ѡ��MF�ļ�����;
    }
    err = ApduTransceive( (uint8_t*)APDUGETSEID, sizeof(APDUGETSEID), buf, &ilength );
    if( err == eAPDUSuccess )
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-GetRandom OK.\r\n");
        ISO_14443A_DBG_EXT( (uint8_t*)APDUGetRandom, sizeof(APDUGetRandom), " TX:" );
        ISO_14443A_DBG_EXT( buf, ilength, " RX:" );
#endif
        uint8_t result = check_apdu_result( &buf[ilength-2] );
        if( result == 0 )
        {
            (void)memcpy( SEIDBUF, &buf[0], 15);
        }
        else
        {
            return eAPDUMatchFail;
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-GetRandom\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUMatchFail;
    }

    for(int i=0; i<5; i++) //�ƶ�SIM�����ع̶����� 0440C930A+SEID
    {
        if(SEIDBUF[i]==headbuf[i])
        {
        }
        else
        {
            return eAPDUMatchFail;
        }
    }
    for(int i=0; i<10; i++) //�Ƚ�SEID
    {
        #if 0
        if(cardMeg.data.SEID[i]==SEIDBUF[5+i])
        {

        }
        else
        {
            return eAPDUMatchFail;
        }
        #endif

    }

    memcpy(pCardMeg->tab,cardMeg.tab,sizeof(cardMeg.tab));

    /*--------THE END-----------------*/
    my_printf("/*---------Add cpu card stop!-----------*/\r\n");

    return err;

}



/***************************************************************************************
**������:       CpuCardEnrollPro
**��������:     ע������
**�������:     CpuCardLim���û�Ȩ�ޣ�UserID���û����
**�������:     �����������
**��ע:         
****************************************************************************************/
uint8_t CpuCardEnrollPro(uint8_t CpuCardLim, uint16_t UserID) 
{
    CARD_MEG_Def *pCpuCardMeg = &CpuCardMessage;

//	static uint8_t firstTime;
	uint8_t checkAidCnt = 0;
	int8_t  nfcCheckRet = 0;
	CARD_MEG_Def  cpuCardMeg ={0};
//	uint32_t addr = 0;
	uint32_t cpuCardId=0;
    
    
    int8_t result = DRV_MFRC522_ReadCardId(1);
    if( 1 == result )       //Ѱ���ɹ�
    {
        /*--------ȷ���������ް󶨸ÿ�--------------*/
        uint32_t address =0;
        CARD_MEG_Def  cardMeg={0};
        cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
        if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, &cardMeg ))  
        {
            HAL_Voice_PlayingVoice(EM_CARD_REGISTED_MP3,1000); //����ע��
            DRV_MFRC522_PcdAntennaOff();
            DRV_MFRC522_Poweroff();
            return CPUCARD_ADD_REGISTERED;
        }
        /*----------------------*/
        else
        {
            uint8_t ret_value=1;
            ret_value=App_CpuCard_FlieInit();
            if(ret_value)//��ʼ��ʧ�ܣ���ǰ��������
            {
                nfcCheckRet=-1;
            }
            #ifdef P1_CM_LOCK 
            if( eAPDUSuccess == App_SIMCard_ApduRegistProcess( &cpuCardMeg ))
            #else
            if( eAPDUSuccess == App_CpuCard_ApduRegistProcess( &cpuCardMeg ) )
            #endif
            {
                checkAidCnt = 0;
                nfcCheckRet = 1;
            }
            else 
            {
                 checkAidCnt++;
                 if( checkAidCnt >= 1 )  //�Ƿ�����
                 {
                    checkAidCnt = 0;
                    nfcCheckRet = -1;
                 }
            }
        }
    }
    else if( -1 == result ) //Ѱ��ʧ��
    {
        nfcCheckRet = -1;
    }
    DRV_MFRC522_PcdAntennaOff();
    DRV_MFRC522_Poweroff();
    if( 1 == nfcCheckRet )       //�鿨�ɹ�
    {
        pCpuCardMeg->data.PageId = Cloud_SearchEmptyCpuCardPageIdFromEeprom();
        pCpuCardMeg->data.CardId = cpuCardId;
        pCpuCardMeg->data.UserId = UserID;
        pCpuCardMeg->data.Privileges = CpuCardLim;
        pCpuCardMeg->data.UserValue = MEM_FACT_MEM_FIG;
        memcpy( pCpuCardMeg->data.ExternAuthenPublicKey, cpuCardMeg.data.ExternAuthenPublicKey, DF_EXTERN_KEY_SIZE );
        memcpy( pCpuCardMeg->data.InnerAuthenPublicKey, cpuCardMeg.data.InnerAuthenPublicKey, DF_INSIDE_KEY_SIZE );
        
//        memcpy(pCpuCardMeg->data.SEID , cpuCardMeg.data.SEID,10);
        Cloud_SaveOneCpuCardMegIntoEeprom( pCpuCardMeg );		 //ע��һ��CPU��Ƭ��Ϣ

        SystemEventLogSave( ADD_CARD, pCpuCardMeg->data.CardId );
        
        HAL_Voice_PlayingVoice(EM_REGISTER_SUCCESS_MP3, 1000); //�Ǽǳɹ�
        return CPUCARD_ADD_SUCCESSFUL;
    }
    else if(nfcCheckRet == -1)
    {
        HAL_Voice_PlayingVoice(EM_REGISTER_FAIL_MP3, 1000); //�Ǽ�ʧ��
        return CPUCARD_ADD_ERROR;
    }
    return CPUCARD_FINDING_ERROR;
}


/***************************************************************************************
**������:       CpuCardGetVeifyState
**��������:     ʶ������
**�������:     Pageid�������ڴ洢ҳ
**�������:     ����0δѰ������1�ɹ���2��֤ʧ��
**��ע:         
****************************************************************************************/
uint8_t CpuCardGetVeifyState(uint16_t *Pageid)
{
    int8_t  nfcCheckRet = 0;
    uint8_t ret = 0;
    CARD_MEG_Def   *pCpuCardMeg = &CpuCardMessage; 
    
    uint8_t result = DRV_MFRC522_ReadCardId(1);
    if( 1 == result )       //Ѱ���ɹ�
    {
        uint8_t ret_value=1;
        ret_value=App_CpuCard_FlieInit();
        if(ret_value)//��ʼ��ʧ�ܣ���ǰ��������
        {
            nfcCheckRet=-1;
        }
        else
        {
            uint32_t cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
            my_printf("cpuCardId=%ld\n",cpuCardId);
            
            if( eAPDUSuccess == App_CpuCard_ApduIdentifyProcess( cpuCardId, pCpuCardMeg ))
            {
                nfcCheckRet = 1;	
            }
            else 
            {
                nfcCheckRet = -1;	
            }
            result=0;
        }	
    }
    else
         DRV_MFRC522_Poweroff();
    if(nfcCheckRet==1)//CPU����֤�ɹ�
    {
        *Pageid = pCpuCardMeg->data.PageId;
        DRV_MFRC522_Poweroff();
        ret = 1;//ֱ�ӷ���.
    }
    else if(nfcCheckRet==-1)//��֤ʧ��
    {
        DRV_MFRC522_Poweroff();
        ret = 2;//ֱ�ӷ���.
    }
    return ret;
}

/***************************************************************************************
**������:       CpuCardEepromEmpty
**��������:     ���CPU��
**�������:     
**�������:     
**��ע:         
****************************************************************************************/
void CpuCardEepromEmpty(void)
{
    Cloud_ClearAllCpuCardMegFromEeprom();
}


/***************************************************************************************
**������:       CpuCardDeleteID
**��������:     ���ɾ��CPU��
**�������:     UserID���û����
**�������:     
**��ע:         
****************************************************************************************/
uint8_t CpuCardDeleteID(uint16_t UserID)
{
    uint32_t addr;
    uint8_t ret = 0;
    CARD_MEG_Def *pCpuCardMeg = &CpuCardMessage;
    memset( pCpuCardMeg->tab, 0, MSG_CPU_CARD_REG_ONE_SIZE );
    
    if( 1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, UserID, &addr, pCpuCardMeg ) )  //���ز�ѯ���� 
    {
        Cloud_DelcCpuCardegFromEeprom( addr );
//	    App_Wifi_CommonTx( CMD_UploadUserTabMeg );  //�����û��б���Ϣ
        //�¼���¼
        SystemEventLogSave( DELETE_CARD, pCpuCardMeg->data.CardId );
        ret = 1; //ɾ���ɹ�
    }
    else
    {
       ret = 2; //ɾ��ʧ��
    }
    return ret;
}


/***************************************************************************************
**������:       CpuCardDeleteComparison
**��������:     �ȶ�ɾ��CPU��
**�������:     
**�������:     ����0��δѰ������1�ɹ���2ʧ��
**��ע:         
****************************************************************************************/
uint8_t CpuCardDeleteComparison(void)
{
    uint8_t ret = 0;
    CARD_MEG_Def *pCpuCardMeg = &CpuCardMessage;
    memset( pCpuCardMeg->tab, 0, MSG_CPU_CARD_REG_ONE_SIZE );
    
	uint8_t result = DRV_MFRC522_ReadCardId(1);
	if( 1 == result )       //Ѱ���ɹ�
	{
		uint8_t ret_value=1;
		ret_value=App_CpuCard_FlieInit();
		if(ret_value==0)//��ʼ��ʧ�ܣ���ǰ��������
		{
			uint32_t cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
			my_printf("cpuCardId=%ld\n",cpuCardId);
			uint32_t address =0;
			if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, pCpuCardMeg ))
			{
				Cloud_DelcCpuCardegFromEeprom( address );
//				FLM_RecordLockLog();
                SystemEventLogSave( DELETE_CARD, pCpuCardMeg->data.CardId );

                ret = 1; //ɾ���ɹ�
			} 
			else 
			{
                ret = 2; //ɾ��ʧ��
			}
		}
		else 
		{
            ret = 2; //ɾ��ʧ��
		}
	}
	DRV_MFRC522_Poweroff();
    return ret;
}

/***************************************************************************************
**������:       App_CpuCard_SleepInit
**��������:     ���ߺ���
**�������:     
**�������:     
**��ע:         
****************************************************************************************/
void App_CpuCard_SleepInit(void)
{
    DRV_MFRC522_GpioSleep();
}

#endif

