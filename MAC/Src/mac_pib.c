/**
 * @file mac_pib.c
 *
 * @brief Implements the MAC PIB attribute handling.
 *
 * $Id: mac_pib.c 22661 2010-07-23 08:49:25Z sschneid $
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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "tal.h"
#include "ieee_const.h"
#include "mac_msg_const.h"
#include "mac_api.h"
#include "mac_msg_types.h"
#include "mac_data_structures.h"
#include "stack_config.h"
#include "mac_internal.h"
#include "mac.h"
#include "mac_build_config.h"
#ifdef MAC_SECURITY_ZIP
#include "mac_security.h"
#endif  /* MAC_SECURITY_ZIP */
#ifdef TEST_HARNESS
#include "private_const.h"
#endif /* TEST_HARNESS */

/* === Macros ============================================================== */

#define MIN(a, b)                       ( ((a) < (b)) ? (a) : (b) )

/* === Globals ============================================================= */

/* Size constants for PHY PIB attributes */
static FLASH_DECLARE(uint8_t phy_pib_size[]) =
{
    sizeof(uint8_t),                // 0x00: phyCurrentChannel
    sizeof(uint32_t),               // 0x01: phyChannelsSupported
    sizeof(uint8_t),                // 0x02: phyTransmitPower
    sizeof(uint8_t),                // 0x03: phyCCAMode
    sizeof(uint8_t),                // 0x04: phyCurrentPage
    sizeof(uint16_t),               // 0x05: phyMaxFrameDuration
    sizeof(uint8_t),                // 0x06: phySHRDuration
    sizeof(uint8_t)                 // 0x07: phySymbolsPerOctet
};

/* Update this one the arry phy_pib_size is updated. */
#define MAX_PHY_PIB_ATTRIBUTE_ID            (phySymbolsPerOctet)

/* Size constants for MAC PIB attributes */
static FLASH_DECLARE(uint8_t mac_pib_size[]) =
{
    sizeof(uint8_t),                // 0x40: macAckWaitDuration
    sizeof(uint8_t),                // 0x41: macAssociationPermit
    sizeof(uint8_t),                // 0x42: macAutoRequest
    sizeof(uint8_t),                // 0x43: macBattLifeExt
    sizeof(uint8_t),                // 0x44: macBattLifeExtPeriods
    sizeof(uint8_t),                // 0x45: macBeaconPayload
    sizeof(uint8_t),                // 0x46: macBeaconPayloadLength
    sizeof(uint8_t),                // 0x47: macBeaconOrder
    sizeof(uint32_t),               // 0x48: macBeaconTxTime
    sizeof(uint8_t),                // 0x49: macBSN
    sizeof(uint64_t),               // 0x4A: macCoordExtendedAddress
    sizeof(uint16_t),               // 0x4B: macCoordShortAddress
    sizeof(uint8_t),                // 0x4C: macDSN
    sizeof(uint8_t),                // 0x4D: macGTSPermit
    sizeof(uint8_t),                // 0x4E: macMaxCSMAbackoffs
    sizeof(uint8_t),                // 0x4F: macMinBE
    sizeof(uint16_t),               // 0x50: macPANId
    sizeof(uint8_t),                // 0x51: macPromiscuousMode
    sizeof(uint8_t),                // 0x52: macRxOnWhenIdle
    sizeof(uint16_t),               // 0x53: macShortAddress
    sizeof(uint8_t),                // 0x54: macSuperframeOrder
    sizeof(uint16_t),               // 0x55: macTransactionPersistenceTime
    sizeof(uint8_t),                // 0x56: macAssociatedPANCoord
    sizeof(uint8_t),                // 0x57: macMaxBE
    sizeof(uint16_t),               // 0x58: macMaxFrameTotalWaitTime
    sizeof(uint8_t),                // 0x59: macMaxFrameRetries
    sizeof(uint16_t),               // 0x5A: macResponseWaitTime
    sizeof(uint16_t),               // 0x5B: macSyncSymbolOffset
    sizeof(uint8_t),                // 0x5C: macTimestampSupported
    sizeof(uint8_t),                // 0x5D: macSecurityEnabled
    sizeof(uint8_t),                // 0x5E: macMinLIFSPeriod
    sizeof(uint8_t)                 // 0x5F: macMinSIFSPeriod
};

