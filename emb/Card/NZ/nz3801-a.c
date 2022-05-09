//-----------------------------------------------------------------------------
// nz3801-a.c
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

#include "Drive\Card\NZ\errno.h"
#include "Drive\Card\NZ\nz3801-a_com.h"
#include "Drive\Card\NZ\nz3801-a.h"
#include "Drive\Card\NZ\nz3801-a_cfg.h"
const u8  FSCTab[8] = {16,24,32,40,48,64,96,128};// 256�ں������жϸ�ֵ
//const u8  SerialSpeedTab[12]={0xFA,0xEB,0xDA,0xCB,0xAB,0x9A,0x7A,0x74,0x5A,0x3A,0x1C,0x15};

u8  CType;		 // 0 TypeA, 1 TypeB
u8  PCB;         // ��ͷ��PCB�ֶ�
u8  FWI = 4;     // �ȴ�ʱ��
u16 FSC;		 // PICC����ֽ���
u16 FSD;         // PCD����ֽ���
u8  CID;         
u8  NAD;
u8  BlockNum = 0;// ��ǰ���

void NZ_DelayMs(uint16 cn)
{ 
	 uint16 u8_i;

    for (;cn>0;cn--)
    {
        for (u8_i=3300;u8_i>0;u8_i--)
        {
             __asm("NOP"); /* delay */
					  __asm("NOP"); /* delay */
		    }
    }
}

/**
  * @brief  power
  * @param  n
  * @retval n��2�η�
  */
u32 power(u8 n)
{
    u8 i;
    u32 t;
    t = 1;
    for(i=0; i<n; i++)
    {
        t *= 2;
    }
    return t;
}

/**
  * @brief  ����ָ���������ݿ���
  * @param  dest - Ŀ�Ļ���
  * @param  src  - Դ����
  * @param  count- ��������
  * @retval Ŀ�Ļ����׵�ַ
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
 * @brief Set NZ3801-A TIMER
 *        ����NZ3801-A����Ƶ��ʱʱ�� ��ʽ1
 * @param
 * @param
 */
