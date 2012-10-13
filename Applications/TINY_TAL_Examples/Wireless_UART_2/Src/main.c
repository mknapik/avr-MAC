/**
 * @file main.c
 *
 * @brief  Main of TINY_TAL Example - Wireless_UART_2
 *
 * Improved Wireless UART, based on TINY_TAL Example.
 * Uses a timer to improve over-the-air-performance through
 * sending data block-wise instead of transmitting frames upon
 * every single byte received over the serial connection.
 *
 * $Id: main.c,v 1.1.4.1 2010/09/07 17:39:38 dam Exp $
 *
 * @author    de: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 */

/* === INCLUDES ============================================================ */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pal.h"
#include "tiny_tal.h"
#include "app_config.h"
#include "ieee_const.h"

/* === TYPES =============================================================== */

/**
 * This enum stores the current transmission state
 */
typedef enum tx_state_tag
{
    TX_IDLE = 0,
    TX_ONGOING,
    TX_RETRY
}
tx_state_t;

/* === MACROS ============================================================== */

#if (TAL_TYPE == AT86RF212)
#ifdef CHINESE_BAND
#define DEFAULT_CHANNEL             (0)
#define DEFAULT_CHANNEL_PAGE        (5)
#else
#define DEFAULT_CHANNEL             (1)
#define DEFAULT_CHANNEL_PAGE        (0)
#endif
#else
#define DEFAULT_CHANNEL             (20)
#endif

#ifdef MULTIPLE_NODES
/* distinguish nodes by different source/sink addressing */
#define NODE                        (2))
#endif
#define DEFAULT_PAN_ID              (0x00DE)
#define DST_PAN_ID                  (DEFAULT_PAN_ID)
#define SRC_PAN_ID                  (DEFAULT_PAN_ID)

#ifdef MULTIPLE_NODES
#if(NODE==1)
#define OWN_SHORT_ADDR              (0x0001)
#define DST_SHORT_ADDR              (0x0002)
#elif(NODE==2)
#define OWN_SHORT_ADDR              (0x0002)
#define DST_SHORT_ADDR              (0x0001)
#else
#error "unsupported NodeID"
#endif
#else
#define OWN_SHORT_ADDR              (0x0001)
#define DST_SHORT_ADDR              (0x0001)
#endif

/* Frame overhead due to selected address scheme incl. FCS */
#if (DST_PAN_ID == SRC_PAN_ID)
#define FRAME_OVERHEAD              (11)
#else
#define FRAME_OVERHEAD              (13)
#endif

#define MAX_APP_DATA_LENGTH         (100)   // <= 102 byte && <= RX/TX buffer size
#define TIMER_INTERVAL              (25)    // milliseconds

#define ALERT_ON_ERROR              (true)  // just for debugging
#define SEND_BLOCKWISE              (true)  // prefer sending data in blocks

/*
 * Ensure that MAX_APP_DATA_LENGTH is NOT larger than USB_TX_BUF_SIZE
 * and USB_RX_BUF_SIZE
 */
#ifdef USB0
#if ((MAX_APP_DATA_LENGTH > USB_TX_BUF_SIZE) || (MAX_APP_DATA_LENGTH > USB_RX_BUF_SIZE) \
        || (MAX_APP_DATA_LENGTH > aMaxMACSafePayloadSize) )
#error "MAX_APP_DATA_LENGTH too large"
#endif
#elif (defined UART0 | defined UART1)
#if ((MAX_APP_DATA_LENGTH > UART_MAX_TX_BUF_LENGTH)  \
        || (MAX_APP_DATA_LENGTH > UART_MAX_RX_BUF_LENGTH) \
        || (MAX_APP_DATA_LENGTH > aMaxMACSafePayloadSize) )
#error "MAX_APP_DATA_LENGTH too large"
#endif
#endif

#ifndef SIO_CHANNEL
#if (PAL_GENERIC_TYPE != SAM3 )
#ifdef UART0
#define SIO_CHANNEL                     (SIO_0)
#endif
#ifdef UART1
#define SIO_CHANNEL                     (SIO_1)
#endif
#endif /* (PAL_GENERIC_TYPE != SAM3 ) */
#ifdef USB0
#define SIO_CHANNEL                     (SIO_2)
#endif
#endif /* #ifndef SIO_CHANNEL */

#if (NO_OF_LEDS >= 3)
#define LED_START                       (LED_0)
#define LED_DATA_RX                     (LED_1)
#define LED_DATA_TX                     (LED_2)
#elif (NO_OF_LEDS == 2)
#define LED_START                       (LED_0)
#define LED_DATA_RX                     (LED_0)
#define LED_DATA_TX                     (LED_1)
#else
#define LED_START                       (LED_0)
#define LED_DATA_RX                     (LED_0)
#define LED_DATA_TX                     (LED_0)
#endif

/* === GLOBALS ============================================================= */

static tx_state_t tx_state = TX_IDLE;
static uint8_t tx_buffer[LARGE_BUFFER_SIZE];
static uint8_t sio_rx_data[MAX_APP_DATA_LENGTH];

