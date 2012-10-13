/**
 * @file tiny_tal_pib.c
 *
 * @brief This file handles the TAL PIB attributes, set/get and initialization
 *
 * $Id: tiny_tal_pib.c 21513 2010-04-13 08:19:13Z sschneid $
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
#include "at86rf212.h"
#include "tiny_tal_internal.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

/**
 * Tx power table
 * Table maps tx power value to register value
 */
#define TX_PWR_TABLE_NA \
/* Tx power, dBm        10     9     8     7     6     5     4     3     2     1     0    -1    -2    -3    -4    -5    -6    -7    -8    -9   -10   -11 */ \
/* Register value */  0xc0, 0xa1, 0x81, 0x82, 0x83, 0x60, 0x61, 0x41, 0x42, 0x22, 0x23, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c

#define TX_PWR_TABLE_EU1 \
/* Tx power, dBm        10     9     8     7     6     5     4     3     2     1     0    -1    -2    -3    -4    -5    -6    -7    -8    -9   -10   -11 */ \
/* Register value */  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x64, 0x65, 0x66, 0x46, 0x26, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c

#define TX_PWR_TABLE_EU2 \
/* Tx power, dBm        10     9     8     7     6     5     4     3     2     1     0    -1    -2    -3    -4    -5    -6    -7    -8    -9   -10   -11 */ \
/* Register value */  0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe8, 0xe9, 0xea, 0xcb, 0xab, 0xac, 0xad, 0x48, 0x27, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c

#define TX_PWR_TABLE_CHINA \
/* Tx power, dBm        10     9     8     7     6     5     4     3     2     1     0    -1    -2    -3    -4    -5    -6    -7    -8    -9   -10   -11 */ \
/* Register value */  0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe8, 0xe9, 0xea, 0xca, 0xaa, 0xab, 0x45, 0x25, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a

#define TX_PWR_TABLE_SIZE           (22)

#define EU_TX_PWR_TABLE             (2)     /* Use power table #2 for EU mode */

#define MAX_TX_PWR                  (10)
#define MIN_TX_PWR                  (-11)

#if (EU_TX_PWR_TABLE == 1)
#define MAX_TX_PWR_BPSK_20          (2)
#define MAX_TX_PWR_OQPSK_100        (-1)
#endif
#if (EU_TX_PWR_TABLE == 2)
#define MAX_TX_PWR_BPSK_20          (5)
#define MAX_TX_PWR_OQPSK_100        (3)
#endif

#ifdef HIGH_DATA_RATE_SUPPORT
#define MAX_TX_PWR_OQPSK_200        MAX_TX_PWR_OQPSK_100
#define MAX_TX_PWR_OQPSK_400        MAX_TX_PWR_OQPSK_100
#define MAX_TX_PWR_OQPSK_500        (7)
#define MAX_TX_PWR_OQPSK_1000       (4)
#endif

#define MAX_TX_PWR_CHINA            (5)

#define GET_CHINA_FREQ(x)           (11 + (2 * x))


/* === GLOBALS ============================================================= */

/**
 * Tx power table in flash.
 */
static FLASH_DECLARE (const uint8_t tx_pwr_table_NA[TX_PWR_TABLE_SIZE]) = { TX_PWR_TABLE_NA };
#if (EU_TX_PWR_TABLE == 1)
static FLASH_DECLARE (const uint8_t tx_pwr_table_EU[TX_PWR_TABLE_SIZE]) = { TX_PWR_TABLE_EU1 };
#endif
#if (EU_TX_PWR_TABLE == 2)
static FLASH_DECLARE (const uint8_t tx_pwr_table_EU[TX_PWR_TABLE_SIZE]) = { TX_PWR_TABLE_EU2 };
#endif
static FLASH_DECLARE (const uint8_t tx_pwr_table_China[TX_PWR_TABLE_SIZE]) = { TX_PWR_TABLE_CHINA };

/* === PROTOTYPES ========================================================== */

