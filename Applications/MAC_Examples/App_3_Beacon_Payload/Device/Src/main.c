/**
 * @file main.c
 *
 * @brief MAC Example App 3 Beacon Payload - Device
 *
 * This is the source code of a simple MAC example. It implements the
 * firmware for all devices of a beacon_enabled network with star topology.
 *
 * The coordinator starts a beaconing network and transmits user data within
 * beacon payload of transmitted beacon frames.
 * The devices receive these beacon frames, extract the receveived user data
 * from the coordinator and print the received data on the terminal.
 *
 * $Id: main.c 22865 2010-08-11 11:57:09Z sschneid $
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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef SIO_HUB
#include <inttypes.h>
#endif  /* SIO_HUB */
#include "pal.h"
#include "tal.h"
#ifdef SIO_HUB
#include "sio_handler.h"
#endif  /* SIO_HUB */
#include "mac_api.h"
#include "app_config.h"
#include "ieee_const.h"

/* === TYPES =============================================================== */

/**
 * This enum stores the current state of the application.
 */
typedef enum app_state_tag
{
    APP_IDLE = 0,
    APP_SCAN_DONE,
    APP_ASSOC_IN_PROGRESS,
    APP_DEVICE_RUNNING
}
app_state_t;

/* === MACROS ============================================================== */

/** Defines the default channel. */
#if (TAL_TYPE == AT86RF212)
    #ifdef CHINESE_BAND
        #define DEFAULT_CHANNEL                 (0)
        #define DEFAULT_CHANNEL_PAGE            (5)
    #else
        #define DEFAULT_CHANNEL                 (1)
        #define DEFAULT_CHANNEL_PAGE            (0)
    #endif  /* #ifdef CHINESE_BAND */
#else
#define DEFAULT_CHANNEL                 (20)
#define DEFAULT_CHANNEL_PAGE            (0)
#endif  /* #if (TAL_TYPE == AT86RF212) */
/** Defines the PAN ID of the network. */
#define DEFAULT_PAN_ID                  (0xBABE)
/** This is a device which will communicate with a single coordinator.
 *  Therefore, the maximum number of devices this code needs to
 *  handle is one.
 */
#define MAX_NUMBER_OF_DEVICES           (1)

/** This is the time period in micro seconds for data transmissions. */
#define DATA_TX_PERIOD                  (2000000)

/** Defines the bit mask of channels that should be scanned. */
#if (TAL_TYPE == AT86RF212)
    #if (DEFAULT_CHANNEL == 0)
        #define SCAN_ALL_CHANNELS       (0x00000001)
    #else
        #define SCAN_ALL_CHANNELS       (0x000007FE)
    #endif
#else
#define SCAN_ALL_CHANNELS               (1UL << DEFAULT_CHANNEL)
#endif
/** Defines the short scan duration time. */
#define SCAN_DURATION_SHORT             (7)
/** Defines the long scan duration time. */
#define SCAN_DURATION_LONG              (8)

/** Defines the length of the msdu payload delivered to the coordinator. */
#ifndef SIO_HUB
    #define PAYLOAD_LEN                     (104)
#else
    /*
     * If SIO_HUB is used we need an additional element at the end of the array
     * for printing the result on the terminal to avoid additional copying.
     */
    #define PAYLOAD_LEN                     (104 + 1)
#endif  /* SIO_HUB */

#if (NO_OF_LEDS >= 3)
#define LED_START                       (LED_0)
#define LED_NWK_SETUP                   (LED_1)
#define LED_DATA                        (LED_2)
#elif (NO_OF_LEDS == 2)
#define LED_START                       (LED_0)
#define LED_NWK_SETUP                   (LED_0)
#define LED_DATA                        (LED_1)
#else
#define LED_START                       (LED_0)
#define LED_NWK_SETUP                   (LED_0)
#define LED_DATA                        (LED_0)
#endif

