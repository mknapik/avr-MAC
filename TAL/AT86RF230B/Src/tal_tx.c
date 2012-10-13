/**
 * @file tal_tx.c
 *
 * @brief This file handles the frame transmission within the TAL.
 *
 * $Id: tal_tx.c 22752 2010-08-04 14:09:28Z uwalter $
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
#include "tal.h"
#include "ieee_const.h"
#include "tal_pib.h"
#include "tal_irq_handler.h"
#include "tal_constants.h"
#include "tal_tx.h"
#include "stack_config.h"
#include "bmm.h"
#include "qmm.h"
#include "tal_rx.h"
#include "tal_internal.h"
#include "at86rf230b.h"
#ifdef BEACON_SUPPORT
#include "tal_slotted_csma.h"
#endif  /* BEACON_SUPPORT */
#include "mac_build_config.h"

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
 * @param tx_frame Pointer to the frame_info_t structure updated by the MAC layer
 * @param csma_mode Indicates mode of csma-ca to be performed for this frame
 * @param perform_frame_retry Indicates whether to retries are to be performed for
 *                            this frame
 *
 * @return MAC_SUCCESS  if the TAL has accepted the data from the MAC for frame
 *                 transmission
 *         TAL_BUSY if the TAL is busy servicing the previous MAC request
 */
retval_t tal_tx_frame(frame_info_t *tx_frame,
                      csma_mode_t csma_mode,
                      bool perform_frame_retry)
{
    if (tal_state != TAL_IDLE)
    {
        return TAL_BUSY;
    }

    /*
     * Store the pointer to the provided frame structure.
     * This is needed for the callback function.
     */
    mac_frame_ptr = tx_frame;

    /* Set pointer to actual mpdu to be downloaded to the transceiver. */
    tal_frame_to_tx = tx_frame->mpdu;

    /*
     * In case the frame is too large, return immediately indicating
     * invalid status.
     */
    if (tal_frame_to_tx == NULL)
    {
        return MAC_INVALID_PARAMETER;
    }

#ifdef BEACON_SUPPORT
    // check if beacon mode is used
    if (csma_mode == CSMA_SLOTTED)
    {
        slotted_csma_start(perform_frame_retry);
    }
    else
    {
#if (MAC_INDIRECT_DATA_FFD == 1)
        /*
         * Check if frame is using indirect transmission, but do not use the
         * indirect_in_transit flag. This flag is not set for null data frames.
         */
        if ((tal_pib_BeaconOrder < NON_BEACON_NWK) && (csma_mode == NO_CSMA_WITH_IFS) && (perform_frame_retry == false))
         {
            /*
             * Check if indirect transmission can be completed before the next
             * beacon transmission.
             */
            uint32_t time_between_beacons_sym;
            uint32_t next_beacon_time_sym;
            uint32_t now_time_sym;
            uint32_t duration_before_beacon_sym;

            /* Calculate the entire transaction duration. Re-use function of slotted CSMA.
             * The additional two backoff periods used for CCA are kept as a guard time.
             */
            calculate_transaction_duration();

            /* Calculate the duration until the next beacon needs to be transmitted. */
            time_between_beacons_sym = TAL_GET_BEACON_INTERVAL_TIME(tal_pib_BeaconOrder);
            next_beacon_time_sym = tal_add_time_symbols(tal_pib_BeaconTxTime,
                                                            time_between_beacons_sym);
            pal_get_current_time(&now_time_sym);
            now_time_sym = TAL_CONVERT_US_TO_SYMBOLS(now_time_sym);
            duration_before_beacon_sym = tal_sub_time_symbols(next_beacon_time_sym, now_time_sym);

            /* Check if transaction can be completed before next beacon transmission. */
            if ((now_time_sym >= next_beacon_time_sym) ||
                ((transaction_duration_periods * aUnitBackoffPeriod) > duration_before_beacon_sym))
            {
                /*
                 * Transaction will not be completed before next beacon transmission.
                 * Therefore the transmission is not executed.
                 */
                 return MAC_CHANNEL_ACCESS_FAILURE;
            }
        }
#endif  /* #if (MAC_INDIRECT_DATA_FFD == 1) */
        send_frame(tal_frame_to_tx, csma_mode, perform_frame_retry);
    }
#else   /* No BEACON_SUPPORT */
    send_frame(tal_frame_to_tx, csma_mode, perform_frame_retry);
#endif  /* BEACON_SUPPORT / No BEACON_SUPPORT */

    return MAC_SUCCESS;
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
                    tal_tx_frame_done_cb(TAL_FRAME_PENDING, mac_frame_ptr);
                    break;

                case TAL_TX_SUCCESS:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_SUCCESS, mac_frame_ptr);
                    break;

                case TAL_TX_ACCESS_FAILURE:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_CHANNEL_ACCESS_FAILURE, mac_frame_ptr);
                    break;

                case TAL_TX_NO_ACK:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(MAC_NO_ACK, mac_frame_ptr);
                    break;

                case TAL_TX_FAILURE:
                    tal_state = TAL_IDLE;
                    tal_tx_frame_done_cb(FAILURE, mac_frame_ptr);
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
                    tal_tx_frame_done_cb(MAC_SUCCESS, mac_frame_ptr);
                    break;

                default:
                    break;
            }
            break;

