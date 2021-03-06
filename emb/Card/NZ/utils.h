//-----------------------------------------------------------------------------
// public.h
//-----------------------------------------------------------------------------
// Copyright 2017 nationz Ltd, Inc.
// http://www.nationz.com
//
// Program Description:
//
// driver definitions for the nfc reader
//
// PRJ:            nfc reader
// Target:         STM32F10X
// Tool chain:     KEIL
// Command Line:   None
//
// Release 1.0
//    -Initial Revision (NZ)
//    -27 Oct 2017
//    -Latest release before new firmware coding standard
//


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UTILS_H_
#define __UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include "type.h"

extern s32 gSysTick;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void delay100ns(void *arg);
void delay100us(void *arg);
void delay1ms(void *arg);
//void delay_ms(s32 dly);
u16  crc16(u8 *start_byte,u16 num_of_bytes);

void PrintInfoDump(u8* info,u16 len);
void PrintInfoDumpExt(u8 *info, u16 len, s8 *name);


#endif /* __PUBLIC_H */


