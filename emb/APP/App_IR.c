/********************************************************************************************************************
 * @Company:   ��ʩ�����磨�й������޹�˾ 
 * @file:      App_IR.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2022-01-06
 * @brief:     ����Թܴ��������ļ�  
 * @Description:   
 * @ChangeList:  01. ����
*********************************************************************************************************************/
 
/*-------------------------------------------------�ļ�����---------------------------------------------------------*/
#include <string.h>
#include "Public.h"

#include "..\DRV\DRV_EXPORT\DRV_74HC4052.h"

#include "..\HAL\HAL_UART\HAL_UART.h"

#include "App_IR.h"

/*-------------------------------------------------�궨��-----------------------------------------------------------*/
#ifdef  IR_FUNCTION_ON                //���⹦�� 
 
#define DRV_IR_UTRT_SWITCH_ON       DRV_74HC0452_UartSelect(HW_74HC4052_UART_IR)               //ģ�⿪���л� 
#define DRV_IR_UTRT_SWITCH_OFF      DRV_74HC0452_UartSelect(HW_74HC4052_UART_OFF)

#define DRV_IR_BACK_HEADH      0  //֡ͷ
#define DRV_IR_BACK_HEADL      1  //֡ͷ
#define DRV_IR_BACK_CMD        2  //������
#define DRV_IR_BACK_ADD        3  //��ַ
#define DRV_IR_BACK_DATA       4  //����
#define DRV_IR_BACK_CRC        5  //У��
#define DRV_IR_BACK_ENDH       6  //֡β
#define DRV_IR_BACK_ENDL       7  //֡β

#define DRV_IR_ACK_TIMEOUT     70  //700MS��ʱ,ʵ��Խ500msӦ�� 
/*-------------------------------------------------ö�ٶ���---------------------------------------------------------*/
// IR ִ��״̬
typedef enum
{
	IR_MODULE_PRO_NONE,      	 //������
	IR_MODULE_PRO_UART_MODE,     //����
	IR_MODULE_PRO_SET,      	 //ָ���·�
	IR_MODULE_PRO_IO_MODE,       //ָ���·�
	IR_MODULE_PRO_SUCCESSFUL,
    IR_MODULE_PRO_FAIL,
}IR_WORK_STATE_E; 
 
/*-------------------------------------------------��������---------------------------------------------------------*/
//IR����ָ����
const uint8_t ir_cmd[11][DRV_IR_BACK_LEN]=
{
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// NULL	
	{0xAA,0xAA,0x02,0x07,0x00,0x09,0x55,0x55},//N101=0,����ģʽ	
	{0xAA,0xAA,0x02,0x07,0x02,0x0b,0x55,0x55},//N102=0,IOģʽ	
	{0xAA,0xAA,0x02,0x03,0x01,0x06,0x55,0x55},//N103=0,�̾���	
	{0xAA,0xAA,0x02,0x03,0x02,0x07,0x55,0x55},//N104=0,�о���	
	{0xAA,0xAA,0x02,0x03,0x03,0x08,0x55,0x55},//N105=0,������	
	{0xAA,0xAA,0x02,0x03,0x00,0x05,0x55,0x55},//N106=0,���̾�	
	{0xAA,0xAA,0x01,0x30,0x00,0x31,0x55,0x55},//N107=0,��ѯAD
	{0xAA,0xAA,0x02,0x05,0x64,0x6B,0x55,0x55},//N108=0,����AD���100
	{0xAA,0xAA,0x02,0x05,0x19,0x20,0x55,0x55},//N109=0,����AD��С25
	{0xAA,0xAA,0x01,0x05,0x00,0x06,0x55,0x55},//N110=0,��ADֵ,1000
	
};
/*-------------------------------------------------ȫ�ֱ�������-----------------------------------------------------*/         

// ��Ϣ�ṹ��
typedef struct
{
	uint8_t RxDataBuf[DRV_IR_BACK_LEN];  	//����BUF
	uint8_t RxPos;       					//��ǰ����λ��
	uint8_t RxResult;
	uint8_t SendData;    					//����֡����
    uint8_t times;
	uint8_t Systick;
	IR_WORK_STATE_E   State;        		//״̬
    DRV_NEAR_MODE_E   Mode;
}DRV_IR_CONTROL; 

