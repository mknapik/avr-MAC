/**
 * @file main.c
 *
 * @brief  Implementation of the PAL Demo Application "Simple Remote LED Control".
 *
 * $Id: main.c 22855 2010-08-11 08:20:55Z sschneid $
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
#include "pal.h"
#include "stack_config.h"
#include "app_config.h"
#include "simple_remote_led_control.h"

/* === TYPES =============================================================== */

/* === MACROS ============================================================== */

/* === GLOBALS ============================================================= */

/** Transmit frame counter, used as sequence number in the frames sent. */
static uint8_t tx_cnt = 0;
/** Receive frame counter, which lower 3 bit are displayed on the LEDs. */
static uint8_t rx_cnt = 0;
/** Flag that indicates that a frame transmission is progress.
 * This variable is modified in app_task() and irq_tx_end_handler().
 */
static volatile uint8_t tx_flag = 0;


/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Main function of the PAL Demo Application "Simple Remote LED Control".
 *
 * @ingroup apiPalAppDemo
 */
int main(void)
{
    app_init();

    app_set_led_value(0);
    while(1)
    {
        app_task();
        /*
         * Calling the PAL task is currently not required for this application,
         * since no timer are used and USB is not enabled.
         * The call to pal_task() is inserted anyway. This allows easy extension
         * of this application, for example to use application timers properly.
         */
        pal_task();
    }
}

/**
 * @brief Application initialisation function.
 *
 * This function initializes
 *  - the PAL module including the IRQ handler registering,
 *  - the radio transceiver, and
 *  - the LED and Button interface.
 *
 * The function of the LEDs is described in section @ref secUIAppInit.
 *
 * @ingroup apiPalAppDemo
 */
void app_init(void)
{
    uint8_t trx_state;

    /* Step 0: init MCU peripherals */
    pal_init();
    pal_led_init();
    pal_button_init();
#if (TAL_TYPE == ATMEGARF_TAL_1)
    pal_trx_irq_init_tx_end((FUNC_PTR)irq_tx_end_handler);
    pal_trx_irq_init_rx_end((FUNC_PTR)irq_rx_end_handler);
#elif (TAL_TYPE == AT86RF230B)
    pal_trx_irq_init((FUNC_PTR)irq_trx_end_handler);
#endif

    app_set_led_value(0);

    /* Step 1: reset transceiver */
    pal_timer_delay(880);
    PAL_RST_LOW();
    PAL_SLP_TR_LOW();
    pal_timer_delay(10);
    PAL_RST_HIGH();
    app_set_led_value(1);

    /* Step 2: set trx to TRX_OFF and verify it. */
    pal_trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);
    pal_timer_delay(510);

    do
    {
        trx_state = pal_trx_bit_read(SR_TRX_STATUS);
    } while (trx_state != TRX_OFF);

    app_set_led_value(2);

    /* Step 3: set channel and enbale IRQ handling and crc. */
    pal_trx_bit_write(SR_CHANNEL, DEFAULT_CHANNEL);
    pal_trx_reg_read(RG_IRQ_STATUS);
#if (TAL_TYPE == ATMEGARF_TAL_1)
    pal_trx_reg_write(RG_IRQ_MASK, _BV(TX_END_EN) | _BV(RX_END_EN));
#elif (TAL_TYPE == AT86RF230B)
    pal_trx_bit_write(SR_TX_AUTO_CRC_ON, 1);
    pal_trx_reg_write(RG_IRQ_MASK, TRX_IRQ_TRX_END);
    pal_trx_irq_en();
#endif
    app_set_led_value(3);

    /* Step 4: ready to receive data. */
    trx_set_state(RX_ON);
    pal_global_irq_enable();
    app_set_led_value(4);

}

/**
 * @brief Application task function.
 *
 * This function checks the state of the transmit button and starts a
 * frame transmission if a button press event is detected.
 *
 * At the end, the function performs a busy wait with a duration of the button debounce
 * interval @ref DEBOUNCE_PERIOD.
 *
 * @par LED Access
 *
 * If a frame transmission is initiated LED0 is toggled.
 *
 * @ingroup apiPalAppDemo
 */
void app_task(void)
{
    app_frame_t tx_frame;
    static uint8_t key_press = 0;

    key_press = app_debounce_button();

    if((key_press != 0) && (tx_flag == 0))
    {
        tx_flag = 1;
        pal_led(LED_0, LED_TOGGLE);

        tx_frame.phr = APP_FRAME_SIZE;
        tx_frame.fcf = BLINK_FCF;
        tx_frame.span = BLINK_PANID;
        tx_frame.saddr = BLINK_SRCAD;
        tx_frame.seq = tx_cnt;
        strncpy(tx_frame.data, ID_STRING, sizeof(ID_STRING));

        trx_set_state(PLL_ON);
        pal_trx_reg_write(RG_TRX_STATE, CMD_TX_START);
        pal_trx_frame_write((uint8_t*)&tx_frame, APP_FRAME_SIZE);
        key_press = 0;

    }
    pal_timer_delay(DEBOUNCE_PERIOD);
}

