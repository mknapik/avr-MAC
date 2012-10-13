/**
 * @file pal_config.h
 *
 * @brief PAL configuration for ATmega128RFA1
 *
 * This header file contains configuration parameters for ATmega128RFA1.
 *
 * @author
 *      dresden elektronik: http://www.dresden-elektronik.de
 *      Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 * Licensed under dresden elektronik's Limited License Agreement --> deEULA.txt
 */


/* Prevent double inclusion */
#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H

/* === Includes =============================================================*/

#include "pal_boardtypes.h"

#if (BOARD_TYPE == deRFmega128_22X00_deRFtoRCB)
 
/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types ================================================================*/

/* Enumerations used to idenfify LEDs */
typedef enum led_id_tag
{
    LED_0,
    LED_1,
    LED_2
} led_id_t;

#define NO_OF_LEDS                      (3)

/* Enumerations used to idenfify buttons */
typedef enum button_id_tag
{
    BUTTON_0
} button_id_t;

/** Number of buttons provided by this board. */
#define NO_OF_BUTTONS                   (1)

/* === Externals ============================================================*/


/* === Macros ===============================================================*/

/*
 * The default CPU clock
 *
 * The board must be configured for 16 MHz clock, which can be fused
 * for either the 16 MHz internal RC oscillator, or for the 16 MHz
 * crystal oscillator that is required for the transceiver operation.
 */
#define F_CPU                           (16000000UL)


/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is slower than 32 MHz we do not need
 * to have this value filled.
 */
#define PAL_WAIT_65_NS()                // empty


/*
 * PORT where LEDs are connected
 */
#define LED_PORT                        (PORTE)
#define LED_PORT_DIR                    (DDRE)

/*
 * PINs where LEDs are connected
 */
#define LED_PIN_0                       (PE2)
#define LED_PIN_1                       (PE3)
#define LED_PIN_2                       (PE4)

/*
 * PORT where button is connected
 */
#define BUTTON_PORT                     (PORTE)
#define BUTTON_PORT_DIR                 (DDRE)
#define BUTTON_INPUT_PINS               (PINE)

/*
 * PIN where button is connected
 */
#define BUTTON_PIN_0                    (PE5)

/*
 * UART0 and UART1 non-generic (board specific) initialization part.
 * While UART0 is routed to 2x3 connector, UART1 is not available.
 */
#define UART_0_INIT_NON_GENERIC()   do {                             \
    /* 8N1 */                                                        \
    UCSR0C = ((0x03<<UCSZ00) | (0x00<<UPM00) | (0x00 << USBS0 ));    \
} while(0)

#define UART_1_INIT_NON_GENERIC()    do {                            \
    /* nothing to do */                                              \
} while(0)


/*
 * Although the deRFmega128-series includes 4K MCU-internal EEPROM as well
 * as 128K external eeprom available via 2-wire-interface (I2C), both are
 * currently unsupported.
 * The deRFtoRCB adapter itself provides no additional eeprom.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (0)
#endif


/**
 * Alert initialization
 */
#define ALERT_INIT()                    do {    \
        LED_PORT = 0;                           \
        LED_PORT_DIR = 0xFF;                    \
} while (0)

/**
 * Alert indication
 * One loop cycle below is about 6 clock cycles (or more, depending
 * on optimization settings), so it takes 2...3 loop iterations per
 * microsecond.  60000 cycles thus results in 20...30 ms of delay,
 * so the LEDs should be seen as flickering.
 */
#define ALERT_INDICATE()                do {    \
        LED_PORT ^= 0xFF;                       \
} while (0)


/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* deRFmega128_22X00_deRFtoRCB */

#endif  /* PAL_CONFIG_H */
/* EOF */
