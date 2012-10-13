/**
 * @file main.c
 *
 * @brief STB Example - Secure_Remote_Control
 *
 * $Id: main.c 21540 2010-04-13 09:04:22Z sschneid $
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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "pal.h"
#include "sio_handler.h"
#include "tal.h"
#include "stb.h"
#include "app_config.h"
#include "ieee_const.h"
#include "bmm.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */

#if (TAL_TYPE == AT86RF212)
    #ifdef CHINESE_BAND
        #define DEFAULT_CHANNEL                 (0)
        #define DEFAULT_CHANNEL_PAGE            (5)
    #else
        #define DEFAULT_CHANNEL                 (1)
        #define DEFAULT_CHANNEL_PAGE            (0)
    #endif  /* #ifdef CHINESE_BAND */
#else
#define DEFAULT_CHANNEL         (20)
#endif
#define DEFAULT_PAN_ID          (0xABBA)
#define DST_PAN_ID              (DEFAULT_PAN_ID)
#define SRC_PAN_ID              (DEFAULT_PAN_ID)
#define OWN_SHORT_ADDR          (0x0001)
#define DST_SHORT_ADDR          (0x0001)

/* Frame overhead due to selected address scheme incl. FCS */
#if (DST_PAN_ID == SRC_PAN_ID)
#define FRAME_OVERHEAD          (11)
#else
#define FRAME_OVERHEAD          (13)
#endif

/*
 * Length of plain MAC payload of frame.
 * This has nothing to do with security.
 */
#define PLD_LEN                 (13)    /* 'Toggle light!' w/o '/0' */

/*
 * In case ZigBee security is used, we need to add the
 * Auxiliary Security Frame header, append the encrptyted payload,
 * and then append the encrypted MIC.
 * This together forms the encrypted payload.
 */
/* Used ZigBee security level: MIC = 8 octets, Data encryption on. */
#define SEC_LEVEL               (6)
/* Length of MIC in octets (depending on the used ZigBee security level. */
#define MIC_LEN                 (8)
/* Network key sequence number */
#define NWK_KEY_NO              (0)

/* Number of LED toggles to indicate security error. */
#define SEC_FAIL_CNT            (10)

#define SEC_CTRL_FIELD          (SEC_LEVEL |                                \
                                 (KEY_ID_NWK_KEY << SEC_CTRL_POS_KEY_ID) |  \
                                 (1 << (SEC_CTRL_POS_EXT_NONCE))            \
                                )

/* This is the time period in micro seconds for button debouncing. */
#define DEBOUNCE_PERIOD         (200000)

#if (NO_OF_LEDS >= 3)
#define LED_START               (LED_0)
#define LED_SEC                 (LED_1)
#define LED_DATA                (LED_2)
#elif (NO_OF_LEDS == 2)
#define LED_START               (LED_0)
#define LED_SEC                 (LED_0)
#define LED_DATA                (LED_1)
#else
#define LED_START               (LED_0)
#define LED_SEC                 (LED_0)
#define LED_DATA                (LED_0)
#endif

/* === GLOBALS ============================================================= */

static uint8_t tx_frame_buffer[LARGE_BUFFER_SIZE];  // Much more room is allocated than actually used.
static frame_info_t *tx_frame_info;
static uint8_t tx_cmd_string[PLD_LEN] = "Toggle light!";

/* Arbitrary 128-bit key of demo application. */
static uint8_t stb_app_key[AES_BLOCKSIZE] =
{
    0xA1, 0xA3, 0xA5, 0xA7,
    0x3C, 0x5E, 0x70, 0xB2,
    0x01, 0x52, 0x93, 0xC4,
    0x84, 0x48, 0x20, 0x10
};

/* Status of security utilization on this board. */
static bool use_security = true;

/* Last button state. */
static button_state_t button_state = BUTTON_OFF;

/* Frame to be transmitted using security. */
static uint8_t frame_payload[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE];
/*
 * The nonce: Initialization Vector (IV) as used in cryptography;
 * the ZigBee nonce (13 bytes long) are the bytes 2...14 of this nonce.
 */
