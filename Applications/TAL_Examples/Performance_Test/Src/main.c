/**
 * @file main.c
 *
 * @brief  Main of TAL Example - Performance_Test
 *
 * $Id: main.c 22695 2010-07-27 13:13:57Z sschneid $
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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include "pal.h"
#include "tal.h"
#include "tfa.h"
#include "app_config.h"
#include "ieee_const.h"
#include "bmm.h"
#include "sio_handler.h"
#if (TAL_TYPE == AT86RF230A)
#include "phy230_registermap.h" // included for legacy reasons
#endif

/* === TYPES =============================================================== */

/* Enumeration used for transceiver operation mode */
typedef enum op_mode_tag
{
    OFF_OP_MODE,
    RX_OP_MODE,
    TX_OP_MODE,
    PROMISCUOUS_OP_MODE,
    CONTINOUS_TX_MODE
} op_mode_t;

/* === MACROS ============================================================== */

#if (TAL_TYPE == AT86RF212)
#define DEFAULT_CHANNEL         (1)
#else
#define DEFAULT_CHANNEL         (20)
#endif
#define DEFAULT_PAN_ID          (0xABCD)
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

#define DEFAULT_SCAN_DURATION   8

/* === GLOBALS ============================================================= */

static bool transmitting = false;
static bool receiving = false;
static uint8_t storage_buffer[LARGE_BUFFER_SIZE];
static frame_info_t *tx_frame_info;
static uint32_t number_test_frames = 100;
static uint8_t phy_frame_length = 20;
static op_mode_t op_mode = RX_OP_MODE;
static uint32_t number_rx_frames;
static uint32_t frame_successful;
static uint32_t frame_no_ack;
static uint32_t frame_access_failure;
static uint32_t frame_failure;
static uint32_t frames_to_transmit;
static bool ack_request = true;
static bool csma_enabled = true;
static bool retry_enabled = true;
static uint32_t start_time;
static uint32_t end_time;
static uint32_t aver_lqi;
static uint8_t channel_before_scan;
static uint32_t scan_channel_mask;
static bool scanning = false;
static uint8_t scan_duration;
#ifdef ANTENNA_DIVERSITY
static bool antenna_diversity = true;
#endif

/* === PROTOTYPES ========================================================== */

static void app_task(void);
static void configure_frame_sending(void);
static void configure_pibs(void);
static void print_main_menu(void);
static void get_number_test_frames(void);
static void get_frame_length(void);
static void get_channel(void);
static void get_tx_pwr(void);
static void get_page(void);
static void start_test(void);
static void print_result(void);
static void toggle_ack_request(void);
static void toggle_csma_enabled(void);
static void toggle_retry_enabled(void);
static void start_ed_scan(void);
static void get_sensor_data(void);
#if ((TAL_TYPE != AT86RF230B) || ((TAL_TYPE == AT86RF230B) && (defined CW_SUPPORTED)))
static void start_cw_transmission(void);
static void pulse_cw_transmission(void);
static void stop_pulse_cb(void *callback_parameter);
#endif
#ifdef ANTENNA_DIVERSITY
static void toogle_antenna_diversity(void);
#endif

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Main function of the Performance_Test application
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
    pal_led(LED_0, LED_ON);     // indicating application is started

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

#if ((!defined __ICCAVR__) && (!defined __ICCARM__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif

    /* Init tx frame info structure value that do not change during program execution */
    tx_frame_info = (frame_info_t *)storage_buffer;

    /* Configure the TAL PIBs; e.g. set short address */
    configure_pibs();

    /* Configure the frame sending; e.g. set short address */
    configure_frame_sending();

    /* Wait for the first key stroke before continuing */
    sio_getchar();

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
    if (frames_to_transmit > 0)
    {
        if (!transmitting)
        {
            transmitting = true;
            tx_frame_info->mpdu[PL_POS_SEQ_NUM]++;
            if (csma_enabled)
            {
                tal_tx_frame(tx_frame_info, CSMA_UNSLOTTED, retry_enabled);
            }
            else
            {
                tal_tx_frame(tx_frame_info, NO_CSMA_NO_IFS, retry_enabled);
            }
        }
    }
    else if (receiving)
    {
        /* While receiving wait for any key to stop receiving. */
        if (sio_getchar_nowait() != -1)
        {
            receiving = false;
            tal_rx_enable(PHY_TRX_OFF);
            print_result();
        }
    }
    else if (op_mode == PROMISCUOUS_OP_MODE)
    {
        if (sio_getchar_nowait() != -1)
        {
            uint8_t mode = false;

            tal_pib_set(macPromiscuousMode, (pib_value_t *)&mode);
            op_mode = OFF_OP_MODE;
            print_result();
        }
    }
    else if (op_mode == CONTINOUS_TX_MODE)
    {
        /* While CW transmission wait for any key to stop transmitting. */
    }
    else
    {
        if (scanning == false)
        {
            print_main_menu();
        }
    }
}


