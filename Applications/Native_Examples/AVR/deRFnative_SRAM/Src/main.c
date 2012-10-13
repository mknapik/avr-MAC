/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFnative_SRAM deRFnative_SRAM
 * @{
 *
 * @brief  Implementation of the native Demo Application "deRFnative_SRAM".
 *
 * There is no use of any stack layer nor API.
 *
 * The native Example "deRFnative_SRAM" demonstrates the native access
 * to the SRAM of the following platform:
 *    - Sensor Terminal Board
 *
 * The example code simply write and read a test pattern to/from SRAM.
 * To start the test, open Hyperterminal (or any other terminal)
 * and connect to the virtual COM port that is assigned to your
 * platform (FTDI serial port).
 *
 * With the button on SensTermBoard you can start the test
 * and see the results on the terminal screen.
 *
 * $Id: main.c,v 1.1.6.1 2010/09/07 17:39:31 dam Exp $
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

#include <avr/io.h>                              // include io definitions
#include <avr/interrupt.h>                       // include interrupt definitions
#include <stdio.h>                               // include standard io definitions
#include <stdbool.h>                             // include bool definition
#include <stdlib.h>                              // include stdlib functions

#include "io_access.h"
#include "usb.h"
#include "sram.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

/** Setup a File stream with putchar() and getchar() functionality over USB */
FILE usb_stream = FDEV_SETUP_STREAM(usb_putc_std, usb_getc_std,_FDEV_SETUP_RW);

/** number of button press event to count -> for debouncing use */
#define KEY_UP_COUNT          (15)

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main initialization function.
 *
 * This function initializes the hardware
 *
 */
void app_init(void)
{
   /* initialize hardware */
   io_init();
   /* initialize USB */
   usb_init();
   /* initialize SRAM */
   sram_init();

   /* redirect standard input/output to USB stream -> now we can use printf/scanf over USB */
   stdout = &usb_stream;
   stdin  = &usb_stream;
}

/**
 * @brief Button debounce routine.
 *
 * Helper function for debouncing the button.
 *
 * @param  button_nr the button number
 *
 * @return the actual state of button
 *
 */
button_state_t app_debounce_button(button_number_t button_nr)
{
    static uint8_t keycnt;

    if(BUTTON_PRESSED == button_pressed(button_nr))
    {
        if (keycnt++ > KEY_UP_COUNT)
        {
            keycnt = 0;
            return BUTTON_PRESSED;
        }
    }
    else
    {
        keycnt = 0;
    }
    return BUTTON_RELEASED;
}

/**
 * @brief Error function, is called if SRAM test failed
 *
 */
void failure(uint16_t addr)
{
  printf(" -> error at address 0x%x\r\n", addr);
  printf(" close application\r\n");
  exit(0);
}

/**
 * @brief Main function of the native Demo Application "deRFnative_SRAM".
 *
 */
int main(void)
{
   /* initialize the hardware */
   app_init();

   uint16_t addr;
   uint8_t data;

   while(1)
   {
      printf("-------------------------\r\n");
      printf("SRAM Test\r\n");
      printf("   write sample data to available address space (%ukB)\r\n", (MAX_ADDRESS/1024));
      printf("   Written data is read out and checked, any error will be reported\r\n");
      printf("\r\n...press STB button to continue...\r\n");

      /* wait until button is pressed */
      while(BUTTON_RELEASED == app_debounce_button(BUTTON_0)){;}
      /* wait until button released */
      while(BUTTON_PRESSED == button_pressed(BUTTON_0)){;}

      /************************************************/
      /* SRAM TEST - Complete Available Address Space */
      /************************************************/

      printf("\r\nStart SRAM Test\r\n");

      /* clear SRAM */
      sram_clear();

      /* write sample data to SRAM */
      for(addr = 0; addr < MAX_ADDRESS; addr++)
      {
         sram_write(addr, (uint8_t)addr);
      }

      /* read and compare sample data from SRAM */
      for(addr = 0; addr < MAX_ADDRESS; addr++)
      {
         sram_read(addr, &data);
         if(data != (uint8_t)addr)
         {
            failure(addr);
         }
      }

      printf(" -> SRAM Test completed successfully\r\n");

      /**********************************************************/
      /* SRAM TEST - Write and Read string to specified address */
      /**********************************************************/

      uint8_t string[] = "That's the sample string";
      uint16_t address = 0x300;
      uint8_t c = 0;

      printf("\r\nNext test writes string '");
      for(c = 0; c < sizeof(string); c++)
      {
         printf("%c", string[c]);
      }
      printf("' to (0x%x).", address);
      printf("\r\n  Any error will be reported. On success the string is printed again.\r\n");
      printf("\r\n...press STB button to continue...\r\n");

      /* wait until button is pressed */
      while(BUTTON_RELEASED == app_debounce_button(BUTTON_0)){;}
      /* wait until button released */
      while(BUTTON_PRESSED == button_pressed(BUTTON_0)){;}

      /* write string to sram */
      for(c = 0; c < sizeof(string); c++)
      {
         sram_write(address, string[c]);
         address++;
      }

      /* revert address to original */
      address = 0x300;

      /* clear string array */
      for(c = 0; c < sizeof(string); c++)
      {
         string[c] = 0;
      }

      /* read from sram and save values to string array */
      for(c = 0; c < sizeof(string); c++)
      {
         sram_read(address, &string[c]);
         address++;
      }

      /* print out restored data */
      printf("\r\n-> restored text: '");
      for(c = 0; c < sizeof(string); c++)
      {
         printf("%c", string[c]);
      }
      printf("'\r\n");

      printf("\r\n...press STB button to start test again...\r\n");

      /* wait until button is pressed */
      while(BUTTON_RELEASED == app_debounce_button(BUTTON_0)){;}
      /* wait until button released */
      while(BUTTON_PRESSED == button_pressed(BUTTON_0)){;}
   }
}

/** @}*/
/** @}*/

/* EOF */


