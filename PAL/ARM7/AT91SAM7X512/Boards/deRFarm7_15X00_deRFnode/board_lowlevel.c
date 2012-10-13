/**
 * @file board_lowlevel.c
 *
 * @brief PAL board specific functionality
 *
 * This file provides low-level initialization function that get called
 * on startup. LowLevelInit() is called in board_startup.S
 *
 * $Id: board_lowlevel.c,v 1.1.6.1 2010/09/07 17:38:25 dam Exp $
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

#include "board.h"
#include "board_memories.h"
#include <utility/trace.h>

/* === Macros =============================================================== */

// Startup time of main oscillator (in number of slow clock ticks).
#define BOARD_OSCOUNT           (AT91C_CKGR_OSCOUNT & (0x40 << 8))

// USB PLL divisor value to obtain a 48MHz clock.
#define BOARD_USBDIV            AT91C_CKGR_USBDIV_1

// PLL frequency range.
#define BOARD_CKGR_PLL          AT91C_CKGR_OUT_0

// PLL startup time (in number of slow clock ticks).
#define BOARD_PLLCOUNT          (16 << 8)

// PLL MUL value.
#define BOARD_MUL               (AT91C_CKGR_MUL & (59 << 16))

// PLL DIV value.
#define BOARD_DIV               (AT91C_CKGR_DIV & 10)

// Master clock prescaler value.
#define BOARD_PRESCALER         AT91C_PMC_PRES_CLK_2


/* === Globals ============================================================== */

/* === Prototypes =========================================================== */

/* === Implementation ======================================================= */

//------------------------------------------------------------------------------
/// Default spurious interrupt handler
//------------------------------------------------------------------------------
void defaultSpuriousHandler( void )
{
    TRACE_ERROR("\r\nspurious irq - pending: 0x%08X", AT91C_BASE_AIC->AIC_IPR);
    // while(1);
}

//------------------------------------------------------------------------------
/// Default handler for fast interrupt requests
//------------------------------------------------------------------------------
void defaultFiqHandler( void )
{
    TRACE_ERROR("def FIQ Handler \r\n");
    while (1);
}

//------------------------------------------------------------------------------
/// Default handler for standard interrupt requests
//------------------------------------------------------------------------------
void defaultIrqHandler( void )
{
    TRACE_ERROR("def IRQ Handler \r\n");
    while (1);
}

//------------------------------------------------------------------------------
/// Performs the low-level initialization of the chip. This includes EFC, master
/// clock, AIC & watchdog configuration, as well as memory remapping.
//------------------------------------------------------------------------------
void LowLevelInit( void )
{
    unsigned char i;

    // Set flash wait states in the EFC
    // 48MHz = 1 wait state
    AT91C_BASE_EFC0->EFC_FMR = AT91C_MC_FWS_1FWS;
    AT91C_BASE_EFC1->EFC_FMR = AT91C_MC_FWS_1FWS;

    // Initialize main oscillator
    AT91C_BASE_PMC->PMC_MOR = BOARD_OSCOUNT | AT91C_CKGR_MOSCEN;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS));

    // Initialize PLL at 96MHz and USB clock to 48MHz
    AT91C_BASE_PMC->PMC_PLLR = BOARD_USBDIV | BOARD_CKGR_PLL | BOARD_PLLCOUNT
                               | BOARD_MUL | BOARD_DIV;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));

    // Wait for the master clock if it was already initialized
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

    // Switch to slow clock + prescaler
    AT91C_BASE_PMC->PMC_MCKR = BOARD_PRESCALER;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

    // Switch to fast clock + prescaler
    AT91C_BASE_PMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

    // Initialize AIC
    AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;

    // set a handler for each possible irq source
    AT91C_BASE_AIC->AIC_SVR[0] = (unsigned int) defaultFiqHandler;
    for (i = 1; i < 31; i++) {

        AT91C_BASE_AIC->AIC_SVR[i] = (unsigned int) defaultIrqHandler;
    }
    AT91C_BASE_AIC->AIC_SPU = (unsigned int) defaultSpuriousHandler;

    // Unstack nested interrupts
    for (i = 0; i < 8 ; i++) {

        AT91C_BASE_AIC->AIC_EOICR = 0;
    }

    // enable all interrupts - done later in the PAL
    //AT91C_BASE_AIC->AIC_IECR = AT91C_ALL_INT;

    // Enable Debug mode
    AT91C_BASE_AIC->AIC_DCR = AT91C_AIC_DCR_PROT;

    // Watchdog initialization
    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;

    // Remap the internal SRAM at 0x0
    BOARD_RemapRam();

    // Disable RTT and PIT interrupts (potential problem when program A
    // configures RTT, then program B wants to use PIT only, interrupts
    // from the RTT will still occur since they both use AT91C_ID_SYS)
    AT91C_BASE_RTTC->RTTC_RTMR &= ~(AT91C_RTTC_ALMIEN | AT91C_RTTC_RTTINCIEN);
    AT91C_BASE_PITC->PITC_PIMR &= ~AT91C_PITC_PITIEN;

    // switch on DBGU
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
}

