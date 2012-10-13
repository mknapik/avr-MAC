/**
 * @file tal_rx.c
 *
 * @brief This file implements the frame reception functions.
 *
 * $Id: tal_rx.c 22618 2010-07-20 14:47:55Z uwalter $
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
#include "stack_config.h"
#include "bmm.h"
#include "qmm.h"
#include "tal_constants.h"
#include "tal_pib.h"
#include "tal_irq_handler.h"
#include "at86rf212.h"
#include "tal_rx.h"
#include "tal_internal.h"
#ifdef BEACON_SUPPORT
#include "tal_slotted_csma.h"
#endif  /* BEACON_SUPPORT */

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/* Constant defines for the LQI calculation */
#define ED_THRESHOLD                    (60)
#define ED_MAX                          (-RSSI_BASE_VAL_OQPSK_250 - ED_THRESHOLD)
#define LQI_MAX                         (3)
#define UPPER_CLIPPING_VALUE            (-60)
/* Constant define for the ED scaling: register value at -35dBm */
#define CLIP_VALUE_REG                  (62)

#define US_PER_OCTECT                   (32)

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */

#ifdef RSSI_TO_LQI_MAPPING
static inline uint8_t normalize_lqi(uint8_t ed_value);
#else
static inline uint8_t normalize_lqi(uint8_t lqi, uint8_t ed_value);
#endif

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Handle received frame interrupt
 *
 * This function handles transceiver interrupts for received frames and
 * uploads the frames from the trx.
 */
void handle_received_frame_irq(void)
{
    uint8_t ed_value;
    /* Actual frame length of received frame. */
    uint8_t phy_frame_len;
    /* Extended frame length appended by LQI and ED. */
    uint8_t ext_frame_length;
    frame_info_t *receive_frame;
    uint8_t *frame_ptr;

    if (tal_rx_buffer == NULL)
    {
        ASSERT("no tal_rx_buffer available" == 0);
        return;
    }

    receive_frame = (frame_info_t*)BMM_BUFFER_POINTER(tal_rx_buffer);

#ifdef PROMISCUOUS_MODE
    if (tal_pib_PromiscuousMode)
    {
        /* Check for valid FCS */
        if (pal_trx_bit_read(SR_RX_CRC_VALID) == CRC16_NOT_VALID)
        {
            return;
        }
    }
#endif

    /* Get ED value; needed to normalize LQI. */
    ed_value = pal_trx_reg_read(RG_PHY_ED_LEVEL);

    /* Get frame length from transceiver. */
    pal_trx_frame_read(&phy_frame_len, LENGTH_FIELD_LEN);

    /* Check for valid frame length. */
    if (phy_frame_len > 127)
    {
        return;
    }

    /*
     * The PHY header is also included in the frame (length field), hence the frame length
     * is incremented.
     * In addition to that, the LQI and ED value are uploaded, too.
     */
    ext_frame_length = phy_frame_len + LENGTH_FIELD_LEN + LQI_LEN + ED_VAL_LEN;

    /* Update payload pointer to store received frame. */
    frame_ptr = (uint8_t *)receive_frame + LARGE_BUFFER_SIZE - ext_frame_length;

    /*
     * Note: The following code is different from single chip
     * transceivers, since reading the frame via SPI contains the length field
     * in the first octet.
     */
    pal_trx_frame_read(frame_ptr, LENGTH_FIELD_LEN + phy_frame_len + LQI_LEN);
    receive_frame->mpdu = frame_ptr;
    /* Add ED value at the end of the frame buffer. */
    receive_frame->mpdu[phy_frame_len + LQI_LEN + ED_VAL_LEN] = ed_value;

#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
    /*
     * Store the timestamp.
     * The timestamping is only required for beaconing networks
     * or if timestamping is explicitly enabled.
     */
    receive_frame->time_stamp = tal_rx_timestamp;
#endif  /* #if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) */

    /* Append received frame to incoming_frame_queue and get new rx buffer. */
    qmm_queue_append(&tal_incoming_frame_queue, tal_rx_buffer);

    /* The previous buffer is eaten up and a new buffer is not assigned yet. */
    tal_rx_buffer = bmm_buffer_alloc(LARGE_BUFFER_SIZE);

    /* Check if receive buffer is available */
    if (NULL == tal_rx_buffer)
    {
        /*
         * Turn off the receiver until a buffer is available again.
         * tal_task() will take care of eventually reactivating it.
         */
        set_trx_state(CMD_PLL_ON);
        tal_rx_on_required = true;
    }
    else
    {
        /*
         * Trx returns to RX_AACK_ON automatically, if this was its previous state.
         * Keep the following as a reminder, if receiver is used with RX_ON instead.
         */
        //pal_trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
    }
}



/**
 * @brief Parses received frame and create the frame_info_t structure
 *
 * This function parses the received frame and creates the frame_info_t
 * structure to be sent to the MAC as a parameter of tal_rx_frame_cb().
 *
 * @param buf Pointer to the buffer containing the received frame
 */
