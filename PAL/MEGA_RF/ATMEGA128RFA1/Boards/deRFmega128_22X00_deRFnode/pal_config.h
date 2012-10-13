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

#if (BOARD_TYPE == deRFmega128_22X00_deRFnode)
 
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
    BUTTON_0,
    BUTTON_1
} button_id_t;

/** Number of buttons provided by this board. */
#define NO_OF_BUTTONS                   (2)

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
 * PORT/PINs definitions for connected LEDs
 */
#define LED0_PIN                    (PG5)
#define LED0_PORT                   (PORTG)
#define LED0_DDR                    (DDRG)

#define LED1_PIN                    (PE3)
#define LED1_PORT                   (PORTE)
#define LED1_DDR                    (DDRE)

#define LED2_PIN                    (PE4)
#define LED2_PORT                   (PORTE)
#define LED2_DDR                    (DDRE)


/*
 * PORT/PINs definitions for connected Buttons
 */
#define BUTTON0_PIN                 (PB7)
#define BUTTON0_PORT                (PORTB)
#define BUTTON0_DDR                 (DDRB)
#define BUTTON0_PINS                (PINB)

#define BUTTON1_PIN                 (PD6)
#define BUTTON1_PORT                (PORTD)
#define BUTTON1_DDR                 (DDRD)
#define BUTTON1_PINS                (PIND)


#ifdef USB0

/*
 * FTDI based USB macros
 */

/* Port definitions which gives indication of reception of byte */
#define USB_RXF_PIN                 (PE2)
#define USB_RXF_PORT                (PORTE)
#define USB_RXF_DDR                 (DDRE)
#define USB_RXF_PINS                (PINE)

/* Port definitions which gives indication of transmission of byte */
#define USB_TXE_PIN                 (PB5)
#define USB_TXE_PORT                (PORTB)
#define USB_TXE_DDR                 (DDRB)
#define USB_TXE_PINS                (PINB)

/* Port definitions for write and read access on USB */
#define USB_WR_PIN                  (PG1)
#define USB_WR_PORT                 (PORTG)
#define USB_WR_DDR                  (DDRG)
#define USB_WR_PINS                 (PING)

#define USB_RD_PIN                  (PD4)
#define USB_RD_PORT                 (PORTD)
#define USB_RD_DDR                  (DDRD)
#define USB_RD_PINS                 (PIND)

/* Port definitions of data port from USB interface */
#define USB_D0_PIN                  (PB0)
#define USB_D0_PORT                 (PORTB)
#define USB_D0_DDR                  (DDRB)
#define USB_D0_PINS                 (PINB)

#define USB_D1_PIN                  (PF2)
#define USB_D1_PORT                 (PORTF)
#define USB_D1_DDR                  (DDRF)
#define USB_D1_PINS                 (PINF)

#define USB_D2_PIN                  (PD5)
#define USB_D2_PORT                 (PORTD)
#define USB_D2_DDR                  (DDRD)
#define USB_D2_PINS                 (PIND)

#define USB_D3_PIN                  (PG2)
#define USB_D3_PORT                 (PORTG)
#define USB_D3_DDR                  (DDRG)
#define USB_D3_PINS                 (PING)

#define USB_D4_PIN                  (PE6)
#define USB_D4_PORT                 (PORTE)
#define USB_D4_DDR                  (DDRE)
#define USB_D4_PINS                 (PINE)

#define USB_D5_PIN                  (PB4)
#define USB_D5_PORT                 (PORTB)
#define USB_D5_DDR                  (DDRB)
#define USB_D5_PINS                 (PINB)

#define USB_D6_PIN                  (PE7)
#define USB_D6_PORT                 (PORTE)
#define USB_D6_DDR                  (DDRE)
#define USB_D6_PINS                 (PINE)

#define USB_D7_PIN                  (PB6)
#define USB_D7_PORT                 (PORTB)
#define USB_D7_DDR                  (DDRB)
#define USB_D7_PINS                 (PINB)

/*
 * USB0 non-generic (board specific) initialization part.
 * That is not required for this board.
 */
#define USB_INIT_NON_GENERIC()

#endif  /* ifdef USB0 */

/*
 * UART0 and UART1 non-generic (board specific) initialization part.
 * While UART0 is routed to 2x3 connector, UART1 is available via the
 * IO-Header
 */
#define UART_0_INIT_NON_GENERIC()   do {                              \
    /* 8N1 */                                                        \
    UCSR0C = ((0x03<<UCSZ00) | (0x00<<UPM00) | (0x00 << USBS0 ));    \
} while(0)

#define UART_1_INIT_NON_GENERIC()    do {                             \
    /* 8N1 */                                                        \
    UCSR1C = ((0x03<<UCSZ10) | (0x00<<UPM10) | (0x00 << USBS1 ));    \
} while(0)


/*
 * Although the deRFmega128-series includes 4K MCU-internal EEPROM as well
 * as 128K external eeprom available via 2-wire-interface (I2C), both are
 * currently unsupported.
 * The deRFnode itself provides an additional eeprom.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (0)
#endif


/**
 * Alert initialization
 */
#define ALERT_INIT()            do {    \
        LED0_DDR |= (1 << LED0_PIN);    \
        LED1_DDR |= (1 << LED1_PIN);    \
        LED2_DDR |= (1 << LED2_PIN);    \
                                        \
        LED0_PORT |= (1 << LED0_PIN);   \
        LED1_PORT |= (1 << LED1_PIN);   \
        LED2_PORT |= (1 << LED2_PIN);   \
} while (0)

/**
 * Alert indication
 * One loop cycle below is about 6 clock cycles (or more, depending
 * on optimization settings), so it takes 2...3 loop iterations per
 * microsecond.  60000 cycles thus results in 20...30 ms of delay,
 * so the LEDs should be seen as flickering.
 */
#define ALERT_INDICATE()        do {    \
        LED0_PORT ^= (1 << LED0_PIN);   \
        LED1_PORT ^= (1 << LED1_PIN);   \
        LED2_PORT ^= (1 << LED2_PIN);   \
} while (0)


/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* deRFmega128_22X00_deRFnode */

#endif  /* PAL_CONFIG_H */
/* EOF */
