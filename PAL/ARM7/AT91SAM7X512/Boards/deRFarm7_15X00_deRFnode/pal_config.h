/**
 * @file pal_config.h
 *
 * @brief PAL configuration for AT91SAM7X512
 *
 * This header file contains configuration parameters for AT91SAM7X512.
 *
 * $Id: pal_config.h,v 1.1.6.1 2010/09/07 17:38:25 dam Exp $
 *
 *  @author
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

/* === Includes ==============================================================*/

#include <stdlib.h>

#include "AT91SAM7X512.h"
#include "pal_boardtypes.h"
#include "peripherals/pio/pio.h"
#include "peripherals/pio/pio_it.h"


#if (BOARD_TYPE == deRFarm7_15X00_deRFnode)

/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types =================================================================*/

/*
 * Identifiers for PIO's in AT91SAM7X512
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
    LED_2
} led_id_t;

#define NO_OF_LEDS                      (3)


/* Enumerations used to identify buttons */
typedef enum button_id_tag
{
    BUTTON_0,
    BUTTON_1
} button_id_t;

#define NO_OF_BUTTONS                   (2)

/* === Externals =============================================================*/

/* === Globals ===============================================================*/
extern const Pin pinIRQ;
extern const Pin pinRXTS;

/* === Macros ================================================================*/

/* The CPU clock */
#define F_CPU                           (BOARD_MCK)

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


/*
 * Macros dealing with the PMC of AT91SAM7X512
 */

/*
 * Enables the clock to the given peripheral id
 */
#define PERIPHERAL_CLOCK_ENABLE(id)     (AT91C_BASE_PMC->PMC_PCER = _BV(id))

/*
 * Disables the clock to the given peripheral id
 */
#define PERIPHERAL_CLOCK_DISABLE(id)    (AT91C_BASE_PMC->PMC_PCDR = _BV(id))



/*
 * SPI Base Register for TRX access
 */
#define AT91C_BASE_SPI_USED             (AT91C_BASE_SPI1)


/*
 * Macros dealing with the PIO of AT91SAM7X512
 */

/*
 * Enables the PIO control for the requested pin and sets it as output
 */
#define PIN_SET_AS_PIO_OUTPUT(pin, pio) do {                            \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
    if(NULL != pio_ptr) {                                               \
        /* Set Bit corresponding to pin in output enable register. */   \
        pio_ptr->PIO_OER = pin;                                         \
        /* PIO control is enabled on the specified pin. */              \
        pio_ptr->PIO_PER = pin;                                         \
    }                                                                   \
} while (0);

/*
 * Enables the PIO control for the requested pin and sets it as input
 */
#define PIN_SET_AS_PIO_INPUT(pin, pio) do {                             \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
    if(NULL != pio_ptr) {                                               \
        /* Set corresponding Bit in output disable register. */         \
        pio_ptr->PIO_ODR = pin;                                         \
        /* PIO control is enabled on the specified pin. */              \
        pio_ptr->PIO_PER = pin;                                         \
    }                                                                   \
} while (0);

/*
 * Disables the PIO control and enables the peripheral A control for the
 * requested pin.
 */
#define PIN_SET_AS_PERIPHERAL_A(pin, pio) do {                          \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
                                                                        \
    if(pio_ptr != NULL) {                                               \
        /* Bit corresponding to pin is set in A select register. */     \
        pio_ptr->PIO_ASR = pin;                                         \
        /* PIO control is disabled on the specified pin. */             \
        pio_ptr->PIO_PDR = pin;                                         \
    }                                                                   \
} while (0);

/*
 * Disables the PIO control and enables the peripheral B control for the
 * requested pin.
 */
#define PIN_SET_AS_PERIPHERAL_B(pin, pio) do {                          \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
                                                                        \
    if(pio_ptr != NULL) {                                               \
        /* Bit corresponding to pin is set in B select register. */     \
        pio_ptr->PIO_BSR = pin;                                         \
        /* PIO control is disabled on the specified pin. */             \
        pio_ptr->PIO_PDR = pin;                                         \
    }                                                                   \
} while (0);


