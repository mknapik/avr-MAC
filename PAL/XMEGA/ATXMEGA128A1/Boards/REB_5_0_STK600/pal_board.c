/**
 * @file pal_board.c
 *
 * @brief PAL board specific functionality
 *
 * This file implements PAL board specific functionality.
 *
 * $Id: pal_board.c 19923 2010-01-14 11:00:19Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================ */

#include <stdbool.h>
#include <stdlib.h>
#include "pal.h"
#include "pal_boardtypes.h"
#include "pal_config.h"
/*
 * 'sys_time' is a entity of timer module which is given to other modules
 * through the interface file below. 'sys_time' is required to obtain the
 * frame timestamp
 */
#include "pal_timer.h"

#if (BOARD_TYPE == REB_5_0_STK600)

/* === Macros ============================================================== */

/* command bytes */
#define AT25010_CMD_WREN  0x06  /* Set Write Enable Latch */
#define AT25010_CMD_WRDI  0x04  /* Reset Write Enable Latch */
#define AT25010_CMD_RDSR  0x05  /* Read Status Register */
#define AT25010_CMD_WRSR  0x01  /* Write Status Register */
#define AT25010_CMD_READ  0x03  /* Read Data from Memory Array */
#define AT25010_CMD_WRITE 0x02  /* Write Data to Memory Array */

/* status register bits */
#define AT25010_STATUS_NRDY 0x01 /* not ready */
#define AT25010_STATUS_WE   0x02 /* write enabled */
#define AT25010_BP0         0x04 /* block-protection bit 0 */
#define AT25010_BP1         0x08 /* block-protection bit 1 */

/* Values for Feature Byte 1: RF frontend features */

/** RF connector enabled */
#define CFG_FEATURE1_RFCONN             0x01

/** On-board antenna enabled */
#define CFG_FEATURE1_ANTENNA            0x02

/** External antenna diversity */
#define CFG_FEATURE1_EXDIV              0x04

/** Reserved */
#define CFG_FEATURE1_RESV3              0x08

/** Transceiver-controlled PA present */
#define CFG_FEATURE1_PA                 0x10

/** Transceiver-controlled LNA present */
#define CFG_FEATURE1_LNA                0x20

/** Reserved */
#define CFG_FEATURE1_RESV6              0x40

/** Reserved */
#define CFG_FEATURE1_RESV7              0x80


/** Base address (in EEPROM) of configuration data record */
#define CFG_BASE_ADDRESS        64

/** Maximal length of textual board name */
#define CFG_NAMELEN             (32 - sizeof(uint16_t))

/** Total space reserved for binary configuration data */
#define CFG_BINARYLEN           32

#if (EXTERN_EEPROM_AVAILABLE == 1)
#if defined(__GNUC__)
#include <util/crc16.h>
#define CRC_CCITT_UPDATE(crc, data) _crc_ccitt_update(crc, data)
#endif /* defined(__GNUC__) */

#if defined(__ICCAVR__)
#define CRC_CCITT_UPDATE(crc, data) crc_ccitt_update(crc, data)
/* Internal helper function for CRC_CCITT_UPDATE. */
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data);
#endif /* __ICCAVR__ */
#endif /* EXTERN_EEPROM_AVAILABLE */


/* === Types =============================================================== */

/**
 * Encoding of the board family in the board_family configuration
 * record member.
 */
enum boardfamilycode
{
    CFG_BFAMILY_RADIO_EXTENDER, /**< Radio Extender boards */
    CFG_BFAMILY_RCB             /**< Radio Controller boards */
} SHORTENUM;

/**
 * Structure of the configuration record EEPROM data.  These data
 * reside at offset CFG_BASE_ADDR, and allocate the upper address
 * space of the EEPROM.  The lower address space is available for
 * further extensions, and/or customer use (the upper part can be
 * software write-protected on demand).
 *
 * The record is divided into 32 bytes of structured binary
 * configuration data, followed by up to 30 bytes of textual
 * configuration data (terminated with \0), and terminated by a CRC-16
 * checksum.  The CRC algorithm used is the same CCITT algorithm that
 * is also used to protect IEEE 802.15.4 frames.
 *
 * All multibyte integer values are stored in little-endia byte order.
 */
