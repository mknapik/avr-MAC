/**
 * @file pal_types.h
 *
 * @brief Definition of supported PAL types
 *
 * This header file contains the supported PAL types.
 *
 * $Id: pal_types.h,v 1.2.2.2 2010/09/07 17:38:25 dam Exp $
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
#ifndef PAL_TYPES_H
#define PAL_TYPES_H

/* PAL_GENERIC_TYPE */
#define AVR                             (0x01)
#define MEGA_RF                         (0x02)
#define XMEGA                           (0x03)
#define AVR32                           (0x04)
#define ARM7                            (0x05)
#define SAM3                            (0x06)


#if (PAL_GENERIC_TYPE == AVR)
    /* PAL_TYPE for AVR 8-bit MCUs */
    #define AT90USB1287                 (0x01)
    #define ATMEGA1281                  (0x02)
    #define ATMEGA1284P                 (0x03)
    #define ATMEGA2561                  (0x04)
    #define ATMEGA644P                  (0x05)

#elif (PAL_GENERIC_TYPE == XMEGA)
    /* PAL_TYPE for XMEGA MCUs */
    #define ATXMEGA128A1                (0x01)
    #define ATXMEGA128A3                (0x02)
    #define ATXMEGA256A3                (0x03)
    #define ATXMEGA256D3                (0x04)

#elif (PAL_GENERIC_TYPE == AVR32)
    /* PAL_TYPE for AVR 32-bit MCUs */
      #define AVR32UC3A3256               (0x01)
      #define AVR32UC3L064T               (0x02)
      #define AVR32UC3A0512               (0x03)

#elif (PAL_GENERIC_TYPE == ARM7)
    /* PAL_TYPE for ARM7 MCUs */
    #define AT91SAM7S256                (0x01)
    /* AT91SAM7X256 and AT91SAM7CX256:
     *
     * AT91SAM7CX256 is the same as AT91SAM7X256 except hardware AES engine.
     * If this boardtype is used, the file are actually taken from the
     * directory PAL/ARM7/AT91SAM7X256.
     * Only in case security shall be used, separate security files are selected
     * based on this PAL type.
     */
    #define AT91SAM7X256                (0x02)
    #define AT91SAM7XC256               (0x03)
    #define AT91SAM7X512                (0x04)

#elif (PAL_GENERIC_TYPE == SAM3)
    /* PAL_TYPE for SAM3 MCUs */
    #define AT91SAM3S4B                 (0x01)

#elif (PAL_GENERIC_TYPE == MEGA_RF)
    /* PAL_TYPE for MEGA-RF single chips (MCU plus transceiver) */
    #define ATMEGA64RFA1                (0x01)
    #define ATMEGA128RFA1               (0x02)
    #define ATMEGA256RFA1               (0x03)
    #define ATMEGA64RFR1                (0x04)
    #define ATMEGA128RFR1               (0x05)
    #define ATMEGA256RFR1               (0x06)

#else
    #error "Undefined PAL_GENERIC_TYPE"
#endif

/* Depending on the generic device type include platform-dependend types (IAR, GCC) */
#if ((PAL_GENERIC_TYPE == AVR) || (PAL_GENERIC_TYPE == XMEGA) || (PAL_GENERIC_TYPE == MEGA_RF))
#include "avrtypes.h"
#elif (PAL_GENERIC_TYPE == ARM7) || (PAL_GENERIC_TYPE == SAM3)
#include "armtypes.h"
#elif (PAL_GENERIC_TYPE == AVR32)
#include "avr32types.h"
#else
#error "Unknown PAL_GENERIC_TYPE"
#endif

#endif  /* PAL_TYPES_H */
/* EOF */
