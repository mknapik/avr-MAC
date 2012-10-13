/**
 * @file stb_internal.h
 *
 * @brief Declarations for security tool box
 *
 * This file contains internal declarations for the high-level security
 * tool box.
 *
 * $Id: stb_internal.h 20253 2010-02-09 09:00:20Z sschneid $
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
#ifndef STB_INTERNAL_H
#define STB_INTERNAL_H

#ifdef STB_ON_SAL


/* === Includes =========================================================== */

/* === Macros ============================================================= */

/* === Types ============================================================== */

/* === Externals ========================================================== */

/* === Prototypes ========================================================= */

#ifdef __cplusplus
extern "C" {
#endif

void compute_mic(uint8_t *buffer,
                 uint8_t *mic,
                 uint8_t *nonce,
                 uint8_t hdr_len,
                 uint8_t pld_len);

void encrypt_pldmic(uint8_t *buffer,
                    uint8_t *nonce,
                    uint8_t mic_len,
                    uint8_t pld_len);

void encrypt_with_padding(uint8_t *start, uint8_t buflen);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* #ifdef STB_ON_SAL */

#endif /* STB_INTERNAL_H */
/* EOF */
