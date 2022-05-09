/**************************************************************************** 
* Copyright (C), 2008-2021,德施曼机电（中国）有限公司 
* 文件名: APP_Update.c 
* 作者：范淑毓
* 版本：V01
* 时间：20220105
* 内容简述：业务通用升级功能(小嘀协议接收，I2C推送模组)
****************************************************************************/
/* 标准头文件 */
#include "app_fifo.h"

/* 内部头文件 */
#include "APP_Update.h"
#include "APP_Screen.h"

/* 外部头文件 */
#include "LockConfig.h"
#include "DRV_BLE.h" 
#include "Public.h"
#include "..\DRV\DRV_IIC\DRV_IIC.h"
#ifdef BLE_SPEED_TEST
#include "App_Key.h" 
#include "nrf_gpio.h"   
#include "..\HAL\HAL_RTC\HAL_RTC.h"
#endif

/* 串口队列，用于通用升级包的数据获取 */
static app_fifo_t s_moduleUpDateDataFifo;
static MODULE_UPDATE_T g_stModuleUpdata;

static uint32_t s_u32AppUpdateCnt = 0;              // 交互最长时间


/***************************************************************************************
**函数名:       ble_module_update_figo_init
**功能描述:      串口队列初始化
**输入参数:     
**输出参数:     uint32_t 初始化结果
**备注:         
****************************************************************************************/
static uint32_t ble_module_update_figo_init(void)
{
	static uint8_t rx_buf[APP_UPDATE_PACK_MAX]={0};  // fifo buff     
	// 初始化队列
	uint32_t err_code = app_fifo_init(&s_moduleUpDateDataFifo, rx_buf, sizeof(rx_buf));
    if ( err_code != FIFO_SUCCESS )
    {
        return err_code;// Propagate error code.
    }
    return FIFO_SUCCESS;
}

/***************************************************************************************
**函数名:       ble_module_update_data_rec
**功能描述:      升级数据接收分析和处理
**输入参数:     
**输出参数:     bool  初始化结果 , false - 非升级数据  ture - 升级处理
**备注:         
****************************************************************************************/
static bool ble_module_update_data_rec(uint8_t *data , uint8_t len )
{
    if(EM_APP_UPDATE_NONE == g_stModuleUpdata.emUpdateType)
    {
	    return 0;
    }
    
	if(g_stModuleUpdata.Enabled == 0)
	{
		if((*data==0x55)&&(*(data+1)==0xAA)&&(*(data+2)==0x01)&&(*(data+3)==0x0B))
		{
			ble_module_update_figo_init();
			memset(&g_stModuleUpdata,0,sizeof(g_stModuleUpdata));
			g_stModuleUpdata.FileSize=(*(data+6))<<24;
			g_stModuleUpdata.FileSize|=(*(data+7))<<16;
			g_stModuleUpdata.FileSize|=(*(data+8))<<8;
			g_stModuleUpdata.FileSize|=(*(data+9))<<0;
			g_stModuleUpdata.FileChack=(*(data+10))<<24;
			g_stModuleUpdata.FileChack|=(*(data+11))<<16;
			g_stModuleUpdata.FileChack|=(*(data+12))<<8;
			g_stModuleUpdata.FileChack|=(*(data+13))<<0;
			g_stModuleUpdata.SinglePackageSize=(*(data+14));
			g_stModuleUpdata.stage=(*(data+15))<<8;
			g_stModuleUpdata.stage|=(*(data+16))<<0;
			my_printf("Update Data - FileSize =%ld",g_stModuleUpdata.FileSize);
			my_printf("Update Data - FileChack =%ld \n",g_stModuleUpdata.FileChack);
			my_printf("Update Data - SinglePackageSize =%d",g_stModuleUpdata.SinglePackageSize);
			my_printf("Update Data - stage =%d \n",g_stModuleUpdata.stage);
            
            uint8_t tx[10]={0x55,0xAA,0x09,0x0B,0x00,0x03,0x00,0x00,0x00,0x00};

            /* 固件内容包最大长度或阶段包的值过大返回失败 */
            if(g_stModuleUpdata.stage > APP_UPDATE_PACK_CACHE_CNT || g_stModuleUpdata.SinglePackageSize > APP_UPDATE_PACK_CACHE_LEN)
            {
			    memset(&g_stModuleUpdata,0,sizeof(g_stModuleUpdata));
                if(g_stModuleUpdata.stage > APP_UPDATE_PACK_CACHE_CNT)
                {
                    tx[6] = 0x02;
                    tx[7] = (uint8_t)(APP_UPDATE_PACK_CACHE_CNT>>8);
                    tx[8] = (uint8_t)(APP_UPDATE_PACK_CACHE_CNT);
                }
                else
                {
                    tx[6] = 0x03;
                    tx[7] = (uint8_t)(APP_UPDATE_PACK_CACHE_LEN>>8);
                    tx[8] = (uint8_t)(APP_UPDATE_PACK_CACHE_LEN);
                }
            }
            else
            {
			    g_stModuleUpdata.Enabled=1;
                tx[6] = 0x00;
                uint8_t au8Param[15] = {0};
                (void)APP_SCREEN_Update(EM_SCREEN_APP_FLOW5_UPDATE, data, len);
            }
            
            for(uint8_t i = 0; i < 9; i++)
            {
                tx[9] += tx[i];
            }
		    ble_tx_event_handle(tx, 10);
			return 1;
		}
	}
	else
	{
		if((*data == 0x55)&&(*(data + 1) == 0xAA))
		{
		    /* 语音芯片只透传数据域 */
            if(EM_APP_UPDATE_TYPE_VOICE == g_stModuleUpdata.emUpdateType)
            {
    			for(uint16_t i=10; i < (len-1); i++)
    			{
    				if( app_fifo_put(&s_moduleUpDateDataFifo,*(data+i)) != FIFO_SUCCESS) // 数据入队
    				{
    					my_printf("Update Data - UpdataFifo Slop Over ------------------\n");
    					return 0;
    				}
    				g_stModuleUpdata.RxChack += *(data+i); // 计算校验
    			}
            }
            else
            {
    			for(uint16_t i=0; i < (len); i++)
    			{
    				if( app_fifo_put(&s_moduleUpDateDataFifo,*(data+i)) != FIFO_SUCCESS) // 数据入队
    				{
    					my_printf("Update Data - UpdataFifo Slop Over ------------------\n");
    					return 0;
    				}
    				g_stModuleUpdata.RxChack += *(data+i); // 计算校验
    			}
            }

			return 1;
		}
	}
	return 0;
}


