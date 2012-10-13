/**
 * @file pal_config.h
 *
 * @brief PAL configuration for ATxmega128A1
 *
 * This header file contains configuration parameters for ATxmega128A1.
 *
 * $Id: pal_config.h 22250 2010-06-16 13:51:39Z sschneid $
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
#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H

/* === Includes =============================================================*/

#include "pal_boardtypes.h"

#if (BOARD_TYPE == REB_4_1_STK600)

/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types ================================================================*/

/* Enumerations used to identify LEDs */
typedef enum led_id_tag
{
    LED_0,
    LED_1,
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7
} led_id_t;

#define NO_OF_LEDS                      (8)


/* Enumerations used to identify buttons */
typedef enum button_id_tag
{
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_7
} button_id_t;

#define NO_OF_BUTTONS                   (8)

/* === Externals ============================================================*/


/* === Macros ===============================================================*/

/** The default CPU clock */
#ifndef F_CPU
#define F_CPU                   (16000000UL)
#endif

/* Currently only the following system clock frequencies are supported */
#if ((F_CPU != (32000000UL)) && (F_CPU != (16000000UL)) && (F_CPU != (8000000UL)) && (F_CPU != (4000000UL)))
#error "Unsupported F_CPU value"
#endif

/**
 * This board uses Antenna Diversity.
 */
#define ANTENNA_DIVERSITY               (1)

/*
 * IRQ macros for ATxmega128A1
 */

/* Mapping of TRX interrupts to ISR vectors */
#define TRX_MAIN_ISR_VECTOR             (PORTC_INT0_vect)
#define TRX_TSTAMP_ISR_VECTOR           (PORTC_INT1_vect)

/** Enables the transceiver main interrupt. */
#define ENABLE_TRX_IRQ()                (PORTC.INTCTRL |= PORT_INT0LVL_gm)

/** Disables the transceiver main interrupt. */
#define DISABLE_TRX_IRQ()               (PORTC.INTCTRL &= ~PORT_INT0LVL_gm)

/** Clears the transceiver main interrupt. */
#define CLEAR_TRX_IRQ()                 (PORTC.INTFLAGS = PORT_INT0IF_bm)


#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (!defined(ANTENNA_DIVERSITY))
/** Enables the transceiver timestamp interrupt. */
#define ENABLE_TRX_IRQ_TSTAMP()         (PORTC.INTCTRL |= PORT_INT1LVL_gm)

/** Disables the transceiver timestamp interrupt. */
#define DISABLE_TRX_IRQ_TSTAMP()        (PORTC.INTCTRL &= ~PORT_INT1LVL_gm)

/** Clears the transceiver timestamp interrupt. */
#define CLEAR_TRX_IRQ_TSTAMP()          (PORTC.INTFLAGS = PORT_INT1IF_bm)
#endif /* #if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (!defined(ANTENNA_DIVERSITY)) */


/** Enables the global interrupts. */
#define ENABLE_GLOBAL_IRQ()             sei()

/** Disables the global interrupts. */
#define DISABLE_GLOBAL_IRQ()            cli()

/*
 * This macro saves the global interrupt status
 */
#define ENTER_CRITICAL_REGION()         {uint8_t sreg = SREG; cli()

/*
 *  This macro restores the global interrupt status
 */
#define LEAVE_CRITICAL_REGION()         SREG = sreg;}

/*
 * This macro saves the trx interrupt status and disables the trx interrupt.
 */
#define ENTER_TRX_REGION()      { uint8_t irq_mask = PORTC.INTCTRL; PORTC.INTCTRL &= ~PORT_INT0LVL_gm

/*
 *  This macro restores the transceiver interrupt status
 */
#define LEAVE_TRX_REGION()      PORTC.INTCTRL = irq_mask; }


/*
 * GPIO macros for ATxmega128A1
 */

/*
 * This board uses an SPI-attached transceiver.
 */
#define PAL_USE_SPI_TRX                 (1)

/* Actual Ports */
/*
 * The data direction register for the transceiver
 */