/**
 * @brief Callback that is called if data has been received by trx.
 *
 * @param mac_frame_info    Pointer to received data structure
 */
void tal_rx_frame_cb(frame_info_t *frame)
{
    number_rx_frames++;
    /* First extract LQI. */
    aver_lqi += frame->mpdu[frame->mpdu[0] + LQI_LEN];

    if (op_mode == PROMISCUOUS_OP_MODE)
    {
        uint8_t i;
        char ascii[5];

        printf("Rx: 0x%.2X ", frame->mpdu[0]);
        for (i = 0; i < (frame->mpdu[0] + 1); i++)    /* Including FCS & LQI */
        {
            sprintf(ascii, "%.2X ", frame->mpdu[i]);
            printf(ascii);
        }
        printf("\r\n");
    }

    /* free buffer that was used for frame reception */
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
        frame_successful++;
    }
    else
    {
        if (status == MAC_NO_ACK)
        {
            frame_no_ack++;
        }
        else if (status == MAC_CHANNEL_ACCESS_FAILURE)
        {
            frame_access_failure++;
        }
        else
        {
            frame_failure++;
        }
    }

    // After transmission is completed, allow next transmission.
    frames_to_transmit--;
    if (frames_to_transmit == 0)
    {
        pal_get_current_time(&end_time);
        tal_rx_enable(PHY_TRX_OFF);
        print_result();
    }
    transmitting = false;

    frame = frame;  /* Keep compiler happy. */
}


/**
 * @brief Print main menu to terminal program
 */
