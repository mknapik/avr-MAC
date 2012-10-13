/**
 * @file tiny_tal_internal.h
 *
 * @brief This header file contains types and variable definition that are used within the TAL only.
 *
 * $Id: tiny_tal_internal.h 21512 2010-04-13 08:18:44Z sschneid $
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
#ifndef TINY_TAL_INTERNAL_H
#define TINY_TAL_INTERNAL_H

/* === INCLUDES ============================================================ */


/* === TYPES =============================================================== */

/** TAL states */
typedef enum tal_state_tag
{
    TAL_IDLE           = 0,
    TAL_TX_AUTO        = 1,
    TAL_TX_DONE        = 2
} SHORTENUM tal_state_t;

/* === EXTERNALS =========================================================== */

/* Global TAL variables */
extern tal_state_t tal_state;
extern tal_trx_status_t tal_trx_status;
extern bool tiny_tal_frame_rx;

/* === MACROS ============================================================== */

/**
 * Conversion of number of PSDU octets to duration in microseconds
 */
#ifdef HIGH_DATA_RATE_SUPPORT
    #define TAL_PSDU_US_PER_OCTET(octets)                                               \
    (                                                                                   \
        tal_pib_CurrentPage == 0 ? ((uint16_t)(octets) * 32) :                          \
           (                                                                            \
              tal_pib_CurrentPage == 2 ? ((uint16_t)(octets) * 16) :                    \
                (                                                                       \
                    tal_pib_CurrentPage == 16 ? ((uint16_t)(octets) * 8) : ((uint16_t)(octets) * 4)  \
                )                                                                       \
           )                                                                            \
    )
#else   /* #ifdef not HIGH_DATA_RATE_SUPPORT */
    #define TAL_PSDU_US_PER_OCTET(octets)       ((uint16_t)(octets) * 32)
#endif

#define TRX_IRQ_DEFAULT     TRX_IRQ_TRX_END

/* === PROTOTYPES ========================================================== */

/*
 * Prototypes from tal.c
 */
tal_trx_status_t set_trx_state(trx_cmd_t trx_cmd);

#endif /* TINY_TAL_INTERNAL_H */