static uint8_t nonce[AES_BLOCKSIZE];
static uint32_t framecounter = 0;
static uint8_t *keyp = stb_app_key;

static bool tx_ongoing = false;
static bool debounce_timer_running = false;
static uint8_t seq_num;

/* === PROTOTYPES ========================================================== */

static void app_task(void);
static void configure_pibs(void);
static void debounce_timer_cb(void *parameter);
static void indicate_failed_sec(void);
static void decrypt_frame(uint8_t secure_payload[AUX_HDR_LEN + PLD_LEN + MIC_LEN]);
static void encrypt_frame(uint8_t frame_payload[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE],
                                 uint8_t nonce[AES_BLOCKSIZE]);
static void tx_frame(bool security);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the Secure Remote Control application
 */
int main(void)
{
    /* Initialize the TAL layer */
    if (tal_init() != MAC_SUCCESS)
    {
        /* Something went wrong during initialization. */
        pal_alert();
    }

    /* Initialize LEDs. */
    pal_led_init();
    pal_led(LED_START, LED_ON);         /* Indicates application is started. */
    pal_led(LED_SEC, LED_OFF);          /* Indicates security not used on this board. */
    pal_led(LED_DATA, LED_OFF);         /* Indicates successfull data transmission. */

    /*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();

#ifdef SIO_HUB
    /* Initialize the serial interface used for communication with terminal program */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        /* Something went wrong during initialization. */
        pal_alert();
    }

#if ((!defined __ICCAVR__) && (!defined __ICCARM__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif
#endif

    /* Initialize Button. */
    pal_button_init();

    if (BUTTON_PRESSED == pal_button_read(BUTTON_0))
    {
        /*
         * Button was pressed during board start-up.
         * LED 1 off indicates security is NOT used on this board.
         */
        use_security = false;
    }

    if (use_security)
    {
        pal_led(LED_SEC, LED_ON);
#ifdef SIO_HUB
        printf("App started with security on\n");
#endif
    }
    else
    {
#ifdef SIO_HUB
        printf("App started with security off\n");
#endif
    }

    /* Init Security Toolbox (incl. AES unit). */
    stb_init();

    /* Configure the TAL PIBs; e.g. set short address */
    configure_pibs();

    /* Generate first sequence number randomly. */
    seq_num = (uint8_t)rand();

    /* Init tx frame info structure value that do not change during program execution */
    tx_frame_info = (frame_info_t *)tx_frame_buffer;

    /* Switch receiver on */
    tal_rx_enable(PHY_RX_ON);

    /* Endless while loop */
    while (1)
    {
        pal_task(); /* Handle platform specific tasks, like serial interface */
        tal_task(); /* Handle transceiver specific tasks */
        app_task(); /* Application task */
    }
}


/**
 * @brief Application task
 */
static void app_task(void)
{
    /*
     * Only read button if last transmission is finished and
     * the button debounce timer is not running.
     */
    if ((!tx_ongoing) && (!debounce_timer_running))
    {
        /*
         * In case the button was NOT pressed, we need to read it again to check
         * if it is pressed now.
         */
        if (BUTTON_OFF == button_state)
        {
            if (BUTTON_PRESSED == pal_button_read(BUTTON_0))
            {
                /* Transmit frame. */
                tx_frame(use_security);

                button_state = BUTTON_PRESSED;
            }
        }
        /*
         * In case the button was pressed already, we need to read it again
         * to check whether it is released now.
         */
        else
        {
            if (BUTTON_OFF == pal_button_read(BUTTON_0))
            {
                /* Button is released now. */
                button_state = BUTTON_OFF;
            }
        }
    }
}



/**
 * @brief Transmits frame
 *
 * @param security Requires usage of security for frame
 */