static void print_main_menu(void)
{
    uint8_t temp_var;
    uint8_t input;
    int8_t tx_pwr_dbm;

    printf("******************************************************\r\n");
    printf("Performance test application (");

    /* Transceiver version */
#if (TAL_TYPE == AT86RF212)
    printf("AT86RF212");
#elif (TAL_TYPE == AT86RF230A)
    printf("AT86RF230A");
#elif (TAL_TYPE == AT86RF230B)
    printf("AT86RF230B");
#elif (TAL_TYPE == AT86RF231)
    printf("AT86RF231");
#elif (TAL_TYPE == AT86RF232)
    printf("AT86RF232");
#elif (TAL_TYPE == ATMEGARF_TAL_1)
    // no output
#else
#error "unknown TAL type ";
#endif

#if (TAL_TYPE != ATMEGARF_TAL_1)
    printf(" / ");
#endif

    /* Print MCU version */
#if (PAL_GENERIC_TYPE == AVR)
    #if (PAL_TYPE == ATMEGA1281)
        printf("ATmega1281");
    #elif (PAL_TYPE == ATMEGA2561)
        printf("ATmega2561");
    #elif (PAL_TYPE == ATMEGA644P)
        printf("ATmega644P");
    #elif (PAL_TYPE == ATMEGA1284P)
        printf("ATmega1284P");
    #elif (PAL_TYPE == AT90USB1287)
        printf("AT90USB1287");
    #else
    #error "unknown PAL_TYPE";
    #endif
#elif (PAL_GENERIC_TYPE == XMEGA)
    #if (PAL_TYPE == ATXMEGA128A1)
        printf("ATxmega128A1");
    #elif (PAL_TYPE == ATXMEGA128A3)
        printf("ATxmega128A3");
    #elif (PAL_TYPE == ATXMEGA256A3)
        printf("ATxmega256A3");
    #elif (PAL_TYPE == ATXMEGA256D3)
        printf("ATxmega256D3");
    #else
    #error "unknown PAL_TYPE";
    #endif
#elif (PAL_GENERIC_TYPE == MEGA_RF)
    #if (PAL_TYPE == ATMEGA128RFA1)
        printf("ATmega128RFA1");
    #else
    #error "unknown PAL_TYPE";
    #endif
#elif (PAL_GENERIC_TYPE == ARM7)
    #if (PAL_TYPE == AT91SAM7X256)
        printf("AT91SAM7X256");
    #else
    #error "unknown PAL_TYPE";
    #endif
#elif (PAL_GENERIC_TYPE == MEGA_RF)
    /* no putput */
#else
#error "unknown PAL_GENERIC_TYPE";
#endif

    printf(")\r\nSettings:\r\n");

    /* Print channel settings */
    tal_pib_get(phyCurrentChannel, &temp_var);
    printf("(C) : Channel = %d\r\n", temp_var);

    /* Print channel page settings */
    printf("(P) : Channel page ");
    if (tal_pib_get(phyCurrentPage, &temp_var) == MAC_SUCCESS)
    {
        printf("= %d\r\n", temp_var);
    }
    else
    {
        printf("not supported\r\n");
    }

    /* Print tx power settings */
    tal_pib_get(phyTransmitPower, &temp_var);
    tx_pwr_dbm = CONV_phyTransmitPower_TO_DBM(temp_var);
    printf("(W) : Tx power = %d dBm\r\n", tx_pwr_dbm);

    printf("(N) : Number of test frames = %" PRIu32 "\r\n", number_test_frames);
    printf("(L) : Frame length (PSDU) = %d\r\n", phy_frame_length);

    /* Print ACK settings */
    printf("(A) : ACK request = ");
    if (ack_request)
    {
        printf("ACK requested\r\n");
    }
    else
    {
        printf("no ACK requested\r\n");
    }

    /* Print frame retry settings */
    printf("(F) : Frame retry enabled = ");
    if (retry_enabled == true)
    {
        printf("true\r\n");
    }
    else
    {
        printf("false\r\n");
    }

    /* Print CSMA settings */
    printf("(M) : CSMA enabled = ");
    if (csma_enabled == true)
    {
        printf("true\r\n");
    }
    else
    {
        printf("false\r\n");
    }

    /* Print operation mode settings */
    printf("(T/R/O/I) : Operating mode Tx/Rx/Off/PromIscuous = ");
    switch (op_mode)
    {
        case OFF_OP_MODE: printf("OFF"); break;
        case RX_OP_MODE: printf("Rx"); break;
        case TX_OP_MODE: printf("Tx"); break;
        case PROMISCUOUS_OP_MODE: printf("Promiscuous Mode"); break;
        default: break;
    }
    printf("\r\n");
    printf("(E) : Energy scan on all channels\r\n");
#if ((TAL_TYPE != AT86RF230B) || ((TAL_TYPE == AT86RF230B) && (defined CW_SUPPORTED)))
    printf("(U) : Continuous transmission on current channel\r\n");
    printf("(D) : Transmit a continuous wave pulse on current channel\r\n");
#endif
#ifdef ANTENNA_DIVERSITY
    printf("(Y) : Toggle antenna diversity - ");
    if (antenna_diversity)
    {
        printf("enabled\r\n");
    }
    else
    {
        printf("disabled\r\n");
    }
#endif
#if (PAL_GENERIC_TYPE == MEGA_RF)
    printf("(V) : Get sensor data, i.e. supply voltage and temperature\r\n");
#else
    printf("(V) : Get sensor data, i.e. supply voltage\r\n");
#endif
    printf("(S) : Start test\r\n");
    printf(">");

    /* Get input from terminal program / user. */
    input = sio_getchar();
    input = toupper(input);

    /* Handle input from terminal program. */
    switch (input)
    {
        case 'C': get_channel(); break;
        case 'P': get_page(); break;
        case 'W': get_tx_pwr(); break;
        case 'N': get_number_test_frames(); break;
        case 'L': get_frame_length(); break;
        case 'A': toggle_ack_request(); break;
        case 'M': toggle_csma_enabled(); break;
        case 'F': toggle_retry_enabled(); break;

        case 'T':
            op_mode = TX_OP_MODE;
            break;

        case 'R':
            op_mode = RX_OP_MODE;
            break;

        case 'O':
            op_mode = OFF_OP_MODE;
            tal_rx_enable(PHY_TRX_OFF);
            break;

        case 'I':
            {
                uint8_t mode = true;

                number_rx_frames = 0;
                aver_lqi = 0;
                if (tal_pib_set(macPromiscuousMode, (pib_value_t *)&mode) == MAC_SUCCESS)
                {
                    op_mode = PROMISCUOUS_OP_MODE;
                    printf("\r\nPromiscuous mode started. Press any key to stop ...\r\n");
                }
            }
            break;

        case 'E':
            start_ed_scan();
            break;

#if ((TAL_TYPE != AT86RF230B) || ((TAL_TYPE == AT86RF230B) && (defined CW_SUPPORTED)))
        case 'U':
            start_cw_transmission();
            break;

        case 'D':
            pulse_cw_transmission();
            break;
#endif

#ifdef ANTENNA_DIVERSITY
        case 'Y': toogle_antenna_diversity();
            break;
#endif

        case 'V':
            get_sensor_data();
            break;

        case 'S':
            start_test();
            break;
    }
}


