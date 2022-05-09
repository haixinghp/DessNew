//-----------------------------------------------------------------------------
// nz3801-ab.c
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

#include "errno.h"
#include "nz3801-ab_com.h"
#include "nz3801-ab.h"
#include "nrf_delay.h"
#include "h_Mfrc522.h"

const uint8_t  FSCTab[8] = {16,24,32,40,48,64,96,128};// 256在函数中判断赋值
const uint8_t  UartSpeedTab[12]={0xFA,0xEB,0xDA,0xCB,0xAB,0x9A,0x7A,0x74,0x5A,0x3A,0x1C,0x15};

uint8_t  CType;		 // 0 TypeA, 1 TypeB
uint8_t  PCB;         // 块头域PCB字段
uint8_t  FWI = 4;     // 等待时间
u16 FSC;		 // PICC最大字节数
u16 FSD;         // PCD最大字节数
uint8_t  CID;         
uint8_t  NAD;
uint8_t  BlockNum = 0;// 当前块号


/*寄存器参数值*/
uint8_t  reg18h_RXTRESHOLD = 0x9b;
uint8_t  reg24h_MODWIDTH = 0x26;

uint8_t  reg27h_GSN = 0xff;  //
uint8_t  reg28h_CWGSP = 0x3f;

uint8_t  reg26h_RFCFG = 0x68;

uint8_t  reg29h_MODGSP = 0x2a;

uint8_t  reg16h_TXSEL = 0x10;
uint8_t  reg38h_ANALOGTEST = 0x00;

// 位速率
u16 TxRxSpeed = 106;     //默认106kBd
uint8_t  PPSEn = 1;           //TypeA读卡是否执行PPS操作,默认执行

// UART通信波特率
uint32_t UartBaudRate = 9600; //default:9600 

// 成功率统计测试
tsSuccessRate SuccessRate;

void NZ_DelayMs(uint16 cn)
{ 
//	 uint16 uint8_t_i;

//    for (;cn>0;cn--)
//    {
////        for (uint8_t_i=12000;uint8_t_i>0;uint8_t_i--)
////        {
////             __asm("NOP"); /* delay 1.003ms   2018.4.19*/
////		    }
//			nrf_delay_us(1000);
//    }
		nrf_delay_ms(cn);	
	
		
}
/**
  * @brief  power
  * @param  n
  * @retval n的2次方
  */
uint32_t power(uint8_t n)
{
    uint8_t i;
    uint32_t t;
    t = 1;
    for(i=0; i<n; i++)
    {
        t *= 2;
    }
    return t;
}

/**
  * @brief  缓存指定长度数据拷贝
  * @param  dest - 目的缓存
  * @param  src  - 源缓存
  * @param  count- 拷贝长度
  * @retval 目的缓存首地址
  */
void *mem_copy(void * dest,const void *src, u16 count)
{
    char *tmp = (char *) dest;
    char *s = (char *) src;
    while (count--)
    {
        *tmp++ = *s++;
    }
    return dest;
}

/**
 * @brief Set NZ3801-AB TIMER
 *        设置NZ3801-AB的射频超时时间 方式1
 * @param
 * @param
 */
void nz3801SetTimer(uint32_t fc)//domod TPrescalEven bit=0
{
	uint32_t prescale = 0;
	uint32_t t;

	t = fc;
	while(fc>65535)
	{
		prescale++;
		fc = t/(2*prescale+1);
		if(fc*(2*prescale+1) != t)
			fc++;
	}

	if(prescale>=4096)
	{
		fc = 65535;
		prescale = 4095;
	}

	nzWriteReg(TMODE, 0x90|((prescale>>8)&0xf));
	nzWriteReg(TPRESCALER, prescale&0xff);
	nzWriteReg(TRELOADHI, (fc>>8)&0xff);
	nzWriteReg(TRELOADLO, fc&0xff);
}

/**
 * @brief Set NZ3801-AB TIMER
 *        设置NZ3801-AB的射频超时时间 方式2
 * @param
 * @param
 */
