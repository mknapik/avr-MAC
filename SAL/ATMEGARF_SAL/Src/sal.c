/**
 * @file sal.c
 *
 * @brief Low-level crypto API for an AES unit implemented in AT86RFA1
 *
 * This file implements the low-level crypto API based on an AES unit
 * implemented in an Atmel's radio transceiver AT86RFA1.
 *
 * $Id: sal.c 22413 2010-07-05 11:49:21Z sschneid $
 *
 */
/**
 * @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================ */

#include "sal_types.h"
#if (SAL_TYPE == ATMEGARF_SAL)
#include <stdio.h>
#include <string.h>
#include "tal.h"
#include "pal.h"
#include "sal.h"

/* === Macros ============================================================== */

/* values for SR_AES_DIR */
#define AES_DIR_VOID      (AES_DIR_ENCRYPT + AES_DIR_DECRYPT + 1)
                                /* Must be different from both summands */

/* value for SR_AES_CON */
#define AES_START         (1)

#define _SR_MASK(addr, mask, pos, val)     (((uint8_t)val << pos) & mask)
#define SR_MASK(SR, val)                   _SR_MASK(SR, val)

/* === Types =============================================================== */


/* === Data ============================================================= */

/* True if decryption key is actual and was computed. */
static bool dec_initialized = false;
/* Encryption mode with flags. */
static uint8_t mode_byte;
/* Last value of "dir" parameter in sal_aes_setup(). */
static uint8_t last_dir = AES_DIR_VOID;
/* Actual encryption key. */
static uint8_t enc_key[AES_KEYSIZE];
/* Actual decryption key (valid if last_dir == AES_DIR_DECRYPT). */
static uint8_t dec_key[AES_KEYSIZE];
/* State of trx before AES use; use to re-store trx state. */
static tal_trx_status_t prev_trx_status;

extern tal_trx_status_t tal_trx_status;

/* === Implementation ====================================================== */

/**
 * @brief Initialization of SAL.
 *
 * This functions initializes the SAL.
 */
void sal_init(void)
{
}



/**
 * @brief Setup AES unit
 *
 * This function perform the following tasks as part of the setup of the
 * AES unit: key initialization, set encryption direction and encryption mode.
 *
 * In general, the contents of SRAM buffer is destroyed. When using
 * sal_aes_wrrd(), sal_aes_read() needs to be called in order to get the result
 * of the last AES operation before you may call sal_aes_setup() again.
 *
 * @param[in] key AES key or NULL (NULL: use last key)
 * @param[in] enc_mode  AES_MODE_ECB or AES_MODE_CBC
 * @param[in] dir AES_DIR_ENCRYPT or AES_DIR_DECRYPT
 *
 * @return  False if some parameter was illegal, true else
 */
