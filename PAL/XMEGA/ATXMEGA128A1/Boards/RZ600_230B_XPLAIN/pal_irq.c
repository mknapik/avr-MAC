/**
 * @file pal_irq.c
 *
 * @brief PAL IRQ functionality
 *
 * This file contains functions to initialize, enable, disable and install
 * handler for the transceiver interrupts.
 *
 * $Id: pal_irq.c 21966 2010-04-30 08:44:44Z uwalter $
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

#if (BOARD_TYPE == RZ600_230B_XPLAIN)

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
    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for spurious
     * interrupts, that can pop up the moment they are enabled
     */
    irq_hdl_trx = (irq_handler_t)trx_irq_cb;

        /* Rising edge on IRQ pin used to trigger IRQ */
        PORTD.PIN2CTRL = PORT_ISC0_bm;

        /* Set pin 2 as source for port interrupt 0 */
        PORTD.INT0MASK = PIN2_bm;

        /* Clear pending interrupts */
        PORTD.INTFLAGS = PORT_INT0IF_bm;
}


/**
 * @brief ISR for transceiver's main interrupt
 */
ISR(TRX_MAIN_ISR_VECTOR)
{
    /* clear capture interrupt */
    TCD1_INTFLAGS |= TC1_CCAIF_bm;

    irq_hdl_trx();
}

#endif /* RZ600_230B_XPLAIN */

/* EOF */