void nz3801SetTimer2(uint8_t fwi)
{
    switch(fwi)
    {
        case 0:                         // (0.302 ms) FWI=0					  
            nzWriteReg(TPRESCALER, 0x20); 
            nzWriteReg(TRELOADLO, 0x3E); 
            nzWriteReg(TRELOADHI, 0);
            break;
        case 1:                         // (0.604 ms) FWI=1
            nzWriteReg(TPRESCALER, 0x20);
            nzWriteReg(TRELOADLO, 0x7E); 	
            nzWriteReg(TRELOADHI, 0);			          				
            break;
        case 2:                         // (1.208 ms) FWI=2
            nzWriteReg(TPRESCALER, 0x20); 
            nzWriteReg(TRELOADLO, 0xFC);
            nzWriteReg(TRELOADHI, 0);
            break;
        case 3:                         // (2.416 ms) FWI=3
            nzWriteReg(TPRESCALER, 0x41); 
            nzWriteReg(TRELOADLO, 0xFC); 
            nzWriteReg(TRELOADHI, 0);
            break;
        case 4:                         // (4.833 ms) FWI=4
            nzWriteReg(TPRESCALER, 0x82); 
            nzWriteReg(TRELOADLO, 0xFC); 
            nzWriteReg(TRELOADHI, 0);
            break;
        case 5:                         // (9.666 ms) FWI=5
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0xF8); 
            nzWriteReg(TRELOADHI, 0X01);
            break;
        case 6:                         // (19.33 ms) FWI=6
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0xF0); 
            nzWriteReg(TRELOADHI, 0X03);
            break;
        case 7:                         // (38.66 ms) FWI=7
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0xE0); 
            nzWriteReg(TRELOADHI, 0X07);
            break;
        case 8:                         // (77.32 ms) FWI=8
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0xC0); 
            nzWriteReg(TRELOADHI, 0X0F);
            break;
        case 9:                         // (154.6 ms) FWI=9
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0x80); 
            nzWriteReg(TRELOADHI, 0X1F);
            break;
        case 10:                        // (309.3 ms) FWI=10
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0x00); 
            nzWriteReg(TRELOADHI, 0X3F);
            break;
        case 11:                        // (618.6 ms) FWI=11
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0x00); 
            nzWriteReg(TRELOADHI, 0X7E);
            break;
        case 12:                        // (1.2371 s) FWI=12
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0x00); 
            nzWriteReg(TRELOADHI, 0XFC);
            break;
        case 13:                        // (2.4742 s) FWI=13
            nzWriteReg(TMODE, 0xF);
            nzWriteReg(TPRESCALER, 0xFF);
            nzWriteReg(TRELOADLO, 0xFF); 
            nzWriteReg(TRELOADHI, 0X0F);
            break;
        case 14:                        // (4.9485 s) FWI=14
            nzWriteReg(TMODE, 0xF);
            nzWriteReg(TPRESCALER, 0xFF);
            nzWriteReg(TRELOADLO, 0xFF); 
            nzWriteReg(TRELOADHI, 0X1F);
            break;
        case 15:                        // (4.9485 s) FWI=15
            nzWriteReg(TMODE, 0xF);
            nzWriteReg(TPRESCALER, 0xFF);
            nzWriteReg(TRELOADLO, 0xFF); 
            nzWriteReg(TRELOADHI, 0X3F);
            break;
        default:                        //
            nzWriteReg(TPRESCALER, 0x82);
            nzWriteReg(TRELOADLO, 0xC0); 
            nzWriteReg(TRELOADHI, 0X0F);
            break;
    }
    nzSetBitMask(TMODE, 0x80);
}

/**
 * @brief nz3801HwReset  
 *        硬掉电
 * @param
 * @param
 */
uint8_t nz3801HwReset(void)
{
  // 	RF_RST_Enable;
      NZ_DelayMs(10);
  // RF_RST_Disable;  
      NZ_DelayMs(10);
    return ERR_NONE;
}

/**
 * @brief nz3801SoftReset  
 *        软复位
 * @param
 * @param
 */
uint8_t nz3801SoftReset(void)
{
    nzWriteReg(COMMAND,COMMAND_SOFTRESET);
     NZ_DelayMs(10);
    return ERR_NONE;
}

/**
 * @brief nz3801SoftPwrDown  
 *        软掉电
 * @param
 * @param
 */
uint8_t nz3801SoftPwrDown(bool bEnable)
{
    if(bEnable)
    {
        nzSetBitMask(COMMAND,BFL_JBIT_POWERDOWN);
    }
    else
    {
        nzClearBitMask(COMMAND,BFL_JBIT_POWERDOWN);
    }
    return ERR_NONE;
}

/**
 * @brief nz3801ActivateField
 *        
 * @param activateField - true:on ; false:off
 * @param
 */
