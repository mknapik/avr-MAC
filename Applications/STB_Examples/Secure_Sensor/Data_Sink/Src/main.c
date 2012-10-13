/**
 * @file main.c
 *
 * @brief STB Example Secure Sensor - Data Sink (Coordinator)
 *
 * This is the source code of a simple secure sensor example using security.
 * It implements the firmware for the data sink (coordinator) of a network with
 * star topology.
 *
 * $Id: main.c 22854 2010-08-11 08:19:47Z sschneid $
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

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "pal.h"
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
#define DEFAULT_PAN_ID                  (0xFACE)
/** Defines the short address of the coordinator. */
#define COORD_SHORT_ADDR                (0x0000)
/** Defines the maximum number of devices this coordinator will handle. */
#define MAX_NUMBER_OF_DEVICES           (2)
/** Defines the bit mask of channels that should be scanned. */
#if (TAL_TYPE == AT86RF212)
    #if (DEFAULT_CHANNEL == 0)
        #define SCAN_ALL_CHANNELS       (0x00000001)
    #else
        #define SCAN_ALL_CHANNELS       (0x000007FE)
    #endif
#else
#define SCAN_ALL_CHANNELS               (0x07FFF800)
#endif
/** Defines the scan duration time. */
#define SCAN_DURATION_COORDINATOR       (1)

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

/* This array stores all device related information. */
static associated_device_t device_list[MAX_NUMBER_OF_DEVICES];

/* === PROTOTYPES ========================================================== */

static void decrypt_frame(uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE],
                          uint8_t msduLength);
static bool assign_new_short_addr(uint64_t addr64, uint16_t *addr16);
static void indicate_failed_sec(led_id_t led_no);
static void led_off_cb(void *parameter);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the data sink application
 *
 * This function initializes the MAC, initiates a MLME reset request
 * (@ref wpan_mlme_reset_req()), and implements a the main loop.
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
    pal_led(LED_NWK_SETUP, LED_OFF);    // indicating network is started
    pal_led(LED_DATA, LED_OFF);         // indicating data reception

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

    printf("\nData Sink\n\n");

    /*
     * Reset the MAC layer to the default values
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
        /*
         * Set the short address of this node.
         * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
         *                             void *PIBAttributeValue);
         *
         * This request leads to a set confirm message -> usr_mlme_set_conf
         */
        uint8_t short_addr[2];

        short_addr[0] = (uint8_t)COORD_SHORT_ADDR;          // low byte
        short_addr[1] = (uint8_t)(COORD_SHORT_ADDR >> 8);   // high byte
        wpan_mlme_set_req(macShortAddress, short_addr);
    }
    else
    {
        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }
}



/**
 * @brief Callback function usr_mlme_set_conf
 *
 * @param status        Result of requested PIB attribute set operation
 * @param PIBAttribute  Updated PIB attribute
 */
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
    if ((status == MAC_SUCCESS) && (PIBAttribute == macShortAddress))
    {
        /*
         * Allow other devices to associate to this coordinator.
         * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
         *                             void *PIBAttributeValue);
         *
         * This request leads to a set confirm message -> usr_mlme_set_conf
         */
         uint8_t association_permit = true;

         wpan_mlme_set_req(macAssociationPermit, &association_permit);
    }
    else if ((status == MAC_SUCCESS) && (PIBAttribute == macAssociationPermit))
    {
        /*
         * Set RX on when idle to enable the receiver as default.
         * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
         *                             void *PIBAttributeValue);
         *
         * This request leads to a set confirm message -> usr_mlme_set_conf
         */
         bool rx_on_when_idle = true;

         wpan_mlme_set_req(macRxOnWhenIdle, &rx_on_when_idle);
    }
    else if ((status == MAC_SUCCESS) && (PIBAttribute == macRxOnWhenIdle))
    {
        /*
         * Initiate an active scan over all channels to determine
         * which channel to use.
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
                           SCAN_DURATION_COORDINATOR,
                           DEFAULT_CHANNEL_PAGE);
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
    /*
     * We are not interested in the actual scan result,
     * because we start our network on the pre-defined channel anyway.
     * Start a nonbeacon-enabled network
     * Use: bool wpan_mlme_start_req(uint16_t PANId,
     *                               uint8_t LogicalChannel,
     *                               uint8_t ChannelPage,
     *                               uint8_t BeaconOrder,
     *                               uint8_t SuperframeOrder,
     *                               bool PANCoordinator,
     *                               bool BatteryLifeExtension,
     *                               bool CoordRealignment)
     *
     * This request leads to a start confirm message -> usr_mlme_start_conf
     */
     wpan_mlme_start_req(DEFAULT_PAN_ID,
                         DEFAULT_CHANNEL,
                         DEFAULT_CHANNEL_PAGE,
                         15, 15,
                         true, false, false);

    /* Keep compiler happy. */
    status = status;
    ScanType = ScanType;
    ChannelPage = ChannelPage;
    UnscannedChannels = UnscannedChannels;
    ResultListSize = ResultListSize;
    ResultList = ResultList;
}



