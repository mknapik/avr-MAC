/**
 * @file pal.c
 *
 * @brief General PAL functions for ARM7 MCUs
 *
 * This file implements generic PAL function for ARM7 MCUs.
 *
 * $Id: pal.c 21606 2010-04-14 14:09:54Z uwalter $
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
#include <stdbool.h>
#include "pal.h"
#include "pal_config.h"
#include "pal_timer.h"
#include "pal_internal.h"

/* === Globals ============================================================= */

/*
 * This section defines all global variables for the PAL
 */

/*
 * This is the most significant part of the system time.
 */
volatile uint16_t sys_time;

/* === Macros =============================================================== */

/*
 * Specifies the number of slow clock cycles multiplied by 8 for the main
 * oscillator start-up time.
 */
#define BOARD_OSCOUNT           (AT91C_CKGR_OSCOUNT & (0x40 << 8))

/* PLL frequency range. */
#define BOARD_CKGR_PLL          (AT91C_CKGR_OUT & AT91C_CKGR_OUT_0)

/*
 * Specifies the number of slow clock cycles before the LOCK bit is set in
 * PMC_SR after CKGR_PLLR is written.
 */
#define BOARD_PLLCOUNT          (AT91C_CKGR_PLLCOUNT & (16 << 8))

/* The PLL output is = ((MUL + 1) / DIV). */
#define BOARD_MUL               (AT91C_CKGR_MUL & (PLL_MULTIPLIER << 16))
#define BOARD_DIV               (AT91C_CKGR_DIV & PLL_DIVIDER)

/*
 * The value to be written in the CSS field of the PMC_MCKR register to
 * select the PLL clock as the input to the PMC.
 */
#define PLL_CLOCK_FOR_PMC       (AT91C_PMC_CSS_PLL_CLK)

/*
 * The value by which the selected clock is prescaled. (In this case PLL output
 * is selected as input clock to the PMC).
 */
#define BOARD_PRESCALER         (PMC_PRESCALER)

/* Mask to act on all the interrupts at once. */
#define ALL_INTERRUPTS_MASK     (0xFFFFFFFF)

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Initialization of PAL
 *
 * This function initializes the PAL.
 *
 * @return MAC_SUCCESS  if PAL initialization is successful, FAILURE otherwise
  */
retval_t pal_init(void)
{
    gpio_init();
    trx_interface_init();
#ifdef CW_SUPPORTED
    TST_INIT();
#endif
    timer_init();

    return MAC_SUCCESS;
}



/**
 * @brief Services timer and sio handler
 *
 * This function calls sio & timer handling functions.
 */
void pal_task(void)
{
#if (TOTAL_NUMBER_OF_TIMERS > 0)
    timer_service();
#endif
}


/**
 * @brief Default handler for spurious interrupts
 */
void default_spurious_handler( void )
{
    pal_alert();
}



/**
 * @brief Default handler for a fiq interrupt
 */
void default_fiq_handler( void )
{
    pal_alert();
}



/**
 * @brief Default handler for a irq interrupt
 */
void default_irq_handler( void )
{
    pal_alert();
}



/**
 * @brief Returns the status of the memory which is mapped to address 0x0
 *
 * This function returns the status of the memory which is remapped at the
 * address 0x0.
 *
 * @return True, if the SRAM is present at address 0x0, false otherwise
 */
bool board_get_remap(void)
{
    uint32_t *remap = (uint32_t *) 0;
    uint32_t *ram = (uint32_t *) AT91C_ISRAM;

    uint32_t temp = *ram;
    *ram = temp + 1;
    if (*remap == *ram)
    {
        *ram = temp;
        return true;
    }
    else
    {
        *ram = temp;
        return false;
    }
}



/**
 * @brief Changes the mapping of the chip so that the remap area mirrors the
 *  internal flash
 *
 * This function changes the mapping of the chip so that the remap area mirrors
 * the internal RAM.
 */
void board_remap_ram(void)
{
    if (!board_get_remap())
    {
        AT91C_BASE_MC->MC_RCR = AT91C_MC_RCB;
    }
}



