/*******************************************************************************
 * @Copyright (C), 2008-2099, haiqing. Co., Ltd.
 * @FileName:               h_TypeDef.h
 * @Author:                 Haiqing.Jin
 * @Version:                v1.0
 * @Date:                   2010.09.08
 * @Description:
 * 
 * @History:
 * @    <author>            <time>          <version >          <desc>
 * @    Haiqing             11/07/15        v1.0                build this moudle
 ******************************************************************************/
#ifndef TYPEDEF_H
#define TYPEDEF_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef TYPEDEF_GLOBALS
#define TYPEDEF_EXT  
#else
#define TYPEDEF_EXT extern
#endif

/*******************************************************************************
 * @Author:                 Haiqing.Jin
 * @Descriptions:           DATA TYPES
 *                          (Compiler Specific)
 *******************************************************************************/


typedef unsigned char  BOOLEAN;                 /* ��������                                 */
typedef unsigned char  INT8U;                   /* �޷���8λ���ͱ���                        */
typedef signed   char  INT8S;                   /* �з���8λ���ͱ���                        */
typedef unsigned short INT16U;                  /* �޷���16λ���ͱ���                       */
typedef signed   short INT16S;                  /* �з���16λ���ͱ���                       */
typedef unsigned int   INT32U;                  /* �޷���32λ���ͱ���                       */
typedef signed   int   INT32S;                  /* �з���32λ���ͱ���                       */
typedef unsigned long long  INT64U;            /* �޷���32λ���ͱ���                       */
typedef signed   long long  INT64S;             /* �з���32λ���ͱ���                       */
typedef float          FP32;                    /* �����ȸ�������32λ���ȣ�                 */
typedef double         FP64;                    /* ˫���ȸ�������64λ���ȣ�                 */

typedef INT32U         OS_STK;                  /* ��ջ��32λ���                           */

/*******************************************************************************
 * @Author:                 Haiqing.Jin E:\work\file\V8\T11_H_nrf52_code\LOCK\ble_peripheral\ble_app_lock\bsp\Card\RC522\h_TypeDef.h
 * @Descriptions:           �����Ǽ���UC/OS V1.XX���������ͣ���uC/OS-IIû��ʹ��
 *                          
 *******************************************************************************/
#define BYTE           INT8S
#define UBYTE          INT8U
#define WORD           INT16S
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U

/*******************************************************************************
 * @Author:                 Haiqing.Jin
 * @Descriptions:           �����ָ��
 *                          
 *******************************************************************************/
#define M_NOP()         __nop()

#ifdef  __cplusplus
}
#endif
#endif
/*******************************************************************************
*   end of file
*******************************************************************************/
