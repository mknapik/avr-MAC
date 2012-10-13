/**
 * @file mac.c
 *
 * @brief This module runs the MAC scheduler.
 *
 * $Id: mac.c 22667 2010-07-23 12:48:57Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================= */

#include <stddef.h>
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

/* === Macros =============================================================== */


/* === Globals ============================================================== */

/**
 * Current state of the MAC state machine
 */
mac_state_t mac_state;

/**
 * Current state of scanning process.
 */
mac_scan_state_t mac_scan_state;

/**
 * Current state of syncronization with parent
 * (either coordinator or PAN coordinator).
 */
mac_sync_state_t mac_sync_state;

/**
 * Current state of MAC poll state machine,
 * e.g. polling for data, polling for Association Response, etc.
 */
mac_poll_state_t mac_poll_state;

/**
 * Radio sleep state
 */
mac_radio_sleep_state_t mac_radio_sleep_state;

#ifdef BEACON_SUPPORT
/**
 * Final Cap Slot of current Superframe
 */
uint8_t mac_final_cap_slot;

/**
 * Flag stating that the last received beacon frame from the parent
 * indicated pending broadcast data to be received.
 */
bool mac_bc_data_indicated;
#endif  /* BEACON_SUPPORT */

/**
 * Global parse data
 */
parse_t mac_parse_data;

/*
 * Flag indicating that RxEnable is still active.
 */
bool mac_rx_enabled;

/*
 * Variables for duplicate detection.
 * In order to detect duplicated frames, the DSN and Source Address of the
 * last received data frame need to be stored.
 */
uint8_t mac_last_dsn;
uint64_t mac_last_src_addr;


/* MAC PIB variables */

#if (MAC_ASSOCIATION_REQUEST_CONFIRM == 1)
/**
 * Indication of whether the device is associated to the PAN through the PAN
 * coordinator. A value of TRUE indicates the device has associated through the
 * PAN coordinator. Otherwise, the value is set to FALSE.
 */
uint8_t mac_pib_macAssociatedPANCoord;
#endif /* (MAC_ASSOCIATION_REQUEST_CONFIRM == 1) */

#if ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT))
/**
 * The maximum number of CAP symbols in a beaconenabled PAN, or symbols in a
 * nonbeacon-enabled PAN, to wait either for a frame intended as a response to
 * a data request frame or for a broadcast frame following a beacon with the
 * Frame Pending subfield set to one.
 * This attribute, which shall only be set by the next higher layer, is
 * dependent upon macMinBE, macMaxBE, macMaxCSMABackoffs and the number of
 * symbols per octet. See 7.4.2 for the formula relating the attributes.
 * Maximum values:
 * O-QPSK (2.4 GHz and 900 MHz for Channel page 2): 25766
 * BPSK (900 MHz for Channel page 0): 26564
 * Both values are valid for
 * macMinBE = 8
 * macMaxBE = 8
 * macMaxCSMABackoffs = 5
 *
 * This PIB attribute is only used if basic indirect data transmission is used
 * or if beacon enabled network is enabled.
 */
uint16_t mac_pib_macMaxFrameTotalWaitTime;
#endif  /* ((MAC_INDIRECT_DATA_BASIC == 1) || defined(BEACON_SUPPORT)) */

/**
 * The maximum time, in multiples of aBaseSuperframeDuration, a device shall
 * wait for a response command frame to be available following a request
 * command frame.
 */
uint16_t mac_pib_macResponseWaitTime;

/**
 * Indication of whether the MAC sublayer has security enabled. A value of
 * TRUE indicates that security is enabled, while a value of FALSE indicates
 * that security is disabled.
 */
bool mac_pib_macSecurityEnabled;

#if (MAC_ASSOCIATION_INDICATION_RESPONSE == 1)
/**
 * Holds the value which states whether a coordinator is currently allowing
 * association. A value of true indicates that association is permitted.
 */
uint8_t mac_pib_macAssociationPermit;
#endif /* (MAC_ASSOCIATION_INDICATION_RESPONSE == 1) */

#if (MAC_INDIRECT_DATA_BASIC == 1)
/**
 * Holds the maximum time (in superframe periods) that a indirect transaction
 * is stored by a PAN coordinator.
 */
uint16_t mac_pib_macTransactionPersistenceTime;
#endif /* (MAC_INDIRECT_DATA_BASIC == 1) */


#if (MAC_START_REQUEST_CONFIRM == 1)
/**
 * Holds the sequence number added to the transmitted beacon frame.
 */
uint8_t mac_pib_macBSN;

/**
 * Holds the contents of the beacon payload.
 */
uint8_t mac_beacon_payload[aMaxBeaconPayloadLength];

/**
 * Holds the length, in octets, of the beacon payload.
 */
uint8_t mac_pib_macBeaconPayloadLength;
#endif  /* (MAC_START_REQUEST_CONFIRM == 1) */

/**
 * Holds the value which states whether a device automatically sends a data
 * request command if its address is listed in the beacon frame. A value of true
 * indicates that the data request command is automatically sent.
 */
uint8_t mac_pib_macAutoRequest;

/**
 * Holds the value which states the number of backoff periods during which the
 * receiver is enabled following a beacon in battery life extension mode.
 * This value is dependent on the currently selected logical channel.
 */
uint8_t mac_pib_macBattLifeExtPeriods;

