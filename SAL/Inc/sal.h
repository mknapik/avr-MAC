/**
 * @file sal.h
 *
 * @brief Declarations for low-level security API
 *
 * This file contains declarations for the low-level security
 * API.
 *
 * $Id: sal.h 19857 2010-01-13 09:56:44Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef SAL_H
#define SAL_H

/* === Includes =========================================================== */

#include "pal.h"
#include "sal_types.h"

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

/* Values for SR_AES_MODE */
#ifndef AES_MODE_ECB
/**
 * Defines AES mode as ECB
 */
#define AES_MODE_ECB                 (0)
#endif
#ifndef AES_MODE_CBC
/**
 * Defines AES mode as CBC
 */
#define AES_MODE_CBC                 (1)
#endif

/* === Types ============================================================== */


/* === Externals ========================================================== */


/* === Prototypes ========================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialization of SAL.
 *
 * This functions initializes the SAL.
 *
 * @ingroup apiSalApi
 */
void sal_init(void);

/**
 * @brief Reads the result of previous AES en/decryption
 *
 * This function returns the result of the previous AES operation,
 * so this function is needed in order to get the last result
 * of a series of sal_aes_wrrd() calls.
 *
 * @param[out] data     - result of previous operation
 *
 * @ingroup apiSalApi
 */
void sal_aes_read(uint8_t *data);


#if defined(DOXYGEN)
/**
 * @brief En/decrypt one AES block.
 *
 * The function returns after the AES operation is finished.
 *
 * @param[in]  data  AES block to be en/decrypted
 *
 * @ingroup apiSalApi
 */
void sal_aes_exec(uint8_t *data);

/**
 * @brief Writes data, reads previous result and does the AES en/decryption
 *
 * The function returns after the AES operation is finished.
 *
 * When sal_aes_wrrd() is called several times in sequence, from the
 * second call onwards, odata contains the result of the previous operation.
 * To obtain the last result, you must call sal_aes_read() at the end.
 * Please note that any call of sal_aes_setup() as well as putting
 * the transceiver to sleep state destroys the SRAM contents,
 * i.e. the next call of sal_aes_wrrd() yields no meaningful result.
 *
 * @param[in]  idata  AES block to be en/decrypted
 * @param[out] odata  Result of previous operation
 *                    (odata may be NULL or equal to idata)
 *
 * @ingroup apiSalApi
 */
void sal_aes_wrrd(uint8_t *idata, uint8_t *odata);

#else   /* !defined(DOXYGEN) */

#if (SAL_TYPE == ATMEGARF_SAL) || (SAL_TYPE == SW_AES_SAL) || \
    (SAL_TYPE == ATXMEGA_SAL)
void sal_aes_exec(uint8_t *data);
#elif (SAL_TYPE == AT86RF2xx)
void sal_aes_wrrd(uint8_t *idata, uint8_t *odata);
#else
#error "unknown SAL_TYPE"
#endif  /* SAL_TYPE */

#endif  /* DOXYGEN */

/**
 * @brief Re-inits key and state after a sleep or TRX reset
 *
 * This function re-initializes the AES key and the state of the
 * AES engine after TRX sleep or reset.
 * The contents of AES register AES_CON is restored,
 * the next AES operation started with sal_aes_exec()
 * will be executed correctly.
 *
 * @ingroup apiSalApi
 */
void sal_aes_restart(void);

/**
 * @brief Cleans up the SAL/AES after STB has been completed
 *
 * This function puts the radio to SLEEP if it has been in SLEEP
 * beforesal_aes_restart().
 *
 * @ingroup apiSalApi
 */
#if (SAL_TYPE == AT86RF2xx) || (SAL_TYPE == ATMEGARF_SAL)
void _sal_aes_clean_up(void);
#define sal_aes_clean_up()      _sal_aes_clean_up()
#else
#define sal_aes_clean_up()
#endif

/**
 * @brief Setup AES unit
 *
 * This function perform the following tasks as part of the setup of the
 * AES unit: key initialization, set encryption mode.
 *
 * @param[in] key AES key or NULL (NULL: use last key)
 * @param[in] enc_mode  AES_MODE_ECB or AES_MODE_CBC
 * @param[in] dir must be AES_DIR_ENCRYPT
 *
 * @return  False if some parameter was illegal, true else
 *
 * @ingroup apiSalApi
 */
bool sal_aes_setup(uint8_t *key,
                   uint8_t enc_mode,
                   uint8_t dir);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SAL_H */
/* EOF */