/**
 * @brief Configure the TAL PIbs
 */
static void configure_pibs(void)
{
    uint16_t temp_value_16;
    uint8_t temp_value_8;

    temp_value_16 = OWN_SHORT_ADDR;
    tal_pib_set(macShortAddress, (pib_value_t *)&temp_value_16);

    temp_value_16 = SRC_PAN_ID;
    tal_pib_set(macPANId, (pib_value_t *)&temp_value_16);

    /* Set channel. */
    temp_value_8 = (uint8_t)DEFAULT_CHANNEL;
    tal_pib_set(phyCurrentChannel, (pib_value_t *)&temp_value_8);
}



/**
 * @brief Configure the frame sending
 */
static void configure_frame_sending(void)
{
    uint16_t temp_value_16;
    uint8_t mac_frame_length;
    uint8_t *frame_ptr;
    uint8_t *temp_frame_ptr;
    uint16_t fcf = 0;

    /*
     * Fill in PHY frame.
     *
     * Also set TAL PIBs:
     * Use: retval_t tal_pib_set(uint8_t attribute, uint8_t *value);
     */
    /* Get length of current frame. */
    mac_frame_length = (phy_frame_length  - FRAME_OVERHEAD);

    /* Set payload pointer. */
    frame_ptr = temp_frame_ptr =
                (uint8_t *)tx_frame_info +
                LARGE_BUFFER_SIZE -
                mac_frame_length - 2; /* Add 2 octets for FCS. */

    /*
     * Assign dummy payload values.
     * Payload is stored to the end of the buffer avoiding payload copying by TAL.
     */
    for (uint8_t i = 0; i < mac_frame_length; i++)
    {
        *temp_frame_ptr++ = i; // dummy value
    }
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
    *frame_ptr = (uint8_t)rand();
    /* Set the FCF. */
    fcf |= FCF_FRAMETYPE_DATA | FCF_SET_SOURCE_ADDR_MODE(FCF_SHORT_ADDR) |
                                FCF_SET_DEST_ADDR_MODE(FCF_SHORT_ADDR);
        if (ack_request)
        {
            fcf |= FCF_ACK_REQUEST;
        }
    frame_ptr -= 2;
    convert_16_bit_to_byte_array(fcf, frame_ptr);

    /* First element shall be length of PHY frame. */
    frame_ptr--;
    *frame_ptr = phy_frame_length;

    /* Finished building of frame. */
    tx_frame_info->mpdu = frame_ptr;
}



/**
 * @brief Start the test procedure
         */
static void start_test(void)
        {
    if (op_mode == TX_OP_MODE)
    {
        printf("\r\nTransmitting... Wait until test is completed.");

        frames_to_transmit = number_test_frames;
        frame_no_ack = 0;
        frame_access_failure = 0;
        frame_failure = 0;
        pal_get_current_time(&start_time);
    }
    else if (op_mode == RX_OP_MODE)
    {
        aver_lqi = 0;
        number_rx_frames = 0;
        receiving = true;
        tal_rx_enable(PHY_RX_ON);
        printf("\r\nReceiving... Press any key when test is completed.");
    }
    else
    {
        printf("\r\nTransceiver is off. Press any key to return.\r\n");
        sio_getchar();
    }
}


/**
 * @brief Start energy scan on current channel page
 */
