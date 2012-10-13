/**
 * @file PAL/AVR/ATMEGA1281/Boards/RCB_3_2_PLAIN/pal_config.h
 *
 * @brief PAL configuration for RCB_3_2_PLAIN with ATmega1281 and AT86RF230B
 *
 * This header file contains configuration parameters for the board
 * RCB_3_2_PLAIN with ATmega1281 and AT86RF230B
 *
 * $Id: pal_config.h 22918 2010-08-13 10:30:50Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/**
 * @defgroup grpPal_AT86RF230B_ATMEGA1281_RCB_3_2_PLAIN \
 *           (Plain) Radio Controller board RCB230 V3.2 with ATmega1281 and AT86RF230
 *
 * @ingroup grpPlatforms
 *
 * @section secIntroduction Introduction
 *
 * This section describes the
 * Board Configuration for
 * RCB_3_2_PLAIN with ATmega1281 and AT86RF230B.
 *
 * The board RCB_3_2_PLAIN with ATmega1281 and AT86RF230B provides an
 * IEEE 802.15.4 dual chip solution for the 2.4 GHz band.
 *
 * For more information about this board see the
 * <a href="../../../User_Guide/AVR2025_User_Guide.pdf">AVR2025 MAC User Guide</a>.
 *
 * For more information about the complete API for all software layers
 * including PAL see the
 * <a href="../../MAC/html/index.html">AVR2025: IEEE 802.15.4-2006 MAC Reference Manual</a>.
 *
 * @section secTransceiverMCUInterface Transceiver-MCU interface
 *
 * The transceiver interfaces the MCU via SPI and a number of GPIOs.
 * This is shown in the picture below:
 *
<pre>
   ------------------+     +----------------
       ATmega1281    |     |    AT86RF230
                     | SPI |
       +----------------------------+
          PB0 (SS)   | --> | @ref SEL
          PB1 (SCK)  | --> | @ref SCK
          PB2 (MOSI) | --> | @ref MOSI
          PB3 (MISO) | <-- | @ref MISO
       +----------------------------+
                     |     |
          PB4        | --> | @ref SLP_TR
          PB5        | --> | @ref RST
                     |     |
          PD4 (ICP1) | <-- | IRQ (see @ref TRX_IRQ and @ref secIRQGeneration)
                     |     |
          PD6 (T1)   | <-- | CLKM (see @ref secTimerSource)
   ------------------+     +----------------
</pre>
 *
 *
 * @section secFCPU Default clock speed (F_CPU)
 *
 * The default clock speed is defined by @ref F_CPU and is currently 8MHz.
 * The CPU clock source is selected in mcu_clock_init() in file pal.c.
 *
 *
 * @section secIRQGeneration IRQ generation
 *
 * The AT86RF230 provides only one transceiver interrupt.
 * The IRQ pin from AT86RF230 is connected to the ICP1 pin of ATmega1281.
 * The ICP1 pin acts as an input capture pin for Timer/Counter1 and is interrupt capable.
 * The corresponding interrupt vector is defined in @ref TRX_IRQ in pal_config.h
 * and used in the ISR for the transceiver main interrupt (@ref TRX_IRQ_vect)
 * in the corresponding file pal_irq.c.
 *
 *
 * @section secTimerUsage Timer usage
 *
 * This board currently uses Timer 1 as system clock and hardware timer.
 * Additionally Timer 2 is used during RC oscillator calibration
 * (see function pal_calibrate_rc_osc() in the corresponding file pal_board.c).
 *
 *
 * @section secTimerSource Timer source
 *
 * The AVR2025 MAC requires a 1MHz timer clock source, since the MAC timers
 * implemented in pal_timer.c provide a timer resolution of 1us.
 *
 * - Timer source during periods where the transceiver is active:
 * <BR><BR>
 * The CLKM from AT86RF230 is connected to T1 pin of ATmega1281 thus acting as
 * Timer/Counter1 Clock Input (i.e. clock source) during time
 * periods, where the transceiver is awake (see macro @ref TIMER_SRC_DURING_TRX_AWAKE).
 * The clock speed of the timer is 1MHz, since the transceiver clock is used directly.
 *
 * - Timer source during periods where the transceiver is sleeping:
 * <BR><BR>
 * During periods where the transceiver is sleeping (and thus the CLKM is not
 * available), the timer source is switched to the internal RC oscillator
 * (see macro @ref TIMER_SRC_DURING_TRX_SLEEP).
 * The clock speed of the timer is also 1MHz, since the timer clock is derived from the
 * current value of the system clock (@ref F_CPU, default is 8MHz) and the corresponding
 * prescaler.
 *
 *
 * @section secTimestamping Timestamping
 *
 * Whenever an interrupt is asserted by the transceiver, the current
 * timestamp is automatically latched into the timestamp register, which is defined by
 * @ref TIME_STAMP_REGISTER.
 * For this board the timestamp register is the Input Capture Register of Timer 1 (ICR1).
 * <BR>
 * The function for reading the current timestamp (pal_trx_read_timestamp())
 * is located in the corresponding file pal_board.c.
 *
 *
 * @section secLEDs LEDs
 *
 * This board provides 3 LEDs that are connected to pins 2-4 of port E.
 * See @ref LED_PORT and other corresponding defines.
 *
 *
 * @section secButtons Buttons
 *
 * This board provides 1 button that is connected to pin 5 of port E.
 * See @ref BUTTON_PORT and other corresponding defines.
 *
 *
 * @section secSIO SIO (Serial I/O)
 *
 * This board provides two UART channels - UART_0 and UART_1,
 * but usually SIO is not used with plain RCBs.
 *
 *
 * @section secEEPROM External EEPROM
 *
 * This board does not provide an external EEPROM,
 * i.e. the IEEE address of the plaform needs to be stored in the
 * internal EEPROM of the MCU.
 * <BR>
 * Function pal_ps_get() (see pal.c) shall only be called with
 * parameter ps_type set to @ref INTERN_EEPROM.
 * <BR>
 * Make sure that an appropiate IEEE address is stored in the internal EEPROM.
 *
 *
 * @section secFuseSettings Fuse settings
 *
 * The following fuse settings are recommended:
 * - LF: 0xE2
 * - HF: 0x91
 * - EF: 0xFE
 *
 * Make sure that the fuse CKDIV8 is NOT set.
 *
 */

