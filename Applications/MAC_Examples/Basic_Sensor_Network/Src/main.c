/**
 * @file main.c
 *
 * @brief  MAC Example - Basic Sensor Network
 *
 * This is the source code of a simple MAC example. It implements the
 * firmware for all nodes of a network with star topology based on a Tiny MAC
 * with only minimal 802.15.4 primitive support.
 *
 * The actual supported primtives are defined in the corresponding file
 * "mac_user_build_config.h" in directory "Inc" of this application.
 *
 * In order to actually use this header file, the build switch "MAC_USER_BUILD_CONFIG"
 * needs to be set in the corresponding Makefiles of this project.
 *
 * $Id: main.c 22913 2010-08-13 09:48:49Z sschneid $
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
#include <stdio.h>
#include "pal.h"
#include "tal.h"
#include "tfa.h"
#include "mac_api.h"
#include "app_config.h"
#include "ieee_const.h"
#include "sio_handler.h"

/* === TYPES =============================================================== */

typedef enum node_type_tag
{
    LEAF,
    ROOT_ROUTER,
    ROUTER,
    ROOT
} node_type_t;

typedef enum node_state_tag
{
    SEARCHING,
    CONNECTED,
    BROADCASTING
} node_state_t;

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
#endif  /* #if (TAL_TYPE == AT86RF212) */

/** Defines the PAN ID of the network. */
#define DEFAULT_PAN_ID                  (0xBEEF)

/** Defines the discovery duration during start-up */
#define DISCOVERY_DURATION_MS           (10000)

/** Defines the tx interval */
#define TX_INTERVAL_S                   (10)

/** Defines the receiver on duration */
#define RX_ON_DURATION_MS               (10)

/** Defines the delay (in us) between each broadcast message */
#define BROADCAST_DELAY                 (50000)

#if (NO_OF_LEDS >= 3)
#define LED_ROOT                        (LED_0)
#define LED_DATA_TX                     (LED_1)
#define LED_DATA_RX                     (LED_2)
#elif (NO_OF_LEDS == 2)
#define LED_ROOT                        (LED_0)
#define LED_DATA_TX                     (LED_0)
#define LED_DATA_RX                     (LED_1)
#else
#define LED_ROOT                        (LED_0)
#define LED_DATA_TX                     (LED_0)
#define LED_DATA_RX                     (LED_0)
#endif

/* === GLOBALS ============================================================= */

static node_type_t node_type;
static uint64_t parent_addr;
static node_state_t node_state;
static uint8_t own_addr[8];

/* === PROTOTYPES ========================================================== */

static void app_peripheral_init(void);
static bool button_pressed(void);
static void scan_timer_expired_cb(void *parameter);
static void data_tx_start_cb(void *parameter);
static void send_sensor_data(void);
#ifdef TEMPERATURE_MEASUREMENT
static void adc_init(void);
#endif
static uint16_t get_battery_value(void);
static uint16_t get_temperature_value(void);
static void app_task(void);
static void send_broadcast(void);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the device application
 */
int main(void)
{
    /* Initialize the MAC layer and its underlying layers, like PAL, TAL, BMM */
    if (wpan_init() != MAC_SUCCESS)
    {
        /*
         * Stay here; we need a valid IEEE address.
         * Check kit documentation how to create an IEEE address
         * and to store it into the EEPROM.
         */
        pal_alert();
    }

    /* Initialize LEDs */
    pal_led_init();
    pal_led(LED_ROOT, LED_OFF);     // indicating device is root
    pal_led(LED_DATA_TX, LED_OFF);  // indicating active period / data transmit
    pal_led(LED_DATA_RX, LED_OFF);  // indicating data reception

    /*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();

#ifdef SIO_HUB
    /* Initialize the serial interface used for communication with terminal program */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        // something went wrong during initialization
        pal_alert();
    }

