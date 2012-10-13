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
#include "at86rf231.h"
#ifdef ENABLE_TFA
#include "tfa.h"
#endif
#ifdef NON_BLOCKING_SPI
#include "tiny_tal_rx.h"
#endif
#ifdef STB_ON_SAL
#include "sal_types.h"
#if (SAL_TYPE == AT86RF2xx)
#include "stb.h"
#endif
#endif

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */

static void generate_rand_seed(void);
static retval_t trx_init(void);
static void trx_config(void);
static void trx_config_csma(void);
static retval_t trx_reset(void);
static retval_t internal_tal_reset(bool set_default_pib);

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
     * Generate random seed.
     */
    if (internal_tal_reset(true) != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /*
     * Configure interrupt handling.
     * Install a handler for the transceiver interrupt.
     */
    pal_trx_irq_init((FUNC_PTR)trx_irq_handler_cb);
    pal_trx_irq_en();     /* Enable transceiver interrupts. */

#ifdef ENABLE_TFA
    tfa_init();
#endif

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

    /* Apply reset pulse */
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
        /* Check if AT86RF231 is connected; omit manufacturer id check */
    } while ((pal_trx_reg_read(RG_VERSION_NUM) != AT86RF231_VERSION_NUM) ||
             (pal_trx_reg_read(RG_PART_NUM) != AT86RF231_PART_NUM));

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
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
static retval_t internal_tal_reset(bool set_default_pib)
{
    if (trx_reset() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /* Write the transceiver values except of the CSMA seed. */
    trx_config();

    /*
     * Generate a seed for the random number generator in function rand().
     * This is required (for example) as seed for the CSMA-CA algorithm.
     */
    generate_rand_seed();

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
    pal_trx_bit_write(SR_PAD_IO_CLKM, PAD_CLKM_2_MA);
    pal_trx_bit_write(SR_CLKM_SHA_SEL, CLKM_SHA_DISABLE);
    pal_trx_bit_write(SR_CLKM_CTRL, CLKM_1MHZ);

    /*
     * Configuration of CSMA seed is done later in function trx_config_csma()
     * after the generation of a proper random seed for function rand();
     */

    pal_trx_bit_write(SR_AACK_SET_PD, PD_ACK_BIT_SET_ENABLE); /* ACKs for data requests, indicate pending data */
    pal_trx_bit_write(SR_RX_SAFE_MODE, RX_SAFE_MODE_ENABLE);    /* Enable buffer protection mode */
    pal_trx_bit_write(SR_IRQ_MASK_MODE, IRQ_MASK_MODE_ON); /* Enable poll mode */
    pal_trx_reg_write(RG_IRQ_MASK, TRX_IRQ_DEFAULT);    /* The TRX_END interrupt of the transceiver is enabled. */

#ifdef ANTENNA_DIVERSITY
    // Use antenna diversity
    pal_trx_bit_write(SR_ANT_CTRL, ANTENNA_DEFAULT);
    pal_trx_bit_write(SR_PDT_THRES, THRES_ANT_DIV_ENABLE);
    pal_trx_bit_write(SR_ANT_DIV_EN, ANT_DIV_ENABLE);
    pal_trx_bit_write(SR_ANT_EXT_SW_EN, ANT_EXT_SW_SWITCH_ENABLE);
#endif
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
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF
 *         FAILURE otherwise
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

    /* Apply reset pulse */
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

#ifdef STB_ON_SAL
#if (SAL_TYPE == AT86RF2xx)
    stb_restart();
#endif
#endif

    return MAC_SUCCESS;
}


/**
 * @brief Resets TAL state machine and sets the default PIB values if requested
 *
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF
 *         FAILURE otherwise
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

#ifdef ENABLE_TFA
    tfa_reset(set_default_pib);
#endif

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
 *
 * This function generates a 16-bit random number by means of using the
 * Random Number Generator from the transceiver.
 * The Random Number Generator generates 2-bit random values. These 2-bit
 * random values are concatenated to the required 16-bit random seed.
 * For further information please check the SWPM AT86RF231.
 *
 * The generated random 16-bit number is feed into function srand()
 * as initial seed.
 *
 * The transceiver state is initally set to RX_ON.
 * After the completion of the random seed generation, the
 * trancseiver is set to TRX_OFF.
 *
 * As a prerequisite the Preamble Detector must not be disabled.
 * Since this function is called right after trx_reset(), the Preamble
 * Detector has its original value, i.e. it is enabled.
 * In case this function is used at a different point of time, having the
 * proper value set must be checked additionally.
 *
 * Also in case the function is called from a different state than TRX_OFF,
 * additional trx state handling is required, such as reading the original
 * value and restoring this state after finishing the sequence.
 * Since in our case the function is called from TRX_OFF, this is not required
 * here.
 */
static void generate_rand_seed(void)
{
    uint16_t seed = 0;
    uint8_t cur_random_val = 0;

    set_trx_state(CMD_RX_ON);

    /*
     * We need to disable TRX IRQs while generating random values in RX_ON,
     * we do not want to receive frames at this point of time at all.
     */
    ENTER_TRX_REGION();

    /*
     * The 16-bit random value is generated from various 2-bit random values.
     */
    for (uint8_t i = 0; i < 8; i++)
    {
        /* Now we can safely read the 2-bit random number. */
        cur_random_val = pal_trx_bit_read(SR_RND_VALUE);
        seed = seed << 2;
        seed |= cur_random_val;
    }

    set_trx_state(CMD_FORCE_TRX_OFF);

    /*
     * Now we need to clear potential pending TRX IRQs and
     * enable the TRX IRQs again.
     */
    pal_trx_reg_read(RG_IRQ_STATUS);
    pal_trx_irq_flag_clr();
    LEAVE_TRX_REGION();

    /* Set the seed for the random number generator. */
    srand(seed);
}
/* EOF */

