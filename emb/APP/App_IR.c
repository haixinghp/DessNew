/********************************************************************************************************************
 * @Company:   德施曼机电（中国）有限公司 
 * @file:      App_IR.c
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2022-01-06
 * @brief:     红外对管串口驱动文件  
 * @Description:   
 * @ChangeList:  01. 初版
*********************************************************************************************************************/
 
/*-------------------------------------------------文件包含---------------------------------------------------------*/
#include <string.h>
#include "Public.h"

#include "..\DRV\DRV_EXPORT\DRV_74HC4052.h"

#include "..\HAL\HAL_UART\HAL_UART.h"

#include "App_IR.h"

/*-------------------------------------------------宏定义-----------------------------------------------------------*/
#ifdef  IR_FUNCTION_ON                //红外功能 
 
#define DRV_IR_UTRT_SWITCH_ON       DRV_74HC0452_UartSelect(HW_74HC4052_UART_IR)               //模拟开关切换 
#define DRV_IR_UTRT_SWITCH_OFF      DRV_74HC0452_UartSelect(HW_74HC4052_UART_OFF)

#define DRV_IR_BACK_HEADH      0  //帧头
#define DRV_IR_BACK_HEADL      1  //帧头
#define DRV_IR_BACK_CMD        2  //命令字
#define DRV_IR_BACK_ADD        3  //地址
#define DRV_IR_BACK_DATA       4  //数据
#define DRV_IR_BACK_CRC        5  //校验
#define DRV_IR_BACK_ENDH       6  //帧尾
#define DRV_IR_BACK_ENDL       7  //帧尾

#define DRV_IR_ACK_TIMEOUT     70  //700MS超时,实测越500ms应答 
/*-------------------------------------------------枚举定义---------------------------------------------------------*/
// IR 执行状态
typedef enum
{
	IR_MODULE_PRO_NONE,      	 //无任务
	IR_MODULE_PRO_UART_MODE,     //唤醒
	IR_MODULE_PRO_SET,      	 //指令下发
	IR_MODULE_PRO_IO_MODE,       //指令下发
	IR_MODULE_PRO_SUCCESSFUL,
    IR_MODULE_PRO_FAIL,
}IR_WORK_STATE_E; 
 
/*-------------------------------------------------常量定义---------------------------------------------------------*/
//IR串口指令打包
const uint8_t ir_cmd[11][DRV_IR_BACK_LEN]=
{
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// NULL	
	{0xAA,0xAA,0x02,0x07,0x00,0x09,0x55,0x55},//N101=0,串口模式	
	{0xAA,0xAA,0x02,0x07,0x02,0x0b,0x55,0x55},//N102=0,IO模式	
	{0xAA,0xAA,0x02,0x03,0x01,0x06,0x55,0x55},//N103=0,短距离	
	{0xAA,0xAA,0x02,0x03,0x02,0x07,0x55,0x55},//N104=0,中距离	
	{0xAA,0xAA,0x02,0x03,0x03,0x08,0x55,0x55},//N105=0,长距离	
	{0xAA,0xAA,0x02,0x03,0x00,0x05,0x55,0x55},//N106=0,超短距	
	{0xAA,0xAA,0x01,0x30,0x00,0x31,0x55,0x55},//N107=0,查询AD
	{0xAA,0xAA,0x02,0x05,0x64,0x6B,0x55,0x55},//N108=0,设置AD最大100
	{0xAA,0xAA,0x02,0x05,0x19,0x20,0x55,0x55},//N109=0,设置AD最小25
	{0xAA,0xAA,0x01,0x05,0x00,0x06,0x55,0x55},//N110=0,读AD值,1000
	
};
/*-------------------------------------------------全局变量定义-----------------------------------------------------*/         

// 消息结构体
typedef struct
{
	uint8_t RxDataBuf[DRV_IR_BACK_LEN];  	//数据BUF
	uint8_t RxPos;       					//当前接受位置
	uint8_t RxResult;
	uint8_t SendData;    					//发送帧数据
    uint8_t times;
	uint8_t Systick;
	IR_WORK_STATE_E   State;        		//状态
    DRV_NEAR_MODE_E   Mode;
}DRV_IR_CONTROL; 

static DRV_IR_CONTROL  DrvIrControl; 
/*-------------------------------------------------局部变量定义-----------------------------------------------------*/
 
 
/*-------------------------------------------------函数声明---------------------------------------------------------*/
 
 
/*-------------------------------------------------函数定义---------------------------------------------------------*/
 