#define TRX_PORT1_DDR                   (PORTC.DIR)

/*
 * The transceiver port
 */
#define TRX_PORT1                       (PORTC)

/*
 * RESET pin of transceiver
 */
#define TRX_RST                         (0)

/*
 * Sleep Transceiver pin
 */
#define SLP_TR                          (3)

/*
 * Slave select pin
 */
#define SEL                             (4)

/*
 * SPI Bus Master Output/Slave Input pin
 */
#define MOSI                            (5)

/*
 * SPI Bus Master Input/Slave Output pin
 */
#define MISO                            (6)

/*
 * SPI serial clock pin
 */
#define SCK                             (7)

/*
 * Set TRX GPIO pins.
 */
#define RST_HIGH()                      (TRX_PORT1.OUTSET = _BV(TRX_RST))
#define RST_LOW()                       (TRX_PORT1.OUTCLR = _BV(TRX_RST))
#define SLP_TR_HIGH()                   (TRX_PORT1.OUTSET = _BV(SLP_TR))
#define SLP_TR_LOW()                    (TRX_PORT1.OUTCLR = _BV(SLP_TR))

/*
 * PORT where LEDs are connected
 */
#define LED_PORT                        (PORTE)

/*
 * PINs where LEDs are connected
 */
#define LED_PIN_0                       (0)
#define LED_PIN_1                       (1)
#define LED_PIN_2                       (2)
#define LED_PIN_3                       (3)
#define LED_PIN_4                       (4)
#define LED_PIN_5                       (5)
#define LED_PIN_6                       (6)
#define LED_PIN_7                       (7)

/*
 * PORT where button is connected
 */
#define BUTTON_PORT                     (PORTF)

/*
 * PINs where buttons are connected
 */
#define BUTTON_PIN_0                    (0)
#define BUTTON_PIN_1                    (1)
#define BUTTON_PIN_2                    (2)
#define BUTTON_PIN_3                    (3)
#define BUTTON_PIN_4                    (4)
#define BUTTON_PIN_5                    (5)
#define BUTTON_PIN_6                    (6)
#define BUTTON_PIN_7                    (7)

/*
 * UART 0 port and pin defines
 * here: UART 0 is located at port D, pin 2 and 3
 */
#ifdef UART0
#define UART0_PORT                      PORTD
#define UART0_TX_PIN                    PIN3_bm
#define UART0_RX_PIN                    PIN2_bm
#define UART0_REG                       USARTD0
#define UART0_RX_ISR_VECT               USARTD0_RXC_vect
#define UART0_TX_ISR_VECT               USARTD0_TXC_vect
#endif

/*
 * UART 1 port and pin defines
 * here: UART 1 is located at port D, pin 6 and 7
 */
#ifdef UART1
#define UART1_PORT                      PORTD
#define UART1_TX_PIN                    PIN7_bm
#define UART1_RX_PIN                    PIN6_bm
#define UART1_REG                       USARTD1
#define UART0_RX_ISR_VECT               USARTD1_RXC_vect
#define UART0_TX_ISR_VECT               USARTD1_TXC_vect
#endif

/*
 * Value of an external PA gain.
 * If no external PA is available, the value is 0.
 */
#define EXTERN_PA_GAIN                  (0)

/*
 * Timer macros for ATxmega128A1
 */
#define WAIT_2_NOPS()                   {nop(); nop();}
#define WAIT_4_NOPS()                   {WAIT_2_NOPS(); WAIT_2_NOPS();}
#define WAIT_8_NOPS()                   {WAIT_4_NOPS(); WAIT_4_NOPS();}
#define WAIT_16_NOPS()                  {WAIT_8_NOPS(); WAIT_8_NOPS();}
#define WAIT_32_NOPS()                  {WAIT_16_NOPS(); WAIT_16_NOPS();}


/*
 * These macros are placeholders for delay functions for high speed processors.
 *
 * The following delay are not reasonbly implemented via delay functions,
 * but rather via a certain number of NOP operations.
 * The actual number of NOPs for each delay is fully MCU and frequency
 * dependent, so it needs to be updated for each board configuration.
 */
