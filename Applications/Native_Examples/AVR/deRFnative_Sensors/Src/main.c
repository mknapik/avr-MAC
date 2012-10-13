/**
 * @file main.c
 *
 * @addtogroup avrExamples AVR Examples
 * @{
 * @addtogroup deRFnative_Sensors deRFnative_Sensors
 * @{
 *
 * @brief  Implementation of the native Demo Application "deRFnative_Sensors".
 *
 * There is no use of any stack layer nor API.
 *
 * The native Example "deRFnative_Sensors" demonstrates the native access
 * to resources (temperature, light and acceleration sensor)
 * of the following platforms:
 *    - deRFnode
 *
 * The example code initialize all external sensors on deRFnode:
 *    - ISL290 light sensor
 *    - BMA150 acceleration sensor
 *    - DS620 temperature sensor
 *
 * And read out actual values from each sensor if button 1 is pressed.
 * The values will be print out via USB interface. Therefore you have to
 * open Hyperterminal (or any other terminal) and connect to the virtual
 * COM port that is assigned to your platform (FTDI serial port).
 *
 * Additionally acceleration sensor values print out, every time the
 * sensor recognized any change (by moving deRFnode). This is done by
 * timer interrupt handling. To enable this option make sure Jumper1
 * (JP1) is connected to interrupt pin of BMA150 (pin2 and pin3 have
 * to be connected on JP1).
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

#include <stdio.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "usb.h"
#include "io_access.h"
#include "DS620.h"
#include "ISL290.h"
#include "BMA150.h"
#include "twi_master.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/** macro for printf */
#define PRINT(...)    printf(__VA_ARGS__)

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ============================================================= */

/** Setup a File stream with putchar() and getchar() functionality over USB */
FILE usb_stream = FDEV_SETUP_STREAM(usb_putc_std, usb_getc_std, _FDEV_SETUP_RW);

/** global variable to save temperature value */
DS620_temperature_t temperature;
/** global variable to save light sensor value */
uint16_t lux;
/** global variable to save acceleration values */
BMA150_data_t BMA150_data;

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Initialize timer system.
 *
 * Since the interrupt pin from BMA150 acceleration sensor is connected to PD6, which is not
 * a interrupt pin, we have to make a little trick to be able to use interrupt from BMA150.
 *
 * Pin PD6 is also external clock source (T1). So we configure Timer1 to be triggered via T1 pin.
 * We switch to CTC (Clear Timer on Compare Match) Mode and set Output Compare Register to 1.
 * So, everytime a interrupt is detected, the timer counter raises up, compare to Output Compare
 * Register and throws an interrupt if equal. This happens every time a interrupt is detected,
 * because Compare register is set to 1.
 */
void init_timer(void)
{
   /* enable CTC mode and set Clock select source to T1 pin on rising edge */
   TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS11) | _BV(CS10);
   /* set Output Compare Register A to 1 */
   OCR1AL = 0x01;

   /* enable Timer/Counter1 Output Compare A Match Interrupt */
   TIMSK1 |= _BV(OCIE1A);

   /* enable global interrupt */
   sei();
}

/**
 * @brief Print sensor results
 */
void print_results(void)
{
   /* print out temperature sensor values, but check if conversion is ready */
   if(temperature.conversion_done == 1)
   {
      PRINT("[DS620]: %c%d.%02d °C\n",(temperature.sign ? '-' : '+'), temperature.integralDigit, temperature.fractionalDigit);
   }
   else
   {
      PRINT("[DS620]: Conversion uncompleted\n");
   }

   /* print out light sensor value */
   PRINT("[ISL290]: %d Lux\n", lux);

   /* print out acceleration values */
   PRINT("[BMA150]  acc_x: %c%d.%02d g\n", (BMA150_data.acc_x_sign ? '-' : '+'), BMA150_data.acc_x_integral, BMA150_data.acc_x_fractional);
   PRINT("[BMA150]  acc_y: %c%d.%02d g\n", (BMA150_data.acc_y_sign ? '-' : '+'), BMA150_data.acc_y_integral, BMA150_data.acc_y_fractional);
   PRINT("[BMA150]  acc_z: %c%d.%02d g\n", (BMA150_data.acc_z_sign ? '-' : '+'), BMA150_data.acc_z_integral, BMA150_data.acc_z_fractional);
}

/**
 * @brief Initialize the hardware:
 *
 * Initialize
 *    - IO's
 *    - USB
 *    - TWI interface
 *    - timer
 *    - sensors
 *
 */
void app_init(void)
{
   /* initialize hardware */
   io_init();

   /* initialize USB interface */
   usb_init();

   /* redirect standard input/output to USB stream
    * -> now we can use printf/scanf over USB */
   stdin = stdout = &usb_stream;

   /* initialize timer to create a interrupt system for BMA150 acceleration sensor */
   init_timer();

   /* initialize TWI interface with 100kHz */
   twi_init(100000); //100kHz

   /* initialize the DS620 temperature sensor */
   DS620_init((DS620_CFG_RESOLUTION_13BIT | DS620_CFG_1SHOT), 0x00);

   /* initialize the ISL290 light sensor and activate it (continuous measurement) */
   ISL290_init(ISL290_MODE_ENABLE);

   /* initialize the BMA150 acceleration sensor */
   BMA150_init();

   /* activate acceleration sensor interrupt system */
   BMA150_set_anymotion_IRQ(BMA150_IRQ_ENABLE);
}

/**
 * @brief Main function of the Native Demo Application "deRFnative_Temp".
 */
int main(void)
{
   /* initialize hardware and all sensors */
   app_init();

   /* start application */
   while(1)
   {
      /* start temperature measuremnt on DS620 temperature sensor
       * (need some time before conversion is completed) */
      DS620_start_measurement();

      /* wait conversion Time (see datasheet for further details) */
      _delay_ms(200);

      /* read back temperature from DS620 */
      DS620_read_temperature(&temperature);

      /* read data from light sensor */
      ISL290_read_data(&lux);

      /* read data from acceleration sensor */
      uint8_t error = BMA150_read_acc(&BMA150_data);

      /* check if acceleration read was successfull */
      if(error){ PRINT("[BMA150]: Error reading sensor\n"); }

      /* print out results */
      print_results();

      /* wait until button is pressed */
      while(BUTTON_RELEASED == button_pressed(BUTTON_0)){;}

      /* now wait until button released (simple debouncing) */
      while(BUTTON_PRESSED == button_pressed(BUTTON_0)){;}

      /* start again */
   }

   return 0;
}

/**
 * @brief Interrupt routine which is execvuted every time acceleration sensor throws an interrupt
 */
ISR(TIMER1_COMPA_vect)
{
   BMA150_read_acc(&BMA150_data);

   PRINT("[BMA150]  acc_x: %c%d.%02d g  -  acc_y: %c%d.%02d g  -  acc_z: %c%d.%02d g\n",
         (BMA150_data.acc_x_sign ? '-' : '+'), BMA150_data.acc_x_integral, BMA150_data.acc_x_fractional,
         (BMA150_data.acc_y_sign ? '-' : '+'), BMA150_data.acc_y_integral, BMA150_data.acc_y_fractional,
         (BMA150_data.acc_z_sign ? '-' : '+'), BMA150_data.acc_z_integral, BMA150_data.acc_z_fractional);

}


/** @}*/
/** @}*/

/* EOF */

