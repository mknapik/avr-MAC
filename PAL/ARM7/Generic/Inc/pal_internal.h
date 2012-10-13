/**
 * @file pal_internal.h
 *
 * @brief PAL internal functions prototypes for ARM7 MCUs
 *
 * $Id: pal_internal.h 16325 2009-06-23 16:40:23Z sschneid $
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
#ifndef PAL_INTERNAL_H
#define PAL_INTERNAL_H

/* === Includes ============================================================= */

#include "pal.h"

/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Macros ================================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

void gpio_init(void);
void trx_interface_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PAL_INTERNAL_H */
/* EOF */