static void tx_frame(bool security)
{
    uint8_t no_of_tx_bytes;
    uint16_t fcf = 0;
    uint8_t *frame_ptr;
    uint16_t temp_value_16;

    if (security)
    {
        /* Send frame using security. */
        no_of_tx_bytes = AUX_HDR_LEN + PLD_LEN + MIC_LEN;

        /* Copy plaintext payload to proper place in Tx buffer. */
        memcpy(frame_payload + AUX_HDR_LEN, tx_cmd_string, PLD_LEN);

        encrypt_frame(frame_payload, nonce);
    }
    else
    {
        /* Send frame in plaintext. */
        no_of_tx_bytes = PLD_LEN;

        /* Copy plaintext payload to proper place in Tx buffer. */
        memcpy(frame_payload, tx_cmd_string, PLD_LEN);
    }


    /* Set payload pointer. */
    frame_ptr = (uint8_t *)tx_frame_info +
                LARGE_BUFFER_SIZE -
                no_of_tx_bytes - 2; /* Add 2 octets for FCS. */

    /* Copy payload to end of frame buffer. */
    memcpy(frame_ptr,
           frame_payload,
           no_of_tx_bytes);


    /* Source Address */
    temp_value_16 = OWN_SHORT_ADDR;
    frame_ptr -= 2;
    convert_16_bit_to_byte_array(temp_value_16, frame_ptr);



    /* Source PAN-Id */
#if (DST_PAN_ID == SRC_PAN_ID)
    /* No source PAN-Id included, but FCF updated. */
    fcf |= FCF_PAN_ID_COMPRESSION;
#else
    frame_ptr -= 2;
    temp_value_16 = SRC_PAN_ID;
    convert_16_bit_to_byte_array(temp_value_16, frame_ptr);
#endif


    /* Destination Address */
    temp_value_16 = DST_SHORT_ADDR;
    frame_ptr -= 2;
    convert_16_bit_to_byte_array(temp_value_16, frame_ptr);


    /* Destination PAN-Id */
    temp_value_16 = DST_PAN_ID;
    frame_ptr -= 2;
    convert_16_bit_to_byte_array(temp_value_16, frame_ptr);


    /* Set DSN. */
    frame_ptr--;
    *frame_ptr = seq_num++;


    /* Set the FCF. */
    fcf = FCF_FRAMETYPE_DATA | FCF_ACK_REQUEST |
          ((uint16_t)FCF_SHORT_ADDR << FCF_DEST_ADDR_OFFSET) |
          ((uint16_t)FCF_SHORT_ADDR << FCF_SOURCE_ADDR_OFFSET);

#if (DST_PAN_ID == SRC_PAN_ID)
    fcf |= FCF_PAN_ID_COMPRESSION;
#endif

    frame_ptr -= 2;
    convert_16_bit_to_byte_array(fcf, frame_ptr);

    /* First element shall be length of PHY frame. */
    frame_ptr--;
    *frame_ptr = no_of_tx_bytes + FRAME_OVERHEAD;

    /* Finished building of frame. */
    tx_frame_info->mpdu = frame_ptr;



    /* Mark ongoing transmission. */
    tx_ongoing = true;
    tal_tx_frame(tx_frame_info, CSMA_UNSLOTTED, true);

    /* Start button debounce timer. */
    debounce_timer_running = true;
    pal_timer_start(TIMER_DEBOUNCE,
                    DEBOUNCE_PERIOD,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)debounce_timer_cb,
                    NULL);
}



/**
 * @brief Callback that is called if data has been received by trx.
 *
 * @param mac_frame_info    Pointer to received data structure
 */
