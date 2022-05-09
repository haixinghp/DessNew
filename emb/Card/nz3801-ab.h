//-----------------------------------------------------------------------------
// nz3801-ab.h
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
#ifndef __NZ_RC5XX_H_
#define __NZ_RC5XX_H_

#include "nz3801-ab_cfg.h"
#include "stdint.h"
#include "stdbool.h"
typedef struct
{
	uint32_t numFieldOnFail;
    uint32_t numFieldOffFail;
    uint32_t numL3OK;
    uint32_t numL4OK;
    uint32_t numWrRegOK;
    uint32_t Totality;
}tsSuccessRate;//成功率统计

typedef enum
{
	CT_A = 0,	// TypeA模式
	CT_B,		// TypeB模式
}teCardType;

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

extern uint8_t  FWI;
extern uint16_t FSD;
extern uint16_t FSC;
extern uint8_t  CID;         
extern uint8_t  NAD;
extern uint8_t  BlockNum;
extern const uint8_t FSCTab[];
extern const uint8_t UartSpeedTab[];


extern uint8_t  reg29h_MODGSP;
extern uint8_t  reg18h_RXTRESHOLD;
extern uint8_t  reg24h_MODWIDTH;
extern uint8_t  reg27h_GSN;
extern uint8_t  reg28h_CWGSP;
extern uint8_t  reg26h_RFCFG;
extern uint8_t  reg16h_TXSEL;
extern uint8_t  reg38h_ANALOGTEST;
extern uint16_t TxRxSpeed;
extern uint8_t  PPSEn;
extern uint32_t UartBaudRate;

extern tsSuccessRate SuccessRate;

extern uint32_t   power(uint8_t n);
extern void* mem_copy(void * dest,const void *src, uint16_t count);

extern void  nz3801SetTimer(uint32_t fc);
extern void  nz3801SetTimer2(uint8_t fwi);
extern uint8_t    nz3801HwReset(void);
extern uint8_t    nz3801SoftReset(void);
extern uint8_t    nz3801SoftPwrDown(bool bEnable);
extern uint8_t    nz3801ActivateField(bool activateField);
extern uint8_t    nz3801Init(teCardType);

extern uint8_t    nz3801Transceive(eCmd command, 
                    const uint8_t *request, uint8_t requestLength, uint8_t txalign, 
                          uint8_t *response, uint8_t *responseLength, uint8_t rxalign);
extern uint8_t    nz3801IBLOCK(const uint8_t *inf, uint16_t infLength, 
                          uint8_t *response, uint16_t *responseLength);
extern void NZ_DelayMs(uint16_t cn);
#endif