static void limit_tx_pwr(void);
static uint8_t convert_phyTransmitPower_to_reg_value(uint8_t phyTransmitPower_value);
static bool apply_channel_page_configuration(uint8_t ch_page);

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
    tal_pib_TransmitPower = TAL_TRANSMIT_POWER_DEFAULT;
    limit_tx_pwr();
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
    pal_trx_bit_write(SR_CCA_MODE, tal_pib_CCAMode);
    pal_trx_bit_write(SR_MIN_BE, tal_pib_MinBE);

    pal_trx_bit_write(SR_AACK_I_AM_COORD, tal_pib_PrivatePanCoordinator);

    /* set phy parameter */
    pal_trx_bit_write(SR_MAX_BE, tal_pib_MaxBE);
    apply_channel_page_configuration(tal_pib_CurrentPage);

    {
        uint8_t reg_value;

        reg_value = convert_phyTransmitPower_to_reg_value(tal_pib_TransmitPower);
        pal_trx_reg_write(RG_PHY_TX_PWR, reg_value);
    }
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

                    if (tal_pib_CurrentPage == 5)
                    {
                        if (((uint32_t)TRX_SUPPORTED_CHANNELS_CHINA & ((uint32_t)0x01 << value->pib_value_8bit)) == false)
                        {
                            return MAC_INVALID_PARAMETER;
                        }
                    }
                    else
                    {
                        if (((uint32_t)TRX_SUPPORTED_CHANNELS & ((uint32_t)0x01 << value->pib_value_8bit)) == false)
                        {
                            return MAC_INVALID_PARAMETER;
                        }
                    }

                    {
                        uint8_t previous_channel;
                        tal_trx_status_t previous_trx_status = TRX_OFF;

                        previous_channel = tal_pib_CurrentChannel;
                        tal_pib_CurrentChannel = value->pib_value_8bit;

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

                        /* Check if frequency band/modulation is changed. */
                        if (tal_pib_CurrentPage == 5)
                        {
                            pal_trx_bit_write(SR_CC_NUMBER, GET_CHINA_FREQ(tal_pib_CurrentChannel));
                        }
                        else if ((tal_pib_CurrentChannel > 0) && (previous_channel > 0))
                        {
                            pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
                        }
                        else
                        {
                            uint8_t reg_value;

                            /* Set modulation and channel */
                            apply_channel_page_configuration(tal_pib_CurrentPage);
                            limit_tx_pwr();
                            reg_value = convert_phyTransmitPower_to_reg_value(tal_pib_TransmitPower);
                            pal_trx_reg_write(RG_PHY_TX_PWR, reg_value);
                        }

                        /* Re-store previous trx state */
                        if (previous_trx_status != TRX_OFF)
                        {
                            /* Set to default state */
                            set_trx_state(CMD_RX_AACK_ON);
                        }
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
                        tal_trx_status_t previous_trx_status = TRX_OFF;
                        bool ret_val;
                        uint8_t reg_value;

                        /*
                         * Changing the channel, channel page or modulation
                         * requires that TRX is in TRX_OFF.
                         * Store current trx state and return to default state
                         * after channel page has been set.
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

                        page = value->pib_value_8bit;

                        ret_val = apply_channel_page_configuration(page);

                        if (previous_trx_status != TRX_OFF)
                        {
                            /* Set to default state */
                            set_trx_state(CMD_RX_AACK_ON);
                        }

                        if (ret_val)
                        {
                            tal_pib_CurrentPage = page;
                        }
                        else
                        {
                            return MAC_INVALID_PARAMETER;
                        }
                        limit_tx_pwr();
                        reg_value = convert_phyTransmitPower_to_reg_value(tal_pib_TransmitPower);
                        pal_trx_reg_write(RG_PHY_TX_PWR, reg_value);
                    }
                    break;

                case macMaxBE:
                    tal_pib_MaxBE = value->pib_value_8bit;
                    pal_trx_bit_write(SR_MAX_BE, tal_pib_MaxBE);
                    break;

                case phyTransmitPower:
                    {
                        uint8_t reg_value;

                        tal_pib_TransmitPower = value->pib_value_8bit;

                        /* Limit tal_pib_TransmitPower to max/min trx values */
                        limit_tx_pwr();
                        reg_value = convert_phyTransmitPower_to_reg_value(tal_pib_TransmitPower);
                        pal_trx_reg_write(RG_PHY_TX_PWR, reg_value);
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
                    pal_trx_bit_write(SR_AACK_I_AM_COORD, tal_pib_PrivatePanCoordinator);
                    break;

                case macAckWaitDuration:
                    /*
                     * AT86RF212 does not support changing this value w.r.t.
                     * compliance operation.
                     */
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
 */
static void limit_tx_pwr(void)
{
    int8_t dbm_value;

    dbm_value = CONV_phyTransmitPower_TO_DBM(tal_pib_TransmitPower);

    /* Limit to the transceiver's absolute maximum/minimum. */
    if (dbm_value <= MIN_TX_PWR)
    {
        dbm_value = MIN_TX_PWR;
    }
    else
    {
        /* Upper Tx power limits depend on the currently used channel and channel page */
        switch (tal_pib_CurrentPage)
        {
            case 0: /* BPSK */
                if (tal_pib_CurrentChannel == 0)
                {
                    if (dbm_value > MAX_TX_PWR_BPSK_20)
                    {
                        dbm_value = MAX_TX_PWR_BPSK_20;
                    }
                }
                else    /* channels 1-10 */
                {
                    if (dbm_value > MAX_TX_PWR)
                    {
                        dbm_value = MAX_TX_PWR;
                    }
                }
                break;
            case 2: /* O-QPSK */
                if (tal_pib_CurrentChannel == 0)
                {
                    if (dbm_value > MAX_TX_PWR_OQPSK_100)
                    {
                        dbm_value = MAX_TX_PWR_OQPSK_100;
                    }
                }
                else    /* channels 1-10 */
                {
                    if (dbm_value > MAX_TX_PWR)
                    {
                        dbm_value = MAX_TX_PWR;
                    }
                }
                break;
            case 5: /* China, O-QPSK */
#ifdef HIGH_DATA_RATE_SUPPORT
            case 18: /* Chinese O-QPSK 500 */
            case 19: /* Chinese O-QPSK 1000 */
#endif
                if (dbm_value > MAX_TX_PWR_CHINA)
                {
                    dbm_value = MAX_TX_PWR_CHINA;
                }
                break;
#ifdef HIGH_DATA_RATE_SUPPORT
            case 16: /* O-QPSK 200, 500 */
                if (tal_pib_CurrentChannel == 0)
                {
                    if (dbm_value > MAX_TX_PWR_OQPSK_200)
                    {
                        dbm_value = MAX_TX_PWR_OQPSK_200;
                    }
                }
                else    /* channels 1-10 */
                {
                    if (dbm_value > MAX_TX_PWR_OQPSK_500)
                    {
                        dbm_value = MAX_TX_PWR_OQPSK_500;
                    }
                }
                break;
            case 17: /* O-QPSK 400, 1000 */
                if (tal_pib_CurrentChannel == 0)
                {
                    if (dbm_value > MAX_TX_PWR_OQPSK_400)
                    {
                        dbm_value = MAX_TX_PWR_OQPSK_400;
                    }
                }
                else    /* channels 1-10 */
                {
                    if (dbm_value > MAX_TX_PWR_OQPSK_1000)
                    {
                        dbm_value = MAX_TX_PWR_OQPSK_1000;
                    }
                }
                break;
#endif  /* #ifdef HIGH_DATA_RATE_SUPPORT */
        }
    }

    tal_pib_TransmitPower = TX_PWR_TOLERANCE | CONV_DBM_TO_phyTransmitPower(dbm_value);
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
    uint8_t reg_value = 0x00;

    dbm_value = CONV_phyTransmitPower_TO_DBM(phyTransmitPower_value);

    /* Select the corresponding tx_pwr_table */
#ifdef HIGH_DATA_RATE_SUPPORT
    if ((tal_pib_CurrentPage == 5) || (tal_pib_CurrentPage == 18) || (tal_pib_CurrentPage == 19))
#else
    if (tal_pib_CurrentPage == 5)
#endif
    {
        reg_value = PGM_READ_BYTE(&tx_pwr_table_China[MAX_TX_PWR - dbm_value]);
    }
    else
    {
        if (tal_pib_CurrentChannel == 0)
        {
            reg_value = PGM_READ_BYTE(&tx_pwr_table_EU[MAX_TX_PWR - dbm_value]);
        }
        else    // channels 1-10
        {
            reg_value = PGM_READ_BYTE(&tx_pwr_table_NA[MAX_TX_PWR - dbm_value]);
        }
    }

    return reg_value;
}


/**
 * @brief Apply channel page configuartion to transceiver
 *
 * @param ch_page Channel page
 *
 * @return true if changes could be applied else false
 */
static bool apply_channel_page_configuration(uint8_t ch_page)
{
    /*
     * Before updating the transceiver a number of TAL PIB attributes need
     * to be updated depending on the channel page.
     */
    switch (ch_page)
    {
        case 0: /* BPSK */
            pal_trx_bit_write(SR_BPSK_OQPSK, BPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, BPSK_TX_OFFSET);
            if (tal_pib_CurrentChannel == 0)
            {
                pal_trx_bit_write(SR_SUB_MODE, LOW_DATA_RATE);
            }
            else
            {
                pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
                pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, HALF_SINE_FILTERING);
            }
            // Compliant ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_12_SYMBOLS);
            pal_trx_bit_write(SR_CC_BAND, 0);
            pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
            break;

        case 2: /* O-QPSK */
            pal_trx_bit_write(SR_BPSK_OQPSK, OQPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, OQPSK_TX_OFFSET);
            pal_trx_bit_write(SR_OQPSK_DATA_RATE, ALTRATE_100_KBPS_OR_250_KBPS);
            if (tal_pib_CurrentChannel == 0)
            {
                pal_trx_bit_write(SR_SUB_MODE, LOW_DATA_RATE);
            }
            else
            {
                pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
                pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, HALF_SINE_FILTERING);
            }
            // Compliant ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_12_SYMBOLS);
            pal_trx_bit_write(SR_CC_BAND, 0);
            pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
            break;

        case 5: /* CHINESE_BAND, O-QPSK */
            pal_trx_bit_write(SR_BPSK_OQPSK, OQPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, OQPSK_TX_OFFSET);
            pal_trx_bit_write(SR_OQPSK_DATA_RATE, ALTRATE_100_KBPS_OR_250_KBPS);
            pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, RC_0_8_FILTERING);
            pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
            // Compliant ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_12_SYMBOLS);
            /* Channel Page 5 supports channels 0-3. */
            if (tal_pib_CurrentChannel > 3)
            {
                tal_pib_CurrentChannel = 0;
            }
            pal_trx_bit_write(SR_CC_BAND, 4);
            pal_trx_bit_write(SR_CC_NUMBER, GET_CHINA_FREQ(tal_pib_CurrentChannel));
            break;

