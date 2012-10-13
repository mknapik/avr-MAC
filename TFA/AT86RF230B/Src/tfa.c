/**
 * @file tfa.c
 *
 * @brief Implementation of Transceiver Feature Access (TFA) functionality.
 *
 * $Id: tfa.c 22671 2010-07-26 09:56:57Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#ifdef ENABLE_TFA

/* === INCLUDES ============================================================ */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "tal_constants.h"
#include "at86rf230b.h"
#include "tal_internal.h"
#include "tfa.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/* Constant define for the ED scaling: register value at -35dBm */
#define CLIP_VALUE_REG                  (56)

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */

static void init_tfa_pib(void);
static void write_all_tfa_pibs_to_trx(void);

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Gets a TFA PIB attribute
 *
 * This function is called to retrieve the transceiver information base
 * attributes.
 *
 * @param[in] tfa_pib_attribute TAL infobase attribute ID
 * @param[out] value TFA infobase attribute value
 *
 * @return MAC_UNSUPPORTED_ATTRIBUTE if the TFA infobase attribute is not found
 *         MAC_SUCCESS otherwise
 */
retval_t tfa_pib_get(tfa_pib_t tfa_pib_attribute, void *value)
{
    /* Keep compiler happy */
    value = value;

    switch (tfa_pib_attribute)
    {
        default:
            /* Invalid attribute id */
            return MAC_UNSUPPORTED_ATTRIBUTE;
    }

    //return MAC_SUCCESS;
}


/**
 * @brief Sets a TFA PIB attribute
 *
 * This function is called to set the transceiver information base
 * attributes.
 *
 * @param[in] tfa_pib_attribute TFA infobase attribute ID
 * @param[in] value TFA infobase attribute value to be set
 *
 * @return MAC_UNSUPPORTED_ATTRIBUTE if the TFA info base attribute is not found
 *         TAL_BUSY if the TAL is not in TAL_IDLE state.
 *         MAC_SUCCESS if the attempt to set the PIB attribute was successful
 */
retval_t tfa_pib_set(tfa_pib_t tfa_pib_attribute, void *value)
{
    /* Keep compiler happy */
    value = value;

    switch (tfa_pib_attribute)
    {
        default:
            /* Invalid attribute id */
            return MAC_UNSUPPORTED_ATTRIBUTE;
    }

    //return MAC_SUCCESS;
}


/**
 * @brief Initializes the TFA
 *
 * This function is called to initialize the TFA.
 *
 * @return MAC_SUCCESS if everything went correct;
 *         FAILURE otherwise
 */
retval_t tfa_init(void)
{
    init_tfa_pib();
    write_all_tfa_pibs_to_trx();

    return MAC_SUCCESS;
}


/**
 * @brief Reset the TFA
 *
 * This function is called to reset the TFA.
 *
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 */
void tfa_reset(bool set_default_pib)
{
    if (set_default_pib)
    {
        init_tfa_pib();
    }

    write_all_tfa_pibs_to_trx();
}


/**
 * @brief Perform a CCA
 *
 * This function performs a CCA request.
 *
 * @return phy_enum_t PHY_IDLE or PHY_BUSY
 */
phy_enum_t tfa_cca_perform(void)
{
    tal_trx_status_t trx_status;
    uint8_t cca_status;

    /* Ensure that trx is not in SLEEP for register access */
    do
    {
        trx_status = set_trx_state(CMD_TRX_OFF);
    } while (trx_status != TRX_OFF);

    // no interest in trx interrupts while doing CCA
    pal_trx_irq_dis();

    /* Set trx to rx mode. */
    do
    {
        trx_status = set_trx_state(CMD_RX_ON);
    } while (trx_status != RX_ON);

    /* Start CCA by writing any dummy value to this register */
    pal_trx_bit_write(SR_CCA_REQUEST, 1);

    /* wait until CCA is done */
    pal_timer_delay(TAL_CONVERT_SYMBOLS_TO_US(CCA_DURATION_SYM));

    do
    {
      /*
       * Poll until CCA is really done.
       * Do NOT use bit read here!
       */
      trx_status = (tal_trx_status_t)pal_trx_reg_read(RG_TRX_STATUS);
    } while ((trx_status & CCA_DONE_BIT) != CCA_DONE_BIT);

    set_trx_state(CMD_TRX_OFF);

    /* Check if channel was idle or busy. */
    if (trx_status & CCA_STATUS_BIT)
    {
        cca_status = PHY_IDLE;
    }
    else
    {
        cca_status = PHY_BUSY;
    }

    /*
     * Since we are not interested in any frames that might be received
     * during CCA, reject any information that indicates a previous frame
     * reception.
     */
    pal_trx_reg_read(RG_IRQ_STATUS);
    pal_trx_irq_flag_clr();
    pal_trx_irq_en();

    return (phy_enum_t)cca_status;
}


/**
 * @brief Perform a single ED measurement
 *
 * @return ed_value Result of the measurement (transceiver's register value)
 */
