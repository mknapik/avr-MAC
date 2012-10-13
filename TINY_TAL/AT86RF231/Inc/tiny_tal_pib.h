/**
 * @file tiny_tal_pib.h
 *
 * @brief This file contains the prototypes for TAL PIB functions.
 *
 * $Id: tiny_tal_pib.h 21512 2010-04-13 08:18:44Z sschneid $
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
#ifndef TINY_TAL_PIB_H
#define TINY_TAL_PIB_H

/* === INCLUDES ============================================================ */


/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


void init_tal_pib(void);
void write_all_tal_pib_to_trx(void);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TINY_TAL_PIB_H */

/* EOF */