#if (F_CPU == (32000000UL))
/*
 * ATxmega128A1 @ 32MHz
 */
    /*
     * Wait for 65 ns.
     * time t7: SLP_TR time (see data sheet or SWPM).
     * In case the system clock is 32 MHz we need to have this value filled,
     * otherwise frame transmission may not be initiated properly.
     */
    #define PAL_WAIT_65_NS()                nop()
    /* Wait for 500 ns. */
    #define PAL_WAIT_500_NS()               WAIT_16_NOPS()
    /* Wait for 1 us. */
    #define PAL_WAIT_1_US()                 WAIT_32_NOPS()

#elif (F_CPU == (16000000UL))
/*
 * ATxmega128A1 @ 16MHz
 */
    /*
     * Wait for 65 ns.
     * time t7: SLP_TR time (see data sheet or SWPM).
     * In case the system clock is slower than 32 MHz we do not need
     * to have this value filled.
     */
    #define PAL_WAIT_65_NS()                // empty
    /* Wait for 500 ns. */
    #define PAL_WAIT_500_NS()               WAIT_8_NOPS()
    /* Wait for 1 us. */
    #define PAL_WAIT_1_US()                 WAIT_16_NOPS()

#elif (F_CPU == (8000000UL))
/*
 * ATxmega128A1 @ 8MHz
 */
    /*
     * Wait for 65 ns.
     * time t7: SLP_TR time (see data sheet or SWPM).
     * In case the system clock is slower than 32 MHz we do not need
     * to have this value filled.
     */
    #define PAL_WAIT_65_NS()                // empty
    /* Wait for 500 ns. */
    #define PAL_WAIT_500_NS()               WAIT_4_NOPS()
    /* Wait for 1 us. */
    #define PAL_WAIT_1_US()                 WAIT_8_NOPS()

#elif (F_CPU == (4000000UL))
/*
 * ATxmega128A1 @ 4MHz
 */
    /*
     * Wait for 65 ns.
     * time t7: SLP_TR time (see data sheet or SWPM).
     * In case the system clock is slower than 32 MHz we do not need
     * to have this value filled.
     */
    #define PAL_WAIT_65_NS()                // empty
    /* Wait for 500 ns. */
    #define PAL_WAIT_500_NS()               WAIT_2_NOPS()
    /* Wait for 1 us. */
    #define PAL_WAIT_1_US()                 WAIT_4_NOPS()

#else
    #error Unknown system clock
#endif

/*
 * The smallest timeout in microseconds
 */
#define MIN_TIMEOUT                     (0x80)

/*
 * The largest timeout in microseconds
 */
#define MAX_TIMEOUT                     (0x7FFFFFFF)

/**
 * Minimum time in microseconds, accepted as a delay request
 */
#define MIN_DELAY_VAL                   (5)

/*
 * Timer clock source while radio is awake.
 *
 * T1 & T0:
 * clk source is event channel 0 triggered by system clock (16MHz) with prescaler 16
 * TRX's clock (CLKM) is never used with Xmega, keep macro empty.
 */
#ifdef ANTENNA_DIVERSITY
#define TIMER_SRC_DURING_TRX_AWAKE()
#else
#define TIMER_SRC_DURING_TRX_AWAKE()
#endif

/*
 * Timer clock source while radio is sleeping.
 */
/*
 * T1 & T0:
 * clk source is event channel 0 triggered by system clock with corresponding
 * event system prescaler (see function event_system_init()).
 */
#ifdef ANTENNA_DIVERSITY
#define TIMER_SRC_DURING_TRX_SLEEP() {TCC0_CTRLA = TC0_CLKSEL3_bm;}
#else
#define TIMER_SRC_DURING_TRX_SLEEP() {TCC0_CTRLA = TC0_CLKSEL3_bm; TCC1_CTRLA = TC1_CLKSEL3_bm;}
#endif


