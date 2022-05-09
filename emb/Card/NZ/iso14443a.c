//-----------------------------------------------------------------------------
// nz3801-a.h
//-----------------------------------------------------------------------------
// Copyright 2017 nationz Ltd, Inc.
// http://www.nationz.com
//
// Program Description:
//
// driver definitions for the nfc reader.
//
//
//      PROJECT:   NZ3801-A firmware
//      $Revision: $
//      LANGUAGE:  ANSI C
//
//
// Release 1.0
//    -Initial Revision (NZ)
//    -14 Feb 2017
//    -Latest release before new firmware coding standard
//
/*! \file
 *
 *
 *  \brief Implementation of ISO-14443A
 *
 */
 
 
/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include <stdio.h>

#include "nz_string.h"
#include "nz_vsprintf.h"
#include "utils.h"

#include "errno.h"
#include "nz3801-ab_cfg.h"
#include "nz3801-ab_com.h"
#include "nz3801-ab.h"
#include "iso14443a.h"
#ifdef M_CARD_FUNCTION
#include "h_Mfrc522.h"
#endif
#include "my_des.h"
#include "nfc_pro.h"

#if ISO14443A_EN
/*
******************************************************************************
* LOCAL MACROS
******************************************************************************
*/

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static uint8_t iso14443ADoAntiCollisionLoop(uint8_t UIDLen,iso14443AProximityCard_t *card);

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
uint8_t iso14443AInitialize(void)
{
    nz3801Init(CT_A);
    return ERR_NONE;
}

uint8_t iso14443ASelect(iso14443ACommand_t cmd, iso14443AProximityCard_t* card)
{
    uint8_t err = ERR_NONE;
    uint8_t buf[2];
    uint8_t atqalen;
    uint8_t UIDLen;
    
    if( (cmd!=ISO14443A_CMD_REQA) && (cmd!=ISO14443A_CMD_WUPA) )
        return ERR_PARA;
//    my_printf(">invoke iso14443ASelect\r\n");
    buf[0] = cmd; 
    err = nz3801Transceive(TA_WUPA, buf, 1, 7, card->atqa, &atqalen, 0);
	if(err==ERR_TIMEOUT)
	{
		if(nzFlagOK())
		{
			return ERR_TIMEOUT;
		}
	}
	if(err!=ERR_NONE)
	{
	    return ERR_PARA;
	}
	if(atqalen!=2) // 16 bits ?
	{
	    return ERR_DATA;   
	}

	if(!nzFlagOK()||!nzCrcOK())
	{
	    return ERR_DATA;
	} 

    if(card->atqa[1]==0xf0)// 2.5,20160714
	{
	    return ERR_DATA;
	}
    
  	UIDLen = ((card->atqa[0])>>6)&0x3;
    err = iso14443ADoAntiCollisionLoop(UIDLen,card);
    
	return err;
}


uint8_t iso14443ASendHlta(void)
{
    uint8_t err;
    uint8_t buf[2];
    uint8_t rec[256];
	uint8_t len;
    
    /* send HLTA command */
    buf[0] = ISO14443A_CMD_HLTA;
    buf[1] = 0;
    err = nz3801Transceive(TA_HLTA, buf, 2, 0, rec, &len, 0);
    return err;
}

