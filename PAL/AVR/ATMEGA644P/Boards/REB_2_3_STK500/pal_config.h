/**
 * @file pal_config.h
 *
 * @brief PAL configuration for ATmega644P
 *
 * This header file contains configuration parameters for ATmega644P.
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

#if (BOARD_TYPE == REB_2_3_STK500)

/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types ================================================================*/

typedef enum port_type_tag
{
    PORT_A,
    PORT_B,
    PORT_C,
    PORT_D
} port_type_t;

typedef enum port_pin_direction_type_tag
{
    DDR_A,
    DDR_B,
    DDR_C,
    DDR_D
} port_pin_direction_type_t;

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
    BUTTON_1
} button_id_t;

#define NO_OF_BUTTONS                   (2)

/* === Externals ============================================================*/


/* === Macros ===============================================================*/

/** The default CPU clock */
#define F_CPU                   (8000000UL)

/*
 * IAR compiler does not include the same register defines as avr-gcc.
 * Provide IAR defines until fixed by IAR Systems.
 */
#ifdef __ICCAVR__
#define SPE   6//SPE0
#define MSTR  4//MSTR0
#define SPI2X 0//SPI2X0
#define SPR0  0
#define SPR1  1
#define SPIF  7//SPIF0
#endif  /* __ICCAVR__ */


/*
 * IRQ macros for ATmega644P
 */

/** Mapping of TRX IRQ to MCU pins. */
#define TRX_IRQ                         (TIMER1_CAPT_vect)


/** Enables the transceiver main interrupt. */
#define ENABLE_TRX_IRQ()                (TIMSK1 |= _BV(ICIE1))

/** Disables the transceiver main interrupt. */
#define DISABLE_TRX_IRQ()               (TIMSK1 &= ~(_BV(ICIE1)))

/** Clears the transceiver main interrupt. */
#define CLEAR_TRX_IRQ()                 (TIFR1 = _BV(ICF1))


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
#define ENTER_TRX_REGION()      { uint8_t irq_mask = TIMSK1; TIMSK1 &= ~(_BV(ICIE1))

/*
 *  This macro restores the transceiver interrupt status
 */
#define LEAVE_TRX_REGION()       TIMSK1 = irq_mask; }


/*
 * GPIO macros for ATmega644P
 */

/*
 * This board uses an SPI-attached transceiver.
 */
#define PAL_USE_SPI_TRX                 (1)

/* Port Identifier */
/*
 * PORTB & PORTD are transceiver ports
 */
#define TRX_PORT1_ID                    (PORT_B)

/* Actual Ports */
/*
 * PortB DDR and PortD DDR are data direction registers for the transceiver
 */
#define TRX_PORT1_DDR                   (DDRB)
#define TRX_PORT2_DDR                   (DDRD)

/*
 * PORTB and PORTD are transceiver ports
 */
#define TRX_PORT1                       (PORTB)
#define TRX_PORT2                       (PORTD)

/*
 * Slave select pin is PORTB 5
 */
#define SEL                             (PB4)
#define SEL_PORT                        (TRX_PORT1_ID)

/*
 * SPI serial clock pin is PORTB 7
 */
#define SCK                             (PB7)
#define SCK_PORT                        (TRX_PORT1_ID)

/*
 * SPI Bus Master Output/Slave Input pin is PORTB 5
 */
#define MOSI                            (PB5)
#define MOSI_PORT                       (TRX_PORT1_ID)

/*
 * SPI Bus Master Input/Slave Output pin is PORTB 6
 */
#define MISO                            (PB6)
#define MISO_PORT                       (TRX_PORT1_ID)

/*
 * Sleep Transceiver pin is PORTD 7
 */
#define SLP_TR                          (PD7)

/*
 * RESET pin is PORTB 2
 */
#define RST                             (PB2)
#define RST_PORT                        (TRX_PORT1_ID)

/*
 * Set TRX GPIO pins.
 */
#define RST_HIGH()                      (TRX_PORT1 |= _BV(RST))
#define RST_LOW()                       (TRX_PORT1 &= ~_BV(RST))
#define SLP_TR_HIGH()                   (TRX_PORT2 |= _BV(SLP_TR))
#define SLP_TR_LOW()                    (TRX_PORT2 &= ~_BV(SLP_TR))

/*
 * PORT where LEDs are connected
 */
#define LED_PORT                        (PORTA)
#define LED_PORT_DIR                    (DDRA)

/*
 * PINs where LEDs are connected
 */
#define LED_PIN_0                       (PA0)
#define LED_PIN_1                       (PA1)
#define LED_PIN_2                       (PA2)
#define LED_PIN_3                       (PA3)
#define LED_PIN_4                       (PA4)
#define LED_PIN_5                       (PA5)
#define LED_PIN_6                       (PA6)
#define LED_PIN_7                       (PA7)

/*
 * PORT where button is connected
 */
#define BUTTON_PORT                     (PORTD)
#define BUTTON_PORT_DIR                 (DDRD)
#define BUTTON_INPUT_PINS               (PIND)

