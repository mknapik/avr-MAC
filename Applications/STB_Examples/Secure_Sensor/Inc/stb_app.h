/**
 * @file stb_app.h
 *
 * @brief This file contains declarations for the STB application.
 *
 * $Id: stb_app.h 17930 2009-09-15 08:00:04Z sschneid $
 *
 */
/**
 *  @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef STB_APP_H
#define STB_APP_H

/* === Includes =========================================================== */

#include "stb.h"

/* === Macros ============================================================= */

/* Length of ZigBee payload to be encrypted in octets of this application. */
#define PLD_LEN                 (4)
/* Used ZigBee security level: MIC = 8 octets, Data encryption on. */
#define SEC_LEVEL               (6)
/* Length of MIC in octets (depending on the used ZigBee security level. */
#define MIC_LEN                 (8)
/* Network key sequence number */
#define NWK_KEY_NO              (0)

/* === Types ============================================================== */


/* === Externals ========================================================== */

extern uint8_t stb_app_key[AES_BLOCKSIZE];

/* === Prototypes ========================================================= */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STB_APP_H */
/* EOF */
