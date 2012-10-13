/**
 * @file main.c
 *
 * @brief STB Example Secure Sensor - Sensor (Device)
 *
 * This is the source code of a simple secure sensor example using security.
 * It implements the firmware for all sensors (devices) of a network with star topology.
 *
 * $Id: main.c 21652 2010-04-16 10:49:45Z sschneid $
 *
 *  @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "sio_handler.h"
#include "tal.h"
#include "stb.h"
#include "stb_app.h"
#include "ieee_const.h"
#include "mac_api.h"
#include "app_config.h"

/* === TYPES =============================================================== */

/*
 * This type definition of a structure can store the short address and the
 * extended address of a device.
 */
typedef struct associated_device_tag
{
    uint16_t short_addr;
    uint64_t ieee_addr;
}
associated_device_t;

/* === MACROS ============================================================== */

/** Defines the default channel. */
#if (TAL_TYPE == AT86RF212)
    #ifdef CHINESE_BAND
        #define DEFAULT_CHANNEL         (0)
        #define DEFAULT_CHANNEL_PAGE    (5)
    #else
        #define DEFAULT_CHANNEL         (1)
        #define DEFAULT_CHANNEL_PAGE    (0)
    #endif  /* #ifdef CHINESE_BAND */
#else
#define DEFAULT_CHANNEL                 (20)
#define DEFAULT_CHANNEL_PAGE            (0)
#endif  /* #if (TAL_TYPE == AT86RF212) */
/* Defines the PAN ID of the network. */
#define DEFAULT_PAN_ID                  (0xFACE)
/*
 * This is a device which will communicate with a single coordinator.
 * Therefore, the maximum number of devices this code needs to
 * handle is one.
 */
#define MAX_NUMBER_OF_DEVICES           (1)
/* This is the time period in micro seconds for data transmissions. */
#define DATA_TX_PERIOD                  (2000000)
/* Defines the bit mask of channels that should be scanned. */
#if (TAL_TYPE == AT86RF212)
    #if (DEFAULT_CHANNEL == 0)
        #define SCAN_ALL_CHANNELS       (0x00000001)
    #else
        #define SCAN_ALL_CHANNELS       (0x000007FE)
    #endif
#else
#define SCAN_ALL_CHANNELS               (0x07FFF800)
#endif
/* Defines the short scan duration time. */
#define SCAN_DURATION_SHORT             (5)
/* Defines the long scan duration time. */
#define SCAN_DURATION_LONG              (6)

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

/* This structure stores the short and extended address of the coordinator. */
static associated_device_t coord_addr;

/* Variables for payload to be transmitted. */
static uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE];
static uint8_t msduHandle = 0;

/* === PROTOTYPES ========================================================== */

static void app_timer_cb(void *parameter);
static void create_fixed_sec_portions(uint8_t nonce[AES_BLOCKSIZE],
                                      uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE]);
static void data_exchange_led_off_cb(void *parameter);
static void network_search_indication_cb(void *parameter);
static void encrypt_frame(uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE]);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the sensor application
 */