static DRV_IR_CONTROL  DrvIrControl; 
/*-------------------------------------------------�ֲ���������-----------------------------------------------------*/
 
 
/*-------------------------------------------------��������---------------------------------------------------------*/
 
 
/*-------------------------------------------------��������---------------------------------------------------------*/
 
/*********************************************************************************************************************
* Function Name :  App_IR_UartHandler()
* Description   :  Э����������� 
* Para          :  none
* Return        :  void
*********************************************************************************************************************/
static void App_IR_RxHandler( void )
{
	uint8_t pdata=0;
	while( UART_SUCCESS == HAL_Uart_PopByteFromQueue( IR_UART_COM, &pdata ))//����������
	{
		if(DrvIrControl.RxPos==0) //������
		{
			if(0xAA==pdata) //��ͷ
			{
				DrvIrControl.RxDataBuf[DrvIrControl.RxPos++]=pdata; //��ʼ����
			}
		}
		else if(DrvIrControl.RxPos<(DRV_IR_BACK_LEN))//δ���
		{
			DrvIrControl.RxDataBuf[DrvIrControl.RxPos++]=pdata;     //��������
			if(DrvIrControl.RxPos==DRV_IR_BACK_LEN)    //һ��ָ���
			{
				if(0x55==DrvIrControl.RxDataBuf[DRV_IR_BACK_LEN-1]) //���ж�֡β������У��
				{
					if(DrvIrControl.RxDataBuf[DRV_IR_BACK_DATA]==DrvIrControl.SendData) //У��ģ�鷵��������ȷ��
					{
						PUBLIC_PrintHex("DRV_IR_UartHandler",DrvIrControl.RxDataBuf,DRV_IR_BACK_LEN);
						//�����¸�����
						DrvIrControl.times=0;//���ʹ���
						DrvIrControl.State++;
					}
				}
			}
		}
		else
		{
			DrvIrControl.RxPos=0;  //���������Ч
		}		
    }	
	
}



/***************************************************************************************
**������:       DRV_IR_Systick
**��������:     ��ʱ��������Ӧ���ʱ�����
**�������:     
**�������:    
**��ע:         ע����Ҫ����10ms��ʱ���ж���
****************************************************************************************/
void App_IR_Tim10Ms (void)
{
	if(DrvIrControl.Systick)
	{
		DrvIrControl.Systick--;
	}
}

/***************************************************************************************
**������:       DRV_IR_UartOn
**��������:     ���ڿ������л�ģ�⿪��
**�������:     
**�������:    
**��ע:        
****************************************************************************************/
static void DRV_IR_UartOn (void)
{
	my_printf("DRV_IR_UartOn()\n");
	//����ģ�⿪���л�
	DRV_IR_UTRT_SWITCH_ON;	

	//���ڳ�ʼ��
	UartCfg_S uartCfg={0};
	uartCfg.BaudRate = UART_BAUD_RATE_9600;
	uartCfg.DataBit = DATA_8_BIT;
	uartCfg.StopBit = STOP_1_BIT;
	uartCfg.ParityType = PARITY_NONE;
	uartCfg.RxInerruptEn = INT_ENABLE;
	uartCfg.TxInerruptEn = INT_DISENABLE;
	HAL_Uart_ConfigInit( E_IR_UART, uartCfg );
	
}  


/***************************************************************************************
**������:       DRV_IR_UartOff
**��������:     ���ڹرա�ģ�⿪��ʧ��
**�������:     
**�������:    
**��ע:        
****************************************************************************************/
static void App_IR_UartOff (void)
{
	my_printf("DRV_IR_UartOff()\n");
	DRV_IR_UTRT_SWITCH_OFF;
    HAL_Uart_DeInit( E_IR_UART );
}  
 

/***************************************************************************************
**������:       DRV_IR_UartTx
**��������:     ���ڷ���ָ������
**�������:     Cmd   ������ ir_cmd�����±�
**�������:    
**��ע:        
****************************************************************************************/
static void App_IR_UartTx( uint8_t Cmd )
{
    uint8_t ir_tx_data[8];

	DrvIrControl.RxPos=0;     //����λ��
	memset(&DrvIrControl.RxDataBuf,0,sizeof(DrvIrControl.RxDataBuf)); //����BUF	
	DrvIrControl.times++;//���ʹ���
	
	DrvIrControl.Systick=DRV_IR_ACK_TIMEOUT; //���ܵȴ���ʱ�趨
	memcpy(ir_tx_data,&ir_cmd[Cmd],DRV_IR_BACK_LEN); //����ָ��������
	DrvIrControl.SendData=ir_tx_data[DRV_IR_BACK_DATA];//��¼��������
	
	HAL_Uart_TxMessage(IR_UART_COM,ir_tx_data,DRV_IR_BACK_LEN); 
	PUBLIC_PrintHex("DRV_IR_UartTx",ir_tx_data,DRV_IR_BACK_LEN);
}