void tal_rx_frame_cb(frame_info_t *frame)
{
#ifdef SIO_HUB
    uint8_t i;
#endif

    /*
     * Length of MSDU (= encrypted or non-encrpyted payload)
     * of received frame.
     */
    uint8_t rx_msdu_len = frame->mpdu[0] - FRAME_OVERHEAD;

    /*
     * Pointer to MSDU (= encrypted or non-encrpyted payload)
     * of received frame.
     */
    uint8_t *rx_frame_msdu = frame->mpdu + FRAME_OVERHEAD + LENGTH_FIELD_LEN - FCS_LEN;

    if (use_security)
    {
#ifdef SIO_HUB
        printf("Security on: ");
#endif

        if ((rx_msdu_len != (AUX_HDR_LEN + PLD_LEN + MIC_LEN)) ||
            (rx_frame_msdu[0] != (uint8_t)(SEC_CTRL_FIELD))
           )
        {
            indicate_failed_sec();
#ifdef SIO_HUB
            printf("Rx unknown frame without security\n");
#endif
        }
        else
        {
            /*
             * Secured frame was received,
             * continue frame evaluation and decryption.
             */
            decrypt_frame(rx_frame_msdu);
        }
    }
    else    /* if (use_security) */
    {
#ifdef SIO_HUB
        printf("Security off: ");
#endif

        /* Check whether this frame was received using security. */
        if (rx_frame_msdu[0] == (uint8_t)(SEC_CTRL_FIELD))
        {
            indicate_failed_sec();
#ifdef SIO_HUB
            printf("Rx unknown frame with security\n");
#endif
        }
        else
        {
            /* Print received data to terminal program using UART/USB. */
#ifdef SIO_HUB
            printf("Rx plaintext: ");

            for (i = 0; i < rx_msdu_len; i++)
            {
                sio_putchar(rx_frame_msdu[i]);
            }
            printf("\n");
#endif

            /* Indicate data recption. */
            pal_led(LED_DATA, LED_TOGGLE);
        }
    }

    /* Free buffer that was used for frame reception. */
    bmm_buffer_free((buffer_t *)(frame->buffer_header));
}



/**
 * @brief Callback that is called once tx is done.
 *
 * @param status    Status of the transmission procedure
 * @param frame     Pointer to the transmitted frame structure
 */
void tal_tx_frame_done_cb(retval_t status, frame_info_t *frame)
{
    if (status == MAC_SUCCESS)
    {
#ifdef SIO_HUB
        /* Print transmitted bytes to terminal program. */
        if (use_security)
        {
            printf("Security on: Tx secured ok\n");
        }
        else
        {
            printf("Security off: Tx plaintext ok\n");
        }
#endif

        /* Indicate successfull data transmission. */
        pal_led(LED_DATA, LED_TOGGLE);
    }
    else
    {
        indicate_failed_sec();
    }

    tx_ongoing = false;

    frame = frame;  /* Keep compiler happy. */
}



/**
 * @brief Callback function for the button debounce application timer
 *
 * @param parameter Pointer to callback parameter
 *                  (not used in this application, but could be if desired).
 */
static void debounce_timer_cb(void *parameter)
{
    /* Mark button debounce timer as not running. */
    debounce_timer_running = false;

    parameter = parameter;  /* Keep compiler happy. */
}



/**
 * @brief Configure the TAL with the required PIB attributes
 */
static void configure_pibs(void)
{
    uint16_t temp_value_16;
    uint8_t temp_value_8;

    /*
     * Set TAL PIBs
     * Use: retval_t tal_pib_set(uint8_t attribute, uint8_t *value);
     */
    temp_value_16 = DEFAULT_PAN_ID;
    tal_pib_set(macPANId, (pib_value_t *)&temp_value_16);

    temp_value_16 = OWN_SHORT_ADDR;
    tal_pib_set(macShortAddress, (pib_value_t *)&temp_value_16);

    temp_value_8 = DEFAULT_CHANNEL;
    tal_pib_set(phyCurrentChannel, (pib_value_t *)&temp_value_8);
}



/**
 * @brief Encrpyt plaintext frame
 *
 * This function creates the actual secured/encrypted frame from plaintext payload
 * including the Auxiliary security header and MIC by calling the Security Toolbox.
 *
 * @param frame_payload Payload to be secured
 * @param nonce Nonce to be used
 */
