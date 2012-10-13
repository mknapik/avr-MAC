/**
 * @file PAL/MEGA_RF/ATMEGA128RFA1/Boards/RCB_6_3_PLAIN/pal_board.c
 *
 * @brief Board specific functions for RCB_6_3_PLAIN with ATmega128RFA1
 *
 * $Id: pal_board.c 21184 2010-03-29 10:21:42Z uwalter $
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

#if (BOARD_TYPE == RCB_6_3_PLAIN)

/**
 * \addtogroup grpPal_ATMEGA128RFA1_RCB_6_3_PLAIN
 * @{
 */

/* === Macros ============================================================== */

/* EEPROM command bytes */
#define AT25010_CMD_WREN  0x06  /**< Set Write Enable Latch */
#define AT25010_CMD_WRDI  0x04  /**< Reset Write Enable Latch */
#define AT25010_CMD_RDSR  0x05  /**< Read Status Register */
#define AT25010_CMD_WRSR  0x01  /**< Write Status Register */
#define AT25010_CMD_READ  0x03  /**< Read Data from Memory Array */
#define AT25010_CMD_WRITE 0x02  /**< Write Data to Memory Array */

/* EEPROM status register bits */
#define AT25010_STATUS_NRDY 0x01 /**< not ready */
#define AT25010_STATUS_WE   0x02 /**< write enabled */
#define AT25010_BP0         0x04 /**< block-protection bit 0 */
#define AT25010_BP1         0x08 /**< block-protection bit 1 */

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

#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
static uint8_t at25010_read_byte(uint8_t addr);
static void at25010_spi_setup(void);
static void at25010_spi_cleanup(void);
#endif  /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */

/* === Implementation ======================================================= */

/**
 * @brief Calibrates the internal RC oscillator
 *
 * This function calibrates the internal RC oscillator, based
 * on the external 32.768 Hz crystal clock source.
 *
 * It will also set the seed for the random generator based as
 * a side effect of the calibration.
 *
 * @return True if calibration is successful, false otherwise.
 */
bool pal_calibrate_rc_osc(void)
{
    /* Nothing to be done. */
    return true;
}



#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
/**
 * @brief Setup the SPI subsystem for accessing an AT25010 EEPROM
 *
 * The SPI subsystem on the RCB 6.3 is located on Port B which is
 * either used as the data port when connecting to an FT245 USB
 * converter, or available as general IO to the user otherwise.  For
 * that reason, PAL will only enable the SPI subsystem in order to
 * access the feature EEPROM (AT25010), and does not touch port B
 * otherwise.
 *
 * The AT25010's chip select signal is located on PG5, outside port B.
 */
static void at25010_spi_setup(void)
{
    /* Ensure /SS, SCK and MOSI become outputs. */
    DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);
    /*
     * Set SPI speed very low to allow proper EEPROM reading
     * even with large capacitors on pins (e.g. for debugging
     * on Breakout Board light.
     * Here the SPI speed is put to fosc/128 by setting
     * SPR1 and SPR0 bit.
     */
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
}
#endif  /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */



#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
/**
 * @brief Clean up the SPI subsystem after accessing an AT25010 EEPROM
 *
 * This frees up the SPI subsystem completely, and thus, leaves port B
 * back to either the FT245 USB converter, or to the user.
 */
static void at25010_spi_cleanup(void)
{
    SPCR = 0;
    DDRB &= ~_BV(PB0);
}
#endif  /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */



#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
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
static uint8_t at25010_read_byte(uint8_t addr)
{
    uint8_t read_value;

    EXT_EE_CS_PORT &= ~EXT_EE_CS_PIN;

    PAL_WAIT_1_US();

    SPDR = AT25010_CMD_READ;
    while ((SPSR & _BV(SPIF)) == 0)
        /* wait */;
    SPDR = addr & 0x7F; // mask out Bit 7 for 128x8 EEPROM
    while ((SPSR & _BV(SPIF)) == 0)
        /* wait */;

    SPDR = 0;   // dummy value to start SPI transfer
    while ((SPSR & _BV(SPIF)) == 0)
        /* wait */;
    read_value = SPDR;

    EXT_EE_CS_PORT |= EXT_EE_CS_PIN;

    return read_value;
}
#endif  /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */



#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
/**
 * @brief Get data from external EEPROM
 *
 * @param[in]  start_offset Start offset within EEPROM
 * @param[in]  length Number of bytes to read from EEPROM
 * @param[out] value Data from persistence storage
 *
 * @return MAC_SUCCESS If external EERPOM is available and contains valid contents
 *         FAILURE else
 */