uint8_t iso14443AEnterProtocolMode(iso14443AProximityCard_t* card, uint8_t* answer, uint8_t* length)
{
    uint8_t err;
    uint8_t buf[2];
		uint8_t TA, TB, TC;
		u32 SFGT = 0;
    
    if(!answer || !length)
        return ERR_PARA;
    
    /* send RATS command */
    buf[0] = ISO14443A_CMD_RATS;
    buf[1] = (card->fsdi<<4) | 0x0; // = 0x80 : FSD=256,cid=0

    BlockNum = 0;
    FWI = 4;
    FSD = 256;
    FSC = 32;
    CID = 0;
    NAD = 0;
    err = nz3801Transceive(TA_RATS, buf, 2, 0, answer, length, 0);
    if(err==ERR_TIMEOUT) // 直接超时
        return ERR_TIMEOUT;
    if(err!=ERR_NONE)
        return ERR_PARA;
    if(!nzFlagOK())
        return ERR_PARA;
    if(!nzCrcOK())
        return ERR_DATA;

    if(answer[0]==0 || answer[0]!=*length)
        return ERR_PROTOCOL;
    if(answer[0]==1)  // 无T0，无后续数据
        return ERR_NONE;

    TA = TB = TC = 0;
    if((answer[1]&BIT4)!=0) TA = 1;
    if((answer[1]&BIT5)!=0) TB = 1;
    if((answer[1]&BIT6)!=0) TC = 1;
    if( answer[0]<(TA+TB+TC+2))
        return ERR_PROTOCOL;

    // T0
    card->fsci = answer[1]&0x0f;
    if(card->fsci>=8)
        FSC = 256;
    else
        FSC = FSCTab[card->fsci];

    // TA中DR/DS可同时为0，所以该字节任何数据都是正确的
    if(TA)
    {
        card->TA = answer[2];//详细见-4协议
    }
    // TB值
    if(TB)
    {
        card->fwi = (answer[2+TA]>>4)&0xf;
        card->sfgi = answer[2+TA]&0xf;
        SFGT = card->sfgi;
        if(SFGT==15) 
        {
            card->sfgi = 0;
            SFGT = 0;
        }
        if(card->fwi==15) 
        {
            card->fwi = 4;
        }
        FWI = card->fwi;  
        if(card->fwi>15 || SFGT>15)
        {
            return ERR_PROTOCOL;
        }
    }
    // TC值没有限制，所有值合法
    if(TC)
    {
        NAD = answer[2+TA+TB]&0x01;
        CID = (answer[2+TA+TB]>>1)&0x01;
    }
  
    if(SFGT)// 延时SFGT
    {
        nzClearFlag();
        nz3801SetTimer((256*16+384)*power(SFGT));
        nzSetBitMask(CONTROL, BFL_JBIT_TSTARTNOW);  // 手动启动
        while((nzReadReg(COMMIRQ)&BFL_JBIT_TIMERI) == 0);
    }

    return ERR_NONE;
}