#if ((!defined __ICCAVR__) && (!defined __ICCARM__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif
#endif  /* SIO_HUB */

    app_peripheral_init();

    if (button_pressed())
    {
        node_type = ROOT_ROUTER;
    }
    else
    {
        node_type = LEAF;
    }

    /*
     * Reset the MAC layer to the default values
     * This request will cause a mlme reset confirm ->
     * usr_mlme_reset_conf
     */
    wpan_mlme_reset_req(true);

    /* Main loop */
    while (1)
    {
        wpan_task();

        app_task();
    }
}


/**
 * @brief Application task
 */
static void app_task(void)
{
    switch (node_state)
    {
        case BROADCASTING:
            break;

        case CONNECTED:
            if (node_type != LEAF)
            {
                if(button_pressed())
                {
                    node_state = BROADCASTING;
                    send_broadcast();
                }
            }
            break;

        default:
            break;
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
        /* Get IEEE address */
        wpan_mlme_get_req(macIeeeAddress);
    }
    else
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
    }
}


/**
 * @brief Callback function usr_mlme_get_conf
 *
 * @param status            Result of requested PIB attribute get operation
 * @param PIBAttribute      PIB attribute
 * @param PIBAttributeValue PIB attribute value
 */
void usr_mlme_get_conf(uint8_t status,
                       uint8_t PIBAttribute,
                       void *PIBAttributeValue)
{
    if (status == MAC_SUCCESS)
    {
        if (PIBAttribute == macIeeeAddress)
        {
            /* Store own IEEE address to variable. */
            memcpy(own_addr, PIBAttributeValue, 8);

            /* Set default channel */
            uint8_t ch = DEFAULT_CHANNEL;
            wpan_mlme_set_req(phyCurrentChannel, &ch);
        }
    }
    else
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
    }
}


/**
 * @brief Callback function usr_mlme_rx_enable_conf
 *
 * @param status Result of the rx enable procedure
 */
void usr_mlme_rx_enable_conf(uint8_t status)
{
    /* Keep compiler happy. */
    status = status;
}


