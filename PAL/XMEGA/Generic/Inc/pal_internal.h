/**
 * @file pal_internal.h
 *
 * @brief PAL internal functions prototypes for AVR ATxmega MCUs
 *
 * $Id: pal_internal.h 19923 2010-01-14 11:00:19Z sschneid $
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

void clock_init(void);
void event_system_init(void);
#if (EXTERN_EEPROM_AVAILABLE == 1)
retval_t extern_eeprom_get(uint8_t start_offset, uint8_t length, void *value);
#endif
void gpio_init(void);
void interrupt_system_init(void);
void trx_interface_init(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PAL_INTERNAL_H */
/* EOF */