int main(void)
{
    /* Initialize the MAC layer and its underlying layers, like PAL, TAL, BMM. */
    if (wpan_init() != MAC_SUCCESS)
    {
        /*
         * Stay here; we need a valid IEEE address.
         * Check kit documentation how to create an IEEE address
         * and store it to the EEPROM.
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

    printf("\nSensor\n\n");
#endif

    /*
     * Reset the MAC layer to the default values.
     * This request will cause a mlme reset confirm message ->
     * usr_mlme_reset_conf
     */
    wpan_mlme_reset_req(true);

    /* Init Security Toolbox (incl. AES unit). */
    stb_init();

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
#endif
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
                /* Store the coordinator's address. */
                if (coordinator->CoordAddrSpec.AddrMode == WPAN_ADDRMODE_SHORT)
                {
                    ADDR_COPY_DST_SRC_16(coord_addr.short_addr, coordinator->CoordAddrSpec.Addr.short_address);
                }
                else if (coordinator->CoordAddrSpec.AddrMode == WPAN_ADDRMODE_LONG)
                {
                    ADDR_COPY_DST_SRC_64(coord_addr.ieee_addr, coordinator->CoordAddrSpec.Addr.long_address);
                }
                else
                {
                    /* Something went wrong; restart. */
                    wpan_mlme_reset_req(true);
                    return;
                }
#ifdef SIO_HUB
                printf("Found network\n");
#endif
                /*
                 * Associate to our coordinator.
                 * Use: bool wpan_mlme_associate_req(uint8_t LogicalChannel,
                 *                                   uint8_t ChannelPage,
                 *                                   wpan_addr_spec_t *CoordAddrSpec,
                 *                                   uint8_t CapabilityInformation);
                 * This request will cause a mlme associate confirm message ->
                 * usr_mlme_associate_conf.
                 */
                wpan_mlme_associate_req(coordinator->LogicalChannel,
                                        coordinator->ChannelPage,
                                        &(coordinator->CoordAddrSpec),
                                        WPAN_CAP_ALLOCADDRESS);
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
        printf("Conntected to sensor network\n");
#endif
        /* Stop timer used for search indication (same as used for data transmission). */
        pal_timer_stop(TIMER_LED_OFF);
        pal_led(LED_NWK_SETUP, LED_ON);

        /* Start a timer that sends some data to the coordinator every 2 seconds. */
        pal_timer_start(TIMER_TX_DATA,
                        DATA_TX_PERIOD,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR)app_timer_cb,
                        NULL);
    }
    else
    {
        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    AssocShortAddress = AssocShortAddress;
}



/**
 * @brief Callback function for the application timer
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be if desired).
 */
static void app_timer_cb(void *parameter)
{
    /*
     * Send some data and restart timer.
     * Use: bool wpan_mcps_data_req(uint8_t SrcAddrMode,
     *                              wpan_addr_spec_t *DstAddrSpec,
     *                              uint8_t msduLength,
     *                              uint8_t *msdu,
     *                              uint8_t msduHandle,
     *                              uint8_t TxOptions);
     *
     * This request will cause a mcps data confirm message ->
     * usr_mcps_data_conf
     */

    uint8_t src_addr_mode = WPAN_ADDRMODE_SHORT;
    wpan_addr_spec_t dst_addr;

    dst_addr.AddrMode = WPAN_ADDRMODE_SHORT;
    dst_addr.PANId = DEFAULT_PAN_ID;
    ADDR_COPY_DST_SRC_16(dst_addr.Addr.short_address, coord_addr.short_addr);

    /* Generate random ZigBee payload (e.g. sensor measurement data). */
    for (uint8_t i = 0; i < PLD_LEN; i++)
    {
        msdu[AUX_HDR_LEN + i] = (uint8_t)rand();
    }

    /* Build the encrypted actual msdu. */
    encrypt_frame(msdu);

    ++msduHandle;

    wpan_mcps_data_req(src_addr_mode,
                       &dst_addr,
                       AUX_HDR_LEN + PLD_LEN + MIC_LEN,
                       msdu,
                       msduHandle,
                       WPAN_TXOPT_ACK);

    pal_timer_start(TIMER_TX_DATA,
                    DATA_TX_PERIOD,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)app_timer_cb,
                    NULL);

    parameter = parameter;  /* Keep compiler happy. */
}



/**
 * @brief Creates secured frame
 *
 * This function creates a secure frame from plaintext msdu.
 *
 * @param msdu Pointer to msdo to be created
 *
 */
