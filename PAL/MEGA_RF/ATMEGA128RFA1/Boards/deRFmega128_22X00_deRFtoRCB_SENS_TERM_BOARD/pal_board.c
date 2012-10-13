/**
 * @file pal_board.c
 *
 * @brief PAL board specific functionality
 *
 * This file implements PAL board specific functionality.
 *
 * $Id: pal_board.c,v 1.1.6.1 2010/09/07 17:38:26 dam Exp $
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

#if (BOARD_TYPE == deRFmega128_22X00_deRFtoRCB_SENS_TERM_BOARD)

/* === Macros ============================================================== */

/* === Types =============================================================== */

/* === Globals ============================================================= */

static uint8_t led_state = 0x00;

/* === Prototypes ========================================================== */

static void led_helper_func(void);


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
    led_state = 0x00;

    /* Switch LEDs off. Both LEDs are low active. */
    LED_PORT_DIR |= (1 << LED_BIT_0);
    LED_PORT_DIR |= (1 << LED_BIT_1);

    LED_PORT |= (1 << LED_BIT_0);
    LED_PORT |= (1 << LED_BIT_1);

    led_helper_func();
}


/**
 * @brief Control LED status
 *
 * @param[in]  led_no LED ID
 * @param[in]  led_setting LED_ON, LED_OFF, LED_TOGGLE
 */
void pal_led(led_id_t led_no, led_action_t led_setting)
{
    uint8_t pin;
    /* New values of LED pins based on new LED state. */
    uint8_t led_pin_value;
    /*
     * Original value of LED port before writing new value.
     * This value needs to be restored.
     */
    uint8_t orig_led_port = LED_PORT & ~LED_BIT_MASK;

    /* Both LEDs need to be updated, since several peripherals
     * are dealing with the same port for this board
     * (USB, EEPROM, LEDs, Button).
     */
    LED_PORT_DIR |= (1 << LED_BIT_0);
    LED_PORT_DIR |= (1 << LED_BIT_1);

    switch (led_no)
    {
        default:
        case LED_0:
            pin = LED_BIT_0;
            break;
        case LED_1:
            pin = LED_BIT_1;
            break;
    }

    switch (led_setting)
    {
        case LED_ON:
            led_state |= _BV(pin);
            break;

        case LED_OFF:
            led_state &= ~_BV(pin);
            break;

        case LED_TOGGLE:
        default:
            if (led_state & _BV(pin))
            {
                /*
                 * LED is currently on,
                 * Switch it off
                 */
                led_state &= ~_BV(pin);
            }
            else
            {
                /*
                 * LED is currently off,
                 * Switch it on
                 */
                led_state |= _BV(pin);
            }
            break;
    }

    led_pin_value = (uint8_t)(~(uint16_t)led_state);  // Implicit casting required to avoid IAR Pa091.
    led_pin_value &= LED_BIT_MASK;

    LED_PORT = orig_led_port | led_pin_value;

    led_helper_func();
}


/**
 * @brief Helper function for LEDs
 */
static void led_helper_func(void)
{
    /*
     * Enable LED address decoding.
     * This is similar to USB, but with other settings.
     */
    LED_ADDR_DEC_PORT |= _BV(6);    // Different to USB
    LED_ADDR_DEC_DDR |= _BV(6);
    LED_ADDR_DEC_PORT &= ~_BV(7);
    LED_ADDR_DEC_DDR |= _BV(7);

    /* Enable desired LED state. */
    DDRE |= _BV(4);
    /* Set PE4 to 0 and back to 1. */
    PORTE &= ~_BV(4);
    PORTE |= _BV(4);

    /*
     * Disable LED address decoding.
     * This enables USB again.
     */
    LED_ADDR_DEC_PORT &= ~_BV(6);
    LED_ADDR_DEC_DDR |= _BV(6);
    LED_ADDR_DEC_PORT &= ~_BV(7);
    LED_ADDR_DEC_DDR |= _BV(7);
}


/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    /* Nothing to do here. */
}


/**
 * @brief Read button
 *
 * @param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    uint8_t cur_button_state;

    /* Keep compiler happy. */
    button_no = button_no;

    /*
     * Enable button address decoding.
     * This is similar to USB, but with other settings.
     */
    BUTTON_ADDR_DEC_PORT |= _BV(6);    // Different to USB
    BUTTON_ADDR_DEC_DDR |= _BV(6);
    BUTTON_ADDR_DEC_PORT &= ~_BV(7);
    BUTTON_ADDR_DEC_DDR |= _BV(7);


    PORTE &= ~_BV(5);
    DDRE |= _BV(5);

    /* Switch port to input. */
    BUTTON_PORT |= (1 << BUTTON_PIN_0);
    BUTTON_PORT_DIR &= ~(1 << BUTTON_PIN_0);

    cur_button_state = BUTTON_INPUT_PINS;

    PORTE |= _BV(5);

    /* Switch port back to output. */
    BUTTON_PORT_DIR |= (1 << BUTTON_PIN_0);

    /*
     * Disable button address decoding.
     * This enables USB again.
     */
    BUTTON_ADDR_DEC_PORT &= ~_BV(6);
    BUTTON_ADDR_DEC_DDR |= _BV(6);
    BUTTON_ADDR_DEC_PORT &= ~_BV(7);
    BUTTON_ADDR_DEC_DDR |= _BV(7);

    if (cur_button_state & 0x01)
    {
        return BUTTON_PRESSED;
    }
    else
    {
        return BUTTON_OFF;
    }
}


#endif /* deRFmega128_22X00_deRFtoRCB_SENS_TERM_BOARD */

/* EOF */
