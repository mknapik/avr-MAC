/**
 * @file io_access.c
 *
 * @brief  Implementation of native IO access (LED's and Button(s)) for different platforms.
 *
 * Low Level functions for I/O access on:
 *    - Sensor Terminal Board
 *    - Radio Controller Board / deRFtoRCBAdapter
 *    - deRFnode
 *
 * $Id: io_access.c,v 1.1.4.1 2010/09/07 17:39:33 dam Exp $
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
#include <stdbool.h>          // include bool definition
#include <avr/io.h>           // include io definitions
#include <avr/interrupt.h>    // include interrupt definitions
#include <util/delay.h>       // include delay definitions

#include "config.h"
#include "io_access.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

/* status register for actual LED's state */
static uint8_t status_led = 0;

/* pointer to external memory location where I/O's are located (only on ATmega1281) */
#if defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)
static volatile uint8_t* pPERIPHERAL = (uint8_t *) PERIPHERAL_XRAM_ADDRESS;
#endif //ATMEGA1281

/* === PROTOTYPES ========================================================== */

uint8_t read_io_byte(void);
void write_io_byte(uint8_t data);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Control LED status.
 *
 * All LED's are 'LOW ACTIVE' (RCB, STB and deRFnode).
 *
 * @param   led_nr   number of LED
 * @param   led_mode mode which should be set (LED_ON, LED_OFF, LED_TOGGLE)
 *
 */
void led_set(led_number_t led_nr, led_mode_t led_mode)
{  
   switch (led_mode)
   {
   case LED_ON:
      status_led &= ~_BV(led_nr);
      break;
   case LED_OFF:
      status_led |= _BV(led_nr);
      break;
   case LED_TOGGLE:
      status_led ^= _BV(led_nr);
      break;
   default:
      break; // do nothing
   }

#if (PLATFORM == RADIO_CONTROLLER_BOARD)

   if(status_led & _BV(LED_0)) {LED_0_PORT |= _BV(LED_0_PIN);} else{LED_0_PORT &= ~_BV(LED_0_PIN);}
   LED_0_DDR  |=  _BV(LED_0_PIN);

   if(status_led & _BV(LED_1)) {LED_1_PORT |= _BV(LED_1_PIN);} else{LED_1_PORT &= ~_BV(LED_1_PIN);}
   LED_1_DDR  |=  _BV(LED_1_PIN);

   if(status_led & _BV(LED_2)) {LED_2_PORT |= _BV(LED_2_PIN);} else{LED_2_PORT &= ~_BV(LED_2_PIN);}
   LED_2_DDR  |=  _BV(LED_2_PIN);

#endif // RADIO_CONTROLLER_BOARD

#if (PLATFORM == SENS_TERM_BOARD)

   write_io_byte(status_led);

#endif //(PLATFORM == SENS_TERM_BOARD)

#if (PLATFORM == DE_RF_NODE)

   if(status_led & _BV(LED_0)) {LED_0_PORT |= _BV(LED_0_PIN);} else{LED_0_PORT &= ~_BV(LED_0_PIN);}
   LED_0_DDR  |=  _BV(LED_0_PIN);

   if(status_led & _BV(LED_1)) {LED_1_PORT |= _BV(LED_1_PIN);} else{LED_1_PORT &= ~_BV(LED_1_PIN);}
   LED_1_DDR  |=  _BV(LED_1_PIN);

   if(status_led & _BV(LED_2)) {LED_2_PORT |= _BV(LED_2_PIN);} else{LED_2_PORT &= ~_BV(LED_2_PIN);}
   LED_2_DDR  |=  _BV(LED_2_PIN);

#endif // (PLATFORM == DE_RF_NODE)
}


/**
 * @brief LED initialization function.
 *
 * This function initializes the led hardware.
 *
 */
void led_init(void)
{
#if (PLATFORM == RADIO_CONTROLLER_BOARD)

   led_set(LED_0, LED_OFF);
   led_set(LED_1, LED_OFF);
   led_set(LED_2, LED_OFF);

#endif // (PLATFORM == RADIO_CONTROLLER_BOARD)

#if (PLATFORM == SENS_TERM_BOARD)

   led_set(LED_0, LED_OFF);
   led_set(LED_1, LED_OFF);

#endif // (PLATFORM == SENS_TERM_BOARD)

#if (PLATFORM == DE_RF_NODE)

   led_set(LED_0, LED_OFF);
   led_set(LED_1, LED_OFF);
   led_set(LED_2, LED_OFF);

#endif // (PLATFORM == DE_RF_NODE)
}

/**
 * @brief Initialization function for IO hardware.
 *
 * This function initializes the hardware.
 *
 */
void io_init(void)
{
   /* make sure that external memory is enabled - only on ATmega1281 */
#if defined(__AVR_ATmega1281__)
   XMCRA |= _BV(SRE);
   XMCRB = _BV(XMBK);
#endif
   /* initialize LED's */
   led_init();
}