static void start_ed_scan(void)
{
    uint8_t first_channel;
    uint8_t i;
    bool character_entered = false; // no character is entered
    char input_char[3]= {0, 0, 0};
    uint8_t input;
#if (TAL_TYPE == AT86RF212)
    uint8_t page;
    uint8_t channel;
#endif

    uint8_t formfeed = 0x0C;
    pal_sio_tx(SIO_CHANNEL, &formfeed, 1);
    printf("Energy scan\r\n");
    printf("Scan duration = aBaseSuperframeDuration * (2^n + 1) symbols\r\n");

#if (TAL_TYPE == AT86RF212)
    tal_pib_get(phyCurrentPage, &page);
    tal_pib_get(phyCurrentChannel, &channel);
    switch (page)
    {
        case 0: printf("E.g. n=0 -> 96/48ms, n=6 -> 3/1.6s, n=14 -> 13/6.6min; ch0/1-10\r\n"); break;
        case 2: printf("E.g. n=0 -> 77/30ms, n=6 -> 2.5/1s, n=14 -> 10.5/4min; ch0/1-10\r\n"); break;
        case 5: printf("E.g. n=0 -> 30ms, n=6 -> 1s, n=14 -> 4min; ch0/0-3\r\n"); break;
        case 16:
        case 17: printf("E.g. n=0 -> 30ms, n=6 -> 1s, n=14 -> 4min\r\n"); break;
        default: return;
    }
#else   /* 2.4 GHz */
    printf("E.g. n=0 -> 30ms, n=6 -> 1s, n=14 -> 4min\r\n");
#endif
    printf("Enter scan duration (n = 0..14) and press Enter: ");
    for (i = 0; i < 3; i++)
    {
        input = sio_getchar();
        if ((input < '0') || (input > '9'))
        {
            break;
        }
        character_entered = true;
        input_char[i] = input;
    }
    scan_duration = atol(input_char);

    if ((character_entered == false) || (scan_duration > 15))
    {
        printf("\r\ninvalid scan duration\r\n");
        printf("\r\nPress any key to return to main menu.");
        sio_getchar();
        return;
    }

    scanning = true;
    printf("\r\nEnergy scaning (duration n=%d)... Wait until test is completed.\r\n", scan_duration);
    printf("Channel\tEnergy (dBm)\r\n");
    tal_pib_get(phyCurrentChannel, &channel_before_scan);
    tal_pib_get(phyChannelsSupported, (uint8_t *)&scan_channel_mask);
    /* Identify first channel */
    for (i = 0; i <= MAX_CHANNEL; i++)
    {
        if ((scan_channel_mask & ((uint32_t)1 << i)) > 0)
        {
            first_channel = i;
            scan_channel_mask &= ~((uint32_t)1 << i);
            break;
        }
    }
    tal_pib_set(phyCurrentChannel, (pib_value_t *)&first_channel);
    tal_ed_start(scan_duration);
}


/**
 * @brief Start CW transmission on current channel page
 */
#if ((TAL_TYPE != AT86RF230B) || ((TAL_TYPE == AT86RF230B) && (defined CW_SUPPORTED)))
static void start_cw_transmission(void)
{
    uint8_t channel;
    uint8_t input;

    tal_pib_get(phyCurrentChannel, &channel);

    printf("\r\nEnter continuous transmission mode; C = CW mode, P = PRBS mode: ");
    input = sio_getchar();
    input = toupper(input);
    switch (input)
    {
        case 'C':
#if (TAL_TYPE == AT86RF212)
            printf("\r\nContinuous wave (CW) transmission on channel %d (-0.1MHz)\r\n", channel);
#else
            printf("\r\nContinuous wave (CW) transmission on channel %d (-0.5MHz)\r\n", channel);
#endif
            tfa_continuous_tx_start(CW_MODE);
            break;

        case 'P':
            printf("\r\nContinuous wave (PBRS) transmission on channel %d\r\n", channel);
            tfa_continuous_tx_start(PRBS_MODE);
            break;

        default:
            printf("\r\nWrong character entered\r\n");
            return;
    }

    op_mode = CONTINOUS_TX_MODE;
    printf("Press any key to cancel continuous transmission.");
    sio_getchar();
    /* Stop CW transmission again */
    tfa_continuous_tx_stop();
    printf("\r\nTRX has been reset\r\n");
    op_mode = OFF_OP_MODE;
    print_main_menu();
}
#endif