/* === GLOBALS ============================================================= */

/** This structure stores the short and extended address of the coordinator. */
static wpan_addr_spec_t coord_addr_spec;
/** This variable stores the current state of the node. */
static app_state_t app_state = APP_IDLE;
/** This array stores the current msdu payload. */
static uint8_t msdu_payload[PAYLOAD_LEN];

/* === PROTOTYPES ========================================================== */

static void network_search_indication_cb(void *parameter);
static void data_exchange_led_off_cb(void *parameter);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the device application
 */
int main(void)
{
    /* Initialize the MAC layer and its underlying layers, like PAL, TAL, BMM. */
    if (wpan_init() != MAC_SUCCESS)
    {
        /*
         * Stay here; we need a valid IEEE address.
         * Check kit documentation how to create an IEEE address
         * and to store it into the EEPROM.
         */
        pal_alert();
    }

    /* Initialize LEDs. */
    pal_led_init();
    pal_led(LED_START, LED_ON);         // indicating application is started
    pal_led(LED_NWK_SETUP, LED_OFF);    // indicating node is associated
    pal_led(LED_DATA, LED_OFF);         // indicating successfull data transmission

    /*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();

#ifdef SIO_HUB
    /* Initialize the serial interface used for communication with terminal program. */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        /* Something went wrong during initialization. */
        pal_alert();
    }

#if ((!defined __ICCAVR__) && (!defined __ICCARM__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif

    printf("\nApp_3_Beacon_Payload\n\n");
    printf("\nDevice\n\n");
#endif  /* SIO_HUB */

    /*
     * Reset the MAC layer to the default values.
     * This request will cause a mlme reset confirm message ->
     * usr_mlme_reset_conf
     */
    wpan_mlme_reset_req(true);

    /* Main loop */
    while (1)
    {
        wpan_task();
    }
}



/**
 * @brief Callback function usr_mlme_reset_conf
 *
 * @param status Result of the reset procedure
 */
void usr_mlme_reset_conf(uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
#ifdef SIO_HUB
        printf("Searching network\n");
#endif  /* SIO_HUB */

        /*
         * Initiate an active scan over all channels to determine
         * which channel is used by the coordinator.
         * Use: bool wpan_mlme_scan_req(uint8_t ScanType,
         *                              uint32_t ScanChannels,
         *                              uint8_t ScanDuration,
         *                              uint8_t ChannelPage);
         *
         * This request leads to a scan confirm message -> usr_mlme_scan_conf
         * Scan for about 50 ms on each channel -> ScanDuration = 1
         * Scan for about 1/2 second on each channel -> ScanDuration = 5
         * Scan for about 1 second on each channel -> ScanDuration = 6
         */
        wpan_mlme_scan_req(MLME_SCAN_TYPE_ACTIVE,
                           SCAN_ALL_CHANNELS,
                           SCAN_DURATION_SHORT,
                           DEFAULT_CHANNEL_PAGE);

        /* Indicate network scanning by a LED flashing. */
        pal_timer_start(TIMER_LED_OFF,
                        500000,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR)network_search_indication_cb,
                        NULL);
    }
    else
    {
        /* Set proper state of application. */
        app_state = APP_IDLE;

        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }
}



/**
 * @brief Callback function usr_mlme_scan_conf
 *
 * @param status            Result of requested scan operation
 * @param ScanType          Type of scan performed
 * @param ChannelPage       Channel page on which the scan was performed
 * @param UnscannedChannels Bitmap of unscanned channels
 * @param ResultListSize    Number of elements in ResultList
 * @param ResultList        Pointer to array of scan results
 */