bool sal_aes_setup(uint8_t *key,
                   uint8_t enc_mode,
                   uint8_t dir)
{
    uint8_t i;

    if (key != NULL)
    {

        /* Setup key. */
        dec_initialized = false;

        last_dir = AES_DIR_VOID;

        /* Save key for later use after decryption or sleep. */
        memcpy(enc_key, key, AES_KEYSIZE);

        /* fill in key */
        for(i = 0; i < AES_BLOCKSIZE; ++i)
        {
            pal_trx_reg_write(RG_AES_KEY, key[i]);
        }
    }

    /* Set encryption direction. */
    switch(dir)
    {
        case AES_DIR_ENCRYPT:
            if (last_dir == AES_DIR_DECRYPT)
            {
                /*
                 * If the last operation was decryption, the encryption
                 * key must be stored in enc_key, so re-initialize it.
                 */
                for(i = 0; i < AES_BLOCKSIZE; ++i)
                {
                    pal_trx_reg_write(RG_AES_KEY, enc_key[i]);
                }
            }
            break;

        case AES_DIR_DECRYPT:
            if (last_dir != AES_DIR_DECRYPT)
            {
                if (!dec_initialized)
                {
                    uint8_t dummy[AES_BLOCKSIZE];

                    /* Compute decryption key and initialize unit with it. */

                    /* Dummy ECB encryption. */
                    mode_byte = SR_MASK(SR_AES_MODE, AES_MODE_ECB) |
                        SR_MASK(SR_AES_DIR, AES_DIR_ENCRYPT);
                    pal_trx_reg_write(RG_AES_CTRL, mode_byte);
                    sal_aes_exec(dummy);

                    /* Read last round key. */
                    for(i = 0; i < AES_BLOCKSIZE; ++i)
                    {
                        dec_key[i] = pal_trx_reg_read(RG_AES_KEY);
                    }

                    dec_initialized = true;
                }

                /* Initialize the key. */
                for(i = 0; i < AES_BLOCKSIZE; ++i)
                {
                    pal_trx_reg_write(RG_AES_KEY, dec_key[i]);
                }

                break;
            }

        default:
            return false;
    }

    last_dir = dir;

    /* Set encryption mode. */
    switch(enc_mode)
    {
        case AES_MODE_ECB:
        case AES_MODE_CBC:
            mode_byte =
                SR_MASK(SR_AES_MODE, enc_mode) | SR_MASK(SR_AES_DIR, dir);
            break;

        default:
            return (false);
    }

    /* set mode and direction */

    pal_trx_reg_write(RG_AES_CTRL, mode_byte);

    return (true);
}



/**
 * @brief Re-inits key and state after a sleep or TRX reset
 *
 * This function re-initializes the AES key and the state of the
 * AES engine after TRX sleep or reset.
 * The contents of AES register AES_CON is restored,
 * the next AES operation started with sal_aes_exec()
 * will be executed correctly.
 */
void sal_aes_restart(void)
{
    uint8_t i;
    uint8_t *keyp;

    /* Ensure that radio is awake */
    prev_trx_status = tal_trx_status;
    if (tal_trx_status == TRX_SLEEP)
    {
        tal_trx_wakeup();
    }

    if (last_dir == AES_DIR_ENCRYPT)
    {
        keyp = enc_key;
    }
    else
    {
        keyp = dec_key;
    }

    /* fill in key */
    for(i = 0; i < AES_BLOCKSIZE; ++i)
    {
        pal_trx_reg_write(RG_AES_KEY, *keyp++);
    }

    pal_trx_reg_write(RG_AES_CTRL, mode_byte);
}


/**
 * @brief Cleans up the SAL/AES after STB has been finished
 */
void _sal_aes_clean_up(void)
{
    /* Set radio to SLEEP, if it has been in SLEEP before sal_aes_restart() */
    if (prev_trx_status == TRX_SLEEP)
    {
        tal_trx_sleep(SLEEP_MODE_1);
    }
}


/**
 * @brief En/decrypt one AES block.
 *
 * The function returns after the AES operation is finished.
 *
 * @param[in]  data  AES block to be en/decrypted
 */
void sal_aes_exec(uint8_t *data)
{
    uint8_t i;

    for(i = 0; i < AES_BLOCKSIZE; ++i)
    {
        pal_trx_reg_write(RG_AES_STATE, *data++);
    }

    pal_trx_reg_write(RG_AES_CTRL,
                      mode_byte | SR_MASK(SR_AES_REQUEST, AES_START));

    /* Wait for the operation to finish - poll RG_AES_RY. */
    while(!pal_trx_bit_read(SR_AES_RY));
}



/**
 * @brief Reads the result of previous AES en/decryption
 *
 * This function returns the result of the previous AES operation,
 * so this function is needed in order to get the last result
 * of a series of sal_aes_wrrd() calls.
 *
 * @param[out] data     - result of previous operation
 */
void sal_aes_read(uint8_t *data)
{
    uint8_t i;

    for(i = 0; i < AES_BLOCKSIZE; ++i)
    {
        *data++ = pal_trx_reg_read(RG_AES_STATE);
    }
}

#endif /* AT86RFAx */

/* EOF */