/***************************************************************************************
**������:       App_IR_GetServerState
**��������:     ����ģ�����ýӿڷ���
**�������:     
**�������:     DrvIrControl.State ��ǰ״̬
**��ע:         ����ѭ��ɨ��
****************************************************************************************/
static uint8_t App_IR_GetServerState(void)
{
	if( E_IR_UART != HAL_Uart_GetCurDeviceType( IR_UART_COM ))
	{
		return 0;
	}
	
	App_IR_RxHandler();
	
	switch (DrvIrControl.State) 
    {
    	case IR_MODULE_PRO_UART_MODE:  		
		    if(DrvIrControl.Systick==0)
			{
				if(DrvIrControl.times>2) //�ط�����
				{
					DrvIrControl.State=IR_MODULE_PRO_FAIL; //����
					break;	
				}
				App_IR_UartTx(1); //���ʹ���ģʽָ��
			}
    		break;		
    	case IR_MODULE_PRO_SET:
		    if(DrvIrControl.Systick==0)
			{
				if(DrvIrControl.times>3) //�ط�����
				{
					DrvIrControl.State=IR_MODULE_PRO_FAIL; //����
					break;	
				}
				//����ADC�趨ָ��
				if(DrvIrControl.Mode==IR_FAR)
				{
					App_IR_UartTx(9); 
				}
				else if(DrvIrControl.Mode==IR_NEAR)
				{
					App_IR_UartTx(8); 
				}
				else if(DrvIrControl.Mode==IR_OFF)
				{
					App_IR_UartTx(6); 
				}
			}
    		break;		
    	case IR_MODULE_PRO_IO_MODE:
		    if(DrvIrControl.Systick==0)
			{
				if(DrvIrControl.times>3) //�ط�����
				{
					DrvIrControl.State=IR_MODULE_PRO_FAIL; //����
					break;	
				}
				//����IOģʽָ��
				App_IR_UartTx(2); 
			}
    		break;		
		case IR_MODULE_PRO_SUCCESSFUL:	
			my_printf("IR_MODULE_PRO_SUCCESSFUL \n");
			App_IR_UartOff();
		    memset(&DrvIrControl,0,sizeof(DrvIrControl));
		    DrvIrControl.State=IR_MODULE_PRO_SUCCESSFUL; //����
			break;	
		case IR_MODULE_PRO_FAIL:	
			App_IR_UartOff();
		    memset(&DrvIrControl,0,sizeof(DrvIrControl));
		    DrvIrControl.State=IR_MODULE_PRO_NONE;
			break;
	
    	default:
    		break;
    }	
	
	return  DrvIrControl.State;
}

 
/***************************************************************************************
**������:       DRV_IR_SetSenseMode
**��������:     ���������ȵ��趨
**�������:     mode  
**�������:     -1= ����ʧ��  0= ִ����  1= ���óɹ�
**��ע:          
****************************************************************************************/
int8_t App_IR_SetSenseMode(DRV_NEAR_MODE_E mode, uint8_t *pfirtflg)
{
	uint8_t ret = 0;
	static uint8_t step;
	if( ! *pfirtflg )
	{
	    *pfirtflg = 1;	
		step = 0;
	}

	switch( step )
	{
		case 0: //����ģʽ
				DrvIrControl.Mode=mode;
				DrvIrControl.State=IR_MODULE_PRO_UART_MODE;
				DRV_IR_UartOn(); //������
		        step = 1;
		break;
	    case 1:
			    ret = App_IR_GetServerState();
				if( ret == IR_MODULE_PRO_SUCCESSFUL )
				{
					step = 0;
					return 1;
				}
				else if( ret == IR_MODULE_PRO_FAIL )
				{
					step = 0;
					return -1;
				}
		break;
		
		default:break;
		
	}
	return 0;
}



#endif