static void encrypt_frame(uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE])
{
    static uint8_t *keyp = stb_app_key;
    /*
     * The nonce: Initialization Vector (IV) as used in cryptography;
     * the ZigBee nonce (13buytes long) are the bytes 2...14 of this nonce.
     */
    static uint8_t nonce[AES_BLOCKSIZE];
    static uint32_t framecounter = 0;

    uint8_t i;
#ifdef SIO_HUB
    char sio_array[255];
#endif

    /*
     * Some portions of the nounce or auxiliary frame header of the frame are
     * fixed. These parts are only calculated for the first frame transmission.
     */
    if (framecounter == 0)
    {
        create_fixed_sec_portions(nonce, msdu);
    }

    /* Insert frame counter into nonce and auxiliary security header. */
    for (i = FRM_COUNTER_LEN; i--; /* */)
    {
        nonce[sizeof(tal_pib_IeeeAddress) + FRM_COUNTER_LEN - i] =
            (framecounter >> (i << 3)) & 0xFF; // MSB first
        msdu[FRM_COUNTER_LEN - i] = (framecounter >> (i << 3)) & 0xFF; // MSB first
    }

    tal_trx_wakeup();

    if (keyp == NULL)
    {
        sal_aes_restart();
    }

#ifdef SIO_HUB
    sprintf(sio_array, "Tx frame %" PRIu32 ": ", framecounter);
    printf(sio_array);
    for (i = 0; i < PLD_LEN; i++)
    {
        sprintf(sio_array, "%" PRIu8 " ", msdu[AUX_HDR_LEN + i]);
        printf(sio_array);
    }
    sprintf(sio_array, "\n");
    printf(sio_array);
#endif

    /* Call Security Toolbox to encrypt frame. */
    stb_ccm_secure(msdu,
                   nonce,
                   keyp,
                   AUX_HDR_LEN,
                   PLD_LEN,
                   SEC_LEVEL,
                   AES_DIR_ENCRYPT);

    /*
     * The actual key shall be used from now.
     * So key is set to NULL. Next time Security Toolbox is called.
     * the last know key will be used, until a new (not NULL) key is
     * handed over.
     */
    keyp = NULL;

    ++framecounter;
}



/**
 * @brief Calculate fixed portions of nonce and secured frame
 *
 * Compute fixed fields of nonce - with offset 1,
 * since the nonce is used as an AES block here,
 * not only the core 13 bytes mentioned in ZigBee standard.
 * Also create fixed portions of auxiliary frame header.
 *
 * @param nonce
 *
 */
static void create_fixed_sec_portions(uint8_t nonce[AES_BLOCKSIZE],
                                      uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE])
{
    uint8_t i;

    /* Initialize nonce at first call. */
    memset(nonce, 0, AES_BLOCKSIZE);

    /* Copy source address (IEEE address) to nonce. */
    for (i = sizeof(tal_pib_IeeeAddress); i--; /* */)
    {
        /* MSB first. */
        nonce[sizeof(tal_pib_IeeeAddress) - i] = (tal_pib_IeeeAddress >> (i << 3)) & 0xFF;
    }

    /*
     * Security control Field:
     *
     * Bit 0-2: Security Level = 6 (MIC length 8 bytes, encryption)
     * Bit 3-4: Key identifier - A Network key
     * Bit 5: Extended Nonce enabled
     */
    nonce[1 + (NONCE_POS_SEC_CTRL - 1)] =
                                SEC_LEVEL |
                                (KEY_ID_NWK_KEY << SEC_CTRL_POS_KEY_ID) |
                                (1 << (SEC_CTRL_POS_EXT_NONCE));

    msdu[MSDU_POS_SEC_CTRL - 1] = nonce[1 + (NONCE_POS_SEC_CTRL - 1)];

    /* Copy IEEE source address to msdu. */
    memcpy(msdu + (MSDU_POS_SRC_ADDR - 1), nonce + 1, sizeof(tal_pib_IeeeAddress));

    /* The first (and the only) network key. */
    msdu[MSDU_POS_KEY_SEQ_NO - 1] = NWK_KEY_NO;
}



/**
 * @brief Callback function usr_mcps_data_conf
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
#endif  /* ENABLE_TSTAMP */
{
    if (status == MAC_SUCCESS)
    {
        /*
         * Sensor data has been transmitted successfully.
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
#endif  /* ENABLE_TSTAMP */
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