/**
 * @brief Callback function usr_mlme_set_conf
 *
 * @param status        Result of requested PIB attribute set operation
 * @param PIBAttribute  Updated PIB attribute
 */
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
    if (status != MAC_SUCCESS)
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
        return;
    }

    if (node_state != CONNECTED)
    {
        if (PIBAttribute == phyCurrentChannel)
        {
            /* Set default channel */
            uint16_t pan_id = DEFAULT_PAN_ID;
            wpan_mlme_set_req(macPANId, &pan_id);
        }
        else if (PIBAttribute == macPANId)
        {
            uint8_t pib_value;

            if (node_type == LEAF)
            {
                pib_value = false;
            }
            else
            {
                pib_value = true;
            }
            node_state = SEARCHING;
            /* Switch receiver on to receive broadcast message */
            wpan_mlme_set_req(macRxOnWhenIdle, &pib_value);
            wpan_mlme_rx_enable_req(false, 0, 0xFFFFFF);
            pal_timer_start(APP_TIMER,
                            ((uint32_t)DISCOVERY_DURATION_MS * 1000),
                            TIMEOUT_RELATIVE,
                            (FUNC_PTR)scan_timer_expired_cb,
                            NULL);
        }

        return;
    }

    if (node_type == LEAF)
    {
        if (PIBAttribute == macRxOnWhenIdle)
        {
            wpan_mlme_rx_enable_req(false, 0, 0);
            pal_timer_start(APP_TIMER,
                            (TX_INTERVAL_S * 1000000),
                            TIMEOUT_RELATIVE,
                            (FUNC_PTR)data_tx_start_cb,
                            NULL);
        }
    }
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
    if (node_state == SEARCHING)
    {
        /* Check if this is an expected broadcast frame */
        if ((SrcAddrSpec->AddrMode == FCF_LONG_ADDR) &&
            (SrcAddrSpec->PANId == DEFAULT_PAN_ID) &&
            (DstAddrSpec->AddrMode == FCF_SHORT_ADDR) &&
            (DstAddrSpec->PANId == DEFAULT_PAN_ID))
        {
            /* Stop scan timer */
            pal_timer_stop(APP_TIMER);

            /* Store parent's address */
            ADDR_COPY_DST_SRC_64(parent_addr, SrcAddrSpec->Addr.long_address);
            node_state = CONNECTED;

            if (node_type == LEAF)
            {
                uint8_t pib_value = false;

                pal_led(LED_DATA_TX, LED_ON);
                wpan_mlme_set_req(macRxOnWhenIdle, &pib_value);
            }
            else    /* ROOT or ROUTER */
            {
                /* Since a broadcast frame has been received, the device operated as router. */
                node_type = ROUTER;
                /* The receiver is kept on. */
            }
        }
    }
    else    /* Sensor data reception */
    {
        if (node_type == LEAF)
        {
        }
        else    /* ROOT or ROUTER */
        {
            pal_led(LED_DATA_RX, LED_TOGGLE);

            /* Store child's node address */

            if (node_type == ROUTER)
            {
                uint8_t src_addr_mode;
                wpan_addr_spec_t dst_addr;

                src_addr_mode = WPAN_ADDRMODE_LONG;
                dst_addr.AddrMode = WPAN_ADDRMODE_LONG;
                dst_addr.PANId = DEFAULT_PAN_ID;
                ADDR_COPY_DST_SRC_64(dst_addr.Addr.long_address, parent_addr);

                /* Forward received data to parent */
                wpan_mcps_data_req(src_addr_mode,
                       &dst_addr,
                       msduLength /* payload length */,
                       msdu,
                       0 /* msduHandle */,
                       WPAN_TXOPT_ACK);

            }
            else    /* ROOT */
            {
#ifdef SIO_HUB
                /* ROOT node prints received data to serial interfcae. */
                char ascii[5];
                uint8_t i;
                uint16_t vcc;
                if (msduLength > 9)
                {
                    printf("from 0x");
                    for (i = 0; i < 8; i++)
                    {
                        sprintf(ascii, "%.2X ", msdu[7-i]);
                        printf(ascii);
                    }
                    vcc = (uint16_t)msdu[8] | (uint16_t)msdu[9] << 8;
                    printf(": Vcc = %u mV\r\n", vcc);
                }
#endif  /* SIO_HUB */
            }
        }
    }

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
 * @brief Sends sensor data
 */