/* Update this one the arry mac_pib_size is updated. */
#define MIN_MAC_PIB_ATTRIBUTE_ID            (macAckWaitDuration)
#define MAX_MAC_PIB_ATTRIBUTE_ID            (macMinSIFSPeriod)


#ifdef MAC_SECURITY_ZIP
/* Size constants for MAC Security PIB attributes */
static FLASH_DECLARE(uint8_t mac_sec_pib_size[]) =
{
    sizeof(mac_key_table_t),        // 0x71: macKeyTable
    sizeof(uint8_t),                // 0x72: macKeyTableEntries
    sizeof(mac_dev_table_t),        // 0x73: macDeviceTable
    sizeof(uint8_t),                // 0x74: macDeviceTableEntries
    sizeof(mac_sec_lvl_table_t),    // 0x75: macSecurityLevelTable
    sizeof(uint8_t),                // 0x76: macSecurityLevelTableEntries
    sizeof(uint32_t),               // 0x77: macFrameCounter
    sizeof(uint8_t),                // 0x78: macAutoRequestSecurityLevel    // Not used in ZIP
    sizeof(uint8_t),                // 0x79: macAutoRequestKeyIdMode        // Not used in ZIP
    sizeof(uint8_t),                // 0x7A: macAutoRequestKeySource        // Not used in ZIP
    sizeof(uint8_t),                // 0x7B: macAutoRequestKeyIndex         // Not used in ZIP
    (8 * sizeof(uint8_t)),          // 0x7C: macDefaultKeySource - 8 octets
    sizeof(uint16_t),               // 0x7D: macPANCoordExtendedAddress     // Not used in ZIP
    sizeof(uint16_t),               // 0x7E: macPANCoordShortAddress        // Not used in ZIP
    0,                              // 0x7F: Unused

    sizeof(uint8_t),                // 0x80: macDefaultKeyEnable
    (16 * sizeof(uint8_t)),         // 0x81: macDefaultKey - 16 octets
    sizeof(uint8_t),                // 0x80: macDefaultSrcAddrEnable
    (8 * sizeof(uint8_t))           // 0x81: macDefaultSrcAddr - 8 octets
};

/* Update this one the arry mac_pib_size is updated. */
#define MIN_MAC_SEC_PIB_ATTRIBUTE_ID        (macKeyTable)
#define MAX_MAC_SEC_PIB_ATTRIBUTE_ID        (macDefaultSrcAddr)
#endif  /* MAC_SECURITY_ZIP */


/* Size constants for Private PIB attributes */
static FLASH_DECLARE(uint8_t private_pib_size[]) =
{
    sizeof(uint64_t)                // 0xF0: macIeeeAddress
#ifdef TEST_HARNESS
    ,
    0,                              // 0xF1: Unused
    0,                              // 0xF2: Unused
    sizeof(uint8_t),                // 0xF3: macPrivateNoDataAfterAssocReq
    sizeof(uint8_t),                // 0xF4: macPrivateIllegalFrameType
    0,                              // 0xF5: Unused
    sizeof(uint8_t),                // 0xF6: macPrivateMACState
    sizeof(uint8_t),                // 0xF7: macPrivateVirtualPANs
    sizeof(uint8_t)                 // 0xF8: macPrivateMACSyncState
#endif /* TEST_HARNESS */
};

/* Update this one the arry private_pib_size is updated. */
#define MIN_PRIVATE_PIB_ATTRIBUTE_ID            (macIeeeAddress)

/* === Prototypes ========================================================== */

#if ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT))
static void recalc_macMaxFrameTotalWaitTime(void);
#endif  /* ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT)) */

/* === Implementation ====================================================== */

#if (MAC_GET_SUPPORT == 1)
/**
 * @brief Handles an MLME-GET.request
 *
 * This function handles an MLME-GET.request.
 * The MLME-GET.request primitive requests information about a
 * given PIB attribute.
 *
 * @param m Pointer to the request structure
 */