struct cfg_eeprom_data
{
    union
    {
        /** Raw access to binary configuration data. */
        uint8_t raw_binary[CFG_BINARYLEN];

        /* Structured access to binary configuration data. */
        struct
        {
            /** factory-supplied EUI-64 address */
            uint64_t mac_address;

            /** factory-supplied board serial number */
            uint64_t serial_number;

            /** board ID: family */
            enum boardfamilycode board_family;

            /** board ID: major, minor, revision */
            uint8_t board_id[3];

            /** feature byte 1: RF frontend features */
            uint8_t feature1;

            /** 16 MHz crystal oscillator calibration value */
            uint8_t cal_16mhz;

            /** RC oscillator calibration value, Vcc = 3.6 V */
            uint8_t cal_rc_36;

            /** RC oscillator calibration value, Vcc = 2.0 V */
            uint8_t cal_rc_20;

            /** antenna gain, 1/10 dB */
            int8_t antenna_gain;

            /* 7 bytes reserved, leave as 0xFF */
        }
        struct_binary;
    }
    binary_data;

    /** Textual board name, ASCIZ string */
    char board_name[CFG_NAMELEN];

    /** CRC-16 over binary_data and board_name[] */
    uint16_t crc;
};

/* === Globals ============================================================= */


/* === Prototypes ========================================================== */

#if (EXTERN_EEPROM_AVAILABLE == 1)
static uint8_t at25010_read_byte(uint8_t addr);
#endif

/* === Implementation ======================================================= */

/**
 * @brief Provides timestamp of the last received frame
 *
 * This function provides the timestamp (in microseconds)
 * of the last received frame.
 *
 * @param[out] Timestamp in microseconds
 */
void pal_trx_read_timestamp(uint32_t *timestamp)
{
    /*
     * Everytime a transceiver interrupt is triggred, input capture register of
     * the AVR is latched. The 'sys_time' is concatenated to the ICR to
     * generate the time stamp of the received frame.
     * 'sys_time'   'ICR'
     *  ---------|--------- => 32 bit timestamp
     *   16 bits   16 bits
     */
    *timestamp = (uint32_t)sys_time << (uint32_t)16;
    *timestamp |= (uint32_t)TIME_STAMP_REGISTER;
}



/**
 * @brief Calibrates the internal RC oscillator
 *
 * This function calibrates the internal RC oscillator.
 *
 * @return True since the RC oscillator is always calibrated
 *         automatically at startup by hardware itself
 */
bool pal_calibrate_rc_osc(void)
{
    return (true);
}



/**
 * @brief Initialize the event system of the ATxmega
 */
void event_system_init(void)
{
    /* Route system clock (16MHz) with prescaler 16 through event channel 0 */
    EVSYS_CH0MUX = EVSYS_CHMUX_PRESCALER_16_gc;

    /* Route port C pin 1 (DIG2) through event channel 1 */
    EVSYS_CH1MUX = EVSYS_CHMUX_PORTC_PIN1_gc;
}



/**
 * @brief Initialize the interrupt system of the ATxmega
 */
void interrupt_system_init(void)
{
    /* Enable high priority interrupts */
    PMIC.CTRL |= PMIC_HILVLEN_bm;
}



/**
 * @brief Initialize the clock of the ATxmega
 */
void clock_init(void)
{
    /*
     * Enable 32MHz internal oscillator
     * (and by thus disable the 2 MHz internal oscillator). */
    OSC.CTRL = OSC_RC32MEN_bm;

    /*
     * The ATxmega shall run from its internal 32MHz Oscillator.
     * Set the clock speed to 16MHz. Use internal 32MHz and DFLL.
     */
    while (0 == (OSC.STATUS & OSC_RC32MRDY_bm))
    {
        /* Hang until the internal 32MHz Oscillator is stable. */
        ;
    }

    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Use Prescaler A to divide 32MHz clock by 2 to 16MHz system clock. */
    CLK.PSCTRL = CLK_PSADIV0_bm;

    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Set internal 32MHz Oscillator as system clock. */
    CLK.CTRL = CLK_SCLKSEL0_bm;

    /* Enable DFLL for the internal oscillator. */
    OSC.DFLLCTRL = OSC_RC32MCREF_bm;
    DFLLRC32M.CTRL |= DFLL_ENABLE_bm;
}