retval_t extern_eeprom_get(uint8_t start_offset, uint8_t length, void *value)
{
    uint8_t i;
    struct cfg_eeprom_data cfg;
    uint8_t *up;
    uint16_t crc;

    // Read data from external EEPROM.
    at25010_spi_setup();
    for (i = 0, up = (uint8_t *)&cfg; i < sizeof(cfg); i++, up++)
    {
        *up = at25010_read_byte(i);
    }
    at25010_spi_cleanup();

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
    memcpy(value, ((uint8_t *)&cfg) + start_offset, length);

    return MAC_SUCCESS;
}
#endif  /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */



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
#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data)
{
    data ^= crc & 0xFF;
    data ^= data << 4;

    return ((((uint16_t)data << 8) | ((crc & 0xFF00) >> 8)) ^ \
            (uint8_t)(data >> 4) ^ \
            ((uint16_t)data << 3));
}
#endif /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */
#endif /* __ICCAVR__ || defined(DOXYGEN) */



/**
 * @brief Initialize LEDs
 */
void pal_led_init(void)
{
    LED_PORT |= (1 << LED_PIN_0);
    LED_PORT_DIR |= (1 << LED_PIN_0);
    LED_PORT |= (1 << LED_PIN_1);
    LED_PORT_DIR |= (1 << LED_PIN_1);
    LED_PORT |= (1 << LED_PIN_2);
    LED_PORT_DIR |= (1 << LED_PIN_2);
}



/**
 * @brief Control LED status
 *
 * @param[in]  led_no LED ID
 * @param[in]  led_setting LED_ON, LED_OFF, LED_TOGGLE
 */
void pal_led(led_id_t led_no, led_action_t led_setting)
{
    switch (led_no)
    {
    default:
    case 0:
        switch (led_setting)
        {
        case LED_ON:    LED_PORT &= ~(1 << LED_PIN_0); break;
        case LED_OFF:   LED_PORT |= (1 << LED_PIN_0); break;
        case LED_TOGGLE: LED_PORT ^= (1 << LED_PIN_0); break;
        }
        break;

    case 1:
        switch (led_setting)
        {
        case LED_ON:    LED_PORT &= ~(1 << LED_PIN_1); break;
        case LED_OFF:   LED_PORT |= (1 << LED_PIN_1); break;
        case LED_TOGGLE: LED_PORT ^= (1 << LED_PIN_1); break;
        }
        break;

    case 2:
        switch (led_setting)
        {
        case LED_ON:    LED_PORT &= ~(1 << LED_PIN_2); break;
        case LED_OFF:   LED_PORT |= (1 << LED_PIN_2); break;
        case LED_TOGGLE: LED_PORT ^= (1 << LED_PIN_2); break;
        }
        break;

    }
}


/**
 * @brief Initialize the button
 */
void pal_button_init(void)
{
    BUTTON_PORT |= (1 << BUTTON_PIN_0);
    BUTTON_PORT_DIR &= ~(1 << BUTTON_PIN_0);
}



/**
 * @brief Read button
 *
 * @param button_no Button ID
 */
button_state_t pal_button_read(button_id_t button_no)
{
    /* Keep compiler happy. */
    button_no = button_no;

    if ((BUTTON_INPUT_PINS & (1 << BUTTON_PIN_0)) == 0x00)
    {
        return BUTTON_PRESSED;
    }
    else
    {
        return BUTTON_OFF;
    }
}



#if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN)
/**
 * @brief Initializes the GPIO used for the extern EEPROM
 *
 * This function is used to initialize the port pins connecting
 * the microcontroller to the extern EEPROM.
 */
void extern_eeprom_init(void)
{
    EXT_EE_CS_PORT |= EXT_EE_CS_PIN;
    EXT_EE_CS_DDR |= EXT_EE_CS_PIN;
}
#endif  /* #if (EXTERN_EEPROM_AVAILABLE == 1) || defined(DOXYGEN) */


/**
 * @brief Power handling
 *
 * Handles power modes for entire system
 *
 * @param pwr_mode desired power mode
 *
 * @return retval_t MAC_SUCCESS
 * @ingroup apiPalApi
 */
retval_t pal_pwr_mode(pwr_mode_t pwr_mode)
{
    if (pwr_mode == SYSTEM_SLEEP)
    {
        // Clear interrupt flag
        EIFR = (1 << INTF5);
        // Enable external interrupt
        EIMSK |= (1 << INT5);
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_mode();
    }
    return MAC_SUCCESS;
}


/**
 * @brief ISR for external interrupt at pin PE5
 */
ISR(INT5_vect)
{
    /* MCU wakeup */
    // Disable external interrupt
    EIMSK &= ~(1 << INT5);
}


/** @} */

#endif /* RCB_6_3_PLAIN */

/* EOF */
