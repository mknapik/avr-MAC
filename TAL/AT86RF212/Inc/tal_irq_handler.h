/**
 * @file tal_irq_handler.h
 *
 * @brief This header file contains the interrupt handling definitions
 *
 * $Id: tal_irq_handler.h 21798 2010-04-23 10:19:18Z sschneid $
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
#ifndef TAL_IRQ_HANDLER_H
#define TAL_IRQ_HANDLER_H

/* === INCLUDES ============================================================ */


/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


void trx_irq_handler_cb(void);

#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) || (defined DOXYGEN)
void trx_irq_timestamp_handler_cb(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TAL_IRQ_HANDLER_H */

/* EOF */