/* Prevent double inclusion */
#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H

/* === Includes =============================================================*/

#include "pal_boardtypes.h"

#if (BOARD_TYPE == RCB_3_2_PLAIN)

/**
 * \addtogroup grpPal_AT86RF230B_ATMEGA1281_RCB_3_2_PLAIN
 * @{
 */

/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types ================================================================*/

/** Enumerations used to identify ports */
typedef enum port_type_tag
{
    PORT_A,
    PORT_B,
    PORT_C,
    PORT_D,
    PORT_E,
    PORT_F,
    PORT_G
} port_type_t;

/** Enumerations used to identify port directions */
typedef enum port_pin_direction_type_tag
{
    DDR_A,
    DDR_B,
    DDR_C,
    DDR_D,
    DDR_E,
    DDR_F,
    DDR_G
} port_pin_direction_type_t;

/** Enumerations used to identify LEDs */
typedef enum led_id_tag
{
    LED_0,
    LED_1,
    LED_2
} led_id_t;

/** Number of LEDs provided by this board. */
#define NO_OF_LEDS                      (3)


/** Enumerations used to identify buttons */
typedef enum button_id_tag
{
    BUTTON_0
} button_id_t;

/** Number of buttons provided by this board. */
#define NO_OF_BUTTONS                   (1)

/* === Externals ============================================================*/


/* === Macros ===============================================================*/

/** The default CPU clock */
#ifndef F_CPU
#define F_CPU                           (8000000UL)
#endif

/**
 * This board supports CW transmission.
 */
#define CW_SUPPORTED                    (1)

/*
 * IRQ macros for ATmega1281
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

/** This macro saves the global interrupt status. */
#define ENTER_CRITICAL_REGION()         {uint8_t sreg = SREG; cli()

/** This macro restores the global interrupt status. */
#define LEAVE_CRITICAL_REGION()         SREG = sreg;}

