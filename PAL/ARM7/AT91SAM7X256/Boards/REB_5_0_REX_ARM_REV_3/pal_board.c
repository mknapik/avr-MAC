/**
 * @file pal_board.c
 *
 * @brief PAL board specific functionality
 *
 * This file implements PAL board specific functionality.
 *
 * $Id: pal_board.c 22788 2010-08-09 06:12:39Z sschneid $
 *
 *  @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
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

#if (BOARD_TYPE == REB_5_0_REX_ARM_REV_3)

/* === Globals ============================================================== */


/* === Prototypes =========================================================== */

static void initialize_timer_channel(uint8_t timer_clk_src_prescaler,
                                     uint8_t timer_id);

/* === Implementation ======================================================= */

/**
 * @brief Initializes the transceiver interface
 *
 * This function initializes the transceiver interface.
 * This board uses SPI1.
 */
void trx_interface_init(void)
{
    /*
     * The PIO control is disabled for the SPI pins MISO, MOSI, SCK and the
     * transceiver interrupt pin and the SPI module control for these pins
     * is enabled.
     *
     * Please note that these SPI pins from SPI1 require setting of
     * Peripheral B.
     */
    AT91C_BASE_PIOA->PIO_BSR = (MISO | MOSI | SCK);
    AT91C_BASE_PIOA->PIO_PDR = (MISO | MOSI | SCK);

    AT91C_BASE_PIOA->PIO_ASR = TRX_INTERRUPT_PIN;
    AT91C_BASE_PIOA->PIO_PDR = TRX_INTERRUPT_PIN;

    /*
     * Select line will be used as a GPIO. The controller recognizes 1 cycle
     * of SPI transaction as 8 bit, hence deactivates the chip select after 1
     * cycle. But the transceiver needs the chip select to be active for two
     * cycles (In one cycle the transceiver gets to know about the address of
     * register or memory location and the kind of operation to be performed
     * on that memory location. And in the second cycle its performs the
     * specified operation). To achieve this, the chip select line will be
     * manually pulled low and high (after the transaction). Hence the SEL line
     * is configured as PIO and the SPI control of SEL is disabled.
     */
    /* Set SEL as output pin. */
    AT91C_BASE_PIOA->PIO_OER = SEL;
    AT91C_BASE_PIOA->PIO_PER = SEL;

    /*
     * Used peripheral interface is SPI1.
     * The clock to the utilized SPI 1 peripheral is enabled.
     */
    AT91C_BASE_PMC->PMC_PCER = _BV(AT91C_ID_SPI1);

    /* The controller is configured to be master. */
    AT91C_BASE_SPI_USED->SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | (AT91C_SPI_PCS & SS_ENABLE));

    /*
     * SPI mode 0 (clock polarity = 0 and clock phase = 1) is selected. The
     * transaction width is set to 8 bits. The SCBR register of the SPI module
     * is written with the divider required for generation of the baud rate. It
     * is calculated as follows. Baud rate = MCK / SPI_BR_DIVIDER.
     */
    AT91C_BASE_SPI_USED->SPI_CSR[2] = (AT91C_SPI_NCPHA |
                            (AT91C_SPI_BITS & AT91C_SPI_BITS_8) |
                            (AT91C_SPI_SCBR & (SPI_BR_DIVIDER << SCBR_FIELD_POS_IN_CSR_REG)));

    /* The SPI is enabled. */
    AT91C_BASE_SPI_USED->SPI_CR = AT91C_SPI_SPIEN;
}



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
     * the timer channel 0 is latched with the timer counter value. The
     * 'sys_time' is concatenated to the value of capture register to generate
     * the time stamp of the received frame.
     *      'sys_time'    'Capture register'
     *  |-----------------|-----------------| => 32 bit timestamp
     *        16 bits           16 bits
     */
    *timestamp = (uint32_t)sys_time << (uint32_t)16;
    *timestamp |= TIME_STAMP_REGISTER;

    /* The timeout is pre scaled accroding to the clock period. */
    *timestamp = (uint32_t)(*timestamp * CLOCK_PERIOD);
}



/**
 * @brief Calibrates the internal RC oscillator
 *
 * This function calibrates the internal RC oscillator.
 *
 * @return True
 */
bool pal_calibrate_rc_osc(void)
{
    return (true);
}



/**
 * @brief Initializes the GPIO pins
 *
 * This function is used to initialize the port pins used to connect
 * the microcontroller to transceiver.
 */
void gpio_init(void)
{
    /*
     * The clock to PIO A and PIO B are enabled. This is necessary, as only
     * when the clock is provided the PIO starts functioning.
     */
    AT91C_BASE_PMC->PMC_PCER = _BV(AT91C_ID_PIOA | AT91C_ID_PIOB);

    /* The following pins are output pins.  */
    AT91C_BASE_PIOA->PIO_OER = RST;
    AT91C_BASE_PIOA->PIO_OER = SLP_TR;
}



/*
 * This function is called by timer_init() to perform the non-generic portion
 * of the initialization of the timer module.
 */
