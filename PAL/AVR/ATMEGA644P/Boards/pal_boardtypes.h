/**
 * @file pal_boardtypes.h
 *
 * @brief PAL board types for ATmega644P
 *
 * This header file contains board types based on ATmega644P.
 *
 * $Id: pal_boardtypes.h 21920 2010-04-27 14:18:12Z sschneid $
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
#ifndef PAL_BOARDTYPES_H
#define PAL_BOARDTYPES_H

/* === Includes ============================================================= */

#if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0)
#include "vendor_boardtypes.h"
#else   /* Use standard board types as defined below. */

/* === Macros =============================================================== */

/*
 * Boards for AT86RF230B
 */
/* STK500 board with Radio Extender board REB230B V2.3 */
#define REB_2_3_STK500              (0x11)



/*
 * Boards for AT86RF231
 */
/* STK500 boards with Radio Extender board REB231ED V4.1.1 */
#define REB_4_1_STK500              (0x21)

#endif  /* #if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0) */

#endif  /* PAL_BOARDTYPES_H */
/* EOF */
