/**
 * @file stb_app_defs.c
 *
 * @brief STB MAC Example 1 definitions
 *
 * This file contain definitions required for the STB MAC Example 1
 * application (both Data Sink and Sensor).
 *
 * $Id: stb_app_defs.c 16321 2009-06-23 16:35:40Z sschneid $
 *
 *  @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include "stb.h"
#include "stb_app.h"

/* === Types =============================================================== */


/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/* Arbitrary 128-bit key of demo application. */
uint8_t stb_app_key[AES_BLOCKSIZE] =
{
    0xA1, 0xA3, 0xA5, 0xA7,
    0x3C, 0x5E, 0x70, 0xB2,
    0x01, 0x52, 0x93, 0xC4,
    0x84, 0x48, 0x20, 0x10
};

/* EOF */