/**
 * @brief Callback function usr_mlme_start_conf
 *
 * @param status        Result of requested start operation
 */
void usr_mlme_start_conf(uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
        printf("Started sensor network\n");

        /*
         * Network is established.
         * Waiting for association indication from a device.
         * -> usr_mlme_associate_ind
         */
        pal_led(LED_NWK_SETUP, LED_ON);
    }
    else
    {
        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }
}



/**
 * @brief Callback function usr_mlme_associate_ind
 *
 * @param DeviceAddress         Extended address of device requesting association
 * @param CapabilityInformation Capabilities of device requesting association
 */
void usr_mlme_associate_ind(uint64_t DeviceAddress,
                            uint8_t CapabilityInformation)
{
    /*
     * Any device is allowed to join the network.
     * Use: bool wpan_mlme_associate_resp(uint64_t DeviceAddress,
     *                                    uint16_t AssocShortAddress,
     *                                    uint8_t status);
     *
     * This response leads to comm status indication -> usr_mlme_comm_status_ind
     * Get the next available short address for this device.
     */
    uint16_t associate_short_addr = macShortAddress_def;

    if (assign_new_short_addr(DeviceAddress, &associate_short_addr) == true)
    {
        wpan_mlme_associate_resp(DeviceAddress,
                                 associate_short_addr,
                                 ASSOCIATION_SUCCESSFUL);
    }
    else
    {
        wpan_mlme_associate_resp(DeviceAddress, associate_short_addr,
                                         PAN_AT_CAPACITY);
    }

    /* Keep compiler happy. */
    CapabilityInformation = CapabilityInformation;
}



/**
 * @brief Callback function usr_mlme_comm_status_ind
 *
 * @param SrcAddrSpec      Pointer to source address specification
 * @param DstAddrSpec      Pointer to destination address specification
 * @param status           Result for related response operation
 */
void usr_mlme_comm_status_ind(wpan_addr_spec_t *SrcAddrSpec,
                              wpan_addr_spec_t *DstAddrSpec,
                              uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
        /*
         * Now the association of the device has been successful and its
         * information, like address, could  be stored.
         * But for the sake of simple handling it has been done
         * during assignment of the short address within the function
         * assign_new_short_addr()
         */
    }

    /* Keep compiler happy. */
    SrcAddrSpec = SrcAddrSpec;
    DstAddrSpec = DstAddrSpec;
}



/**
 * @brief Callback function usr_mcps_data_ind
 *
 * @param SrcAddrSpec      Pointer to source address specification
 * @param DstAddrSpec      Pointer to destination address specification
 * @param msduLength       Number of octets contained in MSDU
 * @param msdu             Pointer to MSDU
 * @param mpduLinkQuality  LQI measured during reception of the MPDU
 * @param DSN              Data Sequence Number of MSDU
 * @param Timestamp        Timestamp when MSDU was received,
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
    /* Evaluate and decrypt received frame. */
    decrypt_frame(msdu, msduLength);

    /* Sensor data has been received successfully. */
    pal_led(LED_DATA, LED_ON);

     /* Start a timer switching off the LED. */
    pal_timer_start(TIMER_LED_OFF,
                    500000,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)led_off_cb,
                    NULL);

    /* Keep compiler happy. */
    SrcAddrSpec = SrcAddrSpec;
    DstAddrSpec = DstAddrSpec;
    mpduLinkQuality = mpduLinkQuality;
    DSN = DSN;
#ifdef ENABLE_TSTAMP
    Timestamp = Timestamp;
#endif  /* ENABLE_TSTAMP */
}



/**
 * @brief Decrpyts secured frame
 *
 * This function evaluates and decrypts a secured frame.
 *
 * @param msdu Pointer to received secured msdu
 *
 */
