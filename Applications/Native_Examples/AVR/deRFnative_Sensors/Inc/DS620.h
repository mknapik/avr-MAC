/**
 * @file DS620.h
 *
 * @brief Application header file for DS620.c.
 *
 * Low Level functions for DS620 temperature sensor access on:
 *    - deRFnode
 *
 * $Id: DS620.h,v 1.1.6.1 2010/09/07 17:39:29 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

#ifndef DS620_H_
#define DS620_H_

/* === INCLUDES ============================================================ */

/* === MACROS / DEFINES ==================================================== */

/********************************************************************
 * DS620 TWI/I2C Address
 *******************************************************************/

#define DS620_ADDRESS               (0x48)   ///< TWI address

/********************************************************************
 * DS620 Register Map
 *******************************************************************/

#define DS620_ADR_TEMP_MSB          (0xAA)   ///< address of temperature register (MSB)
#define DS620_ADR_TEMP_LSB          (0xAB)   ///< address of temperature register (LSB)

#define DS620_ADR_CONFIG_MSB        (0xAC)   ///< address of configuration register (MSB)
#define DS620_ADR_CONFIG_LSB        (0xAD)   ///< address of configuration register (LSB)

/********************************************************************
 * DS620 Configuration Bits (set within Configuration register)
 *******************************************************************/

#define DS620_CFG_RESOLUTION_10BIT  (0x00)   ///< configure DS620 with 10Bit resolution (fast but unexact)
#define DS620_CFG_RESOLUTION_11BIT  (0x04)   ///< configure DS620 with 11Bit resolution
#define DS620_CFG_RESOLUTION_12BIT  (0x08)   ///< configure DS620 with 12Bit resolution
#define DS620_CFG_RESOLUTION_13BIT  (0x0C)   ///< configure DS620 with 13Bit resolution (slow but exact)

#define DS620_CFG_1SHOT             (0x01)   ///< configure DS620 just make a single measurement
#define DS620_CFG_AUTO_CONV         (0x02)   ///< configure DS620 make continous measurement

#define DS620_CFG_DONE              (0x80)   ///< mask bit which indicate if measurement is completed

/*******************************************************************
 * DS620 Commands
 *******************************************************************/

/**
 * Initiates temperature conversions. If the part is in one-shot mode (1SHOT = 1), only one
 * conversion is performed. In continuous mode (1SHOT = 0), continuous temperature conversions
 * are performed until a Stop Convert command is issued (even if 1SHOT is changed to a 1).
 */
#define DS620_CMD_START_CONVERT     (0x51)

/**
 * Stops temperature conversions when  the device is in continuous conversion mode (1SHOT = 0).
 * This command has no function if the device is in one-shot mode (1SHOT = 1)
 */
#define DS620_CMD_STOP_CONVERT      (0x22)

/**
 * Refreshes SRAM shadow register with EEPROM data.
 */
#define DS620_CMD_RECALL_DATA       (0xB8)

/**
 * Copies data from all SRAM shadow registers to EEPROM.
 * The DS620 must be set to the continuous conversion mode and be actively
 * converting temperature to enable the Copy Data command to function properly.
 */
#define DS620_CMD_COPY_DATA         (0x48)

/**
 * Initiates a software power-on-reset (POR), which stops temperature conversions and resets
 * all registers and logic to their power-up states. The software POR allows the user to
 * simulate cycling the power without actually powering down the device. This command should
 * not be issued while a Copy Data command is in progress.
 */
#define DS620_CMD_RESET             (0x54)

/**
 * Define a structure where temperature can be saved to.
 */
typedef struct
{
   uint8_t sign;              ///< (1) - signed, (0) unsigned
   uint8_t integralDigit;     ///< integral part of temperature
   uint8_t fractionalDigit;   ///< fractional part of temperature
   uint8_t conversion_done;   ///< (1) - conversion completed; (0) - conversion uncompleted
} DS620_temperature_t;


/* === TYPES ================================================================ */

/* === PROTOTYPES =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

uint8_t DS620_init(uint8_t config_msb, uint8_t config_lsb);
uint8_t DS620_start_measurement(void);
uint8_t DS620_read_temperature(DS620_temperature_t* temperature);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */

#endif /* DS620_H_ */

/*
 * EOF
 */

