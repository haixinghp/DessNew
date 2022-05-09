//-----------------------------------------------------------------------------
// drv_iic.c
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


#include "my_iodef.h"
#include "drv_iic.h"
#include "nz3801-ab_com.h"

//extern int my_printf(const char *fmt,...);

#if (MIMETIC_IO_IIC_EN==1)
u16 iic1_speed = 1;
static void iic1delay(void)//434k
{
    volatile u16 icnt = iic1_speed;
    while(icnt--);
}

static void iic1_start(void)
{	
	IIC_EN=1;
	M_SDA_OUT_PP();
	M_SCL_OUT_PP();
	IIC1_SET_SDA_HIGH();
	iic1delay();
	IIC1_SET_SCL_HIGH();
	iic1delay();
	IIC1_SET_SDA_LOW();
	iic1delay();
	IIC1_SET_SCL_LOW();
	iic1delay();
	
}
static void iic1_stop(void)
{
	IIC1_SET_SDA_LOW();
	iic1delay();
	IIC1_SET_SCL_HIGH();
	iic1delay();
	IIC1_SET_SDA_HIGH();
	iic1delay();
	IIC_EN=0;
}
static bool iic1_check_ack(void)
{
	bool bret;
	u16  timeout;
  M_SDA_IN_UP();
//	IIC1_SET_SDA_HIGH();
	iic1delay();
	IIC1_SET_SCL_HIGH();
	iic1delay();
	bret = false;

	timeout = 20000;
	do
	{
		if(!IIC1_CHK_SDA())
		{
			bret = true;
			break;
		}
	}while (--timeout);
	IIC1_SET_SCL_LOW();
	iic1delay();
	 M_SDA_OUT_PP();
	return bret;
}
void iic1_tx_ack(type_iic_ack ack)
{
	if(ack==IIC_NACK){
  		IIC1_SET_SDA_HIGH();//NACK
    }else{
		IIC1_SET_SDA_LOW();//ACK
    }
	iic1delay();
	IIC1_SET_SCL_HIGH();
	iic1delay();
	IIC1_SET_SCL_LOW();
	iic1delay();
}
static bool iic1_tx_byte(u8 byte)
{
	u8   i;
	bool bret;
	i = 8;
	do
	{
		if (byte&0x80){
        	IIC1_SET_SDA_HIGH();
        }else{
        	IIC1_SET_SDA_LOW();
        }
		byte<<=1;
		iic1delay();
		IIC1_SET_SCL_HIGH();
		iic1delay();
		IIC1_SET_SCL_LOW();
		iic1delay();
	}while(--i);
	//check ack
	//I2C_DATA_INPUT;
	bret = iic1_check_ack();
	//I2C_DATA_OUTPUT;
	return bret;
}

static u8 iic1_rx_byte(type_iic_ack ack)
{
	u8 i;
    u8 byte;

	//I2C1_DATA_INPUT;
	IIC1_SET_SDA_HIGH();
	M_SDA_IN_UP();
	i = 8;
    byte = 0;
	do
	{
		IIC1_SET_SCL_HIGH();
		iic1delay();
		byte <<= 1;
		if(IIC1_CHK_SDA())
		{
			byte |= 0x01;
		}
		iic1delay();
		IIC1_SET_SCL_LOW();
		iic1delay();
	}while(--i);
	//ack & nack
	//I2C1_DATA_OUTPUT;
	 M_SDA_OUT_PP();
	iic1_tx_ack(ack);

	return byte;
}

void iic1Init(type_iic_baudrate baudrate)
{
    if(baudrate==IIC_50KHZ)
        iic1_speed = 30;
    else if(baudrate==IIC_100KHZ)
        iic1_speed = 14;
    else if(baudrate==IIC_200KHZ)
        iic1_speed = 6;
    else if(baudrate==IIC_400KHZ)
        iic1_speed = 1;

    IIC1_SET_SDA_HIGH();
    IIC1_SET_SCL_HIGH();
		
}

bool iic1SendByte(u8 SlvAddr, u8 WriteAddr, u8 pBuffer)
{
	bool bret;

    bret = false;
	iic1_start();
	if(iic1_tx_byte(SlvAddr))//address
	{
		if(iic1_tx_byte(WriteAddr))//command
		{
			bret = true;
			if(!iic1_tx_byte(pBuffer))//tx data
			{
				bret = false;
			}
		}
	}
	iic1_stop();

	return bret;
}

bool iic1RecvByte(u8 SlvAddr, u8 ReadAddr, u8* pBuffer)
{
	bool flag;
    flag = false;
	iic1_start();
	if (iic1_tx_byte(SlvAddr))//address
	{
		if (iic1_tx_byte(ReadAddr))//command
		{
			iic1_start();
			if(iic1_tx_byte(SlvAddr|0x01))//read cmd
			{
				*pBuffer = iic1_rx_byte(IIC_NACK);//rx data
				flag = true;
			}
		}
	}
	iic1_stop();
    
 	return flag;
}

#else

#define sIIC_FLAG_TIMEOUT         ((u32)0x1000)
#define sIIC_LONG_TIMEOUT         ((u32)(10 * sIIC_FLAG_TIMEOUT))
volatile u32 I2CTimeout;

/**
  * @brief  DeInitializes peripherals used by the I2C EEPROM driver.
  * @param  None
  * @retval None
  */