void mlme_get_request(uint8_t *m)
{
    /* Use the mlme get request buffer for mlme get confirmation */
    mlme_get_conf_t *mgc = (mlme_get_conf_t *)BMM_BUFFER_POINTER((buffer_t *)m);

    /* Do actual PIB attribute reading */
    {
        pib_value_t *attribute_value = &mgc->PIBAttributeValue;
        uint8_t status = MAC_SUCCESS;

        switch (((mlme_get_req_t *)mgc)->PIBAttribute)
        {
#if (MAC_ASSOCIATION_REQUEST_CONFIRM == 1)
            case macAssociatedPANCoord:
                attribute_value->pib_value_8bit = mac_pib_macAssociatedPANCoord;
                break;
#endif /* (MAC_ASSOCIATION_REQUEST_CONFIRM == 1) */

            case macMaxBE:
                attribute_value->pib_value_8bit = tal_pib_MaxBE;
                break;

#if ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT))
            case macMaxFrameTotalWaitTime:
                memcpy(attribute_value,
                       &mac_pib_macMaxFrameTotalWaitTime,
                       sizeof(uint16_t));
                break;
#endif  /* ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT)) */

            case macMaxFrameRetries:
                attribute_value->pib_value_8bit = tal_pib_MaxFrameRetries;
                break;

            case macResponseWaitTime:
                memcpy(attribute_value,
                       &mac_pib_macResponseWaitTime,
                       sizeof(uint16_t));
                break;

            case macSecurityEnabled:
                attribute_value->pib_value_8bit = mac_pib_macSecurityEnabled;
                break;

            case phyCurrentPage:
                attribute_value->pib_value_8bit = tal_pib_CurrentPage;
                break;

            case phyMaxFrameDuration:
                memcpy(attribute_value, &tal_pib_MaxFrameDuration,
                       sizeof(uint16_t));
                break;

            case phySHRDuration:
                attribute_value->pib_value_8bit = tal_pib_SHRDuration;
                break;

            case phySymbolsPerOctet:
                attribute_value->pib_value_8bit = tal_pib_SymbolsPerOctet;
                break;

            case macAutoRequest:
                attribute_value->pib_value_8bit = mac_pib_macAutoRequest;
                break;

#ifdef BEACON_SUPPORT
            case macBattLifeExt:
                attribute_value->pib_value_8bit = tal_pib_BattLifeExt;
                break;

            case macBattLifeExtPeriods:
                attribute_value->pib_value_8bit = mac_pib_macBattLifeExtPeriods;
                break;

            case macBeaconTxTime:
                memcpy(attribute_value,
                       &tal_pib_BeaconTxTime,
                       sizeof(uint32_t));
                break;

            case macBeaconOrder:
                attribute_value->pib_value_8bit = tal_pib_BeaconOrder;
                break;

            case macSuperframeOrder:
                attribute_value->pib_value_8bit = tal_pib_SuperFrameOrder;
                break;
#endif  /* BEACON_SUPPORT */

#if (MAC_ASSOCIATION_INDICATION_RESPONSE == 1)
            case macAssociationPermit:
                attribute_value->pib_value_8bit = mac_pib_macAssociationPermit;
                break;
#endif /* (MAC_ASSOCIATION_INDICATION_RESPONSE == 1) */

#if (MAC_START_REQUEST_CONFIRM == 1)
            case macBeaconPayload:
                memcpy(attribute_value,
                       mac_beacon_payload,
                       mac_pib_macBeaconPayloadLength);
                break;

            case macBeaconPayloadLength:
                attribute_value->pib_value_8bit = mac_pib_macBeaconPayloadLength;
                break;

            case macBSN:
                attribute_value->pib_value_8bit = mac_pib_macBSN;
                break;
#endif  /* (MAC_START_REQUEST_CONFIRM == 1) */

#if (MAC_INDIRECT_DATA_FFD == 1)
            case macTransactionPersistenceTime:
                memcpy(attribute_value,
                       &mac_pib_macTransactionPersistenceTime,
                       sizeof(uint16_t));
                break;
#endif /* (MAC_INDIRECT_DATA_FFD == 1) */

#ifdef PROMISCUOUS_MODE
            case macPromiscuousMode:
                attribute_value->pib_value_8bit = tal_pib_PromiscuousMode;
                break;
#endif  /* PROMISCUOUS_MODE */

            case macCoordExtendedAddress:
                memcpy(attribute_value,
                       &mac_pib_macCoordExtendedAddress,
                       sizeof(uint64_t));
                break;

            case macCoordShortAddress:
                memcpy(attribute_value,
                       &mac_pib_macCoordShortAddress,
                       sizeof(uint16_t));
                break;

            case macDSN:
                attribute_value->pib_value_8bit = mac_pib_macDSN;
                break;

            case macMaxCSMABackoffs:
                attribute_value->pib_value_8bit = tal_pib_MaxCSMABackoffs;
                break;

            case macMinBE:
                attribute_value->pib_value_8bit = tal_pib_MinBE;
                break;

            case macPANId:
                memcpy(attribute_value,
                       &tal_pib_PANId,
                       sizeof(uint16_t));
                break;

            case macRxOnWhenIdle:
                attribute_value->pib_value_8bit = mac_pib_macRxOnWhenIdle;
                break;

            case macShortAddress:
                memcpy(attribute_value,
                       &tal_pib_ShortAddress,
                       sizeof(uint16_t));
                break;

            case macIeeeAddress:
                memcpy(attribute_value,
                       &tal_pib_IeeeAddress,
                       sizeof(uint64_t));
                break;

            case phyCurrentChannel:
                attribute_value->pib_value_8bit = tal_pib_CurrentChannel;
                break;

            case phyChannelsSupported:
                memcpy(attribute_value,
                       &tal_pib_SupportedChannels,
                       sizeof(uint32_t));
                break;

            case phyTransmitPower:
                attribute_value->pib_value_8bit = tal_pib_TransmitPower;
                break;

            case phyCCAMode:
                attribute_value->pib_value_8bit = tal_pib_CCAMode;
                break;

            default:
                status = MAC_UNSUPPORTED_ATTRIBUTE;
                break;

#ifdef MAC_SECURITY_ZIP
            case macKeyTable:
                /* Todo: The PIB attribute index not handled yet. */
                memcpy(attribute_value,
                       &mac_sec_pib.KeyTable[0],
                       sizeof(mac_key_table_t));
                break;

            case macKeyTableEntries:
                attribute_value->pib_value_8bit = mac_sec_pib.KeyTableEntries;
                break;

            case macDeviceTable:
                /* Todo: The PIB attribute index not handled yet. */
                memcpy(attribute_value,
                       &mac_sec_pib.DeviceTable[0],
                       sizeof(mac_dev_table_t));
                break;

            case macDeviceTableEntries:
                attribute_value->pib_value_8bit = mac_sec_pib.DeviceTableEntries;
                break;

            case macSecurityLevelTable:
                /* Todo: The PIB attribute index not handled yet. */
                memcpy(attribute_value,
                       &mac_sec_pib.SecurityLevelTable[0],
                       sizeof(mac_sec_lvl_table_t));
                break;

            case macSecurityLevelTableEntries:
                attribute_value->pib_value_8bit = mac_sec_pib.SecurityLevelTableEntries;
                break;

            case macFrameCounter:
                memcpy(attribute_value,
                       &mac_sec_pib.FrameCounter,
                       sizeof(uint32_t));
                break;

            case macDefaultKeySource:
                /* Key Source length is 8 octets. */
                memcpy(attribute_value, mac_sec_pib.DefaultKeySource, 8);
                break;

            case macDefaultKeyEnable:
                attribute_value->pib_value_8bit = mac_sec_test_pib.DefaultKeyEnable;
                break;

            case macDefaultKey:
                /* Key length is 16 octets. */
                memcpy(attribute_value, mac_sec_test_pib.DefaultKey, 16);
                break;

            case macDefaultSrcAddrEnable:
                attribute_value->pib_value_8bit = mac_sec_test_pib.DefaultSrcAddrEnable;
                break;

            case macDefaultSrcAddr:
                /* Extended Source Address length is 8 octets. */
                memcpy(attribute_value, &mac_sec_test_pib.DefaultSrcAddr, 8);
                break;

#endif  /* MAC_SECURITY_ZIP */

#ifdef TEST_HARNESS
                /*
                 * The following PIB attributes are privately used for special
                 * test scenarios and are not part of the product code path
                 */
            case macPrivateIllegalFrameType:
                attribute_value->pib_value_8bit = mac_pib_privateIllegalFrameType;
                break;

            case macPrivateMACState:
                attribute_value->pib_value_8bit = mac_state;
                break;

            case macPrivateNoDataAfterAssocReq:
                attribute_value->pib_value_8bit = mac_pib_privateNoDataAfterAssocReq;
                break;

            case macPrivateVirtualPANs:
                attribute_value->pib_value_8bit = mac_pib_privateVirtualPANs;
                break;

            case macPrivateMACSyncState:
                attribute_value->pib_value_8bit = mac_sync_state;
                break;
#endif /* TEST_HARNESS */
        }

        mgc->PIBAttribute = ((mlme_get_req_t *)mgc)->PIBAttribute;
        mgc->cmdcode      = MLME_GET_CONFIRM;
        mgc->status       = status;
    }

    /* Append the mlme get confirmation to MAC-NHLE queue */
    qmm_queue_append(&mac_nhle_q, (buffer_t *)m);
}
#endif  /* (MAC_GET_SUPPORT == 1) */