#ifdef HIGH_DATA_RATE_SUPPORT
        case 16:    /* non-compliant OQPSK mode 1 */
            pal_trx_bit_write(SR_BPSK_OQPSK, OQPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, OQPSK_TX_OFFSET);
            pal_trx_bit_write(SR_OQPSK_DATA_RATE, ALTRATE_200_KBPS_OR_500_KBPS);
            if (tal_pib_CurrentChannel == 0)
            {
                pal_trx_bit_write(SR_SUB_MODE, LOW_DATA_RATE);
            }
            else
            {
                pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
                pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, HALF_SINE_FILTERING);
            }
            // Reduced ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_2_SYMBOLS);
            pal_trx_bit_write(SR_CC_BAND, 0);
            pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
            break;

        case 17:    /* non-compliant OQPSK mode 2 */
            pal_trx_bit_write(SR_BPSK_OQPSK, OQPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, OQPSK_TX_OFFSET);
            pal_trx_bit_write(SR_OQPSK_DATA_RATE, ALTRATE_400_KBPS_OR_1_MBPS);
            if (tal_pib_CurrentChannel == 0)
            {
                pal_trx_bit_write(SR_SUB_MODE, LOW_DATA_RATE);
            }
            else
            {
                pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
                pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, HALF_SINE_FILTERING);
            }
            // Reduced ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_2_SYMBOLS);
            tal_pib_SupportedChannels = TRX_SUPPORTED_CHANNELS;
            pal_trx_bit_write(SR_CC_BAND, 0);
            pal_trx_bit_write(SR_CHANNEL, tal_pib_CurrentChannel);
            break;

        case 18:    /* Chinese band, non-compliant mode 1 using O-QPSK 500 */
            pal_trx_bit_write(SR_BPSK_OQPSK, OQPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, OQPSK_TX_OFFSET);
            pal_trx_bit_write(SR_OQPSK_DATA_RATE, ALTRATE_200_KBPS_OR_500_KBPS);
            pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, RC_0_8_FILTERING);
            pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
            // Reduced ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_2_SYMBOLS);
            tal_pib_SupportedChannels = TRX_SUPPORTED_CHANNELS_CHINA;
            /* Channel Page 18 supports channels 0-3. */
            if (tal_pib_CurrentChannel > 3)
            {
                tal_pib_CurrentChannel = 0;
            }
            pal_trx_bit_write(SR_CC_BAND, 4);
            pal_trx_bit_write(SR_CC_NUMBER, GET_CHINA_FREQ(tal_pib_CurrentChannel));
            break;

        case 19:    /* Chinese band, non-compliant mode 2 using O-QPSK 1000 */
            pal_trx_bit_write(SR_BPSK_OQPSK, OQPSK_MODE);
            pal_trx_bit_write(SR_GC_TX_OFFS, OQPSK_TX_OFFSET);
            pal_trx_bit_write(SR_OQPSK_DATA_RATE, ALTRATE_400_KBPS_OR_1_MBPS);
            pal_trx_bit_write(SR_OQPSK_SUB1_RC_EN, RC_0_8_FILTERING);
            pal_trx_bit_write(SR_SUB_MODE, HIGH_DATA_RATE);
            // Reduced ACK timing
            pal_trx_bit_write(SR_AACK_ACK_TIME, AACK_ACK_TIME_2_SYMBOLS);
            tal_pib_SupportedChannels = TRX_SUPPORTED_CHANNELS_CHINA;
            /* Channel Page 18 supports channels 0-3. */
            if (tal_pib_CurrentChannel > 3)
            {
                tal_pib_CurrentChannel = 0;
            }
            pal_trx_bit_write(SR_CC_BAND, 4);
            pal_trx_bit_write(SR_CC_NUMBER, GET_CHINA_FREQ(tal_pib_CurrentChannel));
            break;
#endif  /* #ifdef HIGH_DATA_RATE_SUPPORT */
            default:
                return false;
    }

    return true;
}


/* EOF */
