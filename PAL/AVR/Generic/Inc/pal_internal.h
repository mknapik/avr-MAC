/**
 * @file pal_internal.h
 *
 * @brief PAL internal functions prototypes for AVR ATmega MCUs
 *
 * $Id: pal_internal.h 20035 2010-01-27 10:34:41Z sschneid $
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


/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Macros ================================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#if (EXTERN_EEPROM_AVAILABLE == 1)
retval_t extern_eeprom_get(uint8_t start_offset, uint8_t length, void *value);
#endif
void gpio_init(void);

#if !defined(trx_interface_init)
void trx_interface_init(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PAL_INTERNAL_H */
/* EOF */
