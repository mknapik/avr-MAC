/**
 * @file pal_board.c
 *
 * @brief PAL board specific functionality
 *
 * This file implements PAL board specific functionality.
 *
 * $Id: pal_board.c,v 1.1.6.1 2010/09/07 17:38:25 dam Exp $
 *
 *  @author
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
/*
 * 'sys_time' is a entity of timer module which is given to other modules
 * through the interface file below. 'sys_time' is required to obtain the
 * frame timestamp
 */
#include "pal_timer.h"

#if (BOARD_TYPE == deRFusb_14E001)

/* === Macros =============================================================== */

#define SCBR_FIELD_POS_IN_CSR_REG          (8)


/* === Globals ============================================================== */

/* === Prototypes =========================================================== */

static void initialize_timer_channel(uint8_t timer_clk_src_prescaler,
                                     uint8_t timer_id);
void trx_interface_init(void);

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
     * the timer channel 0 is latched with the timer counter value. The
     * 'sys_time' is concatenated to the value of capture register to generate
     * the time stamp of the received frame.
     *      'sys_time'    'Capture register'
     *  |-----------------|-----------------| => 32 bit timestamp
     *        16 bits           16 bits
     */
    *timestamp = (uint32_t)sys_time << (uint32_t)16;
    *timestamp |= TIME_STAMP_REGISTER;

    /* The timeout is pre-scaled according to the clock period. */
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
     * The clock to PIO A is enabled. This is necessary, as only
     * when the clock is provided the PIO starts functioning.
     */
    PERIPHERAL_CLOCK_ENABLE(AT91C_ID_PIOA);

    /* The following pins are output pins.  */
    PIN_SET_AS_PIO_OUTPUT(RST, PIO_A);
    PIN_SET_AS_PIO_OUTPUT(SLP_TR, PIO_A);
}



/*
 * This function is called by timer_init() to perform the non-generic portion
 * of the initialization of the timer module.
 */
void timer_init_non_generic(void)
{
    initialize_timer_channel(TC_CLKS_MCK32, AT91C_ID_TC0);

    /* The clock for timer channel 0 is enabled here. */
    PERIPHERAL_CLOCK_ENABLE(AT91C_ID_TC0);

    /* The clock is enabled at the timer level. */
    AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN;

    /*
     * The clock for timer channel 1 will be enabled only when a high
     * priority timer is requested to be started, as until that time, the timer
     * channel 1 is not used at all. Also the overall power consumption
     * can be reduced by clocking a peripheral only when required.
     */
#ifdef ENABLE_HIGH_PRIO_TMR
    initialize_timer_channel(TC_CLKS_MCK32, AT91C_ID_TC1);
#endif  /* ENABLE_HIGH_PRIO_TMR */

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
    AIC_ENABLE(AT91C_ID_TC0);

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
    PIN_SET((LED_PINS), PIO_A);     /* Low active */
    PIN_SET_AS_PIO_OUTPUT((LED_PINS), PIO_A);
}



/**
 * @brief Control LED status
 *
 * param led_no LED ID
 * param led_setting LED_ON, LED_OFF, LED_TOGGLE
 */
void pal_led(led_id_t led_no, led_action_t led_setting)
{
    uint32_t led_pin;
    uint32_t pin_status;

    switch (led_no)
    {
        case LED_0: led_pin = AT91C_PIO_PA16; break;
        case LED_1: led_pin = AT91C_PIO_PA17; break;
        case LED_2: led_pin = AT91C_PIO_PA18; break;
        default: return;
    }

    if (led_setting == LED_ON)
    {
        PIN_CLEAR(led_pin, PIO_A);      /* Low active */
    }
    else if (led_setting == LED_OFF)
    {
        PIN_SET(led_pin, PIO_A);
    }
    else if (led_setting == LED_TOGGLE)
    {
        PIN_GET_STATUS(pin_status, led_pin, PIO_A);

        if (pin_status)
        {
            PIN_CLEAR(led_pin, PIO_A);  /* Low active */
        }
        else
        {
            PIN_SET(led_pin, PIO_A);
        }
    }
}



/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    // there is no button!
}



/**
 * @brief Read button
 *
 * param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    // there is no button!
    return BUTTON_OFF;
}



/**
 * @brief Initializes the transceiver interface
 *
 * This function initializes the transceiver interface.
 * This board uses SPI0.
 */
void trx_interface_init(void)
{
    /*
     * The PIO control is disabled for the SPI pins MISO, MOSI, SCK and the
     * transceiver interrupt pin and the SPI module control for these pins
     * is enabled.
     */
    PIN_SET_AS_PERIPHERAL_A(MISO, PIO_A);
    PIN_SET_AS_PERIPHERAL_A(MOSI, PIO_A);
    PIN_SET_AS_PERIPHERAL_A(SCK, PIO_A);

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
    PIN_SET_AS_PIO_OUTPUT(SEL, PIO_A);

    /* Clock to the SPI0 peripheral is enabled. */
    PERIPHERAL_CLOCK_ENABLE(AT91C_ID_SPI);

    /* The controller is configured to be master. */
    AT91C_BASE_SPI->SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_MODFDIS |
                               (AT91C_SPI_PCS & SS_ENABLE));

    /*
     * SPI mode 0 (clock polarity = 0 and clock phase = 1) is selected. The
     * transaction width is set to 8 bits. The SCBR register of the SPI module
     * is written with the divider required for generation of the baud rate. It
     * is calculated as follows. Baud rate = MCK / SPI_BR_DIVIDER.
     */
    AT91C_BASE_SPI->SPI_CSR[2] = (AT91C_SPI_NCPHA |
                       (AT91C_SPI_BITS & AT91C_SPI_BITS_8) |
                       (AT91C_SPI_SCBR & (SPI_BR_DIVIDER << SCBR_FIELD_POS_IN_CSR_REG)));

    /* The SPI is enabled. */
    AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIEN;
}

#endif /* deRFusb_14E001 */

/* EOF */

