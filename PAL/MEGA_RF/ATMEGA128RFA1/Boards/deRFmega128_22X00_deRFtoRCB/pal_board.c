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

#if (BOARD_TYPE == deRFmega128_22X00_deRFtoRCB)
 
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
    // set portpins as output
    LED_PORT_DIR |= (1 << LED_PIN_0) | (1 << LED_PIN_1) | (1 << LED_PIN_2);

    // initialize LED states to OFF, they are low-active
    LED_PORT |= ((1 << LED_PIN_0) | (1 << LED_PIN_1) | (1 << LED_PIN_2));
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
        case 0:
            switch (led_setting)
            {
            case LED_ON:     LED_PORT &= ~(1 << LED_PIN_0); break;
            case LED_OFF:    LED_PORT |= (1 << LED_PIN_0); break;
            case LED_TOGGLE: LED_PORT ^= (1 << LED_PIN_0); break;
            }
            break;

        case 1:
            switch (led_setting)
            {
            case LED_ON:     LED_PORT &= ~(1 << LED_PIN_1); break;
            case LED_OFF:    LED_PORT |= (1 << LED_PIN_1); break;
            case LED_TOGGLE: LED_PORT ^= (1 << LED_PIN_1); break;
            }
            break;

        case 2:
            switch (led_setting)
            {
            case LED_ON:     LED_PORT &= ~(1 << LED_PIN_2); break;
            case LED_OFF:    LED_PORT |= (1 << LED_PIN_2); break;
            case LED_TOGGLE: LED_PORT ^= (1 << LED_PIN_2); break;
            }
            break;

        default:
            break;

    }
}


/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    // button on the deRFtoRCB adapter

    // set portpin as input
    BUTTON_PORT_DIR &= ~(1 << BUTTON_PIN_0);

    // disable internal pullup
    BUTTON_PORT     &= ~(1 << BUTTON_PIN_0);
}


/**
 * @brief Read button
 *
 * @param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    /* low active */
    switch (button_no)
    {
        case BUTTON_0:
            return (BUTTON_INPUT_PINS & (1 << BUTTON_PIN_0))
                    ? BUTTON_OFF : BUTTON_PRESSED;

        default:
            return BUTTON_OFF;
    }
}

#endif /* deRFmega128_22X00_deRFtoRCB */

/* EOF */