/**
 * @brief Performs low-level initialization of the chip
 *
 * This function performs the low-level initialization of the chip. It
 * initializes the on chip oscillator and alos the PMC. It installs the
 * default handlers for all the interrupts and clears all the pending
 * interrupts.
 */
void arm7_low_level_init(void)
{
    uint8_t index;

    /* Wait states for flash read and write set to 2 and 3 respectively. */
    AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_1FWS;

    /* The main oscillator is initialized. */
    AT91C_BASE_PMC->PMC_MOR = BOARD_OSCOUNT | AT91C_CKGR_MOSCEN;

    /* No operation till the main oscillator is stabilized. */
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS));

    /* The PMC is programmed to generate the required clock. */
    AT91C_BASE_PMC->PMC_PLLR = BOARD_CKGR_PLL | BOARD_PLLCOUNT
                               | BOARD_MUL | BOARD_DIV;

    /* No operation till the PLL is locked to the final frequency. */
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));

    /* No operation till the master clock is ready. */
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

    /* Prescaler for the division of the PLL clock is selected. */
    AT91C_BASE_PMC->PMC_MCKR = BOARD_PRESCALER;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

    /*
     * The CSS field of the PMC_MCKR is programmed to select the PLL clock for
     * the usage of PMC.
     */
    AT91C_BASE_PMC->PMC_MCKR |= PLL_CLOCK_FOR_PMC;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

    /*
     * The AIC is initialized with the handlers for different kind of interrupts.
     */

    AT91C_BASE_AIC->AIC_IDCR = ALL_INTERRUPTS_MASK;

    AT91C_BASE_AIC->AIC_SVR[0] = (uint32_t) default_fiq_handler;

    for (index = 1; index < 31; index++)
    {
        AT91C_BASE_AIC->AIC_SVR[index] = (uint32_t) default_irq_handler;
    }

    AT91C_BASE_AIC->AIC_SPU = (uint32_t) default_spurious_handler;

    /*
     * The end of interrupt command register is written with a value. This
     * will result in the acknowledgement of the pending interrupts and hence
     * clearing them.
     */
    for (index = 0; index < 8 ; index++) {

        AT91C_BASE_AIC->AIC_EOICR = 0;
    }

    /* The debug mode is enabled. */
    AT91C_BASE_AIC->AIC_DCR = AT91C_AIC_DCR_PROT;

    /* Watchdog is disabled. */
    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;

    /* Remap is done. */
    board_remap_ram();
}



/**
 * @brief Get data from persistence storage
 *
 * @param[in]  ps_type Persistence storage type
 * @param[in]  start_addr Start offset within EEPROM
 * @param[in]  length Number of bytes to read from EEPROM
 * @param[out] value Data from persistence storage
 *
 * @return MAC_SUCCESS  if everything went OK else FAILURE
 */
retval_t pal_ps_get(ps_type_t ps_type, uint16_t start_addr, uint16_t length, void *value)
{
    /* Board does not has EEPROM */

    /* Keep compiler happy. */
    ps_type = ps_type;
    start_addr = start_addr;
    length = length;
    value = value;

    return FAILURE;
}



/**
 * @brief Write data to persistence storage
 *
 * @param[in]  start_addr Start address offset within EEPROM
 * @param[in]  length Number of bytes to be written to EEPROM
 * @param[in]  value Data to persistence storage
 *
 * @return MAC_SUCCESS  if everything went OK else FAILURE
 */
retval_t pal_ps_set(uint16_t start_addr, uint16_t length, void *value)
{
    /* Board does not has EEPROM */

    /* Keep compiler happy. */
    start_addr = start_addr;
    length = length;
    value = value;

    return FAILURE;
}



void pal_abort_handler(void)
{
    pal_alert();
}



/*
 * @brief Alert indication
 *
 * This Function can be used by any application to indicate an error condition.
 * The function is blocking and does never return.
 */
void pal_alert(void)
{
#if (DEBUG > 0)
    bool debug_flag = false;
#endif
    ALERT_INIT();

    while (1)
    {
        pal_timer_delay(0xFFFF);
        ALERT_INDICATE();

#if (DEBUG > 0)
        /* Used for debugging purposes only */
        if (debug_flag == true)
        {
            break;
        }
#endif
    }
}

/* EOF */