static void send_sensor_data(void)
{
    uint8_t src_addr_mode;
    wpan_addr_spec_t dst_addr;
    uint8_t payload[12];
    uint16_t bat_val;
    uint16_t temperature_val;
    uint8_t i;

    src_addr_mode = WPAN_ADDRMODE_LONG;
    dst_addr.AddrMode = WPAN_ADDRMODE_LONG;
    dst_addr.PANId = DEFAULT_PAN_ID;
    ADDR_COPY_DST_SRC_64(dst_addr.Addr.long_address, parent_addr);

    /* Create payload */
    for (i = 0; i < 8; i++)
    {
        payload[i] = own_addr[i];
    }
    bat_val = get_battery_value();
    payload[8] = (uint8_t)bat_val;   // LSB first
    payload[9] = (uint8_t)(bat_val >> 8);
    temperature_val = get_temperature_value();
    payload[10] = (uint8_t)temperature_val;   // LSB first
    payload[11] = (uint8_t)(temperature_val >> 8);

    wpan_mcps_data_req(src_addr_mode,
                       &dst_addr,
                       12 /* payload length */,
                       payload,
                       0 /* msduHandle */,
                       WPAN_TXOPT_ACK);
    /*
     * This request will cause a mcps data confirm message ->
     * usr_mcps_data_conf
     */
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
#endif  /* ENABLE_TSTAMP */
{
    if (node_state == BROADCASTING)
    {
        node_state = CONNECTED;
        /* Stay here avoiding that broadcast messages are sent too frequently. */
        pal_timer_delay(BROADCAST_DELAY);
        return;
    }

    if (node_type == LEAF)
    {
       pal_timer_start(APP_TIMER,
                       (TX_INTERVAL_S * 1000000),
                       TIMEOUT_RELATIVE,
                       (FUNC_PTR)data_tx_start_cb,
                       NULL);

       pal_led(LED_DATA_TX, LED_OFF);

       /* @TODO Set MCU to sleep or reduced MCU clock speed. */
    }

    /* Keep compiler happy. */
    msduHandle = msduHandle;
    status = status;
#ifdef ENABLE_TSTAMP
    Timestamp = Timestamp;
#endif  /* ENABLE_TSTAMP */
}


/**
 * Callback function that is called when the scan timer expires
 *
 * @param parameter  Pointer to parameter - unused
 */
static void scan_timer_expired_cb(void *parameter)
{
    uint8_t i;

    /* No parent has been found. */
    if (node_type == ROOT_ROUTER)
    {
        node_type = ROOT;
        node_state = CONNECTED;

        pal_led(LED_ROOT, LED_ON);
        pal_led(LED_DATA_TX, LED_ON);
        pal_led(LED_DATA_RX, LED_OFF);
    }
    else    /* LEAF */
    {
        /* No parent has been found. Indicated it using LEDs. */
        while (1)
        {
            pal_led(LED_ROOT, LED_ON);
            for (i = 0; i < 2; i++)
            {
                pal_timer_delay(50000);
            }
            pal_led(LED_ROOT, LED_OFF);
            for (i = 0; i < 2; i++)
            {
                pal_timer_delay(50000);
            }

            if (button_pressed())
            {
                /* Re-start. */

                pal_led(LED_ROOT, LED_OFF);
                pal_led(LED_DATA_TX, LED_OFF);
                pal_led(LED_DATA_RX, LED_OFF);

                wpan_mlme_reset_req(true);
                break;
            }
        }
    }

    /* Keep compiler happy */
    parameter = parameter;
}


/**
 * Callback function that is called when data transmission needs to be started
 *
 * @param parameter  Pointer to parameter - unused
 */
static void data_tx_start_cb(void *parameter)
{
    /* @TODO Set MCU to sleep or reduced MCU clock speed:
     * here set to normal speed */

    pal_led(LED_DATA_TX, LED_ON);

    /* Send sensor data */
    send_sensor_data();

    /* Keep compiler happy */
    parameter = parameter;
}


/**
 * Determine if button is pressed
 *
 * @return true if button is pressed, else false
 */
static bool button_pressed(void)
{
    if (pal_button_read(BUTTON_0) == BUTTON_PRESSED)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * Initialize the ADC that is used for temperature measurement
 */
#ifdef TEMPERATURE_MEASUREMENT
static void adc_init(void)
{
}
#endif


/**
 * Get the battery/supply voltage value
 *
 * @return Supply voltage as measured with TRX' battery monitor
 */
static uint16_t get_battery_value(void)
{
    return tfa_get_batmon_voltage();
}


/**
 * Get the temperatur value
 *
 * @return Supply voltage as measured with TRX' battery monitor
 */
static uint16_t get_temperature_value(void)
{
    /* No temperature value available */
    return 0;
}


/**
 * Initialize peripheral used by application
 */
static void app_peripheral_init(void)
{
    pal_button_init();
#ifdef TEMPERATURE_MEASUREMENT
    adc_init();
#endif
}


/**
 * Create and send broadcast message used for network setup procedure
 */
static void send_broadcast(void)
{
    uint8_t src_addr_mode;
    wpan_addr_spec_t dst_addr;
    uint8_t payload;

    src_addr_mode = WPAN_ADDRMODE_LONG;
    dst_addr.AddrMode = WPAN_ADDRMODE_SHORT;
    dst_addr.PANId = DEFAULT_PAN_ID;
    dst_addr.Addr.short_address = BROADCAST;

    wpan_mcps_data_req(src_addr_mode,
                       &dst_addr,
                       1 /* payload length */,
                       &payload,
                       0 /* msduHandle */,
                       WPAN_TXOPT_OFF);
    /*
     * This request will cause a mcps data confirm message ->
     * usr_mcps_data_conf
     */
}


/* EOF */