/*********************************************************************************************************************
* Function Name :  App_IR_UartHandler()
* Description   :  协议解析处理函数 
* Para          :  none
* Return        :  void
*********************************************************************************************************************/
static void App_IR_RxHandler( void )
{
	uint8_t pdata=0;
	while( UART_SUCCESS == HAL_Uart_PopByteFromQueue( IR_UART_COM, &pdata ))//队列有数据
	{
		if(DrvIrControl.RxPos==0) //无数据
		{
			if(0xAA==pdata) //包头
			{
				DrvIrControl.RxDataBuf[DrvIrControl.RxPos++]=pdata; //开始接受
			}
		}
		else if(DrvIrControl.RxPos<(DRV_IR_BACK_LEN))//未溢出
		{
			DrvIrControl.RxDataBuf[DrvIrControl.RxPos++]=pdata;     //继续接受
			if(DrvIrControl.RxPos==DRV_IR_BACK_LEN)    //一包指令长度
			{
				if(0x55==DrvIrControl.RxDataBuf[DRV_IR_BACK_LEN-1]) //仅判断帧尾，无需校验
				{
					if(DrvIrControl.RxDataBuf[DRV_IR_BACK_DATA]==DrvIrControl.SendData) //校验模组返回数据正确性
					{
						PUBLIC_PrintHex("DRV_IR_UartHandler",DrvIrControl.RxDataBuf,DRV_IR_BACK_LEN);
						//进入下个流程
						DrvIrControl.times=0;//发送次数
						DrvIrControl.State++;
					}
				}
			}
		}
		else
		{
			DrvIrControl.RxPos=0;  //数组溢出无效
		}		
    }	
	
}



/***************************************************************************************
**函数名:       DRV_IR_Systick
**功能描述:     计时处理，用于应答包时间控制
**输入参数:     
**输出参数:    
**备注:         注意需要放在10ms定时器中断中
****************************************************************************************/
void App_IR_Tim10Ms (void)
{
	if(DrvIrControl.Systick)
	{
		DrvIrControl.Systick--;
	}
}

/***************************************************************************************
**函数名:       DRV_IR_UartOn
**功能描述:     串口开启、切换模拟开关
**输入参数:     
**输出参数:    
**备注:        
****************************************************************************************/
static void DRV_IR_UartOn (void)
{
	my_printf("DRV_IR_UartOn()\n");
	//串口模拟开关切换
	DRV_IR_UTRT_SWITCH_ON;	

	//串口初始化
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
**函数名:       DRV_IR_UartOff
**功能描述:     串口关闭、模拟开关失能
**输入参数:     
**输出参数:    
**备注:        
****************************************************************************************/
static void App_IR_UartOff (void)
{
	my_printf("DRV_IR_UartOff()\n");
	DRV_IR_UTRT_SWITCH_OFF;
    HAL_Uart_DeInit( E_IR_UART );
}  
 

/***************************************************************************************
**函数名:       DRV_IR_UartTx
**功能描述:     串口发送指定命令
**输入参数:     Cmd   命令编号 ir_cmd数组下标
**输出参数:    
**备注:        
****************************************************************************************/
static void App_IR_UartTx( uint8_t Cmd )
{
    uint8_t ir_tx_data[8];

	DrvIrControl.RxPos=0;     //接受位置
	memset(&DrvIrControl.RxDataBuf,0,sizeof(DrvIrControl.RxDataBuf)); //接受BUF	
	DrvIrControl.times++;//发送次数
	
	DrvIrControl.Systick=DRV_IR_ACK_TIMEOUT; //接受等待超时设定
	memcpy(ir_tx_data,&ir_cmd[Cmd],DRV_IR_BACK_LEN); //数据指令拿命令
	DrvIrControl.SendData=ir_tx_data[DRV_IR_BACK_DATA];//记录发送数据
	
	HAL_Uart_TxMessage(IR_UART_COM,ir_tx_data,DRV_IR_BACK_LEN); 
	PUBLIC_PrintHex("DRV_IR_UartTx",ir_tx_data,DRV_IR_BACK_LEN);
}



/***************************************************************************************
**函数名:       App_IR_GetServerState
**功能描述:     红外模块设置接口服务
**输入参数:     
**输出参数:     DrvIrControl.State 当前状态
**备注:         需主循环扫描
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
				if(DrvIrControl.times>2) //重发机制
				{
					DrvIrControl.State=IR_MODULE_PRO_FAIL; //结束
					break;	
				}
				App_IR_UartTx(1); //发送串口模式指令
			}
    		break;		
    	case IR_MODULE_PRO_SET:
		    if(DrvIrControl.Systick==0)
			{
				if(DrvIrControl.times>3) //重发机制
				{
					DrvIrControl.State=IR_MODULE_PRO_FAIL; //结束
					break;	
				}
				//发送ADC设定指令
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
				if(DrvIrControl.times>3) //重发机制
				{
					DrvIrControl.State=IR_MODULE_PRO_FAIL; //结束
					break;	
				}
				//发送IO模式指令
				App_IR_UartTx(2); 
			}
    		break;		
		case IR_MODULE_PRO_SUCCESSFUL:	
			my_printf("IR_MODULE_PRO_SUCCESSFUL \n");
			App_IR_UartOff();
		    memset(&DrvIrControl,0,sizeof(DrvIrControl));
		    DrvIrControl.State=IR_MODULE_PRO_SUCCESSFUL; //结束
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
**函数名:       DRV_IR_SetSenseMode
**功能描述:     红外灵敏度的设定
**输入参数:     mode  
**输出参数:     -1= 设置失败  0= 执行中  1= 设置成功
**备注:          
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
		case 0: //触发模式
				DrvIrControl.Mode=mode;
				DrvIrControl.State=IR_MODULE_PRO_UART_MODE;
				DRV_IR_UartOn(); //开串口
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