/** This macro saves the trx interrupt status and disables the trx interrupt. */
#define ENTER_TRX_REGION()      { uint8_t irq_mask = TIMSK1; TIMSK1 &= ~(_BV(ICIE1))

/**  This macro restores the transceiver interrupt status. */
#define LEAVE_TRX_REGION()      TIMSK1 = irq_mask; }


/*
 * GPIO macros for ATmega1281
 */

/** This board uses an SPI-attached transceiver. */
#define PAL_USE_SPI_TRX                 (1)

/* Actual Ports */
/** PortB DDR is data direction register for the transceiver. */
#define TRX_PORT1_DDR                   (DDRB)

/** PORTB is transceiver port. */
#define TRX_PORT1                       (PORTB)

/** Slave select pin is PORTB 0. */
#define SEL                             (PB0)

/** SPI serial clock pin is PORTB 1. */
#define SCK                             (PB1)

/** SPI Bus Master Output/Slave Input pin is PORTB 2. */
#define MOSI                            (PB2)

/** SPI Bus Master Input/Slave Output pin is PORTB 3. */
#define MISO                            (PB3)

/** Sleep Transceiver pin is PORTB 4. */
#define SLP_TR                          (PB4)

/** RESET pin is PORTB 5. */
#define RST                             (PB5)

/*
 * Set TRX GPIO pins.
 */
#define RST_HIGH()                      (TRX_PORT1 |= _BV(RST))         /**< Set Reset pin to high. */
#define RST_LOW()                       (TRX_PORT1 &= ~_BV(RST))        /**< Set Reset pin to low. */
#define SLP_TR_HIGH()                   (TRX_PORT1 |= _BV(SLP_TR))      /**< Set Sleep/TR pin to high. */
#define SLP_TR_LOW()                    (TRX_PORT1 &= ~_BV(SLP_TR))     /**< Set Sleep/TR pin to low. */

/*
 * PORT where LEDs are connected
 */
#define LED_PORT                        (PORTE) /**< Port for LEDs */
#define LED_PORT_DIR                    (DDRE)  /**< Port directions for LEDs */

/*
 * PINs where LEDs are connected
 */
#define LED_PIN_0                       (PE2)   /**< LED 0 pin */
#define LED_PIN_1                       (PE3)   /**< LED 1 pin */
#define LED_PIN_2                       (PE4)   /**< LED 2 pin */

/*
 * PORT where button is connected
 */
#define BUTTON_PORT                     (PORTE) /**< Port for buttons */
#define BUTTON_PORT_DIR                 (DDRE)  /**< Port directions for buttons */
#define BUTTON_INPUT_PINS               (PINE)  /**< Input pins for buttons */

/**
 * PINs where buttons are connected
 */
#define BUTTON_PIN_0                    (PE5)   /**< Button 0 pin */

/**
 * PORT where the test mode pin TST is connected to.
 */
#define TST_PORT                        (PORTB)

/**
 * PORT direction register where the test mode pin TST is connected to.
 */
#define TST_PORT_DIR                    (DDRB)

/**
 * PIN number where the test mode pin TST is connected to.
 */
#define TST_PIN                         (PB6)

/**
 * Initialization of the TST pin: direction is set to output
 */
#define TST_INIT()                      TST_PORT_DIR |= 1<< TST_PIN

/**
 * Macro to set TST pin to high
 */
#define TST_PORT_HIGH()                 TST_PORT |= 1 << TST_PIN

/**
 * Macro to set TST pin to low
 */
#define TST_PORT_LOW()                  TST_PORT &= ~(1 << TST_PIN)

/**
 * Value of an external PA gain.
 * If no external PA is available, the value is 0.
 */
#define EXTERN_PA_GAIN                  (0)

/*
 * Timer macros for ATmega1281
 */

/**
 * This value is used by the calibration routine as target value of the timer
 * to be calibrated. At the end of a calibration cycle this value is compared
 * with the counter result.
 * TARGETVAL_CALIBRATION = CLK(Cal) * Count(Ref) / CLK(Ref) = @ref F_CPU * 256 / 32,768
 *
 * CLK(Cal): Frequency of clock source of timer to be calibrated
 * CLK(Ref): Frequency of clock source of reference timer
 * Count(Ref): Value of reference timer after 1 full cycle (until overflow)
 *             for timer width of 8 bit
 */
