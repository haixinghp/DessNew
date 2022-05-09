/********************************************************************************************************************
 * @file:      App_CpuCard.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2020-11-14
 * @brief:     CPU卡功能函数   复旦微卡
 * @changlist:  2020-11-18  实现复旦微卡的配卡和验卡功能
*********************************************************************************************************************/



/*-------------------------------------------------文件包含---------------------------------------------------------*/
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

/*-------------------------------------------------宏定义-----------------------------------------------------------*/


/*-------------------------------------------------枚举定义---------------------------------------------------------*/


/*-------------------------------------------------常量定义---------------------------------------------------------*/
static const uint8_t APDUSelectMF[] = {0x00,0xA4,0x00, 0x00, 0x00}; // 选择主目录
static const uint8_t APDUGetRandom[] = {0x00,0x84,0x00,0x00,0x04};  // 获取随机数
static uint8_t APDUGetExternalAuthentication[] = {0x00,0x82,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // 外部认证
static const uint8_t APDUEmptyCard[] = {0x80,0x0E,0x00,0x00,0x00};  // 清空卡片
static const uint8_t APDUCreateMFKeyFile[] = {0x80,0xE0,0x00,0x00,0x07,0x3F,0x00,0xB0,0x01,0xFA,0xFF,0xFF}; // 建立MF密钥文件
static const uint8_t APDUAddMFKeyFilePwd[] = {0x80,0xD4,0x01,0x00,0x0D,0x39,0xF0,0xF0,0xAA,0x55,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; // 密钥文件建立好给其增加8字节密钥
static const uint8_t APDUCreateDF[] = {0x80,0xE0,0x3F,0x01,0x0D,0x38,0x05,0x20,0xFA,0xFA,0x95,0xFF,0xFF,0x44,0x44,0x46,0x30,0x31}; // 建立目录 DF文件
static const uint8_t APDUSelectDF[] = {0x00,0xA4,0x00,0x00,0x02,0x3F,0x01}; // 选择DF目录
static const uint8_t APDUCreateDFKeyFile[] = {0x80,0xE0,0x00,0x00,0x07,0x3F,0x01,0x8F,0x95,0xFA,0xFF,0xFF}; // 建立DF文件下的密钥文件
static uint8_t APDUAddDFExternalKey[] = {0x80,0xD4,0x01,0x00,0x0D,0x39,0xF0,0xFA,0xAA,0x55,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08}; // 添加外部认证密钥 8字节
static uint8_t APDUAddDFInsideKey[] = {0x80,0xD4,0x01,0x00,0x0D,0x30,0xF0,0xF0,0xAA,0x33,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08}; 	 // 添加内部认证密钥 8字节

static const uint8_t APDUCreateBin[] = {0x80,0xE0,0x00,0x03,0x07,0x28,0x00,0x1E,0xFA,0xFA,0xFF,0xFF}; // 建立二进制文件
static const uint8_t APDUSelectBin[] = {0x00,0xA4,0x00,0x00,0x02,0x00,0x03}; 													// 选择要操作的二进制文件
static uint8_t APDUWriteBin[] = {0x00,0xD6,0x00,0x00,0x06,0x01,0x02,0x03,0x04,0x05,0x6}; 							// 写二进制数据文件
static const uint8_t APDUReadBin[] = {0x00,0xB0,0x00,0x00,0x00,0x00}; 																// 读取二进制文件
static uint8_t APDUGetInsideAuthentication[] = {0x00,0x88,0x00,0x00,0x08,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88}; // 内部认证8字节，随机数

static uint8_t MFFactorySetkey[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t DFExternalKey[ DF_EXTERN_KEY_SIZE ] = { 0 };  //外部认证密钥
static uint8_t DFInsideKey[ DF_INSIDE_KEY_SIZE ]   = { 0 };  //内部认证密钥


static const uint8_t APDUFINDSIMCARD[] = {0x00,0xA4,0x04, 0x00, 0x00}; // 确认卡片类型
static const uint8_t APDUGETSEID[] = {0x80,0xCA,0x00, 0x44, 0x00}; // 确认卡片类型

/*-------------------------------------------------局部变量定义-----------------------------------------------------*/


/*-------------------------------------------------全局变量定义-----------------------------------------------------*/
CARD_MEG_Def  CpuCardMessage;

/*-------------------------------------------------函数声明---------------------------------------------------------*/
uint8_t check_apdu_result( uint8_t *data );

/*-------------------------------------------------函数定义---------------------------------------------------------*/

/*********************************************************************************************************************
* Function Name :  Cloud_ChangeCpuCardPageidToEepromAddr
* Description   :  获得对应的EEPROM地址
* Para          :
* Return        :  对应的Pageid
*********************************************************************************************************************/
static uint32_t Cloud_ChangeCpuCardPageidToEepromAddr( uint16_t pageid )
{
    uint32_t addr;
    addr = MSG_CPU_CARD_REG_START + pageid * MSG_CPU_CARD_ONE_SIZE;
    return addr;
}


/*********************************************************************************************************************
* Function Name :  CpuCard_QueryUserCpuCardMegFromEeprom
* Description   :  查询EEPROM中是否有对应的cpu卡用户编号
* Para          :  type - 输入编号类型 USER_ID -用户编号  PAGE_ID - 手机在EEPROM内部编号
                   numId - cpu卡用户编号 / cpu卡在EEPROM编号
                   paddr - 此编号对应的EEPROM地址  pIphoneMeg-对应编号手机信息指针
* Return        :  0-未找到  1- 已找到
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

            if( type == USER_ID )       //编号用户
            {
                if( pCardMeg->data.UserId == numId )  //找到了对应的ID
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

            if( type == CARD_ID )  //cpu卡ID
            {
                if( pCardMeg->data.CardId == numId )  //找到了对应的ID
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
* Description   :  查询EEPROM中空的cpu卡对应的Pageid
* Para          :      
* Return        :  对应的Pageid
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
        if( SaveFlag != MEM_FACT_MEM_FIG )//指纹存在标志.
        {
            return pageid; //return empty pageid
        }
    }
    return pageid;
}

/*********************************************************************************************************************
* Function Name :  Cloud_SaveOneCpuCardMegIntoEeprom
* Description   :  将单个cpu卡信息保存于EEPROM中
* Para          :  pIphoneMeg - 手机智卡信息指针
* Return        :  none
*********************************************************************************************************************/
static void Cloud_SaveOneCpuCardMegIntoEeprom( CARD_MEG_Def *pCardMeg )
{
    my_printf("Cloud_SaveOneCpuCardMegIntoEeprom()\n");
    uint16_t pageid;
    uint32_t addr;
//    uint32_t cpuCardId=0;
    
    //CPU个数记录
    SystemSeting.SysCardAllNum++;
    SystemWriteSeting(&SystemSeting.SysCardAllNum,1);
    
    pageid = Cloud_SearchEmptyCpuCardPageIdFromEeprom(); 

    addr = Cloud_ChangeCpuCardPageidToEepromAddr( pageid );
    HAL_EEPROM_WriteBytes(addr,(uint8_t *)&pCardMeg->tab, MSG_CPU_CARD_REG_ONE_SIZE); 

}

/*********************************************************************************************************************
* Function Name :  Cloud_DelcCpuCardegFromEeprom
* Description   :  删除EEPROM中对地址的cpu卡
* Para          :  addr - 待删除手机的地址   
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
* Description   :  清空EEPROM中的所有cpu卡
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
* Description   :  相关文件初始化
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
            err =12;//进入RTAS失败
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
* Description   :  比较两个数组是否一样
* Para Input    :  pSbuf- 数组1  pDbuf- 数组2   length- 待比较的数长度
* Para Output   :  none
* Return        :  0= 不一致  1= 一致
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
* Description   :  和cpu卡的注册流程
* Para Intput   :
* Para Output   :
* Return        :  0- 成功  other- 失败
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_CpuCard_ApduRegistProcess( CARD_MEG_Def *pCardMeg )
{
    uint8_t  err = eAPDUSuccess;			//返回状态。
    uint8_t  buf[270]; 								//获取apdu缓存
    uint16_t ilength;  								//apdu获取到数据到饿长度
    uint8_t  Random_buf[8] = {0}; 		//随机数缓存
    uint8_t  cipher_buf[24]= {0};
//	CARD_MEG_Def  carMeg = {0};

//    /*--------确认锁端是否绑定该卡-----*/
//	uint32 address =0;
//	CARD_MEG_Def  cardMeg={0};
//	uint32 cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
//	if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, &cardMeg ))
//	{
//			Cloud_DelcCpuCardegFromEeprom( address );
//	}
    my_printf("/*---------Add cpu card start!-----------*/\r\n");
    /*--------选择主目录--------------*/
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
            return eAPDUFailSelectMF; //选择MF文件错误
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUFailSelectMF; //选择MF文件错误;
    }

    /*--------获取随机数--------------*/
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

    /*--------外部认证----------------*/
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
    memcpy( &APDUGetExternalAuthentication[5], cipher_buf, 8 ); //填写加密数据
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
            return eAPDUFailGetExternalAuthentication; //注册时外部认证错误，可以继续
#endif
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf(">APDU-iAPDU_External_authenticate\r\n FAIL.[%d]\r\n",err);
#endif
#if 0
        return eAPDUFailGetExternalAuthentication;  //注册时外部认证错误，可以继续
#endif
    }

    /*--------清空卡片----------------*/
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

    /*--------建立MF密钥文件----------*/
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

    /*--------密钥文件增加密钥--------*/
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

    /*--------建立目录DF文件----------*/
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

    /*--------选择DF文件目录----------*/
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

    /*--------建立DF文件下的密钥文件--*/
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

    /*--------添加外部认证密钥--------*/
    // (void)random_vector_generate( DFExternalKey, DF_EXTERN_KEY_SIZE );//随机数8个
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

    /*--------添加内部认证密钥--------*/
    // (void)random_vector_generate( DFInsideKey, DF_INSIDE_KEY_SIZE );//随机数8个
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

    /*--------建立二进制文件----------*/
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

    /*--------选择要操作的二进制文件--*/
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

    /*--------写二进制数据文件（写入锁的MAC）--------*/
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

    /*--------锁端保存卡的相关信息----*/
    memcpy( pCardMeg->data.ExternAuthenPublicKey, DFExternalKey, DF_EXTERN_KEY_SIZE );
    memcpy( pCardMeg->data.InnerAuthenPublicKey, DFInsideKey, DF_INSIDE_KEY_SIZE );
//		pCardMeg->data.CardId = cpuCardId;

    /*--------THE END-----------------*/
    my_printf("/*---------Add cpu card stop!-----------*/\r\n");

    return err;
}