void process_incoming_frame(buffer_t *buf_ptr)
{
    uint8_t frame_len;
    uint8_t *frame_ptr;
    uint8_t ed_level;
    uint8_t lqi;

    frame_info_t *receive_frame = (frame_info_t*)BMM_BUFFER_POINTER(buf_ptr);

    /* The frame is present towards the end of the buffer. */

    /*
     * Store the last frame length for IFS handling.
     * Substract LQI and length fields.
     */
    frame_len = last_frame_length = receive_frame->mpdu[0];

#ifdef PROMISCUOUS_MODE
    if (tal_pib_PromiscuousMode)
    {
        frame_ptr = &(receive_frame->mpdu[frame_len + LQI_LEN]);

        /*
         * The LQI is stored after the FCS.
         * The ED value is stored after the LQI.
         */
        lqi = *frame_ptr++;
        ed_level = *frame_ptr;

        /*
         * The LQI normalization is done using the ED level measured during
         * the frame reception.
         */
#ifdef RSSI_TO_LQI_MAPPING
        lqi = normalize_lqi(ed_level);
#else
        lqi = normalize_lqi(lqi, ed_level);
#endif

        /* Store normalized LQI value again. */
        frame_ptr--;
        *frame_ptr = lqi;

        receive_frame->buffer_header = buf_ptr;

         /* The callback function implemented by MAC is invoked. */
        tal_rx_frame_cb(receive_frame);

        return;
    }
#endif   /* #ifdef PROMISCUOUS_MODE */

#ifdef BEACON_SUPPORT
    /*
     * Are we waiting for a beacon for slotted CSMA?
     * Check if received frame is a beacon.
     */
    if ((receive_frame->mpdu[PL_POS_FCF_1] & FCF_FRAMETYPE_MASK) == FCF_FRAMETYPE_BEACON)
    {
        /* Debug pin to switch on: define ENABLE_DEBUG_PINS, pal_config.h */
        PIN_BEACON_START();

        if (tal_csma_state == BACKOFF_WAITING_FOR_BEACON)
        {
            /* Debug pin to switch on: define ENABLE_DEBUG_PINS, pal_config.h */
            PIN_WAITING_FOR_BEACON_END();
            tal_pib_BeaconTxTime = TAL_CONVERT_US_TO_SYMBOLS(receive_frame->time_stamp);
            tal_csma_state = CSMA_HANDLE_BEACON;
        }

        /* Debug pin to switch on: define ENABLE_DEBUG_PINS, pal_config.h */
        PIN_BEACON_END();
    }
#endif  /* BEACON_SUPPORT */

    /*
     * The LQI is stored after the FCS.
     * The ED value is stored after the LQI.
     */
    frame_ptr = &(receive_frame->mpdu[frame_len + LQI_LEN]);
    lqi = *frame_ptr++;
    ed_level = *frame_ptr;

    /*
     * The LQI normalization is done using the ED level measured during
     * the frame reception.
     */
#ifdef RSSI_TO_LQI_MAPPING
    lqi = normalize_lqi(ed_level);
#else
    lqi = normalize_lqi(lqi, ed_level);
#endif

    /* Store normalized LQI value again. */
    frame_ptr--;
    *frame_ptr = lqi;

    receive_frame->buffer_header = buf_ptr;

    /* The callback function implemented by MAC is invoked. */
    tal_rx_frame_cb(receive_frame);
} /* process_incoming_frame() */



#ifdef RSSI_TO_LQI_MAPPING
/**
 * @brief Normalize LQI
 *
 * This function normalizes the LQI value based on the RSSI/ED value.
 *
 * @param ed_value Read ED value
 *
 * @return The calculated/normalized LQI value: ppduLinkQuality
 */
static inline uint8_t normalize_lqi(uint8_t ed_value)
{
    /*
     * Scale ED result.
     * Clip values to 0xFF if > -35dBm
     */
    if (ed_value > CLIP_VALUE_REG)
    {
        return 0xFF;
    }
    else
    {
        return (uint8_t)(((uint16_t)ed_value * 0xFF) / CLIP_VALUE_REG);
    }
}

#else

/**
 * @brief Normalize LQI
 *
 * This function normalizes the LQI value based on the ED and
 * the originally appended LQI value.
 *
 * @param lqi Measured LQI
 * @param ed_value Read ED value
 *
 * @return The calculated LQI value: ppduLinkQuality
 */
static inline uint8_t normalize_lqi(uint8_t lqi, uint8_t ed_value)
{
    uint16_t link_quality;
    uint8_t lqi_star;

#ifdef HIGH_DATA_RATE_SUPPORT
    if (tal_pib_CurrentPage == 0)
    {
#endif
        if (ed_value > ED_MAX)
        {
            ed_value = ED_MAX;
        }
        else if (ed_value == 0)
        {
            ed_value = 1;
        }

        lqi_star = lqi >> 6;
        link_quality = (uint16_t)lqi_star * (uint16_t)ed_value * 255 / (ED_MAX * LQI_MAX);

        if (link_quality > 255)
        {
            return 255;
        }
        else
        {
            return (uint8_t)link_quality;
        }
#ifdef HIGH_DATA_RATE_SUPPORT
    }
    else    /* if (tal_pib_CurrentPage == 0) */
    {
        /* High data rate modes do not provide a valid LQI value. */
        if (ed_value > ED_MAX)
        {
            return 0xFF;
        }
        else
        {
            return (ed_value * (255 / ED_MAX));
        }
    }
#endif
}
#endif /* #ifdef RSSI_TO_LQI_MAPPING */


/*  EOF */

