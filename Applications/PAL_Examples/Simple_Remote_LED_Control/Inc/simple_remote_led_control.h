/**
 * @file simple_remote_led_control.h
 *
 * @brief Application header file for PAL Demo Application "Simple Remote LED Control".
 *
 * $Id: simple_remote_led_control.h 22832 2010-08-10 08:25:45Z V_Prasad.Anjangi $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef SIMPLE_REMOTE_LED_CONTROL_H
#define SIMPLE_REMOTE_LED_CONTROL_H

/* === Includes ============================================================= */

#include "tal_types.h"

#if (TAL_TYPE == ATMEGARF_TAL_1)
# include "atmega128rfa1.h"
  /** Platform identifier string. */
# define ID_STRING "ATmega128RFA1"
#elif (TAL_TYPE == AT86RF230B)
# include "at86rf230b.h"
# define ID_STRING "AT86RF230B"
#elif (TAL_TYPE == AT86RF231)
# include "at86rf231.h"
# define ID_STRING "AT86RF231"
#endif

/* === Macros =============================================================== */

/** Default 802.15.4 frame control field, used for the frames sent by the application.
 @code
   Frame Control Field: Data (0x8001)
        .... .... .... .001 = Frame Type: Data (0x0001)
        .... .... .... 0... = Security Enabled: False
        .... .... ...0 .... = Frame Pending: False
        .... .... ..0. .... = Acknowledge Request: False
        .... .... .0.. .... = Intra-PAN: False
        .... 00.. .... .... = Destination Addressing Mode: None (0x0000)
        ..00 .... .... .... = Frame Version: 0
        10.. .... .... .... = Source Addressing Mode: Short/16-bit (0x0002)
 @endcode
*/
#define BLINK_FCF   (0x8001)

/** The applications default PAN identifier used in the address field of the frames. */
#define BLINK_PANID (0x4c5a)

/** The applications default short source address used in the address field of the frames. */
#define BLINK_SRCAD (0x4b4e)

#ifndef DEFAULT_CHANNEL
/** The radio channel used by the application.
 * This macro can be overwritten by -DDEFAULT_CHANNEL=XX at
 * the compiler command line.
 */
# define DEFAULT_CHANNEL         (20)
#endif

/** Size of the application frame. */
#define APP_FRAME_SIZE (sizeof(app_frame_t))

#ifndef ASSERT
  /** ASSERT macro, which blocks if the expression "expr" is not true. */
# define ASSERT(expr) do{while(!expr);}while(0);
#endif

/** Interval for button debounce intervall in µs. */
#define DEBOUNCE_PERIOD (10000)

/** Number of necessarry consecutive "button pressed" detections, before a button event is signalled. */
#define KEY_UP_COUNT (8)

/* === Types ================================================================ */

/** Structure of the frames sent by the application. */
#if (PAL_GENERIC_TYPE == AVR32)
#pragma pack(1)
#endif
typedef struct
{
    uint8_t  phr;   /**< PHY header (frame length field). */
    uint16_t fcf;   /**< Frame control field (see @ref BLINK_FCF). */
    uint8_t  seq;   /**< Frame sequence number. */
    uint16_t span;  /**< source PAN identifier (see @ref BLINK_PANID). */
    uint16_t saddr; /**< source address (see @ref BLINK_SRCAD). */
    char  data[sizeof(ID_STRING)]; /**< Frame payload (see @ref ID_STRING). */
    uint16_t crc;   /**< CRC16 field of the frame.  */
} app_frame_t;

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

int main(void);
void app_init(void);
void app_task(void);
void trx_set_state(uint8_t state);
uint8_t trx_frame_read(uint8_t *data);
void irq_trx_end_handler(void);
void irq_tx_end_handler(void);
void irq_rx_end_handler(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */

/**
 * @brief Button debounce routine.
 *
 * Helper function for debouncing the transmit button.
 * @return 1 if a button event is detected, 0 otherwise.
 *
 * @ingroup apiPalAppDemo
 */
static inline uint8_t app_debounce_button(void)
{
    uint8_t ret = 0;
    static uint8_t keycnt;

    if(BUTTON_PRESSED == pal_button_read(BUTTON_0))
    {
        keycnt ++;
        if (keycnt > KEY_UP_COUNT)
        {
            ret = 1;
            keycnt = 0;
        }
    }
    else
    {
        keycnt = 0;
    }
    return ret;
}



/**
 * @brief Display of a decimal value on the available LEDs.
 *
 * @param v value to be displayed decimal on the LEDs.
 *          The 3 lower significant bits of the value are displayed.
 *
 * @ingroup apiPalAppDemo
 */
static void app_set_led_value(uint8_t v)
{
     pal_led(LED_0, (v & 1) ? LED_ON : LED_OFF );
     pal_led(LED_1, (v & 2) ? LED_ON : LED_OFF );
     pal_led(LED_2, (v & 4) ? LED_ON : LED_OFF );
}

#endif /* SIMPLE_REMOTE_LED_CONTROL_H */

/* EOF */
