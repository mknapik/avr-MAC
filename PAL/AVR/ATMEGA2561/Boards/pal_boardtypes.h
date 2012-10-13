/**
 * @file pal_boardtypes.h
 *
 * @brief PAL board types for ATmega2561
 *
 * This header file contains board types based on ATmega2561.
 *
 * $Id: pal_boardtypes.h 22072 2010-05-12 17:07:41Z sschneid $
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

/* Boards for AT86RF230B */
/* STK500 & STK501 boards with Radio Extender board REB230B V2.3 */
#define REB_2_3_STK500_STK501   (0x11)



/*
 * Boards for AT86RF231
 */
/*
 * RCB Breakout Board with Radio Controller board RCB231 4.0
 * http://www.dresden-elektronik.de/shop/prod85.html
 *
 * RCB Breakout Board Light with Radio Controller board RCB231 4.0
 * http://www.dresden-elektronik.de/shop/prod84.html
 */
#define RCB_4_0_BREAKOUT_BOARD  (0x21)

#endif  /* #if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0) */

#endif  /* PAL_BOARDTYPES_H */
/* EOF */