static void encrypt_frame(uint8_t frame_payload[AUX_HDR_LEN + PLD_LEN + AES_BLOCKSIZE],
                                 uint8_t nonce[AES_BLOCKSIZE])
{
    uint8_t i;

    /* Initialize nonce. */
    memset(nonce, 0, AES_BLOCKSIZE);

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
    nonce[1 + (NONCE_POS_SEC_CTRL - 1)] = (uint8_t)SEC_CTRL_FIELD;
    frame_payload[MSDU_POS_SEC_CTRL - 1] = (uint8_t)SEC_CTRL_FIELD;

    /* IEEE source address */
    memcpy(frame_payload + (MSDU_POS_SRC_ADDR - 1), nonce + 1, sizeof(tal_pib_IeeeAddress));

    /* The first (and the only) network key. */
    frame_payload[MSDU_POS_KEY_SEQ_NO - 1] = NWK_KEY_NO;

    /* Insert frame counter into nonce and auxiliary security header. */
    for (i = FRM_COUNTER_LEN; i--; /* */)
    {
        nonce[sizeof(tal_pib_IeeeAddress) + FRM_COUNTER_LEN - i] =
            (framecounter >> (i << 3)) & 0xFF; // MSB first
        frame_payload[FRM_COUNTER_LEN - i] = (framecounter >> (i << 3)) & 0xFF; // MSB first
    }

    ++framecounter;

    /* Call Security Toolbox to encrypt frame. */
    stb_ccm_secure(frame_payload,
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
}



/**
 * @brief Decrypt secured frame
 *
 * This function decrypts a received secured frame by calling the Security Toolbox.
 *
 * @param secure_payload Secured payload to be encrypted
 */
static void decrypt_frame(uint8_t secure_payload[AUX_HDR_LEN + PLD_LEN + MIC_LEN])
{
    uint32_t rcvd_framecounter;
#ifdef SIO_HUB
    char sio_array[255];
#endif
    uint8_t i;

    /* Extract received IEEE source address. */
    memcpy(nonce + 1, secure_payload + (MSDU_POS_SRC_ADDR - 1), sizeof(tal_pib_IeeeAddress));
    nonce[1 + (NONCE_POS_SEC_CTRL - 1)] = (uint8_t)(SEC_CTRL_FIELD);

    /* Read framecounter. */
    for (rcvd_framecounter = 0, i = FRM_COUNTER_LEN; i--; /*  */)
    {
        rcvd_framecounter |= (secure_payload[FRM_COUNTER_LEN - i] << (i << 3));
    }

    /* Copy received framecounter to nonce. */
    memcpy(nonce + 1 + sizeof(tal_pib_IeeeAddress), secure_payload + 1, FRM_COUNTER_LEN);

    /* Call Security Toolbox to decrypt frame. */
    switch(stb_ccm_secure(secure_payload,
                          nonce,
                          keyp,
                          AUX_HDR_LEN,
                          PLD_LEN,
                          SEC_LEVEL,
                          AES_DIR_DECRYPT))
    {
        case STB_CCM_OK:
#ifdef SIO_HUB
            printf("Rx secured, decrypyted: ");
            for (i = AUX_HDR_LEN; i < (AUX_HDR_LEN + PLD_LEN); i++)
            {
                sio_putchar(secure_payload[i]);
            }
            printf("\n");
#endif

            /* Indicate data recption. */
            pal_led(LED_DATA, LED_TOGGLE);
            break;

        case STB_CCM_MICERR:
        default:
            indicate_failed_sec();
#ifdef SIO_HUB
            sprintf(sio_array, "Frame %" PRIu32 ": MIC wrong\n", rcvd_framecounter);
            printf(sio_array);
#endif
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
 * @brief Indicate security error by fast LED blinking.
 */
static void indicate_failed_sec(void)
{
    uint16_t cnt;

    for (cnt = SEC_FAIL_CNT; cnt--; /* */)
    {
        pal_timer_delay(0xFFFF);
        pal_led(LED_SEC, LED_TOGGLE);
    }

    if (use_security)
    {
        pal_led(LED_SEC, LED_ON);
    }
    else
    {
        pal_led(LED_SEC, LED_OFF);
    }
}

/* EOF */
