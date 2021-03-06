/***************************************************************************//**
 * @file
 *   c_Mfrc522.c
 * @brief
 *   MFRC522读卡
 *   属性：驱动层
 *
 * @author
 *   Haiqing.Jin
 * @version
 *   V1.0.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Haiqing.Jin, jjhhqq_2008@163.com</b>
 *******************************************************************************
 * @<author>            <time>          <version >          <desc>
 *   Haiqing            11/04/22        v1.0.0              build this moudle
 *
 ******************************************************************************/
#define MFRC522_GLOBALS

#include "h_Mfrc522.h"
//#include "Drive\Card\NZ\nz3801-a_com.h"
#include "SAFE_HWType.h"
#include "GUI.h"
#include "main.h"

#include "nrf_delay.h"
#include "drv_iic.h"

//#define SPI_DRIVER
#define IIC_DRIVER
/***************************************************************************//**
 * @brief
 *   定义常数
 *
 ******************************************************************************/
#define M_MAXRLEN 18
#define M_SYS_1US_TICKNUM 12
#define M_FN_NOP_DELAY() fn_Nop_Delay(10)

#define M_POWER_CARD_OUT_PP()		M_RC523_PW_OUT_PP()
#define M_POWER_CARD_OUT_OFF		M_RC523_PW_OUT_OFF
#define M_POWER_CARD_OUT_ON			M_RC523_PW_OUT_ON
#define M_MF522_SCK_OUT_PP()		M_SCK_OUT_PP()
#define M_MF522_SCK_IN_FL()		  M_SCK_IN_FL()
#define M_MF522_MOSI_OUT_PP()		M_MOSI_OUT_PP()
#define M_MF522_MOSI_IN_FL()		M_MOSI_IN_FL()
#define M_MF522_MISO_IN_UP()		M_MISO_IN_UP()
#define M_MF522_MISO_IN_FL()		M_MISO_IN_FL()
#define M_MF522_NSS_OUT_PP()		M_RC523_CS_OUT_PP()
#define M_MF522_NSS_IN_FL()	    M_RC523_CS_IN_FL()
#define M_MF522_SCK_OUT_OFF			M_SCK_FUN_OFF
#define M_MF522_SCK_OUT_ON			M_SCK_FUN_ON
#define M_MF522_NSS_OUT_OFF			M_RC523_CS_FUN_OFF
#define M_MF522_NSS_OUT_ON			M_RC523_CS_FUN_ON
#define M_MF522_MOSI_OUT_OFF		M_MOSI_FUN_OFF
#define M_MF522_MOSI_OUT_ON			M_MOSI_FUN_ON
#define M_MF522_MISO_IN_READ		M_MISO_IN_READ
#define M_MF522_RST_OUT_PP()    M_RF_RST_OUT_PP()
#define M_MF522_RST_OUT_ON      M_RF_RST_FUN_ON
#define M_MF522_RST_OUT_OFF     M_RF_RST_FUN_OFF






extern uint8 g_u8_CardInfo[14];//卡信息，2Byte卡类型 + 4Byte卡ID
//INT8U g_u8_CardInfo[8];//卡信息，2Byte卡类型 + 4Byte卡ID
#define M_AD_NUM 8
//#define M_DIFFERENCE_VALUE 0x0000000100000000
//#define M_SQUE_VALUE 0x01000
//#define M_ADD_VALUE 0x100
//#define M_ADD2_VALUE 0x080
#define M_DIFFERENCE_VALUE 0x0000000100000000
#define M_SQUE_VALUE 0x1000
#define M_ADD_VALUE 0x100
#define M_ADD2_VALUE 0x080
INT16U last_detect_lever[M_AD_NUM];
INT16U detect_lever[M_AD_NUM];
INT16U detect_avg_array[M_AD_NUM];
INT16U detect_avg;
INT16U detect_cnts;
INT8U u8_IgnoreTime = 0;

/***************************************************************************//**
 ****************************    LOCAL FUNCTION   ******************************
 ******************************************************************************/
/***************************************************************************//**
 * @brief
 *   延时函数。
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return
 *   none
 ******************************************************************************/

static void fn_Delayus(INT32U u32_Time)
{
	INT32U u32_i;
	for (; u32_Time>0; u32_Time--)
	{
		for (u32_i=M_SYS_1US_TICKNUM; u32_i>0; u32_i--)
		{
			__asm("NOP"); /* delay */
		}
//		nrf_delay_us(1);
	}
//	nrf_delay_us(u32_Time);
	
	
}

