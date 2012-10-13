/**
 * @file stack_config.h
 *
 * @brief Stack configuration parameters
 *
 * $Id: stack_config.h 22984 2010-08-19 09:15:38Z sschneid $
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
#ifndef STACK_CONFIG_H
#define STACK_CONFIG_H

/* Highest stack layer definitions up to MAC
 *
 * Do NOT change order here! The numbering needs to start with the lowest layer
 * increasing to the higher layers.
 */
#define PAL                                 (1)
#define TINY_TAL                            (2)
#define TAL                                 (3)
#define MAC                                 (4)
/*
 * Layers above MAC are defined further below, since the enumeration
 * depends on the usage of the switch VENDOR_STACK_CONFIG.
 */

#ifdef VENDOR_STACK_CONFIG
#include "vendor_stack_config.h"
#else   /* Use standard stack layer configurations as defined below. */

/* Highest stack layer definitions above MAC */
#define RF4CE                               (5)

#if (HIGHEST_STACK_LAYER == PAL)
    /* Reduce the header file dependency by using hard-coded values */
    #define LARGE_BUFFER_SIZE               (160)
    #define SMALL_BUFFER_SIZE               (68)

#elif (HIGHEST_STACK_LAYER == TINY_TAL)
#define LARGE_BUFFER_SIZE                   (LENGTH_FIELD_LEN + aMaxPHYPacketSize + \
                                             LQI_LEN + ED_VAL_LEN)
#define SMALL_BUFFER_SIZE                   (0)

#elif (HIGHEST_STACK_LAYER == TAL) || (HIGHEST_STACK_LAYER == MAC)
/**
 * The following macro holds the size of a large buffer.
 * Additional octets for the length of the frame, the LQI
 * and the ED value are required.
 */
#ifdef MAC_SECURITY_ZIP
    #define MCPS_DATA_IND_SIZE  (32 + 3)    /* Size of mcps_data_ind_t incl. security for ZIP */
#else   /* No Security */
    #define MCPS_DATA_IND_SIZE  (32)        /* Size of mcps_data_ind_t w/o security */
#endif  /* MAC_SECURITY_ZIP */

#if ((PAL_GENERIC_TYPE == AVR) || (PAL_GENERIC_TYPE == XMEGA) || (PAL_GENERIC_TYPE == MEGA_RF))
/*
 * Size of mcps_data_ind_t + max number of payload octets +
 * 1 octet LQI  + 1 octet ED value.
 */
#define LARGE_BUFFER_SIZE                   (MCPS_DATA_IND_SIZE + \
                                             aMaxMACPayloadSize + \
                                             FCS_LEN + \
                                             LQI_LEN + ED_VAL_LEN)

/**
 * The following macro holds the size of a small buffer.
 * Additional octets for the length of the frame, the LQI
 * and the ED value are required.
 */
#define SMALL_BUFFER_SIZE                   (sizeof(frame_info_t) + \
                                             MAX_MGMT_FRAME_LENGTH + \
                                             LENGTH_FIELD_LEN + LQI_LEN + ED_VAL_LEN)
#elif ((PAL_GENERIC_TYPE == ARM7) || (PAL_GENERIC_TYPE == AVR32))
/*
 * Size of mcps_data_ind_t + max number of payload octets +
 * 1 octet LQI  + 1 octet ED value.
 * The buffer size has to be a DWORD.
 */
#define LARGE_BUFFER_SIZE                   (((MCPS_DATA_IND_SIZE + \
                                               aMaxMACPayloadSize + \
                                               LQI_LEN + ED_VAL_LEN) / 4 + 1) * 4)

/**
 * The following macro holds the size of a small buffer.
 * Additional octets for the length of the frame, the LQI
 * and the ED value are required.
 */
#define SMALL_BUFFER_SIZE                   (((sizeof(frame_info_t) + \
                                               MAX_MGMT_FRAME_LENGTH + \
                                               LENGTH_FIELD_LEN + LQI_LEN + ED_VAL_LEN) / 4 + 1) * 4)