void timer_init_non_generic(void)
{
    initialize_timer_channel(TC_CLKS_MCK32, AT91C_ID_TC0);

    /* The clock for timer channel 0 is enabled here. */
    AT91C_BASE_PMC->PMC_PCER = _BV(AT91C_ID_TC0);

    /* The clock is enabled at the timer level. */
    AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN;

#ifdef ENABLE_HIGH_PRIO_TMR
    initialize_timer_channel(TC_CLKS_MCK32, AT91C_ID_TC1);
#endif  /* ENABLE_HIGH_PRIO_TMR */

    /*
     * The clock for timer channel 1 will be enabled only when a high
     * priority timer is requested to be started, as until that time, the timer
     * channel 1 is not used at all. Also the overall power consumption
     * can be reduced by clocking a perpheral only when required.
     */

    /* The AIC is set up for the timer interrupts. */
    AIC_CONFIGURE(AT91C_ID_TC0,
                  AT91C_AIC_SRCTYPE_POSITIVE_EDGE,
                  timer_ch0_irq_handler);

#ifdef ENABLE_HIGH_PRIO_TMR
    AIC_CONFIGURE(AT91C_ID_TC1,
                  AT91C_AIC_SRCTYPE_POSITIVE_EDGE,
                  timer_ch1_irq_handler);
#endif  /* ENABLE_HIGH_PRIO_TMR */

    /*
     * The timer overflow interrupts for the timer channel 0 is enabled, as
     * channel 0 is used for regular timer implementation.
     */
    AT91C_BASE_TC0->TC_IER = AT91C_TC_COVFS;

    /* The timer channel 0 interrupt in AIC is enabled. */
    AT91C_BASE_AIC->AIC_IECR = _BV(AT91C_ID_TC0);

    /*
     * The PB23 pin of PIOB is configured as TIOA line of the timer channel 0.
     * This line is connected to the transceiver interrupt. Whenever an
     * interrupt occurs, the TC_RA register will latch the TC_CV value, which
     * is useful in timestamp generation.
     *
     * The pin requires setting of Peripheral A.
     */
    AT91C_BASE_PIOB->PIO_ASR = TIMER_INPUT_CAPTURE_LINE;
    AT91C_BASE_PIOB->PIO_PDR = TIMER_INPUT_CAPTURE_LINE;

    /*
     * Settings are done to load the RA and RB with counter value at rising and
     * falling edge of the TIOA line (transceiver interrupt) respectively. This
     * is useful in frame timestamp generation.
     *
     * Although RB is not used, RB is also configured, as RA can be loaded only
     * if it is not loaded or RB is loaded. This configuration will load RA on
     * the rising edge and RB on the falling edge of the transceiver interrupt
     * respectively, making RA ready for the loading the counter value on next
     * interrupt.
     */
    AT91C_BASE_TC0->TC_CMR |= ((AT91C_TC_LDRA & AT91C_TC_LDRA_RISING) |
                               (AT91C_TC_LDRB & AT91C_TC_LDRB_FALLING));

    /* Only the timer channel 0 is triggered. */
    AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
}



/**
 * @brief Configures the timer channel
 *
 * This function configures the timer channel. It disables the clock to the
 * timer channel at the timer level, disables all the timer interrupts and
 * programs the prescaler for timer clock.
 *
 * @param timer_clk_src_prescaler Value to be written in the TCCLKS field
 *                                of the TC_CMR register, to select the
 *                                prescaler for the main clock which is
 *                                the timer clock source
 * @param timer_id ID of the timer channel
 */
static void initialize_timer_channel(uint8_t timer_clk_src_prescaler,
                                     uint8_t timer_id)
{
    uint32_t tc_status;
    AT91PS_TC tc_pt;

    if (AT91C_ID_TC0 == timer_id)
    {
        /* Pointer to the TC0 base. */
        tc_pt = AT91C_BASE_TC0;
    }
#ifdef ENABLE_HIGH_PRIO_TMR
    else
    {
        /* Pointer to the TC1 base. */
        tc_pt = AT91C_BASE_TC1;
    }
#endif  /* ENABLE_HIGH_PRIO_TMR */

    /* The clock and the interrupts of the timer channel are disabled. */
    tc_pt->TC_CCR = AT91C_TC_CLKDIS;
    tc_pt->TC_IDR = ALL_TIMER_INTERRUPTS_MASK;

    /* The status register is read to clear any pending interrupt. */
    tc_status = tc_pt->TC_SR;

    /*
     * Done to avoid compiler warning about variable being not used after
     * setting.
     */
    tc_status = tc_status;

    /* The prescaler for the timer clock source (main clock) is selected. */
    tc_pt->TC_CMR = timer_clk_src_prescaler;
}



/**
 * @brief Initialize LEDs
 */
void pal_led_init(void)
{
    /* Set LED pins to output. */
    AT91C_BASE_PIOB->PIO_OER = LED_PINS;
    AT91C_BASE_PIOB->PIO_SODR = LED_PINS;
}



/**
 * @brief Control LED status
 *
 * @param led_no LED ID
 * @param led_setting LED_ON, LED_OFF, LED_TOGGLE
 */
void pal_led(led_id_t led_no, led_action_t led_setting)
{
    uint32_t led_pin;

    switch (led_no)
    {
        case LED_0: led_pin = AT91C_PIO_PB19; break;
        case LED_1: led_pin = AT91C_PIO_PB20; break;
        case LED_2: led_pin = AT91C_PIO_PB21; break;
        case LED_3: led_pin = AT91C_PIO_PB22; break;
        default: return;
    }

    /* LEDs are low active. */
    if (led_setting == LED_ON)
    {
        AT91C_BASE_PIOB->PIO_CODR = led_pin;
    }
    else if (led_setting == LED_OFF)
    {
        AT91C_BASE_PIOB->PIO_SODR = led_pin;
    }
    else    /* LED_TOGGLE */
    {
        if (AT91C_BASE_PIOB->PIO_ODSR & led_pin)
        {
            AT91C_BASE_PIOB->PIO_CODR = led_pin;
        }
        else
        {
            AT91C_BASE_PIOB->PIO_SODR = led_pin;
        }
    }
}

#endif /* REB_5_0_REX_ARM_REV_3 */

/* EOF */