/**
 * @brief Send an energy pulse on current channel page
 */
#if ((TAL_TYPE != AT86RF230B) || ((TAL_TYPE == AT86RF230B) && (defined CW_SUPPORTED)))
static void pulse_cw_transmission(void)
{
    uint8_t channel;

    op_mode = CONTINOUS_TX_MODE;
    tal_pib_get(phyCurrentChannel, &channel);
#if (TAL_TYPE == AT86RF212)
    printf("\r\nContinuous wave (CW) transmission on channel %d (-0.1MHz)\r\n", channel);
#else
    printf("\r\nContinuous wave (CW) transmission on channel %d (-0.5MHz)\r\n", channel);
#endif
    tfa_continuous_tx_start(CW_MODE);
    pal_timer_start(T_APP_TIMER,
                    50000,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR)stop_pulse_cb,
                    NULL);
}
#endif


/**
 * @brief Stop sending a CW signal on current channel page
 */
#if ((TAL_TYPE != AT86RF230B) || ((TAL_TYPE == AT86RF230B) && (defined CW_SUPPORTED)))
static void stop_pulse_cb(void *callback_parameter)
{
    /* Stop CW transmission again */
    tfa_continuous_tx_stop();
    printf("\r\nTRX has been reset\r\n");
    op_mode = OFF_OP_MODE;
    print_main_menu();

    /* Keep compiler happy. */
    callback_parameter = callback_parameter;
}
#endif


/**
 * User call back function for finished ED Scan
 */
void tal_ed_end_cb(uint8_t energy_level)
{
    uint8_t i;
    uint8_t channel;
#if (TAL_TYPE == AT86RF212)
    uint8_t page;
#endif

    /* Print result */
    tal_pib_get(phyCurrentChannel, &channel);

    /* Re-scale ED value to Pin(dBm) */
#if (TAL_TYPE == AT86RF212)
    float reg_val = (float)62/255 * energy_level;
    uint8_t p_in = (uint8_t)(99 - (reg_val * 1.03));
    printf("%d\t-%d  ", channel, p_in);
#elif (TAL_TYPE == ATMEGARF_TAL_1)
    float reg_val = (float)55/255 * energy_level;
    uint8_t p_in = (uint8_t)(90 - reg_val);
    printf("%d\t-%d  ", channel, p_in);
#elif (TAL_TYPE == AT86RF230B) || (TAL_TYPE == AT86RF231) || (TAL_TYPE == AT86RF232)
    float reg_val = (float)56/255 * energy_level;
    uint8_t p_in = (uint8_t)(91 - reg_val);
    printf("%d\t-%d  ", channel, p_in);
#endif

    for (i = 0; i < energy_level / 4; i++)
    {
        printf("X");
    }
    printf("\r\n");

    /* Check for next channel */
    channel = 0xFF;
    for (i = 0; i <= MAX_CHANNEL; i++)
    {
        if ((scan_channel_mask & ((uint32_t)1 << i)) > 0)
        {
            channel = i;
            scan_channel_mask &= ~((uint32_t)1 << i);
            break;
        }
#if (TAL_TYPE == AT86RF212)
        tal_pib_get(phyCurrentPage, &page);
        if (page == 5)
        {
            if (i == 4)
            {
                channel = 0xFF;
            }
        }
#endif

    }

    /* Check if all channels were scanned. */
    if (channel == 0xFF)
    {
        /* Set original channel. */
        tal_pib_set(phyCurrentChannel, (pib_value_t *)&channel_before_scan);
        scanning = false;
        printf("\r\nPress any key to return to main menu.");
        sio_getchar();
    }
    else
    {
        /* Scan next channel */
        tal_pib_set(phyCurrentChannel, (pib_value_t *)&channel);
        tal_ed_start(scan_duration);
    }
}


/**
 * @brief Sub-menu to get channel setting
 */
