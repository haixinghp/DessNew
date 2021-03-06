/*********************************************************************************************************************
 * @file:      App_IR.h
 * @author:    gushengchi
 * @version:   V01.00
 * @date:      2022-01-13
 * @brief:     红外对管串口驱动文件
**********************************************************************************************************************/
#ifndef _APP_IR_H_
#define _APP_IR_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*--------------------------------------------------文件包含---------------------------------------------------------*/
#include <stdint.h>
#include "LockConfig.h" 
/*--------------------------------------------------宏定义-----------------------------------------------------------*/
#ifdef  IR_FUNCTION_ON        		//红外功能 
 
#define DRV_IR_BACK_LEN        8    //包长    
/*--------------------------------------------------枚举声明---------------------------------------------------------*/
// 接近感应模式枚举
typedef enum
{       
	IR_FAR = SENSE_HIGH_GRADE, 
	IR_NEAR= SENSE_LOW_GRADE,   
	IR_OFF = SENSE_OFF_GRADE,
} DRV_NEAR_MODE_E;


/*--------------------------------------------------常量声明---------------------------------------------------------*/


/*--------------------------------------------------变量声明---------------------------------------------------------*/             

 
/*--------------------------------------------------函数声明---------------------------------------------------------*/
void App_IR_Tim10Ms( void );
int8_t App_IR_SetSenseMode( DRV_NEAR_MODE_E mode, uint8_t *pfirtflg );
 
#endif

#endif

 
/*-------------------------------------------------THE FILE END-----------------------------------------------------*/



