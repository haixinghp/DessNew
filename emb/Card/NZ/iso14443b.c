/*
 *****************************************************************************
 * Copyright by ams AG                                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */
/*
 *      PROJECT:   AS3911 firmware
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author Christian Eisendle
 *
 *  \brief Implementation of ISO-14443B
 *
 */
/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "nz_string.h"
#include "nz_vsprintf.h"
#include "utils.h"

#include "errno.h"
#include "nz3801-ab_cfg.h"
#include "nz3801-ab_com.h"
#include "nz3801-ab.h"
#include "iso14443b.h"
#ifdef M_CARD_FUNCTION
#include "h_Mfrc522.h"
#endif
#include "Public.h" 
#if ISO14443B_EN
/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/


/*
******************************************************************************
* LOCAL MACROS
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
extern u8 ApduTransceive(u8 *inf, u16 infLength, u8 *response, u16 *responseLength);

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static u8 iso14443BSendApfAndGetResult(iso14443BCommand_t cmd,
                                    iso14443BProximityCard_t* card,
                                    u8 afi,
                                    iso14443BSlotCount_t slotCount,
                                    u8 *actlength);
static u8 iso14443BDoAntiCollisionLoop(iso14443BCommand_t cmd,
                                    iso14443BProximityCard_t* card,
                                    u8 afi,
                                    iso14443BSlotCount_t slotCount);

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
u8 iso14443BInitialize(void)
{
    nz3801Init(CT_B);
    return ERR_NONE;
}


#if 1
u8 iso14443BSelect(iso14443BCommand_t cmd,
                iso14443BProximityCard_t* card,
                u8 afi)
{
    u8 err;
    u8 buf[3];
    u8 actlength;
    u8 fsci;
    
    if( (cmd!=ISO14443B_CMD_REQB) && (cmd!=ISO14443B_CMD_WUPB) )
        return ERR_PARA;
    
    buf[0] = ISO14443B_PARAM_APF;//"\x05\x00\x08"
    buf[1] = afi;
    buf[2] = cmd;
    err = nz3801Transceive(TB_WUPB, buf, 3, 0, (u8*)card, &actlength, 0);
    if(err==ERR_TIMEOUT)
    {
        if(nzFlagOK())
        {
            return ERR_TIMEOUT;
        }
    }

    if((nzReadReg(REGERROR)&(BFL_JBIT_COLLERR|BFL_JBIT_PROTERR))!=0)// ????/??????
		return ERR_DATA;

    if(!((nzReadReg(REGERROR)&(BFL_JBIT_CRCERR))==0))
		return ERR_DATA;

    if( ERR_NONE==err && actlength!=0xc )
    {
        return ERR_DATA;
    }

    if( ERR_NONE!=err )
        return ERR_DATA;

	if(card->atqb!=0x50)
		return ERR_DATA;

    fsci = (card->protocolInfo[1]>>4) & 0x0f;
    if(fsci>=8)
		FSC = 256;
	else
		FSC = FSCTab[fsci];

    FWI = (card->protocolInfo[2]>>4) & 0x0f;
    if(FWI==0x0f) // 2.5,20160714
       FWI = 4; 
    
    return err;
}
#else
u8 iso14443BSelect(iso14443BCommand_t cmd,
                iso14443BProximityCard_t* card,
                u8 afi,
                iso14443BSlotCount_t slotCount)
{
    u8 err;
    u8 fsci;
    u8 actlength;
    
    if( (cmd!=ISO14443B_CMD_REQB) && (cmd!=ISO14443B_CMD_WUPB) )
        return ERR_PARA;

    err = iso14443BSendApfAndGetResult(cmd, card, afi, ISO14443B_SLOT_COUNT_1,&actlength);
    if (ERR_COLL == err)
    {
       // my_printf("Got ATQB from more PICCs\r\n");
        card->collision = true;
        err = iso14443BDoAntiCollisionLoop(cmd, card, afi, slotCount);
    }
    else if (ERR_NONE == err)
    {
       // my_printf("Got ATQB\r\n");
        /* no crc error - only one PICC replied */
        card->collision = false;
    }

	if(card->atqb!=0x50)
    {
		return ERR_DATA;
	}

    if (ERR_NONE != err)
    {
        return err;
    }
    
    fsci = (card->protocolInfo[1]>>4) & 0x0f;
    if(fsci>=8)
		FSC = 256;
	else
		FSC = FSCTab[fsci];

    FWI = (card->protocolInfo[2]>>4) & 0x0f;
    if(FWI==0x0f) // 2.5,20160714
    {
        FWI = 4; 
    }
    
    return ERR_NONE;
}
#endif