/**
 * @brief Setting of MAC PIB attributes via functional access
 *
 * In case the highest stack layer is above MAC (e.g. NWK or even
 * higher), it is not efficient to change PIB attributes using
 * the standard request / confirm primitive concept via the NHLE_MAC
 * queue. In order to allow a more efficient way to change PIB attributes
 * residing in MAC or TAL, this function replaces the standard primitive
 * access via a functional interface.
 *
 * An additional parameter allows for forcing the transceiver back to sleep
 * after PIB setting. Otherwise the transceiver will stay awake (if it has been
 * woken up before).
 * This enables the higher layer to change several PIB attributes without
 * waking up the transceiver and putting it back to sleep several times.
 *
 * @param attribute PIB attribute to be set
 * @param attribute_value Attribute value to be set
 * @param set_trx_to_sleep Set TRX back to sleep after this PIB access if it was
 *        before starting this TRX access. Other the transceiver state will
 *        remain as it is, i.e. in case the transceiver was woken up, it stay
 *        awake.
 *        The default value for just changing one variable is true, i.e. the
 *        transceiver will put back to sleep if it has been woken up.
 *
 * @return Status of the attempt to set the TAL PIB attribute:
 *         MAC_UNSUPPORTED_ATTRIBUTE if the PIB attribute was not found
 *         MAC_SUCCESS if the attempt to set the PIB attribute was successful
 *         TAL_BUSY if the TAL is not in an idle state to change PIB attributes
 */
