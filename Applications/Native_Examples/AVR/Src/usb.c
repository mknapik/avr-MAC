/**
 * @file usb.c
 *
 * @brief  Implementation of the native USB interface (FTDI chip).
 * 
 * Implements methods to get access to USB hardware and
 * to read and write bytes over USB interface.
 * Available on following platforms:
 *    - Sensor Terminal Board
 *    - deRFnode
 *
 * Note: Input and Output is done by mechanism of the "stdio.h" interface. One
 * need to setup a FILE stream with links to the putchar and getchar functions.
 *
 * $Id: usb.c,v 1.1.4.1 2010/09/07 17:39:33 dam Exp $
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
#include <util/delay.h>       // include delay definitions
#include <stdio.h>            // include standard io definitions
#include <stdbool.h>          // include bool definition

#include "config.h"           // include configuration
#include "usb.h"              // include USB specific definitions

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === PROTOTYPES ========================================================== */

uint8_t read_usb_byte(void);
void write_usb_byte(uint8_t data);

/* === GLOBALS ============================================================= */

#if (PLATFORM == SENS_TERM_BOARD) && defined(__AVR_ATmega1281__)
static volatile unsigned int* pUSB = (unsigned int*) USB_ADDRESS;
#endif // (PLATFORM == SENS_TERM_BOARD) && defined(__AVR_ATmega1281__)

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief USB interface initialization function.
 *
 * This function initializes the USB hardware.
 * This is only working if platform is Sensor Terminal Board or deRFnode.
 */
void usb_init(void)
{
#if (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)

   /* make sure that external memory is enabled - only on ATmega1281 */
#if defined(__AVR_ATmega1281__)
   XMCRA |= _BV(SRE);
   XMCRB = _BV(XMBK);
#endif

   /* Make sure USB_RXF and USB_TXE are inputs */
   USB_TXE_DDR  &= ~_BV(USB_TXE_PIN);
   USB_TXE_PORT |=  _BV(USB_TXE_PIN);

   USB_RXF_DDR  &= ~_BV(USB_RXF_PIN);
   USB_RXF_PORT |=  _BV(USB_RXF_PIN);

#endif // (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)
}

/**
 * @brief Checks if a new character received.
 *
 * This is only working if platform is Sensor Terminal Board or deRFnode.
 * 
 * @return  Returns '1' if a new character is available on the FTDI USB chip
 *
 */
uint8_t usb_keypressed(void)
{
#if (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)

   return (!(USB_RXF_PINS & _BV(USB_RXF_PIN))) ? 1 : 0;

#endif // (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)
   return 0;
}

/**
 * @brief Reads a character via USB.
 *
 * This function waits for character available on the USB chip and gives it back.
 * Blocking function.
 * This function is the standard I/O implementation which can be used for std. I/O facilities.
 *
 * This is only working if platform is Sensor Terminal Board or deRFnode.
 *
 * @param     dummy_file  not used
 * @return    character on the USB chip
 *
 */
int usb_getc_std(FILE* dummy_file)
{
#if (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)

   /* wait until new char received */
   while (USB_RXF_PINS & _BV(USB_RXF_PIN)){;}
   /* return the data */
   return(read_usb_byte());

#endif // (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)
   return 0;
}

/**
 * @brief Writes a character via USB.
 *
 * This function waits for empty buffer and then writes a character to USB.
 * This function is the standard I/O implementation which can be used for std. I/O facilities.
 *
 * This is only working if platform is Sensor Terminal Board or deRFnode.
 *
 * @param     c           the character to write
 * @param     dummy_file  not used
 *
 * @return    char: character written to USB
 *
 */
int usb_putc_std(char c, FILE* dummy_file)
{
#if (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)

   /* Wait for empty transmit buffer */
   while (USB_TXE_PINS & _BV(USB_TXE_PIN)){;}
   /* write the byte into the USB FIFO */
   write_usb_byte(c);
   /* return the char */
   return(c);

#endif // (PLATFORM == SENS_TERM_BOARD) || (PLATFORM == DE_RF_NODE)
   return 0;
}

/**
 * @brief Helper for usb byte input.
 *
 * This function reads a byte from USB interface.
 * This is only working if platform is Sensor Terminal Board or deRFnode.
 *
 * @return    the character read from USB
 *
 */
