/**
 * @file pal_board.c
 *
 * @brief PAL board specific functionality
 *
 * This file implements PAL board specific functionality.
 *
 * $Id: pal_board.c 19228 2009-11-13 11:52:02Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================ */

#include <stdbool.h>
#include <stdlib.h>
#include "pal.h"
#include "pal_boardtypes.h"
#include "pal_config.h"
/*
 * 'sys_time' is a entity of timer module which is given to other modules
 * through the interface file below. 'sys_time' is required to obtain the
 * frame timestamp
 */
#include "pal_timer.h"

#if (BOARD_TYPE == REB_4_0_STK600)

/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ======================================================= */

/**
 * @brief Provides timestamp of the last received frame
 *
 * This function provides the timestamp (in microseconds)
 * of the last received frame.
 *
 * @param[out] Timestamp in microseconds
 */
void pal_trx_read_timestamp(uint32_t *timestamp)
{
    /*
     * Everytime a transceiver interrupt is triggred, input capture register of
     * the AVR is latched. The 'sys_time' is concatenated to the ICR to
     * generate the time stamp of the received frame.
     * 'sys_time'   'ICR'
     *  ---------|--------- => 32 bit timestamp
     *   16 bits   16 bits
     */
    *timestamp = (uint32_t)sys_time << (uint32_t)16;
    *timestamp |= (uint32_t)TIME_STAMP_REGISTER;
}



/**
 * @brief Calibrates the internal RC oscillator
 *
 * This function calibrates the internal RC oscillator.
 *
 * @return True since the RC oscillator is always calibrated
 *         automatically at startup by hardware itself
 */
bool pal_calibrate_rc_osc(void)
{
    return (true);
}



/**
 * @brief Initialize the event system of the ATxmega
 */
void event_system_init(void)
{
    /* Route system clock (16MHz) with prescaler 16 through event channel 0 */
    EVSYS_CH0MUX = EVSYS_CHMUX_PRESCALER_16_gc;

    /* Route port C pin 1 (DIG2) through event channel 1 */
    EVSYS_CH1MUX = EVSYS_CHMUX_PORTC_PIN1_gc;
}



/**
 * @brief Initialize the interrupt system of the ATxmega
 */
void interrupt_system_init(void)
{
    /* Enable high priority interrupts */
    PMIC.CTRL |= PMIC_HILVLEN_bm;
}



/**
 * @brief Initialize the clock of the ATxmega
 */
void clock_init(void)
{
    /*
     * Enable 32MHz internal oscillator
     * (and by thus disable the 2 MHz internal oscillator). */
    OSC.CTRL = OSC_RC32MEN_bm;

    /*
     * The ATxmega shall run from its internal 32MHz Oscillator.
     * Set the clock speed to 16MHz. Use internal 32MHz and DFLL.
     */
    while (0 == (OSC.STATUS & OSC_RC32MRDY_bm))
    {
        /* Hang until the internal 32MHz Oscillator is stable. */
        ;
    }

    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Use Prescaler A to divide 32MHz clock by 2 to 16MHz system clock. */
    CLK.PSCTRL = CLK_PSADIV0_bm;

    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Set internal 32MHz Oscillator as system clock. */
    CLK.CTRL = CLK_SCLKSEL0_bm;

    /* Enable DFLL for the internal oscillator. */
    OSC.DFLLCTRL = OSC_RC32MCREF_bm;
    DFLLRC32M.CTRL |= DFLL_ENABLE_bm;
}



/**
 * @brief Initializes the GPIO pins
 *
 * This function is used to initialize the port pins used to connect
 * the microcontroller to transceiver.
 */
void gpio_init(void)
{
    /* The following pins are output pins.  */
    TRX_PORT1_DDR |= _BV(SEL);
    TRX_PORT1_DDR |= _BV(SCK);
    TRX_PORT1_DDR |= _BV(TRX_RST);
    TRX_PORT1_DDR |= _BV(MOSI);
    TRX_PORT1_DDR |= _BV(SLP_TR);

    /* The following pins are input pins.  */
    TRX_PORT1_DDR &= ~_BV(MISO);
}



/*
 * This function is called by timer_init() to perform the non-generic portion
 * of the initialization of the timer module.
 *
 * sys_clk = 16MHz -> Will be used as source for Event Channel 0 with Prescaler 16
 *
 * Timer usage
 * TCC0_CCA: Systime (software timer based on compare match)
 * TCC1_CCA: Input capture for time stamping
 */
void timer_init_non_generic(void)
{
    /* Select proper clock as timer clock source when radio is sleeping */
    TIMER_SRC_DURING_TRX_SLEEP();

    /*
     * Clear pending output compare matches of all.
     */
    TCC0_INTFLAGS = TC0_CCAIF_bm | TC0_CCBIF_bm | TC0_CCCIF_bm | TC0_CCDIF_bm;
    TCC1_INTFLAGS = TC1_CCAIF_bm | TC1_CCBIF_bm;

    /* Enable the timer overflow interrupt for TCC0 used for systime overflow. */
    TCC0_INTCTRLA =  TC_OVFINTLVL_HI_gc;

    /* Assign event channel 1 as input capture to TCC1_CCA */
    TCC1_CTRLB |= TC1_CCAEN_bm;
    TCC1_CTRLD = TC1_EVACT0_bm | TC1_EVSEL3_bm | TC1_EVSEL0_bm;
}



/**
 * @brief Initialize LEDs
 */
void pal_led_init(void)
{
    /* All pins are output. */
    LED_PORT.OUT = 0xFF;
    LED_PORT.DIR = 0xFF;
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

    switch (led_no)
    {
        case LED_0: pin = LED_PIN_0; break;
        case LED_1: pin = LED_PIN_1; break;
        case LED_2: pin = LED_PIN_2; break;
        case LED_3: pin = LED_PIN_3; break;
        case LED_4: pin = LED_PIN_4; break;
        case LED_5: pin = LED_PIN_5; break;
        case LED_6: pin = LED_PIN_6; break;
        case LED_7: pin = LED_PIN_7; break;
        default: pin = LED_PIN_0; break;
    }

    switch (led_setting)
    {
        case LED_ON: LED_PORT.OUTCLR = _BV(pin); break;

        case LED_OFF: LED_PORT.OUTSET = _BV(pin); break;

        case LED_TOGGLE:
        default:
            LED_PORT.OUTTGL = _BV(pin);
            break;
    }
}



/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    /* All pins are input. */
    BUTTON_PORT.DIR = 0;
}



/**
 * @brief Read button
 *
 * @param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    uint8_t pin;

    switch (button_no)
    {
        case BUTTON_0: pin = BUTTON_PIN_0; break;
        case BUTTON_1: pin = BUTTON_PIN_1; break;
        case BUTTON_2: pin = BUTTON_PIN_2; break;
        case BUTTON_3: pin = BUTTON_PIN_3; break;
        case BUTTON_4: pin = BUTTON_PIN_4; break;
        case BUTTON_5: pin = BUTTON_PIN_5; break;
        case BUTTON_6: pin = BUTTON_PIN_6; break;
        case BUTTON_7: pin = BUTTON_PIN_7; break;
        default: pin = BUTTON_PIN_0; break;
    }

    if ((BUTTON_PORT.IN & (1 << pin)) == 0x00)
    {
        return BUTTON_PRESSED;
    }
    else
    {
        return BUTTON_OFF;
    }
}

#endif /* REB_4_0_STK600 */

/* EOF */
