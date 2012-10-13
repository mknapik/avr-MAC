/**
 * @file stb.h
 *
 * @brief Declarations for security tool box
 *
 * This file contains declarations for the high-level security tool box API.
 *
 * $Id: stb.h 22706 2010-07-30 12:46:05Z sschneid $
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
#ifndef STB_H
#define STB_H

#if defined(STB_ON_SAL) || defined(STB_ARMCRYPTO) || defined(DOXYGEN)

/* === Includes =========================================================== */

#if defined(STB_ON_SAL)
#include "sal.h"
#elif defined (STB_ARMCRYPTO)
#include "stb_armcrypto.h"
#else
#error "Unknown STB type"
#endif

/* === Macros ============================================================= */

/*
 * The following defines are taken from the ZigBee specification and can be
 * to implement ZigBee security.
 */
/**
 * Length of ZigBee Frame Counter in octets.
 */
#define FRM_COUNTER_LEN                 (4)

/**
 * Position of Security Control octet in Auxiliary Security Header.
 */
#define MSDU_POS_SEC_CTRL               (1)
/**
 * Position of Frame Counter octet in Auxiliary Security Header.
 */
#define MSDU_POS_FRM_CNTR               (2)
/**
 * Position of first Source Addess octet in Auxiliary Security Header.
 */
#define MSDU_POS_SRC_ADDR               (6)
/**
 * Position of Key Sequence Number octet in Auxiliary Security Header.
 */
#define MSDU_POS_KEY_SEQ_NO             (14)

/**
 * Position of Security Control octet in Nonce.
 */
#define NONCE_POS_SEC_CTRL              (13)

/*
 * Definition of Security Control Field:
 *
 * Bit 0-2: Security Level = 6 (MIC length 8 bytes, encryption)
 * Bit 3-4: Key identifier - A Network key
 * Bit 5:   Extended Nonce enabled
 * Bit 6-7: Reserved
 */
/**
 * Position of Security Level bits in Security Control Field.
 */
#define SEC_CTRL_POS_SEC_LEVEL          (0)
/**
 * Position of Key identifier bits in Security Control Field.
 */
#define SEC_CTRL_POS_KEY_ID             (3)
/**
 * Position ofExtended Nonce enabled bit in Security Control Field.
 */
#define SEC_CTRL_POS_EXT_NONCE          (5)

/*
 * Definition of Key Identifier Sub-field of Security Control Field:
 *
 * 0x00: A Data key
 * 0x01: A Network key
 * 0x01: A Key-transport key
 * 0x01: A Key-load key
 */
/**
 * Key Identifier Sub-field of Security Control Field indicates Data key.
 */
#define KEY_ID_DATA_KEY                 (0x00)
/**
 * Key Identifier Sub-field of Security Control Field indicates Network key.
 */
#define KEY_ID_NWK_KEY                  (0x01)
/**
 * Key Identifier Sub-field of Security Control Field indicates Key-transport key.
 */
#define KEY_ID_KEY_TRANSPORT_KEY        (0x02)
/**
 * Key Identifier Sub-field of Security Control Field indicates Key-load key.
 */
#define KEY_ID_KEY_LOAD_KEY             (0x03)

/**
 * Maximum length of ZigBee Auxiliary Frame Header.
 */
/*
 * Format:
 * Securiy Control: 0/1 octets
 * Frame Counter: 4 octets
 * Source Address 0/8 octets
 * Key Sequence Number: 0/1 octet
 */
#define AUX_HDR_LEN                     (MSDU_POS_KEY_SEQ_NO)

/* === Types ============================================================== */

/**
 * CCM status values
 */
typedef enum
{
/** CCM en/decryption OK */
    STB_CCM_OK = 0,
/** Illegal parameter value in stb_ccm_secure() */
    STB_CCM_ILLPARM,
/** No previous key init in stb_ccm_secure() */
    STB_CCM_KEYMISS,
/** MIC error detected in stb_ccm_secure() */
    STB_CCM_MICERR
} SHORTENUM stb_ccm_t;

/* === Externals ========================================================== */


/* === Prototypes ========================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief STB Initialization
 *
 * This function initializes the STB.
 *
 * @ingroup apiStbApi
 */
void stb_init(void);

/**
 * @brief STB Restart
 *
 * This function re-starts the STB after power down.
 *
 * @ingroup apiStbApi
 */
void stb_restart(void);

/**
 * @brief Secure one block with CCM*
 *
 * This functions secures one block with CCM* according to 802.15.4.
 *
 * @param[in,out] buffer Input: plaintext header and payload concatenated;
 *                       for encryption: MUST HAVE 'AES_BLOCKSIZE'
 *                       BYTES SPACE AT THE END FOR THE MIC!
 *                       Output: frame secured (with MIC at end)/unsecured
 * @param[in]  nonce   The nonce: Initialization Vector (IV) as used in
 *                     cryptography; the ZigBee nonce (13 bytes long)
 *                     are the bytes 2...14 of this nonce
 * @param[in] key The key to be used; if NULL, use the current key
 * @param[in] hdr_len Length of plaintext header (will not be encrypted)
 * @param[in] pld_len Length of payload to be encrypted; if 0, then only MIC
 *                    authentication implies
 * @param[in] sec_level Security level according to IEEE 802.15.4,
 *                    7.6.2.2.1, Table 95:
 *                    - the value may be 0 ... 7;
 *                    - the two LSBs contain the MIC length in bytes
 *                      (0, 4, 8 or 16);
 *                    - bit 2 indicates whether encryption applies or not
 * @param[in] aes_dir AES_DIR_ENCRYPT if secure, AES_DIR_DECRYPT if unsecure
 *
 * @return STB CCM Status
 *
 * @ingroup apiStbApi
 */
stb_ccm_t stb_ccm_secure(uint8_t *buffer,
                         uint8_t nonce[AES_BLOCKSIZE],
                         uint8_t *key,
                         uint8_t hdr_len,
                         uint8_t pld_len,
                         uint8_t sec_level,
                         uint8_t aes_dir);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* #ifdef STB_ON_SAL */

#endif /* STB_H */
/* EOF */