/*
 * Sets the pin of requested PIO
 */
#define PIN_SET(pin, pio)   do {                                        \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
    if(NULL != pio_ptr) {                                               \
        pio_ptr->PIO_SODR = pin;                                        \
    }                                                                   \
} while (0);

/*
 * Clears the pin of requested PIO
 */
#define PIN_CLEAR(pin, pio) do {                                        \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
    if(NULL != pio_ptr) {                                               \
        pio_ptr->PIO_CODR = pin;                                        \
    }                                                                   \
} while (0);

/*
 * Gets the status of the requested pin of PIO
 * Differ between input and output pins
 */
#define PIN_GET_STATUS(status, pin, pio) do {                           \
    AT91S_PIO *pio_ptr=NULL;                                            \
    if(pio == PIO_A)                                                    \
        pio_ptr = AT91C_BASE_PIOA;                                      \
    else if(pio == PIO_B)                                               \
        pio_ptr = AT91C_BASE_PIOB;                                      \
                                                                        \
    if(pio_ptr->PIO_OER) /* output pin? */                              \
        status = ((pio_ptr->PIO_ODSR & pin) ? true : false);            \
    else                                                                \
        status = ((pio_ptr->PIO_PDSR & pin) ? true : false);            \
} while (0);

/*
 * Configures a given pin to trigger interrupts over the PIO Controller
 * This works only after PIO Interrupts have been enabled.
 */
#define PIN_CONFIGURE_PIO_IRQ(irq_pin, handler)     do {          \
    PIO_Configure(&irq_pin, 1);                                   \
    PIO_ConfigureIt(&irq_pin, (void (*)(const Pin *)) handler);   \
} while(0);


/*
 *  Macros dealing with AIC of AT91SAM7X512
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

/* This macro enables the interrupt of the specified peripheral at the AIC. */
#define AIC_ENABLE(peripheral_id)       (AT91C_BASE_AIC->AIC_IECR = \
                                        _BV(peripheral_id))

/* This macro disables the interrupt of the specified peripheral at the AIC. */
#define AIC_DISABLE(peripheral_id)      (AT91C_BASE_AIC->AIC_IDCR = \
                                         _BV(peripheral_id))

/* This macro clears the interrupt of the specified peripheral at the AIC. */
#define AIC_CLEAR(peripheral_id)        (AT91C_BASE_AIC->AIC_ICCR = \
                                        _BV(peripheral_id))


/*
 * IRQ macros for AT91SAM7X512
 */

/* Enables the transceiver interrupts. */
#define ENABLE_TRX_IRQ()                  ( PIO_EnableIt(&pinIRQ) )

/* Disables the transceiver interrupts. */
#define DISABLE_TRX_IRQ()                 ( PIO_DisableIt(&pinIRQ) )

/* Clears the transceiver interrupts. */
#define CLEAR_TRX_IRQ()                   ( pinIRQ.pio->PIO_ISR )


/* Enables the transceiver interrupts. */
#define ENABLE_TRX_IRQ_TSTAMP()           ( PIO_EnableIt(&pinRXTS) )

/* Disables the transceiver interrupts. */
#define DISABLE_TRX_IRQ_TSTAMP()          ( PIO_DisableIt(&pinRXTS) )

/* Clears the transceiver interrupts. */
#define CLEAR_TRX_IRQ_TSTAMP()            ( pinRXTS.pio->PIO_ISR )


/*
 * Macros dealing with the global interrupt (IRQ bit) of AT91SAM7X512.
 */

#if 1
/* Enables the global interrupt. */
#define ENABLE_GLOBAL_IRQ()             sei()
/* Disables the global interrupt. */
#define DISABLE_GLOBAL_IRQ()            cli()
#else
//#define ENABLE_GLOBAL_IRQ()     (AT91C_BASE_AIC->AIC_IECR = AT91C_ALL_INT)
#define ENABLE_GLOBAL_IRQ()     (AT91C_BASE_AIC->AIC_IECR = 0xFFFFFFFF)
//#define DISABLE_GLOBAL_IRQ()     (AT91C_BASE_AIC->AIC_IDCR = AT91C_ALL_INT)
#define DISABLE_GLOBAL_IRQ()     (AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF)
#endif