retval_t mlme_set(uint8_t attribute, pib_value_t *attribute_value, bool set_trx_to_sleep)
{
    /*
     * Variables indicates whether the transceiver has been woken up for
     * setting a TAL PIB attribute.
     */
    static bool trx_pib_wakeup;

    retval_t status = MAC_SUCCESS;

    switch (attribute)
    {
#if (MAC_ASSOCIATION_REQUEST_CONFIRM == 1)
        case macAssociatedPANCoord:
            mac_pib_macAssociatedPANCoord = attribute_value->pib_value_8bit;
            break;
#endif /* (MAC_ASSOCIATION_REQUEST_CONFIRM == 1) */

#if ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT))
        case macMaxFrameTotalWaitTime:
            mac_pib_macMaxFrameTotalWaitTime = attribute_value->pib_value_16bit;
            break;
#endif  /* ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT)) */

        case macResponseWaitTime:
            mac_pib_macResponseWaitTime = attribute_value->pib_value_16bit;
            break;

        case macAutoRequest:
#if (MAC_BEACON_NOTIFY_INDICATION == 1)
            /*
             * If the beacon notification indications are not included
             * in the build, macAutoRequest can be changed as desired, since
             * beacon frames will be indicated to the higher
             * layer if required as defined by IEEE 802.15.4.
             */
            mac_pib_macAutoRequest = attribute_value->pib_value_8bit;
            break;
#else
            /*
             * If the beacon notification indications are not included
             * in the build, macAutoRequest must not be changed, since
             * beacon frames will never be indicated to the higher
             * layer, i.e. the higher would not be able to act on
             * received beacon frame information itself.
             */
            status = MAC_INVALID_PARAMETER;
            break;
#endif  /* (MAC_BEACON_NOTIFY_INDICATION == 1) */

        case macBattLifeExtPeriods:
            mac_pib_macBattLifeExtPeriods = attribute_value->pib_value_8bit;
            break;

#if (MAC_ASSOCIATION_INDICATION_RESPONSE == 1)
        case macAssociationPermit:
            mac_pib_macAssociationPermit = attribute_value->pib_value_8bit;
            break;
#endif /* (MAC_ASSOCIATION_INDICATION_RESPONSE == 1) */

#if (MAC_START_REQUEST_CONFIRM == 1)
        case macBeaconPayload:
            memcpy(mac_beacon_payload, attribute_value,
                   mac_pib_macBeaconPayloadLength);
            break;

        case macBeaconPayloadLength:
#ifndef REDUCED_PARAM_CHECK
            /*
             * If the application sits directly  on top of the MAC,
             * this is also checked in mac_api.c.
             */
            if (attribute_value->pib_value_8bit > aMaxBeaconPayloadLength)
            {
                status = MAC_INVALID_PARAMETER;
                break;
            }
#endif  /* REDUCED_PARAM_CHECK */
            mac_pib_macBeaconPayloadLength = attribute_value->pib_value_8bit;
            break;

        case macBSN:
            mac_pib_macBSN = attribute_value->pib_value_8bit;
            break;
#endif  /* (MAC_START_REQUEST_CONFIRM == 1) */

#if (MAC_INDIRECT_DATA_FFD == 1)
         case macTransactionPersistenceTime:
            mac_pib_macTransactionPersistenceTime = attribute_value->pib_value_16bit;
            break;
#endif /* (MAC_INDIRECT_DATA_FFD == 1) */
            case macCoordExtendedAddress:
            mac_pib_macCoordExtendedAddress = attribute_value->pib_value_64bit;
            break;

        case macCoordShortAddress:
            mac_pib_macCoordShortAddress = attribute_value->pib_value_16bit;
            break;

        case macDSN:
            mac_pib_macDSN = attribute_value->pib_value_8bit;
            break;

        case macRxOnWhenIdle:
            mac_pib_macRxOnWhenIdle = attribute_value->pib_value_8bit;
            /* Check whether radio state needs to change now, */
            if (mac_pib_macRxOnWhenIdle)
            {
                /* Check whether the radio needs to be woken up. */
                mac_trx_wakeup();
                /* Set transceiver in rx mode, otherwise it may stay in TRX_OFF). */
                tal_rx_enable(PHY_RX_ON);
            }
            else
            {
                /* Check whether the radio needs to be put to sleep. */
                mac_sleep_trans();
            }
            break;

        case macBattLifeExt:
        case macBeaconOrder:
        case macMaxCSMABackoffs:
        case macMaxBE:
        case macMaxFrameRetries:
        case macMinBE:
        case macPANId:
#ifdef PROMISCUOUS_MODE
        case macPromiscuousMode:
#endif/* PROMISCUOUS_MODE */
        case macShortAddress:
        case macSuperframeOrder:
        case macIeeeAddress:
        case phyCurrentChannel:
        case phyCurrentPage:
        case phyTransmitPower:
        case phyCCAMode:
#ifdef TEST_HARNESS
        case macPrivateCCAFailure:
        case macPrivateDisableACK:
#endif /* TEST_HARNESS */
            {
                /* Now only TAL PIB attributes are handled anymore. */
                status = tal_pib_set(attribute, attribute_value);

                if (status == TAL_TRX_ASLEEP)
                {
                    /*
                     * Wake up the transceiver and repeat the attempt
                     * to set the TAL PIB attribute.
                     */
                    tal_trx_wakeup();
                    status = tal_pib_set(attribute, attribute_value);
                    if (status == MAC_SUCCESS)
                    {
                        /*
                         * Set flag indicating that the trx has been woken up
                         * during PIB setting.
                         */
                        trx_pib_wakeup = true;
                    }
               }

#if ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT))
               /*
                * In any case that the PIB setting was successful (no matter
                * whether the trx had to be woken up or not), the PIB attribute
                * recalculation needs to be done.
                */
               if (status == MAC_SUCCESS)
               {
                   /*
                    * The value of the PIB attribute
                    * macMaxFrameTotalWaitTime depends on the values of the
                    * following PIB attributes:
                    * macMinBE
                    * macMaxBE
                    * macMaxCSMABackoffs
                    * phyMaxFrameDuration
                    * In order to save code space and since changing of PIB
                    * attributes is going to happen not too often, this is done
                    * always whenever a PIB attribute residing in TAL is changed
                    * (since all above mentioned PIB attributes are in TAL).
                    */
                   recalc_macMaxFrameTotalWaitTime();
               }
#endif  /* ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT)) */
            }
            break;

        case macAckWaitDuration:
        default:
            status = MAC_UNSUPPORTED_ATTRIBUTE;
            break;

#ifdef MAC_SECURITY_ZIP
        case macSecurityEnabled:
            mac_pib_macSecurityEnabled = attribute_value->pib_value_8bit;
            break;

        case macKeyTable:
            /* Todo: The PIB attribute index not handled yet. */
            memcpy(&mac_sec_pib.KeyTable[0], attribute_value, sizeof(mac_key_table_t));
            break;

        case macKeyTableEntries:
            mac_sec_pib.KeyTableEntries = attribute_value->pib_value_8bit;
            break;

        case macDeviceTable:
            /* Todo: The PIB attribute index not handled yet. */
            memcpy(&mac_sec_pib.DeviceTable[0], attribute_value, sizeof(mac_dev_table_t));
            break;

        case macDeviceTableEntries:
            mac_sec_pib.DeviceTableEntries = attribute_value->pib_value_8bit;
            break;

        case macSecurityLevelTable:
            /* Todo: The PIB attribute index not handled yet. */
            memcpy(&mac_sec_pib.SecurityLevelTable[0], attribute_value, sizeof(mac_sec_lvl_table_t));
            break;

        case macSecurityLevelTableEntries:
            mac_sec_pib.SecurityLevelTableEntries = attribute_value->pib_value_8bit;
            break;

        case macFrameCounter:
            mac_sec_pib.FrameCounter = attribute_value->pib_value_32bit;
            break;

        case macDefaultKeySource:
            /* Key Source length is 8 octets. */
            memcpy(mac_sec_pib.DefaultKeySource, attribute_value, 8);
            break;

        case macDefaultKeyEnable:
            mac_sec_test_pib.DefaultKeyEnable = attribute_value->pib_value_8bit;
            break;

        case macDefaultKey:
            /* Key length is 16 octets. */
            memcpy(mac_sec_test_pib.DefaultKey, attribute_value, 16);
            break;

        case macDefaultSrcAddrEnable:
            mac_sec_test_pib.DefaultSrcAddrEnable = attribute_value->pib_value_8bit;
            break;

        case macDefaultSrcAddr:
            /* Key length is 8 octets. */
            memcpy(&mac_sec_test_pib.DefaultSrcAddr, attribute_value, 8);
            break;

#endif  /* MAC_SECURITY_ZIP */

#ifdef TEST_HARNESS
        case macPrivateIllegalFrameType:
            mac_pib_privateIllegalFrameType = attribute_value->pib_value_8bit;
            break;

        case macPrivateNoDataAfterAssocReq:
            mac_pib_privateNoDataAfterAssocReq = attribute_value->pib_value_8bit;
            break;

        case macPrivateVirtualPANs:
            mac_pib_privateVirtualPANs = attribute_value->pib_value_8bit;
            break;
#endif /* TEST_HARNESS */
    }

    /*
     * In case the transceiver shall be forced back to sleep and
     * has been woken up, it is put back to sleep again.
     */
    if (set_trx_to_sleep && trx_pib_wakeup)
    {
        tal_trx_sleep(SLEEP_MODE_1);
        trx_pib_wakeup = false;
    }

    return status;
}



