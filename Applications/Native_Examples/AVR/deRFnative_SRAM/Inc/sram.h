/**
 * @file sram.h
 *
 * @brief Application header file for sram.c.
 *
 * Low Level functions to access SRAM which can be found on:
 *    - Sensor Terminal Board
 *
 * $Id: sram.h,v 1.1.4.1 2010/09/07 17:39:33 dam Exp $
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
#ifndef SRAM_H_
#define SRAM_H_

/* === Includes ============================================================= */

#include "config.h"     // include configuration

/* === MACROS / DEFINES ==================================================== */

#if (PLATFORM != SENS_TERM_BOARD)
#error "PLATFORM not supported - SRAM only available on Sensor Terminal Board"

#else

#if defined(__AVR_ATmega128RFA1__)
/* macros to easy enable and disable the latch on STB */
#define LATCH_ENABLE()           (PORTG |=  _BV(PG2), DDRG |= _BV(PG2))
#define LATCH_DISABLE()          (PORTG &= ~_BV(PG2), DDRG |= _BV(PG2))

/* macros to set write mode of SRAM on STB */
#define SRAM_WR_ENABLE()         (PORTE &= ~_BV(PE4), DDRE |= _BV(PE4))
#define SRAM_WR_DISABLE()        (PORTE |=  _BV(PE4), DDRE |= _BV(PE4))
#define SRAM_WR_TOGGLE()         (DDRE |= _BV(PE4), PORTE &= ~_BV(PE4), PORTE |=  _BV(PE4))

/* macros to set read mode of SRAM on STB */
#define SRAM_RD_ENABLE()         (PORTE &= ~_BV(PE5), DDRE |= _BV(PE5))
#define SRAM_RD_DISABLE()        (PORTE |=  _BV(PE5), DDRE |= _BV(PE5))

#define SRAM_HIGH_ADDRESS_PORT   () // ATmega128 have less pins than mega1281 -> only 8K available
#define SRAM_HIGH_ADDRESS_DDR    () // and different ports are used to get acces to available 8K of SRAM

#define SRAM_LOW_ADDRESS_PORT    (PORTB)
#define SRAM_LOW_ADDRESS_DDR     (DDRB)

#define SRAM_DATA_PORT           (PORTB)
#define SRAM_DATA_DDR            (DDRB)
#define SRAM_DATA_PINS           (PINB)
#endif // defined(__AVR_ATmega128RFA1__)

#if defined(__AVR_ATmega1281__)
/* address where SRAM is mapped to (when external memeory interface is used) */
#define SRAM_MEM_AD  (0x8000)
#endif

/* maximum address space of SRAM */
#if defined(__AVR_ATmega1281__)
#define MAX_ADDRESS     (0x8000)  // 32KB
#elif defined(__AVR_ATmega128RFA1__)
#define MAX_ADDRESS     (0x2000)  // 8KB
#endif

#if defined(__AVR_ATmega1281__)
/* enable external memory interface (ONLY ATmega1281) */
#define XMEM_ENABLE()            (XMCRA |= _BV(SRE), XMCRB = _BV(XMBK))
#elif defined(__AVR_ATmega128RFA1__)
#define XMEM_ENABLE()
#endif

#endif // defined(PLATFORM == SENS_TERM_BOARD)

/* === Types ================================================================ */

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

void sram_init(void);
uint8_t sram_write(uint16_t address, uint8_t data);
uint8_t sram_read(uint16_t address, uint8_t* data);
void sram_clear(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */

#endif /* SRAM_H_ */

/* EOF */
