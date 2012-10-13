/**
 * @file tal_tx.h
 *
 * @brief File contains the TAL sub-state macros and functions that
 * perform frame transmission.
 *
 * $Id: tal_tx.h 16323 2009-06-23 16:38:09Z sschneid $
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
#ifndef TAL_TX_H
#define TAL_TX_H


/* === INCLUDES ============================================================ */

#include "tal.h"

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


void handle_tx_end_irq(bool underrun_occured);
void send_frame(uint8_t *frame_tx, csma_mode_t csma_mode, bool tx_retries);
void tx_state_handling(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TAL_TX_H */

/* EOF */