#if (HIGHEST_STACK_LAYER == MAC)
/**
 * @brief Handles an MLME-SET.request primitive
 *
 * This function handles the MLME-SET.request. The MLME-SET.request primitive
 * attempts to write the given value to the indicated PIB attribute.
 *
 * @param m Pointer to the request structure
 */
void mlme_set_request(uint8_t *m)
{
    mlme_set_req_t  *msr = (mlme_set_req_t *)BMM_BUFFER_POINTER((buffer_t *)m);

    /* Do the actual PIB attribute set operation */
    {
        pib_value_t *attribute_value = &msr->PIBAttributeValue;
        retval_t status = MAC_SUCCESS;
        mlme_set_conf_t *msc;

        /*
         * Call internal PIB attribute handling function. Always force
         * the trx back to sleep when using request primitives via the
         * MLME queue.
         */
        status = mlme_set(msr->PIBAttribute, attribute_value, true);

        msc = (mlme_set_conf_t *)msr;
        msc->PIBAttribute = msr->PIBAttribute;
        msc->cmdcode      = MLME_SET_CONFIRM;
        msc->status       = status;
    }

    /* Append the mlme set confirmation message to the MAC-NHLE queue */
    qmm_queue_append(&mac_nhle_q, (buffer_t *)m);
}
#endif  /* (HIGHEST_STACK_LAYER == MAC) */



