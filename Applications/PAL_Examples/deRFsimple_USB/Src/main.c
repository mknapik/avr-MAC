/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFsimple_USB deRFsimple_USB
 * @{
 *
 * @brief  Implementation of the Simple Demo Application "deRFsimple_USB".
 * 
 * The PAL Example "deRFsimple_USB" demonstrates the access to USB resources
 * on the following platforms:
 *    - Sensor Terminal Board
 *
 * This is done with help of PAL layer.
 *
 * The example code reads and writes bytes from USB interface.
 * To test the example connect any terminal program to the virtual COM port
 * assigned to FTDI platform chip (Sensor Terminal Board)
 * (baud rate 115kBit, 8 data bits, no parity, 1 stop bit).
 * The examples code waits for an arbitrary character on keyboard. 
 * If occurs, the actual temperature is read an send to USB interface.
 * 
 * If using ATmega128RFA1, internal temperature sensor is choosen. 
 * Else temperature sensor from SensTermBoard is selected.   
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sio_handler.h"
#include "pal.h"

#include "app_config.h"


/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

#if (PAL_GENERIC_TYPE == MEGA_RF && PAL_TYPE == ATMEGA128RFA1) 
#define INTERNAL_TEMP // use internal temperature measurement from Single Chip
#else
#define EXTERNAL_TEMP // use temperature measurement from STB, other MCU's do not have internal temperature measurement
#define IO_THERM_AD  (0x4000)
// this is the pointer to the external memory address of the IO chip
static volatile unsigned char* pIO_THERM = (unsigned char *) IO_THERM_AD;
#endif

/** Number of necessarry consecutive "button pressed" detections, before a button event is signalled. */
#define KEY_UP_COUNT (15)

/* === GLOBALS ============================================================= */

/* structure that contains temperature values */
typedef struct
{
   uint8_t sign;
   uint8_t integralDigit;
   uint8_t fractionalDigit;
}temperature_t;

/* generate instance of temperature_t structure */
temperature_t temperature;

#ifdef EXTERNAL_TEMP

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
#endif

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Initialize temperature hardware
 *
 */
static void app_init_temp(void) 
{
#ifdef INTERNAL_TEMP 
   //ADMUX order is important -> first set MUX5:0, than MUX5 on ADCSRB and after that Bits REFS0:1 on ADMUX

   ADCSRA |= (1 << ADPS2) | (1 << ADPS1);             // divider 64 (250 kHz clock frequency)
   DIDR0 = _BV(1) | _BV(0);                           // disable digital input buffers
   ADMUX = (1 << MUX3) | (1 << MUX0);                 // preselect temperatur sensor
   ADCSRB |= (1 << MUX5);                             // select temperature sensor
   ADMUX |= _BV(REFS1) | _BV(REFS0);                  // reference is 1.6V intern
   ADCSRA |= (1<<ADEN);                               // enable ADC 
#endif
#ifdef EXTERNAL_TEMP 
   PRR0 &= ~(1 << PRADC);                               // power up ADC
   ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // divider 128 on
   ADCSRB = 0x00;                                       // not the free running mode
   DIDR0 = _BV(1) | _BV(0);                             // disable digital input buffers
   ACSR = (1 << ACD);                                   // disable analog comparator
   ADMUX = (1<<REFS0) | (1<<MUX1) | (1<<MUX0);          // reference is 3,3V VCC and ADC3 is selected
   ADCSRA = (1<<ADEN);                                  // enable ADC
#endif 
}


/**
 * @brief Returns internal chip temperature in Degree
 *
 * @return    float internal chip temperature
 *
 */
void app_read_temp(temperature_t* temperature)
{
#ifdef INTERNAL_TEMP
   /* start conversion */
   ADCSRA |= (1<<ADSC);
   /* wait until conversion completed */
   while(ADCSRA & (1<<ADSC)) { ; }

   /* calculate temperature (see datasheet for further details) */
   int32_t temp = (113 * ADC) - 27280;
   /* set integral and fractional part */
   if(temp < 0) {temperature->sign = 1;} else{temperature->sign = 0;}
   temperature->integralDigit = temp / 100;
   temperature->fractionalDigit = temp % 100;
#endif
#ifdef EXTERNAL_TEMP
   static uint32_t temp;
   uint8_t i;
   uint32_t volt;

   /* enable external Memory Interface (XRAM) */
   XMCRA |= _BV(SRE);
   XMCRB = _BV(XMBK);
   /* Thermistor On, LEDs off */
   *pIO_THERM = 0x03;

   /* start ADC-Conversion */
   ADCSRA |= (1<<ADSC);
   /* wait until conversion is completed */
   while ( ADCSRA & (1<<ADSC) ) { ; }

   /* read ADC RAW value */
   volt = ADC;
   /* 3300mV at 1024 steps are 3223mV per step */
   volt = (3223UL*volt);

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
 *  - the PAL module,
 *  - the LED and Button interface
 *  - temperature sensor
 *
 */
void app_init(void) {

   /* initialize MCU peripherals */
   pal_init();
   pal_led_init();
   pal_button_init();

   /* open stream and connect to device (in this case USB) */
   fdevopen(_sio_putchar, _sio_getchar);

   /* inititialize temperature sensor */
   app_init_temp();

   /* Initialize the serial interface used for communication with terminal program */
   if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
   {
      /* something went wrong during initialization */
      pal_alert();
   }

   printf("-- Press an arbitrary character to get temperature --\r\n");

}


/**
 * @brief Application task function.
 *
 * This function send the actual temperature to USB interface
 * and toggle LED0 if a keyboard key is pressed.
 *
 */
void app_task(void) 
{
   /* wait until char is typed in -> return pressed character, but is not needed yet */
   sio_getchar();

   /* now read temperature */
   app_read_temp(&temperature);

   /* print temperature */
   printf("Temperature: %c%d.%02d °C\n",(temperature.sign ? '-' : '+'), temperature.integralDigit, temperature.fractionalDigit);

   /* just toggle LED */
   pal_led(LED_0, LED_TOGGLE);

}

/**
 * @brief Main function of the PAL Demo Application "deRFsimple_USB".
 *
 */
int main(void)
{
   /* initialize hardware */
   app_init();

   /* start application */
   while(1)
   {
      /* main application */
      app_task();
      /* calls usb_handler() which checks on USB data */
      pal_task();
   }
}

/** @}*/
/** @}*/

/* EOF */
