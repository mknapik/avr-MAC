/**
 * @file tiny_tal_rx.h
 *
 * @brief File contains macros and modules used while processing
 * a received frame.
 *
 * $Id: tiny_tal_rx.h 21512 2010-04-13 08:18:44Z sschneid $
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
#ifndef TINY_TAL_RX_H
#define TINY_TAL_RX_H

/* === INCLUDES ============================================================ */


/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

void handle_received_frame_irq(void);

void process_incoming_frame(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINY_TAL_RX_H */
