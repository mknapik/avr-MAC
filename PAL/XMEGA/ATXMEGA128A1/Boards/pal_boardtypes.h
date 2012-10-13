/**
 * @file pal_boardtypes.h
 *
 * @brief PAL board types for ATxmega128A1
 *
 * This header file contains board types based on ATxmega128A1.
 *
 * $Id: pal_boardtypes.h 21966 2010-04-30 08:44:44Z uwalter $
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

/* RZ600 Boards - AT86RF230B on top of the Xplain Boards */
#define RZ600_230B_XPLAIN           (0x31)


#endif  /* #if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0) */

#endif  /* PAL_BOARDTYPES_H */

/* EOF */
