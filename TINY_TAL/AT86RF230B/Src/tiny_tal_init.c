/**
 * @file tiny_tal_init.c
 *
 * @brief This file implements functions for initializing TAL.
 *
 * $Id: tiny_tal_init.c 22801 2010-08-09 08:44:18Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "tiny_tal.h"
#include "ieee_const.h"
#include "tiny_tal_pib.h"
#include "tiny_tal_irq_handler.h"
#include "stack_config.h"
#include "pal.h"
#include "tiny_tal.h"
#include "tiny_tal_internal.h"
#include "tiny_tal_constants.h"
#include "at86rf230b.h"
#ifdef NON_BLOCKING_SPI
#include "tiny_tal_rx.h"
#endif

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */

static void generate_rand_seed(void);
static retval_t trx_init(void);
static void trx_config(void);
static void trx_config_csma(void);
static retval_t trx_reset(void);
static retval_t internal_tal_reset(bool set_default_pib, bool perform_rand_seed);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Initializes the TAL
 *
 * This function is called to initialize the TAL. The transceiver is
 * initialized, the TAL PIBs are set to their default values, and the TAL state
 * machine is set to TAL_IDLE state.
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
retval_t tal_init(void)
{
    /* Init the PAL and by this means also the transceiver interface */
    if (pal_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    if (trx_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

#if (EXTERN_EEPROM_AVAILABLE == 1)
    pal_ps_get(EXTERN_EEPROM, EE_IEEE_ADDR, 8, &tal_pib_IeeeAddress);
#else
    pal_ps_get(INTERN_EEPROM, EE_IEEE_ADDR, 8, &tal_pib_IeeeAddress);
#endif

    /*
     * Do the reset stuff.
     * Set the default PIBs.
     * Do not generate random seed again.
     */
    if (internal_tal_reset(true, false) != MAC_SUCCESS)
    {
        return FAILURE;
    }

    pal_trx_reg_read(RG_IRQ_STATUS);    /* clear pending irqs, dummy read */

    /*
     * Configure interrupt handling.
     * Install a handler for the transceiver interrupt.
     */
    pal_trx_irq_init((FUNC_PTR)trx_irq_handler_cb);
    pal_trx_irq_en();     /* Enable transceiver interrupts. */

    return MAC_SUCCESS;
} /* tal_init() */


/**
 * @brief Initializes the transceiver
 *
 * This function is called to initialize the transceiver.
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
static retval_t trx_init(void)
{
    tal_trx_status_t trx_status;
    uint8_t poll_counter = 0;

    PAL_RST_HIGH();
    PAL_SLP_TR_LOW();

    pal_timer_delay(P_ON_TO_CLKM_AVAILABLE);

    // apply reset pulse
    PAL_RST_LOW();
    pal_timer_delay(RST_PULSE_WIDTH_US);
    PAL_RST_HIGH();

    /* Verify that TRX_OFF can be written */
    do
    {
        if (poll_counter == 0xFF)
        {
            return FAILURE;
        }
        poll_counter++;
        /* Check if AT86RF230 is connected; omit manufacturer id check */
    } while ((pal_trx_reg_read(RG_VERSION_NUM) != AT86RF230_REV_B) ||
             (pal_trx_reg_read(RG_PART_NUM) != AT86RF230));

    pal_trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);

    /* verify that trx has reached TRX_OFF */
    poll_counter = 0;
    do
    {
        trx_status = (tal_trx_status_t)pal_trx_bit_read(SR_TRX_STATUS);
        if (poll_counter == 0xFF)
        {
#if (DEBUG > 0)
            pal_alert();
#endif
            return FAILURE;
        }
        poll_counter++;
    } while (trx_status != TRX_OFF);

    tal_trx_status = TRX_OFF;

    return MAC_SUCCESS;
}


/**
 * @brief Internal TAL reset function
 *
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 * @param perform_rand_seed Defines whether PIB the random seed generation
 *                          needs to be done
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
static retval_t internal_tal_reset(bool set_default_pib, bool perform_rand_seed)
{
    if (trx_reset() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /* Write the transceiver values except of the CSMA seed. */
    trx_config();

    if (perform_rand_seed)
    {
        /*
         * Generate a seed for the random number generator in function rand().
         * This is required (for example) as seed for the CSMA-CA algorithm.
         */
        generate_rand_seed();
    }

    /* Once we have initialized a proper seed for rand(), we
     * can now initialize the transceiver's CSMA seed. */
    trx_config_csma();

    if (set_default_pib)
    {
        /* Set the default PIB values */
        init_tal_pib(); /* implementation can be found in 'tal_pib.c' */
    }
    else
    {
        /* nothing to do - the current TAL PIB attribute values are used */
    }

    /*
     * Write all PIB values to the transceiver
     * that are needed by the transceiver itself.
     */
    write_all_tal_pib_to_trx(); /* implementation can be found in 'tal_pib.c' */

    /* Reset TAL variables. */
    tal_state = TAL_IDLE;

    return MAC_SUCCESS;
}


/**
 * @brief Configures the transceiver
 *
 * This function is called to configure the transceiver after reset.
 */