void nz3801ASetTimer(u32 fc)//domod TPrescalEven bit=0
{
	u32 prescale = 0;
	u32 t;

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
 * @brief Set NZ3801-A TIMER
 *        ����NZ3801-A����Ƶ��ʱʱ�� ��ʽ2
 * @param
 * @param
 */
void nz3801ASetTimer2(u8 fwi)
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
        case 15:                        // (4.9485 s) FWI=14
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
 * @brief nz3801AHwReset  
 *        Ӳ����
 * @param
 * @param
 */

u8 nz3801AHwReset(void)
{
   	RF_RST_Enable;
    NZ_DelayMs(10);
   	RF_RST_Disable;  
    NZ_DelayMs(10);
   // nzWriteReg(COMMAND,COMMAND_SOFTRESET);
    NZ_DelayMs(20);
    return 0;
}

/**
 * @brief nz3801ASoftPwrDown  
 *        �����
 * @param
 * @param
 */
u8 nz3801ASoftPwrDown(void)
{
    nzWriteReg(COMMAND,0x0f);
    nzWriteReg(COMMAND,BFL_JBIT_POWERDOWN);
    return 0;
}

/**
 * @brief nz3801AActivateField
 *        
 * @param activateField - TRUE:on ; FALSE:off
 * @param
 */
u8 nz3801AActivateField(bool activateField)
{
    u8 reg_val;
#if FIELD_ONOFF_RETRY_EN
    u8 retry = 5; 
    if(activateField)
    {
        while(retry--)
        {
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
            return ERR_ONOFFFIELD;  
        }
	}
	else
    {
		nzClearBitMask(TXCONTROL,0x03);
        reg_val = nzReadReg(TXCONTROL);
        if((reg_val&0x03)!=0x00)
        {
            return ERR_ONOFFFIELD;  
        }   
	} 
#endif

	return ERR_NONE;  
}

/**
 * @brief nz3801AInit
 *        ���������Э�����ͳ�ʼ��RC5XX�Ĵ���
 * @param
 * @param
 */
u8 nz3801AInit(eCardType card)
{
	nzWriteReg(COMMAND,0x0F);
	nzWriteReg(COMMIEN,0x00);
	nzWriteReg(DIVIEN,0x00);//��ֹ�����ж�
	nzWriteReg(COMMIRQ,0x3f);
	nzWriteReg(DIVIRQ,0x3f);//����ж�λ��ʾ
	nzClearBitMask(STATUS2,BIT3);//��m1����ģʽ

	nzWriteReg(RXSEL,0x88);
	nzWriteReg(TXMODE,0x00|BFL_JBIT_106KBPS|BFL_JBIT_TYPEA); 
	//crc off /106k/ no inverted./TxMix off/RxFraming 14443A	 
	nzWriteReg(RXMODE,0x00|BFL_JBIT_106KBPS|BFL_JBIT_TYPEA);
	//crc off /106k/  valid received/RxMultiple inable/RxFraming 14443A
	nzWriteReg(TXAUTO,0X00|BFL_JBIT_FORCE100ASK);
	//no AutoRFOFF/ Force100ASK/ AutoWakeUp 0 /CAOn/InitialRFOn/ Tx2RFAutoEn off/ Tx1RFAutoEn off
	nzWriteReg(MODWIDTH, 0x26);
	//miler 
	nzWriteReg(RXTRESHOLD, 0x9b);// 0x8b 0x9b 0x7b(0x66)  
	
	//not sure 0  use osc    // 8f
	nzWriteReg(GSN, 0xff);    // 0x8f  0xff
	//Initiator
	nzWriteReg(RFCFG, 0x58);	 // 0x78
	nzWriteReg(BITFRAMING, 0);
	nzWriteReg(TYPEBREG, 0x00);
    
	nzWriteReg(CONTROL, 0x40);    // stop timer	
	nzWriteReg(CWGSP, 0x3f);      // 10  1f  3f  	 
    nzSetBitMask(TXCONTROL, 0x83);//nzClearBitMask(TXCONTROL,0x03); 

    CType = CT_A;
	return ERR_NONE;   
}

#if ISO14443A_EN
/**
 * @brief nz3801ATransceive
 * @param txalign  ���1���ֽڷ��Ͷ���bit
 * @param rxalign  ������ʱ�ӵڼ���bit��ʼ
 * @param len  �������ݳ��ȣ�Ϊ0ʱֻ�ղ���
 * @param I-BLOCK�ķ�֡���ڸú����ڽ��У�ÿ������շ�256�ֽ�
 */
u8 nz3801ATransceive(eCmd command, 
    const u8 *request, u8 requestLength, u8 txalign, 
    u8 *response, u8 *responseLength, u8 rxalign)
{
    u32 t,tmp;
    u8  i;
    u8  err;
    u8  noise=0,timerout=0,ov=0; //��־

    if( requestLength==0 || !request || !response || !responseLength )
        return ERR_PARA;
    
    if(CType==CT_B && command==TA_IBLOCK)
    {
        command = TB_IBLOCK;
    }
    
    nzWriteReg(RXSEL, 0x88);
    if(command==TB_ATTRIB || command==TB_IBLOCK)
    nzWriteReg(RXSEL, 0x88);
    nzStopCmd();
    nzClearFifo();
    nzClearFlag();

    // ����CRC��PARITYУ��
    if(CType==CT_A)
    {
        nzSetPARITY(TRUE); 
    }
    else
    {
        nzSetPARITY(FALSE); 
    }
    if(command==TA_REQA||command==TA_WUPA||command==TA_ANT)
    {
        nzSetCRC(FALSE);
    }
    else
    {
        nzSetCRC(TRUE);
    }
    
    // ���÷���/����λ���뷽ʽ
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
    // ����timeoutʱ��
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
        t = t * (4096) + 49152;

        tmp = (4096)*power(14)+49152;
        if(t>tmp)
        {
            t = tmp;   
        }
    }

    nz3801ASetTimer(t);
    
    // д���������ݣ�64Ϊ����fifo��С��len�Ĵ�С��PICC���ⲿ��֡����
    if((TA_IBLOCK==command)||(command==TB_IBLOCK))
    {
        nzWriteReg(FIFODATA, PCB);
    }
    for(i=0; i<requestLength&&i<64-3; i++)
    {
        nzWriteReg(FIFODATA, request[i]);
    }
    if(command==TB_ATTRIB)
    {
        nzStartCmd(COMMAND_TRANSMIT);
    }
    else
    {
        nzStartCmd(COMMAND_TRANSCEIVE);
    }
    nzSetBitMask(BITFRAMING,BFL_JBIT_STARTSEND);// �������ͺͽ���
    
    while(i<requestLength) // д���������
    {
        if(nzReadReg(FIFOLEVEL) < (64-2))
        {
            nzWriteReg(FIFODATA, request[i++]);
        }
    }
    
    while((nzReadReg(COMMIRQ)&BFL_JBIT_TXI)==0);  // ���ݷ������  xu.kai 20170328
    nzWriteReg(COMMIRQ, 0x01); 

    if(command==TB_ATTRIB)      
    {   
        while(nzReadReg(TCOUNTERVALHI)<=0x03);
        while(nzReadReg(TCOUNTERVALLO)<=0x0f);
        nzStartCmd(COMMAND_RECEIVE);// ��������
    }
    
REC:
    while(1)
    {
        if((nzReadReg(STATUS2)&0x07) <= 0x01)// ֹͣ��������
        {
            break;
        }
        
        if(nzReadReg(FIFOLEVEL) != 0) 
        {
            break;
        }
        
        if((nzReadReg(COMMIRQ) & BFL_JBIT_TIMERI))// ��ʱ
        {
            if(timerout==0)
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
    while((i=(nzReadReg(STATUS2))&0x07)>0x01) // �ڽ�������
    {
        while(nzReadReg(FIFOLEVEL)!=0)
        {
            response[*responseLength] = nzReadReg(FIFODATA);
            if(*responseLength >= 256-2)// 2�ֽ�CRC��ֻ��I-BLOCK�Ż�����������
                ov = 1;
            else
                *responseLength += 1; 
        }
    }

    while(nzReadReg(FIFOLEVEL)!=0) // ���պ�������
    {
        response[*responseLength] = nzReadReg(FIFODATA);
        if(*responseLength >= 256-2)      // 2�ֽ�CRC��ֻ��I-BLOCK�Ż�����������
            ov = 1;
        else
            *responseLength += 1;
    }
    
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

    // �ж��Ƿ�Ϊnoise������noise�ж�ֻ��RATS��I-BLOCK
    noise = 0;
    //secstatus = nzReadReg(CONTROL);
    err = nzReadReg(REGERROR);
    if((nzReadReg(COMMIRQ)&BFL_JBIT_TIMERI))// ��ʱ
    {
        timerout = 1;
    }
    
    nzWriteReg(RXSEL, 0x82);
    nzStartCmd(COMMAND_RECEIVE);//need?
    nzSetBitMask(CONTROL,BFL_JBIT_TSTARTNOW);  
    
    // CRC��
    if((err&BFL_JBIT_CRCERR)!=0)
    {
        if((command==TB_ATTRIB) && (*responseLength==0) && ((err&BFL_JBIT_PROTERR)==0))// Protocol error����CRC�����
        {
            noise = 1;
        }
        if(*responseLength<4)
        {
            noise = 1;
        }
    }
    if(*responseLength<2)// +CRC��С��4�ֽ�
    {
        if((err&(BFL_JBIT_CRCERR|BFL_JBIT_PARITYERR))!=0)// CRC������ż��
            noise = 1;
    }
    if((err&BFL_JBIT_COLLERR)!=0)// λ��ͻ
    {
        noise = 1;
    }
    if(*responseLength==0)// û���յ���Ч����
    {
        noise = 1;
    }
    if(((err&BFL_JBIT_PROTERR)!=0) && (*responseLength<2))// ���տ�ʼʱ�� <FDT(picc,min)
    {
        noise = 1;
    }
    if(noise)
    {
        nzWriteReg(COMMIRQ, 0x7e);
        nzWriteReg(DIVIRQ, 0x7f);
        nzClearFifo();
        goto REC;
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

/**
 * @brief IBLOCK
 *
 * @param
 * @param
 */
u8 nz3801AIBLOCK(const u8 *inf, u16 infLength, u8 *response, u16 *responseLength)
{
	u8 rec[256];
	u16 sp, l;  // sp �ѷ��ͳ���,Ҳ��������ʾ����pcb��λ��;
	u8 rlen;
	u8 r;
	u8 RB[5];
	u8 timer, re;
	u8 tmp;
	u8 brec;	// �Ƿ��ڽ�������״̬
	u8 Rerr;

	#define ACK		0
	#define NAK		0x10
	#define SendRB(type)	if(++timer>=3+Rerr) return ERR_TIMEOUT; RB[0]=0xa2|type|BlockNum; r = nz3801ATransceive(TA_RSBLOCK, RB, 1, 0, rec, &rlen, 0);
	#define SendFinish()	((PCB&BIT4)==0)

	*responseLength = 0;
	sp = 0;
	while(1)
	{
		Rerr = 0;
		// ���͹���
		if(infLength-sp+1+2>FSC)	// ʣ�����ݳ���+PCB+CRC
		{
			l = FSC-2;			// ���ֵ-CRC
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
		r = nz3801ATransceive(TA_IBLOCK, inf+sp, l-1, 0, rec, &rlen, 0);
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
		if(r==ERR_OVERLOAD)		// �������ݳ���
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
        
		// Э�����
		tmp = (rec[0]>>6)&0x3;
		if(tmp==0x00)	// I BLOCK
		{
			if((rec[0]&BIT5)!=0)	
                return ERR_PROTOCOL;
			if((rec[0]&BIT1)==0)	
                return ERR_PROTOCOL;// b2=0
			if((rec[0]&(BIT2|BIT3))!=0)	
                return ERR_PROTOCOL;// ��CID, NAD
			if((rec[0]&0x01)!=BlockNum)	
                return ERR_PROTOCOL;// ��Ų���
			if(!SendFinish())	
                return ERR_PROTOCOL;
			Rerr = 0;
			timer = 0;
			BlockNum = 1 - BlockNum;
			if(brec==0)	
            {
                infLength = 0;	// slenΪ��������λ��
			}
            brec = 1;
			mem_copy(response+*responseLength, rec+1, rlen-1);
			*responseLength += (rlen-1);
			if((rec[0]&BIT4)==0)	// ������֡
			{
				return ERR_NONE;
			}
			else
			{
				SendRB(ACK);
				timer = 0;	// ����ACK
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
			if((rec[0]&(BIT0|BIT1))==1)
				return ERR_PROTOCOL;
			if((rec[0]&BIT4)!=0)// NAK
				return ERR_PROTOCOL;
			else
			{
				if(brec)	
                   return ERR_PROTOCOL;
				if((rec[0]&0x01)!=BlockNum)
				{
					if(++re>=3) 
                       return ERR_TIMEOUT;
					goto SEND;
				}
				if(SendFinish())	
                    return ERR_PROTOCOL;
				BlockNum = 1-BlockNum;
				sp += (l-1);
				continue;// �������ͺ�������
			}
		}
		else if(tmp==0x03)// S BLOCK
		{
			u8 t;
			u8 sb[2];
            
			t = (rec[0]>>4)&0x03;
			if(t==0)	
                return ERR_PROTOCOL;// DESELECT
			else if(t!=3)	
               return ERR_PROTOCOL; // ��WTX
			t = rec[0]&0x02;
			if(t==0)	
                return ERR_PROTOCOL;
			if(rlen!=2 || rec[1]==0||rec[1]>59)	
                return ERR_PROTOCOL;	// WTX=0
			sb[0] = 0xf2;
			sb[1] = rec[1];
			r = nz3801ATransceive(TA_RSBLOCK, sb, 2, 0, rec, &rlen, 0);	// ����wtx
			goto CHECK;
		}
		else
        {
			return ERR_PROTOCOL;
		}
	}
}
#endif