uint8_t iso14443ASendProtocolAndParameterSelection(uint8_t pps1)
{
    uint8_t  err;
    uint8_t  buf[3],rec[256];
    uint8_t  len;

    /* send PPS command */
    buf[0] = ISO14443A_CMD_PPSS | 0x0;
    buf[1] = 0x11;
    buf[2] = pps1;
	err = nz3801Transceive(TA_PPS, buf, 3, 0, rec, &len, 0);
	if(err==ERR_TIMEOUT) // 直接超时
		return ERR_TIMEOUT;
	if(err!=ERR_NONE)
		return ERR_PARA;
	if(!nzFlagOK())
		return ERR_PARA;
	if(!nzCrcOK())
		return ERR_DATA;

	if(rec[0]!=buf[0] || len!=1)
        return ERR_PROTOCOL;

	return ERR_NONE;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
#if 0
static uint8_t iso14443ADoAntiCollisionLoop(uint8_t UIDLen,iso14443AProximityCard_t *card)
{
    uint8_t cmd[20], rec[256];
    uint8_t i, len;
    uint8_t err;
    eCmd cmdtype;
    uint8_t cl;
    uint8_t timer = 0;

	card->actlength = 0;
	mem_copy(cmd, (uint8_t*)"\x93\x20", 2);
	// 0 - 5:  0,2,4发送ant;  1,3,5发送select
	for(i=0; i<(UIDLen+1)*2; i++)
	{
		cl = i/2;
		cmd[0] = 0x93+(i/2)*2;	// 0x93, 0x95, 0x97
		if((i&1)==0)	// ant
		{
			len = 2;
			cmd[1] = 0x20;
			cmdtype = TA_ANT;
		}
		else 			// select
		{
			len = 2+4+1;
			cmd[1] = 0x70;
			cmdtype = TA_SELECT;
		}
        
        //PUBLIC_PrintHex("TX:",cmd,len);
		err = nz3801Transceive(cmdtype, cmd, len, 0, rec, &len, 0);
        
        //PUBLIC_PrintHex("RX:",rec,len);  
        
		if(err==ERR_TIMEOUT)
		{
			if(nzFlagOK())
			{
				timer++;
				i--;		// 可能是0xff
				if(timer >= 3)
					return ERR_TIMEOUT;
				else
					continue;
			}
		}
		timer = 0;
		if(err!=ERR_NONE)
        {
			return ERR_PARA;
		}

		if(!nzFlagOK())
		{
		    return ERR_DATA;
		}
		if((i&1)==0)	// ant，对数据内容没有限制
		{
			uint8_t j, bcc = 0;

			mem_copy(card->uid+card->actlength, rec, 4);
			card->actlength += 4;
			if(len!=5) 
            {
                return ERR_DATA;
			}
            for(j=0; j<5; j++)	
            {
                bcc ^= rec[j];
			}
            if(bcc!=0)	
            {
                return ERR_DATA;
			}
            cl = i/2;
			if(rec[0]==0x88)// 已经是最后一级时报错
			{
				if(cl==UIDLen)
                {
					return ERR_DATA;
				}
			}
			else// 不是最后一级报错
			{
				if(cl!=UIDLen)
                {
					//return ERR_DATA;
				    UIDLen = cl;// 2.5,20160714,TA102中存在多重UID没发的情况
				}
			}
			mem_copy(cmd+2, rec, 5);
		}
		else// select
		{
			card->sak[0] = rec[0];
			if(!nzCrcOK())
            {
				return ERR_DATA;
			}
            if(len!=1)
			{
			    return ERR_DATA;
			}
            if(cl==UIDLen && (rec[0]&BIT2)!=0)// 最后一级仍然显示UID不完整
			{
			    return ERR_DATA;
			}
            if((rec[0]&BIT2)==0)
			{
			    return ERR_NONE;
            }
		}

	}

	return ERR_DATA;// UID指示的级数与实际SAK不一致
}
#endif

static uint8_t iso14443AVerifyBcc(const uint8_t* uid, uint8_t length, uint8_t bcc)
{
    uint8_t actbcc = 0;

    do {
        length--;
        actbcc ^= uid[length];
    } while (length);

    if (actbcc != bcc)
    {
        return ERR_CRC;
    }
    
    return ERR_NONE;
}

static uint8_t iso14443ADoAntiCollisionLoop(uint8_t UIDLen,iso14443AProximityCard_t *card)
{
    uint8_t cscs[ISO14443A_MAX_CASCADE_LEVELS][ISO14443A_CASCADE_LENGTH];
    uint8_t cl = ISO14443A_CMD_SELECT_CL1;
    uint8_t regcoll;
    uint8_t regerr;
    uint8_t bytes = 0;
    uint8_t bits = 0;
    uint8_t txlength = 2;
    uint8_t rxlength;
    uint8_t saklength;
    uint8_t savecoll = 0;
    uint8_t err;
    uint8_t i;
    uint8_t* buf;

    nzClearBitMask(STATUS2, 0x08);
    nzWriteReg(BITFRAMING, 0x00);
    nzClearBitMask(COLL, 0x80);
    
    card->cascadeLevels = 0;
    card->collision = false;
    card->actlength = 0;
    buf = cscs[card->cascadeLevels];
    /* start anticollosion loop by sending SELECT command and NVB 0x20 */
    buf[1] = 0x20;

    do 
    {
        buf[0] = cl;
				#ifdef DEBUG_LOG_ENABLE							
						PUBLIC_PrintHex("T", buf, txlength);							
				#endif
        
        err = nz3801Transceive(TA_ANT,buf,txlength,bits,buf+2,&rxlength,bits);
        if(err!=ERR_NONE) goto out;
				#ifdef DEBUG_LOG_ENABLE							
						my_printf("Received %d bytes\r\n", rxlength);
        PUBLIC_PrintHex("R", buf+2, rxlength);						
			  #endif
        
        
        /* now check for collision */
        regcoll = nzReadReg(COLL);/* read out collision register */
        regerr = nzReadReg(REGERROR);
        if( (!(regcoll&0x20)) || (regerr&BFL_JBIT_COLLERR) )
        {
            //my_printf("Has Collision!\r\n");
            //my_printf("regcoll=%d\r\n", (regcoll&0x1F));
            card->collision = true;
            /* save the colision byte */
            savecoll = buf[2+(regcoll&0x1F)/8];
            
            bits = (regcoll&0x1F) % 8;
            bytes = (regcoll&0x1F) / 8;
            //bits++;
            //if (bits==8)
            //{
            //    bits = 0;
            //    bytes++;
            //}
            
            /* FIXME handle c_pb collision in parity bit */
            /* update NVB. Add 2 bytes for SELECT and NVB itself */
            buf[1] = 0x20 + (bytes<<4) + (bits&0xF);
            if(bits!=0)
            {
                txlength = bytes + 2 + 1;
            }
            else
            {
                txlength = bytes + 2;
            }
            //my_printf("Bytes=%d,Bits=%d,txlength=%d,savecoll=0x%02x\r\n",bytes,bits,txlength,savecoll);
        }
        else
        {
            //my_printf("Got a frame\r\n");
            /* got a frame w/o collision - store the uid and check for CT */

            /* answer with complete uid and check for SAK. */
            buf[1] = 0x70;
            buf[0] = cl;
            buf[2+bytes] |= savecoll;
            txlength = rxlength + 2;
            
            //my_printf("Request SAK\r\n");
            //PUBLIC_PrintHex( "T",buf, txlength);
            err = nz3801Transceive(TA_SELECT,
                  buf,txlength,0,&card->sak[card->cascadeLevels],&saklength,0);
            if(err!=ERR_NONE) goto out;
            //my_printf("Got SAK = 0x%02x\r\n",card->sak[card->cascadeLevels]);
            if(!nzCrcOK())
            {
                err = ERR_DATA;
                goto out;
            }
            if(saklength!=1)
            {
                err = ERR_DATA;
                goto out;
            }
            
            if (card->sak[card->cascadeLevels] & 0x4)
            {
                //my_printf("Next cascading level\r\n");
                /* reset variables for next cascading level */
                txlength = 2;
                bytes = 0;
                bits = 0;
                
                if (ISO14443A_CMD_SELECT_CL1 == cl)
                {
                    cl = ISO14443A_CMD_SELECT_CL2;
                }
                else if (ISO14443A_CMD_SELECT_CL2 == cl)
                {
                    cl = ISO14443A_CMD_SELECT_CL3;
                }
                else
                {
                    /* more than 3 cascading levels are not possible ! */
                    err = ERR_COLL;
                    goto out;
                }
            }
            else
            {
                //my_printf("UID done\r\n");
                card->cascadeLevels++;
                break;
            }
            card->cascadeLevels++;
            buf = cscs[card->cascadeLevels];
            buf[0] = cl;
            buf[1] = 0x20;
        } /* no collision detected */

    } while (card->cascadeLevels <= ISO14443A_MAX_CASCADE_LEVELS);

    /* do final checks... */
    for (i = 0; i< card->cascadeLevels; i++)
    {
        err = iso14443AVerifyBcc(&cscs[i][2], 4, cscs[i][6]);
        if(err!=ERR_NONE) goto out;
    }

    //my_printf("cascadeLevels=0x%x\r\n", card->cascadeLevels);
            
    /* extract pure uid */
    switch (card->cascadeLevels)
    {
        case 3:
            memmove(card->uid+6, cscs[2]+2, 4);
            memmove(card->uid+3, cscs[1]+3, 3);
            memmove(card->uid+0, cscs[0]+3, 3);
            card->actlength = 10;
            break;
        case 2:
            memmove(card->uid+3, cscs[1]+2, 4);
            memmove(card->uid+0, cscs[0]+3, 3);
            card->actlength = 7;
            break;
        case 1:
            memmove(card->uid+0, cscs[0]+2, 4);
            card->actlength = 4;
            break;
        default:
            err = ERR_COLL;
            goto out;
    }
                  
out:
    nzSetBitMask(COLL, 0x80);
    return err;
}


uint8_t ApduTransceive(uint8_t *inf, u16 infLength, uint8_t *response, u16 *responseLength)
{
    if(!inf || !response || !responseLength)
        return ERR_DATA;
    return nz3801IBLOCK(inf,infLength,response,responseLength);    
		
}

extern int exampleMain(void);
//-----------------------------------------------------------------------------
uint8_t Iso14443ADemonstrate(void)
{
//																			CLA   INS  P1   P2   Le   	
    const uint8_t iAPDU_GetRandom[5] 	 	 = {0x00,0x84,0x00,0x00,0x08};
//																			CLA   INS  P1   P2   Lc command data
//    const uint8_t iAPDU_SelectMainFile[7] = {0x00,0xA4,0x00,0x00,0x02,0x3f,0x00};
////																			CLA   INS  P1   P2   Le 
//    const uint8_t iAPDU_Read_records[5] =   {0x00,0xB2,0x01,0x0C,0x00};		
////																			CLA   INS  P1   P2   Le 
//    const uint8_t iAPDU_Read_records1[5] =   {0x00,0xB2,0x03,0x14,0x00};		
//																						 CLA   INS  P1   P2   Lc command data
    uint8_t iAPDU_Internal_authenticate[31] = {0x00,0x88,0x01,0x02,0x1A,0x01,0x0E,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x02,0x08,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88};//008801021A010E111111111111111111111111111102088888888888888888
//																				 CLA   INS  P1   P2    Lc      command data
    uint8_t iAPDU_External_authenticate[13] = {0x00,0x82,0x00,0x02,0x08,0x33,0x05,0x82,0x86,0x4e,0x66,0xd6,0x21};
//									 								 CLA   INS  P1   P2   Lc command data
    const uint8_t iAPDU_SelectAID[21] = {0x00,0xA4,0x04,0x00,0x10,0xA0,0x00,0x00,0x03,0x33,0x01,0x01,0x02,0x00,0x63,0x48,0x57,0x50,0x41,0x50,0x21};		//00A4040010A0000003330101020063485750415021
//																			 CLA   INS  P1   P2   Lc   	
     uint8_t iAPDU_Write_CardID[] 	 	 = {0x04,0xD6,0x82,0x00,0x1C,0x6b,0x14,0xb9,0x69,0x83,0x42,0x2e,0x01,0xde,0x8f,0x33,0x15,0x23,0x06,0xf3,0xb8,0xff,0xac,0x1e,0x50,0x86,0xd8,0x7c,0x33,0x41,0x76,0x1f,0xa0};//00B0820000
			
//																			 CLA   INS  P1   P2   Le   	
    const uint8_t iAPDU_Get_CardID[5] 	 	 = {0x00,0xB0,0x82,0x00,0x00};//00B0820000
//																									CLA   INS  P1   P2   Le   	
    const uint8_t iAPDU_Get_RSA_puk_key_step1[5] 	 	 = {0x00,0xB0,0x93,0x00,0xFF};//00B09300FF
//										        												CLA   INS  P1   P2   Le   	
    const uint8_t iAPDU_Get_RSA_puk_key_step2[5] 	 	 = {0x00,0xB0,0x93,0xFF,0x0B};//00B093FF0B
//								 								  	CLA   INS  P1   P2   Lc  command data   	
//    const uint8_t iAPDU_Write_LockID[25]= {0x04,0xD6,0x81,0x00,0x14,0x8a,0xd5,0x89,0x39,0x65,0xa5,0x33,0xae,0x23,0x53,0x3d,0xd7,0x2b,0x16,0x6f,0xac,0xa1,0x59,0x93,0x4f};//00B0820000
//		const uint8_t iAPDU_Write_LockID[19]= {0x04,0xD6,0x81,0x00,0x0e,0x8a,0xd5,0x89,0x39,0x65,0xa5,0x33,0xae,0x23,0x53,0x3d,0xd7,0x2b,0x16};//00B0820000
	 uint8_t iAPDU_Write_LockID[]= {0x04,0xD6,0x81,0x00,0x14,0xc2,0xe3,0x68,0xb8,0xaf,0xbc,0x9f,0x36,0xe6,0x76,0x6d,0x71,0x2f,0x15,0x34,0xe8,0x13,0x66,0x74,0x50};//00B0820000
	const uint8_t iAPDU_Get_LockID[]  = {0x00,0xB0,0x81,0x00,0x00};//00B0820000	

//																					CLA   INS  P1   P2   Lc command data
    const uint8_t iAPDU_Select_ReportFile[7] = {0x00,0xA4,0x00,0x00,0x02,0xA0,0xF1};	//00A4000002A0F1	
//																	CLA   INS  P1   P2   Lc command data
     uint8_t iAPDU_Report_OK[11] = {0x04,0xD6,0x00,0x00,0x06,0x00,0x10,0x71,0xd9,0x7e,0x99};	//	04D6000006001071d97e99 上报配对成功
			
			
  	uint8_t  err;
    uint8_t  buf[260];
    uint8_t  clength;
    u16 ilength;
    iso14443AProximityCard_t gvCardA;

    iso14443AInitialize();
    err = nz3801ActivateField(false);
    if(err!=ERR_NONE)
    {
			SuccessRate.numFieldOffFail++;
			my_printf(">Field OFF\r\n FAIL.\r\n");
			return err;
    }
    NZ_DelayMs(5); 
//    nrf_delay_ms(20);		
    err = nz3801ActivateField(true);
    if(err!=ERR_NONE)
    {
        SuccessRate.numFieldOnFail++;
        my_printf(">Field ON\r\n FAIL.\r\n");
        return err;
    }
    NZ_DelayMs(15); 
    
    SuccessRate.Totality++;
    // REQAorWUPA & ANTICOLLION
    err = iso14443ASelect(ISO14443A_CMD_WUPA, &gvCardA);
    if(err==ERR_NONE)
    {
        my_printf(">REQA\r\n OK.\r\n");
        my_printf(" YES Card.\r\n");
		PUBLIC_PrintHex(" ATQA:",gvCardA.atqa,sizeof(gvCardA.atqa));				
        PUBLIC_PrintHex(" UID:",gvCardA.uid,gvCardA.actlength);
        SuccessRate.numL3OK++;
    }
    else
    {   
//       my_printf(">REQA\r\n FAIL.[%d]\r\n",err);
//       my_printf(" NO Card!!!\r\n");
    }

    // RATS
    if(err==ERR_NONE)
    {
        gvCardA.fsdi = 8; // fsdi=8 -> FSD=256
        err = iso14443AEnterProtocolMode(&gvCardA,buf,&clength);
        if(err==ERR_NONE)
        {
            my_printf(">RATS\r\n OK.\r\n");
            PUBLIC_PrintHex(" ATS:",buf,clength);
            my_printf(" FSCI=%d\r\n FWI=%d\r\n TA=%02X\r\n",gvCardA.fsci,gvCardA.fwi,gvCardA.TA);
        }
        else  
        {   
           my_printf(">RATS\r\n FAIL.[%d]\r\n",err);
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
        
        err = iso14443ASendProtocolAndParameterSelection(pps1);
        if(err==ERR_NONE)
        {
            my_printf(">PPS\r\n OK.\r\n");
            // here set tx&rxbit speed
            if(pps1==0x5)
            {
                nzWriteReg(MODWIDTH, reg24h_MODWIDTH);
                nzSetBitMask(TXMODE, BFL_JBIT_212KBPS);
                nzSetBitMask(RXMODE, BFL_JBIT_212KBPS);
                my_printf(" SPEED=212kBd.\r\n");
            }
            else if(pps1==0xA)
            {
                nzWriteReg(MODWIDTH, reg24h_MODWIDTH);
                nzSetBitMask(TXMODE, BFL_JBIT_424KBPS);
                nzSetBitMask(RXMODE, BFL_JBIT_424KBPS);
                my_printf(" SPEED=424kBd.\r\n");
            }
            else if(pps1==0xF)
            {
                nzWriteReg(MODWIDTH, reg24h_MODWIDTH);
                nzSetBitMask(TXMODE, BFL_JBIT_848KBPS);
                nzSetBitMask(RXMODE, BFL_JBIT_848KBPS);
                my_printf(" SPEED=848kBd.\r\n");
            }  
            else
            {
                my_printf(" SPEED=106kBd.\r\n");
            }
        }
        else
        {               
            my_printf(">PPS\r\n FAIL.[%d]\r\n",err);
            my_printf(" SPEED=106kBd.\r\n");
        }
        err = ERR_NONE; 
    }
#endif

    
		static uint8 is_register=0;
		
#if 1
		is_register=1;
		// APDU iAPDU_SelectAID
    if(err==ERR_NONE)
    {
        err = ApduTransceive((uint8_t*)iAPDU_SelectAID,sizeof(iAPDU_SelectAID),buf,&ilength);
        if(err==ERR_NONE)
        {
            SuccessRate.numL4OK++;
            my_printf(">APDU-iAPDU_SelectAID\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_SelectAID,sizeof(iAPDU_SelectAID));
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
            my_printf(">APDU-iAPDU_SelectAID\r\n FAIL.[%d]\r\n",err);
        }
    }  

	// APDU iAPDU_GetRandom
	unsigned char Random_buf[8]={0};
    if(err==ERR_NONE)
    {
        err = ApduTransceive((uint8_t*)iAPDU_GetRandom,sizeof(iAPDU_GetRandom),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-GetRandom\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_GetRandom,sizeof(iAPDU_GetRandom));
            PUBLIC_PrintHex(" RX:",buf,ilength);
						memcpy(Random_buf,buf,8);
        }
        else
        {
           my_printf(">APDU-GetRandom\r\n FAIL.[%d]\r\n",err);
        }
    }
	
	
	// APDU iAPDU_External_authenticate
    if(err==ERR_NONE)
    {
			unsigned char cipher_buf[24]={0};	
			unsigned char key[16] = {0xba, 0x6b, 0x11, 0x74, 0xe6, 0x57, 0x4c, 0x91,
				0xeb, 0xbe, 0x71, 0x10, 0x0e, 0xa8, 0x05, 0x50};
			//3DES ECB 加密
			my_printf("3DES ECB ENC:\r\n");
			uint8 ret = des3_ecb_encrypt(cipher_buf,Random_buf,8,key,16);
			for(int i = 0;i < ret;i++)
			{
				my_printf("%02X",cipher_buf[i]);
			}
			my_printf("\r\n");
/*			
//				uint8_t p_buff[40]  = {0x04,0xD6,0x82,0x00,0x1C,0x11,0x10,0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0x80,0x00,0x00,0x00,0x00,0x00};
//				uint8_t p_buff_size=29;
//				uint8_t p_out[16]  = {0x02,0x4E,0x14,0x25,0x00};
//				memset(p_out,0,16);
//				key[15]=0x52;
//				//3DES ECB 加密
//				my_printf("3DES ECB ENC: \r\n");
//				 ret = des3_ecb_encrypt(cipher_buf,&p_buff[5],24,key,16);
//				for(int i = 0;i < ret;i++)
//				{
//					my_printf("%02X",cipher_buf[i]);
//				}
//				my_printf("\r\n");
//				
////				memcpy(&p_buff[5],cipher_buf,24);
//				
//				caculate_MAC_use_3des(key, p_buff,  p_buff_size, p_out,  8);
//				uint8_t p_buff1[40]  = {0x04,0xD6,0x00,0x00,0x06,0x00,0x10};
//				uint8_t p_out1[16]  = {0x02,0x4E,0x14,0x25,0x00};
//				p_buff_size=7;
//				memset(key,0,16);
//				caculate_MAC_use_3des(key, p_buff1,  p_buff_size, p_out1,  8);
*/		
			
			
			memcpy(&iAPDU_External_authenticate[5],cipher_buf,8);
			err = ApduTransceive((uint8_t*)iAPDU_External_authenticate,sizeof(iAPDU_External_authenticate),buf,&ilength);
			if(err==ERR_NONE)
			{			
					my_printf(">APDU-iAPDU_External_authenticate\r\n OK.\r\n");
					PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_External_authenticate,sizeof(iAPDU_External_authenticate));
					my_printf("RXlen=%d ",ilength);
					PUBLIC_PrintHex(" RX:",buf,ilength);
			}
			else
			{
				 my_printf(">APDU-iAPDU_External_authenticate\r\n FAIL.[%d]\r\n",err);
			}
    }
		
		// APDU iAPDU_Write_CardID
    if(err==ERR_NONE&&is_register==0)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Write_CardID,sizeof(iAPDU_Write_CardID),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Write_CardID\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Write_CardID,sizeof(iAPDU_Write_CardID));
            my_printf("RXlen=%d ",ilength);
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Write_CardID\r\n FAIL.[%d]\r\n",err);
        }
    }
		// APDU iAPDU_Get_CardID
    if(err==ERR_NONE)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Get_CardID,sizeof(iAPDU_Get_CardID),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Get_CardID\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Get_CardID,sizeof(iAPDU_Get_CardID));
            my_printf("RXlen=%d ",ilength);
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Get_CardID\r\n FAIL.[%d]\r\n",err);
        }
    }
		
	// APDU iAPDU_Get_RSA_puk_key_step1
    if(err==ERR_NONE&&is_register==0)
    {
				ilength=0;
        err = ApduTransceive((uint8_t*)iAPDU_Get_RSA_puk_key_step1,sizeof(iAPDU_Get_RSA_puk_key_step1),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Get_RSA_puk_key_step1\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Get_RSA_puk_key_step1,sizeof(iAPDU_Get_RSA_puk_key_step1));
            my_printf("RXlen=%d ",ilength);
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Get_RSA_puk_key_step1\r\n FAIL.[%d]\r\n",err);
        }
    }
		
		// APDU iAPDU_Get_RSA_puk_key_step2
    if(err==ERR_NONE&&is_register==0)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Get_RSA_puk_key_step2,sizeof(iAPDU_Get_RSA_puk_key_step2),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Get_RSA_puk_key_step2\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Get_RSA_puk_key_step2,sizeof(iAPDU_Get_RSA_puk_key_step2));
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Get_RSA_puk_key_step2\r\n FAIL.[%d]\r\n",err);
        }
    }
		
		// APDU iAPDU_Write_LockID
    if(err==ERR_NONE&&is_register==0)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Write_LockID,sizeof(iAPDU_Write_LockID),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Write_LockID\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:"，(uint8_t*)iAPDU_Write_LockID,sizeof(iAPDU_Write_LockID));
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Write_LockID\r\n FAIL.[%d]\r\n",err);
        }
    }
		// APDU iAPDU_Get_LockID
    if(err==ERR_NONE&&is_register==0)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Get_LockID,sizeof(iAPDU_Get_LockID),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Get_LockID\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:", (uint8_t*)iAPDU_Get_LockID,sizeof(iAPDU_Get_LockID));
            PUBLIC_PrintHex(" RX:", buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Get_LockID\r\n FAIL.[%d]\r\n",err);
        }
    }
		// APDU iAPDU_Internal_authenticate
    if(err==ERR_NONE&&is_register==1)
    {
				ilength=0;
        err = ApduTransceive((uint8_t*)iAPDU_Internal_authenticate,sizeof(iAPDU_Internal_authenticate),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Internal_authenticate\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Internal_authenticate,sizeof(iAPDU_Internal_authenticate));
            my_printf("RXlen=%d \r\n",ilength);
            PUBLIC_PrintHex(" RX:",buf,ilength);
            is_register=2;
            //exampleMain();
        }
        else
        {
           my_printf(">APDU-iAPDU_Get_RSA_puk_key_step1\r\n FAIL.[%d]\r\n",err);
        }
    }
		
		
		
		// APDU iAPDU_Select_ReportFile
    if(err==ERR_NONE)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Select_ReportFile,sizeof(iAPDU_Select_ReportFile),buf,&ilength);
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Select_ReportFile\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Select_ReportFile,sizeof(iAPDU_Select_ReportFile));
            PUBLIC_PrintHex(" RX:",buf,ilength);
        }
        else
        {
           my_printf(">APDU-iAPDU_Select_ReportFile\r\n FAIL.[%d]\r\n",err);
        }
    }
		// APDU iAPDU_Report_OK
    if(err==ERR_NONE&&is_register==0)
    {
				if(is_register)
				{
					uint8_t iAPDU_Report_OK[11] = {0x04,0xD6,0x00,0x00,0x06,0x00,0x20,0x7b,0x42,0xdd,0x6e};	//	04D600000600207b42dd6e 上报配对成功
					err = ApduTransceive((uint8_t*)iAPDU_Report_OK,sizeof(iAPDU_Report_OK),buf,&ilength);
				}
				else{
					err = ApduTransceive((uint8_t*)iAPDU_Report_OK,sizeof(iAPDU_Report_OK),buf,&ilength);
				}
        
        if(err==ERR_NONE)
        {
            my_printf(">APDU-iAPDU_Report_OK\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:",(uint8_t*)iAPDU_Report_OK,sizeof(iAPDU_Report_OK));
            PUBLIC_PrintHex(" RX:",buf,ilength);
						is_register=1;
						
        }
        else
        {
           my_printf(">APDU-iAPDU_Report_OK\r\n FAIL.[%d]\r\n",err);
        }
    }
		
		
