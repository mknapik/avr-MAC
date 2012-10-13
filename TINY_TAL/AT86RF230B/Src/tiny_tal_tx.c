/**
 * @file tiny_tal_tx.c
 *
 * @brief This file handles the frame transmission within the TAL.
 *
 * $Id: tiny_tal_tx.c 21828 2010-04-23 12:01:36Z sschneid $
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
#include "at86rf230b.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/*
 * Mask used to extract TAL main state
 */
#define TAL_MAIN_STATE_MASK             (0x0F)

/*
 * Mask used to extract the TAL tx sub state
 */
#define TAL_TX_SUB_STATE_MASK           (0xF0)

/* === GLOBALS ============================================================= */


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
 * @brief Implements the TAL tx state machine.
 *
 * This function implements the TAL tx state machine.
 */
void tx_state_handling(void)
{
    tal_tx_sub_state_t tx_sub_state = (tal_tx_sub_state_t)(tal_state & TAL_TX_SUB_STATE_MASK);

    switch (tal_state & TAL_MAIN_STATE_MASK)
    {
        case TAL_TX_AUTO:
            switch (tx_sub_state)
            {
                case TAL_TX_FRAME_PENDING:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(TAL_FRAME_PENDING);
                    break;

                case TAL_TX_SUCCESS:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_SUCCESS);
                    break;

                case TAL_TX_ACCESS_FAILURE:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_CHANNEL_ACCESS_FAILURE);
                    break;

                case TAL_TX_NO_ACK:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_NO_ACK);
                    break;

                case TAL_TX_FAILURE:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(FAILURE);
                    break;

                default:
                    break;
            }
            break;

        case TAL_TX_BASIC:
            switch (tx_sub_state)
            {
                case TAL_TX_SUCCESS:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_SUCCESS);
                    break;

                default:
                    break;
            }
            break;
    }
} /* tx_state_handling() */


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
    else if ((csma_mode != NO_CSMA_NO_IFS) && (csma_mode != NO_CSMA_WITH_IFS)) // only necessary while using auto modes
    {
        pal_trx_bit_write(SR_MAX_FRAME_RETRIES, 0);
    }

    /* Prepare trx for transmission depending on the csma mode. */
    if ((csma_mode == NO_CSMA_NO_IFS) || (csma_mode == NO_CSMA_WITH_IFS))
    {
        do
        {
            trx_status = set_trx_state(CMD_PLL_ON);
        } while (trx_status != PLL_ON);

        tal_state = TAL_TX_BASIC;

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
    }
    else
    {
        do
        {
            trx_status = set_trx_state(CMD_TX_ARET_ON);
        } while (trx_status != TX_ARET_ON);

        tal_state = TAL_TX_AUTO;
    }

    pal_trx_irq_dis();

    /* Toggle the SLP_TR pin triggering transmission. */
    PAL_SLP_TR_HIGH();
    PAL_WAIT_65_NS();
    PAL_SLP_TR_LOW();

    /*
     * Send the frame to the transceiver.
     * Note: The PhyHeader is the first byte of the frame to
     * be sent to the transceiver and this contains the frame
     * length.
     * The actual length of the frame to be downloaded
     * (parameter two of pal_trx_frame_write)
     * is
     * 1 octet frame length octet
     * + n octets frame (i.e. value of frame_tx[0])
     * - 2 octets FCS
     */
    pal_trx_frame_write(frame_tx, frame_tx[0] - 1);

#ifndef NON_BLOCKING_SPI
    pal_trx_irq_en();
#endif
}


/**
 * @brief Handles interrupts issued due to end of transmission
 */
void handle_tx_end_irq(void)
{
    uint8_t trac_status;

    if (tal_state == TAL_TX_AUTO)
    {
        trac_status = pal_trx_bit_read(SR_TRAC_STATUS);

        /* Map status message of transceiver to TAL constants. */
        switch (trac_status)
        {
            case TRAC_SUCCESS_DATA_PENDING:
                tal_state |= TAL_TX_FRAME_PENDING;
                break;

            case TRAC_SUCCESS:
                tal_state |= TAL_TX_SUCCESS;
                break;

            case TRAC_CHANNEL_ACCESS_FAILURE:
                tal_state |= TAL_TX_ACCESS_FAILURE;
                break;

            case TRAC_NO_ACK:
                tal_state |= TAL_TX_NO_ACK;
                break;

            case TRAC_INVALID:
                tal_state |= TAL_TX_FAILURE;
                break;

            default:
                ASSERT("not handled trac status" == 0);
                tal_state |= TAL_TX_FAILURE;
                break;
        }
    }
    else if (tal_state == TAL_TX_BASIC)
    {
        tal_state |= TAL_TX_SUCCESS;
    }
}

/* EOF */

