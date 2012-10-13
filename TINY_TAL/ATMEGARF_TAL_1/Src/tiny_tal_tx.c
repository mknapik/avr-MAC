/**
 * @file tiny_tal_tx.c
 *
 * @brief This file handles the frame transmission within the TAL.
 *
 * $Id: tiny_tal_tx.c 21517 2010-04-13 08:20:34Z sschneid $
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
#include "tiny_tal.h"
#include "ieee_const.h"
#include "tiny_tal_pib.h"
#include "tiny_tal_irq_handler.h"
#include "tiny_tal_constants.h"
#include "tiny_tal_tx.h"
#include "stack_config.h"
#include "tiny_tal_rx.h"
#include "tiny_tal_internal.h"
#include "atmega128rfa1.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */

static trx_trac_status_t trx_trac_status;

/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Requests to TAL to transmit frame
 *
 * This function is called by the MAC to deliver a frame to the TAL
 * to be transmitted by the transceiver.
 *
 * @param tx_frame Pointer to the frame array to be sent
 * @param csma_mode Indicates mode of csma-ca to be performed for this frame
 * @param perform_frame_retry Indicates whether to retries are to be performed for
 *                            this frame
 *
 * @return MAC_SUCCESS  if the TAL has accepted the data from the MAC for frame
 *                 transmission
 *         TAL_BUSY if the TAL is busy servicing the previous MAC request
 */
void tal_tx_frame(uint8_t *tx_frame,
                  csma_mode_t csma_mode,
                  bool perform_frame_retry)
{
    send_frame(tx_frame, csma_mode, perform_frame_retry);
}



/**
 * @brief Implements the handling of the transmission end.
 *
 * This function handles the callback for the transmission end.
 */
void tx_done_handling(void)
{
    tal_state = TAL_IDLE;

    switch (trx_trac_status)
    {
        case TRAC_SUCCESS:
            tal_tx_frame_done_cb(MAC_SUCCESS);
            break;

        case TRAC_SUCCESS_DATA_PENDING:
            tal_tx_frame_done_cb(TAL_FRAME_PENDING);
            break;

        case TRAC_CHANNEL_ACCESS_FAILURE:
            tal_tx_frame_done_cb(MAC_CHANNEL_ACCESS_FAILURE);
            break;

        case TRAC_NO_ACK:
            tal_tx_frame_done_cb(MAC_NO_ACK);
            break;

        case TRAC_INVALID:
            tal_tx_frame_done_cb(FAILURE);
            break;

        default:
            ASSERT("Unexpected tal_tx_state" == 0);
            tal_tx_frame_done_cb(FAILURE);
            break;
    }
} /* tx_done_handling() */



/**
 * @brief Sends frame
 *
 * @param frame_tx Pointer to prepared frame
 * @param use_csma Flag indicating if CSMA is requested
 * @param tx_retries Flag indicating if transmission retries are requested
 *                   by the MAC layer
 */
void send_frame(uint8_t *frame_tx, csma_mode_t csma_mode, bool tx_retries)
{
    tal_trx_status_t trx_status;

    // configure tx according to tx_retries
    if (tx_retries)
    {
        pal_trx_bit_write(SR_MAX_FRAME_RETRIES, tal_pib_MaxFrameRetries);
    }
    else
    {
        pal_trx_bit_write(SR_MAX_FRAME_RETRIES, 0);
    }

    // configure tx according to csma usage
    if ((csma_mode == NO_CSMA_NO_IFS) || (csma_mode == NO_CSMA_WITH_IFS))
    {
        if (tx_retries)
        {
            pal_trx_bit_write(SR_MAX_CSMA_RETRIES, tal_pib_MaxCSMABackoffs);
            pal_trx_reg_write(RG_CSMA_BE, 0x00);
        }
        else
        {
            pal_trx_bit_write(SR_MAX_CSMA_RETRIES, 7);
        }
    }
    else
    {
        pal_trx_reg_write(RG_CSMA_BE, ((tal_pib_MaxBE << 4) | tal_pib_MinBE));
        pal_trx_bit_write(SR_MAX_CSMA_RETRIES, tal_pib_MaxCSMABackoffs);
    }

    do
    {
        trx_status = set_trx_state(CMD_TX_ARET_ON);
    } while (trx_status != TX_ARET_ON);

    ENTER_CRITICAL_REGION();    // prevent from buffer underrun

    /* Handle interframe spacing */
    if (csma_mode == NO_CSMA_WITH_IFS)
    {
        if (frame_tx[0] > aMaxSIFSFrameSize)
        {
            pal_timer_delay(TAL_CONVERT_SYMBOLS_TO_US(macMinLIFSPeriod_def)
                            - IRQ_PROCESSING_DLY_US - PRE_TX_DURATION_US);
        }
        else
        {
            pal_timer_delay(TAL_CONVERT_SYMBOLS_TO_US(macMinSIFSPeriod_def)
                            - IRQ_PROCESSING_DLY_US - PRE_TX_DURATION_US);
        }
    }

    /* Toggle the SLP_TR pin triggering transmission. */
    PAL_SLP_TR_HIGH();
    PAL_WAIT_65_NS();
    PAL_SLP_TR_LOW();

    /*
     * Send the frame to the transceiver.
     * Note: The PhyHeader is the first byte of the frame to
     * be sent to the transceiver and this contains the frame
     * length.
     */
    pal_trx_frame_write(frame_tx, frame_tx[0] - 1);

    tal_state = TAL_TX_AUTO;

    LEAVE_CRITICAL_REGION();
}


/**
 * @brief Handles interrupts issued due to end of transmission
 */
void handle_tx_end_irq(void)
{
    if (tal_state == TAL_IDLE)
    {
        /*
         * This is a TX_END interrupt issued to indicate the
         * completion of the ACK transmission within an RX_AACK
         * transaction.  Ignore it.
         */
        return;
    }

    /* Read trac status before enabling RX_AACK_ON. */
    trx_trac_status = (trx_trac_status_t)pal_trx_bit_read(SR_TRAC_STATUS);

    // Trx has handled the entire transmission incl. CSMA
    tal_state = TAL_TX_DONE;    // Further handling is done by tx_done_handling()

    /*
     * After transmission has finished, switch receiver on again.
     */
    set_trx_state(CMD_RX_AACK_ON);
}

/* EOF */