/*********************************************************************************************************************
* Function Name :  App_CpuCard_ApduRegistProcess()
* Description   :  和cpu卡的交互解绑流程
* Para Intput   :  cardId- 卡号
* Para Output   :  pCardMeg- 输出卡片信息
* Return        :  0- 成功  other- 失败
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_CpuCard_ApduIdentifyProcess( uint32_t cardId, CARD_MEG_Def *pCardMeg )
{
    uint8_t  err = eAPDUSuccess;			//返回状态。
    uint8_t  buf[270]; 								//获取apdu缓存
    uint16_t ilength;  								//apdu获取到数据到饿长度
    uint8_t  Random_buf[ 8 ] = { 0 }; //随机数缓存
    uint8_t  cipher_buf[24]= {0};

    /*--------确认锁端是否绑定该卡------*/
    uint32_t address =0;
    CARD_MEG_Def  cardMeg= {0};
    if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cardId, &address, &cardMeg ))
    {
        if( cardMeg.data.TimelinessState == 1 )  //时效开启
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
    /*--------选择DF文件--------------*/
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

    /*--------获取随机数--------------*/
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

    /*--------外部验证----------------*/
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

    /*--------内部认证随机数----------*/
    uint8_t insideRadomData[8];
    // (void)random_vector_generate( insideRadomData, sizeof insideRadomData );//随机数8个
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

    /*--------选择二进制文件----------*/
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

    /*--------读取二进制文件----------*/
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
            
            if( 0 == Api_Math_CompareBufByte( buf, lockMac, 6 ) )  // 对比读取到的文件内容
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
    /*--------输出卡片信息-----------*/
    memcpy( pCardMeg->tab, cardMeg.tab, sizeof cardMeg.tab );

    /*--------THE END----------------*/
    my_printf("/*---------Check cpu card stop!-----------*/\r\n");
    return err;
}
/*********************************************************************************************************************
* Function Name :  check_apdu_result
* Description   :  获取应答的结果并判断是否正确
* Para          :  data- 接收到的数据
* Return        :  0：操作成功  其他值：操作失败
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
//* Description   :  解绑CPU卡流程
//* Para Intput   :
//* Para Output   :
//* Return        :  0- 成功  other- 失败
//* author				:  gushengchi
//*********************************************************************************************************************/
//static uint8_t App_CpuCard_ApduUnregistProcess( uint8 userLimit, TimValueType_Enum timeValueType, CARD_MEG_Def *pCardMeg )
//{
//    uint8_t  err = eAPDUSuccess;			//返回状态。
//    uint8_t  buf[270]; 								//获取apdu缓存
//    uint16_t ilength;  								//apdu获取到数据到饿长度
//	uint8_t  Random_buf[8] = {0}; 		//随机数缓存
//	uint8_t  cipher_buf[24]= {0};
//
//		my_printf("/*---------Empty cpu card start!-----------*/\r\n");
//		/*--------选择主目录--------------*/
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
//						return eAPDUFailSelectMF; //选择MF文件错误
//				}
//		}
//		else
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
//				#endif
//				return eAPDUFailSelectMF; //选择MF文件错误;
//		}
//
//		/*--------获取随机数--------------*/
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
//		/*--------外部认证----------------*/
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
//    memcpy( &APDUGetExternalAuthentication[5], cipher_buf, 8 ); //填写加密数据
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
//						return eAPDUFailGetExternalAuthentication; //注册时外部认证错误，可以继续
//						#endif
//				}
//		}
//		else
//		{
//				#ifdef DEBUG_LOG_ENABLE
//				my_printf(">APDU-iAPDU_External_authenticate\r\n FAIL.[%d]\r\n",err);
//				#endif
//				#if 0
//				return eAPDUFailGetExternalAuthentication;  //注册时外部认证错误，可以继续
//				#endif
//		}
//
//		/*--------清空卡片----------------*/
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
* Description   :  和超级SIM卡的注册流程
* Para Intput   :
* Para Output   :
* Return        :  0- 成功  other- 失败
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_SIMCard_ApduRegistProcess(CARD_MEG_Def *pCardMeg )
{
    static uint8_t SEIDBUF[20]= {0};
    uint8_t  err = eAPDUSuccess;			//返回状态。
    uint8_t  buf[255]; 								//获取apdu缓存
    uint8_t  headbuf[5]= {0x00,0x44,0x0C,0x93,0x0A};
    uint16_t ilength;  		//apdu获取到数据到饿长度
//	/*--------确认锁端有无绑定该卡--------------*/
//	uint32 address =0;
//	CARD_MEG_Def  cardMeg={0};
//	uint32 cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
//	if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, &cardMeg ))
//	{
//		Cloud_DelcCpuCardegFromEeprom( address );
//	}
    my_printf("/*---------Add SIM card start!-----------*/\r\n");
    /*--------选择主目录--------------*/
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
            return eAPDUMatchFail; //选择MF文件错误
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUMatchFail; //选择MF文件错误;
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

    for(int i=0; i<5; i++) //移动SIM卡返回固定数据 0440C930A+SEID
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
* Description   :  和超级SIM卡的验证流程
* Para Intput   :
* Para Output   :
* Return        :  0- 成功  other- 失败
* author				:  gushengchi
*********************************************************************************************************************/
static uint8_t App_SIMCard_ApduIdentifyProcess( uint32_t cardId, CARD_MEG_Def *pCardMeg )
{
    static uint8_t SEIDBUF[20]= {0};
    uint8_t  err = eAPDUSuccess;			//返回状态。
    uint8_t  buf[255]; 								//获取apdu缓存
    uint8_t  headbuf[5]= {0x00,0x44,0x0C,0x93,0x0A};
    uint16_t ilength;  		//apdu获取到数据到饿长度
    /*--------确认锁端是否绑定该卡------*/
    uint32_t address =0;
    CARD_MEG_Def  cardMeg= {0};
    if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cardId, &address, &cardMeg ))
    {
        if( cardMeg.data.TimelinessState == 1 )  //时效开启
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
            return eAPDUMatchFail; //选择MF文件错误
        }
    }
    else
    {
#ifdef DEBUG_LOG_ENABLE
        my_printf("CPU card>APDU-SelectMF\r\n FAIL.[%d]\r\n",err);
#endif
        return eAPDUMatchFail; //选择MF文件错误;
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

    for(int i=0; i<5; i++) //移动SIM卡返回固定数据 0440C930A+SEID
    {
        if(SEIDBUF[i]==headbuf[i])
        {
        }
        else
        {
            return eAPDUMatchFail;
        }
    }
    for(int i=0; i<10; i++) //比较SEID
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
**函数名:       CpuCardEnrollPro
**功能描述:     注册流程
**输入参数:     CpuCardLim：用户权限，UserID：用户编号
**输出参数:     输出：故障码
**备注:         
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
    if( 1 == result )       //寻卡成功
    {
        /*--------确认锁端有无绑定该卡--------------*/
        uint32_t address =0;
        CARD_MEG_Def  cardMeg={0};
        cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
        if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, &cardMeg ))  
        {
            HAL_Voice_PlayingVoice(EM_CARD_REGISTED_MP3,1000); //卡已注册
            DRV_MFRC522_PcdAntennaOff();
            DRV_MFRC522_Poweroff();
            return CPUCARD_ADD_REGISTERED;
        }
        /*----------------------*/
        else
        {
            uint8_t ret_value=1;
            ret_value=App_CpuCard_FlieInit();
            if(ret_value)//初始化失败，提前结束进程
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
                 if( checkAidCnt >= 1 )  //非法攻击
                 {
                    checkAidCnt = 0;
                    nfcCheckRet = -1;
                 }
            }
        }
    }
    else if( -1 == result ) //寻卡失败
    {
        nfcCheckRet = -1;
    }
    DRV_MFRC522_PcdAntennaOff();
    DRV_MFRC522_Poweroff();
    if( 1 == nfcCheckRet )       //验卡成功
    {
        pCpuCardMeg->data.PageId = Cloud_SearchEmptyCpuCardPageIdFromEeprom();
        pCpuCardMeg->data.CardId = cpuCardId;
        pCpuCardMeg->data.UserId = UserID;
        pCpuCardMeg->data.Privileges = CpuCardLim;
        pCpuCardMeg->data.UserValue = MEM_FACT_MEM_FIG;
        memcpy( pCpuCardMeg->data.ExternAuthenPublicKey, cpuCardMeg.data.ExternAuthenPublicKey, DF_EXTERN_KEY_SIZE );
        memcpy( pCpuCardMeg->data.InnerAuthenPublicKey, cpuCardMeg.data.InnerAuthenPublicKey, DF_INSIDE_KEY_SIZE );
        
//        memcpy(pCpuCardMeg->data.SEID , cpuCardMeg.data.SEID,10);
        Cloud_SaveOneCpuCardMegIntoEeprom( pCpuCardMeg );		 //注册一张CPU卡片信息

        SystemEventLogSave( ADD_CARD, pCpuCardMeg->data.CardId );
        
        HAL_Voice_PlayingVoice(EM_REGISTER_SUCCESS_MP3, 1000); //登记成功
        return CPUCARD_ADD_SUCCESSFUL;
    }
    else if(nfcCheckRet == -1)
    {
        HAL_Voice_PlayingVoice(EM_REGISTER_FAIL_MP3, 1000); //登记失败
        return CPUCARD_ADD_ERROR;
    }
    return CPUCARD_FINDING_ERROR;
}


