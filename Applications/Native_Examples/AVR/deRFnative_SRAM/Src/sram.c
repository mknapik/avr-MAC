/**
 * @file sram.c
 *
 * @brief  Implementation of native access to SRAM on Sensor Terminal Board.
 *
 * Low Level functions for SRAM access on:
 *    - Sensor Terminal Board
 *
 * The SRAM provides 32K x 8Bit of memory. With use of deRFmega128 (ATmega128RFA1)
 * only 8K can be used, since there are less pins on ATmega128RFA1 than ATmega1281.
 *
 * $Id: sram.c,v 1.1.4.1 2010/09/07 17:39:31 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

/* === INCLUDES ============================================================ */
#include <avr/io.h>           // include io definitions

#include "config.h"
#include "sram.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

#if defined(__AVR_ATmega1281__)
static volatile unsigned char* pSRAM = (unsigned char *) SRAM_MEM_AD;
#endif

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief SRAM initialization function.
 *
 * This function initializes the SRAM hardware
 *
 */
void sram_init(void)
{
   /* make sure that external memory is enabled - only on ATmega1281 */
#if defined(__AVR_ATmega1281__)
   XMCRA |= _BV(SRE);
   XMCRB = _BV(XMBK);
#endif

}

/**
 * @brief Funtion to set address to SRAM
 *
 * Before read or write any data to SRAM the specified address has to be set.
 * This function is only needed on deRFmega128, since it does not provide any
 * external memory interface like ATmega1281.
 *
 * @param address address that should be set
 *
 */
void sram_set_address(uint16_t address)
{
#if defined(__AVR_ATmega128RFA1__) && (PLATFORM == SENS_TERM_BOARD)
   /* prevent that data is send to USB interface, because data lines are shared -> disable USB CE */
   PORTD |= _BV(PD7);
   DDRD |= _BV(PD7);

   /*
    * LOW ADDRESS PART
    */

   /* enable latch to take over following data */
   LATCH_ENABLE();

   /* prepare data which should be stored in latch */
   SRAM_LOW_ADDRESS_PORT = address & 0xFF;
   SRAM_LOW_ADDRESS_DDR  = 0xFF;

   /* disable latch to be able set new data on same port */
   LATCH_DISABLE();

   /*
    * HIGH ADDRESS PART
    */

   /* set two bits from Port G -> two bits from high address */
   if(address & 0x0100){PORTG |= _BV(PG1);} else{PORTG &= ~_BV(PG1);}
   if(address & 0x0200){PORTG |= _BV(PG5);} else{PORTG &= ~_BV(PG5);}

   /* set bits to output for these two pins */
   DDRG  |= _BV(PG1) | _BV(PG5); // bits to output

   /* set bits from Port D -> three bits from high address */
   if(address & 0x0400){PORTD |= _BV(PD4);} else{PORTD &= ~_BV(PD4);}
   if(address & 0x0800){PORTD |= _BV(PD5);} else{PORTD &= ~_BV(PD5);}
   if(address & 0x1000){PORTD |= _BV(PD6);} else{PORTD &= ~_BV(PD6);}

   /* set bits to output for these three pins */
   DDRD |= _BV(PD4) | _BV(PD5) | _BV(PD6);

#endif // defined(__AVR_ATmega128RFA1__)
}

/**
 * @brief Funtion to write to sram.
 *
 * This function writes a byte at the specified address to SRAM
 *
 * Attention: With deRFmega128 only 8K of SRAM are available
 *            -> there are less pins on ATmega128RFA1 available
 *
 * different steps:
 * 1) write low part from addr[0:7] to latch
 * 2) enable Latch
 * 3) enable sram access
 * 4) write data to Port
 * 5) send write signal to sram
 *
 * @param   address  address where data should be written
 * @param   data     data which should be written
 *
 * @return '1' on success, '0' on error
 *
 */
uint8_t sram_write(uint16_t address, uint8_t data)
{
   if(address > MAX_ADDRESS){return 0;}

#if defined(__AVR_ATmega128RFA1__) && (PLATFORM == SENS_TERM_BOARD)

   /* set address, which to write, to SRAM */
   sram_set_address(address);

   /* set data to port which to write */
   SRAM_DATA_PORT = data;
   /* all these pins have to be output */
   SRAM_DATA_DDR = 0xFF;

   /* send write signal */
   SRAM_WR_TOGGLE();

#endif // defined(__AVR_ATmega128RFA1__) && (PLATFORM == SENS_TERM_BOARD)

#if defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)

   pSRAM[address] = data;

#endif // defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)

   return 1;
}

/**
 * @brief Funtion to read from sram.
 *
 * This function reads a byte at the specified address from SRAM
 *
 * Attention: With deRFmega128 only 8K of SRAM is available
 *            -> there are less pins on ATmega128RFA1 available
 *
 * different steps:
 * 1) write low part from addr[0:7] to latch
 * 2) enable Latch
 * 3) enable sram access
 * 4) write data to Port
 * 5) send write signal to sram
 *
 * @param address address where data should be read from
 * @param data    pointer to data
 *
 * @return  '1' on success, '0' on error
 *
 */
uint8_t sram_read(uint16_t address, uint8_t* data)
{
   if(address > MAX_ADDRESS){return 0;}

#if defined(__AVR_ATmega128RFA1__) && (PLATFORM == SENS_TERM_BOARD)

   /* set address, which to read, to SRAM */
   sram_set_address(address);

   /* data input port should be set to input */
   SRAM_DATA_DDR = 0x00;

   /* activate read signal */
   SRAM_RD_ENABLE();

   /* read back data from SRAM */
   *data = SRAM_DATA_PINS;

   /* disable read signal */
   SRAM_RD_DISABLE();

#endif // defined(__AVR_ATmega128RFA1__) && (PLATFORM == SENS_TERM_BOARD)

#if defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)

   *data = pSRAM[address];

#endif // defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)

   return 1;
}

/**
 * @brief Funtion clear complete SRAM.
 *
 * This function clear sequentially the complete available memory of SRAM
 *
 */
void sram_clear(void)
{
#if (PLATFORM == SENS_TERM_BOARD)
   uint16_t addr = 0;

   /* clear SRAM */
   for(; addr < MAX_ADDRESS; addr++)
   {
      sram_write(addr, 0x00);
   }
#endif // (PLATFORM == SENS_TERM_BOARD)
}

/* EOF */


