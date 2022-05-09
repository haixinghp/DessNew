//-----------------------------------------------------------------------------
// nz3801-a_cfg.h
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


#ifndef __NZ3801_A_CFG_H_
#define __NZ3801_A_CFG_H_
#include "main.h"
#include "all_header.h"
#include "type.h"
#include "SAFE_HWType.h"
//-SOCKET NZ3801-A-----------------------------------------------------
#define SOCKET_RF_SPI_CS_DIS      M_RC523_CS_FUN_ON
#define SOCKET_RF_SPI_CS_EN       M_RC523_CS_FUN_OFF
#define SOCKET_RF_RST_DIS         M_RF_RST_FUN_ON
#define SOCKET_RF_RST_EN          M_RF_RST_FUN_OFF
#define SOCKET_RF_IRQ_INTN        M_RF_IRQ_IN_READ()

//-SPI1-------------------------------------------------------------
#define SPI1_SET_CLK_HIGH         M_SCK_FUN_ON
#define SPI1_SET_CLK_LOW          M_SCK_FUN_OFF
#define SPI1_SET_MOSI_HIGH        M_MOSI_FUN_ON
#define SPI1_SET_MOSI_LOW         M_MOSI_FUN_OFF
#define SPI1_CHK_MISO             M_MISO_IN_READ
/*
******************************************************************************
* SPI 
******************************************************************************
*/
#define SPISendRecvByte      SPI1TxRxByte       // 接口 u8 function(u8)
#define SPI_CS_Enable        SOCKET_RF_SPI_CS_EN 
#define SPI_CS_Disable       SOCKET_RF_SPI_CS_DIS

/*
******************************************************************************
* HW RST 
******************************************************************************
*/
#define RF_RST_Enable        SOCKET_RF_RST_EN
#define RF_RST_Disable       SOCKET_RF_RST_DIS

/*
******************************************************************************
* DELAY 
******************************************************************************
*/

/*
******************************************************************************
* FIELD RETRY 
******************************************************************************
*/
#define FIELD_ONOFF_RETRY_EN  1                //开关场重试: 1使能，0禁止

/*
******************************************************************************
* DEMO 
******************************************************************************
*/
#define ISO14443A_EN          1                //使能TYPEA 功能

#endif /* __SYS_CFG_H_ */



