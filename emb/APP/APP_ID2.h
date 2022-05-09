#ifndef _APP_ID2_H_
#define _APP_ID2_H_
	
#ifdef __cplusplus
	extern "C" {
#endif

/* 标准头文件 */
#include "stdint.h"

#include "DRV_BLE.h" 

#include "LockConfig.h"
/*-------------------------------------------------宏定义-----------------------------------------------------------*/ 
#define MBEDTLS_SHA256_C 
#define MBEDTLS_SHA1_C 
#define MBEDTLS_BASE64_C 
#define TGT_A71ID2
#define SCP_MODE        NO_C_MAC_NO_C_ENC_NO_R_MAC_NO_R_ENC



/*--------------------------------------------------变量声明---------------------------------------------------------*/             


typedef struct
{
	uint8_t ID2_EN;
	uint8_t R1[16];
	uint8_t R1_ENC[16];
	uint8_t R2[16];
	uint8_t R2_ENC[16];
	uint8_t AESKEY[16];
	
}APP_ID2_DATA;



extern APP_ID2_DATA   App_Id2_Data;


void BleID2CasePro(void);
void APP_ID2Init(void);
void APP_ID2HWSleep(void);
uint8_t APP_ID2Check(void);


#ifdef __cplusplus
	}
#endif
#endif
//.end of the file.
