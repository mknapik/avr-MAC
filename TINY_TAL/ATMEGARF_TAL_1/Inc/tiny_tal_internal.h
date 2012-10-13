/**
 * @file tiny_tal_internal.h
 *
 * @brief This header file contains types and variable definition that are used within the TAL only.
 *
 * $Id: tiny_tal_internal.h 21688 2010-04-19 12:43:53Z sschneid $
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

/** Transceiver interrupt reasons */
typedef enum trx_irq_reason_tag
{
    /** No interrupt is indicated by IRQ_STATUS register */
    TRX_NO_IRQ                      = (0x00),

    /** PLL goes to lock-state. */
    TRX_IRQ_PLL_LOCK                = (0x01),

    /** Signals an unlocked PLL */
    TRX_IRQ_PLL_UNLOCK              = (0x02),

    /** Signals begin of a receiving frame */
    TRX_IRQ_RX_START                = (0x04),

    /** Signals end of frames (transmit and receive) */
    TRX_IRQ_RX_END                  = (0x08),

    /** Signals the end of a CCA or ED measurement. */
    TRX_IRQ_CCA_ED_READY            = (0x10),

    /** Signals an address match. */
    TRX_IRQ_AMI                     = (0x20),

    /** Signals the completion of a frame transmission. */
    TRX_IRQ_TX_END                  = (0x40),

    /**
     * Indicates that the radio transceiver reached TRX_OFF state
     * after P_ON, RESET, or SLEEP states.
     */
    TRX_IRQ_AWAKE                   = (0x80)
} trx_irq_reason_t;

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

/*
 * For Tiny TAL RX Start IRQ is NOT enabled, since the timestamp
 * shall not be taken.
 */
#define TRX_IRQ_DEFAULT         (TRX_IRQ_RX_END | TRX_IRQ_TX_END)
#define TRX_IRQ_NONE            (0)

/* === PROTOTYPES ========================================================== */

/*
 * Prototypes from tal.c
 */
tal_trx_status_t set_trx_state(trx_cmd_t trx_cmd);

#endif /* TINY_TAL_INTERNAL_H */