#if (F_CPU == (8000000UL))
#define TARGETVAL_CALIBRATION           (62500)
#elif  (F_CPU == (4000000UL))
#define TARGETVAL_CALIBRATION           (31250)
#elif  (F_CPU == (2000000UL))
#define TARGETVAL_CALIBRATION           (15625)
#elif  (F_CPU == (1000000UL))
#define TARGETVAL_CALIBRATION           (7813)
#else
#error "Unsupported F_CPU value"
#endif

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
 * ATmega1281 @ 8MHz
 */
/**
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is slower than 32 MHz we do not need
 * to have this value filled.
 */
#define PAL_WAIT_65_NS()                // empty

/** Wait for 500 ns. */
#define PAL_WAIT_500_NS()               {nop(); nop(); nop(); nop(); }

/** Wait for 1 us. */
#define PAL_WAIT_1_US()                 {nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();}

/** The smallest timeout in microseconds */
#define MIN_TIMEOUT                     (0x80)

/** The largest timeout in microseconds */
#define MAX_TIMEOUT                     (0x7FFFFFFF)

/** Minimum time in microseconds, accepted as a delay request */
#define MIN_DELAY_VAL                   (5)

/**
 * Timer clock source while radio is awake.
 * This is the CLKM of the tranceiver running at 1MHz.
 */
#define TIMER_SRC_DURING_TRX_AWAKE() \
        (TCCR1B = ((1 << ICES1) | (1 << CS12) | (1 << CS11) | (1 << CS10)))

/**
 * Timer clock source while radio is sleeping.
 * This is the Internal RC oscillator of the MCU running at 8MHz
 * using prescaler 8 (to gain a clock source of 1MHz.
 */
#define TIMER_SRC_DURING_TRX_SLEEP() \
        (TCCR1B = (1 << ICES1) | (1 << CS11))

/** Maximum numbers of software timers running at a time. */
#define MAX_NO_OF_TIMERS (25)
#if (MAX_NO_OF_TIMERS > 255)
#error "MAX_NO_OF_TIMERS must not be larger than 255"
#endif

/** Hardware register that holds the Rx timestamp. */
#define TIME_STAMP_REGISTER             (ICR1)


/*
 * TRX Access macros for ATmega1281
 */

/** Bit mask for slave select */
#define SS_BIT_MASK                     (1 << SEL)

/** Slave select made low */
#define SS_LOW()                        ((TRX_PORT1) &= (~SS_BIT_MASK))

/** Slave select made high */
#define SS_HIGH()                       ((TRX_PORT1) |= (SS_BIT_MASK))

/** Mask for SPIF bit in SPSR register */
#define SPIF_MASK                       (1 << SPIF)

/** Wait for SPI interrupt flag */
#define SPI_WAIT()                      while (!(SPSR & SPIF_MASK))

/** Dummy value written in SPDR to retrieve data form it. */
#define SPI_DUMMY_VALUE                 (0x00)

/** TRX Initialization */
#define TRX_INIT()                      do {                        \
    /* Enable the SPI and make the microcontroller as SPI master */ \
    SPCR = _BV(SPE) | _BV(MSTR);                                    \
    SPSR = _BV(SPI2X); /* Double the SPI clock frequency */         \
    /* Select the serial clock SCK to be (FOSC/4)                   \
     * For Example for ATmega1281, FOSC = 8Mhz                      \
     * (Microcontroller operating frequency)                        \
     * SPI clock is thus set to (8/4)*2 = 4 Mhz                     \
     */                                                             \
    SPCR &= ~(_BV(SPR0) | _BV(SPR1));                               \
                                                                    \
    /* Set SEL pin which is on PortB to high */                     \
    TRX_PORT1 |= _BV(SEL);                                          \
} while (0)

/** SPI Interrupt enable macro */
#define SPI_IRQ_ENABLE()        (SPCR |= _BV(SPIE))

