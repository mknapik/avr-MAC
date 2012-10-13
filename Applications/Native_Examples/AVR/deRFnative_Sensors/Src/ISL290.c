/**
 * @file ISL290.c
 *
 * @brief Implementation of the native ISL290 light sensor interface.
 *
 * Low Level functions for ISL290 light sensor access on:
 *    - deRFnode
 *
 * $Id: ISL290.c,v 1.1.6.1 2010/09/07 17:39:30 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stdint.h>

#include "twi_master.h"
#include "ISL290.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/*
 * Select the modes for ISL290, the modes are the configuration data for command register
 */
#define MODE_FSR              ISL290_MODE_FSR_64K        ///< set the Full Scale Range
#define MODE_MEASUREMENT      ISL290_MODE_SINGLE_SHOT    ///< set the Operation Mode (Single Shot, Continuous)
#define MODE_LIGHT            ISL290_MODE_AMBIENT_LIGHT  ///< set the measurment method (ambient light or infrared light)
#define MODE_RES_TIMER        ISL290_MODE_INT_16BIT_ADC  ///< set the resolution and timer

/**
 * Define RANGE and COUNT for proper light value (in LUX) calculation (see datasheet for further details)
 */
#if MODE_FSR == ISL290_MODE_FSR_1K
#define RANGE     (1000UL)
#elif MODE_FSR == ISL290_MODE_FSR_4K
#define RANGE     (4000UL)
#elif MODE_FSR == ISL290_MODE_FSR_16K
#define RANGE     (16000UL)
#elif MODE_FSR == ISL290_MODE_FSR_64K
#define RANGE     (64000UL)
#else
#define RANGE     (1)
#endif

#if MODE_RES_TIMER == ISL290_MODE_INT_16BIT_ADC
#define COUNT     (65536UL) // 2^16
#elif MODE_RES_TIMER == ISL290_MODE_INT_12BIT_ADC
#define COUNT     (4096UL) // 2^12
#elif MODE_RES_TIMER == ISL290_MODE_INT_8BIT_ADC
#define COUNT     (256UL) // 2^8
#elif MODE_RES_TIMER == ISL290_MODE_INT_4BIT_ADC
#define COUNT     (16UL) // 2^4
#else
#define COUNT     (1)
#endif

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ============================================================= */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief initialize the ISL290 light sensor
 *
 * @param   enable   enable or disable the sensor (ISL290_MODE_DISABLE, ISL290_MODE_ENABLE)
 *
 * @return           0 if everthing work fine, 1 if any TWI error occurs
 */
uint8_t ISL290_init(uint8_t enable)
{
   uint8_t status;

   /* assemble command register */
   uint8_t config = enable | MODE_LIGHT | MODE_MEASUREMENT | MODE_RES_TIMER | MODE_FSR;

   /* send address from command register and write data to it */
   status = twi_start(ISL290_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(ISL290_ADR_COMMAND);
   if(status){return 1;}
   status = twi_write(config);
   if(status){return 1;}

   return 0;
}

/**
 * @brief Read light data from ISL290 light sensor and calculate correct value in LUX
 *
 * @param   lux   pointer to data where result value is stored in lux
 *
 * @return        0 if no errors occur, 1 if any TWI error occured
 */
uint8_t ISL290_read_data(uint16_t* lux)
{
   uint8_t status;

   /* send address where we want read data from */
   status = twi_start(ISL290_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(ISL290_ADR_DATA_LSB);
   if(status){return 1;}

   /* stop connection to restart with read access */
   twi_stop();
   status = twi_start(ISL290_ADDRESS, TWI_READ);
   if(status){return 1;}

   /* read back temperature and configuration registers */
   uint8_t data_lsb = twi_readAck();
   uint8_t data_msb = twi_readNack();

   /*                                                              RANGE
    * Calculation: E (Lux) = a * (data_msb|data_lsb) = a * data = ------- * data
    *                                                              COUNT
    *
    * RANGE depends on Full Scale Range and COUNT depends on ADC resolution
    */
   *lux = (uint16_t)((float)((uint16_t)data_lsb | ((uint16_t)data_msb << 8)) * ((float)RANGE/(float)COUNT));

   return 0;
}

/**
 * @brief Enable ISL290 light sensor.
 *
 * @return  0 if successfully, 1 if an TWI error occured
 */
uint8_t ISL290_enable()
{
   return ISL290_init(ISL290_MODE_ENABLE);
}

/**
 * @brief Disable ISL290 light sensor.
 *
 * @return  0 if successfully, 1 if an TWI error occured
 */
uint8_t ISL290_disable()
{
   return ISL290_init(ISL290_MODE_DISABLE);
}
