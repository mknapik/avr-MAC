/**
 * @file main.c
 *
 * @brief MAC Example App 4 Beacon Broadcast Data - Device
 *
 * This is the source code of a simple MAC example. It implements the
 * firmware for all devices of a beacon_enabled network with star topology.
 *
 * The coordinator starts a beaconing network and transmits broadcast data frames
 * periodically.
 * The devices receive these broadcast data frames and increase a counter.
 *
 * $Id: main.c 22225 2010-06-15 12:21:48Z sschneid $
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
#include <inttypes.h>
#include "pal.h"
#include "tal.h"
#include "sio_handler.h"
#include "mac_api.h"
#include "app_config.h"
#include "ieee_const.h"

/* === TYPES =============================================================== */


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
#define SCAN_DURATION_SHORT             (5)
/** Defines the long scan duration time. */
#define SCAN_DURATION_LONG              (6)

/*
 * This is the timeout in ms once the node tries to synchronize with the
 * coordinator. This timeout is set so that the node should be able to receive
 * at least a beacon frame from its parent.
 * In case the Beacon Order is increased to a high value, this timeout might
 * need to be increased as well.
 */
#define TIMER_SYNC_BEFORE_ASSOC_MS      (3000)

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
/** This variable counts the number of received data frames. */
static uint32_t rx_cnt;

/* === PROTOTYPES ========================================================== */

static void network_search_indication_cb(void *parameter);
static void init_assoc_cb(void *parameter);
static void rx_data_led_off_cb(void *parameter);

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

    /* Initialize the serial interface used for communication with terminal program. */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        /* Something went wrong during initialization. */
        pal_alert();
    }

#if ((!defined __ICCAVR__) && (!defined __ICCARM__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif

    printf("\nApp_4_Beacon_Broadcast_Data\n\n");
    printf("\nDevice\n\n");

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
        printf("Searching network\n");

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
        pal_timer_start(APP_TIMER,
                        500000,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR)network_search_indication_cb,
                        NULL);
    }
    else
    {
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

                printf("Found network\n");

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

        /* Stop timer used for search indication (same as used for data reception). */
        pal_timer_stop(APP_TIMER);

        /* Start application timer again after sync to initiate association later. */
        pal_timer_start(APP_TIMER,
                        ((uint32_t)TIMER_SYNC_BEFORE_ASSOC_MS * 1000),
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR)init_assoc_cb,
                        NULL);
    }
    else
    {
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
        printf("Conntected to beacon-enabled network\n");

        pal_led(LED_NWK_SETUP, LED_ON);
    }
    else
    {
        pal_led(LED_NWK_SETUP, LED_OFF);

        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    AssocShortAddress = AssocShortAddress;
}



/**
 * @brief Callback function usr_mcps_data_ind
 *
 * @param SrcAddrSpec      Pointer to source address specification
 * @param DstAddrSpec      Pointer to destination address specification
 * @param msduLength       Number of octets contained in MSDU
 * @param msdu             Pointer to MSDU
 * @param mpduLinkQuality  LQI measured during reception of the MPDU
 * @param DSN              DSN of the received data frame.
 * @param Timestamp        The time, in symbols, at which the data were received.
 *                         (only if timestamping is enabled).
 */
void usr_mcps_data_ind(wpan_addr_spec_t *SrcAddrSpec,
                       wpan_addr_spec_t *DstAddrSpec,
                       uint8_t msduLength,
                       uint8_t *msdu,
                       uint8_t mpduLinkQuality,
#ifdef ENABLE_TSTAMP
                       uint8_t DSN,
                       uint32_t Timestamp)
#else
                       uint8_t DSN)
#endif  /* ENABLE_TSTAMP */
{
    char sio_array[255];
    rx_cnt++;

    sprintf(sio_array, "Frame received: %" PRIu32 "\n", rx_cnt);
    printf(sio_array);

    /*
     * Dummy data has been received successfully.
     * Application code could be added here ...
     */
    pal_led(LED_DATA, LED_ON);

     /* Start a timer switching off the LED. */
    pal_timer_start(APP_TIMER,
                    500000,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)rx_data_led_off_cb,
                    NULL);

    /* Keep compiler happy. */
    SrcAddrSpec = SrcAddrSpec;
    DstAddrSpec = DstAddrSpec;
    msduLength = msduLength;
    msdu = msdu;
    mpduLinkQuality = mpduLinkQuality;
    DSN = DSN;
#ifdef ENABLE_TSTAMP
    Timestamp = Timestamp;
#endif  /* ENABLE_TSTAMP */
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
    /* Stub for beacon notify indication. Add code here if required. */

    /* Keep compiler happy. */
    sduLength = sduLength;
    sdu = sdu;
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

    /* Re-start timer again. */
    pal_timer_start(APP_TIMER,
                    500000,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)network_search_indication_cb,
                    NULL);

    parameter = parameter;    /* Keep compiler happy. */
}



/**
 * @brief Callback function while being synced before association.
 *        Once this function is called it is assumed that this node
 *        has received at least one beacon frame from its coordinator,
 *        and it now able to associate properly.
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be if desired).
 */
static void init_assoc_cb(void *parameter)
{
    pal_led(LED_NWK_SETUP, LED_OFF);

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

    parameter = parameter;    /* Keep compiler happy. */
}



/**
 * @brief Callback function for switching off the LED after data reception
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be if desired).
 */
static void rx_data_led_off_cb(void *parameter)
{
    pal_led(LED_DATA, LED_OFF);

    parameter = parameter;    /* Keep compiler happy. */
}

/* EOF */
