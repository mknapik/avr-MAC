/**
 * @file eeprom.h
 *
 * @brief Application header file for eeprom.c.
 * 
 * Implements methods to get native access to EEPROM hardware.
 * You can read and write bytes to and from EEPROM.
 * 
 * $Id: eeprom.h,v 1.1.4.1 2010/09/07 17:39:32 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */


/* Prevent double inclusion */
#ifndef EEPROM_H_
#define EEPROM_H_

/* === Includes ============================================================= */

#include "config.h"     // include configuration

/* === MACROS / DEFINES ==================================================== */

#if !defined(__AVR_ATmega128RFA1__)
#error "EEPROM does not work with this MCU - only with deRFmega128 Rev. 2 (ATmega128RFA1)"
#endif // !defined(__AVR_ATmega128RFA1__)

/** @brief Macro to easy enable EEPROM */
#define EEPROM_ENABLE()       (PORTD &= ~_BV(PD6), DDRD |= _BV(PD6))
/** @brief Macro to easy disable EEPROM */
#define EEPROM_DISABLE()      (PORTD |= _BV(PD6),  DDRD |= _BV(PD6))

// SDI == PD3   on deRFmega128
// SCK == PD5   on deRFmega128


/**
 * TWI address for 24C1024 EEPROM:
 * <br>---------------------------------------
 * <br>| 1 | 0 | 1 | 0 | A2 | A1 | P0 | R/~W |
 * <br>---------------------------------------
 * <br>
 * A2/A1 = address selection <br>
 * P0    = highest address bit (17. bit of address) <br>
 * R/~W  = Read / Write <br>
 *
 */
#define TWI_SLA_24CXX               (0xA0)

/** maximum address number mask (AT24C1024 = 17 Bit address space -> masks 17. bit) */
#define MAX_ADDRESS_MASK_24CXX      (0x10000)

/** highest address bit position at SLA (AT24C1024) */
#define ADDR_HIGH_24CXX             (0x02)


/**
 * Maximal number of iterations to wait for a device to respond for a
 * selection.  Should be large enough to allow for a pending write to
 * complete, but low enough to properly abort an infinite loop in case
 * a slave is broken or not present at all.  With 100 kHz TWI clock,
 * transfering the start condition and SLA+R/W packet takes about 10
 * µs.  The longest write period is supposed to not exceed ~ 10 ms.
 * Thus, normal operation should not require more than 100 iterations
 * to get the device to respond to a selection.
 *
 */
#define MAX_ITER                    (200)


/**
 * Number of bytes that can be written in a row, see comments for
 * ee24xx_write_page() below.  Some vendor's devices would accept 16,
 * but 8 seems to be the lowest common denominator.
 *
 * Note that the page size must be a power of two, this simplifies the
 * page boundary calculations below.
 *
 */
//#define PAGE_SIZE                   (8)         /* 24C01 .. 24C16 */
//#define PAGE_SIZE                   (64)        /* 24C128 */
#define PAGE_SIZE                   (256UL)     /* AT24C1024 */


/** Number of pages (on AT24C1024) */
#define NUMBER_PAGES                (512UL)     /* AT24C1024 */

/** Last page address (on AT24C1024) */
#define LAST_PAGE_ADDRESS           ((PAGE_SIZE * NUMBER_PAGES) - PAGE_SIZE)     /* AT24C1024 */

/** First page address (on AT24C1024) - dummy value */
#define FIRST_PAGE_ADDRESS 0UL     /* AT24C1024 */

/* === Types ================================================================ */

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

void eeprom_init(void);
int32_t eeprom_read_bytes(uint32_t eeaddr, uint16_t len, uint8_t *buf);
int16_t eeprom_write_bytes(uint32_t eeaddr, uint16_t len, uint8_t *buf);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */


#endif /* EEPROM_H_ */

/* EOF */
