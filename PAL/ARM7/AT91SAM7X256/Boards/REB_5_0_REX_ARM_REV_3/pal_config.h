/**
 * @file pal_config.h
 *
 * @brief PAL configuration for AT91SAM7X256
 *
 * This header file contains configuration parameters for AT91SAM7X256.
 *
 * $Id: pal_config.h 22761 2010-08-05 07:15:47Z sschneid $
 *
 */
/**
 * @author
 * Atmel Corporation: http://www.atmel.com
 * Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H

/* === Includes ==============================================================*/

#include "AT91SAM7X256.h"
#include "pal_boardtypes.h"

#if (BOARD_TYPE == REB_5_0_REX_ARM_REV_3)

/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types =================================================================*/

/*
 * Indentifiers for PIO's in AT91SAM7X256
 */
typedef enum pio_type_tag
{
    PIO_A,
    PIO_B
} pio_type_t;

/* Enumerations used to identify LEDs */
typedef enum led_id_tag
{
    LED_0,
    LED_1,
    LED_2,
    LED_3
} led_id_t;

#define NO_OF_LEDS                      (4)


/* Enumerations used to identify buttons */
typedef enum button_id_tag
{
    BUTTON_0
} button_id_t;

#define NO_OF_BUTTONS                   (0)

/* === Externals =============================================================*/


/* === Macros ================================================================*/

/** The default CPU clock */
#ifndef F_CPU
#define F_CPU                           (32000000UL)
#endif

/*
 * F_CPU dependent variables
 *      F_CPU(MHz)    PLL MULTIPLIER      PLL DIVIDER     SPI BAUD DIVIDER
 *       32000000         124                 18                8
 *       48000000         72                  14                4
 */
#if (F_CPU == 32000000UL)

/* PLL multiplier to generate PLL clock of 128Mhz. */
#define PLL_MULTIPLIER                  (124)

/* PLL divider to generate PLL clock of 128Mhz. */
#define PLL_DIVIDER                     (18)

/* SPI baud rate divider to generate 4MHz SPI clock, when F_CPU is 32MHz. */
#define SPI_BR_DIVIDER                  (8)

/*
 * The prescaler of the master clock controller used to generate 32MHz clock
 * from 128MHz PLL clock.
 */
#define PMC_PRESCALER                   (AT91C_PMC_PRES_CLK_4)

#elif (F_CPU == 48000000UL)

/* PLL multiplier to generate PLL clock of 96Mhz. */
#define PLL_MULTIPLIER                  (72)

/* PLL divider to generate PLL clock of 96Mhz. */
#define PLL_DIVIDER                     (14)

/* SPI baud rate divider to generate 4MHz SPI clock, when F_CPU is 48MHz. */
#define SPI_BR_DIVIDER                  (12)

/*
 * The prescaler of the master clock controller used to generate 32MHz clock
 * from 96MHz PLL clock.
 */
#define PMC_PRESCALER                   (AT91C_PMC_PRES_CLK_2)

#endif


/**
 * Position of SCBR field in SPI Chip Select Register 2
 * (required for setting the SPI Serial clock baud rate).
 */
#define SCBR_FIELD_POS_IN_CSR_REG          (8)

/* The pin on AT91SAM7X256 to which the transceiver interrupt is connected. */
#define TRX_INTERRUPT_PIN               (AT91C_PA30_IRQ0)


/*
 *  Macros dealing with AIC of AT91SAM7X256
 */

/* This macro configures the AIC for specified peripheral interrupt. */
#define AIC_CONFIGURE(peripheral_id, mode, handler) do {                    \
    /* The interrupts for specified peripheral are first disabled. */       \
    AT91C_BASE_AIC->AIC_IDCR = _BV(peripheral_id);                          \
    /*                                                                      \
     * The edge at which the interrupt is to be triggered and handler for   \
     * the same is configured.                                              \
     */                                                                     \
    AT91C_BASE_AIC->AIC_SMR[peripheral_id] = mode;                          \
    AT91C_BASE_AIC->AIC_SVR[peripheral_id] = (uint32_t) handler;            \
    /* Pending interrupts if any for the peripheral are cleared. */         \
    AT91C_BASE_AIC->AIC_ICCR = _BV(peripheral_id);                          \
} while (0);


/*
 * IRQ macros for AT91SAM7X256
 */

