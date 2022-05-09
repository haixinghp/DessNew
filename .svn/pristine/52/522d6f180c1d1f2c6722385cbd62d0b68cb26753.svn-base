#ifndef _APP_SMARTKEY_H_
#define _APP_SMARTKEY_H_
	
#ifdef __cplusplus
	extern "C" {
#endif

#include "APP_BLE.h" 




#define PAGEID_NULL				0XFFFF
#define SMARTKEY_ID_LEN         13

typedef enum 
{
	AES_ENC,
	XRO_ENC,
}SMARTKEY_ENC_TYPE; 



uint16_t SmartkeyCheckRegId (SMARTKEY_ENC_TYPE mode, const uint8_t *CheakId)  ;
uint8_t SmartKeyWriteId (uint8_t *id )  ;
uint8_t SmartKeyDeleteId (uint8_t *id ) ;
void SmartKeyReadId (uint16_t pageid , uint8_t *id ) ;
void SmartKeyDeleteClear( void );

#ifdef __cplusplus
	}
#endif
#endif
//.end of the file.
