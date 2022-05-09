//-----------------------------------------------------------------------------
// nz3801-ab_com.c
//-----------------------------------------------------------------------------
// Copyright 2017 nationz Ltd, Inc.
// http://www.nationz.com
//
// Program Description:
//
// driver definitions for the nfc reader.
//
//
//      PROJECT:   NZ3801-AB firmware
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
#include "nz3801-ab_com.h"
#include "main.h"
#include "all_header.h"
#include "h_Mfrc522.h"
eComMode ComMode = SPI;
//eComMode ComMode = IIC;
//eComMode ComMode = UART;


/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/**
 * @brief G E N E R I C    W R I T E
 *        write value at the specified address
 * @param address
 * @param value
 */
void nzWriteReg(INT8U reg, INT8U value)
{
    WriteRawRC(reg,value);
}

/**
 * @brief GENERIC    READ
 *        read value from the specified address
 * @param address
 * @param
 */
INT8U nzReadReg(INT8U reg)
{
    return ReadRawRC(reg);
}

/**
 * @brief SET  A  BIT   MASK
 *
 * @param address
 * @param mask
 */
void nzSetBitMask(INT8U reg,INT8U mask)
{
	uint8_t tmp;
	tmp = nzReadReg(reg);
	nzWriteReg(reg,(tmp|mask));
}

/**
 * @brief C L E A R   A   B I T   M A S K
 *
 * @param address
 * @param mask
 */
void nzClearBitMask(INT8U reg,INT8U mask)
{
	uint8_t tmp;
	tmp = nzReadReg(reg);
	nzWriteReg(reg,(tmp&(~mask)));
}

/**
 * @brief SET EXT REG DATA
 *
 * @param extRegAddr
 * @param extRegData
 */
void nzSetRegExt(INT8U extRegAddr,INT8U extRegData)
{
    uint8_t addr,regdata;

    addr = BFL_JREG_EXT_REG_ENTRANCE;
    regdata = BFL_JBIT_EXT_REG_WR_ADDR + extRegAddr;
    nzWriteReg(addr,regdata);

    addr = BFL_JREG_EXT_REG_ENTRANCE;
    regdata = BFL_JBIT_EXT_REG_WR_DATA + extRegData;
    nzWriteReg(addr,regdata);
}

/**
 * @brief Clear Fifo 
 *
 * @param
 * @param
 */
void nzClearFifo(void)
{
    while(nzReadReg(FIFOLEVEL&0x7f)!=0)
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
void nzStartCmd(INT8U cmd)
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
//		;
//	;
//	;
//	nrf_delay_ms(1);
	
}

/**
* @brief check errors flag if Set or Not
*
* @param 
* @param 
*/
bool nzFlagOK(void) 
{
    if((nzReadReg(REGERROR)&(BFL_JBIT_CRCERR|BFL_JBIT_PROTERR/*|BFL_JBIT_COLLERR|BFL_JBIT_PARITYERR*/))==0)
    {
        return true;
    }
    return false;
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
        return true;
    }
    return false;
}