/*
 * The second interrupt (i.e the timestamp IRQ) is always disabled for this board.
 */
#define DISABLE_TSTAMP_IRQ              (1)

/* Enables the transceiver interrupts. */
#define ENABLE_TRX_IRQ()                (AT91C_BASE_AIC->AIC_IECR = _BV(AT91C_ID_IRQ0))

/* Disables the transceiver interrupts. */
#define DISABLE_TRX_IRQ()               (AT91C_BASE_AIC->AIC_IDCR = _BV(AT91C_ID_IRQ0))

/* Clears the transceiver interrupts. */
#define CLEAR_TRX_IRQ()                 (AT91C_BASE_AIC->AIC_ICCR = _BV(AT91C_ID_IRQ0))


/*
 * Macros dealing with the global interrupt (IRQ bit) of AT91SAM7X256.
 */

/* Enables the global interrupt. */
#define ENABLE_GLOBAL_IRQ()             sei()

/* Disables the global interrupt. */
#define DISABLE_GLOBAL_IRQ()            cli()

/*
 * This macro is used to mark the start of a critical region. It saves the
 * current status register and then disables the interrupt.
 */
#define ENTER_CRITICAL_REGION()         \
    {uint32_t sreg; GET_CPSR(sreg); DISABLE_GLOBAL_IRQ()

/*
 * This macro is used to mark the end of a critical region. It restores the
 * current status register.
 */
#define LEAVE_CRITICAL_REGION()         SET_CPSR(sreg);}

/*
 * This macro saves the trx interrupt status and disables the trx interrupt.
 */
#define ENTER_TRX_REGION()      { uint32_t irq_mask = AT91C_BASE_AIC->AIC_ICCR; AT91C_BASE_AIC->AIC_IDCR = _BV(AT91C_ID_IRQ0)

/*
 *  This macro restores the transceiver interrupt status
 */
#define LEAVE_TRX_REGION()      AT91C_BASE_AIC->AIC_ICCR = irq_mask; }


/*
 * GPIO macros for AT91SAM7X256
 */

/*
 * This board uses an SPI-attached transceiver.
 */
#define PAL_USE_SPI_TRX                 (1)

/*
 * SPI Base Register:
 * SPI1 is used with REX ARM Rev. 3.
 */
#define AT91C_BASE_SPI_USED             (AT91C_BASE_SPI1)



/* RESET pin is pin 9 of PIOA. */
#define RST                             (AT91C_PIO_PA9)

/* Sleep Transceiver pin is pin 8 of PIOA. */
#define SLP_TR                          (AT91C_PIO_PA8)

/*
 * Slave select pin is PA21
 */
#define SEL                             (AT91C_PA21_SPI1_NPCS0)

/*
 * SPI Bus Master Output/Slave Input pin is PA23
 */
#define MOSI                            (AT91C_PA23_SPI1_MOSI)

/*
 * SPI Bus Master Input/Slave Output pin is PA24
 */
#define MISO                            (AT91C_PA24_SPI1_MISO)

/*
 * SPI serial clock pin is PA22
 */
#define SCK                             (AT91C_PA22_SPI1_SPCK)

/*
 * Set TRX GPIO pins.
 */
#define RST_HIGH()                      {AT91C_BASE_PIOA->PIO_SODR = RST;}
#define RST_LOW()                       {AT91C_BASE_PIOA->PIO_CODR = RST;}
#define SLP_TR_HIGH()                   {AT91C_BASE_PIOA->PIO_SODR = SLP_TR;}
#define SLP_TR_LOW()                    {AT91C_BASE_PIOA->PIO_CODR = SLP_TR;}


/*
 * Timer macros for AT91SAM7X256
 */

/*
 * These macros are placeholders for delay functions for high speed processors.
 *
 * The following delay are not reasonbly implemented via delay functions,
 * but rather via a certain number of NOP operations.
 * The actual number of NOPs for each delay is fully MCU and frequency
 * dependent, so it needs to be updated for each board configuration.
 *
 * AT91SAM7X256 @ FPU
 */
#if (F_CPU == 32000000UL)
/* 32 MHz */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is 32 MHz we need to have this value filled,
 * otherwise frame transmission may not be initiated properly.
 */
#define PAL_WAIT_65_NS()                nop()

/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               {nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop();}
/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 {nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop();}
#elif (F_CPU == 48000000UL)
/* 48 MHz */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is 48 MHz we need to have this value filled,
 * otherwise frame transmission may not be initiated properly.
 */
#define PAL_WAIT_65_NS()                {nop(); nop();}

/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               {nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop();}
/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 {nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop(); \
                                         nop(); nop(); nop(); nop();}
#endif

/*
 * The smallest timeout in microseconds
 */
#define MIN_TIMEOUT                     (0x80)

/*
 * The largest timeout in microseconds
 */
#define MAX_TIMEOUT                     (0x7FFFFFFF)

/*
 * Timer clock source while radio is awake.
 *
 */
#define TIMER_SRC_DURING_TRX_AWAKE()

/*
 * Timer clock source while radio is sleeping.
 *
 */
#define TIMER_SRC_DURING_TRX_SLEEP()

/*
 * Maximum numbers of software timers running at a time
 */
#define MAX_NO_OF_TIMERS                (25)
#if (MAX_NO_OF_TIMERS > 255)
#error "MAX_NO_OF_TIMERS must not be larger than 255"
#endif

/*
 * Hardware register that holds Rx timestamp
 */
#define TIME_STAMP_REGISTER             (AT91C_BASE_TC0->TC_RA)


/*
 * TRX Access macros for AT91SAM7X256
 */

/*
 * Position of the PCS (peripheral chip select) field in the SPI_MR register.
 */
#define PCS_FIELD_IN_MR                 (16)

/*
 * Value that needs to be written to in the PCS field of the SPI_MR to
 * activate the line to which the trx select line is connected.
 */
#define PCS_FIELD_VALUE                 (3)

/*
 * Value of PCS field in SPI_MR (mode register) which indicates the contoller
 * about the line to which the slave is connected.
 */
#define SS_ENABLE                       (PCS_FIELD_VALUE << PCS_FIELD_IN_MR)

/*
 * Slave select made low
 */
#define SS_LOW()                        {AT91C_BASE_PIOA->PIO_CODR = SEL;}

/*
 * Slave select made high
 */
#define SS_HIGH()                       {AT91C_BASE_PIOA->PIO_SODR = SEL;}


/*
 * Dummy value to be written in SPI transmit register to retrieve data form it
 */
#define SPI_DUMMY_VALUE                 (0x00)

/* Reads the data from the SPI receive register. */
#define SPI_READ(register_value)    do {                                \
    while ((AT91C_BASE_SPI_USED->SPI_SR & AT91C_SPI_RDRF) == 0);        \
    register_value = (AT91C_BASE_SPI_USED->SPI_RDR & 0xFFFF);           \
} while (0);

