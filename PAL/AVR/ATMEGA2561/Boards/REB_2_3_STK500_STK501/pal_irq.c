/**
 * @file pal_irq.c
 *
 * @brief PAL IRQ functionality
 *
 * This file contains functions to initialize, enable, disable and install
 * handler for the transceiver interrupts.
 *
 * $Id: pal_irq.c 21904 2010-04-26 14:42:31Z sschneid $
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

#if (BOARD_TYPE == REB_2_3_STK500_STK501)

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

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Initializes the transceiver main interrupt
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver main interrupt
 *
 * @param trx_irq_cb Callback function for the transceiver main
 * interrupt
 */
void pal_trx_irq_init(FUNC_PTR trx_irq_cb)
{
    irq_hdl_trx = (irq_handler_t)trx_irq_cb;

    /* Init RX TIME STAMP interrupt */
    /* The input capture interrupt of timer is disabled */
    TIMSK1 &= ~(_BV(ICIE1));
    /* Rising edge on input capture pin used to trigger IRQ */
    TCCR1B |= (_BV(ICES1));
    /* Input capture flag is cleared */
    TIFR1 |= (_BV(ICF1));
}


/**
 * @brief ISR for transceiver's main interrupt
 */
ISR(TRX_IRQ)
{
    irq_hdl_trx();
}

#endif /* REB_2_3_STK500_STK501 */

/* EOF */