/***************************************************************************//**
 * @brief
 *   延时tm个NOP指令
 *
 * @param[in] INT16U tm
 *   延时NOP数
 *
 * @param[out]
 *   none
 *
 * @return[out]
 *   none
 ******************************************************************************/
static void fn_Nop_Delay(INT16U tm)
{
//       uint8 u8_i;

    for (;tm>0;tm--)
    {
//			nrf_delay_us(1);
			  __asm("NOP"); /* delay */
//			  __asm("NOP"); /* delay */
//			 __asm("NOP"); /* delay */
//			 __asm("NOP"); /* delay */
//        for (u8_i=1;u8_i>0;u8_i--)
//        {
//             __asm("NOP"); /* delay */
//        }
    }
}

void NZ_DelayMs(uint16 cn);
/***************************************************************************//**
 * @brief
 *   配置相关IO引脚
 *
 * @param[in]
 *   none
 *
 * @param[out]                                                  
 *   none
 *
 * @return[out]
 *   none
 ******************************************************************************/
 void fn_ConfigRc522Gpio(void)
{
#ifdef SPI_DRIVER
    M_POWER_CARD_OUT_PP();
	  M_POWER_CARD_OUT_ON;
	
//	  M_POWER_CARD_OUT_OFF;
	

    M_MF522_NSS_OUT_PP();
	
    M_MF522_SCK_OUT_PP();
//	  M_MF522_SCK_OUT_ON;
    M_MF522_MOSI_OUT_PP();
//  	M_MF522_MOSI_OUT_ON;
	
    M_MF522_MISO_IN_UP();
	
#endif
	
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/***************************************************************************//**
 * @brief
 *   寻卡
 *
 * @param[in] INT8U req_code
 *   寻卡方式
 *   0x52 = 寻感应区内所有符合14443A标准的卡
 *   0x26 = 寻未进入休眠状态的卡
 *
 * @param[out] INT8U *pTagType
 *   卡片类型代码
 *   0x4400 = Mifare_UltraLight
 *   0x0400 = Mifare_One(S50)
 *   0x0200 = Mifare_One(S70)
 *   0x0800 = Mifare_Pro(X)
 *   0x4403 = Mifare_DESFire
 *
 * @return[out] INT8S
 *   成功返回MI_OK，失败返回MI_ERR
 ******************************************************************************/
INT8S PcdRequest(INT8U req_code, INT8U *pTagType)
{
    INT8S status;
    INT16U  unLen;
    INT8U ucComMF522Buf[M_MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x07);
    SetBitMask(TxControlReg, 0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
//		my_printf("PcdRequest status=%d\n",status);
    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType     = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/***************************************************************************//**
 * @brief
 *   防冲撞
 *
 * @param[in]
 *   none
 *
 * @param[out] INT8U *pSnr
 *   卡片序列号，4字节
 *
 * @return[out] INT8S
 *   成功返回MI_OK，失败返回MI_ERR
 ******************************************************************************/
INT8S PcdAnticoll(INT8U *pSnr)
{
    INT8S status;
    INT8U i, snr_check = 0;
    INT16U  unLen;
    INT8U ucComMF522Buf[M_MAXRLEN];


    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80);
    return status;
}

/***************************************************************************//**
 * @brief
 *   选定卡片
 *
 * @param[in] INT8U *pSnr
 *   卡片序列号，4字节
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8S
 *   成功返回MI_OK，失败返回MI_ERR
 ******************************************************************************/
INT8S PcdSelect(INT8U *pSnr)
{
    INT8S status;
    INT8U i;
    INT16U  unLen;
    INT8U ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i+2] = *(pSnr + i);
        ucComMF522Buf[6]  ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/***************************************************************************//**
 * @brief
 *   验证卡片密码
 *
 * @param[in] INT8U auth_mode
 *   密码验证模式
 *   0x60 = 验证A密钥
 *   0x61 = 验证B密钥
 *
 * @param[in] INT8U addr
 *   块地址
 *
 * @param[in] INT8U *pKey
 *   密码
 *
 * @param[in] INT8U *pSnr
 *   卡片序列号，4字节
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8S
 *   成功返回MI_OK，失败返回MI_ERR
 ******************************************************************************/
INT8S PcdAuthState(INT8U auth_mode, INT8U addr, INT8U *pKey, INT8U *pSnr)
{
    INT8S status;
    INT16U  unLen;
    INT8U i, ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i+2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i+8] = *(pSnr + i);
    }

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
INT8S PcdRead(INT8U addr, INT8U *pData)
{
    INT8S status;
    INT16U  unLen;
    INT8U i, ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
INT8S PcdWrite(INT8U addr, INT8U *pData)
{
    INT8S status;
    INT16U  unLen;
    INT8U i, ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：扣款和充值
//参数说明: dd_mode[IN]：命令字
//               0xC0 = 扣款
//               0xC1 = 充值
//          addr[IN]：钱包地址
//          pValue[IN]：4字节增(减)值，低位在前
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
INT8S PcdValue(INT8U dd_mode, INT8U addr, INT8U *pValue)
{
    INT8S status;
    INT16U  unLen;
    INT8U i, ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pValue + i);
        }
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：备份钱包
//参数说明: sourceaddr[IN]：源地址
//          goaladdr[IN]：目标地址
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
INT8S PcdBakValue(INT8U sourceaddr, INT8U goaladdr)
{
    INT8S status;
    INT16U  unLen;
    INT8U ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status != MI_OK)
    {
        return MI_ERR;
    }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
INT8S PcdHalt(void)
{
    INT16U  unLen;
    INT8U ucComMF522Buf[M_MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(INT8U *pIndata, INT8U len, INT8U *pOutData)
{
    INT8U i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////

INT8S PcdReset()
{
	CARD_RESET=0;
	MBI5024_out (mbi5024_data, 1);
	fn_Delayus(100);
	
	
	fn_ConfigRc522Gpio();
    #ifdef SPI_DRIVER   
//	M_POWER_CARD_OUT_OFF;
//	fn_Delayus(2000);
	M_POWER_CARD_OUT_ON;
	fn_Delayus(2000);
    #endif
    return MI_OK;

}

/////////////////////////////////////////////////////////////////////
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////

INT8U ReadRawRC(INT8U Address)
{
	INT8U ucResult = 0;
	#ifdef IIC_DRIVER
	
		if(!iic1RecvByte(NZ3801_IIC_ADDR,Address,&ucResult))
		{
				my_printf("iic error!\r\n");
		} 
		#else
    INT8U i, ucAddr;
    
  	M_MF522_SCK_OUT_OFF;
	  M_FN_NOP_DELAY();
		M_MF522_NSS_OUT_OFF;
	    
    ucAddr = ((Address << 1) & 0x7E) | 0x80;
	
//		my_printf(">read ucAddr=%02x, ",ucAddr);
    M_FN_NOP_DELAY();
    for(i = 8; i > 0; i--)
    {
        if ((ucAddr & 0x80) == 0x80)
        {
            M_MF522_MOSI_OUT_ON;
        }
        else
        {
            M_MF522_MOSI_OUT_OFF;
        }
        M_FN_NOP_DELAY();
        M_MF522_SCK_OUT_ON;
        ucAddr <<= 1;
        M_FN_NOP_DELAY();
        M_MF522_SCK_OUT_OFF;
//        M_FN_NOP_DELAY();
    }

		M_FN_NOP_DELAY();
    for(i = 8; i > 0; i--)
    {
        M_MF522_SCK_OUT_ON;
        ucResult <<= 1;
        M_FN_NOP_DELAY();
        if (1 == M_MISO_IN_READ())
        {
            ucResult |= 0x01;
        }
        else
        {
            ucResult &= 0xFE;
        }

        M_MF522_SCK_OUT_OFF;
        M_FN_NOP_DELAY();
    }

		M_MF522_NSS_OUT_ON;
    M_FN_NOP_DELAY();
//    M_MF522_SCK_OUT_ON;
		M_MF522_SCK_OUT_OFF;
		
//		my_printf("ucResult=%02x\n",ucResult);
		
	#endif
    return ucResult;

}

/////////////////////////////////////////////////////////////////////
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(INT8U Address, INT8U value)
{
		#ifdef IIC_DRIVER
	
				if(!iic1SendByte(NZ3801_IIC_ADDR, Address, value))
        {
            my_printf("iic error!\r\n");
        }
	
	  #else
    INT8U i, ucAddr;
	  M_MF522_SCK_OUT_OFF;
	  M_FN_NOP_DELAY();
		M_MF522_NSS_OUT_OFF;
	
   
//    M_FN_NOP_DELAY();
    
//	  while(1);
    ucAddr = ((Address << 1) & 0x7E);
//  	my_printf("Address=%02x,value=%02x\n",Address,value);
    M_FN_NOP_DELAY();
    for(i = 8; i > 0; i--)
    {
        if ((ucAddr & 0x80) == 0x80)
        {
            M_MF522_MOSI_OUT_ON;
        }
        else
        {
            M_MF522_MOSI_OUT_OFF;
        }
        M_FN_NOP_DELAY();
        M_MF522_SCK_OUT_ON;
        ucAddr <<= 1;
        M_FN_NOP_DELAY();
        M_MF522_SCK_OUT_OFF;
//        M_FN_NOP_DELAY();
    }

		 M_FN_NOP_DELAY();
    for(i = 8; i > 0; i--)
    {
        if ((value & 0x80) == 0x80)
        {
            M_MF522_MOSI_OUT_ON;
        }
        else
        {
            M_MF522_MOSI_OUT_OFF;
        }
        M_FN_NOP_DELAY();
        M_MF522_SCK_OUT_ON;
        value <<= 1;
        M_FN_NOP_DELAY();
        M_MF522_SCK_OUT_OFF;
        M_FN_NOP_DELAY();
    }
		
   M_MF522_NSS_OUT_ON;
    M_FN_NOP_DELAY();
//    M_MF522_SCK_OUT_ON;
		M_MF522_SCK_OUT_OFF;
		#endif

}

/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(INT8U reg, INT8U mask)
{
    INT8S tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(INT8U reg, INT8U mask)
{
    INT8S tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
INT8S PcdComMF522(INT8U Command,
                  INT8U *pInData,
                  INT8U InLenByte,
                  INT8U *pOutData,
                  INT16U  *pOutLenBit)
{
    INT8S status = MI_ERR;
    INT8U irqEn   = 0x00;
    INT8U waitFor = 0x00;
    INT8U lastBits;
    INT8U n;
    INT16U i;
    switch (Command)
    {
				case PCD_AUTHENT:
						irqEn   = 0x12;
						waitFor = 0x10;
						break;
				case PCD_TRANSCEIVE:
						irqEn   = 0x77;
						waitFor = 0x30;
						break;
				default:
						break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    WriteRawRC(CommandReg, Command);


    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }

    i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if(!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > M_MAXRLEN)
                {
                    n = M_MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    SetBitMask(ControlReg, 0x80);          // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}


/////////////////////////////////////////////////////////////////////
//开启天线
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
    INT8U i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
		
    }
}


/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

/***************************************************************************//**
 * @brief
 *   设置读卡类型
 *
 * @param[in] INT8S u8_Type
 *   读卡类型
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8S
 *   成功返回MI_OK，失败返回MI_ERR
 ******************************************************************************/
INT8S fn_M500PcdConfigISOType(INT8S u8_Type)
{
    if ('A' == u8_Type)
    {
    	WriteRawRC(CommandReg, PCD_RESETPHASE);
    	M_FN_NOP_DELAY();
    	
		WriteRawRC(CommandReg, 0x00);
    	WriteRawRC(ModeReg, 0x3D);
		M_FN_NOP_DELAY();
    	M_FN_NOP_DELAY();
    	WriteRawRC(TReloadRegL, 30);
    	M_FN_NOP_DELAY();
    	WriteRawRC(TReloadRegH, 0);
    	M_FN_NOP_DELAY();
    	WriteRawRC(TModeReg, 0x8D);
    	M_FN_NOP_DELAY();
    	WriteRawRC(TPrescalerReg, 0x3E);
    	M_FN_NOP_DELAY();
    	WriteRawRC(TxAutoReg, 0x40);

		ClearBitMask(Status2Reg, 0x08);
		WriteRawRC(ModeReg, 0x3D);
		WriteRawRC(RxSelReg, 0x86);
		WriteRawRC(RFCfgReg, 0x7F);
		WriteRawRC(TReloadRegL, 30);
		WriteRawRC(TReloadRegH, 0);
		WriteRawRC(TModeReg, 0x8D);
		WriteRawRC(TPrescalerReg, 0x3E);		
		WriteRawRC(MifareReg, 0x10);//电源选择 0x10:3.3V   0x40:5V
    }
    else if ('B' == u8_Type)
    {
		WriteRawRC(CommandReg, 0x30);
    }
    else
    {
      return MI_ERR;
    }

    return MI_OK;
}
/***************************************************************************//**
 * @brief
 *   RC522芯片电源关闭
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return[out]
 *   none
 ******************************************************************************/
void fn_Rc522_Poweroff(void)
{
#ifdef SPI_DRIVER
	M_POWER_CARD_OUT_PP();
	M_POWER_CARD_OUT_OFF;  //关电源
  M_MF522_NSS_OUT_PP();
	M_MF522_NSS_OUT_ON;  //关SPI
	M_MISO_IN_FL();
	M_MOSI_FUN_OFF;
	M_SCK_OUT_PP();
#endif
}
/***************************************************************************//**
 * @brief
 *   RC522芯片休眠配置引脚
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return[out]
 *   none
 ******************************************************************************/
void fn_Rc522_GpioSleep(void)
{
#ifdef SPI_DRIVER
    M_POWER_CARD_OUT_OFF;

	M_MF522_NSS_OUT_PP();
    M_MF522_NSS_OUT_ON;  

    M_MISO_IN_FL();
    M_MOSI_FUN_OFF;
    M_SCK_FUN_OFF;
#endif
}

/***************************************************************************//**
 * @brief
 *   初始化Rc522。
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return
 *   成功返回1，失败返回0
 ******************************************************************************/
INT8U fn_Rc522_Init(void)
{	
   if (MI_OK == PcdReset())
	{
		fn_M500PcdConfigISOType('A');
		return (1);
	}
	return (0);
}



static void Delay_us(INT32U times)
{
    while(times--);
}



/***************************************************************************//**
 * @brief
 *   CLOCKOUT初始化
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return[out]
 *   none
 ******************************************************************************/




/***************************************************************************//**
 * @brief
 *   增加卡时得到一张S50卡卡号。
 *
 * @param[in] INT8U u8_times
 *   偿试次数
 *
 * @param[out]
 *   none
 *
 * @return INT8U
 *   成功返回1，失败返回0
 ******************************************************************************/
INT8U fn_ReadAddCardIdBak(INT8U u8_times)	
{
    u8_IgnoreTime = 1;//读卡后，忽略后面1次检测卡结果
    if (0 == fn_Rc522_Init())//初始化
    {
      return (0);//卡模块连接错误,初始化失败
			PcdAntennaOff();	
    }
			PcdAntennaOn();	
		  fn_Delayus(5000);
	while (1)
	{
		if (MI_OK == PcdRequest(0x52, &g_u8_CardInfo[0]))//请求卡
		{
			break;
		}
		u8_times--;
		if (0 == u8_times)
		{
		 PcdAntennaOff();	
			fn_Delayus(5000);
			return (0);
		}
	}

	for (u8_times=3; u8_times>0; u8_times--)
	{
		if (MI_OK == PcdAnticoll(&g_u8_CardInfo[2]))//防碰撞
		{
            if (MI_OK ==  PcdSelect(&g_u8_CardInfo[2]))
            {
                PcdHalt();
								PcdAntennaOff();	
							  fn_Delayus(5000);
                return (1);//读取成功
            }
		}
	}
		PcdAntennaOff();	
	return (0);
}

/***************************************************************************//**
 * @brief
 *   增加卡时得到一张S50卡卡号。
 *
 * @param[in] INT8U u8_times
 *   偿试次数
 *
 * @param[out]
 *   none
 *
 * @return INT8U
 *   成功返回1，失败返回0
 ******************************************************************************/
INT8U fn_ReadAddCardId(INT8U u8_times)	
{
	u8_IgnoreTime = 1;//读卡后，忽略后面1次检测卡结果
	while (1)
	{
		if (MI_OK == PcdRequest(0x26, &g_u8_CardInfo[0]))//请求卡
		{
			break;
		}
//		else{
//			my_printf("PcdRequest failed\n");
//		}
		u8_times--;
		if (0 == u8_times)
		{			
			return (0);
		}
	}

	for (u8_times=3; u8_times>0; u8_times--)
	{
		if (MI_OK == PcdAnticoll(&g_u8_CardInfo[2]))//防碰撞
		{
       return (1);//读取成功
		}
	}
	return (0);
}
/***************************************************************************//**
 * @brief
 *   得到一张S50卡卡号。
 *
 * @param[in] INT8U u8_TryTimes
 *   偿试次数
 *
 * @param[out]
 *   none
 *
 * @return INT8U
 *   成功返回1，失败返回0
 ******************************************************************************/

INT8U fn_ReadCardId(INT8U u8_TryTimes)	
{
		if (0 == fn_Rc522_Init())//初始化
    {
		my_printf("fn_Rc522_Init failed\n");
        return (0);//卡模块连接错误,初始化失败
    }
		 PcdAntennaOn();	
		 nrf_delay_ms(6);
    if (1 == fn_ReadAddCardId(u8_TryTimes))//防碰撞
    {			  	
			PcdAntennaOff();
//			nrf_delay_ms(6);
			nrf_delay_ms(100);
            return (1);//读取成功
    }	
		PcdAntennaOff();
		nrf_delay_ms(100);	
		return (0);	
}


uint8_t NFC_FuncBleWriteCard(void)
{
	uint8_t find_card = 0;
	uint16 card_num = 0;
	int i = 0;
	card_num = get_eeprom_card_num();
	//查询NFC卡存储是否已满
	if(card_num == 0)
	{
		return EEPROM_FULL;//返回NFC卡存储个数已满
	}
	
	for(i = 0;i<20;i++)
	{
		
		find_card = fn_ReadCardId(1);
		if (find_card == 1)
		{
			if (FLASHREG_PAGEID_NULL == Cloud_CheckMcardRegMEM()) //未注册
			{
				PcdAntennaOff();
				nrf_delay_ms(100);
				return MI_OK;
			}
			else
			{
				PcdAntennaOff();
				nrf_delay_ms(100);
				return CARD_EXISTED;
			}
		}
		nrf_delay_ms(500);
	}
	if(i >= 20)
	{
		PcdAntennaOff();
		nrf_delay_ms(100);
		my_printf("SEARCH_ERR \n");
	}
    return SEARCH_ERR;								// 返回寻卡错误
}

/***************************************************************************//**
 * @brief
 *   检测卡是否为本公司注册的有效卡
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8U
 *   有效卡1，其他情况返回0
 ******************************************************************************/
#define M_DESSMANN_CODE 0x44534D4E

typedef __packed struct
{
    __packed union 
    {
        __packed struct
        {
            INT32U u32_Check_Code;//酒店识别码
            INT32U u32_SerialNumber;//发卡流水号
            INT8U u8_CardFunction;//卡片功能识别码
            INT8U u8_CardType;//制卡类型 01为新卡，02为旧卡，03为补卡
            INT8U u8_RoomFloor;//楼栋
            INT8U u8_RoomLay;//楼层
            INT16U u16_RoomNum;//房间
            INT8U u8_CardCounter;//同房间发卡序号 某房间发出的第几张卡
            INT8U u8_CheckSum;//校验码 前15字节的累加和
        } st_Block0;
        INT8U a_u8_Block0[16];
    } un_Block0;
    __packed union
    {
        __packed struct
        {
            INT16U u16_StartYear;//时效开始时间
            INT8U u8_StartMonth;//月
            INT8U u8_StartDay;//日
            INT8U u8_StartHour;//时
            INT8U u8_StartMin;//分
            INT8U u8_StartSec;//秒
            INT16U u16_EndYear;//时效结束时间
            INT8U u8_EndMonth;//月
            INT8U u8_EndDay;//日
            INT8U u8_EndHour;//时
            INT8U u8_EndMin;//分
            INT8U u8_EndSec;//秒
            INT8U u8_Week;//时效,星期
            INT8U u8_CheckSum;//校验码 前15字节的累加和
        } st_Block1;
        INT8U a_u8_Block1[16];
    } un_Block1;
    __packed union
    {
        __packed struct
        {
            INT8U u8_Byte0;//预留
            INT8U u8_Byte1;//预留
            INT8U u8_Byte2;//预留
            INT8U u8_Byte3;//预留
            INT8U u8_Byte4;//预留
            INT8U u8_Byte5;//预留
            INT8U u8_Byte6;//预留
            INT8U u8_Byte7;//预留
            INT8U u8_Byte8;//预留
            INT8U u8_Byte9;//预留
            INT8U u8_Byte10;//预留
            INT8U u8_Byte11;//预留
            INT8U u8_Byte12;//预留
            INT8U u8_Byte13;//预留
            INT8U u8_Byte14;//预留
            INT8U u8_CheckSum;//校验码 前15字节的累加和
        } st_Block2;
        INT8U a_u8_Block2[16];
    } un_Block2;
    __packed union
    {
        __packed struct
        {
            INT8U a_u8_DessmannKeyA[6];//修改后的密钥
            INT8U a_u8_DessmannAccessBits[4];//访问控制
            INT8U a_u8_DessmannKeyB[6];//修改后的密钥
        } st_Block3;
        INT8U a_u8_Block3[16];
    } un_Block3;
} Types_WriteCardBlockType;

INT8U g_u8_NewCard = 0;//是否新卡?
INT8U u8_DefaultKeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/***************************************************************************//**
 * @brief
 *   检测卡是否为本公司注册的有效卡
 *
 * @param[in] INT8U u8_times
 *   偿试次数
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8U
 *   有效卡1，其他情况返回0
 ******************************************************************************/
INT8U fn_CheckCardKey(void)
{
    Types_WriteCardBlockType st_WriteCardBlockData;
    INT8U u8_CheckSum = 0;
    INT8U u8_i;

    if (0x04 != g_u8_CardInfo[0] || 0x00 != g_u8_CardInfo[1])//在此可以增加对其他卡的支持
    {
        return (1);
    }
  //  u8_times = 2;//尝试
    while(1)
    {
        if (MI_OK != PcdSelect(&g_u8_CardInfo[2]))
        {
      //      u8_times--;
  //          if (0 == u8_times)
            {
 //               return (0);
            }
        }
        else
        {
            break;
        }
    }
    
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[0] = 'D';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[1] = 'E';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[2] = 'S';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[3] = 'M';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[4] = 'A';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[5] = 'N';
    if (MI_OK != PcdAuthState(PICC_AUTHENT1A, 60, &(st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[0]), &g_u8_CardInfo[2])) //偿试密钥
    {
        return (0);
    }
    if (MI_OK != PcdRead(60, &(st_WriteCardBlockData.un_Block0.a_u8_Block0[0])))
    {
        return (0);
    }
    if (M_DESSMANN_CODE != st_WriteCardBlockData.un_Block0.st_Block0.u32_Check_Code )
    {
        return (0);//非本酒店卡
    }
    for (u8_i = 0; u8_i < 15; u8_i++)
    {
        u8_CheckSum += st_WriteCardBlockData.un_Block0.a_u8_Block0[u8_i];//校验和
    }
    if (u8_CheckSum != st_WriteCardBlockData.un_Block0.st_Block0.u8_CheckSum)
    {
        return (0);//校验和错误
    }
    if (0x01 == st_WriteCardBlockData.un_Block0.st_Block0.u8_CardFunction )//是否客人卡
    {
        return (1);
    }

    return (0);
}

/***************************************************************************//**
 * @brief
 *   将注册信息写入卡片中,默认地址为60
 *
 * @param[in]
 *   none
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8U
 *   成功返回1，失败返回0
 ******************************************************************************/
INT8U fn_WriteCardBlock(void)
{
    INT8U u8_i;
    Types_WriteCardBlockType st_WriteCardBlockData;
    INT8U u8_CheckSum = 0;
//	INT32U u32_j;

    if (0x04 != g_u8_CardInfo[0] || 0x00 != g_u8_CardInfo[1])//MF1卡,在此可以增加对其他卡的支持
    {
        return (1);
    }

    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[0] = 'D';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[1] = 'E';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[2] = 'S';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[3] = 'M';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[4] = 'A';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[5] = 'N';

    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[0] = 0x7E;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[1] = 0x17;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[2] = 0x88;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[3] = 0xff;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[0] = 'd';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[1] = 'e';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[2] = 's';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[3] = 'm';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[4] = 'a';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[5] = 'n';
    st_WriteCardBlockData.un_Block0.st_Block0.u32_Check_Code = M_DESSMANN_CODE;
    st_WriteCardBlockData.un_Block0.st_Block0.u32_SerialNumber = 0x00000001;
    st_WriteCardBlockData.un_Block0.st_Block0.u8_CardFunction = 0x01; //初始化为客人卡
    st_WriteCardBlockData.un_Block0.st_Block0.u8_CardType = 0x01; //新卡
    st_WriteCardBlockData.un_Block0.st_Block0.u8_RoomFloor = 0x01;
    st_WriteCardBlockData.un_Block0.st_Block0.u8_RoomLay = 0x01;
    st_WriteCardBlockData.un_Block0.st_Block0.u16_RoomNum = 0x0001;
    st_WriteCardBlockData.un_Block0.st_Block0.u8_CardCounter = 0x01;
    for (u8_i = 0; u8_i < 15; u8_i++)
    {
        u8_CheckSum += st_WriteCardBlockData.un_Block0.a_u8_Block0[u8_i];//校验和
    }
    st_WriteCardBlockData.un_Block0.st_Block0.u8_CheckSum = u8_CheckSum;

    if (MI_OK != PcdSelect(&g_u8_CardInfo[2]))
    {
        return (0);
    }

    if (MI_OK != PcdAuthState(PICC_AUTHENT1A, 60, u8_DefaultKeyA, &g_u8_CardInfo[2]))
    {
//		if (0 == fn_Rc522_Init())//初始化
//        {
//            return (0);//卡模块连接错误,初始化失败
//        }
        fn_ReadAddCardId(4);//重新读一次卡号
        if (MI_OK != PcdSelect(&g_u8_CardInfo[2]))
        {
            return (0);
        }

        if (MI_OK != PcdAuthState(PICC_AUTHENT1B, 60, &(st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[0]), &g_u8_CardInfo[2])) //偿试密钥
        {
            return (0);
        }
    }

//	PcdWrite(60, &(st_WriteCardBlockData.un_Block0.a_u8_Block0[0]));
//	PcdWrite(63, &(st_WriteCardBlockData.un_Block3.a_u8_Block3[0]));

    if (MI_OK != PcdWrite(60, &(st_WriteCardBlockData.un_Block0.a_u8_Block0[0])))
    {
        return (0);
    }
    if (MI_OK != PcdWrite(63, &(st_WriteCardBlockData.un_Block3.a_u8_Block3[0])))//修改密钥和控制位
    {
        return (0);
    }

    return (1);
}

/***************************************************************************//**
 * @brief
 *   删除本公司注册的有效卡
 *
 * @param[in] INT8U *p_CardData
 *   卡片信息
 *
 * @param[out]
 *   none
 *
 * @return[out] INT8U
 *   成功返回1，其他情况返回0
 ******************************************************************************/
INT8U fn_DelCardId(INT8U *p_CardData)
{
    Types_WriteCardBlockType st_WriteCardBlockData;
    INT8U u8_i;

    if (0x04 != p_CardData[0] || 0x00 != p_CardData[1])//在此可以增加对其他卡的支持
    {
        return 0;
    }
    if(0 == fn_ReadCardId(4))//读取卡ID
    {
			fn_Rc522_Poweroff();		
			  return 0;
    }
    for (u8_i=0; u8_i<6; u8_i++)
    {
        if (g_u8_CardInfo[u8_i] != p_CardData[u8_i])
        {
            return 0;
        }
    }
    if (MI_OK != PcdSelect(p_CardData + 2))
    {
        return 0;
    }
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[0] = 'd';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[1] = 'e';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[2] = 's';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[3] = 'm';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[4] = 'a';
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[5] = 'n';
    if (MI_OK != PcdAuthState(PICC_AUTHENT1B, 60, &(st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[0]), p_CardData + 2)) //偿试密钥
    {
        return (0);
    }
    for (u8_i=0; u8_i<16; u8_i++)
    {
        st_WriteCardBlockData.un_Block0.a_u8_Block0[u8_i] = 0xFF;
    }
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[0] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[1] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[2] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[3] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[4] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyA[5] = 0xFF;

    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[0] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[1] = 0x07;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[2] = 0x80;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannAccessBits[3] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[0] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[1] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[2] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[3] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[4] = 0xFF;
    st_WriteCardBlockData.un_Block3.st_Block3.a_u8_DessmannKeyB[5] = 0xFF;
    PcdWrite(60, &(st_WriteCardBlockData.un_Block0.a_u8_Block0[0]));
    PcdWrite(63, &(st_WriteCardBlockData.un_Block3.a_u8_Block3[0]));
    return (1);
}

/***************************************************************************//**
 * @@end of file
 ******************************************************************************/