uint8_t read_usb_byte(void)
{
#if (PLATFORM == SENS_TERM_BOARD)
#ifdef __AVR_ATmega128RFA1__
   uint8_t data = 0;
   /* prepare data -> all bits are inputs */
   USB_DATA_DDR = 0x00;
   /* prepare chip select for usb */
   USB_DECODE_PORT &= ~_BV(USB_CS0_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS0_PIN);
   USB_DECODE_PORT &= ~_BV(USB_CS1_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS1_PIN);

   /* generate a single low / hi edge RD */
   USB_RD_PORT |=  _BV(USB_RD_PIN);
   USB_RD_DDR  |=  _BV(USB_RD_PIN);
   USB_RD_PORT &= ~_BV(USB_RD_PIN);
   /* give chip a chance to put out the data */
   _delay_us(1);
   /* save data */
   data = USB_DATA_PINS;
   /* and now back ... */
   USB_RD_PORT |= _BV(USB_RD_PIN);

   /* disable chip select */
   USB_DECODE_PORT &= ~_BV(USB_CS0_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS0_PIN);
   USB_DECODE_PORT &= ~_BV(USB_CS1_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS1_PIN);
   return(data);
#endif

#ifdef __AVR_ATmega1281__
   return *pUSB;
#endif

#elif (PLATFORM == DE_RF_NODE)

   uint8_t data = 0;

   /* set Data Directions to input */
   USB_D0_DDR &= ~_BV(USB_D0_PIN);
   USB_D1_DDR &= ~_BV(USB_D1_PIN);
   USB_D2_DDR &= ~_BV(USB_D2_PIN);
   USB_D3_DDR &= ~_BV(USB_D3_PIN);
   USB_D4_DDR &= ~_BV(USB_D4_PIN);
   USB_D5_DDR &= ~_BV(USB_D5_PIN);
   USB_D6_DDR &= ~_BV(USB_D6_PIN);
   USB_D7_DDR &= ~_BV(USB_D7_PIN);

   /*
    * RD and WR are low active
    */
   /* make sure WR pin is high, to enable correct latch direction */
   USB_WR_PORT |= _BV(USB_WR_PIN);
   USB_WR_DDR  |= _BV(USB_WR_PIN);

   /* set RD pin active -> low */
   USB_RD_PORT &= ~_BV(USB_RD_PIN);
   USB_RD_DDR  |=  _BV(USB_RD_PIN);

   /* give chip a chance to put out the data */
   _delay_us(5);

   /* read data */
   if(USB_D0_PINS & _BV(USB_D0_PIN)){ data |= _BV(USB_D0_BIT);} else{ data &= ~_BV(USB_D0_BIT);}
   if(USB_D1_PINS & _BV(USB_D1_PIN)){ data |= _BV(USB_D1_BIT);} else{ data &= ~_BV(USB_D1_BIT);}
   if(USB_D2_PINS & _BV(USB_D2_PIN)){ data |= _BV(USB_D2_BIT);} else{ data &= ~_BV(USB_D2_BIT);}
   if(USB_D3_PINS & _BV(USB_D3_PIN)){ data |= _BV(USB_D3_BIT);} else{ data &= ~_BV(USB_D3_BIT);}
   if(USB_D4_PINS & _BV(USB_D4_PIN)){ data |= _BV(USB_D4_BIT);} else{ data &= ~_BV(USB_D4_BIT);}
   if(USB_D5_PINS & _BV(USB_D5_PIN)){ data |= _BV(USB_D5_BIT);} else{ data &= ~_BV(USB_D5_BIT);}
   if(USB_D6_PINS & _BV(USB_D6_PIN)){ data |= _BV(USB_D6_BIT);} else{ data &= ~_BV(USB_D6_BIT);}
   if(USB_D7_PINS & _BV(USB_D7_PIN)){ data |= _BV(USB_D7_BIT);} else{ data &= ~_BV(USB_D7_BIT);}

   /* set RD pin inactive -> high */
   USB_RD_PORT |= _BV(USB_RD_PIN);

   return data;

#endif // defined (PLATFORM == SENS_TERM_BOARD)

   return 0;
}


