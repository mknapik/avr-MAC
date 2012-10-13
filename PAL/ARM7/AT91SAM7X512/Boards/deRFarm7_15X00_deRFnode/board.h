/**
 * @file board.h
 *
 * @brief AT91Lib board specific functionality
 *
 * This file provides a number of definitions, which are needed by the AT91Lib
 * and therefore cannot be placed into the pal_config.h.
 *
 * $Id: board.h,v 1.1.6.1 2010/09/07 17:38:25 dam Exp $
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

#ifndef BOARD_H
#define BOARD_H

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <AT91SAM7X512.h>

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Board
//------------------------------------------------------------------------------
/// String containing the name of the board.
#define BOARD_NAME      "deRFarm7_15X00_deRFnode"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Clocks
//------------------------------------------------------------------------------
/// Frequency of the board main oscillator, in Hz.
#define BOARD_MAINOSC           16000000

/// Master clock frequency (when using board_lowlevel.c), in Hz.
#define BOARD_MCK               48000000
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// USB
//------------------------------------------------------------------------------
/// Chip has a UDP controller.
#define BOARD_USB_UDP

/// Indicates the D+ pull-up is always connected.
#define BOARD_USB_PULLUP_ALWAYSON

/// Number of endpoints in the USB controller.
#define BOARD_USB_NUMENDPOINTS                  6

/// Returns the maximum packet size of the given endpoint.
#define BOARD_USB_ENDPOINTS_MAXPACKETSIZE(i)    ((((i) == 4) || ((i) == 5)) ? 256 : (((i) == 0) ? 8 : 64))

/// Returns the number of FIFO banks for the given endpoint.
#define BOARD_USB_ENDPOINTS_BANKS(i)            ((((i) == 0) || ((i) == 3)) ? 1 : 2)

/// USB attributes configuration descriptor (bus or self powered, remote wakeup)
#define BOARD_USB_BMATTRIBUTES                  USBConfigurationDescriptor_SELFPOWERED_NORWAKEUP
//#define BOARD_USB_BMATTRIBUTES                  USBConfigurationDescriptor_BUSPOWERED_NORWAKEUP
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PIO definitions
//------------------------------------------------------------------------------
// DBGU pins definition.
#define PINS_DBGU  {0x18000000, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/// LED 0 pin definition.
#define PIN_LED_0  {1 << 26, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
/// LED 1 pin definition.
#define PIN_LED_1  {1 << 21, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
/// LED 2 pin definition.
#define PIN_LED_2  {1 << 19, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
/// List of all LEDs pin definitions.
#define PINS_LEDS  PIN_LED_0, PIN_LED_1, PIN_LED_2

/// Push button #0 definition.
#define PIN_PUSHBUTTON_0    {1 << 3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEGLITCH | PIO_PULLUP}
/// Push button #1 definition - shared with the IRQ line from BMA150
#define PIN_PUSHBUTTON_1    {1 << 4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEGLITCH | PIO_PULLUP}
/// List of all push button definitions.
#define PINS_PUSHBUTTONS    PIN_PUSHBUTTON_0, PIN_PUSHBUTTON_1

/// BMA150 IRQ line
#define PIN_BMA150_IRQ      {1 << 4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT}


/// pins not used for any peripheral -> may be used as custom inputs or outputs

/// INPUT 0 pin definition.
#define PIN_INPUT_0  {1 << 0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
/// INPUT 1 pin definition.
#define PIN_INPUT_1  {1 << 1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
/// INPUT 2 pin definition.
#define PIN_INPUT_2  {1 << 28, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_PULLUP}
/// List of all INPUT pin definitions.
#define PINS_INPUT   PIN_INPUT_0, PIN_INPUT_1, PIN_INPUT_2

/// OUTPUT 0 pin definition.
#define PIN_OUTPUT_0 {1 << 0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
/// OUTPUT 1 pin definition.
#define PIN_OUTPUT_1 {1 << 1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
/// OUTPUT 2 pin definition.
#define PIN_OUTPUT_2 {1 << 28, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}
/// List of all OUTPUT pin definitions.
#define PINS_OUTPUT  PIN_OUTPUT_0, PIN_OUTPUT_1, PIN_OUTPUT_2

/// TWI pins definition.
#define PINS_TWI  {0x00000C00, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/// USB VBUS detection pin definition.
#define PIN_USB_VBUS  {1 << 25, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Serial data flash
//------------------------------------------------------------------------------
/// Base address of SPI peripheral connected to the serialflash.
#define BOARD_AT25_SPI_BASE                AT91C_BASE_SPI0
/// Identifier of SPI peripheral connected to the serialflash.
#define BOARD_AT25_SPI_ID                  AT91C_ID_SPI0
/// Pins of the SPI peripheral connected to the serialflash.
#define BOARD_AT25_SPI_PINS                PINS_SPI0, PIN_SPI0_NPCS0
/// Chip select connected to the serialflash.
#define BOARD_AT25_NPCS                    2
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// EMAC
//------------------------------------------------------------------------------
/// Board EMAC mode - RMII/MII ( 1/0 )
#define BOARD_EMAC_MODE_RMII 1 // MII is the default mode
/// The PIN list of PIO for EMAC
#define BOARD_EMAC_PINS     {0x3FFFF, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Two Wire Interface
//------------------------------------------------------------------------------
/// TWI clock frequency in Hz.
#define BOARD_TWI_CLK               100000

// TWI sensor addresses
// all adresses are one bit shifted right!!
#define BOARD_SENS_ADDR_TEMP         (0x48)    // 0x90 according to datasheet
#define BOARD_SENS_ADDR_LIGHT        (0x44)    // 0x88 according to datasheet
#define BOARD_SENS_ADDR_ACCEL        (0x38)    // 0x70 according to datasheet
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Flash
//------------------------------------------------------------------------------
/// Indicates chip has an EFC.
#define BOARD_FLASH_EFC
/// Address of the IAP function in ROM.
#define BOARD_FLASH_IAP_ADDRESS         0x300008
//------------------------------------------------------------------------------

#endif //#ifndef BOARD_H