uint8_t tfa_ed_sample(void)
{
    uint8_t ed_value;
    tal_trx_status_t trx_status;

    /* Make sure that receiver is switched on. */
    do
    {
        trx_status = set_trx_state(CMD_RX_ON);
    } while (trx_status != RX_ON);

    /*
     * Disable the transceiver interrupts to prevent frame reception
     * while performing ED scan.
     */
    pal_trx_irq_dis();

    /*
     * Initiate ED operation by writing any value into transceiver register
     * PHY_ED_LEVEL.
     */
    pal_trx_reg_write(RG_PHY_ED_LEVEL, 0x00);

    /*
     * Start timer for reading ED value from the transceiver after
     * 140 microseconds.
     */
    pal_timer_delay(TAL_CONVERT_SYMBOLS_TO_US(ED_SAMPLE_DURATION_SYM+1));

    /* Read the ED Value. */
    ed_value = pal_trx_reg_read(RG_PHY_ED_LEVEL);

    /* Clear IRQ register */
    pal_trx_reg_read(RG_IRQ_STATUS);
    /* Enable reception agian */
    pal_trx_irq_flag_clr();
    pal_trx_irq_en();
    /* Switch receiver off again */
    set_trx_state(CMD_TRX_OFF);

    /*
     * Scale ED result.
     * Clip values to 0xFF if > -35dBm
     */
    if (ed_value > CLIP_VALUE_REG)
    {
        ed_value = 0xFF;
    }
    else
    {
        ed_value = (uint8_t)(((uint16_t)ed_value * 0xFF) / CLIP_VALUE_REG);
    }

    return ed_value;
}


/**
 * @brief Get the transceiver's supply voltage
 *
 * @return mv Milli Volt; 0 if below threshold, 0xFFFF if above threshold
 */
uint16_t tfa_get_batmon_voltage(void)
{
    tal_trx_status_t previous_trx_status;
    uint8_t vth_val;
    uint8_t i;
    uint16_t mv = 1;    // 1 used as indicator flag
    bool range;

    pal_trx_irq_dis();

    previous_trx_status = tal_trx_status;
    if (tal_trx_status == TRX_SLEEP)
    {
        set_trx_state(CMD_TRX_OFF);
    }

    /* Check if supply voltage is within lower range */
    pal_trx_bit_write(SR_BATMON_HR, BATMON_LOW_RANGE);
    pal_trx_bit_write(SR_BATMON_VTH, 0x0F);
    if (pal_trx_bit_read(SR_BATMON_OK) == BATMON_BELOW_THRES)
    {
        /* Lower range */
        /* Check if supply voltage is below lower limit */
        pal_trx_bit_write(SR_BATMON_VTH, 0);
        if (pal_trx_bit_read(SR_BATMON_OK) == BATMON_BELOW_THRES)
        {
            /* below lower limit */
            mv = 0x0000;
        }
        range = LOW;
    }
    else
    {
        /* Higher range */
        pal_trx_bit_write(SR_BATMON_HR, BATMON_HIGH_RANGE);
        /* Check if supply voltage is above upper limit */
        pal_trx_bit_write(SR_BATMON_VTH, 0x0F);
        if (pal_trx_bit_read(SR_BATMON_OK) == BATMON_ABOVE_THRES)
        {
            /* above upper limit */
            mv = 0xFFFF;
        }
        range = HIGH;
    }

    if (mv == 1)
    {
        vth_val = 0x0F;
        for (i = 0; i < 16; i++)
        {
            pal_trx_bit_write(SR_BATMON_VTH, i);
            if (pal_trx_bit_read(SR_BATMON_OK) == BATMON_BELOW_THRES)
            {
                if (i > 0)
                {
                    vth_val = i - 1;
                }
                else
                {
                    vth_val = i;
                }
                break;
            }
        }

        if (range == HIGH)
        {
            mv = 2550 + (75 * vth_val);
        }
        else
        {
            mv = 1700 + (50 * vth_val);
        }
    }

    pal_trx_reg_read(RG_IRQ_STATUS);
    if (previous_trx_status == TRX_SLEEP)
    {
        set_trx_state(CMD_TRX_SLEEP);
    }

    pal_trx_irq_en();

    return mv;
}



/**
 * @brief Initialize the TFA PIB
 *
 * This function initializes the TFA information base attributes
 * to their default values.
 */
static void init_tfa_pib(void)
{
}


/**
 * @brief Write all shadow PIB variables to the transceiver
 *
 * This function writes all shadow PIB variables to the transceiver.
 * It is assumed that the radio does not sleep.
 */
static void write_all_tfa_pibs_to_trx(void)
{
}


#ifdef CW_SUPPORTED
/**
 * @brief Starts continuous transmission on current channel
 */
void tfa_continuous_tx_start(continuous_tx_mode_t tx_mode)
{
    uint8_t txcwdata[127];

    pal_trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);
    pal_trx_bit_write(SR_TX_AUTO_CRC_ON, TX_AUTO_CRC_DISABLE);
    pal_trx_reg_write(0x36, 0x0F);     /* TST_CTRL_DIGI */

    if (tx_mode == CW_MODE)
    {
        txcwdata[0] = 1;    // step 9
        txcwdata[1] = 0;
        pal_trx_frame_write(txcwdata, 2);
        // Step 10
        pal_trx_reg_write(0x3D, 0x80);  /* Configure continuous Tx (2) */
    }
    else    // PRBS mode
    {
        txcwdata[0] = 127;   // = max length
        for (uint8_t i = 1; i < 128; i++)
        {
            txcwdata[i] = (uint8_t)rand();
        }
        pal_trx_frame_write(txcwdata, 128);
        pal_trx_reg_write(0x3D, 0x00);  /* Configure continuous Tx (2) */
    }

    // set tst pin to high
    TST_PORT_HIGH();

    set_trx_state(CMD_PLL_ON);
    PAL_SLP_TR_HIGH();
    PAL_SLP_TR_LOW();
}
#endif


#ifdef CW_SUPPORTED
/**
 * @brief Stops continuous transmission
 */
void tfa_continuous_tx_stop(void)
{
    TST_PORT_LOW();
    tal_reset(false);
}
#endif



#endif /* #ifdef ENABLE_TFA */

/* EOF */

