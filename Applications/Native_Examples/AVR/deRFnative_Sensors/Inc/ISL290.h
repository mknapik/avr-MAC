/**
 * @file ISL290.h
 *
 * @brief Application header file for ISL290.c.
 *
 * Low Level functions for ISL290 light sensor access on:
 *    - deRFnode
 *
 * $Id: ISL290.h,v 1.1.6.1 2010/09/07 17:39:29 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

#ifndef ISL290_H_
#define ISL290_H_

/* === INCLUDES ============================================================ */

/* === MACROS / DEFINES ==================================================== */

/********************************************************************
 * ISL290 TWI/I2C Address
 *******************************************************************/

#define ISL290_ADDRESS              (0x44)   ///< TWI address

/********************************************************************
 * ISL290 Register Map
 *******************************************************************/

#define ISL290_ADR_COMMAND          (0x00)   ///< address of command

#define ISL290_ADR_DATA_LSB         (0x01)   ///< address of data LSB
#define ISL290_ADR_DATA_MSB         (0x02)   ///< address of data MSB


/********************************************************************
 * ISL290 Configuration Bits (set within Command register)
 *******************************************************************/

#define ISL290_MODE_DISABLE          (0x00) ///< Disable ISL290
#define ISL290_MODE_ENABLE           (0x80) ///< Enable ISL290

#define ISL290_MODE_SINGLE_SHOT      (0x00) ///< One Time measurement
#define ISL290_MODE_CONTINUOUS       (0x40) ///< Continuous time measurement

#define ISL290_MODE_AMBIENT_LIGHT    (0x00) ///< Ambient light measurement
#define ISL290_MODE_INFRARED_LIGHT   (0x20) ///< Infrared light measurement

#define ISL290_MODE_FSR_1K           (0x00) ///< Full Scale Range to max. 1.000 Lux
#define ISL290_MODE_FSR_4K           (0x01) ///< Full Scale Range to max. 4.000 Lux
#define ISL290_MODE_FSR_16K          (0x02) ///< Full Scale Range to max. 16.000 Lux
#define ISL290_MODE_FSR_64K          (0x03) ///< Full Scale Range to max. 64.000 Lux

#define ISL290_MODE_INT_16BIT_ADC    (0x00) ///< internal timing with 16Bit ADC
#define ISL290_MODE_INT_12BIT_ADC    (0x04) ///< internal timing with 12Bit ADC
#define ISL290_MODE_INT_8BIT_ADC     (0x08) ///< internal timing with 8Bit ADC
#define ISL290_MODE_INT_4BIT_ADC     (0x0C) ///< internal timing with 4Bit ADC
#define ISL290_MODE_EXT_ADC          (0x10) ///< external timing with ADC data output
#define ISL290_MODE_EXT_TIMER        (0x14) ///< external timing with timer data output

/* === TYPES ================================================================ */

/* === PROTOTYPES =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ISL290_init(uint8_t enable);
uint8_t ISL290_read_data(uint16_t* lux);
uint8_t ISL290_enable();
uint8_t ISL290_disable();

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */


#endif /* ISL290_H_ */