/** SPI Interrupt disable macro */
#define SPI_IRQ_DISABLE()       (SPCR &= ~_BV(SPIE))

/** SPI Interrupt clear macro */
#define SPI_IRQ_CLEAR()         (SPSR &= ~_BV(SPIF))

/*
 * SIO macros for ATmega1281
 */
/*
 * UART0 and UART1 non-generic (board specific) initialization part.
 */
/* Data Length is 8 bit */
/** UART0 non-generic (board specific) initialization part. */
#define UART_0_INIT_NON_GENERIC()   (UCSR0C = (1 << UCSZ01) | (1 << UCSZ00))
/** UART1 non-generic (board specific) initialization part. */
#define UART_1_INIT_NON_GENERIC()   (UCSR1C = (1 << UCSZ11) | (1 << UCSZ10))


/**
 * This board does NOT have an external eeprom available.
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
 */
#define ALERT_INDICATE()                (LED_PORT = (uint8_t)(~(uint16_t)(LED_PORT)))


/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

/* MCU pins used for debugging  */
#if defined (ENABLE_DEBUG_PINS) || defined (DOXYGEN)
// Pin and port assignment
#define TST_PIN_0_HIGH()                PORTF |= (1 << 0)   /**< Test 0 high */
#define TST_PIN_0_LOW()                 PORTF &= ~(1 << 0)  /**< Test 0 low */
#define TST_PIN_1_HIGH()                PORTF |= (1 << 1)   /**< Test 1 high */
#define TST_PIN_1_LOW()                 PORTF &= ~(1 << 1)  /**< Test 1 low */
#define TST_PIN_2_HIGH()                PORTF |= (1 << 2)   /**< Test 2 high */
#define TST_PIN_2_LOW()                 PORTF &= ~(1 << 2)  /**< Test 2 low */
#define TST_PIN_3_HIGH()                PORTF |= (1 << 3)   /**< Test 3 high */
#define TST_PIN_3_LOW()                 PORTF &= ~(1 << 3)  /**< Test 3 low */
#define TST_PIN_4_HIGH()                PORTE |= (1 << 2)   /**< Test 4 high */
#define TST_PIN_4_LOW()                 PORTE &= ~(1 << 2)  /**< Test 4 low */
#define TST_PIN_5_HIGH()                PORTE |= (1 << 3)   /**< Test 5 high */
#define TST_PIN_5_LOW()                 PORTE &= ~(1 << 3)  /**< Test 5 low */
#define TST_PIN_6_HIGH()                PORTE |= (1 << 4)   /**< Test 6 high */
#define TST_PIN_6_LOW()                 PORTE &= ~(1 << 4)  /**< Test 6 low */
#define TST_PIN_7_HIGH()                PORTE |= (1 << 6)   /**< Test 7 high */
#define TST_PIN_7_LOW()                 PORTE &= ~(1 << 6)  /**< Test 7 low */
#define TST_PIN_8_HIGH()                PORTE |= (1 << 7)   /**< Test 8 high */
#define TST_PIN_8_LOW()                 PORTE &= ~(1 << 7)  /**< Test 8 low */
#else   /* ENABLE_DEBUG_PINS */
// empty pin assignment
#define TST_PIN_0_HIGH()
#define TST_PIN_0_LOW()
#define TST_PIN_1_HIGH()
#define TST_PIN_1_LOW()
#define TST_PIN_2_HIGH()
#define TST_PIN_2_LOW()
#define TST_PIN_3_HIGH()
#define TST_PIN_3_LOW()
#define TST_PIN_4_HIGH()
#define TST_PIN_4_LOW()
#define TST_PIN_5_HIGH()
#define TST_PIN_5_LOW()
#define TST_PIN_6_HIGH()
#define TST_PIN_6_LOW()
#define TST_PIN_7_HIGH()
#define TST_PIN_7_LOW()
#define TST_PIN_8_HIGH()
#define TST_PIN_8_LOW()
#endif  /* ENABLE_DEBUG_PINS */

#endif /* RCB_3_2_PLAIN */

#endif  /* PAL_CONFIG_H */
/* EOF */