static void trx_config(void)
{
    /* Set pin driver strength */
    pal_trx_reg_write(RG_TRX_CTRL_0, ((CLKM_2mA << 6) |
                      (CLKM_2mA << 4) | CLKM_1MHz)); // fast change

    pal_trx_bit_write(SR_AACK_SET_PD, PD_ACK_BIT_SET_ENABLE); /* 1 == frame pending bit is always set to 1 */
    pal_trx_bit_write(SR_TX_AUTO_CRC_ON, TX_AUTO_CRC_ENABLE);    /* enable auto crc */
    pal_trx_reg_write(RG_IRQ_MASK, TRX_IRQ_TRX_END); /* enable TRX_END interrupt */
}



/**
 * @brief Configures the transceiver's CSMA seed
 *
 * This function is called to configure the transceiver's CSMA seed after reset.
 * it needs to be called in conjunction with funciton trx_config(), but
 * it needs be assured that a seed for function rand() had been generated before.
 */
static void trx_config_csma(void)
{
    uint16_t rand_value;

    /*
     * Init the SEED value of the CSMA backoff algorithm.
     */
    rand_value = (uint16_t)rand();
    pal_trx_reg_write(RG_CSMA_SEED_0, (uint8_t)rand_value);
    pal_trx_bit_write(SR_CSMA_SEED_1, (uint8_t)(rand_value >> 8));

    /*
     * To make sure that the CSMA seed is properly set within the transceiver,
     * put the trx to sleep briefly and wake it up again.
     */
    tal_trx_sleep(SLEEP_MODE_1);

    tal_trx_wakeup();
}



/**
 * @brief Reset transceiver
 */
static retval_t trx_reset(void)
{
    tal_trx_status_t trx_status;
    uint8_t poll_counter = 0;
#if (EXTERN_EEPROM_AVAILABLE == 1)
    uint8_t xtal_trim_value;
#endif

    /* Get trim value for 16 MHz xtal; needs to be done before reset */
#if (EXTERN_EEPROM_AVAILABLE == 1)
    pal_ps_get(EXTERN_EEPROM, EE_XTAL_TRIM_ADDR, 1, &xtal_trim_value);
#endif

    /* trx might sleep, so wake it up */
    PAL_SLP_TR_LOW();
    pal_timer_delay(SLEEP_TO_TRX_OFF_US);

    PAL_RST_LOW();
    pal_timer_delay(RST_PULSE_WIDTH_US);
    PAL_RST_HIGH();

    /* verify that trx has reached TRX_OFF */
    do
    {
        trx_status = (tal_trx_status_t)pal_trx_bit_read(SR_TRX_STATUS);
        poll_counter++;
        if (poll_counter > 250)
        {
#if (DEBUG > 0)
            pal_alert();
#endif
            return FAILURE;
        }

    } while (trx_status != TRX_OFF);

    tal_trx_status = TRX_OFF;

    // Write 16MHz xtal trim value to trx.
    // It's only necessary if it differs from the reset value.
#if (EXTERN_EEPROM_AVAILABLE == 1)
    if (xtal_trim_value != 0x00)
    {
        pal_trx_bit_write(SR_XTAL_TRIM, xtal_trim_value);
    }
#endif

    return MAC_SUCCESS;
}


/**
 * @brief Resets TAL state machine and sets the default PIB values if requested
 *
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 */
retval_t tal_reset(bool set_default_pib)
{
    if (set_default_pib)
    {
        /* Set the default PIB values */
        init_tal_pib(); /* implementation can be found in 'tal_pib.c' */
    }
    else
    {
        /* nothing to do - the current TAL PIB attribute values are used */
    }

    if (trx_reset() != MAC_SUCCESS)
    {
        return FAILURE;
    }
    trx_config();
    trx_config_csma();

#if (NUMBER_OF_TAL_TIMERS > 0)
    /* Clear all running TAL timers. */
    {
        uint8_t timer_id;

        ENTER_CRITICAL_REGION();
        for (timer_id = TAL_FIRST_TIMER_ID; timer_id <= TAL_LAST_TIMER_ID;
             timer_id++)
        {
            pal_timer_stop(timer_id);
        }
        LEAVE_CRITICAL_REGION();
    }
#endif

    /*
     * Write all PIB values to the transceiver
     * that are needed by the transceiver itself.
     */
    write_all_tal_pib_to_trx(); /* implementation can be found in 'tal_pib.c' */

    tal_state = TAL_IDLE;

    /*
     * Configure interrupt handling.
     * Install a handler for the transceiver interrupt.
     */
    pal_trx_irq_init((FUNC_PTR)trx_irq_handler_cb);
    /* The pending transceiver interrupts on the microcontroller are cleared. */
    pal_trx_irq_flag_clr();
    pal_trx_irq_en();     /* Enable transceiver interrupts. */

    return MAC_SUCCESS;
}



/**
 * @brief Generates a 16-bit random number used as initial seed for srand()
 */
static void generate_rand_seed(void)
{
    uint16_t seed;
    /*
     * For AT86RF230B  bysed systems the random seed needs to be generated
     * by the PAL, since there is no tranceiver based random number generator
     * available.
     */
    seed = pal_generate_rand_seed();

    /* Set the seed for the random number generator. */
    srand(seed);
}
/* EOF */