/* === PROTOTYPES ========================================================== */

static void app_task(void);
static void configure_frame_sending(void);
static void start_timer(void);
static void data_fwd_cb(void*);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the Wireless UART application
 */
int main(void)
{
    /* Initialize the TAL layer */
    if (tal_init() != MAC_SUCCESS)
    {
        // something went wrong during initialization
        pal_alert();
    }

    /* Calibrate MCU's RC oscillator */
    pal_calibrate_rc_osc();

    /* Initialize LEDs */
    pal_led_init();
    pal_led(LED_START, LED_ON);     // indicating application is started
    pal_led(LED_DATA_RX, LED_OFF);  // indicating data reception
    pal_led(LED_DATA_TX, LED_OFF);  // indicating successfull data transmission

    /*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();

    /* Initialize the serial interface used for communication with terminal program */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        // something went wrong during initialization
        pal_alert();
    }

    /* Configure TX frame and transceiver */
    configure_frame_sending();

    /* Switch receiver on */
    tal_rx_enable(PHY_RX_ON);

#if(SEND_BLOCKWISE==true)
    start_timer();
#endif

    /* Endless while loop */
    while (1)
    {
        pal_task(); /* Handle platform specific tasks, like serial interface */
        tal_task(); /* Handle transceiver specific tasks */
#if(!(SEND_BLOCKWISE==true))
        app_task(); /* Application task */
#endif
    }
}


/**
 * @brief Application task
 */
static void app_task(void)
{
    uint8_t number_of_bytes_to_be_transmitted;

    if (tx_state == TX_IDLE)
    {
        number_of_bytes_to_be_transmitted = pal_sio_rx(SIO_CHANNEL, sio_rx_data, MAX_APP_DATA_LENGTH);

        // If bytes are received via UART/USB, transmit the bytes.
        if (number_of_bytes_to_be_transmitted > 0)
        {
            pal_led(LED_START, LED_TOGGLE);        // indicate data processing
            tx_state = TX_ONGOING;

            tx_buffer[PL_POS_SEQ_NUM]++;

            /* Check for maximum allowed IEEE 802.15.4 frame length. */
            if (number_of_bytes_to_be_transmitted > aMaxMACSafePayloadSize)
            {
                number_of_bytes_to_be_transmitted = aMaxMACSafePayloadSize;
            }

            /* Update mpdu length within frame. */
            tx_buffer[0] = number_of_bytes_to_be_transmitted + FRAME_OVERHEAD;

            /* Copy MSDU (actual MAC payload) into frame. */
            /*
             * Note: Usually the MSDU is copied beginning from the end of
             * the frame. Since the header is always the same for this
             * application, the start of the MSDU is always the same position.
             * Therefore the payload copying is done from the beginning.
             */
            memcpy(&tx_buffer[LENGTH_FIELD_LEN + FRAME_OVERHEAD  - FCS_LEN],
                    sio_rx_data,
                    number_of_bytes_to_be_transmitted);

            tal_tx_frame(tx_buffer, CSMA_UNSLOTTED, true);
        }
    }
    else if (tx_state == TX_RETRY)
    {
        tx_state = TX_ONGOING;

        /* Retransmit the previous frame until frame transmission was successful. */
        tal_tx_frame(tx_buffer, CSMA_UNSLOTTED, true);
    }
}


/**
 * @brief callback for timer expiry.
 * The timer interval determines how often data frames are to be transmitted
 * and also by the size relationship between frame payload and frame header.
 * Larger timer intervals result in a better ratio, although too large
 * intervals will mean data loss if the usart buffer isn't emptied fast enough
 * and flow control is disabled.

 * @param par just for compatibility with the PAL timer callback system
 */
static void data_fwd_cb(void* par)
{
    app_task();

#if(SEND_BLOCKWISE==true)
    /* restart timer */
    start_timer();
#endif
}


/**
 * @brief Callback that is called if data has been received by trx.
 *
 * @param rx_frame_array Pointer to data array containing received frame
 */
void tal_rx_frame_cb(uint8_t *rx_frame_array)
{
    uint8_t rx_payload_len = rx_frame_array[0] - FRAME_OVERHEAD;
    uint8_t *rx_payload_ptr = rx_frame_array + FRAME_OVERHEAD + LENGTH_FIELD_LEN - FCS_LEN;
    uint8_t sio_len_rx;

    /* Print received data to terminal program. */
    bool sio_ongoing = true;

    do
    {
        sio_len_rx = pal_sio_tx(SIO_CHANNEL, rx_payload_ptr, rx_payload_len);

        if (sio_len_rx < rx_payload_len)
        {
            rx_payload_len -= sio_len_rx;
            rx_payload_ptr += sio_len_rx;
            pal_task();
        }
        else
        {
            sio_ongoing = false;
        }
    } while (sio_ongoing);

    pal_led(LED_DATA_RX, LED_TOGGLE);    // indicating data recption
}


/**
 * @brief Callback that is called once tx is done.
 *
 * @param status    Status of the transmission procedure
 */
