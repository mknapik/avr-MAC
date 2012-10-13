/**
 * @file pal_boardtypes.h
 *
 * @brief PAL board types for AT91SAM7XC256
 *
 * This header file contains board types based on AT91SAM7XC256.
 *
 * $Id: pal_boardtypes.h 21920 2010-04-27 14:18:12Z sschneid $
 *
 */
/**
 * @author
 * Atmel Corporation: http://www.atmel.com
 * Support email: avr@atmel.com
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

/* AT91SAM7XC-EK boards with Radio Extender board REB230B V2.3 on REX_ARM adapter Revision 2 */
#define REB_2_3_REX_ARM_REV_2       (0x02)

#endif  /* #if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0) */

#endif  /* PAL_BOARDTYPES_H */

/* EOF */
