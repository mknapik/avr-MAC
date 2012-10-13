/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFnative_EEPROM_I2C deRFnative_EEPROM_I2C
 * @{
 *
 * @brief  Implementation of the native Demo Application "deRFnative_EEPROM_I2C".
 *
 * There is no use of any stack layer nor API.
 *
 * The native Example "deRFnative_EEPROM_I2C" demonstrates the native access
 * to EEPROM resources on the following platforms:
 *    - Sensor Terminal Board (ONLY with deRFmega128 Rev. 2)
 *    - deRFnode
 *
 * The example code simply write and read a test pattern to/from EEPROM.
 * To start the test, open Hyperterminal (or any other terminal)
 * and connect to the virtual COM port that is assigned to your
 * platform (FTDI serial port).
 * With the button you can start/restart the test and watch the
 * results on terminal screen.
 * 
 * There are 2 test scenarios:
 *    - only a string test (write string to EEPROM and read back)
 *      (this is sufficient to check proper working of EEPROM)
 *    - a test which write data to complete address space of EEPROM,
 *      read back data and check it
 *      --> latter test can be enabled by setting macro 'MAKE_LARGE_TEST' to '1'
 *
 * ------------------------------------------------------------------
 * THIS APPLICATION ONLY WORKS WITH deRFmega128 REVISION 2 AND HIGHER
 * ------------------------------------------------------------------
 *
 * $Id: main.c,v 1.1.6.1 2010/09/07 17:39:36 dam Exp $
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
#include <stdio.h>               // include standard io definitions
#include <stdbool.h>             // include bool definition
#include <stdlib.h>              // include stdlib functions
#include <string.h>              // include string functions (e.g. memset)
#include <avr/io.h>              // include io definitions

#include "io_access.h"           // include definitions to get access to IO resources
#include "usb.h"                 // include definitions to get access to USB
#include "eeprom.h"              // include definitions to get access to EEPROM

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/** macro for printf - with variable argument list */
#define PRINTF(...)           (printf(__VA_ARGS__))

/** select button which should be used as user input */
#define EEPROM_BUTTON         (BUTTON_0)

/** number of button press event to count -> for debouncing use */
#define KEY_UP_COUNT          (15)

/** it's possible to make a large test, which write to complete address space of EEPROM and checks it*/
#define MAKE_LARGE_TEST       (0)

/* === GLOBALS ============================================================= */

/** Setup a File stream with putchar() and getchar() functionality over USB */
FILE usb_stream = FDEV_SETUP_STREAM(usb_putc_std, usb_getc_std,_FDEV_SETUP_RW);

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief IO initialization function.
 *
 * This function initializes the IO hardware
 *
 */
void app_init(void)
{
   /* initialize the IO hardware */
   io_init();
   /* initialize the USB */
   usb_init();
   /* initialize EEPROM */
   eeprom_init();

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
 * @brief Error Routine.
 *
 * Toggle LED's periodically if an error occurs
 *
 **/
void error(void)
{
   /* set flashing time -> CPU frequency / (toggle/flashing time (in ms)) = tick time */
   /* --> this results into 2,5Hz */
   uint32_t FLASH_TICKS = F_CPU / 200;
   uint32_t counter = 0;
   PRINTF("\r\n[EEPROM] Error - Application stopped\r\n");
   while(1)
   {
      if(counter++ == FLASH_TICKS)
      {
         counter = 0;
         led_set(LED_0, LED_TOGGLE);
         led_set(LED_1, LED_TOGGLE);
      }
   }
}

/**
 * @brief Helper function to read from EEPROM.
 *
 * We use this function to easy enable and disable the EEPROM,
 * and check if read was successfully.
 *
 * @param address the EEPROM address where to read data from
 * @param len     the length of data
 * @param buf     pointer to data which should be read
 *
 */
void eeprom_read(uint32_t address, uint16_t len, uint8_t *buf)
{
   EEPROM_ENABLE();
   int32_t ret = eeprom_read_bytes(address, len, buf);
   EEPROM_DISABLE();
   if(ret <= 0) { error(); }
}

/**
 * @brief Helper function to write to EEPROM.
 *
 * We use this function to easy enable and disable the EEPROM,
 * and check if write access was successfully.
 *
 * @param address the EEPROM address where to write data
 * @param len     the length of data
 * @param buf     pointer to data which should be written
 *
 */
void eeprom_write(uint32_t address, uint16_t len, uint8_t *buf)
{
   EEPROM_ENABLE();
   int16_t ret = eeprom_write_bytes(address, len, buf);
   EEPROM_DISABLE();
   if(ret <= 0) { error(); }
}

/**
 * @brief Helper function to print out the content of an array.
 *
 * @param pArray  pointer to array
 * @param len     size of array
 *
 */
void print_array(uint8_t* pArray, uint8_t len)
{
   do
   {
      PRINTF("%c", *pArray++);
   } while (len-- > 0);
}

/**
 * @brief Function make test over complete address space of EEPROM.
 *
 * Therefore it write's test pattern to all pages on EEPROM memory,
 * read back and check if data is equal to pattern.
 *
 * All steps printed out.
 *
 */
void make_large_test(void)
{
#if MAKE_LARGE_TEST
   /* buffer of size PAGE_SIZE where test data is saved */
   static uint8_t buffer[PAGE_SIZE];
   /* EEPROM address counter */
   uint32_t cnt = 0;
   /* number of written and read pages */
   uint16_t number_pages = 0;
   /* temporary counter */
   uint16_t c = 0;

   PRINTF("\r\n[EEPROM] MEMORY TEST\r\n");
   PRINTF("\r\nThis test write's to complete address space of EEPROM, read back content and show result.\r\n");

   /* generate EEPROM data which should be written to each page */
   for (cnt = 0; cnt < sizeof(buffer); cnt++)
   {
      buffer[cnt] = cnt;
   }

   PRINTF("\r\n  Start write process\r\n");

   /* write data to all pages on EEPROM */
   for (cnt = 0; cnt <= LAST_PAGE_ADDRESS; cnt += PAGE_SIZE)
   {
      eeprom_write(cnt, PAGE_SIZE, (uint8_t*)buffer);
      PRINTF("    written pages: %d\r\n", ++number_pages);
   }
   PRINTF("\r\n  -> Write process successfully completed\r\n");


   PRINTF("\r\n  Start read process\r\n");

   /* delete content from buffer */
   memset(buffer, 0, sizeof(buffer));

   /* reset page counter */
   number_pages = 0;

   /* read from all pages of EEPROM */
   for (cnt = 0; cnt <= LAST_PAGE_ADDRESS; cnt += PAGE_SIZE)
   {
      eeprom_read(cnt, PAGE_SIZE, (uint8_t*)buffer);

      /* check EEPROM content */
      for (c = 0; c < sizeof(buffer); c++)
      {
         if(buffer[c] != c)
         {
            error();
         }
      }

      /* delete content from buffer */
      memset(buffer, 0, sizeof(buffer));

      PRINTF("    pages read: %d\r\n", ++number_pages);
   }

   PRINTF("\r\n  -> Read process successfully completed\r\n");

   /* process was successfully -> clear LED */
   led_set(LED_1, LED_OFF);

#endif // MAKE_LARGE_TEST
}

/**
 * @brief Function make a simple test of EEPROM.
 *
 * Therefore it write's a string to a defined address,
 * read back and print out this string.
 *
 * All steps printed out.
 *
 */
void make_string_test(void)
{
   uint8_t addr_string = 55;
   uint8_t string[] = "The quick brown fox jumps over the lazy dog.";

   PRINTF("\r\n[EEPROM] STRING TEST\r\n");
   PRINTF("\r\nThis test write a string to EEPROM, read it back and show result.\r\n");

   PRINTF("\r\n  String to write: '");
   print_array(&string[0], sizeof(string));
   PRINTF("'\r\n");

   /* write string into EEPROM */
   eeprom_write(addr_string, sizeof(string)-1, string);

   /* EEPROM was written successfully */
   PRINTF("    -> String successfully written\r\n");

   /* delete string array */
   memset(&string[0], 0, sizeof(string));

   /* read string back */
   eeprom_read(addr_string, sizeof(string)-1, (uint8_t*)string);

   /* read back was successfully -> clear LED */
   led_set(LED_1, LED_OFF);

   PRINTF("  String read back: '");
   print_array(&string[0], sizeof(string));
   PRINTF("'\r\n");
   PRINTF("    -> String successfully read back\r\n");
}

/**
 * @brief Main function of the native Demo Application "deRFnative_EEPROM_I2C".
 *
 * write and read data to/from EEPROM and show progress via LED, if error, all LED's flashes
 *
 */
int main(void)
{
   /* initialize hardware */
   app_init();

   /* start application */
   while(1)
   {
      /* switch on both LED's -> both should be off when test successful (one LED for one test) */
      led_set(LED_0, LED_ON);
      led_set(LED_1, LED_ON);

      PRINTF("[EEPROM] TEST APPLICATION\r\n");
      PRINTF(" Number of Pages: %u\r\n", (uint16_t)NUMBER_PAGES);
      PRINTF(" Page Size      : %u\r\n", (uint16_t)PAGE_SIZE);
      PRINTF("\r\n-> press board button to go on\r\n");

      /* wait until button pressed */
      while(BUTTON_RELEASED == app_debounce_button(EEPROM_BUTTON)){;}
      /* wait until button released */
      while(BUTTON_PRESSED == button_pressed(EEPROM_BUTTON)){;}

#if MAKE_LARGE_TEST
      /* make large EEPROM test (write to complete address space) */
      make_large_test();
      PRINTF("\r\n----------------------------------------------------\r\n");
#endif

      /* write a single string to EEPROM and read back */
      make_string_test();

      PRINTF("\r\n-- Press board button to start test again --\r\n");

      /* wait until button pressed */
      while(BUTTON_RELEASED == app_debounce_button(EEPROM_BUTTON)){;}
      /* wait until button released */
      while(BUTTON_PRESSED == button_pressed(EEPROM_BUTTON)){;}

      /* start test again */
   }
}

/** @}*/
/** @}*/

/* EOF */


