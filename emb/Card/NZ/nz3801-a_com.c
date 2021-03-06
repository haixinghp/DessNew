//-----------------------------------------------------------------------------
// nz3801-a_com.c
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

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "errno.h"
#include "Drive\Card\NZ\nz3801-a_com.h"
#include "main.h"
#include "Drive\Card\RC522\h_Mfrc522.h"

/**
 * @brief G E N E R I C    W R I T E
 *        write value at the specified address
 * @param address
 * @param value
 */
void nzWriteReg(u8 reg, u8 value)
{
    WriteRawRC(reg,value);
}

/**
 * @brief G E N E R I C    R E A D
 *        read value from the specified address
 * @param address
 * @param
 */
u8 nzReadReg(u8 reg)
{
    return ReadRawRC(reg);
}

/**
 * @brief S E T   A   B I T   M A S K
 *
 * @param address
 * @param mask
 */
void nzSetBitMask(u8 reg,u8 mask)
{
	u8 tmp;
	tmp = nzReadReg(reg);
	nzWriteReg(reg,(tmp|mask));
}

/**
 * @brief C L E A R   A   B I T   M A S K
 *
 * @param address
 * @param mask
 */
void nzClearBitMask(u8 reg,u8 mask)
{
	u8 tmp;
	tmp = nzReadReg(reg);
	nzWriteReg(reg,(tmp&(~mask)));
}

/**
 * @brief Clear Fifo 
 *
 * @param
 * @param
 */
void nzClearFifo(void)
{
    //while(nzReadReg(FIFOLEVEL&0x7f)!=0)
    {
        nzSetBitMask(FIFOLEVEL,0x00|BFL_JBIT_FLUSHBUFFER);
    }
}
/**
 * @brief Clear Flag
 *
 * @param
 * @param
 */
void nzClearFlag(void)
{
    nzWriteReg(COMMIRQ, 0x7f);
    nzWriteReg(DIVIRQ, 0x7f);
}

/**
 * @brief Start Cmd
 *
 * @param cmd
 * @param 
 */
void nzStartCmd(u8 cmd)
{
    nzWriteReg(COMMAND, cmd);
}

/**
* @brief Stop Cmd
*
* @param 
* @param 
*/
void nzStopCmd(void)
{
    nzWriteReg(COMMAND, 0x00);
}

/**
* @brief Set CRC enable or disable
*
* @param bEN
* @param 
*/
void nzSetCRC(bool bEN)
{
    if(bEN) 
    {
        nzSetBitMask(TXMODE, BFL_JBIT_CRCEN);
        nzSetBitMask(RXMODE, BFL_JBIT_CRCEN);
    }
    else  
    {
        nzClearBitMask(TXMODE, BFL_JBIT_CRCEN);
        nzClearBitMask(RXMODE, BFL_JBIT_CRCEN);
    }
}
/**
* @brief Set PARITY enable or disable
*
* @param bEN
* @param 
*/
void nzSetPARITY(bool bEN)
{
    ;
}

/**
* @brief check errors flag if Set or Not
*
* @param 
* @param 
*/
bool nzFlagOK(void) 
{
    if((nzReadReg(REGERROR)&(BFL_JBIT_CRCERR|BFL_JBIT_PROTERR|/*BFL_JBIT_COLLERR|*/BFL_JBIT_PARITYERR))==0)
    {
        return TRUE;
    }
    return FALSE;
}

/**
* @brief check crc flag if Set or Not
*
* @param 
* @param 
*/
bool nzCrcOK(void)
{
    if((nzReadReg(REGERROR)&(BFL_JBIT_CRCERR))==0)
    {
        return TRUE;
    }
    return FALSE;
}