u8 iso14443BSendHltb(iso14443BProximityCard_t* card)
{
    u8 err;
    u8 actlength;
    u8 buf[5];

    buf[0] = ISO14443B_CMD_HLTB;
    mem_copy(&buf[1], card->pupi, 4);

    err = nz3801Transceive(TB_HLTB, buf, 5, 0, buf, &actlength, 0);
	if ((ERR_TIMEOUT == err) || (buf[0] != 0x0))
    {
        err = ERR_DATA;
    }

    return err;
}

u8 iso14443BEnterProtocolMode(iso14443BProximityCard_t* card,
                                iso14443BAttribParameter_t* param,
                                iso14443BAttribAnswer_t* answer)
{
    u8 err;
    u8 actlength;
    u8 buf[9];

    if(!answer)
        return ERR_PARA;

    buf[0] = ISO14443B_CMD_ATTRIB;
    mem_copy(&buf[1], card->pupi, 4);
    mem_copy(&buf[5], (u8*)param, sizeof(iso14443BAttribParameter_t));//"\x00\x08\x01\x00"
    err = nz3801Transceive(TB_ATTRIB, buf, 9, 0, buf, &actlength, 0);
    if(err==ERR_TIMEOUT)
        return ERR_TIMEOUT;
    if(err!=ERR_NONE)
        return ERR_PARA;
    if (actlength!=1)
        err = ERR_DATA;
    if((buf[0]&0x0f)!=0)
		return ERR_DATA;
    
    answer->mbli = (buf[0] >> 4) & 0xf;
    answer->cid = buf[0] & 0xf;
    BlockNum = 0;

    return err;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
static u8 iso14443BDoAntiCollisionLoop(iso14443BCommand_t cmd,
                                    iso14443BProximityCard_t* card,
                                    u8 afi,
                                    iso14443BSlotCount_t slotCount)
{
    u8 err = ERR_COLL;
    u8 apn;
    u8 i;
    u8 j;
    u8 actlength;
   
    //my_printf("Start AC loop\r\n");
    for (i = 0; i < ISO14443B_MAX_AC_LOOP_COUNT; i++)
    {
        //my_printf("loop iteration %d\r\n", i);
        /* first send Apf (wupb or reqb) with given slot count.
           if only one PICC respond return immediately. Otherwise
           send slot marker */
        err = iso14443BSendApfAndGetResult(cmd, card, afi, slotCount,&actlength);
        if(err==ERR_TIMEOUT) goto out;
        if ((err==ERR_NONE) && (actlength == 0xc))
        {
            /* found unique PICC */
            goto out;
        }
        
        for (j = 1; j < (1 << slotCount); j++)
        {
            apn = (j << 4) | 0x5;
            //my_printf("Send slot marker 0x%02x\r\n", apn);
            /* send slot marker 
            err = iso14443TransmitAndReceive(&apn,
                    sizeof(u8),
                    (u8*)card,
                    0xc,
                    &actlength);*/
            err = nz3801Transceive(TB_WUPB, &apn, 1, 0, (u8*)card, &actlength, 0);
            if (actlength != 0xc)
            {
                //my_printf("Received %d bytes only\r\n", actlength);
                /* map this error to not unique error cause this normally
                   happens if more PICCs in field */
                err = ERR_COLL;
            }
            else if (ERR_NONE == err)
            {
                /* found unique PICC */
                goto out;
            }
        }
        
        if (ERR_TIMEOUT == err)
        {
            //my_printf("No card in field\r\n");
            break;
        }
    }
out:
    return err;
}

static u8 iso14443BSendApfAndGetResult(iso14443BCommand_t cmd,
                                    iso14443BProximityCard_t* card,
                                    u8 afi,
                                    iso14443BSlotCount_t slotCount,u8 *actlength)
{
    u8 err = ERR_NONE;
    u8 buf[3];
    u8 errreg;

    buf[0] = ISO14443B_PARAM_APF;
    buf[1] = afi;
    buf[2] = slotCount;
    buf[2] |= cmd;

    err = nz3801Transceive(TB_WUPB, buf, 3, 0, (u8*)card, actlength, 0);
    errreg = nzReadReg(REGERROR);
    if(err==ERR_TIMEOUT)
    {
        return ERR_TIMEOUT;
    }

    if (ERR_NONE == err && *actlength != 0xc)
    {
        //my_printf("Received %d bytes only\r\n", *actlength);
        //PrintInfoDump((u8*)card, *actlength);
        /* map this error to not unique error cause this normally
           happens if more PICCs in field */
        err = ERR_COLL;
    }

    errreg = nzReadReg(REGERROR);
    if(errreg&(BFL_JBIT_COLLERR|BFL_JBIT_CRCERR|BFL_JBIT_PROTERR))
    {
        err = ERR_COLL;
    }
    
    return err;
}

u8 Iso14443BDemonstrate(void)
{
		static iso14443BProximityCard_t gvCardB;
    static iso14443BAttribParameter_t gvParam;
    static iso14443BAttribAnswer_t gvAnswer;
    const u8 iAPDU_GetRND[5] = {0x00,0x84,0x00,0x00,0x08}; //??????????  
    const u8 iAPDU_GetID[5] = {0x00,0x36,0x00,0x00,0x08}; //????2????????ID
    
    u8 err;
    u8 RspLen;
    u8 RspBuf[256];
    u16 ilength;
    
   // my_printf("\r\n\r\n******Type B Card Present***********************\r\n");
    iso14443BInitialize();
    err = nz3801ActivateField(false);
    if(err!=ERR_NONE)
    {
        SuccessRate.numFieldOffFail++;
       // my_printf(">Field OFF\r\n FAIL.\r\n");
        return err;
    }
    NZ_DelayMs(5);  
    err = nz3801ActivateField(true);
    if(err!=ERR_NONE)
    {
        SuccessRate.numFieldOnFail++;
        //my_printf(">Field ON\r\n FAIL.\r\n");
        return err;
    }
    NZ_DelayMs(15); 

    SuccessRate.Totality++;
    // REQBorWUPB
	err = iso14443BSelect(ISO14443B_CMD_WUPB, &gvCardB, 0x00);
	
    if(err==ERR_NONE)
    {
        my_printf(">WUQB\r\n OK.\r\n");
        my_printf(" YES Card.\r\n");
        PUBLIC_PrintHex(" PUPI:", gvCardB.pupi,ISO14443B_PUPI_LENGTH);
        PUBLIC_PrintHex(" APP DATA:", gvCardB.applicationData,ISO14443B_APPDATA_LENGTH);
        my_printf(" FSC=%d\r\n", FSC);
        my_printf(" FWI=%d\r\n", FWI);
    }
    else
    {   
       // my_printf(">WUQB\r\n FAIL.[%d]\r\n",err);
       // my_printf(" NO Card!!!\r\n");
    }

    if(err==ERR_NONE)
    {
        gvParam.param1 = 0x00;
        gvParam.param2 = 0x08;
        gvParam.param3 = 0x01;
        gvParam.param4 = 0x00;
        err = iso14443BEnterProtocolMode(&gvCardB,&gvParam,&gvAnswer);
        if(err==ERR_NONE)
        {
            SuccessRate.numL3OK++;
            my_printf(">ATTRIB\r\n OK.\r\n");
            my_printf(" MBLI=%d\r\n", gvAnswer.mbli);
        }
        else
        {   
            my_printf(">ATTRIB\r\n FAIL.[%d]\r\n",err);
        }
    }

    if(err==ERR_NONE)
    {
		//??????????-????APDU
        err = nz3801Transceive(TB_xBLOCK,(u8*)iAPDU_GetRND,sizeof(iAPDU_GetRND),0,RspBuf,&RspLen,0);
        if(err==ERR_NONE)
        {
            my_printf("\r\n>iAPDU_GetRandom\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:", (u8*)iAPDU_GetRND,sizeof(iAPDU_GetRND));
            PUBLIC_PrintHex(" RX:", RspBuf,RspLen);
        }
        else  
        {
           my_printf("\r\n>iAPDU_GetRandom\r\n FAIL.[%d]\r\n",err);
        }

		//??????????-????APDU
        err = nz3801Transceive(TB_xBLOCK,(u8*)iAPDU_GetID,sizeof(iAPDU_GetID),0,RspBuf,&RspLen,0);
        if(err==ERR_NONE)
        {  
            SuccessRate.numL4OK++;
            my_printf("\r\n>iAPDU_GetID\r\n OK.\r\n");
            PUBLIC_PrintHex(" TX:", (u8*)iAPDU_GetID,sizeof(iAPDU_GetID));
            PUBLIC_PrintHex(" RX:", RspBuf,RspLen);
        } 
        else  
        {
            my_printf("\r\n>iAPDU_GetID\r\n FAIL.[%d]\r\n",err);
        }
    }   

    //my_printf("<<RFOnFail=%d,RFOffFail=%d,L3Pass=%d,L4Pass=%d,Totality=%d\r\n",
      //  SuccessRate.numFieldOnFail,SuccessRate.numFieldOffFail,
      //  SuccessRate.numL3OK,SuccessRate.numL4OK,SuccessRate.Totality);
    
    return err;
}

#endif