/* Writes the data into the SPI transmit register. */
#define SPI_WRITE(data)     do {                                        \
    while ((AT91C_BASE_SPI_USED->SPI_SR & AT91C_SPI_TDRE) == 0);        \
    AT91C_BASE_SPI_USED->SPI_TDR = data & 0xFFFF;                       \
    while ((AT91C_BASE_SPI_USED->SPI_SR & AT91C_SPI_TXEMPTY) == 0);     \
} while (0);


/*
 * LED pins on the AT91SAM7X256 board
 */
#define LED_PINS                (AT91C_PIO_PB19 | AT91C_PIO_PB20 | \
                                 AT91C_PIO_PB21 | AT91C_PIO_PB22)

/*
 * 5-way Joystick on the AT91SAM7X256 board cannot be used for this
 * adaptor board, since SPI1 is used for the transceiver access.
 */

/*
 * Value of an external PA gain.
 * If no external PA is available, the value is 0.
 */
#define EXTERN_PA_GAIN                  (0)

/*
 * This board does NOT have an external eeprom available.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (0)
#endif

/*
 * Alert initialization
 */
#define ALERT_INIT()        {AT91C_BASE_PIOB->PIO_OER = LED_PINS;}

/*
 * Alert indication
 */
#define ALERT_INDICATE()    do                      \
{                                                   \
    if (AT91C_BASE_PIOB->PIO_ODSR & LED_PINS)       \
    {                                               \
        AT91C_BASE_PIOB->PIO_CODR = LED_PINS;       \
    }                                               \
    else                                            \
    {                                               \
        AT91C_BASE_PIOB->PIO_SODR = LED_PINS;       \
    }                                               \
} while (0);


/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REB_5_0_REX_ARM_REV_3 */

#endif  /* PAL_CONFIG_H */
/* EOF */
