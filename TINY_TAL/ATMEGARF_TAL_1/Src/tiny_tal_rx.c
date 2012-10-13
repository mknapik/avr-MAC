/**
 * @file tiny_tal_rx.c
 *
 * @brief This file implements the frame reception functions.
 *
 * $Id: tiny_tal_rx.c 22626 2010-07-20 16:11:41Z sschneid $
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
#include "stack_config.h"
#include "tiny_tal_constants.h"
#include "tiny_tal_pib.h"
#include "tiny_tal_irq_handler.h"
#include "atmega128rfa1.h"
#include "tiny_tal_rx.h"
#include "tiny_tal_internal.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/* Constant defines for the LQI calculation */
#define ED_THRESHOLD                    (30)
#define ED_MAX_VAL                      (-RSSI_BASE_VAL - ED_THRESHOLD)
#define LQI_MAX                         (3)
#define UPPER_CLIPPING_VALUE            (-30)
/* Constant define for the ED scaling: register value at -35dBm */
#define CLIP_VALUE_REG                  (55)

#define US_PER_OCTECT                   (32)

/* === GLOBALS ============================================================= */

/**
 * Static receive buffer that can be used to upload a frame from the trx.
 * This buffer needs three extra octets:
 * Length, LQI, ED value
 */
uint8_t tiny_tal_rx_buffer[LARGE_BUFFER_SIZE];

/*
 * Flag indicating if an unprocessed frame has been received.
 */
bool tiny_tal_frame_rx;

/* === PROTOTYPES ========================================================== */

#ifdef RSSI_TO_LQI_MAPPING
static inline uint8_t normalize_lqi(uint8_t ed_value);
#else
static inline uint8_t normalize_lqi(uint8_t lqi, uint8_t ed_value);
#endif

/* === IMPLEMENTATION ====================================================== */

void handle_received_frame_irq(void)
{
    uint8_t ed_value;
    /* Actual frame length of received frame. */
    uint8_t phy_frame_len;

    uint8_t *rx_frame_ptr = tiny_tal_rx_buffer;

    /* Get ED value; needed to normalize LQI. */
    ed_value = pal_trx_reg_read(RG_PHY_ED_LEVEL);

    /* Get frame length from transceiver. */
    phy_frame_len = pal_trx_reg_read(RG_TST_RX_LENGTH);

    /* Check for valid frame length. */
    if (phy_frame_len > 127)
    {
        return;
    }

    /* Write length information. */
    *rx_frame_ptr++ = phy_frame_len;

    pal_trx_frame_read(rx_frame_ptr, phy_frame_len + LQI_LEN);

    /* Add ED value at the end of the frame buffer. */
    tiny_tal_rx_buffer[phy_frame_len + LQI_LEN + ED_VAL_LEN] = ed_value;

    /*
     * Release the protected buffer and set it again for further protection.
     */
    pal_trx_bit_write(SR_RX_SAFE_MODE, RX_SAFE_MODE_DISABLE);  /* Disable buffer protection mode */
    pal_trx_bit_write(SR_RX_SAFE_MODE, RX_SAFE_MODE_ENABLE);  /* Enable buffer protection mode */

    /* Set flag indicating received frame to be handled. */
    tiny_tal_frame_rx = true;

    /*
     * Trx returns to RX_AACK_ON automatically, if this was its previous state.
     * Keep the following as a reminder, if receiver is used with RX_ON instead.
     */
}



/**
 * @brief Processes received frame and invokes callback in application
 */
void process_incoming_frame(void)
{
    uint8_t ed_level;
    uint8_t lqi;

    lqi = tiny_tal_rx_buffer[tiny_tal_rx_buffer[0] + LQI_LEN];
    ed_level = tiny_tal_rx_buffer[tiny_tal_rx_buffer[0] + LQI_LEN + 1];

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
    tiny_tal_rx_buffer[tiny_tal_rx_buffer[0] + LQI_LEN] = lqi;

    /*
     * The callback function implemented by the application on top of Tiny-TAL.
     */
    tal_rx_frame_cb(tiny_tal_rx_buffer);
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
        if (ed_value > ED_MAX_VAL)
        {
            ed_value = ED_MAX_VAL;
        }
        else if (ed_value == 0)
        {
            ed_value = 1;
        }

        lqi_star = lqi >> 6;
        link_quality = (uint16_t)lqi_star * (uint16_t)ed_value * 255 / (ED_MAX_VAL * LQI_MAX);

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
        if (ed_value > ED_MAX_VAL)
        {
            return 0xFF;
        }
        else
        {
            return (ed_value * (255 / ED_MAX_VAL));
        }
    }
#endif
}
#endif /* #ifdef RSSI_TO_LQI_MAPPING */


/*  EOF */

