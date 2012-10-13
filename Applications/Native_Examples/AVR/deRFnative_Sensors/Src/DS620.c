/**
 * @file DS620.c
 *
 * @brief Implementation of the native DS620 temperature sensor interface.
 *
 * Low Level functions for DS620 temperature sensor access on:
 *    - deRFnode
 *
 * $Id: DS620.c,v 1.1.6.1 2010/09/07 17:39:30 dam Exp $
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
#include "DS620.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ============================================================= */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Initialize the DS620
 *
 * @param   config_msb  MSB of configuration register
 * @param   config_lsb  LSB of configuration register
 *
 * @return     0 if initialization started, 1 if any error on TWI interface occured
 */
uint8_t DS620_init(uint8_t config_msb, uint8_t config_lsb)
{
   uint8_t status;

   status = twi_start(DS620_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(DS620_ADR_CONFIG_MSB);
   if(status){return 1;}
   status = twi_write(config_msb);
   if(status){return 1;}
   status = twi_write(config_lsb);
   if(status){return 1;}

   return 0;
}

/**
 * @brief start conversion on DS620 temperature sensor
 *
 * @return     0 if measurement started, 1 if any error on TWI interface occured
 */
uint8_t DS620_start_measurement(void)
{
   uint8_t status = 0;

   status = twi_start(DS620_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(DS620_CMD_START_CONVERT);
   if(status){return 1;}
   return 0;
}

/**
 * @brief Read temperature of DS620
 *
 * @param   temperature pointer to structure that hold temperature values
 *
 * @return  If any error occurs '1' is returned, else '0'
 */
uint8_t DS620_read_temperature(DS620_temperature_t* temperature)
{
   uint8_t status = 0;

   /* set start address which to read */
   status = twi_start(DS620_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(DS620_ADR_TEMP_MSB);
   if(status){return 1;}

   /* send stop + start condition to start read from DS620 */
   twi_stop();
   status = twi_start(DS620_ADDRESS, TWI_READ);
   if(status){return 1;}

   /* read back temperature and configuration registers */
   uint8_t temp_msb = twi_readAck();
   uint8_t temp_lsb = twi_readAck();
   uint8_t cfg_msb = twi_readNack();
   //uint8_t cfg_lsb = TWIM_ReadNack();

   /*
    * Temperature Calculation
    *
    *  data_reg_msb = temp_msb
    *    BIT   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    *    VALUE |  S  | 2^7 | 2^6 | 2^5 | 2^4 | 2^3 | 2^2 | 2^1 |
    *
    *  data_reg_lsb = temp_lsb
    *    BIT   |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    *    VALUE | 2^0 | 2^-1| 2^-2| 2^-3| 2^-4|  0  |  0  |  0  |
    *
    */
   temperature->conversion_done = ((cfg_msb & DS620_CFG_DONE) == 0x80) ? 1 : 0;
   temperature->sign = ((temp_msb & 0x80) == 0x80) ? 1 : 0;
   temperature->integralDigit = (temp_msb << 1) + ((temp_lsb & 0x80) >> 7);
   temperature->fractionalDigit = (uint16_t)((float)(temp_lsb >> 3) * 6.25) % 100;

   return 0;
}

/*
 * EOF
 */
