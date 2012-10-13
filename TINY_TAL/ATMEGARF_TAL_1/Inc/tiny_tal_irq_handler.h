/**
 * @file tiny_tal_irq_handler.h
 *
 * @brief This header file contains the interrupt handling definitions
 *
 * $Id: tiny_tal_irq_handler.h 21516 2010-04-13 08:20:23Z sschneid $
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
#ifndef TINY_TAL_IRQ_HANDLER_H
#define TINY_TAL_IRQ_HANDLER_H

/* === INCLUDES ============================================================ */


/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


void trx_rx_end_handler_cb(void);
void trx_tx_end_handler_cb(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TINY_TAL_IRQ_HANDLER_H */

/* EOF */