#if 1
/*
 * This macro is used to mark the start of a critical region. It saves the
 * current status register and then disables the interrupt.
 */
#define ENTER_CRITICAL_REGION() { uint32_t sreg; GET_CPSR(sreg); \
                                DISABLE_GLOBAL_IRQ()

/*
 * This macro is used to mark the end of a critical region. It restores the
 * current status register.
 */
#define LEAVE_CRITICAL_REGION() SET_CPSR(sreg);}

#else
#define ENTER_CRITICAL_REGION() { uint32_t _imask = AT91C_BASE_AIC->AIC_IMR; \
                                AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;

#define LEAVE_CRITICAL_REGION() AT91C_BASE_AIC->AIC_IECR = _imask; }
#endif

/*
 * This macro disables the trx interrupt.
 */
#define ENTER_TRX_REGION()      { PIO_DisableIt(&pinIRQ);

/*
 *  This macro restores the transceiver interrupt status.
 */
#define LEAVE_TRX_REGION()      PIO_EnableIt(&pinIRQ); }


/*
 * GPIO macros for AT91SAM7X512
 */

/*
 * This board uses an SPI-attached transceiver.
 */
#define PAL_USE_SPI_TRX                 (1)

/* RESET pin */
#define RST                             (AT91C_PIO_PA9)

/* Sleep Transceiver pin */
#define SLP_TR                          (AT91C_PIO_PA8)

/* Slave select pin */
#define SEL                             (AT91C_PIO_PA21)

/* SPI Bus Master Input/Slave Output pin */
#define MISO                            (AT91C_PIO_PA24)

/* SPI Bus Master Output/Slave Input pin */
#define MOSI                            (AT91C_PIO_PA23)

/* SPI serial clock pin */
#define SCK                             (AT91C_PIO_PA22)

/*
 * Set TRX GPIO pins.
 */
#define RST_HIGH()                      {PIN_SET(RST, PIO_A)}
#define RST_LOW()                       {PIN_CLEAR(RST, PIO_A)}
#define SLP_TR_HIGH()                   {PIN_SET(SLP_TR, PIO_A)}
#define SLP_TR_LOW()                    {PIN_CLEAR(SLP_TR, PIO_A)}

/* The pin on AT91SAM7X512 to which the transceiver interrupt is connected. */
//#define TRX_INTERRUPT_PIN               (AT91C_PA29_FIQ)
#define TRX_IRQ_PIN       {1 << 29, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT}

/* The pin on AT91SAM7X512 to which the transceiver timestamp interrupt is connected. */
//#define TRX_TS_INTERRUPT_PIN            (AT91C_PA30_IRQ0)
#define TRX_TS_IRQ_PIN    {1 << 30, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}


/*
 * Timer macros for AT91SAM7X512
 */

/*
 * These macros are placeholders for delay functions for high speed processors.
 *
 * The following delay are not reasonbly implemented via delay functions,
 * but rather via a certain number of NOP operations.
 * The actual number of NOPs for each delay is fully MCU and frequency
 * dependent, so it needs to be updated for each board configuration.
 *
 * AT91SAM7X512 @ FPU
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
/* 48 MHz -> one cycle takes 2.08E-8s = 20.8E-9s */
/*
 * Wait for 65 ns. -> 65/20.8 ~ 3
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is 48 MHz we need to have this value filled,
 * otherwise frame transmission may not be initiated properly.
 */
#define PAL_WAIT_65_NS()                {nop(); nop(); nop();}

/* Wait for 500 ns. -> 500/20.8 ~ 24 */
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
 * Settings to give clocking to timer when radio is awake
 *
 */
#define TIMER_SRC_DURING_TRX_AWAKE()

/*
 * Settings to give clocking to timer when radio is sleeping
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
 * TRX Access macros for AT91SAM7X512
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
#define SS_LOW()                        PIN_CLEAR(SEL, PIO_A)

/*
 * Slave select made high
 */
#define SS_HIGH()                       PIN_SET(SEL, PIO_A)

