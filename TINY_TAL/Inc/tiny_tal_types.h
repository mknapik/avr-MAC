/**
 * @file tiny_tal_types.h
 *
 * @brief This file contains defines for Tiny-TAL types.
 *
 * $Id: tiny_tal_types.h 22457 2010-07-06 16:05:21Z sschneid $
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
#ifndef TINY_TAL_TYPES_H
#define TINY_TAL_TYPES_H

/* === INCLUDES ============================================================ */

/* TAL types: */
#define AT86RF230A              (0x01)
#define AT86RF230B              (0x02)
#define AT86RF231               (0x11)
#define AT86RF212               (0x21)
/* TAL Type for Mega RF single chips, e.g. ATMEGA128RFA1 */
#define ATMEGARF_TAL_1          (0x31)
#define AT86RF232               (0x41)

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINY_TAL_TYPES_H */
/* EOF */
