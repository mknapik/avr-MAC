/**
 * @file tiny_tal_tx.h
 *
 * @brief File contains the TAL sub-state macros and functions that
 * perform frame transmission.
 *
 * $Id: tiny_tal_tx.h 21512 2010-04-13 08:18:44Z sschneid $
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
#ifndef TINY_TAL_TX_H
#define TINY_TAL_TX_H


/* === INCLUDES ============================================================ */

#include "tiny_tal.h"

/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/* States used by TX state machine */
typedef enum tal_tx_sub_state_tag
{
    TAL_TX_FRAME_PENDING            = (1 << 4),
    TAL_TX_SUCCESS                  = (2 << 4),
    TAL_TX_ACCESS_FAILURE           = (3 << 4),
    TAL_TX_NO_ACK                   = (4 << 4),
    TAL_TX_FAILURE                  = (5 << 4),
    TAL_TX_ACK_REQUIRED             = (6 << 4)
} tal_tx_sub_state_t;

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


void handle_tx_end_irq(void);
void send_frame(uint8_t *frame_tx, csma_mode_t csma_mode, bool tx_retries);
void tx_state_handling(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINY_TAL_TX_H */

/* EOF */