/*
 * Dummy value to be written in SPI transmit register to retrieve data form it
 */
#define SPI_DUMMY_VALUE                 (0x00)

/* Reads the data from the SPI receive register. */
#define SPI_READ(register_value)    do {                        \
    while ((AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_RDRF) == 0);    \
    register_value = (AT91C_BASE_SPI1->SPI_RDR & 0xFFFF);       \
} while (0);

/* Writes the data into the SPI transmit register. */
#define SPI_WRITE(data)     do {                                    \
    while ((AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_TDRE) == 0);        \
    AT91C_BASE_SPI1->SPI_TDR = data & 0xFFFF;                       \
    while ((AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_TXEMPTY) == 0);     \
} while (0);


/*
 * LED pins on the AT91SAM7X512 board
 */
#define LED_PIN_0               AT91C_PIO_PB26
#define LED_PIN_1               AT91C_PIO_PB21
#define LED_PIN_2               AT91C_PIO_PB19
#define LED_PINS                (LED_PIN_0 | LED_PIN_1 | LED_PIN_2)

/*
 * Button pins on the AT91SAM7X512 board
 */
#define BUTTON_PIN_0            AT91C_PIO_PA3
#define BUTTON_PIN_1            AT91C_PIO_PA4
#define BUTTON_PINS             (BUTTON_PIN_0 | BUTTON_PIN_1)

/*
 * Value of an external PA gain
 * If no external PA is available, value is 0.
 */
#define EXTERN_PA_GAIN                  (0)


/*
 * This board does NOT have an external eeprom available.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE         (0)
#endif


/*
 * Alert initialization
 */
#define ALERT_INIT()        {PIN_SET_AS_PIO_OUTPUT((LED_PINS), PIO_B);}

/*
 * Alert indication
 */
#define ALERT_INDICATE()    do                      \
{                                                   \
    bool pin_status;                                \
    PIN_GET_STATUS(pin_status, LED_PINS, PIO_B);    \
    if (pin_status)                                 \
    {                                               \
        PIN_CLEAR(LED_PINS, PIO_B);                 \
    }                                               \
    else                                            \
    {                                               \
        PIN_SET(LED_PINS, PIO_B);                   \
    }                                               \
} while (0);


/*
 * ADC Initialization values
 */
/*
 * Number of ADC conversions to be done during generation of random number.
 * Since a 16-bit value is needed and 4 ADC channels are used
 * to get 1 single bit (always the LSB), 4 subsequent conversions are required.
 */
#define NO_OF_CONVERSIONS               (4)

/* Value of the ADC clock in Hz */
#define USED_ADC_CLOCK_FREQ             (5000000)
/* Value of MCK in Hz */
#define USED_MCK_CLOCK_FREQ             (F_CPU)

#define ADC_STARTUP_TIME_NS             (10)        /* In us */
#define ADC_SAMPLE_AND_HOLD_TIME_US     (1200)      /* In ns */

/* Defines for the ADC Mode register */
#define ADC_PRESCAL     ((USED_MCK_CLOCK_FREQ / (2 * USED_ADC_CLOCK_FREQ)) - 1)
#define ADC_STARTUP     (((USED_ADC_CLOCK_FREQ / 1000000) * ADC_STARTUP_TIME_NS / 8) - 1)
#define ADC_SHTIM       ((((USED_ADC_CLOCK_FREQ / 1000000) * ADC_SAMPLE_AND_HOLD_TIME_US) / 1000) - 1)

#define ADC_CHANNEL_4                   (4)
#define ADC_CHANNEL_5                   (5)
#define ADC_CHANNEL_6                   (6)
#define ADC_CHANNEL_7                   (7)

#define ADC_NUM_1                       (ADC_CHANNEL_4)
#define ADC_NUM_2                       (ADC_CHANNEL_5)
#define ADC_NUM_3                       (ADC_CHANNEL_6)
#define ADC_NUM_4                       (ADC_CHANNEL_7)

/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* deRFarm7_15X00_deRFnode */

#endif  /* PAL_CONFIG_H */
/* EOF */
