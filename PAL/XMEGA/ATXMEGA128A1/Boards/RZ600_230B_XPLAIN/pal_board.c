/**
 * @file pal_board.c
 *
 * @brief PAL board specific functionality
 *
 * This file implements PAL board specific functionality.
 *
 * $Id: pal_board.c 22413 2010-07-05 11:49:21Z sschneid $
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

#if (BOARD_TYPE == RZ600_230B_XPLAIN)

/* === Globals ============================================================= */


/* === Prototypes ========================================================== */

int16_t adc_get_signed_word(ADC_CH_t * adc_ch);
void adc_wait_8mhz(ADC_t * adc);

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

    /* Route port D pin 2 (TRX IRQ) through event channel 1 */
    EVSYS_CH1MUX = EVSYS_CHMUX_PORTD_PIN2_gc;
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
    LED_PORT.OUT = 0xFF;    /* low active LEDs */
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
        /* LEDs are low active */
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
    PORTF_PIN0CTRL = 7<<3;
    PORTF_PIN1CTRL = 7<<3;
    PORTF_PIN2CTRL = 7<<3;
    PORTF_PIN3CTRL = 7<<3;
    PORTF_PIN4CTRL = 7<<3;
    PORTF_PIN5CTRL = 7<<3;
    PORTF_PIN6CTRL = 7<<3;
    PORTF_PIN7CTRL = 7<<3;

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



/**
 * @brief Generation of random seed for function srand() in case this
 *        is not supported by the transceiver (e.g. AT86RF230)
 *
 * @return uint16_t Random number to be used as seed for function srand()
 * @ingroup apiPalApi
 */
uint16_t pal_generate_rand_seed(void)
{
    uint16_t cur_random_seed = 0;
    uint8_t cur_random_bit = 0;
    int16_t cur_adc_sample = 0;

    /*
     * Loading the calibration values to the corresponding ADC is not required,
     * since we are only targeting to random bits.
     */

    /* Set up the corresponding ADC to have signed conversion mode and 12 bit resolution. */
    ADC_FOR_RANDOM_SEED.CTRLB = (ADC_FOR_RANDOM_SEED.CTRLB & (~(ADC_RESOLUTION_gm | ADC_CONMODE_bm))) |
                                (ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm);

    /* Set sample rate. */
    ADC_FOR_RANDOM_SEED.PRESCALER = (ADC_FOR_RANDOM_SEED.PRESCALER & (~ADC_PRESCALER_gm)) |
                                     ADC_PRESCALER_DIV32_gc;

    /* Set reference voltage on the corresponding ADC to be VCC/1.6 V.*/
    ADC_FOR_RANDOM_SEED.REFCTRL = (ADC_FOR_RANDOM_SEED.REFCTRL & ~(ADC_REFSEL_gm)) |
                                  ADC_REFSEL_VCC_gc;

    /* Setup channel 0. */
    ADC_FOR_RANDOM_SEED.ADC_CHANNEL.CTRL = (ADC_FOR_RANDOM_SEED.ADC_CHANNEL.CTRL & (~(ADC_CH_INPUTMODE_gm | ADC_CH_GAINFAC_gm))) |
                                   ((uint8_t) ADC_CH_INPUTMODE_SINGLEENDED_gc | ADC_CH_GAIN_1X_gc);

    /* Set input to the corresonding channel. */
    ADC_FOR_RANDOM_SEED.ADC_CHANNEL.MUXCTRL = (ADC_POS_MUX_SEL << ADC_CH_MUXPOS_gp) |
                                              (ADC_NEG_MUX_SEL << ADC_CH_MUXNEG_gp);


    /* Enable the ADC. */
    ADC_FOR_RANDOM_SEED.CTRLA |= ADC_ENABLE_bm;

    /*
     * Wait until common mode voltage is stable. Default clk is 2MHz and
     * therefore below the maximum frequency to use this function.
     */
    adc_wait_8mhz(&ADCA);

    /* Enable free running mode. */
    ADC_FOR_RANDOM_SEED.CTRLB |= ADC_FREERUN_bm;

    /* Store samples in table.*/
    for (uint16_t i = 0; i < SAMPLE_COUNT; ++i)
    {
        do
        {
            /*
             * If the conversion on the corresponding channel of the ADC never is
             * complete this will be a deadlock.
             */
        }
        while (!ADC_CH_CONVERSION_COMPLETEs(&ADCA.ADC_CHANNEL));

        cur_adc_sample = adc_get_signed_word(&ADCA.ADC_CHANNEL);

        cur_random_bit = cur_adc_sample & LSB_MASK_INT_16;
        cur_random_seed = cur_random_seed << 1;
        cur_random_seed |= cur_random_bit;
    }

    /* Turn off free running and disable ADC module.*/
    ADC_FOR_RANDOM_SEED.CTRLB = ADC_FOR_RANDOM_SEED.CTRLB & (~ADC_FREERUN_bm);

    /* Disable the ADC. */
    ADC_FOR_RANDOM_SEED.CTRLA = ADC_FOR_RANDOM_SEED.CTRLA & (~ADC_ENABLE_bm);

    return (cur_random_seed);
}



/**
 * @brief This function clears the interrupt flag and returns the signed coversion result
 *
 *  This function should be used together with the ADC_CH_CONVERSION_COMPLETE macro.
 *  When the conversion result is ready this function reads out the result.
 *
 *  @param  adc_ch  Pointer to ADC channel register section.
 *
 *  @return  The signed conversion result.
 */
int16_t adc_get_signed_word(ADC_CH_t *adc_ch)
{
    int16_t answer;

    /* Clear interrupt flag.*/
    adc_ch->INTFLAGS = ADC_CH_CHIF_bm;

    /* Return result register contents*/
    answer = adc_ch->RES;

    return answer;
}



/**
 * @brief  This function waits until the adc common mode is settled
 *
 *  After the ADC clock has been turned on, the common mode voltage in the ADC
 *  need some time to settle. The time it takes equals one dummy conversion.
 *  Instead of doing a dummy conversion this function waits until the common
 *  mode is settled.
 *
 * @param adc Pointer to ADC module register section
 */
void adc_wait_8mhz(ADC_t * adc)
{
    /* Store old prescaler value. */
    uint8_t prescaler_val = adc->PRESCALER;

    /* Set prescaler value to minimum value. */
    adc->PRESCALER = ADC_PRESCALER_DIV4_gc;

    /* Wait 4 * COMMON_MODE_CYCLES for common mode to settle. */
    pal_timer_delay(4 * COMMON_MODE_CYCLES);

    /* Set prescaler to old value*/
    adc->PRESCALER = prescaler_val;
}

#endif /* RZ600_230B_XPLAIN */

/* EOF */