/**
 * @brief Wakes-up the radio and sets the corresponding TAL PIB attribute
 *
 * @param attribute PIB attribute to be set
 * @param attribute_value Attribute value to be set
 *
 * @return Status of the attempt to set the TAL PIB attribute
 */
retval_t set_tal_pib_internal(uint8_t attribute, pib_value_t *attribute_value)
{
    retval_t status;

    if (RADIO_SLEEPING == mac_radio_sleep_state)
    {
        /* Wake up the radio */
        mac_trx_wakeup();

        status = tal_pib_set(attribute, attribute_value);

        /* Set radio to sleep if allowed */
        mac_sleep_trans();

    }
    else
    {
        status = tal_pib_set(attribute, attribute_value);
    }

    return status;
}



#if ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT))
/**
 * @brief Re-caclulates PIB attribute macMaxFrameTotalWaitTime
 *
 * This function re-calculates the MAC PIB attribute macMaxFrameTotalWaitTime
 * whenever one of the following PIB attributes change:
 * macMinBE
 * macMaxBE
 * macMaxCSMABackoffs
 * phyMaxFrameDuration
 *
 * See IEEE 802.15.4-2006 equation (14) in section 7.4.2.
 */
static void recalc_macMaxFrameTotalWaitTime(void)
{
    uint8_t m;

    m = (uint8_t)MIN((tal_pib_MaxBE - tal_pib_MinBE), tal_pib_MaxCSMABackoffs);

    mac_pib_macMaxFrameTotalWaitTime = (tal_pib_MaxCSMABackoffs - m) * ((1 << tal_pib_MaxBE) - 1);

    /* Calculate sum of equation (14). */
    for (uint8_t k = 0; k < m; k++)
    {
        mac_pib_macMaxFrameTotalWaitTime += 1 << (tal_pib_MinBE + k);
    }

    /* Calculate the rest. */
    mac_pib_macMaxFrameTotalWaitTime *= aUnitBackoffPeriod;
    mac_pib_macMaxFrameTotalWaitTime += MAX_FRAME_DURATION;
}
#endif  /* ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT)) */