/***************************************************************************************
**函数名:       CpuCardGetVeifyState
**功能描述:     识别流程
**输入参数:     Pageid：卡所在存储页
**输出参数:     返回0未寻到卡，1成功，2验证失败
**备注:         
****************************************************************************************/
uint8_t CpuCardGetVeifyState(uint16_t *Pageid)
{
    int8_t  nfcCheckRet = 0;
    uint8_t ret = 0;
    CARD_MEG_Def   *pCpuCardMeg = &CpuCardMessage; 
    
    uint8_t result = DRV_MFRC522_ReadCardId(1);
    if( 1 == result )       //寻卡成功
    {
        uint8_t ret_value=1;
        ret_value=App_CpuCard_FlieInit();
        if(ret_value)//初始化失败，提前结束进程
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
    if(nfcCheckRet==1)//CPU卡验证成功
    {
        *Pageid = pCpuCardMeg->data.PageId;
        DRV_MFRC522_Poweroff();
        ret = 1;//直接返回.
    }
    else if(nfcCheckRet==-1)//验证失败
    {
        DRV_MFRC522_Poweroff();
        ret = 2;//直接返回.
    }
    return ret;
}

/***************************************************************************************
**函数名:       CpuCardEepromEmpty
**功能描述:     清空CPU卡
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void CpuCardEepromEmpty(void)
{
    Cloud_ClearAllCpuCardMegFromEeprom();
}


/***************************************************************************************
**函数名:       CpuCardDeleteID
**功能描述:     编号删除CPU卡
**输入参数:     UserID：用户编号
**输出参数:     
**备注:         
****************************************************************************************/
uint8_t CpuCardDeleteID(uint16_t UserID)
{
    uint32_t addr;
    uint8_t ret = 0;
    CARD_MEG_Def *pCpuCardMeg = &CpuCardMessage;
    memset( pCpuCardMeg->tab, 0, MSG_CPU_CARD_REG_ONE_SIZE );
    
    if( 1 == CpuCard_QueryUserCpuCardMegFromEeprom( USER_ID, UserID, &addr, pCpuCardMeg ) )  //本地查询到了 
    {
        Cloud_DelcCpuCardegFromEeprom( addr );
//	    App_Wifi_CommonTx( CMD_UploadUserTabMeg );  //推送用户列表信息
        //事件记录
        SystemEventLogSave( DELETE_CARD, pCpuCardMeg->data.CardId );
        ret = 1; //删除成功
    }
    else
    {
       ret = 2; //删除失败
    }
    return ret;
}


/***************************************************************************************
**函数名:       CpuCardDeleteComparison
**功能描述:     比对删除CPU卡
**输入参数:     
**输出参数:     返回0，未寻到卡，1成功，2失败
**备注:         
****************************************************************************************/
uint8_t CpuCardDeleteComparison(void)
{
    uint8_t ret = 0;
    CARD_MEG_Def *pCpuCardMeg = &CpuCardMessage;
    memset( pCpuCardMeg->tab, 0, MSG_CPU_CARD_REG_ONE_SIZE );
    
	uint8_t result = DRV_MFRC522_ReadCardId(1);
	if( 1 == result )       //寻卡成功
	{
		uint8_t ret_value=1;
		ret_value=App_CpuCard_FlieInit();
		if(ret_value==0)//初始化失败，提前结束进程
		{
			uint32_t cpuCardId = ((uint32_t)g_u8_CardInfo[2]<<24) + ((uint32_t)g_u8_CardInfo[3]<<16) + ((uint32_t)g_u8_CardInfo[4]<<8)+g_u8_CardInfo[5];
			my_printf("cpuCardId=%ld\n",cpuCardId);
			uint32_t address =0;
			if( CpuCard_QueryUserCpuCardMegFromEeprom( CARD_ID, cpuCardId, &address, pCpuCardMeg ))
			{
				Cloud_DelcCpuCardegFromEeprom( address );
//				FLM_RecordLockLog();
                SystemEventLogSave( DELETE_CARD, pCpuCardMeg->data.CardId );

                ret = 1; //删除成功
			} 
			else 
			{
                ret = 2; //删除失败
			}
		}
		else 
		{
            ret = 2; //删除失败
		}
	}
	DRV_MFRC522_Poweroff();
    return ret;
}

/***************************************************************************************
**函数名:       App_CpuCard_SleepInit
**功能描述:     休眠函数
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void App_CpuCard_SleepInit(void)
{
    DRV_MFRC522_GpioSleep();
}

#endif

