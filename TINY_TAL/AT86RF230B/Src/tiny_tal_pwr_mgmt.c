/**
 * @file tiny_tal_pwr_mgmt.c
 *
 * @brief This file implements TAL power management functionality
 *        of the transceiver.
 *
 * $Id: tiny_tal_pwr_mgmt.c 21512 2010-04-13 08:18:44Z sschneid $
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "pal.h"
#include "return_val.h"
#include "tiny_tal.h"
#include "tiny_tal_constants.h"
#include "at86rf230b.h"
#include "tiny_tal_internal.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Sets the transceiver to sleep
 *
 * This function sets the transceiver to sleep state.
 *
 * @param mode Defines sleep mode of transceiver SLEEP or PHY_TRX_OFF)
 *
 * @return   TAL_BUSY - The transceiver is busy in TX or RX
 *           MAC_SUCCESS - The transceiver is put to sleep
 *           TAL_TRX_ASLEEP - The transceiver is already asleep
 *           MAC_INVALID_PARAMETER - The specified sleep mode is not supported
 */
retval_t tal_trx_sleep(sleep_mode_t mode)
{
    tal_trx_status_t trx_status;

    /* Current transceiver only supports SLEEP_MODE_1 mode. */
    if (SLEEP_MODE_1 != mode)
    {
        return MAC_INVALID_PARAMETER;
    }

    if (tal_trx_status == TRX_SLEEP)
    {
        return TAL_TRX_ASLEEP;
    }

    /* Device can be put to sleep only when the TAL is in IDLE state. */
    if (TAL_IDLE != tal_state)
    {
        return TAL_BUSY;
    }

    /*
     * First set trx to TRX_OFF.
     * If trx is busy, like ACK transmission, do not interrupt it.
     */
    do
    {
        trx_status = set_trx_state(CMD_TRX_OFF);
    } while (trx_status != TRX_OFF);

    pal_timer_source_select(TMR_CLK_SRC_DURING_TRX_SLEEP);

    trx_status = set_trx_state(CMD_TRX_SLEEP);

    if (trx_status == TRX_SLEEP)
    {
        return MAC_SUCCESS;
    }
    else
    {
        /* State could not be set due to TAL_BUSY state. */
        return TAL_BUSY;
    }
}


/**
 * @brief Wakes up the transceiver from sleep
 *
 * This function awakes the transceiver from sleep state.
 *
 * @return   TAL_TRX_AWAKE - The transceiver is already awake
 *           MAC_SUCCESS - The transceiver is woken up from sleep
 *           FAILURE - The transceiver did not wake-up from sleep
 */
retval_t tal_trx_wakeup(void)
{
    tal_trx_status_t trx_status;

    if (tal_trx_status != TRX_SLEEP)
    {
        return TAL_TRX_AWAKE;
    }

    trx_status = set_trx_state(CMD_TRX_OFF);

    if (trx_status == TRX_OFF)
    {
        pal_timer_source_select(TMR_CLK_SRC_DURING_TRX_AWAKE);
        return MAC_SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

/* EOF */