/**
 * Holds the 64 bit address of the coordinator with which the
 * device is associated.
 */
uint64_t mac_pib_macCoordExtendedAddress;

/**
 * Holds the 16 bit short address of the coordinator with which the device is
 * associated. A value of 0xfffe indicates that the coordinator is only using
 * its 64 bit extended address. A value of 0xffff indicates that this
 * value is unknown.
 */
uint16_t mac_pib_macCoordShortAddress;

/**
 * Holds the sequence number of the transmitted data or command frame.
 */
uint8_t mac_pib_macDSN;

/**
 * Holds the value which states whether the MAC sublayer is to enable its
 * receiver during idle periods.
 */
bool mac_pib_macRxOnWhenIdle;

#ifdef MAC_SECURITY_ZIP
/**
 * Holds the values of all security related PIB attributes.
 */
mac_sec_pib_t mac_sec_pib;

/**
 * Holds the values of all security related test PIB attributes.
 */
mac_sec_test_pib_t mac_sec_test_pib;
#endif  /* MAC_SECURITY_ZIP */

#ifdef TEST_HARNESS
/* Private MAC PIB variables, only valid for testing purposes */

/**
 * Holds the private MAC PIB attribute to generate a frame with an illegale
 * frame type.
 */
uint8_t mac_pib_privateIllegalFrameType;

/**
 * Holds the private MAC PIB attribute which suppresses the initiation of a
 * data request frame after association request.
 */
uint8_t mac_pib_privateNoDataAfterAssocReq;

/**
 * Holds the private MAC PIB attribute to pretend virtual Beacon-enabled PANs.
 */
uint8_t mac_pib_privateVirtualPANs;

#endif /* TEST_HARNESS */

/**
 * Holds the mlme request buffer pointer, used to give the respective
 * confirmation in scan, poll and association.
 */
uint8_t *mac_conf_buf_ptr;

#if (MAC_SCAN_SUPPORT == 1)
/**
 * Stores the original channel before start of scanning.
 */
uint8_t mac_scan_orig_channel;

/**
 * Stores the original channel page before start of scanning.
 */
uint8_t mac_scan_orig_page;

#if ((MAC_SCAN_ACTIVE_REQUEST_CONFIRM == 1) || (MAC_SCAN_PASSIVE_REQUEST_CONFIRM == 1))
/**
 * Stores the original PAN-Id before start of scanning.
 */
uint16_t mac_scan_orig_panid;
#endif /* ((MAC_SCAN_ACTIVE_REQUEST_CONFIRM == 1) || (MAC_SCAN_PASSIVE_REQUEST_CONFIRM == 1)) */

/**
 * Holds the buffer pointer which is used to send scan command.
 */
uint8_t *mac_scan_cmd_buf_ptr;
#endif /* (MAC_SCAN_SUPPORT == 1) */

/**
 * MAC busy state, indicates whether MAC can process any
 * request from NHLE.
 */
bool mac_busy;

/**
 * NHLE to MAC queue in which NHLE pushes all the requests to the MAC layer
 */
queue_t nhle_mac_q;

/**
 * Queue used by MAC for its internal operation. TAL pushes the incoming frames
 * in this queue.
 */
queue_t tal_mac_q;

#if (MAC_START_REQUEST_CONFIRM == 1)
#ifdef BEACON_SUPPORT
/**
 * Queue used by MAC layer in beacon-enabled network to put in broadcast data.
 * Any broadcast data given by NHLE at a Coordinator or PAN Coordinator
 * in a beacon-enabled network is placed here by MAC.
 */
queue_t broadcast_q;
#endif  /* BEACON_SUPPORT */
#endif /* (MAC_START_REQUEST_CONFIRM == 1) */


#if (MAC_INDIRECT_DATA_FFD == 1)
/**
 * Queue used by MAC layer to put in indirect data. Any indirect data given by
 * NHLE is placed here by MAC, until the device polls for the data.
 */
queue_t indirect_data_q;
#endif /* (MAC_INDIRECT_DATA_FFD == 1) */

/* === Prototypes =========================================================== */


/* === Implementation ======================================================= */

/**
 * @brief Runs the MAC scheduler
 *
 * This function runs the MAC scheduler.
 *
 * MLME and MCPS queues are removed alternately, starting with MLME queue.
 *
 * @return true if event is dispatched, false if no event to dispatch.
 */
bool mac_task(void)
{
    uint8_t *event = NULL;
    bool processed_event = false;

    if (!mac_busy)
    {
        /* Check whether queue is empty */
        if (nhle_mac_q.size != 0)
        {
            event = (uint8_t *)qmm_queue_remove(&nhle_mac_q, NULL);

            /* If an event has been detected, handle it. */
            if (NULL != event)
            {
                /* Process event due to NHLE requests */
                dispatch_event(event);
                processed_event = true;
            }
        }
    }

    /*
     * Internal event queue should be dispatched
     * irrespective of the dispatcher state.
     */
    /* Check whether queue is empty */
    if (tal_mac_q.size != 0)
    {
        event = (uint8_t *)qmm_queue_remove(&tal_mac_q, NULL);

        /* If an event has been detected, handle it. */
        if (NULL != event)
        {
            dispatch_event(event);
            processed_event = true;
        }
    }

    return processed_event;
}

/* EOF */
