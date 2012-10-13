/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFsimple_Button deRFsimple_Button
 * @{
 *
 * @brief  Implementation of the PAL Demo Application "deRFsimple_Button"
 *
 * SensTermBoard Example:
 * The PAL Example "deRFsimple_Button" demonstrates access to IO resources
 * (here Button and LED's) of the following platforms:
 *    - Radio Controller Board
 *    - Sensor Terminal Board
 *    - deRFnode
 *
 * This is done with help of PAL layer.
 *
 * The example code toggles LED_0 and LED_1 every time a button is pressed.
 *  
 * $Id: main.c,v 1.1.6.1 2010/09/07 17:39:33 dam Exp $
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pal.h"
#include "app_config.h"
#include "deRFsimple_Button.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the PAL Demo Application "deRFsimple_Button".
 *
 */
int main(void)
{
   /* initialize hardware */
   app_init();

   /* start application */
   while(1)
   {
      app_task();
   }
}

/**
 * @brief Button debounce routine.
 *
 * Helper function for debouncing the button.
 * @return 1 if a button event is detected, 0 otherwise.
 *
 */
uint8_t app_debounce_button(void) 
{
   static uint8_t keycnt;

   if(BUTTON_PRESSED == pal_button_read(BUTTON_0))
   {
      keycnt ++;
      if (keycnt > KEY_UP_COUNT)
      {
         keycnt = 0;
         return BUTTON_PRESSED;
      }
   }
   else
   {
      keycnt = 0;
   }
   return BUTTON_OFF;
}

/**
 * @brief Application initialization function.
 *
 * This function initializes
 *  - the PAL module including the IRQ handler registering,
 *  - the LED and Button interface.
 *
 */
void app_init(void)
{
   /* Step 0: initialize MCU peripherals */
   pal_init();
   pal_led_init();
   pal_button_init();

   /* Step 1: switch one LED on */
   pal_led(LED_0, LED_ON);
   pal_led(LED_1, LED_OFF);

   /* Step 2: ready to receive data. */
   //FIXME pal_global_irq_enable();
}

/**
 * @brief Application task function.
 *
 * This function checks the state of button and starts 
 * to toggle LED's if a button press event is detected.
 *
 * @par LED Access
 *
 */
void app_task(void) 
{
   if(BUTTON_PRESSED == app_debounce_button())
   {
      /* wait until button released */
      while(BUTTON_PRESSED == pal_button_read(BUTTON_0)){;}
      pal_led(LED_0, LED_TOGGLE);
      pal_led(LED_1, LED_TOGGLE);
   }
}

/** @}*/
/** @}*/

/* EOF */