uint8_t nz3801ActivateField(bool activateField)
{
    uint8_t reg_val;
#if FIELD_ONOFF_RETRY_EN
    uint8_t retry = 5; 
    if(activateField)
    {
        while(retry--)
        {
//					  my_printf("nzSetBitMask(TXCONTROL,0x03) true\n");
            nzSetBitMask(TXCONTROL,0x03);
            reg_val = nzReadReg(TXCONTROL);
            if((reg_val&0x03)==0x03)
            {
                break;
            }
        }
        if(retry==0)
        {
            return ERR_ONOFFFIELD;  
        }
    }
    else
    {
        while(retry--)
        {
					 
//					  my_printf("nzClearBitMask(TXCONTROL,0x03) false\n");
            nzClearBitMask(TXCONTROL,0x03);
            reg_val = nzReadReg(TXCONTROL);
            if((reg_val&0x03)==0x00)
            {
                break;
            }
        }
        if(retry==0)
        {
            return ERR_ONOFFFIELD;  
        }
    }
#else
    if(activateField)
    {
		nzSetBitMask(TXCONTROL,0x03);
        reg_val = nzReadReg(TXCONTROL);
        if((reg_val&0x03)!=0x03)
        {
            //my_printf("Field ON FAIL!!!\r\n");
            return ERR_ONOFFFIELD;  
        }
	}
	else
    {
		nzClearBitMask(TXCONTROL,0x03);
        reg_val = nzReadReg(TXCONTROL);
        if((reg_val&0x03)!=0x00)
        {
            //my_printf("Field OFF FAIL!!!\r\n");
            return ERR_ONOFFFIELD;  
        }   
	} 
#endif

	return ERR_NONE;  
}

/**
 * @brief nz3801Init
 *        根据输入的协议类型初始化RC5XX寄存器
 * @param
 * @param
 */
uint8_t nz3801Init(teCardType card)
{
  nzWriteReg(COMMAND,COMMAND_SOFTRESET);
	nzWriteReg(COMMIEN,0x00);
	nzWriteReg(DIVIEN,0x00);//禁止所有中断
	nzWriteReg(COMMIRQ,0x3f);
	nzWriteReg(DIVIRQ,0x3f);//清除中断位标示
	nzClearBitMask(STATUS2,BIT3);//非m1加密模式

	nzSetRegExt(0x01,0x21);
	nzSetRegExt(0x03,0x04);
	nzWriteReg(RXSEL,0x88);// 8个bit位后接收所有后续数据, 8*128fc，速度不为106时需调整
	if(card==CT_B)// TypeB
	{
		nzWriteReg(TXMODE,0x00|BFL_JBIT_CRCEN|BFL_JBIT_106KBPS|BFL_JBIT_TYPEB);
		//crc on /106k/ no inverted/TxMix off/RxFraming 14443b
		nzWriteReg(RXMODE,0x00|BFL_JBIT_CRCEN|BFL_JBIT_106KBPS|BFL_JBIT_TYPEB );
		//crc on /106k/ not valid received/RxMultiple inable/RxFraming 14443b
		nzWriteReg(TXAUTO,0x00); //no Force100ASK
		//no AutoRFOFF/no Force100ASK/ AutoWakeUp 0/CAOn/ InitialRFOn/Tx2RFAutoEn off/Tx1RFAutoEn off
		nzWriteReg(RXTRESHOLD,0x68 );	//0x43(0x64)
		//not sure 0  use osc  //  8f
		nzWriteReg(GSN, 0xfa);// 低四位改变调制深度，数值越小，调制深度越大；
		//改变调制深度，数值越小，调制深度越大
		//CWGsNOn/ModGsNOn
		nzWriteReg(MODGSP, reg29h_MODGSP);//#2(12.8%/)   // 0x2a

		nzWriteReg(TYPEBREG, 0xc0);
		//Initiator
		nzWriteReg(RFCFG, reg26h_RFCFG);		 //0x64
		nzWriteReg(DEMOD, 0X5D);  //19 BIT4
		nzWriteReg(BITFRAMING, 0);
		CType = CT_B;
	}
	else // TypeA
	{
		nzWriteReg(TXMODE,0x00|BFL_JBIT_106KBPS|BFL_JBIT_TYPEA); 
		//crc off /106k/ no inverted./TxMix off/RxFraming 14443A	 
		nzWriteReg(RXMODE,0x00|BFL_JBIT_106KBPS|BFL_JBIT_TYPEA);
		//crc off /106k/  valid received/RxMultiple inable/RxFraming 14443A
		nzWriteReg(TXAUTO,0X00|BFL_JBIT_FORCE100ASK);
		//no AutoRFOFF/ Force100ASK/ AutoWakeUp 0 /CAOn/InitialRFOn/ Tx2RFAutoEn off/ Tx1RFAutoEn off
		nzWriteReg(MODWIDTH, 0x26);
		//miler 
		nzWriteReg(RXTRESHOLD, reg18h_RXTRESHOLD);// 0x8b 0x9b 0x7b(0x66)  
		
		//not sure 0  use osc    // 8f
		nzWriteReg(GSN, reg27h_GSN);    // 0x8f  0xff
		//Initiator
		nzWriteReg(RFCFG, reg26h_RFCFG);	 // 0x78
		nzWriteReg(BITFRAMING, 0);
		nzWriteReg(TYPEBREG, 0x00);

    CType = CT_A;
	}

	nzWriteReg(CONTROL, 0x40);    // stop timer	
	nzWriteReg(CWGSP, reg28h_CWGSP);      // 10  1f  3f  	 
    nzSetBitMask(TXCONTROL, 0x83);//nzClearBitMask(TXCONTROL,0x03); 

    nzSetBitMask(TXSEL, reg16h_TXSEL);  //MFOUT Test  14 15     
    nzSetBitMask(ANALOGTEST, reg38h_ANALOGTEST);// AUX Test 24 56 cd   
 
	return ERR_NONE;   
}