/***************************************************************************************
**函数名:       app_update_data_push
**功能描述:      升级数据推送下层应用
**输入参数:     
**输出参数:     bool 是否结束
**备注:         
****************************************************************************************/
static bool app_update_data_push()
{
    bool ret = false;
    switch (g_stModuleUpdata.emUpdateType)
    {
        case EM_APP_UPDATE_TYPE_VOICE:
			if(g_stModuleUpdata.BinPos==512) // 发送512字节+ 校验
			{
				g_stModuleUpdata.Package++;
                
    			// 这里自动调速，等蓝牙
    			if((s_moduleUpDateDataFifo.write_pos - (s_moduleUpDateDataFifo.read_pos)  )>=512)
    			{
    				g_stModuleUpdata.I2cSpeed=1;
    				my_printf("Speed Fast  Package= %ld \n",g_stModuleUpdata.Package);
    			}
    			else
    			{
    				g_stModuleUpdata.I2cSpeed=0;
    			}
    			
    			
    			DRV_I2CWriteSoft(g_stModuleUpdata.Bin,512);
    			g_stModuleUpdata.BinPos=0;
    			g_stModuleUpdata.offset+=512;
    			if(g_stModuleUpdata.offset>=g_stModuleUpdata.FileSize)
    			{
    				DRV_I2CStopUpdateI2C();
    				my_printf("voice ota finish  %ld \n",g_stModuleUpdata.FileSize);
    				my_printf("voice ota FileChack %ld \n",g_stModuleUpdata.FileChack);
    				my_printf("voice ota offset %ld \n",g_stModuleUpdata.offset);
    				my_printf("voice ota RxChack %ld \n",g_stModuleUpdata.RxChack);
                    ret = true;
    			}
			}					
            break;
        case EM_APP_UPDATE_TYPE_SCREEN:
            APP_SCREEN_Update(EM_SCREEN_APP_FLOW6_UPDATE_DATA, g_stModuleUpdata.Bin, g_stModuleUpdata.BinPos);
            break;
        case EM_APP_UPDATE_TYPE_EMOJI:
            break;
        default:
            break;
    }

    return ret;
}


/****************************************************************************** 
* 函数名：APP_UpdateGetWorkCnt
* 功 能：获取当前计时时间
* 输 入：void 
* 输 出：void
* 返 回：uint32_t  返回计数值
*/ 
uint32_t APP_UpdateGetWorkCnt(void)
{
    return s_u32AppUpdateCnt;
}

/****************************************************************************** 
* 函数名：APP_UpdateWorkCntCountDown
* 功 能：提供给外部定时器(每10毫秒调用一次)
* 输 入：void 
* 输 出：void
* 返 回：uint8_t 返回计数值
*/ 
uint32_t APP_UpdateWorkCntCountDown(void)
{
    return (s_u32AppUpdateCnt > 0) ? (s_u32AppUpdateCnt--) : 0;
}