/**
 * @brief Initializes the GPIO pins
 *
 * This function is used to initialize the port pins used to connect
 * the microcontroller to transceiver.
 */
void gpio_init(void)
{
    /* The following pins are output pins.  */
    TRX_PORT1_DDR |= _BV(SEL);
    TRX_PORT1_DDR |= _BV(SCK);
    TRX_PORT1_DDR |= _BV(TRX_RST);
    TRX_PORT1_DDR |= _BV(MOSI);
    TRX_PORT1_DDR |= _BV(SLP_TR);

    /* The following pins are input pins.  */
    TRX_PORT1_DDR &= ~_BV(MISO);
}



/*
 * This function is called by timer_init() to perform the non-generic portion
 * of the initialization of the timer module.
 *
 * sys_clk = 16MHz -> Will be used as source for Event Channel 0 with Prescaler 16
 *
 * Timer usage
 * TCC0_CCA: Systime (software timer based on compare match)
 * TCC1_CCA: Input capture for time stamping (only if Antenna diversity is not used)
 */
void timer_init_non_generic(void)
{
    /* Select proper clock as timer clock source when radio is sleeping */
    TIMER_SRC_DURING_TRX_SLEEP();

    /*
     * Clear pending output compare matches of all.
     */
    TCC0_INTFLAGS = TC0_CCAIF_bm | TC0_CCBIF_bm | TC0_CCCIF_bm | TC0_CCDIF_bm;
    TCC1_INTFLAGS = TC1_CCAIF_bm | TC1_CCBIF_bm;

    /* Enable the timer overflow interrupt for TCC0 used for systime overflow. */
    TCC0_INTCTRLA =  TC_OVFINTLVL_HI_gc;

    /* Assign event channel 1 as input capture to TCC1_CCA */
    TCC1_CTRLB |= TC1_CCAEN_bm;
    TCC1_CTRLD = TC1_EVACT0_bm | TC1_EVSEL3_bm | TC1_EVSEL0_bm;
}



/**
 * @brief Read one byte from an AT25010 EEPROM.
 *
 * Due to the connection between MCU, TRX and EEPROM, an EEPROM access
 * causes a transceiver reset. Therefore an entire transceiver configuration
 * is necessary after EEPROM access.
 *
 * @param addr Byte address to read from
 *
 * @return Data read from EEPROM
 */
#if (EXTERN_EEPROM_AVAILABLE == 1)
static uint8_t at25010_read_byte(uint8_t addr)
{
    uint8_t read_value;

    ENTER_TRX_REGION();

    RST_LOW();
    SS_LOW();

    PAL_WAIT_1_US();

    SPI_DATA_REG = AT25010_CMD_READ;
    SPI_WAIT();
    SPI_DATA_REG = addr & 0x7F; // mask out Bit 7 for 128x8 EEPROM
    SPI_WAIT();

    SPI_DATA_REG = 0;   // dummy value to start SPI transfer
    SPI_WAIT();
    read_value = SPI_DATA_REG;

    SS_HIGH();
    RST_HIGH();

    LEAVE_TRX_REGION();

    return read_value;
}
#endif



/**
 * @brief Get data from external EEPROM
 *
 * @param[in]  start_offset Start offset within EEPROM
 * @param[in]  length Number of bytes to read from EEPROM
 * @param[out] value Data from persistence storage
 *
 * @return MAC_SUCCESS  if external EERPOM is available and contains valid contents
 *         FAILURE else
 */