/**
 * @brief nz3801Transceive
 * @param txalign  最后1个字节发送多少bit
 * @param rxalign  收数据时从第几个bit开始
 * @param len  发送数据长度，为0时只收不发
 * @param I-BLOCK的分帧不在该函数内进行，每次最多收发256字节
 */
uint8_t nz3801Transceive(eCmd command, 
    const uint8_t *request, uint8_t requestLength, uint8_t txalign, 
    uint8_t *response, uint8_t *responseLength, uint8_t rxalign)
{
//	my_printf(">invoke nz3801Transceive,CType=%d\r\n",CType);
    uint32_t t,tmp;
    uint8_t  i;
    uint8_t  err;
    uint8_t  rec_times;
    uint8_t  noise=0,timerout=0,ov=0; //标志

    if( requestLength==0 || !request || !response || !responseLength )  
		return ERR_PARA;
    
    if(CType==CT_B && command==TA_IBLOCK)
    {
        command = TB_IBLOCK;
    }
    
    nzStopCmd();
    nzClearFifo();
    nzClearFlag();

    // 设置CRC和PARITY校验
    if(CType==CT_A)
    {
        nzSetPARITY(true); 
    }
    else
    {
        nzSetPARITY(false); 
    }
    if(command==TA_REQA||command==TA_WUPA||command==TA_ANT)
    {
        nzSetCRC(false);
    }
    else
    {
        nzSetCRC(true);
    }
    
    // 设置发送/接收位对齐方式
    if(txalign>=8 || rxalign>=8)
    {
        return ERR_PARA;
    }
    if(txalign!=0 || rxalign!=0)
    {
        nzWriteReg(BITFRAMING, (rxalign<<4)|txalign);
    }
    else 
    {
        nzWriteReg(BITFRAMING,0);
    }

     NZ_DelayMs(1);
//		NZ_DelayMs(10);
    // 设置timeout时间
    if(command==TA_REQA||command==TA_WUPA||command==TA_ANT||command==TA_SELECT||command==TA_HLTA)
    {
        t = 9*128+20;
    }
    else if(command==TA_RATS || command==TA_PPS)
    {
        t = 559*128+20;
    }
    else if(command==TB_WUPB)
    {
        t = 7680;
    }
    else    // I BLOCK
    {
        if((request[0]&0xf0)==0xf0 && command==TA_RSBLOCK) // wtx
        {
            t = (request[1]&0x3f)*power(FWI);
        }
        else
        {
            t = power(FWI);
        }
        t = t * (4096) + 49152;            // 2.5,20160714

        tmp = (4096)*power(14)+49152;       // 2.5,20160714 两边文档不一致，理论上不应该 +384
        if(t>tmp)
        {
            t = tmp;   
        }
    }

    nz3801SetTimer(t);//mask by banglin.zhang@20181124
		
//		nz3801SetTimer2(FWI);

//    NZ_DelayMs(10);
    // 写入所有数据，64为本地fifo大小，len的大小由PICC和外部分帧决定
    if((TA_IBLOCK==command)||(command==TB_IBLOCK))
    {
        nzWriteReg(FIFODATA, PCB);
    }
    for(i=0; i<requestLength&&i<64-3; i++)
    {
        nzWriteReg(FIFODATA, request[i]);
    }
  	nzStartCmd(COMMAND_TRANSCEIVE);
  
    nzSetBitMask(BITFRAMING,BFL_JBIT_STARTSEND);// 启动发送和接收
    
    while(i<requestLength) // 写入后续数据
    {
        if(nzReadReg(FIFOLEVEL) < (64-2))
        {
            nzWriteReg(FIFODATA, request[i++]);
        }
    }
//    my_printf(">while((nzReadReg(COMMIRQ)&BFL_JBIT_TXI)==0)\r\n");
		uint16_t retry_cnt=0;
    while((nzReadReg(COMMIRQ)&BFL_JBIT_TXI)==0)
		{

			retry_cnt++;
			NZ_DelayMs(5);
			if(retry_cnt>400)
			{
				my_printf("(nzReadReg(COMMIRQ)&BFL_JBIT_TXI)==0 ERR_TIMEOUT\n");
				retry_cnt=0;
				fn_Rc522_Poweroff();
				NZ_DelayMs(50);
				fn_ConfigRc522Gpio();
				return ERR_TIMEOUT;
			}
		}
    nzWriteReg(COMMIRQ, 0x01); 
//		my_printf("> nzWriteReg(COMMIRQ, 0x01); \r\n");
//    if(command==TB_ATTRIB)      
//    {   
////        while(nzReadReg(TCOUNTERVALHI)<=0x03);
////        while(nzReadReg(TCOUNTERVALLO)<=0x0f);
//        nzStartCmd(COMMAND_RECEIVE);// 启动接收
//    }

    rec_times = 0;
REC:
    while(1)
    {
        if((nzReadReg(STATUS2)&0x07) <= 0x01)// 停止接受数据
        {
            break;
        }
        
        if(nzReadReg(FIFOLEVEL) != 0) 
        {
            break;
        }
        
        if((nzReadReg(COMMIRQ) & BFL_JBIT_TIMERI))// 超时
        {
            if(timerout==0)// timerout后再延时400us继续收后续数据
            {
                timerout = 1;
                NZ_DelayMs(1);
                continue;
            }
            
            nzSetBitMask(CONTROL, BFL_JBIT_TSTOPNOW);
            return ERR_TIMEOUT;
        }
    }
 
    *responseLength = 0;
		int n=0;
    while((i=(nzReadReg(STATUS2))&0x07)>0x01) // 在接收数据  updata by banglin.zhang@20181125
    {
			n=nzReadReg(FIFOLEVEL);
			while(n)
			{				 
					do
					{
						n--;
							response[*responseLength] = nzReadReg(FIFODATA);
            if(*responseLength >= 256-2)      // 2字节CRC，只有I-BLOCK才会存在这种情况
                ov = 1;
            else
									*responseLength += 1; 
					}while(n);
					n=nzReadReg(FIFOLEVEL);
			}
				
    }

    while(nzReadReg(FIFOLEVEL)!=0) // 接收后续数据
    {
        response[*responseLength] = nzReadReg(FIFODATA);
        if(*responseLength >= 256-2)      // 2字节CRC，只有I-BLOCK才会存在这种情况
            ov = 1;
        else
            *responseLength += 1;
    }
//		while((i=(nzReadReg(STATUS2))&0x07)>0x01) // 在接收数据
//		{
//				while(nzReadReg(FIFOLEVEL)!=0)
//				{
//						response[*responseLength] = nzReadReg(FIFODATA);
//						if(*responseLength >= 256-2)      // 2字节CRC，只有I-BLOCK才会存在这种情况
//								ov = 1;
//						else
//								*responseLength += 1; 
//				}
//		}
//		while(nzReadReg(FIFOLEVEL)!=0) // 接收后续数据
//		{
//				response[*responseLength] = nzReadReg(FIFODATA);
//				if(*responseLength >= 256-2)      // 2字节CRC，只有I-BLOCK才会存在这种情况
//						ov = 1;
//				else
//						*responseLength += 1;
//		}
    
    if(ov)
    {
        nzSetBitMask(CONTROL, BFL_JBIT_TSTOPNOW);
        return ERR_OVERLOAD;
    }
    
    if(command==TA_REQA||command==TA_WUPA||command==TA_HLTA||command==TA_ANT||command==TA_SELECT||command==TB_WUPB)
    {
        nzSetBitMask(CONTROL, BFL_JBIT_TSTOPNOW);
        return ERR_NONE;
    }

    // 判断是否为noise，后续noise判断只有RATS和I-BLOCK
    noise = 0;
    //secstatus = nzReadReg(CONTROL);
    err = nzReadReg(REGERROR);
//    if((nzReadReg(COMMIRQ)&BFL_JBIT_TIMERI))// 超时
//    {
//        timerout = 1;
//    }
    
    nzWriteReg(RXSEL, 0x82);
    nzStartCmd(COMMAND_RECEIVE);//need?
    nzSetBitMask(CONTROL,BFL_JBIT_TSTARTNOW);  
    
    // CRC错
    if((err&BFL_JBIT_CRCERR)!=0)
    {
        if((command==TB_ATTRIB) && (*responseLength==0) && ((err&BFL_JBIT_PROTERR)==0))// Protocol error，无CRC的情况
        {
            noise = 1;// 2.5,2060714,return ERR_PROTOCOL;
        }
        if(*responseLength<4)
        {
            noise = 1;
        }
    }
    if(*responseLength<2)// +CRC，小于4字节
    {
        if((err&(BFL_JBIT_CRCERR|BFL_JBIT_PARITYERR))!=0)// CRC或者奇偶错
            noise = 1;
    }
    if((err&BFL_JBIT_COLLERR)!=0)// 位冲突
    {
        noise = 1;
    }
    if(*responseLength==0)// 没有收到有效数据
    {
        noise = 1;
    }
    if(((err&BFL_JBIT_PROTERR)!=0) && (*responseLength<2))// 接收开始时间 <FDT(picc,min)
    {
        noise = 1;
    }
    if(noise)
    {
        nzWriteReg(COMMIRQ, 0x7e);
        nzWriteReg(DIVIRQ, 0x7f);
        nzClearFifo();
        if(++rec_times<5)
            goto REC;
        else
            return ERR_TIMEOUT;
    }
    else
    {
        if ((err&(BFL_JBIT_COLLERR|BFL_JBIT_PROTERR|BFL_JBIT_PARITYERR|BFL_JBIT_CRCERR))!=0)
        {
            nzWriteReg(COMMAND, 0);
            nzSetBitMask(CONTROL, BFL_JBIT_TSTOPNOW);   
            return ERR_PARA;
        }
    }
    nzWriteReg(COMMAND, 0);
    nzSetBitMask(CONTROL, BFL_JBIT_TSTOPNOW);
    
    return ERR_NONE;
     
}
#include "h_Mfrc522.h"
extern INT8S PcdComMF522(INT8U Command,
                  INT8U *pInData,
                  INT8U InLenByte,
                  INT8U *pOutData,
                  INT16U  *pOutLenBit);