static void get_channel(void)
{
    char input_char[3]= {0, 0, 0};
    uint8_t i;
    uint8_t input;
    uint8_t channel;
#if (TAL_TYPE == AT86RF212)
    uint8_t ch_page;
#endif

#if (TAL_TYPE == AT86RF212)
    tal_pib_get(phyCurrentPage, &ch_page);
    if (ch_page == 5)
    {
        printf("\r\nEnter channel (0..3) and press 'Enter': ");
    }
    else
    {
        printf("\r\nEnter channel (0..10) and press 'Enter': ");
    }
#else
    printf("\r\nEnter channel (11..26) and press 'Enter': ");
#endif
    for (i = 0; i < 3; i++)
    {
        input = sio_getchar();
        if ((input < '0') || (input > '9'))
        {
            break;
        }
        input_char[i] = input;
    }

    channel = atol(input_char);
    tal_pib_set(phyCurrentChannel, (pib_value_t *)&channel);
}


/**
 * @brief Sub-menu to get channel page setting
 */
static void get_page(void)
{
#if (TAL_TYPE == AT86RF232)
    printf("\r\nOnly channel page 0 supported by AT86RF232.\r\n");
    printf("\r\nPress any key to return to main menu.\r\n");
    sio_getchar();
    return;
#else
    char input_char[3]= {0, 0, 0};
    uint8_t i;
    uint8_t input;
    uint8_t ch_page;

#if (TAL_TYPE == AT86RF212)
    printf("\r\nchannel page\tbrutto data rate (kbit/s)\r\n");
    printf("page\tch 0 or \tch 1-10\r\n");
    printf("0\t20 or \t\t40 (BPSK)\r\n");
    printf("2\t100 or \t\t250 (O-QPSK)\r\n");
    printf("5\t250 (Chinese band, ch 0-3)\r\n");
    printf("16 *)\t200 or \t\t500\r\n");
    printf("17 *)\t400 or \t\t1000\r\n");
    printf("18 *)\t500 (Chinese band, ch 0-3)\r\n");
    printf("19 *)\t1000 (Chinese band, ch 0-3)\r\n");
    printf("*) proprietary channel page\r\n");
    printf("\r\nEnter channel page (0, 2, 5, 16, 17, 18 or 19) and press 'Enter': ");
#endif
#if ((TAL_TYPE == AT86RF231) || (TAL_TYPE == ATMEGARF_TAL_1))
    printf("\r\nchannel page\tbrutto data rate (kbit/s)\r\n");
    printf("0\t\t250\r\n");
    printf("2 *)\t\t500\r\n");
    printf("16 *)\t\t1000\r\n");
    printf("17 *)\t\t2000\r\n");
    printf("*) proprietary channel page\r\n");
    printf("\r\nEnter channel page (0, 2, 16, or 17) and press 'Enter': ");
#endif

    for (i = 0; i < 3; i++)
    {
        input = sio_getchar();
        if ((input < '0') || (input > '9'))
        {
            break;
        }
        input_char[i] = input;
    }

    ch_page = atoi(input_char);
    tal_pib_set(phyCurrentPage, (pib_value_t *)&ch_page);
#endif  /* #if (TAL_TYPE == AT86RF232) */
}


/**
 * @brief Sub-menu to get transmit power value
 */
static void get_tx_pwr(void)
{
    char input_char[4]= {0, 0, 0, 0};
    uint8_t i;
    char input;
    uint8_t temp_var;
    int8_t tx_pwr;

    printf("\r\nEnter transmit power value (in dBm, e.g. \"-5\") and press 'Enter': ");
    for (i = 0; i < 4; i++)
    {
        input = sio_getchar();
        if (((input < '0') || (input > '9')) && (input != '-') && (input != '+'))
        {
            break;
        }
        input_char[i] = input;
    }
    tx_pwr = atoi(input_char);
    temp_var = CONV_DBM_TO_phyTransmitPower(tx_pwr);
    tal_pib_set(phyTransmitPower, (pib_value_t *)&temp_var);
}


/**
 * @brief Sub-menu to get amount of test frames
 */
static void get_number_test_frames(void)
{
    char input_char[10]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t i;
    uint8_t input;

    printf("\r\nEnter number (<= 2^32) of test frame and press 'Enter': ");
    for (i = 0; i < 10; i++)
    {
        input = sio_getchar();
        if ((input < '0') || (input > '9'))
        {
            break;
        }
        input_char[i] = input;
    }

    number_test_frames = atol(input_char);
}


/**
 * @brief Sub-menu to get test frames length
 */
