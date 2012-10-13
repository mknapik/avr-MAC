/**
 * @file tiny_tal_pib.c
 *
 * @brief This file handles the TAL PIB attributes, set/get and initialization
 *
 * $Id: tiny_tal_pib.c 21512 2010-04-13 08:18:44Z sschneid $
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
#include <string.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "tiny_tal.h"
#include "ieee_const.h"
#include "tiny_tal_constants.h"
#include "tiny_tal_pib.h"
#include "at86rf230b.h"
#include "tiny_tal_internal.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/*
 * Translation table converting register values to power levels (dBm).
 */
static FLASH_DECLARE(int8_t tx_pwr_table[16]) =
{
    3, /* 3.2 */
    2, /* 2.6 */
    2, /* 2.1 */
    1, /* 1.6 */
    1, /* 1.1 */
    0, /* 0.5 */
    0, /* -0.2 */
    -1, /* -1.2 */
    -2, /* -2.2 */
    -3, /* -3.2 */
    -4, /* -4.2 */
    -5, /* -5.2 */
    -7, /* -7.2 */
    -9, /* -9.2 */
    -12, /* -12.2 */
    -17, /* -17.2 */
};

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */

static uint8_t limit_tx_pwr(uint8_t tal_pib_TransmitPower);
static uint8_t convert_phyTransmitPower_to_reg_value(uint8_t phyTransmitPower_value);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Initialize the TAL PIB
 *
 * This function initializes the TAL information base attributes
 * to their default values.
 */
void init_tal_pib(void)
{
    tal_pib_MaxCSMABackoffs = TAL_MAX_CSMA_BACKOFFS_DEFAULT;
    tal_pib_MinBE = TAL_MINBE_DEFAULT;
    tal_pib_PANId = TAL_PANID_BC_DEFAULT;
    tal_pib_ShortAddress = TAL_SHORT_ADDRESS_DEFAULT;
    tal_pib_CurrentChannel = TAL_CURRENT_CHANNEL_DEFAULT;
    tal_pib_CurrentPage = TAL_CURRENT_PAGE_DEFAULT;
    tal_pib_MaxBE = TAL_MAXBE_DEFAULT;
    tal_pib_MaxFrameRetries = TAL_MAXFRAMERETRIES_DEFAULT;
    tal_pib_TransmitPower = limit_tx_pwr(TAL_TRANSMIT_POWER_DEFAULT);
    tal_pib_CCAMode = TAL_CCA_MODE_DEFAULT;
    tal_pib_PrivatePanCoordinator = TAL_PAN_COORDINATOR_DEFAULT;
}


/**
 * @brief Write all shadow PIB variables to the transceiver
 *
 * This function writes all shadow PIB variables to the transceiver.
 * It is assumed that the radio does not sleep.
 */
void write_all_tal_pib_to_trx(void)
{
    uint8_t *ptr_to_reg;

    /* configure RX_AACK */
    pal_trx_reg_write(RG_PAN_ID_0, (uint8_t)tal_pib_PANId);
    pal_trx_reg_write(RG_PAN_ID_1, (uint8_t)(tal_pib_PANId >> 8));

    ptr_to_reg = (uint8_t *)&tal_pib_IeeeAddress;
    for (uint8_t i = 0; i < 8; i++)
    {
        pal_trx_reg_write((RG_IEEE_ADDR_0 + i), *ptr_to_reg);
        ptr_to_reg++;
    }

    pal_trx_reg_write(RG_SHORT_ADDR_0, (uint8_t)tal_pib_ShortAddress);
    pal_trx_reg_write(RG_SHORT_ADDR_1, (uint8_t)(tal_pib_ShortAddress >> 8));

    /* configure TX_ARET; CSMA and CCA */
    {
        uint8_t reg_value;

        reg_value = convert_phyTransmitPower_to_reg_value(tal_pib_TransmitPower);
        pal_trx_bit_write(SR_TX_PWR, reg_value);
    }
    pal_trx_bit_write(SR_CCA_MODE, tal_pib_CCAMode);
    pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
    pal_trx_bit_write(SR_MIN_BE, tal_pib_MinBE);
}


