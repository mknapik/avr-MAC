/**
 * @file tiny_tal_tx.h
 *
 * @brief File contains the TAL sub-state macros and functions that
 * perform frame transmission.
 *
 * $Id: tiny_tal_tx.h 21516 2010-04-13 08:20:23Z sschneid $
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


/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


void handle_tx_end_irq(void);
void send_frame(uint8_t *frame_tx, csma_mode_t csma_mode, bool tx_retries);
void tx_done_handling(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINY_TAL_TX_H */

/* EOF */