static void get_frame_length(void)
{
    char input_char[4]= {0, 0, 0, 0};
    uint8_t i;
    uint8_t input;
    uint16_t length;

    printf("\r\nEnter length (%d .. 127) of test frame (incl. FCS) and press 'Enter': ", FRAME_OVERHEAD);
    for (i = 0; i < 4; i++)
    {
        input = sio_getchar();
        if ((input < '0') || (input > '9'))
        {
            break;
        }
        input_char[i] = input;
    }

    length = atoi(input_char);

    /*
     * Check for minimum required and maximum allowed IEEE 802.15.4
     * frame length.
     */
    if (length < FRAME_OVERHEAD)
    {
        phy_frame_length = FRAME_OVERHEAD;
    }
    else if (length > aMaxPHYPacketSize)
    {
        phy_frame_length = aMaxPHYPacketSize;
    }
    else
    {
        phy_frame_length = (uint8_t)length;
    }


    /* The FCF has to be udpated. */
    configure_frame_sending();
}


/**
 * @brief Support function to toggle ACK request value
 */
static void toggle_ack_request(void)
{
    if (ack_request)
    {
        ack_request = false;
    }
    else
    {
        ack_request = true;
    }

    /* The FCF has to be udpated. */
    configure_frame_sending();
}


/**
 * @brief Support function to toggle CSMA usage value
 */
static void toggle_csma_enabled(void)
{
    if (csma_enabled)
    {
        csma_enabled = false;
    }
    else
    {
        csma_enabled = true;
    }
}


/**
 * @brief Support function to toggle retry value
 */
static void toggle_retry_enabled(void)
{
    if (retry_enabled)
    {
        retry_enabled = false;
    }
    else
    {
        retry_enabled = true;;
    }
}


#ifdef ANTENNA_DIVERSITY
/**
 * @brief Support function toggling antenna diversity
 */
static void toogle_antenna_diversity(void)
{
    if (antenna_diversity)
    {
        antenna_diversity = false;
        pal_trx_bit_write(SR_ANT_DIV_EN, ANT_DIV_DISABLE);
    }
    else
    {
        antenna_diversity = true;
        pal_trx_bit_write(SR_ANT_DIV_EN, ANT_DIV_ENABLE);
    }
}
#endif


/**
 * @brief Sub-menu to print test result
 */
static void print_result(void)
{
    uint32_t duration;
    float data_volume;
    float duration_s;
    float data_rate;

    if (op_mode == TX_OP_MODE)
    {
        duration = pal_sub_time_us(end_time, start_time);
        printf(" Done.\r\n\r\nTest result:\r\n");
        printf("Test duration = %0.6f s\r\n", (double)((float)duration/1000000));
        printf("Transmitted frames = %" PRIu32 ", invalid frames = %" PRIu32 "\r\n", number_test_frames, frame_failure);

        if (ack_request)
        {
            printf("Frames w/o ACK = %" PRIu32 "\r\n", frame_no_ack);
        }
        if (csma_enabled)
        {
            printf("Channel access failures = %" PRIu32 "\r\n", frame_access_failure);
        }

        data_volume = phy_frame_length * number_test_frames * 8;
        duration_s = (float)duration / 1000000;
        data_rate = data_volume / duration_s / 1000;

        printf("Net data rate = %.2f kbit/s\r\n", (double)data_rate);
        printf("Press any key to continue");
        sio_getchar();
    }
    else // RX_OP_MODE or PROMISCUOUS_OP_MODE
    {
        printf("\r\ndone.\r\n\r\nTest result:\r\n");
        if (number_rx_frames != 0)
        {
            aver_lqi = aver_lqi / number_rx_frames;
        }
        printf("Number of received frames = %" PRIu32 "; average LQI = %d\r\n", number_rx_frames, (uint8_t)aver_lqi);
    }
}


/**
 * @brief Sub-menu to print sensor data
 */
static void get_sensor_data(void)
{
    uint16_t bat_mon;
#if (PAL_GENERIC_TYPE == MEGA_RF)
    double temperature;
#endif

    bat_mon = tfa_get_batmon_voltage();
    printf("\r\nBattery monitor: U = %" PRIu16 ".%" PRIu16 " V\r\n", bat_mon/1000, bat_mon%1000);

#if (PAL_GENERIC_TYPE == MEGA_RF)
    temperature = tfa_get_temperature();
    printf("Temperature: T = %2.1f C\r\n", temperature);
#endif
}

/* EOF */