#if (EXTERN_EEPROM_AVAILABLE == 1)
retval_t extern_eeprom_get(uint8_t start_offset, uint8_t length, void *value)
{
    uint8_t i;
    struct cfg_eeprom_data cfg;
    uint8_t *up;
    uint16_t crc;
    uint8_t *data_ptr;
    uint8_t *value_ptr;

    // Read data from external EEPROM.
    for (i = 0, up = (uint8_t *)&cfg; i < sizeof(cfg); i++, up++)
    {
        *up = at25010_read_byte(i);
    }

    // Calcute CRC to validate data correctness
    for (i = 0, crc = 0, up = (uint8_t *)&cfg; i < sizeof(cfg); i++, up++)
    {
        crc = CRC_CCITT_UPDATE(crc, *up);
    }
    if (crc != 0)
    {
        return FAILURE;
    }

    /* Copy data from local cfg structure variable to target storage location */
    data_ptr = (uint8_t *)&cfg;
    data_ptr = &data_ptr[start_offset];
    value_ptr = (uint8_t *)value;
    for (i = 0; i < length; i++)
    {
        *value_ptr = *data_ptr;
        value_ptr++;
        data_ptr++;
    }

    return MAC_SUCCESS;
}
#endif


#if defined(__ICCAVR__) || defined(DOXYGEN)
/* This function is available in WINAVR library */
/**
 * @brief Computes the CCITT-CRC16 on a byte by byte basis
 *
 * This function computes the CCITT-CRC16 on a byte by byte basis.
 * It updates the CRC for transmitted and received data using the CCITT 16bit
 * algorithm (X^16 + X^12 + X^5 + 1).
 *
 * @param crc Current crc value
 * @param data Next byte that should be included into the CRC16
 *
 * @return updated CRC16
 */
#if (EXTERN_EEPROM_AVAILABLE == 1)
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data)
{
    data ^= crc & 0xFF;
    data ^= data << 4;

    return ((((uint16_t)data << 8) | ((crc & 0xFF00) >> 8)) ^ \
            (uint8_t)(data >> 4) ^ \
            ((uint16_t)data << 3));
}
#endif /* EXTERN_EEPROM_AVAILABLE */
#endif /* __ICCAVR__ || defined(DOXYGEN) */


/**
 * @brief Initialize LEDs
 */
void pal_led_init(void)
{
    /* All pins are output. */
    LED_PORT.OUT = 0xFF;
    LED_PORT.DIR = 0xFF;
}



/**
 * @brief Control LED status
 *
 * @param[in]  led_no LED ID
 * @param[in]  led_setting LED_ON, LED_OFF, LED_TOGGLE
 */
void pal_led(led_id_t led_no, led_action_t led_setting)
{
    uint8_t pin;

    switch (led_no)
    {
        case LED_0: pin = LED_PIN_0; break;
        case LED_1: pin = LED_PIN_1; break;
        case LED_2: pin = LED_PIN_2; break;
        case LED_3: pin = LED_PIN_3; break;
        case LED_4: pin = LED_PIN_4; break;
        case LED_5: pin = LED_PIN_5; break;
        case LED_6: pin = LED_PIN_6; break;
        case LED_7: pin = LED_PIN_7; break;
        default: pin = LED_PIN_0; break;
    }

    switch (led_setting)
    {
        case LED_ON: LED_PORT.OUTCLR = _BV(pin); break;

        case LED_OFF: LED_PORT.OUTSET = _BV(pin); break;

        case LED_TOGGLE:
        default:
            LED_PORT.OUTTGL = _BV(pin);
            break;
    }
}



/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    /* All pins are input. */
    BUTTON_PORT.DIR = 0;
}



/**
 * @brief Read button
 *
 * @param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    uint8_t pin;

    switch (button_no)
    {
        case BUTTON_0: pin = BUTTON_PIN_0; break;
        case BUTTON_1: pin = BUTTON_PIN_1; break;
        case BUTTON_2: pin = BUTTON_PIN_2; break;
        case BUTTON_3: pin = BUTTON_PIN_3; break;
        case BUTTON_4: pin = BUTTON_PIN_4; break;
        case BUTTON_5: pin = BUTTON_PIN_5; break;
        case BUTTON_6: pin = BUTTON_PIN_6; break;
        case BUTTON_7: pin = BUTTON_PIN_7; break;
        default: pin = BUTTON_PIN_0; break;
    }

    if ((BUTTON_PORT.IN & (1 << pin)) == 0x00)
    {
        return BUTTON_PRESSED;
    }
    else
    {
        return BUTTON_OFF;
    }
}

#endif /* REB_5_0_STK600 */

/* EOF */