#else
#error "Unknown PAL_GENERIC_TYPE for buffer calcluation"
#endif

#elif (HIGHEST_STACK_LAYER == RF4CE)
    /* Reduce the header file dependency by using hard-coded values */
    #define MCPS_DATA_IND_SIZE  (32)        /* Size of mcps_data_ind_t w/o MAC security */
    #define LARGE_BUFFER_SIZE           (MCPS_DATA_IND_SIZE + \
                                             aMaxMACPayloadSize + \
                                             FCS_LEN + \
                                             LQI_LEN + ED_VAL_LEN)
#else
#error "Unknown HIGHEST_STACK_LAYER for buffer calcluation"
#endif  /* #if (HIGHEST_STACK_LAYER == ...) */

/* Configuration if PAL is the highest stack layer */
#if (HIGHEST_STACK_LAYER == PAL)
#define NUMBER_OF_TOTAL_STACK_TIMERS        (0)
#define LAST_STACK_TIMER_ID                 (0)
#define NUMBER_OF_LARGE_STACK_BUFS          (0)
#define NUMBER_OF_SMALL_STACK_BUFS          (0)
#endif  /* (HIGHEST_STACK_LAYER == PAL) */

/* Configuration if TINY_TAL is the highest stack layer */
#if (HIGHEST_STACK_LAYER == TINY_TAL)
#include "tal_config.h"
#define NUMBER_OF_TOTAL_STACK_TIMERS        (0)
#define LAST_STACK_TIMER_ID                 (0)
#define NUMBER_OF_LARGE_STACK_BUFS          (0)
#define NUMBER_OF_SMALL_STACK_BUFS          (0)
#endif  /* (HIGHEST_STACK_LAYER == TINY_TAL) */

/* Configuration if TAL is the highest stack layer */
#if (HIGHEST_STACK_LAYER == TAL)
#include "tal_config.h"
#define NUMBER_OF_TOTAL_STACK_TIMERS        (NUMBER_OF_TAL_TIMERS)
#define LAST_STACK_TIMER_ID                 (TAL_LAST_TIMER_ID)
#define NUMBER_OF_LARGE_STACK_BUFS          (3)
#define NUMBER_OF_SMALL_STACK_BUFS          (0)
#endif  /* (HIGHEST_STACK_LAYER == TAL) */

/* Configuration if MAC is the highest stack layer */
#if (HIGHEST_STACK_LAYER == MAC)
#include "mac_config.h"
#define NUMBER_OF_TOTAL_STACK_TIMERS        (NUMBER_OF_TAL_TIMERS + NUMBER_OF_MAC_TIMERS)
#define LAST_STACK_TIMER_ID                 (MAC_LAST_TIMER_ID)
#if (MAC_INDIRECT_DATA_FFD == 1)
    #define NUMBER_OF_LARGE_STACK_BUFS      (6)
#else
    #define NUMBER_OF_LARGE_STACK_BUFS      (4)
#endif  /* (MAC_INDIRECT_DATA_FFD == 1) */
#define NUMBER_OF_SMALL_STACK_BUFS          (0)
#endif  /* (HIGHEST_STACK_LAYER == MAC) */

/* Configuration if RF4CE is the highest stack layer */
#if (HIGHEST_STACK_LAYER == RF4CE)
#include "nwk_config.h"
#define NUMBER_OF_TOTAL_STACK_TIMERS        (NUMBER_OF_TAL_TIMERS + NUMBER_OF_MAC_TIMERS + NUMBER_OF_NWK_TIMERS)
#define LAST_STACK_TIMER_ID                 (NWK_LAST_TIMER_ID)
#define NUMBER_OF_LARGE_STACK_BUFS          (5)
#define NUMBER_OF_SMALL_STACK_BUFS          (0)
#endif  /* (HIGHEST_STACK_LAYER == RF4CE) */

#endif  /* #ifdef VENDOR_STACK_CONFIG */

#endif /* STACK_CONFIG_H */
/* EOF */
