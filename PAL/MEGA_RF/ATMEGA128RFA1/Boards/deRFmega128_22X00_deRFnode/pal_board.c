/**
 * @file pal_board.c
 *
 * @brief PAL board specific functions
 *
 * This file implements PAL board specific functionality.
 *
 * @author
 *      dresden elektronik: http://www.dresden-elektronik.de
 *      Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 * Licensed under dresden elektronik's Limited License Agreement --> deEULA.txt
 */

/* === Includes ============================================================ */

#include <stdbool.h>
#include <stdlib.h>
#include "pal.h"
#include "pal_boardtypes.h"
#include "pal_config.h"
#include "pal_usb.h"

#if (BOARD_TYPE == deRFmega128_22X00_deRFnode)
 
/* === Macros ============================================================== */

/* === Types =============================================================== */

/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ======================================================= */

/**
 * @brief Calibrates the internal RC oscillator
 *
 * This function does nothing, it was left here just for compatibility with
 * the PAL API.
 *
 * @return True if calibration is successful, false otherwise.
 */
bool pal_calibrate_rc_osc(void)
{
    /* Nothing to be done. */
    return true;
}


/**
 * @brief Initialize LEDs
 */
void pal_led_init(void)
{
    // LEDs on deRFnode

    // set portpins as output
    LED0_DDR |= (1 << LED0_PIN);
    LED1_DDR |= (1 << LED1_PIN);
    LED2_DDR |= (1 << LED2_PIN);

    // initialize LED states to OFF, they are low-active
    LED0_PORT |= (1 << LED0_PIN);
    LED1_PORT |= (1 << LED1_PIN);
    LED2_PORT |= (1 << LED2_PIN);
}


/**
 * @brief Control LED status
 *
 * @param[in]  led_no LED ID
 * @param[in]  led_setting LED_ON, LED_OFF, LED_TOGGLE
 */
void pal_led(led_id_t led_no, led_action_t led_setting)
{
    switch (led_no)
    {
        case LED_0:
            switch (led_setting)
            {
                case LED_ON:        LED0_PORT &= ~(1 << LED0_PIN); break;
                case LED_OFF:       LED0_PORT |=  (1 << LED0_PIN); break;
                case LED_TOGGLE:    LED0_PORT ^=  (1 << LED0_PIN); break;
            }
            break;

        case LED_1:
            switch (led_setting)
            {
                case LED_ON:        LED1_PORT &= ~(1 << LED1_PIN); break;
                case LED_OFF:       LED1_PORT |=  (1 << LED1_PIN); break;
                case LED_TOGGLE:    LED1_PORT ^=  (1 << LED1_PIN); break;
            }
            break;

        case LED_2:
            switch (led_setting)
            {
                case LED_ON:        LED2_PORT &= ~(1 << LED2_PIN); break;
                case LED_OFF:       LED2_PORT |=  (1 << LED2_PIN); break;
                case LED_TOGGLE:    LED2_PORT ^=  (1 << LED2_PIN); break;
            }
            break;

        default: break;
    }
}


/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    // button on the deRFnode

    // set portpins as input
    BUTTON0_DDR &= ~(1 << BUTTON0_PIN);
    BUTTON1_DDR &= ~(1 << BUTTON1_PIN);

    // enable internal pullups
    BUTTON0_PORT |= (1 << BUTTON0_PIN);
    BUTTON1_PORT |= (1 << BUTTON1_PIN);
}


/**
 * @brief Read button
 *
 * @param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    switch (button_no)
    {
        /* buttons are low-active! */
        case BUTTON_0:
            return (BUTTON0_PINS & (1 << BUTTON0_PIN)) ? BUTTON_OFF : BUTTON_PRESSED;

        case BUTTON_1:
            return (BUTTON1_PINS & (1 << BUTTON1_PIN)) ? BUTTON_OFF : BUTTON_PRESSED;

        default:
            return BUTTON_OFF;
    }
}

#endif /* deRFmega128_22X00_deRFnode */

/* EOF */
