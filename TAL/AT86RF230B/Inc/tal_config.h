/**
 * @file tal_config.h
 *
 * @brief File contains TAL configuration parameters.
 *
 * $Id: tal_config.h 22647 2010-07-21 14:19:36Z sschneid $
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
#ifndef TAL_CONFIG_H
#define TAL_CONFIG_H

/* === INCLUDES ============================================================ */

#include "at86rf230b.h"
#include "mac_build_config.h"

/* === EXTERNALS =========================================================== */

/* === MACROS ============================================================== */

#define TAL_RADIO_WAKEUP_TIME_SYM       (TAL_CONVERT_US_TO_SYMBOLS(SLEEP_TO_TRX_OFF_US))
#define TAL_FIRST_TIMER_ID              (0)

#ifdef ENABLE_FTN_PLL_CALIBRATION
/*
 * PLL calibration and filter tuning timer timeout in minutes
 */
#define TAL_CALIBRATION_TIMEOUT_MIN         (5UL)

/*
 * PLL calibration and filter tuning timer timeout in us,
 */
#define TAL_CALIBRATION_TIMEOUT_US          ((TAL_CALIBRATION_TIMEOUT_MIN) * (60UL) * (1000UL) * (1000UL))
#endif  /* ENABLE_FTN_PLL_CALIBRATION */

/* === TYPES =============================================================== */

/* Timer ID's used by TAL */
// ED Scan is switched on
// Network with beacon support
#if ((MAC_SCAN_ED_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT))
    #ifdef ENABLE_FTN_PLL_CALIBRATION
        typedef enum tal_timer_id_tag
        {
            TAL_ACK_WAIT_TIMER              = (TAL_FIRST_TIMER_ID),
            TAL_ED_SCAN_DURATION_TIMER      = (TAL_FIRST_TIMER_ID + 1),
            TAL_ED_SAMPLE_TIMER             = (TAL_FIRST_TIMER_ID + 2),
            TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID + 3),
            TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 4),
            TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID + 5)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (6)
    #else
        typedef enum tal_timer_id_tag
        {
            TAL_ACK_WAIT_TIMER              = (TAL_FIRST_TIMER_ID),
            TAL_ED_SCAN_DURATION_TIMER      = (TAL_FIRST_TIMER_ID + 1),
            TAL_ED_SAMPLE_TIMER             = (TAL_FIRST_TIMER_ID + 2),
            TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID + 3),
            TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 4)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (5)
    #endif  /* ENABLE_FTN_PLL_CALIBRATION */
#endif

// ED Scan is switched on
// No beacon support
#if ((MAC_SCAN_ED_REQUEST_CONFIRM == 1) && (!defined BEACON_SUPPORT))
    #ifdef ENABLE_FTN_PLL_CALIBRATION
        typedef enum tal_timer_id_tag
        {
            TAL_ED_SCAN_DURATION_TIMER      = (TAL_FIRST_TIMER_ID),
            TAL_ED_SAMPLE_TIMER             = (TAL_FIRST_TIMER_ID + 1),
            TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID + 2)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (3)
    #else
        typedef enum tal_timer_id_tag
        {
            TAL_ED_SCAN_DURATION_TIMER      = (TAL_FIRST_TIMER_ID),
            TAL_ED_SAMPLE_TIMER             = (TAL_FIRST_TIMER_ID + 1)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (2)
    #endif  /* ENABLE_FTN_PLL_CALIBRATION */
#endif

// ED Scan is switched off
// Network with beacon support
#if ((MAC_SCAN_ED_REQUEST_CONFIRM == 0) && (defined BEACON_SUPPORT))
    #ifdef ENABLE_FTN_PLL_CALIBRATION
        typedef enum tal_timer_id_tag
        {
            TAL_ACK_WAIT_TIMER              = (TAL_FIRST_TIMER_ID),
            TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID + 1),
            TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 2),
            TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID + 3)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (4)
    #else
        typedef enum tal_timer_id_tag
        {
            TAL_ACK_WAIT_TIMER              = (TAL_FIRST_TIMER_ID),
            TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID + 1),
            TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 2)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (3)
    #endif  /* ENABLE_FTN_PLL_CALIBRATION */
#endif

// ED Scan is switched off
// No beacon support
#if ((MAC_SCAN_ED_REQUEST_CONFIRM == 0) && (!defined BEACON_SUPPORT))
    #ifdef ENABLE_FTN_PLL_CALIBRATION
        typedef enum tal_timer_id_tag
        {
            TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID)
        } tal_timer_id_t;

        #define NUMBER_OF_TAL_TIMERS        (1)
    #else
        #define NUMBER_OF_TAL_TIMERS        (0)
    #endif  /* ENABLE_FTN_PLL_CALIBRATION */
#endif

#if (NUMBER_OF_TAL_TIMERS > 0)
#define TAL_LAST_TIMER_ID    (TAL_FIRST_TIMER_ID + NUMBER_OF_TAL_TIMERS - 1) // -1: timer id starts with 0
#else
#define TAL_LAST_TIMER_ID    (TAL_FIRST_TIMER_ID)
#endif

#ifdef ENABLE_QUEUE_CAPACITY
#define TAL_INCOMING_FRAME_QUEUE_CAPACITY   (255)
#endif  /* ENABLE_QUEUE_CAPACITY */

/* === PROTOTYPES ========================================================== */


#endif /* TAL_CONFIG_H */