#endif
		
#if 0
    if(err==ERR_NONE)
    {
        err = ApduTransceive((uint8_t*)iAPDU_SelectMainFile,sizeof(iAPDU_SelectMainFile),buf,&ilength);
        if(err==ERR_NONE)
        {
            SuccessRate.numL4OK++;
            my_printf(">APDU-SelectMainFile\r\n OK.\r\n");
            PUBLIC_PrintHex((uint8_t*)iAPDU_SelectMainFile,sizeof(iAPDU_SelectMainFile)," TX:");
            PUBLIC_PrintHex(buf,ilength," RX:");
        }
        else
        {
            my_printf(">APDU-SelectMainFile\r\n FAIL.[%d]\r\n",err);
        }
    }  

    if(err==ERR_NONE)
    {
        err = ApduTransceive((uint8_t*)iAPDU_Read_records,sizeof(iAPDU_Read_records),buf,&ilength);
        if(err==ERR_NONE)
        {
            SuccessRate.numL4OK++;
            my_printf(">APDU-iAPDU_Read_records\r\n OK.\r\n");
            PUBLIC_PrintHex((uint8_t*)iAPDU_Read_records,sizeof(iAPDU_Read_records)," TX:");
            PUBLIC_PrintHex(buf,ilength," RX:");
        }
        else
        {
            my_printf(">APDU-SelectMainFile\r\n FAIL.[%d]\r\n",err);
        }
    }  
//		if(err==ERR_NONE)
//    {
//        err = ApduTransceive((uint8_t*)iAPDU_Read_records1,sizeof(iAPDU_Read_records1),buf,&ilength);
//        if(err==ERR_NONE)
//        {
//            SuccessRate.numL4OK++;
//            my_printf(">APDU-iAPDU_Read_records1\r\n OK.\r\n");
//            PUBLIC_PrintHex((uint8_t*)iAPDU_Read_records,sizeof(iAPDU_Read_records1)," TX:");
//						my_printf("RXlen=%d ",ilength);
//            PUBLIC_PrintHex(buf,ilength," RX:");
//        }
//        else
//        {
//            my_printf(">APDU-SelectMainFile\r\n FAIL.[%d]\r\n",err);
//        }
//    }  
#endif
  //  my_printf("<<RFOnFail=%d,RFOffFail=%d,L3Pass=%d,L4Pass=%d,Totality=%d\r\n",
      //  SuccessRate.numFieldOnFail,SuccessRate.numFieldOffFail,
   //     SuccessRate.numL3OK,SuccessRate.numL4OK,SuccessRate.Totality);

    return is_register;
}
#endif