/**
 * @brief Helper for usb byte output.
 *
 * This function writes a byte to USB interface.
 * This is only working if platform is Sensor Terminal Board or deRFnode.
 *
 * @param   data  data which should be written
 *
 */
void write_usb_byte(uint8_t data)
{
#if (PLATFORM == SENS_TERM_BOARD)

#ifdef __AVR_ATmega128RFA1__

   /* prepare data */
   USB_DATA_PORT = data;
   /* all bits are outputs */
   USB_DATA_DDR = 0xff;

   /* prepare chip select for usb */
   USB_DECODE_PORT &= ~_BV(USB_CS0_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS0_PIN);
   USB_DECODE_PORT &= ~_BV(USB_CS1_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS1_PIN);

   /* generate a single low / hi edge WR */
   USB_WR_PORT |= _BV(USB_WR_PIN);
   USB_WR_DDR  |= _BV(USB_WR_PIN);
   /* and now back ... */
   USB_WR_PORT &= ~_BV(USB_WR_PIN);
   USB_WR_PORT |=  _BV(USB_WR_PIN);

   /* disable chip select */
   USB_DECODE_PORT &= ~_BV(USB_CS0_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS0_PIN);
   USB_DECODE_PORT &= ~_BV(USB_CS1_PIN);
   USB_DECODE_DDR  |=  _BV(USB_CS1_PIN);
#endif

#ifdef __AVR_ATmega1281__
   *pUSB = data;
#endif

#elif (PLATFORM == DE_RF_NODE)

   /* set data pins, depending on character */
   if(data & _BV(USB_D0_BIT)){ USB_D0_PORT |= _BV(USB_D0_PIN);} else{ USB_D0_PORT &= ~_BV(USB_D0_PIN);}
   if(data & _BV(USB_D1_BIT)){ USB_D1_PORT |= _BV(USB_D1_PIN);} else{ USB_D1_PORT &= ~_BV(USB_D1_PIN);}
   if(data & _BV(USB_D2_BIT)){ USB_D2_PORT |= _BV(USB_D2_PIN);} else{ USB_D2_PORT &= ~_BV(USB_D2_PIN);}
   if(data & _BV(USB_D3_BIT)){ USB_D3_PORT |= _BV(USB_D3_PIN);} else{ USB_D3_PORT &= ~_BV(USB_D3_PIN);}
   if(data & _BV(USB_D4_BIT)){ USB_D4_PORT |= _BV(USB_D4_PIN);} else{ USB_D4_PORT &= ~_BV(USB_D4_PIN);}
   if(data & _BV(USB_D5_BIT)){ USB_D5_PORT |= _BV(USB_D5_PIN);} else{ USB_D5_PORT &= ~_BV(USB_D5_PIN);}
   if(data & _BV(USB_D6_BIT)){ USB_D6_PORT |= _BV(USB_D6_PIN);} else{ USB_D6_PORT &= ~_BV(USB_D6_PIN);}
   if(data & _BV(USB_D7_BIT)){ USB_D7_PORT |= _BV(USB_D7_PIN);} else{ USB_D7_PORT &= ~_BV(USB_D7_PIN);}

   /* set Data Directions to output */
   USB_D0_DDR |= _BV(USB_D0_PIN);
   USB_D1_DDR |= _BV(USB_D1_PIN);
   USB_D2_DDR |= _BV(USB_D2_PIN);
   USB_D3_DDR |= _BV(USB_D3_PIN);
   USB_D4_DDR |= _BV(USB_D4_PIN);
   USB_D5_DDR |= _BV(USB_D5_PIN);
   USB_D6_DDR |= _BV(USB_D6_PIN);
   USB_D7_DDR |= _BV(USB_D7_PIN);

   /*
    * RD and WR are low active
    */
   /* make sure RD pin is inactive -> high */
   USB_RD_PORT |= _BV(USB_RD_PIN);
   USB_RD_DDR  |= _BV(USB_RD_PIN);

   /* set WR pin active -> low */
   USB_WR_PORT &= ~_BV(USB_WR_PIN);
   USB_WR_DDR  |=  _BV(USB_WR_PIN);

   /* set WR pin inactive -> high */
   USB_WR_PORT |= _BV(USB_WR_PIN);

   /* --> data has be written */

#endif // defined (PLATFORM == SENS_TERM_BOARD)
}