void iicLowLevel_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
      
  /* I2C Peripheral Disable */
  I2C_Cmd(I2C1, DISABLE);
 
  /* I2C DeInit */
  I2C_DeInit(I2C1);

  /*!< I2C Periph clock disable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    
  /*!< GPIO configuration */  
  /*!< Configure I2C pins: SCL & SDA*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief  Initializes peripherals used by the I2C nz3801-ab driver.
  * @param  None
  * @retval None
  */  
void iicLowLevel_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
  
    /*!< Periph clock enable 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);*/

    /*!< GPIO configuration */  
    /*!< Configure I2C pins: SCL & SDA*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void iic1Init(type_iic_baudrate baudrate)
{
    u32 speed;
    I2C_InitTypeDef  I2C_InitStructure;

    iicLowLevel_Init();
    
    /*!< I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    if(baudrate==IIC_50KHZ)
        speed = 50000;
    else if(baudrate==IIC_100KHZ)
        speed = 100000;
    else if(baudrate==IIC_200KHZ)
        speed = 200000;
    else if(baudrate==IIC_400KHZ)
        speed = 400000;
    else 
        speed = 100000;
    
    /*!< I2C configuration */
    /* NZ3801-AB_I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable; 
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = speed; 

    /* NZ3801-AB_I2C Peripheral Enable */
    I2C_Cmd(I2C1, ENABLE);
    
    /* Apply NZ3801-AB_I2C configuration after enabling it */
    I2C_Init(I2C1, &I2C_InitStructure);  

    /*允许一字节一应答模式*/  
    I2C_AcknowledgeConfig(I2C1, ENABLE); //使能IIC2应答状态
}

bool iic1SendByte(u8 SlvAddr, u8 WriteAddr, u8 pBuffer)
{
    /* Send STRAT condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    
    I2CTimeout = sIIC_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    //启动信号发出之后要等待状态寄存器SR1的位0（SB=1）,状态寄存器SR2的位1(BUSY=1)和位0(MSL=1),此时表明主模式下，起始条件已发送，总线处于忙状态；确保IIC通讯正确
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0){
            my_printf("TO1\r\n");
            return false;
        }
    } 

    /* Send slave address for write */
    I2C_Send7bitAddress(I2C1, SlvAddr, I2C_Direction_Transmitter);//7bit slave address + read/write (0write,1 read)
  
    I2CTimeout = sIIC_FLAG_TIMEOUT;
    
    /* Test on EV6 and clear it */
    //从机地址发出之后，等待 BUSY, MSL, ADDR, TXE and TRA flags标志位
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) 
    {
        if((I2CTimeout--) == 0){
            my_printf("TO2\r\n");
            return false;
        }
    }  
      
    /* Send the slave's internal address to write to */
    I2C_SendData(I2C1, WriteAddr);
  
    I2CTimeout = sIIC_FLAG_TIMEOUT;
    /* Test on EV8 and clear it */
    /* TRA, BUSY, MSL, TXE and BTF flags */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2CTimeout--) == 0){
            my_printf("TO3\r\n");
            return false;
        }
    } 

    /* Send the byte to be written */
    I2C_SendData(I2C1, pBuffer); 
    
    I2CTimeout = sIIC_FLAG_TIMEOUT;
   
    /* Test on EV8 and clear it */
    /* TRA, BUSY, MSL, TXE and BTF flags */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))    
    {
        if((I2CTimeout--) == 0){
            my_printf("TO4\r\n");
            return false;
        }
    } 
    
    /* Send STOP condition */
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    return true; //正常返回false
}

bool iic1RecvByte(u8 SlvAddr, u8 ReadAddr, u8* pBuffer)
{
    I2CTimeout = sIIC_LONG_TIMEOUT;
    
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // Added by Najoua 27/08/2008    
    {
        if((I2CTimeout--) == 0){
            my_printf("TO5\r\n");
            return false;
        }
    }
    
    I2C_GenerateSTART(I2C1, ENABLE);
  
    I2CTimeout = sIIC_FLAG_TIMEOUT;
     
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0){
            my_printf("TO6\r\n");
            return false;
        }
    }
    
    /* Send slave address for write */
    I2C_Send7bitAddress(I2C1, SlvAddr, I2C_Direction_Transmitter);

    I2CTimeout = sIIC_FLAG_TIMEOUT;
     
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) 
    {
        if((I2CTimeout--) == 0){
            my_printf("TO7\r\n");
            return false;
        }
    }
    
    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(I2C1, ENABLE);

    /* Send the slave's internal address to write to */
    I2C_SendData(I2C1, ReadAddr);  

    I2CTimeout = sIIC_FLAG_TIMEOUT;
     
    /* Test on EV8 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2CTimeout--) == 0){
            my_printf("TO8\r\n");
            return false;
        }
    }
    
    /* Send STRAT condition a second time */  
    I2C_GenerateSTART(I2C1, ENABLE);
  
    I2CTimeout = sIIC_FLAG_TIMEOUT;
    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2CTimeout--) == 0){
            my_printf("TO9\r\n");
            return false;
        }
    }
        
    /* Send slave address for read */
    I2C_Send7bitAddress(I2C1, SlvAddr, I2C_Direction_Receiver);
  
    I2CTimeout = sIIC_FLAG_TIMEOUT;
     
    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if((I2CTimeout--) == 0){
            my_printf("TO10\r\n");
            return false;
        }
    }
  
    /* While there is data to be read */
    /* Disable Acknowledgement */
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    /* Send STOP Condition */
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* Test on EV7 and clear it */
    if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  
    {      
        /* Read a byte from the slave */
        *pBuffer = I2C_ReceiveData(I2C1);       
    }   

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    return true; //正常，返回1
}

#endif

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------

