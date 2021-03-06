//-----------------------------------------------------------------------------
// nz3801-a.h
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

#ifndef __ISO_14443_A_H_
#define __ISO_14443_A_H_

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "nz3801-a_cfg.h"
#include "stdint.h"
#include "stdbool.h"
/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define ISO14443A_MAX_UID_LENGTH 	 10
#define ISO14443A_MAX_CASCADE_LEVELS 3
#define ISO14443A_CASCADE_LENGTH 	 7
#define ISO14443A_RESPONSE_CT  		 0x88

#define ISO_14443A_DBG      my_printf
/*
******************************************************************************
* GLOBAL DATATYPES
******************************************************************************
*/
/*!< 
 * struct representing an ISO14443A PICC as returned by
 * #iso14443ASelect.
 */
typedef struct  //_AProximityCard_s
{
    uint8_t uid[ISO14443A_MAX_UID_LENGTH]; /*<! UID of the PICC */
    uint8_t actlength;     /*!< actual UID length */
    uint8_t atqa[2];       /*!< content of answer to request byte */
    uint8_t sak[ISO14443A_MAX_CASCADE_LEVELS]; /*!< SAK bytes */
    uint8_t cascadeLevels; /*!< number of cascading levels */
    bool collision;   /*!< TRUE, if there was a collision which has been resolved,
                        otherwise no collision occured */
		uint8_t fsdi;          /*!< Frame size integer of the PCD. */

		uint8_t fwi;           /*!< Frame wait integer of the PICC. */
    uint8_t fsci;          /*!< Frame size integer of the PICC. */
    uint8_t sfgi;          /*!< Special frame guard time of the PICC. */
    uint8_t TA;            /*!< Data rate bits PICC->PCD. & Data rate bits PCD->PICC.*/
}iso14443AProximityCard_t;

/*! 
 * PCD command set.
 */
typedef enum
{
    ISO14443A_CMD_REQA = 0x26,  /*!< command REQA */
    ISO14443A_CMD_WUPA = 0x52, /*!< command WUPA */
    ISO14443A_CMD_SELECT_CL1 = 0x93, /*!< command SELECT cascade level 1 */
    ISO14443A_CMD_SELECT_CL2 = 0x95, /*!< command SELECT cascade level 2 */
    ISO14443A_CMD_SELECT_CL3 = 0x97, /*!< command SELECT cascade level 3 */
    ISO14443A_CMD_HLTA = 0x50, /*!< command HLTA */
    ISO14443A_CMD_PPSS = 0xd0, /*!< command PPSS */
    ISO14443A_CMD_RATS = 0xe0, /*!< command RATS */
}iso14443ACommand_t;

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
extern uint8_t iso14443AInitialize(void);
extern uint8_t iso14443ASelect(iso14443ACommand_t cmd, iso14443AProximityCard_t* card);
extern uint8_t iso14443ASendHlta(void);
extern uint8_t iso14443AEnterProtocolMode(iso14443AProximityCard_t* card, uint8_t* answer, uint8_t* length);
extern uint8_t iso14443ASendProtocolAndParameterSelection(uint8_t pps1);

extern uint8_t ApduTransceive(uint8_t *inf, uint16_t infLength, uint8_t *response, uint16_t *responseLength);

extern uint8_t Iso14443ADemonstrate(void);

void ISO_14443A_DBG_EXT(uint8_t* buff,int length,char *info_header);

#endif /* __ISO_14443_A_H_ */