void usr_mlme_scan_conf(uint8_t status,
                        uint8_t ScanType,
                        uint8_t ChannelPage,
                        uint32_t UnscannedChannels,
                        uint8_t ResultListSize,
                        void *ResultList)
{
    if (status == MAC_SUCCESS)
    {
        wpan_pandescriptor_t *coordinator;
        uint8_t i;

        /*
         * Analyze the ResultList.
         * Assume that the first entry of the result list is our coodinator.
         */
        coordinator = (wpan_pandescriptor_t *)ResultList;

        for (i = 0; i < ResultListSize; i++)
        {
            /*
             * Check if the PAN descriptor belongs to our coordinator.
             * Check if coordinator allows association.
             */
            if ((coordinator->LogicalChannel == DEFAULT_CHANNEL) &&
                (coordinator->ChannelPage == DEFAULT_CHANNEL_PAGE) &&
                (coordinator->CoordAddrSpec.PANId == DEFAULT_PAN_ID) &&
                ((coordinator->SuperframeSpec & ((uint16_t)1 << ASSOC_PERMIT_BIT_POS)) == ((uint16_t)1 << ASSOC_PERMIT_BIT_POS))
               )
            {
                /* Store the coordinator's address information. */
                coord_addr_spec.AddrMode = WPAN_ADDRMODE_SHORT;
                coord_addr_spec.PANId = DEFAULT_PAN_ID;
                ADDR_COPY_DST_SRC_16(coord_addr_spec.Addr.short_address, coordinator->CoordAddrSpec.Addr.short_address);

#ifdef SIO_HUB
                printf("Found network\n");
#endif  /* SIO_HUB */

                /* Set proper state of application. */
                app_state = APP_SCAN_DONE;

                /*
                 * Set the PAN-Id of the scanned network.
                 * This is required in order to perform a proper sync
                 * before assocation.
                 * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
                 *                             void *PIBAttributeValue);
                 *
                 * This request leads to a set confirm message -> usr_mlme_set_conf
                 */
                uint8_t pan_id[2];

                pan_id[0] = (uint8_t)DEFAULT_PAN_ID;          // low byte
                pan_id[1] = (uint8_t)(DEFAULT_PAN_ID >> 8);   // high byte
                wpan_mlme_set_req(macPANId, pan_id);

                return;
            }

            /* Get the next PAN descriptor. */
            coordinator++;
        }

        /*
         * If here, the result list does not contain our expected coordinator.
         * Let's scan again.
         */
        wpan_mlme_scan_req(MLME_SCAN_TYPE_ACTIVE,
                           SCAN_ALL_CHANNELS,
                           SCAN_DURATION_SHORT,
                           DEFAULT_CHANNEL_PAGE);
    }
    else if (status == MAC_NO_BEACON)
    {
        /*
         * No beacon is received; no coordiantor is located.
         * Scan again, but used longer scan duration.
         */
        wpan_mlme_scan_req(MLME_SCAN_TYPE_ACTIVE,
                           SCAN_ALL_CHANNELS,
                           SCAN_DURATION_LONG,
                           DEFAULT_CHANNEL_PAGE);
    }
    else
    {
        /* Set proper state of application. */
        app_state = APP_IDLE;

        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    ScanType = ScanType;
    ChannelPage = ChannelPage;
    UnscannedChannels = UnscannedChannels;
}



/**
 * @brief Callback function usr_mlme_set_conf
 *
 * @param status        Result of requested PIB attribute set operation
 * @param PIBAttribute  Updated PIB attribute
 */
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
    if ((status == MAC_SUCCESS) && (PIBAttribute == macPANId))
    {
        /*
         * Set the Coordinator Short Address of the scanned network.
         * This is required in order to perform a proper sync
         * before assocation.
         */
         wpan_mlme_set_req(macCoordShortAddress, &coord_addr_spec.Addr);
    }
    else if ((status == MAC_SUCCESS) && (PIBAttribute == macCoordShortAddress))
    {
        /*
         * Sync with beacon frames from our coordinator.
         * Use: bool wpan_mlme_sync_req(uint8_t LogicalChannel,
         *                              uint8_t ChannelPage,
         *                              bool TrackBeacon);
         *
         * This does not lead to an immediate reaction.
         *
         * In case we receive beacon frames from our coordinator including
         * a beacon payload, this is indicated in the callback function
         * usr_mlme_beacon_notify_ind().
         *
         * In case the device cannot find its coordinator or later looses
         * synchronization with its parent, this is indicated in the
         * callback function usr_mlme_sync_loss_ind().
         */
        wpan_mlme_sync_req(DEFAULT_CHANNEL,
                           DEFAULT_CHANNEL_PAGE,
                           1);
    }
    else
    {
        /* Set proper state of application. */
        app_state = APP_IDLE;

        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }
}



