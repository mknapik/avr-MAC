/**
 * @file eeprom.c
 *
 * @brief  Implementation of the native EEPROM interface.
 * 
 * Implements methods to get access to EEPROM hardware and
 * to read and write bytes to and from EEPROM
 * Available on following platforms:
 *    - Radio COntroller Board
 *    - Sensor Terminal Board
 *    - deRFnode
 *
 *
 * IMPORTANT: THIS FEATURE ONLY WORKS WITH deRFmega128 REVISION 2 AND HIGHER!!
 *
 * $Id: eeprom.c,v 1.1.4.1 2010/09/07 17:39:36 dam Exp $
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
#include <stdio.h>            // include standard io definitions
#include <stdbool.h>          // include bool definition
#include <avr/io.h>           // include io definitions
#include <util/twi.h>         // include 2-wire definitions
#include <util/delay.h>

#include "config.h"           // include configuration
#include "eeprom.h"           // include EEPROM specific definitions

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ============================================================= */

/**
 * Saved TWI status register, for error messages only.  We need to
 * save it in a variable, since the datasheet only guarantees the TWSR
 * register to have valid contents while the TWINT bit in TWCR is set.
 */
uint8_t twst;

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Initialize the EEPROM hardware
 *
 * Do all the startup-time peripheral initializations: TWI clock, ...
 *
 */
void eeprom_init(void)
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  /* has prescaler (mega128 & newer) */

  /*              F_CPU
   *  SCL = -----------------
   *        16+2(TWBR)*4^TWPS
   */
  TWSR = 0;

#if F_CPU < 3600000UL

  TWBR = 10;   /* smallest TWBR value, see note [5] */
#else

  TWBR = (F_CPU / 100000UL - 16) / 2;  // with 8MHz -> 32;
#endif
}


/**
 * @brief Read "len" bytes from EEPROM starting at "eeaddr" into "buf".
 *
 * This requires two bus cycles: during the first cycle, the device
 * will be selected (master transmitter mode), and the address
 * transfered.
 *
 * The second bus cycle will reselect the device (repeated start
 * condition, going into master receiver mode), and transfer the data
 * from the device to the TWI master.  Multiple bytes can be
 * transfered by ACKing the client's transfer.  The last transfer will
 * be NACKed, which the client will take as an indication to not
 * initiate further transfers.
 *
 * @param eeaddr  the EEPROM address where to read data from
 * @param len     the length of data
 * @param buf     pointer to data which should be read
 *
 * @return        number of readed bytes, or -1 if any error occured
 *
 *
*/
int32_t eeprom_read_bytes(uint32_t eeaddr, uint16_t len, uint8_t *buf)
{
  uint8_t sla, twcr, n = 0;
  int32_t rv = 0;

  sla = TWI_SLA_24CXX;

  if(eeaddr & MAX_ADDRESS_MASK_24CXX)
  {
    sla |= ADDR_HIGH_24CXX;
  }

  /*
   * First cycle: master transmitter mode
   */
restart:
  if (n++ >= MAX_ITER)
    return -1;
begin:

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_REP_START:    /* OK, but should not happen */
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -1;  /* error: not in start condition */
      /* NB: do /not/ send stop condition */
  }

  TWDR = sla | TW_WRITE;         /* send SLA+W */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:   /* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:   /* re-arbitrate */
      goto begin;

    default:
      goto error;  /* must send stop condition */
  }

  TWDR = (eeaddr >> 8) & 0xFF; /* high 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN);  /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;  /* must send stop condition */
  }

  TWDR = eeaddr & 0xFF;  /* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;  /* must send stop condition */
  }

  /*
   * Next cycle(s): master receiver mode
   */
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send (rep.) start condition */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_START:    /* OK, but should not happen */
    case TW_REP_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;
  }

  /* send SLA+R */
  TWDR = sla | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MR_SLA_ACK:
      break;

    case TW_MR_SLA_NACK:
      goto quit;

    case TW_MR_ARB_LOST:
      goto begin;

    default:
      goto error;
  }

  for (twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
      len > 0;
      len--)
  {
    if (len == 1)
    {
      twcr = _BV(TWINT) | _BV(TWEN); /* send NAK this time */
    }
    TWCR = twcr;  /* clear int to start transmission */
    while ((TWCR & _BV(TWINT)) == 0)
      ; /* wait for transmission */
    switch ((twst = TW_STATUS))
    {
      case TW_MR_DATA_NACK:
        len = 0;  /* force end of loop */
        *buf++ = TWDR;
        rv++;
        goto quit;
      case TW_MR_DATA_ACK:
        *buf++ = TWDR;
        rv++;
        break;

      default:
        goto error;
    }
  }
quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

error:
  rv = -1;
  goto quit;
}

/**
 * @brief Write "len" bytes into EEPROM starting at "eeaddr" from "buf".
 *
 * This is a bit simpler than the previous function since both, the
 * address and the data bytes will be transfered in master transmitter
 * mode, thus no reselection of the device is necessary.  However, the
 * EEPROMs are only capable of writing one "page" simultaneously, so
 * care must be taken to not cross a page boundary within one write
 * cycle.  The amount of data one page consists of varies from
 * manufacturer to manufacturer: some vendors only use 8-byte pages
 * for the smaller devices, and 16-byte pages for the larger devices,
 * while other vendors generally use 16-byte pages.  We thus use the
 * smallest common denominator of 8 bytes per page, declared by the
 * macro PAGE_SIZE above.
 *
 * The function simply returns after writing one page, returning the
 * actual number of data byte written.  It is up to the caller to
 * re-invoke it in order to write further data.
 *
 * @param eeaddr  the EEPROM address where to write data
 * @param len     the length of data
 * @param buf     pointer to data which should be written
 *
 * @return        number of written bytes, or -1 if any error occured
 *
**/
int16_t eeprom_write_page(uint32_t eeaddr, uint16_t len, uint8_t *buf)
{
  uint8_t sla, n = 0;
  int16_t rv = 0;
  uint32_t endaddr;

  if (eeaddr + len < (eeaddr | (PAGE_SIZE - 1)))
  {
    endaddr = eeaddr + len;
  }
  else
  {
    endaddr = (eeaddr | (PAGE_SIZE - 1)) + 1;
  }
  len = endaddr - eeaddr;

  sla = TWI_SLA_24CXX;

  if(eeaddr & MAX_ADDRESS_MASK_24CXX)
  {
    sla |= ADDR_HIGH_24CXX;
  }

restart:
  if (n++ >= MAX_ITER)
  {
    return -1;
  }
begin:

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_REP_START:    /* OK, but should not happen */
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -1;  /* error: not in start condition */
      /* NB: do /not/ send stop condition */
  }

  TWDR = sla | TW_WRITE; /* send SLA+W */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:   /* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:   /* re-arbitrate */
      goto begin;

    default:
      goto error;  /* must send stop condition */
  }

  TWDR = (eeaddr >> 8) & 0xFF; /* high 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;  /* must send stop condition */
  }

  TWDR = eeaddr & 0xFF; /* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0)
    ; /* wait for transmission */
  switch ((twst = TW_STATUS))
  {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;  /* must send stop condition */
  }

  for (; len > 0; len--)
  {
    TWDR = *buf++;
    TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
    while ((TWCR & _BV(TWINT)) == 0)
      ; /* wait for transmission */
    switch ((twst = TW_STATUS))
    {
      case TW_MT_DATA_NACK:
        goto error;  /* device write protected -- Note [16] */

      case TW_MT_DATA_ACK:
        rv++;
        break;

      default:
        goto error;
    }
  }
quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

error:
  rv = -1;
  goto quit;
}


/**
 * @brief Write 'len' bytes to EEPROM
 *
 * Wrapper around ee24xx_write_page() that repeats calling this
 * function until either an error has been returned, or all bytes
 * have been written.
 *
 * @param eeaddr  the EEPROM address where to write data
 * @param len     the length of data
 * @param buf     pointer to data which should be written
 *
 * @return        number of written bytes, or -1 if any error occured
 *
**/
int16_t eeprom_write_bytes(uint32_t eeaddr, uint16_t len, uint8_t *buf)
{
  int16_t rv;
  uint32_t total = 0;

  do
  {
    rv = eeprom_write_page(eeaddr, len, buf);
    if (rv == -1)
      return -1;
    eeaddr += rv;
    len -= rv;
    buf += rv;
    total += rv;
  }
  while (len > 0);

  return total;
}

