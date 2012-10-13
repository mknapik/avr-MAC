/**
 * @file tiny_tal_rx_enable.c
 *
 * @brief File provides functionality supporting RX-Enable feature.
 *
 * $Id: tiny_tal_rx_enable.c 21512 2010-04-13 08:18:44Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "tiny_tal_tx.h"
#include "tiny_tal.h"
#include "ieee_const.h"
#include "tiny_tal_constants.h"
#include "at86rf230b.h"
#include "tiny_tal_rx.h"
#include "tiny_tal_internal.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Switches receiver on or off
 *
 * This function switches the receiver on (PHY_RX_ON) or off (PHY_TRX_OFF).
 *
 * @param state New state of receiver
 *
 * @return TAL_BUSY if the TAL state machine cannot switch receiver on or off,
 *         TRX_OFF if receiver has been switched off, or
 *         RX_ON otherwise.
 *
 * @ingroup apiTalApi
 */
uint8_t tal_rx_enable(uint8_t state)
{
    /*
     * Trx can only be enabled if TAL is not busy;
     * i.e. if TAL is IDLE.
     */
    if (TAL_IDLE != tal_state)
    {
        return TAL_BUSY;
    }

    if (state == PHY_TRX_OFF)
    {
        /*
         * If the rx needs to be switched off, we are not interested in a frame
         * that is currently being received.
         * This must not be a Forced TRX_OFF (CMD_FORCED_TRX_OFF) since this could
         * corrupt an already outoing ACK frame.
         */
         set_trx_state(CMD_TRX_OFF);
         return TRX_OFF;
    }
    else
    {
        set_trx_state(CMD_RX_AACK_ON);
        return RX_ON;   // MAC layer assumes RX_ON as return value
    }
}

/* EOF */