/**
 * @brief Callback function usr_mlme_associate_conf
 *
 * @param AssocShortAddress    Short address allocated by the coordinator
 * @param status               Result of requested association operation
 */
void usr_mlme_associate_conf(uint16_t AssocShortAddress, uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
#ifdef SIO_HUB
        printf("Conntected to beacon-enabled network\n");
#endif  /* SIO_HUB */

        /* Set proper state of application. */
        app_state = APP_DEVICE_RUNNING;

        /* Stop timer used for search indication (same as used for data transmission). */
        pal_timer_stop(TIMER_LED_OFF);
        pal_led(LED_NWK_SETUP, LED_ON);
    }
    else
    {
        /* Set proper state of application. */
        app_state = APP_IDLE;

        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    AssocShortAddress = AssocShortAddress;
}



/**
 * Callback function usr_mcps_data_conf
 *
 * @param msduHandle  Handle of MSDU handed over to MAC earlier
 * @param status      Result for requested data transmission request
 * @param Timestamp   The time, in symbols, at which the data were transmitted
 *                    (only if timestamping is enabled).
 *
 */
#ifdef ENABLE_TSTAMP
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status, uint32_t Timestamp)
#else
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status)
#endif  /*  ENABLE_TSTAMP*/
{
    if (status == MAC_SUCCESS)
    {
        /*
         * Dummy data has been transmitted successfully.
         * Application code could be added here ...
         */
        pal_led(LED_DATA, LED_ON);
        /* Start a timer switching off the LED. */
        pal_timer_start(TIMER_LED_OFF,
                        500000,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR)data_exchange_led_off_cb,
                        NULL);
    }

    /* Keep compiler happy. */
    msduHandle = msduHandle;
#ifdef ENABLE_TSTAMP
    Timestamp = Timestamp;
#endif  /*  ENABLE_TSTAMP*/
}



/**
 * Callback function usr_mlme_sync_loss_ind
 *
 * @param LossReason     Reason for synchronization loss.
 * @param PANId          The PAN identifier with which the device lost
 *                       synchronization or to which it was realigned.
 * @param LogicalChannel The logical channel on which the device lost
 *                       synchronization or to which it was realigned.
 * @param ChannelPage    The channel page on which the device lost
 *                       synchronization or to which it was realigned.
 */
void usr_mlme_sync_loss_ind(uint8_t LossReason,
                            uint16_t PANId,
                            uint8_t LogicalChannel,
                            uint8_t ChannelPage)
{
    /*
     * Once we lost sync this the coordinator we need to re-sync.
     * Since we the network parameter are not supposed ot change,
     * use the already known parameters form our coordinator.
     */
    wpan_mlme_sync_req(LogicalChannel,
                       ChannelPage,
                       1);

    /* Keep compiler happy. */
    LossReason = LossReason;
    PANId = PANId;
}



/**
 * Callback function usr_mlme_beacon_notify_ind
 *
 * @param BSN            Beacon sequence number.
 * @param PANDescriptor  Pointer to PAN descriptor for received beacon.
 * @param PendAddrSpec   Pending address specification in received beacon.
 * @param AddrList       List of addresses of devices the coordinator has pending data.
 * @param sduLength      Length of beacon payload.
 * @param sdu            Pointer to beacon payload.
 */
