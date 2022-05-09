/***************************************************************************//**
 * @file
 *   h_Mfrc522.h
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
#ifndef MFRC522_H
#define MFRC522_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MFRC522_GLOBALS
#define MFRC522_EXT
#else
#define MFRC522_EXT extern
#endif

#include "h_TypeDef.h"

#define  NZ3801_IIC_ADDR  0x50        //I2C=1 EA=0

#define	FREQ_SPLI_302us					0x7FF
#define	RIC_DELAY5MS					17
#define PCD_DELAY555MS					250

/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  0x3F

/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define MI_OK                          0
#define MI_NOTAGERR                    (-1)
#define MI_ERR                         (-2)

#define		SEARCH_ERR		0x01
#define		CRASH_ERR		0x02
#define		SELECT_ERR		0x03
#define		AUTHEN_ERR		0x04
#define		READ_ERR		0x05
#define		WRITE_ERR		0x06
#define		WRITE_CARD		0x07
#define		READ_CARD		0x08
#define		MEM_ERR			0xFD//EEPROM里没找到对应NFC卡号
#define		CARD_EXISTED	0xFE//EEPROM里没找到对应NFC卡号
#define		EEPROM_FULL		0xFF//EEPROM里没找到对应NFC卡号 

/////////////////////////////////////////////////////////////////////
//函数原型
/////////////////////////////////////////////////////////////////////
extern void fn_ConfigRc522Gpio(void);
INT8S PcdReset(void);
void PcdAntennaOn(void);
void PcdAntennaOff(void);
INT8S PcdRequest(INT8U req_code,INT8U *pTagType);   
INT8S PcdAnticoll(INT8U *pSnr);
INT8S PcdSelect(INT8U *pSnr);         
INT8S PcdAuthState(INT8U auth_mode,INT8U addr,INT8U *pKey,INT8U *pSnr);     
INT8S PcdRead(INT8U addr,INT8U *pData);     
INT8S PcdWrite(INT8U addr,INT8U *pData);    
INT8S PcdValue(INT8U dd_mode,INT8U addr,INT8U *pValue);   
INT8S PcdBakValue(INT8U sourceaddr, INT8U goaladdr);                                 
INT8S PcdHalt(void);
INT8S PcdComMF522(INT8U Command, 
                 INT8U *pInData, 
                 INT8U InLenByte,
                 INT8U *pOutData, 
                 INT16U  *pOutLenBit);
void CalulateCRC(INT8U *pIndata,INT8U len,INT8U *pOutData);
void WriteRawRC(INT8U Address,INT8U value);
INT8U ReadRawRC(INT8U Address); 
void SetBitMask(INT8U reg,INT8U mask); 
void ClearBitMask(INT8U reg,INT8U mask); 
void fn_MFRC522_Sleep_init(void);
void fn_MFRC522_Exit_Sleep(void);
void fn_ConfigRc522Gpio(void);
void fn_Rc522_Poweroff(void);

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
MFRC522_EXT INT8S fn_M500PcdConfigISOType(INT8S u8_Type);

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
MFRC522_EXT void fn_Rc522_GpioSleep(void);

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
MFRC522_EXT INT8U fn_ReadAddCardIdBak(INT8U u8_times);
MFRC522_EXT INT8U fn_ReadAddCardId(INT8U u8_times);	

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
MFRC522_EXT INT8U fn_ReadCardId(INT8U u8_TryTimes);	

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
extern unsigned char fn_CheckCardKey(void);

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
MFRC522_EXT INT8U fn_WriteCardBlock(void);

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
MFRC522_EXT INT8U fn_DelCardId(INT8U *p_CardData);

#ifdef  __cplusplus
    }
#endif

#endif
/***************************************************************************//**
 * @@end of file
 ******************************************************************************/