static void decrypt_frame(uint8_t msdu[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE],
                          uint8_t msduLength)
{
    static uint8_t *keyp = stb_app_key;
    static uint32_t last_framecounter = 0;
    static uint8_t nonce[AES_BLOCKSIZE];
    uint32_t rcvd_framecounter;
    stb_ccm_t decrypt_status;

    uint8_t i;
    uint8_t sec_ctrl = SEC_LEVEL |
                       (KEY_ID_NWK_KEY << SEC_CTRL_POS_KEY_ID) |
                       (1 << (SEC_CTRL_POS_EXT_NONCE));
    char sio_array[255];


    if ((msduLength != AUX_HDR_LEN + PLD_LEN + MIC_LEN) ||
        (msdu[0] != sec_ctrl)
       )
    {
        indicate_failed_sec(LED_START);
    }

    /*
     * Init fixed part of nonce during first call - with offset 1
     * since the nonce is used as an AES block here,
     * not only the core 13 bytes mentioned in ZigBee standard.
     */
    if (keyp != NULL)
    {
        /*
         * Copy received IEEE source address from msdu to fixed part of nounce.
         */
        memcpy(nonce + 1, msdu + (MSDU_POS_SRC_ADDR - 1), sizeof(tal_pib_IeeeAddress));
        nonce[1 + (NONCE_POS_SEC_CTRL - 1)] = sec_ctrl;
    }

    /* Read received framecounter. */
    for (rcvd_framecounter = 0, i = FRM_COUNTER_LEN; i--; /*  */)
    {
        rcvd_framecounter |= (msdu[FRM_COUNTER_LEN - i] << (i << 3));
    }

    if (rcvd_framecounter < last_framecounter)
    {
        sprintf(sio_array,
                "Frame %" PRIu32 " received, expected at least %" PRIu32 "\n",
                 rcvd_framecounter, last_framecounter);

        sprintf(sio_array, "Device %" PRIu8 " associated\n", i + 1);
        printf(sio_array);

        indicate_failed_sec(LED_NWK_SETUP);
    }

    last_framecounter = rcvd_framecounter;

    /* Copy received framecounter to nonce. */
    memcpy(nonce + 1 + sizeof(tal_pib_IeeeAddress), msdu + 1, FRM_COUNTER_LEN);

    /* Check MIC. */
    decrypt_status = stb_ccm_secure(msdu,
                                    nonce,
                                    keyp,
                                    AUX_HDR_LEN,
                                    PLD_LEN,
                                    SEC_LEVEL,
                                    AES_DIR_DECRYPT);

    switch(decrypt_status)
    {
        case STB_CCM_OK:
            sprintf(sio_array, "Rx Frame %" PRIu32 ": ", rcvd_framecounter);
            printf(sio_array);
            /* Print received payload. */
            for (i = 0; i < PLD_LEN; i++)
            {
                sprintf(sio_array, "%" PRIu8 " ", msdu[AUX_HDR_LEN + i]);
                printf(sio_array);
            }
            sprintf(sio_array, " (MIC ok)\n");
            printf(sio_array);
            break;

        case STB_CCM_ILLPARM:
        case STB_CCM_KEYMISS:
            /* should never happen */
            indicate_failed_sec(LED_DATA);
            break;

        case STB_CCM_MICERR:
            sprintf(sio_array, "Frame %" PRIu32 ": MIC wrong\n", rcvd_framecounter);
            printf(sio_array);
            indicate_failed_sec(LED_DATA);
            break;
    }

    /*
     * The actual key shall be used from now.
     * So key is set to NULL. Next time Security Toolbox is called.
     * the last know key will be used, until a new (not NULL) key is
     * handed over.
     */
    keyp = NULL;
}



/**
 * @brief Indicate security error by endless LED blinking.
 *
 * @param led_no LED to be used
 */

static void indicate_failed_sec(led_id_t led_no)
{
    uint16_t cnt;

    pal_led(led_no, LED_ON);

    for (cnt = 1000; cnt--; /* */)
    {
        pal_timer_delay(100);
    }

    pal_led(led_no, LED_OFF);

    for (cnt = 1000; cnt--; /* */)
    {
        pal_timer_delay(100);
    }
}


/**
 * @brief Callback function switching off the LED
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be used
 *                  to indicated LED to be switched off)
 */
static void led_off_cb(void *parameter)
{
    pal_led(LED_DATA, LED_OFF);

    parameter = parameter;  /* Keep compiler happy. */
}



/**
 * @brief Application specific function to assign a short address
 */
static bool assign_new_short_addr(uint64_t addr64, uint16_t *addr16)
{
    uint8_t i;

    /* Check if device has been associated before. */
    for (i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
    {
        if (device_list[i].short_addr == 0x0000)
        {
            /* If the short address is 0x0000, it has not been used before. */
            continue;
        }
        if (device_list[i].ieee_addr == addr64)
        {
            /* Assign the previously assigned short address again. */
            *addr16 = device_list[i].short_addr;
            return true;
        }
    }

    for (i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
    {
        if (device_list[i].short_addr == 0x0000)
        {
            *addr16 = i + 0x0001;
            device_list[i].short_addr = i + 0x0001; /* Get next short address. */
            device_list[i].ieee_addr = addr64;      /* Store extended address. */
            return true;
        }
    }

    /* If we are here, no short address could be assigned. */
    return false;
}

/* EOF */