/**
 * @brief Gets the size of a PIB attribute
 *
 * @param attribute PIB attribute
 *
 * @return Size (number of bytes) of the PIB attribute
 */
uint8_t mac_get_pib_attribute_size(uint8_t pib_attribute_id)
{
#if (MAC_START_REQUEST_CONFIRM == 1)
    /*
     * Since the current length of the beacon payload is not a contant, but
     * a variable, it cannot be stored in a Flash table. Therefore we need
     * to handle this PIB attribute special.
     */
    if (macBeaconPayload == pib_attribute_id)
    {
       return (mac_pib_macBeaconPayloadLength);
    }
#endif  /* (MAC_START_REQUEST_CONFIRM == 1) */

    if (MAX_PHY_PIB_ATTRIBUTE_ID >= pib_attribute_id)
    {
       return (PGM_READ_BYTE(&phy_pib_size[pib_attribute_id]));
    }

    if (MIN_MAC_PIB_ATTRIBUTE_ID <= pib_attribute_id && MAX_MAC_PIB_ATTRIBUTE_ID >= pib_attribute_id)
    {
       return(PGM_READ_BYTE(&mac_pib_size[pib_attribute_id - MIN_MAC_PIB_ATTRIBUTE_ID]));
    }

#ifdef MAC_SECURITY_ZIP
    if (MIN_MAC_SEC_PIB_ATTRIBUTE_ID <= pib_attribute_id && MAX_MAC_SEC_PIB_ATTRIBUTE_ID >= pib_attribute_id)
    {
       return(PGM_READ_BYTE(&mac_sec_pib_size[pib_attribute_id - MIN_MAC_SEC_PIB_ATTRIBUTE_ID]));
    }
#endif  /* MAC_SECURITY_ZIP */

    if (MIN_PRIVATE_PIB_ATTRIBUTE_ID <= pib_attribute_id)
    {
        return(PGM_READ_BYTE(&private_pib_size[pib_attribute_id - MIN_PRIVATE_PIB_ATTRIBUTE_ID]));
    }

    return(0);
}


/* EOF */
