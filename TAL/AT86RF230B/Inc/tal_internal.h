/**
 * @file tal_internal.h
 *
 * @brief This header file contains types and variable definition that are used within the TAL only.
 *
 * $Id: tal_internal.h 22724 2010-08-03 11:12:04Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef TAL_INTERNAL_H
#define TAL_INTERNAL_H

/* === INCLUDES ============================================================ */

#include "bmm.h"
#include "qmm.h"
#ifdef BEACON_SUPPORT
#include "tal_slotted_csma.h"
#endif  /* BEACON_SUPPORT */
#ifdef ENABLE_DEBUG_PINS
#include "pal_config.h"
#endif
#include "mac_build_config.h"

/* === TYPES =============================================================== */

/** TAL states */
#if ((defined BEACON_SUPPORT) && (MAC_SCAN_ED_REQUEST_CONFIRM == 1))
typedef enum tal_state_tag
{
    TAL_IDLE           = 0,
    TAL_TX_BEACON      = 1,
    TAL_TX_AUTO        = 2,
    TAL_TX_BASIC       = 3,
    TAL_SLEEP          = 4,
    TAL_SLOTTED_CSMA   = 5,
    TAL_ED             = 6
} tal_state_t;
#endif

#if ((!defined BEACON_SUPPORT) && (MAC_SCAN_ED_REQUEST_CONFIRM == 1))
typedef enum tal_state_tag
{
    TAL_IDLE           = 0,
    TAL_TX_AUTO        = 2,
    TAL_TX_BASIC       = 3,
    TAL_SLEEP          = 4,
    TAL_ED             = 6
} tal_state_t;
#endif

#if ((defined BEACON_SUPPORT) && (MAC_SCAN_ED_REQUEST_CONFIRM == 0))
typedef enum tal_state_tag
{
    TAL_IDLE           = 0,
    TAL_TX_AUTO        = 2,
    TAL_TX_BASIC       = 3,
    TAL_SLEEP          = 4,
    TAL_SLOTTED_CSMA   = 5
} tal_state_t;
#endif

#if ((!defined BEACON_SUPPORT) && (MAC_SCAN_ED_REQUEST_CONFIRM == 0))
typedef enum tal_state_tag
{
    TAL_IDLE           = 0,
    TAL_TX_AUTO        = 2,
    TAL_TX_BASIC       = 3,
    TAL_SLEEP          = 4
} tal_state_t;
#endif

/* === EXTERNALS =========================================================== */

/* Global TAL variables */
extern tal_state_t tal_state;
extern tal_trx_status_t tal_trx_status;
extern frame_info_t *mac_frame_ptr;
extern queue_t tal_incoming_frame_queue;
extern uint8_t *tal_frame_to_tx;
extern buffer_t *tal_rx_buffer;
extern bool tal_rx_on_required;
extern uint8_t last_frame_length;

#ifdef BEACON_SUPPORT
extern csma_state_t tal_csma_state;
#if (MAC_START_REQUEST_CONFIRM == 1)
extern uint8_t transaction_duration_periods;
#endif
#endif  /* BEACON_SUPPORT */

#if ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT))
extern bool tal_beacon_transmission;
#endif /* ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT)) */
/* === MACROS ============================================================== */

/*
 * Debug synonyms
 * These debug defines are only applicable if
 * the build switch "-DENABLE_DEBUG_PINS" is set.
 * The implementation of the debug pins is located in
 * pal_config.h
 */
#ifdef ENABLE_DEBUG_PINS
#define PIN_BEACON_START()              TST_PIN_0_HIGH()
#define PIN_BEACON_END()                TST_PIN_0_LOW()
#define PIN_CSMA_START()                TST_PIN_1_HIGH()
#define PIN_CSMA_END()                  TST_PIN_1_LOW()
#define PIN_BACKOFF_START()             TST_PIN_2_HIGH()
#define PIN_BACKOFF_END()               TST_PIN_2_LOW()
#define PIN_CCA_START()                 TST_PIN_3_HIGH()
#define PIN_CCA_END()                   TST_PIN_3_LOW()
#define PIN_TX_START()                  TST_PIN_4_HIGH()
#define PIN_TX_END()                    TST_PIN_4_LOW()
#define PIN_ACK_WAITING_START()         TST_PIN_5_HIGH()
#define PIN_ACK_WAITING_END()           TST_PIN_5_LOW()
#define PIN_WAITING_FOR_BEACON_START()  TST_PIN_6_HIGH()
#define PIN_WAITING_FOR_BEACON_END()    TST_PIN_6_LOW()
#define PIN_BEACON_LOSS_TIMER_START()
#define PIN_BEACON_LOSS_TIMER_END()
#define PIN_ACK_OK_START()              TST_PIN_7_HIGH()
#define PIN_ACK_OK_END()                TST_PIN_7_LOW()
#define PIN_NO_ACK_START()              TST_PIN_8_HIGH()
#define PIN_NO_ACK_END()                TST_PIN_8_LOW()
#else
#define PIN_BEACON_START()
#define PIN_BEACON_END()
#define PIN_CSMA_START()
#define PIN_CSMA_END()
#define PIN_BACKOFF_START()
#define PIN_BACKOFF_END()
#define PIN_CCA_START()
#define PIN_CCA_END()
#define PIN_TX_START()
#define PIN_TX_END()
#define PIN_ACK_WAITING_START()
#define PIN_ACK_WAITING_END()
#define PIN_WAITING_FOR_BEACON_START()
#define PIN_WAITING_FOR_BEACON_END()
#define PIN_BEACON_LOSS_TIMER_START()
#define PIN_BEACON_LOSS_TIMER_END()
#define PIN_ACK_OK_START()
#define PIN_ACK_OK_END()
#define PIN_NO_ACK_START()
#define PIN_NO_ACK_END()
#endif

/* === PROTOTYPES ========================================================== */

/*
 * Prototypes from tal.c
 */
tal_trx_status_t set_trx_state(trx_cmd_t trx_cmd);
#ifdef ENABLE_FTN_PLL_CALIBRATION
void calibration_timer_handler_cb(void *parameter);
#endif  /* ENABLE_FTN_PLL_CALIBRATION */

#endif /* TAL_INTERNAL_H */
