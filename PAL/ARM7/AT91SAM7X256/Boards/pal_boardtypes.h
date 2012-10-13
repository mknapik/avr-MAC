/**
 * @file pal_boardtypes.h
 *
 * @brief PAL board types for AT91SAM7X256
 *
 * This header file contains board types based on AT91SAM7X256.
 *
 * $Id: pal_boardtypes.h 22731 2010-08-03 14:54:19Z sschneid $
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

/*
 * AT91SAM7X-EK boards with Radio Extender board REB230B V2.3 on REBtoSAM7EK adapter
 * http://www.dresden-elektronik.de/shop/prod72.html
 * http://www.dresden-elektronik.de/shop/prod90.html
 */
#define REB_2_3_REB_TO_SAM7EK       (0x01)

/* AT91SAM7X-EK boards with Radio Extender board REB230B V2.3 on REX_ARM adapter Revision 2 */
#define REB_2_3_REX_ARM_REV_2       (0x02)



/* Boards for AT86RF231 */
/* AT91SAM7X-EK boards with Radio Extender board REB231 V4.0.1/4.0.2 on REX_ARM adapter Revision 3 */
#define REB_4_0_2_REX_ARM_REV_3     (0x11)



/* Boards for AT86RF212 */
/* AT91SAM7X-EK boards with Radio Extender board REB212 V5.0.2 on REX_ARM adapter Revision 3 */
#define REB_5_0_REX_ARM_REV_3       (0x21)

#endif  /* #if defined(VENDOR_BOARDTYPES) && (VENDOR_BOARDTYPES != 0) */

#endif  /* PAL_BOARDTYPES_H */

/* EOF */