void tal_tx_frame_done_cb(retval_t status)
{
    if (status == MAC_SUCCESS)
    {
        uint8_t tx_payload_len = tx_buffer[0] - FRAME_OVERHEAD;
        uint8_t *tx_payload_ptr = tx_buffer + FRAME_OVERHEAD + LENGTH_FIELD_LEN - FCS_LEN;
        uint8_t sio_len_tx;

        /* Print transmitted bytes to terminal program. */

        bool sio_ongoing = true;

        do
        {
            sio_len_tx = pal_sio_tx(SIO_CHANNEL, tx_payload_ptr, tx_payload_len);

            if (sio_len_tx < tx_payload_len)
            {
                tx_payload_len -= sio_len_tx;
                tx_payload_ptr += sio_len_tx;
                pal_task();
            }
            else
            {
                sio_ongoing = false;
            }
        } while (sio_ongoing);

        pal_led(LED_DATA_TX, LED_TOGGLE);    // indicating successfull data transmission

        /* After transmission is completed, allow next transmission. */
        tx_state = TX_IDLE;
    }
    else if (status == MAC_CHANNEL_ACCESS_FAILURE)
        /*
         * Channel access failure is the only transmission failure that makes sense
         * to be handled within this application. For handling other status codes,
         * such as MAC_NO_ACK, this is probably the wrong application on the wrong layer.
         * For absolutely reliable transmission, please use a MAC or TAL based
         * application. The Tiny-TAL is not designed for such a purpose.
         *
         * In case of channel access failure the frame is retried.
         */
    {
        /* Transmission was not successful, initiate retry. */
        tx_state = TX_RETRY;
        //TODO: retry counter?
    }
    else
        /*
         * Other transmission status codes, such as MAC_NO_ACK are not handled
         * within this application.
         * The transmission is considered as beeing completed for this frame.
         */
    {
        tx_state = TX_IDLE;

#if(ALERT_ON_ERROR==true)
        pal_alert();
#endif
    }
}


/**
 * @brief starts a timer
 */
static void start_timer()
{
    uint8_t status = pal_timer_start(    TIMER_DATA_FWD,
            (uint32_t)1000*TIMER_INTERVAL,
            TIMEOUT_RELATIVE,
            (void*)data_fwd_cb,
            NULL);

#if(ALERT_ON_ERROR==true)
    if(status != MAC_SUCCESS)
    {
        pal_alert();
    }
#endif
}


/**
 * @brief Configure the frame sending
 *
 * Prepares the static contents of the frame buffer and
 * writes settings to the TAL pib.
 */
static void configure_frame_sending(void)
{
    uint8_t temp_value_8;
    uint16_t temp_value_16;
    uint16_t fcf = 0;

    /* Set initial frame length to MHR length without additional MSDU. */
    tx_buffer[0] = FRAME_OVERHEAD;

    /* Calculate FCF. */
    fcf = FCF_FRAMETYPE_DATA | FCF_ACK_REQUEST;
    fcf |= FCF_SET_SOURCE_ADDR_MODE(FCF_SHORT_ADDR);
    fcf |= FCF_SET_DEST_ADDR_MODE(FCF_SHORT_ADDR);

#if (DST_PAN_ID == SRC_PAN_ID)
    fcf |= FCF_PAN_ID_COMPRESSION;
#endif

    /* Set FCF in frame. */
    convert_16_bit_to_byte_array(fcf, &tx_buffer[PL_POS_FCF_1]);

    /* Set initial sequence number. */
    tx_buffer[PL_POS_SEQ_NUM] = (uint8_t)rand();

    /* Set Destination PAN-Id in frame. */
    temp_value_16 = DEFAULT_PAN_ID;
    convert_16_bit_to_byte_array(temp_value_16, &tx_buffer[PL_POS_DST_PAN_ID_START]);

    /* Set Destination Address in frame. */
    temp_value_16 = DST_SHORT_ADDR;
    convert_16_bit_to_byte_array(temp_value_16, &tx_buffer[PL_POS_DST_ADDR_START]);

    /* Set Source PAN-Id in frame. */
    temp_value_16 = DEFAULT_PAN_ID;
    tal_pib_set(macPANId, (pib_value_t *)&temp_value_16);
#if (DST_PAN_ID != SRC_PAN_ID)
    convert_16_bit_to_byte_array(temp_value_16, &tx_buffer[PL_POS_DST_ADDR_START + 2]);
#endif

    /* Set Source Address in frame. */
    temp_value_16 = OWN_SHORT_ADDR;
    tal_pib_set(macShortAddress, (pib_value_t *)&temp_value_16);
#if (DST_PAN_ID != SRC_PAN_ID)
    convert_16_bit_to_byte_array(temp_value_16, &tx_buffer[PL_POS_DST_ADDR_START + 4]);
#else
    convert_16_bit_to_byte_array(temp_value_16, &tx_buffer[PL_POS_DST_ADDR_START + 2]);
#endif

    /* Set proper channel. */
    temp_value_8 = DEFAULT_CHANNEL;
    tal_pib_set(phyCurrentChannel, (pib_value_t *)&temp_value_8);
}

/* EOF */
