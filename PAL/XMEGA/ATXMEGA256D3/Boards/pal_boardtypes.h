/**
 * @file pal_boardtypes.h
 *
 * @brief PAL board types for ATxmega256D3
 *
 * This header file contains board types based on ATxmega256D3.
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

/* Boards for AT86RF230B */

/* STK600 board with
 * - REB to STK600 Adapter
 * - Radio Extender board REB230B V2.3
 */
#define REB_2_3_STK600              (0x01)

/* Boards for AT86RF231 */
/* STK600 board with
 * - REB to STK600 Adapter
 * - Radio Extender board REB231 V4.0.1
 */
#define REB_4_0_STK600              (0x11)

/* STK600 board with
 * - REB to STK600 Adapter
 * - Radio Extender board REB231 V4.0.1
 * CLKM from transceiver is used as timer source
 */
#define REB_4_0_STK600_USING_CLKM   (0x12)

/* STK600 board with
 * - REB to STK600 Adapter
 * - Radio Extender board REB231ED V4.1.1
 */
#define REB_4_1_STK600              (0x13)



/* Boards for AT86RF212 */
/* STK600 board with
 * - REB to STK600 Adapter
 * - Radio Extender board REB212 V5.0.2
 */
#define REB_5_0_STK600              (0x21)

#endif  /* #if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0) */

#endif  /* PAL_BOARDTYPES_H */
/* EOF */
