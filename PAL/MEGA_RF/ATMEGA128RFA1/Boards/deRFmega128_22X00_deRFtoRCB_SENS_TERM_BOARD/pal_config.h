/**
 * @file pal_config.h
 *
 * @brief PAL configuration for ATmega128RFA1
 *
 * This header file contains configuration parameters for ATmega128RFA1.
 *
 * $Id: pal_config.h,v 1.1.6.1 2010/09/07 17:38:26 dam Exp $
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

#if (BOARD_TYPE == deRFmega128_22X00_deRFtoRCB_SENS_TERM_BOARD)

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
    LED_1
} led_id_t;

#define NO_OF_LEDS                      (2)


/* Enumerations used to idenfify buttons */
typedef enum button_id_tag
{
    BUTTON_0
} button_id_t;

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
#define LED_PORT                        (PORTB)
#define LED_PORT_DIR                    (DDRB)

/* LED address decoding port output register */
#define LED_ADDR_DEC_PORT               (PORTD)

/* LED address decoding port direction register */
#define LED_ADDR_DEC_DDR                (DDRD)

/*
 * Bit numbers address where LEDs are mapped to.
 */
#define LED_BIT_0                       (0)
#define LED_BIT_1                       (1)

/*
 * LED bit mask
 */
#define LED_BIT_MASK                    ((1 << LED_BIT_0) | (1 << LED_BIT_1))

/*
 * PORT where button is connected
 */
#define BUTTON_PORT                     (PORTB)
#define BUTTON_PORT_DIR                 (DDRB)
#define BUTTON_INPUT_PINS               (PINB)

/*
 * PIN where button is connected
 */
#define BUTTON_PIN_0                    (PB0)

/* Button address decoding port output register */
#define BUTTON_ADDR_DEC_PORT            (PORTD)

/* Button address decoding port direction register */
#define BUTTON_ADDR_DEC_DDR             (DDRD)


#ifdef USB0

/*
 * FTDI based USB macros
 */

/* USB specific port macros */

/* USB control port output register */
#define USB_CTRL_PORT                   (PORTE)

/* USB control port direction register */
#define USB_CTRL_DDR                    (DDRE)

/* USB control port input register */
#define USB_CTRL_PIN                    (PINE)

/* Port pin which gives indication of reception of byte */
#define USB_RXF                         (0x80)  /* PE7*/

/* Port pin which gives indication of transmission of byte */
#define USB_TXE                         (0x40)  /* PE6*/

/* Port pin which is used to signal a read access from the FT245 */
#define USB_RD                          (0x20)  /* PE5 */

/* Port pin which is used to signal a write access to the FT245 */
#define USB_WR                          (0x10)  /* PE4 */

/* USB data port output register */
#define USB_DATA_PORT                   (PORTB)

/* USB data port direction register */
#define USB_DATA_DDR                    (DDRB)

/* USB data port input register */
#define USB_DATA_PIN                    (PINB)

/* Memory address mapped to USB FIFO */
#define USB_FIFO_AD                     (0x2200)

/* Port where the latch's ALE signal is connected to */
#define USB_ALE_PORT                    (PORTG)

/* Data direction register used to activate the ALE signal */
#define USB_ALE_DDR                     (DDRG)

/* Pin where the latch's ALE signal is connected to */
#define USB_ALE_PIN                     (0x04)  /* PG2 */

/* USB address decoding port output register */
#define USB_ADDR_DEC_PORT               (PORTD)

/* USB address decoding port direction register */
#define USB_ADDR_DEC_DDR                (DDRD)

/*
 * USB0 non-generic (board specific) initialization part.
 * If this is required, the following macro is filled with the proper
 * implementation.
 */
/* Enable USB address decoding. */
#define USB_INIT_NON_GENERIC()      do { \
    USB_ADDR_DEC_PORT &= ~_BV(6);        \
    USB_ADDR_DEC_DDR |= _BV(6);          \
    USB_ADDR_DEC_PORT &= ~_BV(7);        \
    USB_ADDR_DEC_DDR |= _BV(7);          \
    } while (0)

#endif  /* ifdef USB0 */


/*
 * UART0 and UART1 non-generic (board specific) initialization part.
 * UART0 is available via the 3x2 header on the deRFtoRCB adapter,
 * UART1 via the screw terminals on the STB.
 */
#define UART_0_INIT_NON_GENERIC()   do {                             \
    /* 8N1 */                                                        \
    UCSR0C = ((0x03<<UCSZ00) | (0x00<<UPM00) | (0x00 << USBS0 ));    \
} while(0)

#define UART_1_INIT_NON_GENERIC()   do {                             \
    /* 8N1 */                                                        \
    UCSR1C = ((0x03<<UCSZ10) | (0x00<<UPM10) | (0x00 << USBS1 ));    \
} while(0)


/*
 * Although the deRFmega128-series includes 4K MCU-internal EEPROM as well
 * as 128K external eeprom available via 2-wire-interface (I2C), both are
 * currently unsupported.
 * The Sensor-Terminal Board itself provides no additional eeprom.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (0)
#endif


/*
 * Alert initialization
 */
#define ALERT_INIT()                    (pal_led_init())


/*
 * Alert indication
 */
#define ALERT_INDICATE()                do {    \
        pal_led(LED_0, LED_TOGGLE);             \
        pal_led(LED_1, LED_TOGGLE);             \
} while (0)


/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* deRFmega128_22X00_deRFtoRCB_SENS_TERM_BOARD */

#endif  /* PAL_CONFIG_H */
/* EOF */