/*
 * Maximum numbers of software timers running at a time
 */
#define MAX_NO_OF_TIMERS                (25)
#if (MAX_NO_OF_TIMERS > 255)
#error "MAX_NO_OF_TIMERS must not be larger than 255"
#endif

#ifndef ANTENNA_DIVERSITY
/*
 * Hardware register that holds Rx timestamp
 */
#define TIME_STAMP_REGISTER             (TCC1_CCA)
#endif  /* #ifndef ANTENNA_DIVERSITY */

/*
 * TRX Access macros for ATxmega128A1
 */

/*
 * Bit mask for slave select
 */
#define SS_BIT_MASK                     (1 << SEL)

/*
 * Slave select made low
 */
#define SS_LOW()                        (TRX_PORT1.OUTCLR = SS_BIT_MASK)

/*
 * Slave select made high
 */
#define SS_HIGH()                       (TRX_PORT1.OUTSET = SS_BIT_MASK)

/*
 * Mask for SPIF bit in status register
 */
#define SPIF_MASK                       (SPI_IF_bm)

/*
 * SPI Data Register
 */
#define SPI_DATA_REG                    (SPIC.DATA)

/*
 * Wait for SPI interrupt flag
 */
#define SPI_WAIT()                      do {                        \
    while ((SPIC.STATUS & SPIF_MASK) == 0) { ; }                    \
} while (0)

/*
 * Dummy value written in SPDR to retrieve data form it
 */
#define SPI_DUMMY_VALUE                 (0x00)

/*
 * TRX Initialization
 */
#if (F_CPU == (32000000UL))
    #define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 4MHz. */                                \
        /* CLK2X = 1; PRESCALER = 1 (clkPER/8) */                       \
        /* Resulting SPI speed: 4MHz */                                 \
        SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        SPIC.CTRL |= (1 << SPI_CLK2X_bp) | (1 << SPI_PRESCALER0_bp);    \
        /* Set SEL pin to high */                                       \
        TRX_PORT1.OUTSET = _BV(SEL);                                    \
    } while (0)

#elif (F_CPU == (16000000UL))
    #define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 4MHz. */                                \
        /* CLK2X = 0; PRESCALER = 0 (clkPER/4) */                       \
        SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        /* Set SEL pin to high */                                       \
        TRX_PORT1.OUTSET = _BV(SEL);                                    \
    } while (0)

#elif (F_CPU == (8000000UL))
    #define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 4MHz. */                                \
        /* CLK2X = 1; PRESCALER = 0 (clkPER/2) */                       \
        SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        SPIC.CTRL |= (1 << SPI_CLK2X_bp);                               \
        /* Set SEL pin to high */                                       \
        TRX_PORT1.OUTSET = _BV(SEL);                                    \
    } while (0)

#elif (F_CPU == (4000000UL))
    #define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 2MHz. */                                \
        /* CLK2X = 1; PRESCALER = 0 (clkPER/2) */                       \
        SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        SPIC.CTRL |= (1 << SPI_CLK2X_bp);                               \
        /* Set SEL pin to high */                                       \
        TRX_PORT1.OUTSET = _BV(SEL);                                    \
    } while (0)

#else
    #error Unknown system clock
#endif


/*
 * This board has an external eeprom that stores the IEEE address
 * and other information.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (1)
#endif

/*
 * This board has an external eeprom that stores the IEEE address
 * and other information.
 */
#define EXTERN_EEPROM_AVAILABLE            (1)

/*
 * Storage location for crystal trim value - within external EEPROM
 */
#define EE_XTAL_TRIM_ADDR                  (21)

/*
 * Alert initialization
 */
#define ALERT_INIT()                    do {    \
        LED_PORT.OUT    = 0;                    \
        LED_PORT.DIRSET = 0xFF;                 \
} while (0)

/*
 * Alert indication
 */
#define ALERT_INDICATE()                (LED_PORT.OUTTGL = 0xFF)



/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REB_4_1_STK600 */

#endif  /* PAL_CONFIG_H */
/* EOF */