#if ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT))
        case TAL_TX_BEACON:
            switch (tx_sub_state)
            {
                case TAL_TX_SUCCESS:
                    if (tal_csma_state == BACKOFF_WAITING_FOR_BEACON)
                    {
                        tal_csma_state = CSMA_HANDLE_BEACON;
                        tal_state = TAL_SLOTTED_CSMA;
                    }
                    else
                    {
                        tal_state = TAL_IDLE;
                    }
                    break;

                default:
                    break;
            }
            break;
#endif /* ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT)) */
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

#if (DEBUG > 0)
    if ((tx_retries) && ((csma_mode == NO_CSMA_NO_IFS) || (csma_mode == NO_CSMA_WITH_IFS)))
    {
        ASSERT((tx_retries == true) && ((csma_mode == NO_CSMA_NO_IFS) || (csma_mode == NO_CSMA_WITH_IFS)));
    }
#endif

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
            if (last_frame_length > aMaxSIFSFrameSize)
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
    pal_trx_frame_write(frame_tx, frame_tx[0]-1);

#ifndef NON_BLOCKING_SPI
    pal_trx_irq_en();
#endif
}


/**
 * @brief Handles interrupts issued due to end of transmission
 */
void handle_tx_end_irq(bool underrun_occured)
{
    uint8_t trac_status;

    if (tal_state == TAL_TX_AUTO)
    {
        if (underrun_occured)
        {
            trac_status = TRAC_INVALID;
        }
        else
        {
            trac_status = pal_trx_bit_read(SR_TRAC_STATUS);
        }

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
#if ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT))
    else    // TAL_TX_BEACON
    {
        // debug pin to switch on: define ENABLE_DEBUG_PINS, pal_config.h
        PIN_BEACON_END();

        tal_state |= TAL_TX_SUCCESS;
    }
#endif /* ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT)) */
}



/**
 * @brief Beacon frame transmission
 */
#if ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT))
void tal_tx_beacon(frame_info_t *tx_frame)
{
    tal_trx_status_t trx_status;

    /* Set pointer to actual mpdu to be downloaded to the transceiver. */
    uint8_t *tal_beacon_to_tx = tx_frame->mpdu;

    /*
     * Avoid that the beacon is transmitted while transmitting
     * a frame using slotted CSMA.
     */
    if ((tal_csma_state == FRAME_SENDING_WITH_ACK) ||
        (tal_csma_state == FRAME_SENDING_NO_ACK) ||
        (tal_csma_state == WAITING_FOR_ACK))
    {
        ASSERT("trying to transmit beacon while ongoing transmission" == 0);
        return;
    }

    /* Send the pre-created beacon frame to the transceiver. */
    // debug pin to switch on: define ENABLE_DEBUG_PINS, pal_config.h
    PIN_BEACON_START();

    // @TODO wait for talbeaconTxTime
    do
    {
        trx_status = set_trx_state(CMD_FORCE_PLL_ON);
#if (DEBUG > 1)
        if (trx_status != PLL_ON)
        {
            ASSERT("PLL_ON failed for beacon transmission" == 0);
        }
#endif
    } while (trx_status != PLL_ON);


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
    pal_trx_frame_write(tal_beacon_to_tx, tal_beacon_to_tx[0] - 1);

#ifndef NON_BLOCKING_SPI
    pal_trx_irq_en();
#endif

    tal_state = TAL_TX_BEACON;
}
#endif /* ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT)) */

/* EOF */