/*
 * PINs where buttons are connected
 */
#define BUTTON_PIN_0                    (PD4)
#define BUTTON_PIN_1                    (PD5)

/**
 * Value of an external PA gain.
 * If no external PA is available, the value is 0.
 */
#define EXTERN_PA_GAIN                  (0)

/*
 * Timer macros for ATmega644P
 */

/*
 * This value is used by the calibration routine. At the end of a calibration
 * cycle this value is compared with the counter result.
 *
 * Reference clock: CLKM 1MHz from TRX
 * F_CPU (8MHz) is used with prescaler (32)
 * TARGETVAL_CALIBRATION: How many CPU clocks are required (using a prescaler value of 32)
 * to achieve an overflow of the timer using the reference clock of 1MHz with
 * 8-bit width.
 *
 * TARGETVAL_CALIBRATION * F_CPU/32 = 1MHz * 256
 */
#define TARGETVAL_CALIBRATION           ((1000000ULL * 256 * 32) / F_CPU)

/** This value is the loop counter for calibration routine. */
#define MAX_CAL_LOOP_CNT                (100)

/*
 * These macros are placeholders for delay functions for high speed processors.
 *
 * The following delay are not reasonbly implemented via delay functions,
 * but rather via a certain number of NOP operations.
 * The actual number of NOPs for each delay is fully MCU and frequency
 * dependent, so it needs to be updated for each board configuration.
 *
 * ATmega644P @ 8MHz
 */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is slower than 32 MHz we do not need
 * to have this value filled.
 */
#define PAL_WAIT_65_NS()                // empty

/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               {nop(); nop(); nop(); nop(); }

/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 {nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();}

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
 */
#define TIMER_SRC_DURING_TRX_AWAKE() \
        (TCCR1B = ((1 << ICES1) | (1 << CS12) | (1 << CS11) | (1 << CS10)))

/*
 * Timer clock source while radio is sleeping.
 */
#define TIMER_SRC_DURING_TRX_SLEEP() \
        (TCCR1B = (1 << ICES1) | (1 << CS11))

/*
 * Maximum numbers of software timers running at a time
 */
#define MAX_NO_OF_TIMERS (25)
#if (MAX_NO_OF_TIMERS > 255)
#error "MAX_NO_OF_TIMERS must not be larger than 255"
#endif

/*
 * Hardware register that holds Rx timestamp
 */
#define TIME_STAMP_REGISTER             (ICR1)


/*
 * TRX Access macros for ATmega644P
 */

/*
 * Bit mask for slave select
 */
#define SS_BIT_MASK                     (1 << SEL)

/*
 * Slave select made low
 */
#define SS_LOW()                        ((TRX_PORT1) &= (~SS_BIT_MASK))

/*
 * Slave select made high
 */
#define SS_HIGH()                       ((TRX_PORT1) |= (SS_BIT_MASK))

/*
 * Mask for SPIF bit in SPSR register
 */
#define SPIF_MASK                       (1 << SPIF)

/*
 * Wait for SPI interrupt flag
 */
#define SPI_WAIT()                      while (!(SPSR & SPIF_MASK))

/*
 * Dummy value written in SPDR to retrieve data form it
 */
#define SPI_DUMMY_VALUE                 (0x00)

/*
 * TRX Initialization
 */
#define TRX_INIT()                      do {                        \
    /* Enable the SPI and make the microcontroller as SPI master */ \
    SPCR = _BV(SPE) | _BV(MSTR);                                    \
    SPSR = _BV(SPI2X); /* Double the SPI clock frequency */         \
    /* Select the serial clock SCK to be (FOSC/4)                   \
     * For Example for ATmega644P, FOSC = 8Mhz                      \
     * (Microcontroller operating frequency)                        \
     * SPI clock is thus set to (8/4)*2 = 4 Mhz                     \
     */                                                             \
    SPCR &= ~(_BV(SPR0) | _BV(SPR1));                               \
                                                                    \
    /* Set SEL pin which is on PortB to high */                     \
    TRX_PORT1 |= _BV(SEL);                                          \
} while (0)



/*
 * UART0 and UART1 non-generic (board specific) initialization part.
 */
/* Data Length is 8 bit */
#define UART_0_INIT_NON_GENERIC()   (UCSR0C = (1 << UCSZ01) | (1 << UCSZ00))
#define UART_1_INIT_NON_GENERIC()   (UCSR1C = (1 << UCSZ11) | (1 << UCSZ10))


/**
 * This board does NOT have an external eeprom available.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (0)
#endif

/*
 * Alert initialization
 */
#define ALERT_INIT()                    do {    \
        PORTA = 0;                              \
        DDRA = 0xFF;                            \
} while (0)

/*
 * Alert indication
 */
#define ALERT_INDICATE()                (PORTA = (uint8_t)(~(uint16_t)(PORTA)))


/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REB_2_3_STK500 */

#endif  /* PAL_CONFIG_H */
/* EOF */