/****************************************************************************** 
* 函数名：APP_UpdateWorkCntReset
* 功 能：重置计时，单位是 10ms
* 输 入：void 
* 输 出：void
* 返 回：void
*/ 
void APP_UpdateWorkCntReset(void)
{
    /* 1个计数单位是10ms  */
    s_u32AppUpdateCnt = 200;
    return;
}


/***************************************************************************************
**函数名:       APP_UpdateDataWrite
**功能描述:      升级数据推送至下级业务
**输入参数:     
**输出参数:     
**备注:         
****************************************************************************************/
void APP_UpdateDataWrite(void)
{
	uint8_t byte=0;
	while(g_stModuleUpdata.Enabled && EM_APP_UPDATE_NONE != g_stModuleUpdata.emUpdateType)
	{
		switch (g_stModuleUpdata.Step)
		{
			case 0:	
            {
                if(0 == APP_UpdateGetWorkCnt())
                {
                    g_stModuleUpdata.Step = 2;
                    break;
                }
                if(EM_APP_UPDATE_TYPE_VOICE != g_stModuleUpdata.emUpdateType)
                {
                    g_stModuleUpdata.Step++;
                    break;
                }
				if(EM_APP_UPDATE_TYPE_VOICE == g_stModuleUpdata.emUpdateType && app_fifo_get(&s_moduleUpDateDataFifo, &byte) == FIFO_SUCCESS)
				{
				    APP_UpdateWorkCntReset();
					g_stModuleUpdata.Bin[g_stModuleUpdata.BinPos++]=byte;
					if(g_stModuleUpdata.BinPos==500) // 等第一包来了再发握手，手机准备数据时间太长
					{
						for(uint8_t i=0;i<3;i++)
						{
							DRV_IICVoiceHandShake();// 握手
							if(DRV_IICVoicePairing())// 配对
							{
								break;
							}
							else if(i==2)
							{
								uint8_t tx[8]={0x55,0xAA,0X01,0X0D,0X00,0X01,0X05,0X00};
								ble_tx_event_handle(tx, 8);
								g_stModuleUpdata.Enabled=0;
								return;	
							}
						}
						g_stModuleUpdata.Step++;
					}
				}
				break;
            }
			case 1:
            {
                if(0 == APP_UpdateGetWorkCnt())
                {
                    g_stModuleUpdata.Step = 2;
                    break;
                }
				if(app_fifo_get(&s_moduleUpDateDataFifo, &byte) == FIFO_SUCCESS)
				{
				    APP_UpdateWorkCntReset();
					g_stModuleUpdata.Bin[g_stModuleUpdata.BinPos++]=byte;

                    if(app_update_data_push())
					{
						g_stModuleUpdata.Step++;
					}			
				}
				break;
            }
			case 2: 
			{
				uint8_t tx[8]={0x55,0xAA,0X01,0X0D,0X00,0X01,0X00,0X00};
				ble_tx_event_handle(tx, 8);
				memset(&g_stModuleUpdata,0,sizeof(g_stModuleUpdata));
				break;
			}
			default:
				break;
		}
	}

    return;
}

/***************************************************************************************
**函数名:       APP_UpdateSetType
**功能描述:      设置升级类型
**输入参数:     APP_UPDATE_TYPE_E _emUpdateType
**输出参数:     bool 
**备注:         
****************************************************************************************/
bool APP_UpdateSetType(APP_UPDATE_TYPE_E _emUpdateType)
{
    if(0 == g_stModuleUpdata.Enabled && EM_APP_UPDATE_NONE == _emUpdateType)
    {
        g_stModuleUpdata.emUpdateType = _emUpdateType;
        return true;
    }

    return false;
}


/***************************************************************************************
**函数名:       APP_UpdateDataHandler
**功能描述:      升级数据接收回调接口，接收升级数据
**输入参数:     
**输出参数:     uint8_t  是否为升级数据
                                返回1，蓝牙协议栈不再继续接收数据处理协议
**备注:         
****************************************************************************************/
uint8_t APP_UpdateDataHandler(uint8_t* _pu8BleData, uint8_t _u8DataLen)
{
    if(ble_module_update_data_rec(_pu8BleData, _u8DataLen))
    {
        return 1;
    }

	#ifdef BLE_SPEED_TEST
	static uint32_t temp_time=0;
	static uint32_t BleDataLen=0;
	if(Rtc_Real_Time.timestamp- temp_time  >=3) //每3秒 打印一次速率
	{
		temp_time=Rtc_Real_Time.timestamp;
		my_printf("SPEED = %d KB/S \n",BleDataLen/3/1024);
		BleDataLen=0;//长度清零
	}
	BleDataLen+=_u8DataLen; //长度累加
	nrf_gpio_pin_toggle(GPIO_PIN_NO_17);	
	if(DRV_GpioRead( KEY_OPEN_GPIO_PIN ) )
	{
		return 1;
	}
	#endif

    return 0;
}


