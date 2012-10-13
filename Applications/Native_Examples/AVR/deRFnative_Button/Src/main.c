/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFnative_Button deRFnative_Button
 * @{
 *
 * @brief  Implementation of the native Demo Application "deRFnative_Button".
 *
 * There is no use of any stack layer nor API.
 *
 * The native Example "deRFnative_Button" demonstrates the native access
 * to IO resources (here Button and LED's) of the following platforms:
 *    - Radio Controller Board
 *    - Sensor Terminal Board
 *    - deRFnode
 *
 * The example code changes the state of the LED's if the button is pressed.
 * Button state is scanned from the timer interrupt and debounced
 * by testing many times for the new state.
 *
 * $Id: main.c,v 1.1.6.1 2010/09/07 17:39:30 dam Exp $
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

#include "io_access.h"        // include definitions to get access to IO resources

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

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
}

/**
 * @brief Timer initialization function.
 *
 * This function initializes timer 1 for one millisecond interrupting.
 * This is done via compare mode with CTC enabled (Clear Timer on Compare match).
 *
 */
void app_timer_init(void) 
{
   /* set clock source to prescaler F_CPU/256 + enable CTC (Clear Timer on Compare match) */
   TCCR1B |= _BV(CS12) | _BV(WGM12);
   /* timer interrupt every 1 millisecond */
   OCR1A = (F_CPU / 256) / 1000;
   /* clear capture flag */
   TIFR1  |= (1 << OCF1A);
   /* Enable timer interrupt */
   TIMSK1 |= (1 << OCIE1A);
}

/**
 * @brief Main function of the native Demo Application "deRFnative_Button".
 *
 */
int main(void)
{
   /* initialize hardware */
   app_init();
   /* initialize timer (interrupt every 1ms with capture compare mode and clear on match) */
   app_timer_init();
   /* enable global interrupt */
   sei();
   /* switch on one LED to have LED_0 on, LED_1 off and vice versa */
   led_set(LED_0, LED_ON);
   /* empty infinite loop, since the system is interrupt controlled */
   while(1) {;}
}

/**
 * @brief Timer 1 capture and compare interrupt routine
 *
 * Timer interrupts every 1 millisecond with a compare event.
 * This event also clear timer 1 to zero via CTC mode.
 * The compare counter gets reseted and every 10 milliseconds
 * the button state is scanned for changes. If true it debounces
 * the button for 50ms. If debounced the active LED is changed.
 *
 *
*/
ISR(TIMER1_COMPA_vect)
{
   /* counter for button debounce */
   static uint8_t button_cnt = 0;
   /* memory for button state */
   static bool button_state = false;
   /* counter for milliseconds */
   static uint16_t ms_cnt = 0;

   /* 10ms gone? */
   if(ms_cnt++ >= 10)
   {
      /* reset milliseconds counter */
      ms_cnt = 0;
      if(button_state)
      {
         /* is button released */
         if(!button_pressed(BUTTON_0))
         {
            /* is release time greater than 50ms? */
            if(5 <= ++button_cnt)
            {
               button_state = false;
            }
         }
         /* is button debounced? - if true reset counter */
         else
         {
            button_cnt = 0;
         }
      }
      else
      {
         /* check if button pressed */
         if(button_pressed(BUTTON_0))
         {
            /* yes, button is pressed */
            button_state = true;
            /* toggle both LED's */
            led_set(LED_0, LED_TOGGLE);
            led_set(LED_1, LED_TOGGLE);
         }
      }
   }
}

/** @}*/
/** @}*/

/* EOF */


