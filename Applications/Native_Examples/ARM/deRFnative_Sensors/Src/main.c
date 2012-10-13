/**
 * @file main.c
 *
 * @addtogroup armExamples ARM Examples
 * @{
 * @addtogroup deRFnative_Sensors_ARM deRFnative_Sensors ARM
 * @{
 *
 * @brief  Implementation of a Demo Application showing LED and button usage.
 *
 * This application example focuses on the sensors available on the
 * dresden elektronik node/gateway boards. It bases on the AT91 Library
 * and additionally employs a custom I2C Library for accessing the sensors
 * but does not make usage of the Atmel MAC stack.
 * The basic prerequisite is, that JP1 is set to interconnect pins 2-3.
 *
 * After application start, a timer is responsible for gathering current
 * temperature, luminosity and three-axial acceleration values and printing
 * them on the debug unit. This is indicated by LED0 blinking slowly.
 *
 * By pressing Button1 you can switch on/off the 'motion detection'-feature
 * which is indicated by LED1 glowing respectively not glowing. A movement
 * of the board such as shaking it will be indicated by LED1 blinking fast.
 * Also the gathering sensor values is stopped. This state must be explicitely
 * reset by pressing Button1 again where the application returns to its initial
 * state.
 *
 *
 * $Id: main.c,v 1.1.6.1 2010/09/07 17:39:32 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 *
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

/* === INCLUDES ============================================================ */
#include <stdbool.h>
#include "i2c.h"

#include <board.h>
#include <aic/aic.h>
#include <dbgu/dbgu.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <tc/tc.h>
#include <utility/led.h>
#include <utility/trace.h>
#include <stdio.h>


/* === TYPES =============================================================== */
typedef enum
{
    MD_DISABLED,
    MD_ENABLED,
    MD_TRIGGERED
} motion_detection_state_t;


/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

// Button pin instance.
const Pin pinPB1 = PIN_PUSHBUTTON_0;

// Pin to which the acceleration sensor is connected
const Pin pinBMA150 = PIN_BMA150_IRQ;

// current sensor values
float temperature1, temperature2;
unsigned int luminosity;
unsigned int ac_x, ac_y, ac_z;

// current state of motion detection feature
motion_detection_state_t motion_detection_state;


/* === PROTOTYPES ========================================================== */
void ConfigureBMA150IRQ(bool on);
void ISR_BMA150(void);
void ISR_Bp1(void);
void ConfigureButtons(void);
void ConfigureLeds(void);
void ConfigureI2C(void);
void ISR_TC0();
void ISR_TC1();
void ConfigureTc0(void);
void ConfigureTc1(void);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Service routine for Interrupts caused by the BMA150 acceleration sensor.
 */
void ISR_BMA150(void)
{
    // wait for IRQ line going L
    while (PIO_Get(&pinBMA150))
        ;

    // disable interrupt
    ConfigureBMA150IRQ(false);

    // stop gathering sensor data
    TC_Stop(AT91C_BASE_TC0);
    LED_Clear(0);

    // start timer 1
    TC_Start(AT91C_BASE_TC1);

    motion_detection_state = MD_TRIGGERED;
}

/**
 * @brief Enables/disables the Accceleration Sensor throwing Interrupts
 * upon motion detection.
 */
void ConfigureBMA150IRQ(bool on)
{
    // Configure IRQ line as input Pin which is shared with button2.
    // While the button triggers on Low, the BMA150 on high level.
    PIO_Configure(&pinBMA150, 1);
    // assign an ISR
    PIO_ConfigureIt(&pinBMA150, (void (*)(const Pin *)) ISR_BMA150);

    // enable/disable Interrupt generation
    if(on)
    {
        BMA150_Set_Anymotion_IRQ(true);
        PIO_EnableIt(&pinBMA150);
    }
    else
    {
        PIO_DisableIt(&pinBMA150);
        BMA150_Set_Anymotion_IRQ(false);
    }
}

/**
 * @brief Interrupt handler for Button 1 - toggles motion detection feature.
 */
void ISR_Bp1(void)
{
    // Check if the button has been pressed
    if (!PIO_Get(&pinPB1))
    {
        //wait until button gets released
        while (!PIO_Get(&pinPB1))
            ;

        // toggle motion detection state
        switch(motion_detection_state)
        {
            case MD_DISABLED:
                // enable motion detection
                ConfigureBMA150IRQ(true);
                LED_Set(1);
                motion_detection_state = MD_ENABLED;
                break;

            case MD_ENABLED:
                // disable motion detection
                ConfigureBMA150IRQ(false);
                LED_Clear(1);
                motion_detection_state = MD_DISABLED;
                break;

            case MD_TRIGGERED:
                // reset to initial state

                // stop LED1 flashing
                TC_Stop(AT91C_BASE_TC1);
                AT91C_BASE_TC1->TC_SR;
                LED_Clear(1);

                // re-start gathering sensor data
                TC_Start(AT91C_BASE_TC0);
                motion_detection_state = MD_DISABLED;
                break;

            default:
                break;
        }
    }
}


