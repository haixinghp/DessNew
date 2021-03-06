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
#ifndef __NZ_RC5XX_H_
#define __NZ_RC5XX_H_

#include "nz3801-a_cfg.h"

typedef enum
{
	CT_A = 0,	// TypeA模式
	CT_B		// TypeB模式
}eCardType;

typedef enum
{
	TA_REQA,
	TA_WUPA,
	TA_HLTA,
	TA_ANT,
	TA_SELECT,
	TA_RATS,
	TA_PPS,
	TA_IBLOCK,
	TA_RSBLOCK,
	TA_REQB,
	TB_WUPB,
	TB_ATTRIB,
	TB_HLTB,
	TB_IBLOCK,
	TB_xBLOCK, //二代身份证
	CMD_TOTAL
}eCmd;

extern u8  FWI;
extern u16 FSD;
extern u16 FSC;
extern u8  CID;         
extern u8  NAD;
extern u8  BlockNum;
extern const u8 FSCTab[];

extern u32  power(u8 n);
extern void *mem_copy(void * dest,const void *src, u16 count);

extern void nz3801ASetTimer(u32 fc);
extern void nz3801ASetTimer2(u8 fwi);
extern u8   nz3801AHwReset(void);
extern u8   nz3801ASoftPwrDown(void);
extern u8   nz3801AActivateField(bool activateField);
extern u8   nz3801AInit(eCardType);
extern u8   nz3801ATransceive(eCmd command, 
                    const u8 *request, u8 requestLength, u8 txalign, 
                          u8 *response, u8 *responseLength, u8 rxalign);
extern u8   nz3801AIBLOCK(const u8 *inf, u16 infLength, 
                          u8 *response, u16 *responseLength);
extern void NZ_DelayMs(uint16 cn);
#endif