void usr_mlme_beacon_notify_ind(uint8_t BSN,
                                wpan_pandescriptor_t *PANDescriptor,
                                uint8_t PendAddrSpec,
                                uint8_t *AddrList,
                                uint8_t sduLength,
                                uint8_t *sdu)
{
    if (APP_SCAN_DONE == app_state)
    {
        /* Set proper state of application. */
        app_state = APP_ASSOC_IN_PROGRESS;

        /*
         * Associate to our coordinator.
         * Use: bool wpan_mlme_associate_req(uint8_t LogicalChannel,
         *                                   uint8_t ChannelPage,
         *                                   wpan_addr_spec_t *CoordAddrSpec,
         *                                   uint8_t CapabilityInformation);
         * This request will cause a mlme associate confirm message ->
         * usr_mlme_associate_conf.
         */
        wpan_mlme_associate_req(DEFAULT_CHANNEL,
                                DEFAULT_CHANNEL_PAGE,
                                &coord_addr_spec,
                                WPAN_CAP_ALLOCADDRESS);
    }
    else if (APP_DEVICE_RUNNING == app_state)
    {
        /* This is the standard portion once the node is associated
         * with the application.
         */
        /*
         * Extract the beacon payload from our coordinator and feed it back
         * to the coordiantor via a data frame.
         */
        /* Use: bool wpan_mcps_data_req(uint8_t SrcAddrMode,
         *                              wpan_addr_spec_t *DstAddrSpec,
         *                              uint8_t msduLength,
         *                              uint8_t *msdu,
         *                              uint8_t msduHandle,
         *                              uint8_t TxOptions);
         *
         * This request will cause a mcps data confirm message ->
         * usr_mcps_data_conf
         */
        if (sduLength > PAYLOAD_LEN)
        {
            sduLength = PAYLOAD_LEN;
        }

        /* Copy payload from beacon frame. */
        memcpy(&msdu_payload, sdu, sduLength);

        uint8_t src_addr_mode = WPAN_ADDRMODE_SHORT;
        static uint8_t msdu_handle = 0;

        msdu_handle++;              // Increment handle
        wpan_mcps_data_req(src_addr_mode,
                           &coord_addr_spec,
                           sduLength,
                           &msdu_payload[0],
                           msdu_handle,
                           WPAN_TXOPT_ACK);

#ifdef SIO_HUB
        {
            static uint32_t rx_cnt;
            char sio_array[255];  /* sizeof(uint32_t) + 1 */

            /* Print received payload. */
            rx_cnt++;
            sprintf(sio_array, "Rx beacon payload (%" PRIu32 "): ", rx_cnt);
            printf(sio_array);

            /* Set last element to 0. */
            if (sduLength == PAYLOAD_LEN)
            {
                msdu_payload[PAYLOAD_LEN - 1] = '\0';
            }
            else
            {
                msdu_payload[sduLength] = '\0';
            }
            printf((char *)msdu_payload);
            printf("\n");
        }
#endif  /* SIO_HUB */
    }

    /* Keep compiler happy. */
    BSN = BSN;
    PANDescriptor = PANDescriptor;
    PendAddrSpec = PendAddrSpec;
    AddrList = AddrList;
}



/**
 * @brief Callback function indicating network search
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be if desired).
 */
static void network_search_indication_cb(void *parameter)
{
    pal_led(LED_NWK_SETUP, LED_TOGGLE);

    /* Re-start led timer again. */
    pal_timer_start(TIMER_LED_OFF,
                    500000,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)network_search_indication_cb,
                    NULL);

    parameter = parameter;    /* Keep compiler happy. */
}



/**
 * @brief Callback function switching off the LED
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be if desired).
 */
static void data_exchange_led_off_cb(void *parameter)
{
    pal_led(LED_DATA, LED_OFF);

    parameter = parameter;    /* Keep compiler happy. */
}

/* EOF */