/**
 * @brief Configures the button to generate interrupts when pressed.
 */
void ConfigureButtons(void)
{
    // Configure PIO
    PIO_Configure(&pinPB1, 1);

    // Initialize interrupts
    PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);
    PIO_ConfigureIt(&pinPB1, (void (*)(const Pin *)) ISR_Bp1);
    PIO_EnableIt(&pinPB1);
}


/**
 * @brief Configures the onboard-LEDs (cleared by default).
 */
void ConfigureLeds(void)
{
    LED_Configure(0);
    LED_Configure(1);
    LED_Configure(2);
}


/**
 * @brief Configures Two-Wire Interface aka I2C and connected sensors.
 */
void ConfigureI2C(void)
{
    // Configure TWI
    Configure_TWI();

    DS620_Configure();
    ISL29020_Configure();
    BMA150_Configure();
}


/**
 * @brief Handles interrupts coming from Timer0.
 */
void ISR_TC0()
{
    // Clear status bit to acknowledge interrupt
    unsigned int status = AT91C_BASE_TC0->TC_SR;

    // keep compiler happy
    status = status;

    LED_Toggle(0);

    // invalidate previous read data
    // such that displayed zeroes indicate an I2C-error
    temperature1 = 0;
    luminosity = 0;
    ac_x = ac_y = ac_z = 0;
    temperature2 = 0;

    status = DS620_Read_Temperature(&temperature1);
    printf("DS620's current temperature:   %2.3f [°C] \n\r",temperature1);

    status=ISL29029_Read_Luminosity(&luminosity);
    printf("ISL29020's current luminosity: %d    [lux] \r\n", luminosity);

    status = BMA150_Read_Temperature(&temperature2);
    printf("BMA150's current temperature:  %2.1f   [°C] \r\n", temperature2);

    status = BMA150_Read_Acceleration(&ac_x, &ac_y, &ac_z);
    printf("BMA150's curr. acceleration (x,y,z): %04u, %04u, %04u [E-3g] \r\n", ac_x, ac_y, ac_z);

    printf("\r\n");
}


/*
 * @brief Configures the Timer0 to generate an interrupt every 1000ms.
 */
void ConfigureTc0(void)
{
    // enable timer peripheral
    AT91C_BASE_PMC->PMC_PCER |= (1 << AT91C_ID_TC0);
    // timer clock is BOARD_MCK/1024, compare to Register C interrupt
    TC_Configure(AT91C_BASE_TC0, AT91C_TC_CLKS_TIMER_DIV5_CLOCK | AT91C_TC_WAVESEL_UP_AUTO | AT91C_TC_WAVE);
    // set compare value
    AT91C_BASE_TC0->TC_RC = (BOARD_MCK/1024);
    // enable interrupt
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_ConfigureIT(AT91C_ID_TC0, 0, ISR_TC0);
    AIC_EnableIT(AT91C_ID_TC0);
    // start timer
    TC_Start(AT91C_BASE_TC0);
}


/**
 * @brief Handles interrupts coming from Timer1.
 */
void ISR_TC1()
{
    // Clear status bit to acknowledge interrupt
    unsigned int status = AT91C_BASE_TC1->TC_SR;

    // keep compiler happy
    status = status;

    LED_Toggle(1);
}


/*
 * @brief Configures the Timer1 to generate an interrupt every 500ms.
 * Timer needs to be started separately!
 */
void ConfigureTc1(void)
{
    // enable timer peripheral
    AT91C_BASE_PMC->PMC_PCER |= (1 << AT91C_ID_TC1);
    // timer clock is BOARD_MCK/1024, compare to Register C interrupt
    TC_Configure(AT91C_BASE_TC1, AT91C_TC_CLKS_TIMER_DIV5_CLOCK | AT91C_TC_WAVESEL_UP_AUTO | AT91C_TC_WAVE);
    // set compare value
    AT91C_BASE_TC1->TC_RC = (BOARD_MCK/(1024*4));
    // enable interrupt
    AT91C_BASE_TC1->TC_IER = AT91C_TC_CPCS;
    AIC_ConfigureIT(AT91C_ID_TC1, 0, ISR_TC1);
    AIC_EnableIT(AT91C_ID_TC1);
}


/**
 * @brief Main application.
 * Configures Buttons, LEDs, Timer and finally loops infinite
 */
int main(void)
{
    // configure the Debug Unit for console printout
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    printf("-- deRFnode sensor usage project --\r\n");
    printf("-- Compiled: %s %s --\r\n", __DATE__, __TIME__);

    // Configuration
    ConfigureButtons();
    ConfigureLeds();
    ConfigureI2C();

    // configure _and_ start a timer triggering every second
    ConfigureTc0();

    // configure but do not start a timer triggering every 500ms
    ConfigureTc1();

    motion_detection_state = MD_DISABLED;

    // Main loop
    while (1)
        ;
}

/** @}*/
/** @}*/

/* EOF */