/**
 * @brief Sets a TAL PIB attribute
 *
 * This function is called to set the transceiver information base
 * attributes.
 *
 * @param attribute TAL infobase attribute ID
 * @param value TAL infobase attribute value to be set
 *
 * @return MAC_UNSUPPORTED_ATTRIBUTE if the TAL info base attribute is not found
 *         TAL_BUSY if the TAL is not in TAL_IDLE state. An exception is
 *         macBeaconTxTime which can be accepted by TAL even if TAL is not
 *         in TAL_IDLE state.
 *         MAC_SUCCESS if the attempt to set the PIB attribute was successful
 *         TAL_TRX_ASLEEP if trx is in SLEEP mode and access to trx is required
 */
retval_t tal_pib_set(uint8_t attribute, pib_value_t *value)
{
    /*
     * Distinguish between PIBs that need to be changed in trx directly
     * and those that are simple variable udpates.
     * Ensure that the transceiver is not in SLEEP.
     * If it is in SLEEP, change it to TRX_OFF.
     * For all other state force TRX_OFF.
     * Abort any TAL_BUSY state, because the change might have an impact to
     * ongoing transactions.
     */

    switch (attribute)
    {
        case macMaxFrameRetries:
                    /*
                     * The new PIB value is not immediately written to the
                     * transceiver. This is done on a frame-by-frame base.
                     */
                    tal_pib_MaxFrameRetries = value->pib_value_8bit;
                    break;

        case macMaxCSMABackoffs:
                    /*
                     * The new PIB value is not immediately written to the
                     * transceiver. This is done on a frame-by-frame base.
                     */
                    tal_pib_MaxCSMABackoffs = value->pib_value_8bit;
                    break;

        default:
            /*
             * Following PIBs require access to trx.
             * Therefore trx must be at least in TRX_OFF.
             */

            if (tal_trx_status == TRX_SLEEP)
            {
                /* While trx is in SLEEP, register cannot be accessed. */
                return TAL_TRX_ASLEEP;
            }

            switch (attribute)
            {
                case macMinBE:
                    tal_pib_MinBE = value->pib_value_8bit;
                    pal_trx_bit_write(SR_MIN_BE, tal_pib_MinBE);
                    break;

                case macPANId:
                    tal_pib_PANId = value->pib_value_16bit;
                    pal_trx_reg_write(RG_PAN_ID_0, (uint8_t)tal_pib_PANId);
                    pal_trx_reg_write(RG_PAN_ID_1, (uint8_t)(tal_pib_PANId >> 8));
                    break;

                case macShortAddress:
                    tal_pib_ShortAddress = value->pib_value_16bit;
                    pal_trx_reg_write(RG_SHORT_ADDR_0, (uint8_t)tal_pib_ShortAddress);
                    pal_trx_reg_write(RG_SHORT_ADDR_1, (uint8_t)(tal_pib_ShortAddress >> 8));
                    break;

                case phyCurrentChannel:
                    if (tal_state != TAL_IDLE)
                    {
                        return TAL_BUSY;
                    }
                    if ((uint32_t)TRX_SUPPORTED_CHANNELS & ((uint32_t)0x01 << value->pib_value_8bit))
                    {
                        tal_trx_status_t previous_trx_status = TRX_OFF;
                        /*
                         * Set trx to "soft" off avoiding that ongoing
                         * transaction (e.g. ACK) are interrupted.
                         */
                        if (tal_trx_status != TRX_OFF)
                        {
                            previous_trx_status = RX_AACK_ON;   /* any other than TRX_OFF state */
                            do
                            {
                                /* set TRX_OFF until it could be set;
                                 * trx might be busy */
                            } while (set_trx_state(CMD_TRX_OFF) != TRX_OFF);
                        }
                        tal_pib_CurrentChannel = value->pib_value_8bit;
                        pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
                        /* Re-store previous trx state */
                        if (previous_trx_status != TRX_OFF)
                        {
                            /* Set to default state */
                            set_trx_state(CMD_RX_AACK_ON);
                        }
                    }
                    else
                    {
                        return MAC_INVALID_PARAMETER;
                    }
                    break;

                case phyCurrentPage:


                if (tal_state != TAL_IDLE)
                {
                    return TAL_BUSY;
                }
                else
                {
                    uint8_t page;

                    page = value->pib_value_8bit;
                    if (page != 0)
                    {
                        return MAC_INVALID_PARAMETER;
                    }
                }
                break;

                case macMaxBE:
                    tal_pib_MaxBE = value->pib_value_8bit;
                    break;

                case phyTransmitPower:
                    {
                        uint8_t reg_value;

                        tal_pib_TransmitPower = value->pib_value_8bit;

                        /* Limit tal_pib_TransmitPower to max/min trx values */
                        tal_pib_TransmitPower = limit_tx_pwr(tal_pib_TransmitPower);
                        reg_value = convert_phyTransmitPower_to_reg_value(tal_pib_TransmitPower);
                        pal_trx_bit_write(SR_TX_PWR, reg_value);
                    }
                    break;

                case phyCCAMode:
                    tal_pib_CCAMode = value->pib_value_8bit;
                    pal_trx_bit_write(SR_CCA_MODE, tal_pib_CCAMode);
                    break;

                case macIeeeAddress:
                    {
                        uint8_t *ptr;

                        tal_pib_IeeeAddress = value->pib_value_64bit;
                        ptr = (uint8_t *)&tal_pib_IeeeAddress;

                        for (uint8_t i = 0; i < 8; i++)
                        {
                            pal_trx_reg_write((RG_IEEE_ADDR_0 + i), *ptr);
                            ptr++;
                        }
                    }
                    break;

                case mac_i_pan_coordinator:
                    tal_pib_PrivatePanCoordinator = value->pib_value_bool;
                    pal_trx_bit_write(SR_I_AM_COORD, tal_pib_PrivatePanCoordinator);
                    break;

                case macAckWaitDuration:
                    /* AT86RF230B does not support changing this value */
                    return MAC_UNSUPPORTED_ATTRIBUTE;

                default:
                    return MAC_UNSUPPORTED_ATTRIBUTE;
            }

            break; /* end of 'default' from 'switch (attribute)' */
    }
    return MAC_SUCCESS;
} /* tal_pib_set() */


