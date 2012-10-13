/**
 * @file tiny_tal_tx_frame_done_cb.c
 *
 * @brief This file contains user call back function for
 * tal_tx_frame_done_cb.
 *
 * $Id: tiny_tal_tx_frame_done_cb.c 21515 2010-04-13 08:20:12Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================= */

#include <stdint.h>
#include <stdbool.h>
#include "tiny_tal.h"

/* === Macros ============================================================== */

/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ====================================================== */

void tal_send_frame_done_cb(retval_t status)
{
    /* Keep compiler happy. */
    status = status;
}

/* EOF */
