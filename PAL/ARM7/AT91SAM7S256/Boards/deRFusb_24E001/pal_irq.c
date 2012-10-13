/**
 * @file pal_irq.c
 *
 * @brief PAL IRQ functionality
 *
 * This file contains functions to initialize, enable, disable and install
 * handler for the transceiver interrupts.
 *
 * $Id: pal_irq.c,v 1.1.6.1 2010/09/07 17:38:25 dam Exp $
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

/* === Includes ============================================================= */

#include <stdint.h>
#include "pal.h"
#include "pal_boardtypes.h"

#if (BOARD_TYPE == deRFusb_24E001)


/* === Types ============================================================== */

/**
 * This is a typedef of the function which is called from the transceiver ISR
 */
typedef void (*irq_handler_t)(void);

/* === Globals ============================================================== */

/* pin references */
const Pin pinIRQ  = TRX_IRQ_PIN;
const Pin pinRXTS = TRX_TS_IRQ_PIN;


/*
 * Function pointers to store the callback function of
 * the transceiver interrupt
 */

/** Function pointer to store callback for transceiver main (TX_END) interrupt. */
static irq_handler_t irq_hdl_trx;

#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) || (defined DOXYGEN)
/** Function pointer to store callback for transceiver timestamp (RX_START) interrupt. */
static irq_handler_t irq_hdl_trx_tstamp;
#endif


/* === Prototypes =========================================================== */

static void trx_isr(void);

#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) || (defined DOXYGEN)
static void trx_ts_isr(void);
#endif

/* === Implementation ======================================================= */

/**
 * @brief Initializes the transceiver interrupts
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver interrupts
 *
 * @param trx_irq_num Transceiver interrupt line to be initialized
 * @param trx_irq_cb Callback function for the given transceiver
 * interrupt
 */
void pal_trx_irq_init(FUNC_PTR trx_irq_cb)
{
    /* disable and clear the Interrupt source */
    DISABLE_TRX_IRQ();
    CLEAR_TRX_IRQ();

    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for
     * spurious interrupts, that can pop up the moment they are enabled.
     */
    irq_hdl_trx = (irq_handler_t)trx_irq_cb;

    /* Initialize PIO Interrupt system, since Transceiver-IRQs are PIO controlled */
    PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);

    /*
     * The AIC is configured and updated with the mode for generating
     * a PIO interrupt for IRQ0, which is connected to the transceiver
     * interrupt line. Also the interrupt handler is installed for the same.
     */
    PIN_CONFIGURE_PIO_IRQ(pinIRQ, trx_isr);
}


#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) || defined(DOXYGEN)
void pal_trx_irq_init_tstamp(FUNC_PTR trx_irq_cb)
{
    /* disable and clear the Interrupt source */
    DISABLE_TRX_IRQ();
    CLEAR_TRX_IRQ();

    /* Set the handler function. */
    irq_hdl_trx_tstamp = (irq_handler_t)trx_irq_cb;

    /* enable interrupt generation */
    PIN_CONFIGURE_PIO_IRQ(pinRXTS, trx_ts_isr);
}
#endif


/**
 * @brief ISR for IRQ1
 *
 * This function is an ISR for IRQ1 interrupt (transceiver interrupt) of the
 * microcontroller. It calls the user installed handler for transceiver
 * interrupt.
 */
void trx_isr(void)
{
    irq_hdl_trx();
}


#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) || defined(DOXYGEN)
/**
 * @brief ISR for IRQ0
 *
 * This function is an ISR for IRQ0 interrupt (transceiver timestamp interrupt)
 * of the microcontroller. It calls the respective user handler.
 */
void trx_ts_isr(void)
{
    irq_hdl_trx_tstamp();
}
#endif

#endif /* deRFusb_24E001 */

/* EOF */