/**
 * @brief Set transceiver state function.
 *
 * This function is used to switch the transceiver in a new state.
 * Before a state change is performed, the command FORCE_TRX_OFF is used,
 * which causes that an ongoing transmit or receive operation is aborted and the requested
 * state is entered immediately.
 *
 * @param state The new transceiver state {TRX_OFF, PLL_ON, RX_ON}.
 *
 * @return This function blocks with a inifinite loop, if the requested state
 *         is not reached within 48µs.
 *
 * @ingroup apiPalAppDemo
 */
void trx_set_state(uint8_t state)
{
    uint8_t cmd = 0, curr_state;

    switch(state)
    {
        case TRX_OFF:
            cmd = CMD_TRX_OFF;
            break;

        case PLL_ON:
            cmd = CMD_PLL_ON;
            break;

        case RX_ON:
            cmd = CMD_RX_ON;
            break;

        default:
            ASSERT(1==0);
            break;
    }

    pal_trx_bit_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    pal_trx_bit_write(SR_TRX_CMD, cmd);
    pal_timer_delay(48);

    do
    {
        curr_state = pal_trx_bit_read(SR_TRX_STATUS);
    } while (curr_state != state);
}



/**
 * @brief Frame read function.
 *
 * @param[out] data A pointer to a memory array, where the frame is stored.
 *              The array needs to hold at maximum 128 bytes.
 * @return length of the received frame.
 *
 * @ingroup apiPalAppDemo
 */
uint8_t trx_frame_read(uint8_t *data)
{
uint8_t phr;
#if (TAL_TYPE == ATMEGARF_TAL_1)
    phr = *data = pal_trx_reg_read(RG_TST_RX_LENGTH);
    pal_trx_frame_read(data + 1,(phr&0x7f));
#elif (TAL_TYPE == AT86RF230B)
    pal_trx_frame_read(&phr,1);
    pal_trx_frame_read(data,(phr&0x7f));
#endif
    return phr;
}

#if (TAL_TYPE == AT86RF230B)
/**
 * @brief Main IRQ handler.
 *
 * This is the IRQ handler function for transceivers with a common shared IRQ line
 * (e.g. all plattforms with AT86RF2xx transceivers).
 * The function determines if the  TX_END or RX_END handler function has to be called
 * by means of the global variable @ref tx_flag.
 *
 * @ingroup apiPalAppDemo
 */
void irq_trx_end_handler(void)
{
    pal_trx_reg_read(RG_IRQ_STATUS);
    if (tx_flag != 0)
    {
        irq_tx_end_handler();
    }
    else
    {
        irq_rx_end_handler();
    }
}
#endif

/**
 * @brief TX_END IRQ handler.
 *
 * This function is executed, if a TX_END interrupt has occured.
 * It switches the transceiver back to state RX_ON, resets the global
 * variable @ref tx_flag and increments @ref tx_cnt.
 *
 * @par LED Access
 *
 * LED0 is toggled to indicate that the frame transmission is complete.
 *
 * @ingroup apiPalAppDemo
 */
void irq_tx_end_handler(void)
{
    pal_led(LED_0, LED_TOGGLE);
    trx_set_state(RX_ON);
    tx_cnt ++;
    tx_flag = 0;
}

/**
 * @brief RX_END IRQ handler.
 *
 * This function is executed, if a RX_END interrupt has occured at the end
 * of a frame reception.
 *
 * @par LED Access
 *
 * At the begin of the function, LED0 is toggled to indicate that a frame was received.
 * The current value of @ref rx_cnt is displayed at the end of the routine on the LEDs.
 * @ingroup apiPalAppDemo
 */
void irq_rx_end_handler(void)
{
    uint8_t rxframe[129], crc;
    app_frame_t *pfrm;

    pal_led(LED_0, LED_TOGGLE);
    crc = pal_trx_bit_read(SR_RX_CRC_VALID);
    if (CRC16_VALID == crc)
    {
        trx_frame_read(rxframe);
        pfrm = (app_frame_t *) rxframe;
        if ((pfrm->span == BLINK_PANID) &&
            (pfrm->saddr == BLINK_SRCAD) )
        {
            rx_cnt ++;
        }
    }
    app_set_led_value(rx_cnt);
}

/* EOF */