/**
 * @brief Gives back the button's state
 *
 * IMPORTANT: Button(s) are 'LOW ACTIVE' (on RCB, STB and deRFnode)
 *
 * @param button_nr  button to select
 *
 * @return button state - BUTTON_PRESSED, BUTTON_RELEASED
 *
 */
button_state_t button_pressed(button_number_t button_nr)
{
#if (PLATFORM == RADIO_CONTROLLER_BOARD)

   /* make compiler happy --> since there is only one button */
   button_nr = button_nr;

   /* Switch port to input. */
   BUTTON_0_PORT |=  _BV(BUTTON_0_PIN);
   BUTTON_0_DDR  &= ~_BV(BUTTON_0_PIN);

   return ((BUTTON_0_PINS & (1 << BUTTON_0_PIN)) ? BUTTON_RELEASED : BUTTON_PRESSED);

#endif // (PLATFORM == RADIO_CONTROLLER_BOARD)

#if (PLATFORM == SENS_TERM_BOARD)

   /* make compiler happy --> since there is only one button */
   button_nr = button_nr;

   return ((read_io_byte() & (1 << BUTTON_0_PIN)) ? BUTTON_PRESSED : BUTTON_RELEASED);

#endif // (PLATFORM == SENS_TERM_BOARD)

#if (PLATFORM == DE_RF_NODE)

   switch(button_nr)
   {
   case BUTTON_0:
      break;
   case BUTTON_1:
      BUTTON_1_DDR  &= ~_BV(BUTTON_1_PIN);
      BUTTON_1_PORT |=  _BV(BUTTON_1_PIN);
      return (BUTTON_1_PINS & _BV(BUTTON_1_PIN)) ? BUTTON_RELEASED : BUTTON_PRESSED;
      break;
   }

#endif // (PLATFORM == DE_RF_NODE)

   return BUTTON_RELEASED;
}

/**
 * @brief Helper for io byte input.
 *
 * This function reads a byte from memory mapped IO (ATmega1281)
 * or directly via port pins (ATmega128RFA1)
 *
 * @return    data that has been read back
 *
 */
uint8_t read_io_byte(void)
{
#if (PLATFORM == SENS_TERM_BOARD)

#ifdef __AVR_ATmega128RFA1__

   uint8_t data = 0;
   IO_DATA_DDR  = 0x00;

   /* prepare chip select for IO */
   IO_DECODE_PORT |=  _BV(IO_CS0_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS0_PIN);
   IO_DECODE_PORT &= ~_BV(IO_CS1_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS1_PIN);

   /* generate a single low / hi edge RD */
   IO_RD_PORT |=  _BV(IO_RD_PIN);
   IO_RD_DDR  |=  _BV(IO_RD_PIN);
   IO_RD_PORT &= ~_BV(IO_RD_PIN);
   /* and now back ... */
   IO_RD_PORT |=  _BV(IO_RD_PIN);

   /* read back data */
   data = IO_DATA_PINS;

   /* disable chip select */
   IO_DECODE_PORT &= ~_BV(IO_CS0_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS0_PIN);
   IO_DECODE_PORT &= ~_BV(IO_CS1_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS1_PIN);
   return(data);
#endif

#ifdef __AVR_ATmega1281__
   return *pPERIPHERAL;
#endif // (MCU == ATMEGA1281)

#endif //SENS_TERM_BOARD

   return 0;
}

/**
 * @brief Helper for io byte output.
 *
 * This function writes a byte to memory mapped IO (ATmega1281)
 * or directly via port pins (ATmega128RFA1)
 *
 * @param data data which should be written
 *
 */
void write_io_byte(uint8_t data)
{
#if (PLATFORM == SENS_TERM_BOARD)

#ifdef __AVR_ATmega128RFA1__
   /* prepare data --> corresponding bits are outputs */
   IO_DATA_PORT = data;
   IO_DATA_DDR = 0xFF;

   /* prepare chip select for IO */
   IO_DECODE_PORT |=  _BV(IO_CS0_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS0_PIN);
   IO_DECODE_PORT &= ~_BV(IO_CS1_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS1_PIN);

   /* generate a single low / hi edge WR */
   IO_WR_PORT |=  _BV(IO_WR_PIN);
   IO_WR_DDR  |=  _BV(IO_WR_PIN);
   IO_WR_PORT &= ~_BV(IO_WR_PIN);
   /* and now back ... */
   IO_WR_PORT |=  _BV(IO_WR_PIN);

   /* disable chip select */
   IO_DECODE_PORT &= ~_BV(IO_CS0_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS0_PIN);
   IO_DECODE_PORT &= ~_BV(IO_CS1_PIN);
   IO_DECODE_DDR  |=  _BV(IO_CS1_PIN);
#endif

#ifdef __AVR_ATmega1281__

   *pPERIPHERAL = data;

#endif // (MCU == ATMEGA1281)

#endif //(PLATFORM == SENS_TERM_BOARD)
}

/* EOF */


