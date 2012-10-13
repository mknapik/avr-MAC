/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFnative_USB deRFnative_USB
 * @{
 *
 * @brief  Implementation of the native Demo Application "deRFnative_USB".
 *
 * There is no use of any stack layer nor API.
 *
 * The native Example "deRFnative_USB" demonstrates the native access
 * to USB resources on the following platforms:
 *    - Sensor Terminal Board
 *    - deRFnode
 *
 * The example code reads and writes bytes from USB interface.
 * To start the test, open Hyperterminal (or any other terminal)
 * and connect to the virtual COM port that is assigned to your
 * platform (FTDI serial port).
 * The examples codes waits for any character received via USB and answers 
 * by writing "Hello World: 'x'" to USB. Where 'x' is the character received from USB.
 *
 * Note: The USB handling is done by usb.c
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
#include <stdbool.h>                             // include bool definition
#include <avr/io.h>                              // include io definitions
#include <stdio.h>                               // include standard io definitions

#include "usb.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ============================================================= */

/* Setup a File stream with putchar() and getchar() functionality over USB */
FILE usb_stream = FDEV_SETUP_STREAM(usb_putc_std, usb_getc_std,_FDEV_SETUP_RW);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief IO initialization function.
 *
 * This function initializes the IO hardware
 *
 */
void app_init(void)
{
   /* initialize USB */
   usb_init();

   /* redirect standard input/output to USB stream -> now we can use printf/scanf over USB */
   stdout = &usb_stream;
   stdin  = &usb_stream;
}

/**
 * @brief Main function of the native Demo Application "deRFnative_USB".
 *
 */
int main(void)
{
    char c = ' ';

    /* initialize hardware */
   app_init();

    while(1)
    {
       /* wait for a key press */
        c = getchar();
        /* print the typed key with message */
        printf("Hello World:%c\r\n", c);
    }
}

/** @}*/
/** @}*/

/* EOF */


