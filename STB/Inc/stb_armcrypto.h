/**
 * @file stb_armcrypto.h
 *
 * @brief Declarations for security tool box required for ARM crypto engine
 *
 * This file contains declarations for the high-level security tool box API
 * required for the ARM crypto engine. This is necessary, since the STB
 * implementation utilizing the ARM crypto engine does not use the SAL as
 * security base layer.
 *
 * $Id: stb_armcrypto.h 20235 2010-02-08 16:16:59Z sschneid $
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
#ifndef STB_ARMCRYPTO_H
#define STB_ARMCRYPTO_H

#if defined(STB_ARMCRYPTO) || defined(DOXYGEN)

/* === Includes =========================================================== */

#include "pal.h"

/* === Macros ============================================================= */

/**
 * Size of AES blocks
 */
#define AES_BLOCKSIZE               (16)

/**
 * Size of AES key
 */
#define AES_KEYSIZE                 (16)

/* Values for SR_AES_DIR */
#ifndef AES_DIR_ENCRYPT
/**
 * Defines AES direction as encryption
 */
#define AES_DIR_ENCRYPT              (0)
#endif
#ifndef AES_DIR_DECRYPT
/**
 * Defines AES direction as decryption
 */
#define AES_DIR_DECRYPT              (1)
#endif

/* === Types ============================================================== */


/* === Externals ========================================================== */


/* === Prototypes ========================================================= */

#ifdef __cplusplus
extern "C" {
#endif

void sal_aes_restart(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* #ifdef STB_ARMCRYPTO */

#endif /* STB_ARMCRYPTO_H */
/* EOF */
