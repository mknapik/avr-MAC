/**
 * @file tiny_tal_constants.h
 *
 * @brief This file contains constants used througthout the TAL.
 *
 * $Id: tiny_tal_constants.h 21513 2010-04-13 08:19:13Z sschneid $
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
#ifndef TINY_TAL_CONSTANTS_H
#define TINY_TAL_CONSTANTS_H

/* === INCLUDES ============================================================ */


/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/*
 * TAL PIB default values
 */

/*
 * Default value of custom TAL PIB channel page
 */
#ifdef CHINESE_BAND
    #define TAL_CURRENT_PAGE_DEFAULT        (0x05)
#else
    #ifdef OQPSK_TEST
    #define TAL_CURRENT_PAGE_DEFAULT        (0x02)
    #else
    #define TAL_CURRENT_PAGE_DEFAULT        (0x00)
    #endif  /* #ifdef OQPSK_TEST */
#endif  /* #ifdef CHINESE_BAND */

/*
 * Default value of maximum number of symbols in a frame
 */
#define TAL_MAX_FRAME_DURATION_DEFAULT      (MAX_FRAME_DURATION)

/*
 * Default value of duration of the synchronization header (SHR) in symbols
 * for the current PHY
 */
#define TAL_SHR_DURATION_DEFAULT            (NO_OF_SYMBOLS_PREAMBLE_SFD)

/*
 * Default value of number of symbols per octet for the current PHY
 */
#define TAL_SYMBOLS_PER_OCTET_DEFAULT       (SYMBOLS_PER_OCTET)

/*
 * Default value of maximum backoff exponent used while performing csma ca
 */
#define TAL_MAXBE_DEFAULT                   (0x05)

/*
 * Default value of PIB attribute macMaxFrameRetries
 */
#define TAL_MAXFRAMERETRIES_DEFAULT         (0x03)

/*
 * Default value of maximum csma ca backoffs
 */
#define TAL_MAX_CSMA_BACKOFFS_DEFAULT       (0x04)

/*
 * Default value of minimum backoff exponent used while performing csma ca
 */
#define TAL_MINBE_DEFAULT                   (0x03)

/*
 * Value of a broadcast PAN ID
 */
#define TAL_PANID_BC_DEFAULT                (0xFFFF)

/*
 * Default value of short address
 */
#define TAL_SHORT_ADDRESS_DEFAULT           (0xFFFF)

/*
 * Default value of current channel in TAL
 */
#define TAL_CURRENT_CHANNEL_DEFAULT         (0x01)

/*
 * Default value of transmit power of transceiver
 * using IEEE defined format of phyTransmitPower
 * Following value corresponds to +3 dBm
 */
#define TAL_TRANSMIT_POWER_DEFAULT          (TX_PWR_TOLERANCE | 0x0A)

/*
 * Default value CCA mode
 */
#define TAL_CCA_MODE_DEFAULT                (TRX_CCA_MODE1)

/*
 * Default value beacon order set to 15
 */
#define TAL_BEACON_ORDER_DEFAULT            (15)

/*
 * Default value supeframe order set to 15
 */
#define TAL_SUPERFRAME_ORDER_DEFAULT        (15)

/*
 * Default value of BeaconTxTime
 */
#define TAL_BEACON_TX_TIME_DEFAULT          (0x00000000)

/*
 * Default value of BatteryLifeExtension.
 */
#define TAL_BATTERY_LIFE_EXTENSION_DEFAULT  (false)

/*
 * Default value of PAN Coordiantor custom TAL PIB
 */
#define TAL_PAN_COORDINATOR_DEFAULT         (false)

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINY_TAL_CONSTANTS_H */

/* EOF */


