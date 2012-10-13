/**
 * @file pal_irq.c
 *
 * @brief PAL IRQ functionality
 *
 * This file contains functions to initialize, enable, disable and install
 * handler for the transceiver interrupts.
 *
 * $Id: pal_irq.c 21862 2010-04-23 15:13:21Z sschneid $
 *
 */
/**
 * @author
 * Atmel Corporation: http://www.atmel.com
 * Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================= */

#include <stdint.h>
#include "pal.h"
#include "pal_boardtypes.h"

#if (BOARD_TYPE == REB_5_0_REX_ARM_REV_3)

/* === Types ============================================================== */

/**
 * This is a typedef of the function which is called from the transceiver ISR
 */
typedef void (*irq_handler_t)(void);

/* === Globals ============================================================== */

/*
 * Function pointers to store the callback function of
 * the transceiver interrupt
 */
static irq_handler_t irq_hdl_trx;

/* === Prototypes =========================================================== */

static void trx_isr(void);

/* === Implementation ======================================================= */

/**
 * @brief Initializes the transceiver main interrupt
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver interrupts
 *
 * @param trx_irq_cb Callback function for the given transceiver
 * interrupt
 */
void pal_trx_irq_init(FUNC_PTR trx_irq_cb)
{
    /*
     * The AIC is configured and updated with the mode for generating
     * interrupt for IRQ0, which is connected to the transceiver interrupt
     * line. Also the interrupt handler is installed for the same.
     */
    AIC_CONFIGURE(AT91C_ID_IRQ0,
                  AT91C_AIC_SRCTYPE_POSITIVE_EDGE,
                  trx_isr);

    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for
     * spurious interrupts, that can pop up the moment they are enabled.
     */
    irq_hdl_trx = (irq_handler_t)trx_irq_cb;

    /* The input capture interrupt of timer is disabled. */
    DISABLE_TRX_IRQ();

    /*
     * The interrupt bit corresponding to the trx interrupt is cleared.
     */
    CLEAR_TRX_IRQ();
}



/**
 * @brief ISR for IRQ0
 *
 * This function is an ISR for IRQ0 interrupt (transceiver interrupt) of the
 * microcontroller. It calls the user installed handler for transceiver
 * interrupt.
 */
void trx_isr(void)
{
    irq_hdl_trx();
}

#endif /* REB_5_0_REX_ARM_REV_3 */

/* EOF */
