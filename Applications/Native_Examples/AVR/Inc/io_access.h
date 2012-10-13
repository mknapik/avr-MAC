/**
 * @file io_access.h
 *
 * @brief Application header file for io_access.c.
 *
 * Low Level functions for I/O access on:
 *    - Sensor Terminal Board
 *    - Radio Controller Board / deRFtoRCBAdapter
 *    - deRFnode
 *
 * $Id: io_access.h,v 1.1.4.1 2010/09/07 17:39:31 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */


/* Prevent double inclusion */
#ifndef IO_ACCESS_H_
#define IO_ACCESS_H_

/* === Includes ============================================================= */

#include "config.h"     // include configuration

/* === MACROS / DEFINES ==================================================== */

/******************************************************************************
 * GLOBAL DEFINITIONS - VALID FOR ALL PLATFORMS
 ******************************************************************************/

/** All state mode's which available for a LED */
typedef enum led_mode_tag
{
    LED_ON,
    LED_OFF,
    LED_TOGGLE
} led_mode_t;

/** All state mode's which available for a Button */
typedef enum button_state_tag
{
   BUTTON_PRESSED,
   BUTTON_RELEASED
} button_state_t;


/******************************************************************************
 * PLATFORM SPECIFIC DEFINITIONS
 ******************************************************************************/

/******************************************************************************
 * specify external memory definitions
 ******************************************************************************/
#if defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)

/* location of external memory address space for IO's */
#define PERIPHERAL_XRAM_ADDRESS   (0x4000)

#endif // defined(__AVR_ATmega1281__) && (PLATFORM == SENS_TERM_BOARD)

/******************************************************************************
 * specify ports for external memory emulation (necessary on MCUs without XRAM interface)
 ******************************************************************************/

#if (PLATFORM == SENS_TERM_BOARD)

#if !defined(__AVR_ATmega1281__)
/* Port definition to IO data port */
#define IO_DATA_PORT         (PORTB)
#define IO_DATA_DDR          (DDRB)
#define IO_DATA_PINS         (PINB)

/* Port definition to IO address decoding port */
#define IO_DECODE_PORT       (PORTD)
#define IO_DECODE_DDR        (DDRD)
/* ChipSelect bits
 *  CS_usb  = 1 (CS1 = 0, CS0 = 0)
 *  CS_io   = 1 (CS1 = 0, CS0 = 1)
 *  CS_sram = 1 (CS1 = 1, CS0 = 0)
 */
#define IO_CS0_PIN           (PD6)
#define IO_CS1_PIN           (PD7)

/* Port definitions for write and read access to IO's */
#define IO_WR_PIN            (PE4)
#define IO_WR_PORT           (PORTE)
#define IO_WR_DDR            (DDRE)
#define IO_WR_PINS           (PINE)

#define IO_RD_PIN            (PE5)
#define IO_RD_PORT           (PORTE)
#define IO_RD_DDR            (DDRE)
#define IO_RD_PINS           (PINE)

#endif //!defined(__AVR_ATmega1281__)

#endif // (PLATFORM == SENS_TERM_BOARD)


/******************************************************************************
 * specify button and LED definitions for RADIO_CONTROLLER_BOARD
 ******************************************************************************/

#if (PLATFORM == RADIO_CONTROLLER_BOARD)

/* specify all LED's available on this platform */
typedef enum led_number_tag
{
    LED_0,
    LED_1,
    LED_2
} led_number_t;

/*
 * PORT, DDR and PIN where LEDs are connected
 */
#define LED_0_PIN             (PE2)
#define LED_0_PORT            (PORTE)
#define LED_0_DDR             (DDRE)

#define LED_1_PIN             (PE3)
#define LED_1_PORT            (PORTE)
#define LED_1_DDR             (DDRE)

#define LED_2_PIN             (PE4)
#define LED_2_PORT            (PORTE)
#define LED_2_DDR             (DDRE)


/* specify all BUTTON's available on this platform */
typedef enum button_number_tag
{
    BUTTON_0
} button_number_t;

/*
 * PORT, DDR, PIN and INPUTS defined where Button is connected
 */
#define BUTTON_0_PIN          (PE5)
#define BUTTON_0_PORT         (PORTE)
#define BUTTON_0_DDR          (DDRE)
#define BUTTON_0_PINS         (PINE)

#endif // (PLATFORM == RADIO_CONTROLLER_BOARD)

/******************************************************************************
 * specify button and LED definitions for SENS_TERM_BOARD
 ******************************************************************************/

#if (PLATFORM == SENS_TERM_BOARD)

/* specify all LED's available on this platform */
typedef enum led_number_tag
{
    LED_0,
    LED_1
} led_number_t;

/* specify all BUTTON's available on this platform */
typedef enum button_number_tag
{
    BUTTON_0
} button_number_t;

/*
 * PORT, DDR, PIN and INPUTS defined where Button is connected
 */
/* NOTHING HAS TO BE DEFINED; SINCE BUTTON READBACK IS MADE VIA MEMORY INTERFACE */
#define BUTTON_0_PIN          (0) /* Bit position of button pin */
    
#endif //(PLATFORM == SENS_TERM_BOARD)

/******************************************************************************
 * specify button and LED definitions for DE_RF_NODE
 ******************************************************************************/

#if (PLATFORM == DE_RF_NODE)

/* specify all LED's available on this platform */
typedef enum led_number_tag
{
    LED_0,
    LED_1,
    LED_2
} led_number_t;

/*
 * PORT, DDR and PIN where LEDs are connected
 */
#define LED_0_PIN          (PG5)
#define LED_0_PORT         (PORTG)
#define LED_0_DDR          (DDRG)

#define LED_1_PIN          (PE3)
#define LED_1_PORT         (PORTE)
#define LED_1_DDR          (DDRE)

#define LED_2_PIN          (PE4)
#define LED_2_PORT         (PORTE)
#define LED_2_DDR          (DDRE)

/* specify all BUTTON's available on this platform */
typedef enum button_number_tag
{
    BUTTON_0,
    BUTTON_1
} button_number_t;

/*
 * PORT, DDR, PIN and INPUTS defined where Button is connected
 */
#define BUTTON_0_PIN          (PB7)
#define BUTTON_0_PORT         (PORTB)
#define BUTTON_0_DDR          (DDRB)
#define BUTTON_0_PINS         (PINB)

#define BUTTON_1_PIN          (PD6)
#define BUTTON_1_PORT         (PORTD)
#define BUTTON_1_DDR          (DDRD)
#define BUTTON_1_PINS         (PIND)

#endif //(PLATFORM == DE_RF_NODE)


/* === Types ================================================================ */

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


void led_set(led_number_t led_nr, led_mode_t led_mode);
void io_init(void);
button_state_t button_pressed(button_number_t button_nr);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */

#endif /* IO_ACCESS_H_ */

/* EOF */