/**
 * @brief IBLOCK
 *
 * @param
 * @param
 */
uint8_t nz3801IBLOCK(const uint8_t *inf, u16 infLength, uint8_t *response, u16 *responseLength)
{
	uint8_t rec[256+2];
	u16 sp, l;  // sp 已发送长度,也可用来表示发送pcb的位置;
	uint8_t rlen;
	uint8_t r;
	uint8_t RB[5];
	uint8_t timer, re;
	uint8_t tmp;
	uint8_t brec;	// 是否在接收数据状态
	uint8_t Rerr;

	#define ACK		0
	#define NAK		0x10
	#define SendRB(type)	if(++timer>=3+Rerr) return ERR_TIMEOUT; RB[0]=0xa2|type|BlockNum;r = nz3801Transceive(TA_RSBLOCK, RB, 1, 0, rec, &rlen, 0);
	#define SendFinish()	((PCB&BIT4)==0)


	/*mem_copy(send+1, (uint8_t*)"\x00\xa4\x04\x00\x0e", 5);
	slen = 5;
	sp = strlen("2PAY.SYS.DDF01")+1;
	mem_copy(send+1+5, "2PAY.SYS.DDF01", sp);
	slen += sp;*/
	
	*responseLength = 0;
	sp = 0;
	while(1)
	{
		Rerr = 0;
		// 发送过程
		if(infLength-sp+1+2>FSC)	// 剩余数据长度+PCB+CRC
		{
			l = FSC-2;			// 最大值-CRC
			PCB = 0x12;
		}
		else
		{
			l = infLength-sp+1;		// PCB
			PCB = 0x02;
		}
		PCB |= BlockNum;
		timer = re = 0;

SEND:
		r = nz3801Transceive(TA_IBLOCK, inf+sp, l-1, 0, rec, &rlen, 0);
		brec = 0;
        
CHECK:
		if(r==ERR_TIMEOUT)
		{
			if(brec)
			{
				SendRB(ACK);
			}
			else
			{
				SendRB(NAK);
			}

			goto CHECK;
		}
		if(r==ERR_OVERLOAD)		// 接收数据超长
		{
			return ERR_PROTOCOL;
		}
		if(r==ERR_PARA)
		{
			if(brec)
			{
				SendRB(ACK);
			}
			else
			{
				SendRB(NAK);
			}

			goto CHECK;
		}

		if(rlen==0) // xu.kai added 20170331
		{
				return ERR_PROTOCOL;
		}
        
		// 协议错误
		tmp = (rec[0]>>6)&0x3;
		if(tmp==0x00)	// I BLOCK
		{
			if((rec[0]&BIT5)!=0)	
                return ERR_PROTOCOL;// b6=1,2.5,20160714
			if((rec[0]&BIT1)==0)	
                return ERR_PROTOCOL;// b2=0
			if((rec[0]&(BIT2|BIT3))!=0)	
                return ERR_PROTOCOL;// 有CID, NAD
			if((rec[0]&0x01)!=BlockNum)	
                return ERR_PROTOCOL;// 块号不对
			if(!SendFinish())	
                return ERR_PROTOCOL;// 有后续数据要发送，不应该回I BLOCK
			Rerr = 0;
			timer = 0;
			BlockNum = 1 - BlockNum;
			if(brec==0)	
      {
        infLength = 0;	// slen为接收数据位置
			}
      brec = 1;
			mem_copy(response+*responseLength, rec+1, rlen-1);
			*responseLength += (rlen-1);
//				my_printf("rec[0]=%02X,(rlen-1)=%d\r\n",rec[0],(rlen-1));
			if((rec[0]&BIT4)==0)	// 无链接帧
			{
			
				return ERR_NONE;
			}
			else
			{
				
				SendRB(ACK);
				timer = 0;	// 正常ACK
				goto CHECK;
			}

		}
		else if(tmp==0x02)	// R BLOCK
		{
			Rerr = 1;
			if((rec[0]&(BIT2|BIT3))!=0)
				return ERR_PROTOCOL;
			if((rec[0]&BIT5)==0)
				return ERR_PROTOCOL;
			if((rec[0]&(BIT0|BIT1))==1)// 2.5,20160714
				return ERR_PROTOCOL;
			if((rec[0]&BIT4)!=0)		// NAK
				return ERR_PROTOCOL;
			else
			{
				if(brec)	
                   return ERR_PROTOCOL;	// 接收数据过程中, PICC是不应该发送ACK的
				if((rec[0]&0x01)!=BlockNum)
				{
					if(++re>=3) 
                return ERR_TIMEOUT;
					goto SEND;
				}
				if(SendFinish())	
           return ERR_PROTOCOL;	// 此时应该收到数据
				BlockNum = 1-BlockNum;
				sp += (l-1);
				continue;	// 继续发送后续数据
			}
		}
		else if(tmp==0x03)	// S BLOCK
		{
			uint8_t t;
			uint8_t sb[2];
            
			t = (rec[0]>>4)&0x03;
			if(t==0)	
                return ERR_PROTOCOL;// DESELECT
			else if(t!=3)	
               return ERR_PROTOCOL; // 非WTX
			t = rec[0]&0x02;
			if(t==0)	
                return ERR_PROTOCOL;
			if(rlen!=2 || rec[1]==0||rec[1]>59)	
                return ERR_PROTOCOL;	// WTX=0
			sb[0] = 0xf2;
			sb[1] = rec[1];
			r = nz3801Transceive(TA_RSBLOCK, sb, 2, 0, rec, &rlen, 0);	// 发送wtx
			goto CHECK;
		}
		else
        {
					return ERR_PROTOCOL;
		}
	}
}