/**
 * @brief Limit the phyTransmitPower to the trx limits
 *
 * @param phyTransmitPower phyTransmitPower value
 *
 * @return limited tal_pib_TransmitPower
 */
static uint8_t limit_tx_pwr(uint8_t tal_pib_TransmitPower)
{
    uint8_t ret_val = tal_pib_TransmitPower;
    int8_t dbm_value;

    dbm_value = CONV_phyTransmitPower_TO_DBM(tal_pib_TransmitPower);
    if (dbm_value > (int8_t)PGM_READ_BYTE(&tx_pwr_table[0]))
    {
        dbm_value = (int8_t)PGM_READ_BYTE(&tx_pwr_table[0]);
        ret_val = CONV_DBM_TO_phyTransmitPower(dbm_value);

    }
    else if (dbm_value < (int8_t)PGM_READ_BYTE(&tx_pwr_table[sizeof(tx_pwr_table)-1]))
    {
        dbm_value = (int8_t)PGM_READ_BYTE(&tx_pwr_table[sizeof(tx_pwr_table)-1]);
        ret_val = CONV_DBM_TO_phyTransmitPower(dbm_value);
    }

    return (ret_val | TX_PWR_TOLERANCE);
}


/**
 * @brief Converts a phyTransmitPower value to a register value
 *
 * @param phyTransmitPower_value phyTransmitPower value
 *
 * @return register value
 */
static uint8_t convert_phyTransmitPower_to_reg_value(uint8_t phyTransmitPower_value)
{
    int8_t dbm_value;
    uint8_t i;
    int8_t trx_tx_level;

    dbm_value = CONV_phyTransmitPower_TO_DBM(phyTransmitPower_value);

    /* Compare to the register value to identify the value that matches. */
    for (i = 0; i < sizeof(tx_pwr_table); i++)
    {
        trx_tx_level = (int8_t)PGM_READ_BYTE(&tx_pwr_table[i]);
        if (trx_tx_level <= dbm_value)
        {
            if (trx_tx_level < dbm_value)
            {
                return (i - 1);
            }
            return i;
        }
    }

    /* This code should never be reached. */
    return 0;
}


/* EOF */
