/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFnative_Temp deRFnative_Temp
 * @{
 *
 * @brief  Implementation of the Native Demo Application "deRFnative_Temp"
 *
 * There is no use of any stack layer nor API.
 *
 * The native Example "deRFnative_Temp" demonstrates the native access to
 * temperature sensor either on MCU or platform. The following platforms supported:
 *    - Sensor Terminal Bord
 *
 * If you want use temperature sensor which is provides on deRFnode platform,
 * please see example application 'deRFnative_Sensors'.
 *
 * The example code reads out the value from temperature sensor
 * and print the result to terminal. Any time the button is pressed
 * the actual temperature is printed out again.
 * To start the test, open Hyperterminal (or any other terminal)
 * and connect to the virtual COM port that is assigned to your
 * platform (FTDI serial port).
 *
 * Not every MCU combination use the same temperature sensor.
 *
 *    -# Sensor Terminal Board + ATmega1281:
 *       - temperature sensor on Sensor Terminal Board
 *    -# Sensor Terminal Board + ATmega128RFA1:
 *       - internal temperature sensor from ATmega128RFA1
 *
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
#include <stdbool.h>                             // include bool definition
#include <stdio.h>                               // include standard io definitions
#include <avr/io.h>                              // include io definitions
#include <avr/interrupt.h>                       // include interrupt definitions

#include "io_access.h"
#include "usb.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */
#if (PLATFORM != SENS_TERM_BOARD)
#error "PLATFORM not supported - use example only on Sensor Terminal Board"
#else

#if defined(__AVR_ATmega128RFA1__)
/* use internal temperature sensor from Single Chip */
#define INTERNAL_TEMP
#endif // defined(__AVR_ATmega128RFA1__)

#if defined(__AVR_ATmega1281__)
/* use temperature measurement from STB, this MCU also do not have internal temperature sensor */
#define EXTERNAL_TEMP
/* location of external memory address space for temperature sensor */
#define IO_THERM_AD  (0x4000)
/* pointer to memory mapped address */
static volatile unsigned char* pIO_THERM = (unsigned char *) IO_THERM_AD;
#endif // defined(__AVR_ATmega1281__)

/* Setup a File stream with putchar() and getchar() functionality over USB */
FILE usb_stream = FDEV_SETUP_STREAM(usb_putc_std, usb_getc_std,_FDEV_SETUP_RW);

/* number of button press event to count -> for debouncing use */
#define KEY_UP_COUNT          (15)

/* structure that contains temperature values */
typedef struct
{
   uint8_t sign;
   uint8_t integralDigit;
   uint8_t fractionalDigit;
}temperature_t;

/* generate instance of temperature_t structure */
temperature_t temperature;

#endif // (PLATFORM != SENS_TERM_BOARD)

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

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
 * @brief Initialize temperature hardware (ADC) of ATmega128RFA1 or
 * ATmega1281 (use temperature sensor from Sensor Terminal Board)
 *
 * ATmega128RFA1 uses the internal 1,6V voltage reference and
 * ATmega1281 use the 3,3V system power reference 
 *
 */
void app_init_temp(void) 
{
#ifdef EXTERNAL_TEMP
   PRR0 &= ~(1 << PRADC);                               // power up ADC
   ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // divider 128 on
   ADCSRB = 0x00;                                       // not the free running mode
   DIDR0 = _BV(1) | _BV(0);                             // disable digital input buffers
   ACSR = (1 << ACD);                                   // disable analog comparator
   ADMUX = (1<<REFS0) | (1<<MUX1) | (1<<MUX0);          // reference is 3,3V VCC and ADC3 is selected
   ADCSRA = (1<<ADEN);                                  // enable ADC
#endif  

#ifdef INTERNAL_TEMP
   /*
    * ADMUX order is important
    * -> first set MUX5:0, than MUX5 on ADCSRB and after that Bits REFS0:1 on ADMUX
    */
   ADCSRA |= (1<<ADEN) | (1 << ADPS2) | (1 << ADPS1); // divider 64 (250 kHz clock frequency)
   DIDR0 = _BV(1) | _BV(0);                           // disable digital input buffers
   ADMUX = (1 << MUX3) | (1 << MUX0);                 // preselect temperatur sensor
   ADCSRB |= (1 << MUX5);                             // select temperature sensor
   ADMUX |= _BV(REFS1) | _BV(REFS0);                  // reference is 1.6V intern
#endif   
}

/**
 * @brief Returns the measured sensor temperature in degree.
 *
 * If INTERNAL_TEMP is defined the MCU internal temperature sensor is used (only on ATmega128RFA1 available)
 * If EXTERNAL_TEMP is defined the Sensor Terminal Board temperature sensor is used (only on ATmmega1281)

 * The EXTERNAL_TEMP uses a table with precalculated voltage measures and does linear
 * interpolation between two sampling points.
 *
 * @param temperature   pointer to temperature_t structure --> include integral and fractional part of temperature
 *
 */
void app_read_temp(temperature_t* temperature)
{
#ifdef INTERNAL_TEMP
   /* start ADC Conversion */
   ADCSRA |= (1<<ADSC);
   /* wait until conversion is completed */
   while ( ADCSRA & (1<<ADSC) ) {;}

   /* calculate temperature (see datasheet for further details) */
   int32_t temp = (113 * ADC) - 27280;
   /* set integral and fractional part */
   if(temp < 0) {temperature->sign = 1;} else{temperature->sign = 0;}
   temperature->integralDigit = temp / 100;
   temperature->fractionalDigit = temp % 100;
#endif

#ifdef EXTERNAL_TEMP
   /* make sure the external Memory Interface is enabled */
   XMCRA |= _BV(SRE);
   XMCRB = _BV(XMBK);
   /* Thermistor On, LEDs off */
   *pIO_THERM = 0x03;

   /* table with precalculated voltages */
   static struct _ttable
   {
      int32_t temp; int32_t millivolts;
   } ttable[] =
         {
               { -40000 ,  86000   } ,
               { -35000 ,  120000  } ,
               { -30000 ,  164000  } ,
               { -25000 ,  221000  } ,
               { -20000 ,  292000  } ,
               { -15000 ,  380000  } ,
               { -10000 ,  487000  } ,
               { -5000  ,  612000  } ,
               { 0      ,  756000  } ,
               { 5000   ,  916000  } ,
               { 10000  ,  1091000 } ,
               { 15000  ,  1274000 } ,
               { 20000  ,  1462000 } ,
               { 25000  ,  1650000 } ,
               { 30000  ,  1832000 } ,
               { 35000  ,  2006000 } ,
               { 40000  ,  2166000 } ,
               { 45000  ,  2313000 } ,
               { 50000  ,  2445000 } ,
               { 55000  ,  2562000 } ,
               { 60000  ,  2665000 } ,
               { 65000  ,  2754000 } ,
               { 70000  ,  2830000 } ,
               { 75000  ,  2897000 } ,
         };

   static uint32_t temp;
   uint8_t i;
   uint32_t volt;

   /* start ADC Conversion */
   ADCSRA |= (1<<ADSC);
   /* wait until conversion is completed */
   while ( ADCSRA & (1<<ADSC) ) {;}

   /* read ADC RAW value */
   volt  = ADC;
   /* 3300mV at 1024 steps are 3223mV per step */
   volt  = (3223UL*volt);

   /*
    * We use 32bit integer values for calculation and precalculated voltage table
    * By doing this we prevent the use of 'float' values.
    * This minimizes the footprint a lot.
    */

   for (i = 1; i < (sizeof(ttable)/sizeof(ttable[0])-1); i++)
   {
      if (volt < ttable[i].millivolts)  // look for a fitting table entry
      {
         i--;
         // linear Interpolation
         temp = ttable[i].temp                // linear Interpolation
               + 5000 * (volt - ttable[i].millivolts)
               / (ttable[i+1].millivolts - ttable[i].millivolts);
         break;
      }
   }
   /* check if temperature is positive or negative */
   if(temp < 0) {temperature->sign = 1;} else{temperature->sign = 0;}
   /* get integral part */
   temperature->integralDigit = temp / 1000;
   /* get fractional part (divide by 10 because only last two digits are interesting) */
   temperature->fractionalDigit = (temp % 1000) / 10;
#endif
}

/**
 * @brief Application initialization function.
 *
 * This function initializes
 *  - the LED's
 *  - temperature sensor
 *  - USB Interface
 *
 */
void app_init(void)
{
   /* initialize hardware */
   io_init();
   /* initialize USB */
   usb_init();
   /* initialize temperature sensor */
   app_init_temp();

   /* redirect standard input/output to USB stream -> now we can use printf/scanf over USB */
   stdout = &usb_stream;
   stdin  = &usb_stream;
}

/**
 * @brief Application task function.
 *
 * This function checks the state of the button. If an button event recognized
 * the actual temperature is measured and printed out.
 *
 */
void app_task(void)
{
   /* is button pressed? */
   if(BUTTON_PRESSED == app_debounce_button(BUTTON_0))
   {
      /* wait until button released */
      while(BUTTON_PRESSED == button_pressed(BUTTON_0)){;}

      /* toggle LED */
      led_set(LED_0, LED_TOGGLE);

      /* read temperature and print out */
      app_read_temp(&temperature);
      printf("Temperature: %c%d.%02d °C\n",(temperature.sign ? '-' : '+'), temperature.integralDigit, temperature.fractionalDigit);
   }
}

/**
 * @brief Main function of the Native Demo Application "deRFnative_Temp".
 *
 */
int main(void)
{
   /* initialize USB, IO's, temperature sensors */
   app_init();

   printf("-- Press board button to get actual temperature --\r\n");

   /* start application */
   while(1)
   {
      app_task();
   }
}

/** @} */
/** @} */

/* EOF */
