/**
 * @file main.c
 *
 * @addtogroup armExamples ARM Examples
 * @{
 * @addtogroup deRFnative_Button_LED deRFnative_Button_LED
 * @{
 *
 * @brief  Implementation of a Demo Application showing LED and button usage.
 *
 * This application example demonstrates how to use the buttons and LED
 * available on the dresden elektronik node/gateway boards.
 * It bases on the AT91 Library but does not make usage of the Atmel MAC stack.
 * By pressing one of the two buttons, an internal counter may be incremented
 * or decremented while the three LEDs show the current counter value.
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

#include <board.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <utility/led.h>

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

// Button pin instances
const Pin pinPB1 = PIN_PUSHBUTTON_0;
const Pin pinPB2 = PIN_PUSHBUTTON_1;

// current counter value
volatile static unsigned int counter = 0;

/* === PROTOTYPES ========================================================== */
void UpdateLedValue(void);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Interrupt handler for Button 1 - increments the counter value
 */
void ISR_Bp1(void)
{
    // Check if the button has been pressed
    if (!PIO_Get(&pinPB1))
    {
        //wait until button gets released
        while (!PIO_Get(&pinPB1))
            ;

        counter = ((counter+1)%8);
    }
}


/**
 * @brief Interrupt handler for Button 2 - decrements the counter value
 */
static void ISR_Bp2(void)
{
    // Check if the button has been pressed
    if (!PIO_Get(&pinPB2))
    {
        //wait until button gets released
        while (!PIO_Get(&pinPB2))
            ;

        counter = ((counter-1)%8);
    }
}


/**
 * @brief Configures the buttons to generate interrupts when pressed.
 */
void ConfigureButtons(void)
{
    // Configure PIO
    PIO_Configure(&pinPB1, 1);
    PIO_Configure(&pinPB2, 1);

    // Initialize interrupts
    PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);
    PIO_ConfigureIt(&pinPB1, (void (*)(const Pin *)) ISR_Bp1);
    PIO_ConfigureIt(&pinPB2, (void (*)(const Pin *)) ISR_Bp2);
    PIO_EnableIt(&pinPB1);
    PIO_EnableIt(&pinPB2);
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
 * @brief Main application.
 * Configures Buttons, LEDs and finally loops infinite.
 */
int main(void)
{
    // Configuration
    ConfigureButtons();
    ConfigureLeds();

    // Main loop
    while (1)
    {
        UpdateLedValue();
    }
}


/**
 * @brief Displays the current counter value on the onboard LEDs.
 */
void UpdateLedValue(void)
{
    (counter & 0x01) ? LED_Set(0) : LED_Clear(0);
    (counter & 0x02) ? LED_Set(1) : LED_Clear(1);
    (counter & 0x04) ? LED_Set(2) : LED_Clear(2);
}

/** @}*/
/** @}*/

/* EOF */

