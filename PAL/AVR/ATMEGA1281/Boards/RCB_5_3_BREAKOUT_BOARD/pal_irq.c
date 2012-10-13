/**
 * @file pal_irq.c
 *
 * @brief PAL IRQ functionality
 *
 * This file contains functions to initialize, enable, disable and install
 * handler for the transceiver interrupts.
 *
 * $Id: pal_irq.c 21807 2010-04-23 10:27:01Z sschneid $
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

#include <stdint.h>
#include "pal.h"
#include "pal_boardtypes.h"

#if (BOARD_TYPE == RCB_5_3_BREAKOUT_BOARD)

/* === Types ============================================================== */

/**
 * This is a typedef of the function which is called from the transceiver ISR
 */
typedef void (*irq_handler_t)(void);

/* === Globals ============================================================= */

/*
 * Function pointers to store the callback function of
 * the transceiver interrupt
 */
static irq_handler_t irq_hdl_trx;
#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
static irq_handler_t irq_hdl_trx_tstamp;
#endif

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Initializes the transceiver main interrupt
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver interrupt
 *
 * @param trx_irq_cb Callback function for the main transceiver interrupt
 */
void pal_trx_irq_init(FUNC_PTR trx_irq_cb)
{
    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for spurious
     * interrupts, that can pop up the moment they are enabled
     */
    irq_hdl_trx = (irq_handler_t)trx_irq_cb;

    /* Init Main TRX interrupt */
    /* Select rising edge */
    EICRA |= _BV(ISC00) | _BV(ISC01);
    /* clear pending interrupt */
    EIFR = _BV(INTF0);
}


#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
/**
 * @brief Initializes the transceiver timestamp interrupt
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver timestamp interrupt
 *
 * @param trx_irq_cb Callback function for the timestamp transceiver interrupt
 */
void pal_trx_irq_init_tstamp(FUNC_PTR trx_irq_cb)
{
    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for spurious
     * interrupts, that can pop up the moment they are enabled
     */
    irq_hdl_trx_tstamp = (irq_handler_t)trx_irq_cb;

    /* Init RX TIME STAMP interrupt */
    /* The input capture interrupt of timer is disabled */
    TIMSK1 &= ~(_BV(ICIE1));
    /* Rising edge on input capture pin used to trigger IRQ */
    TCCR1B |= (_BV(ICES1));
    /* Input capture flag is cleared */
    TIFR1 |= (_BV(ICF1));
}
#endif


/**
 * @brief ISR for transceiver's main interrupt
 */
ISR(TRX_MAIN_ISR_VECTOR)
{
    irq_hdl_trx();
}


/**
 * @brief ISR for transceiver's RX TIME STAMP interrupt
 */
ISR(TRX_TSTAMP_ISR_VECTOR)
{
#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
    irq_hdl_trx_tstamp();
#endif
}

#endif /* RCB_5_3_BREAKOUT_BOARD */

/* EOF */
