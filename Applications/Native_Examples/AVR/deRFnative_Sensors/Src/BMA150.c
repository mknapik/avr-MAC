/**
 * @file BMA150.c
 *
 * @brief Implementation of the native BMA150 acceleration sensor interface.
 *
 * Low Level functions for BMA150 temperature sensor access on:
 *    - deRFnode
 *
 * $Id: BMA150.c,v 1.1.6.1 2010/09/07 17:39:30 dam Exp $
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

#include "BMA150.h"
#include "twi_master.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === PROTOTYPES ========================================================== */

/* === GLOBALS ============================================================= */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Initialize the BMA150 acceleration sensor.
 *
 * @return  If any error occurs '1' is returned, else '0' is returned
 */
uint8_t BMA150_init()
{
   uint8_t status;

   /* configure anymotion threshold (the higher the value, the lower the sensitivity) */
   status = twi_start(BMA150_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(BMA150_ADR_ANY_MOTION_THRES);
   if(status){return 1;}
   status = twi_write(0x08); // threshold value
   if(status){return 1;}

   /* configure anymotion duration
    * (allowed values are: 0,1,2,3 -> the higher, the less sensitivity) */
   status = twi_start(BMA150_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(BMA150_ADR_ANY_MOTION_DUR);
   if(status){return 1;}
   status = twi_write(0 << 6);
   if(status){return 1;}

   return 0;
}

/**
 * @brief Read a register byte from the BMA150.
 *
 * @param   address  the register address from which to read
 *
 * @return  -1 in case of error, otherwise the register value
 */
static int16_t BMA150_read_reg(uint8_t address)
{
   uint8_t status;

   /* update the control register */
   status = twi_start(BMA150_ADDRESS, TWI_WRITE);
   if(status){return -1;}
   status = twi_write(address & 0xFF); // send address from which to read
   if(status){return -1;}

   /* send stop + start condition */
   twi_stop();
   status = twi_start(BMA150_ADDRESS, TWI_READ);
   if(status){return -1;}

   /* read register value and return it */
   return twi_readNack();
}

/**
 * @brief Read out the acceleration values from BMA150 and calculate corresponding values
 *
 * @param   data  pointer to acceleration data structure
 *
 * @return  If any error occures a '1' is returned, else '0' is returned
 */
uint8_t BMA150_read_acc(BMA150_data_t* data)
{
   uint8_t tmp;
   uint16_t acc_z;
   uint16_t acc_y;
   uint16_t acc_x;

   /* Read Z axis acceleration values */
   tmp = BMA150_read_reg(BMA150_ADR_ACC_Z_MSB);
   if(tmp == -1) { return 1; }
   acc_z = (tmp << 2);

   tmp = BMA150_read_reg(BMA150_ADR_ACC_Z_LSB)&0xA0;
   if(tmp == -1) { return 1; }
   acc_z |= (tmp >> 6);

   /* Calculate integral and fractional digit of z axis acceleration*/
   if ((acc_z & (1 << 9)) == 0)
   {
      data->acc_z_sign = 0;
      acc_z = (uint16_t) ((float) acc_z * 0.78125);
      data->acc_z_integral = acc_z / 100;
      data->acc_z_fractional = acc_z % 100;
   }
   else
   {
      data->acc_z_sign = 1;
      acc_z &= ~(1 << 9);
      acc_z = 400 - (uint16_t) ((float) acc_z * 0.78125);
      data->acc_z_integral = acc_z / 100;
      data->acc_z_fractional = acc_z % 100;
   }

   /* Read Y axis acceleration values */
   tmp = BMA150_read_reg(BMA150_ADR_ACC_Y_MSB);
   if(tmp == -1) { return 1; }
   acc_y = (tmp << 2);

   tmp = BMA150_read_reg(BMA150_ADR_ACC_Y_LSB);
   tmp &= 0xA0;
   if(tmp == -1) { return 1; }
   acc_y |= (tmp >> 6);

   /* Calculate integral and fractional digit of y axis acceleration*/
   if ((acc_y & (1 << 9)) == 0)
   {
      data->acc_y_sign = 0;
      acc_y = (uint16_t) ((float) acc_y * 0.78125);
      data->acc_y_integral = acc_y / 100;
      data->acc_y_fractional = acc_y % 100;
   }
   else
   {
      data->acc_y_sign = 1;
      acc_y &= ~(1 << 9);
      acc_y = 400 - (uint16_t) ((float) acc_y * 0.78125);
      data->acc_y_integral = acc_y / 100;
      data->acc_y_fractional = acc_y % 100;
   }

   /* Read X axis acceleration values */
   tmp = BMA150_read_reg(BMA150_ADR_ACC_X_MSB);
   if(tmp == -1) { return 1; }
   acc_x = (tmp << 2);

   tmp = BMA150_read_reg(BMA150_ADR_ACC_X_LSB);
   tmp &= 0xA0;
   if(tmp == -1) { return 1; }
   acc_x |= (tmp >> 6);

   /* Calculate integral and fractional digit of x axis acceleration*/
   if ((acc_x & (1 << 9)) == 0)
   {
      data->acc_x_sign = 0;
      acc_x = (uint16_t) ((float) acc_x * 0.78125);
      data->acc_x_integral = acc_x / 100;
      data->acc_x_fractional = acc_x % 100;
   }
   else
   {
      data->acc_x_sign = 1;
      acc_x &= ~(1 << 9);
      acc_x = 400 - (uint16_t) ((float) acc_x * 0.78125);
      data->acc_x_integral = acc_x / 100;
      data->acc_x_fractional = acc_x % 100;
   }

   return 0;
}

/**
 * @brief Enable/disable the BMA150 throw interrupts on motion detection.
 * Enable the Anymotion detection and the Advanced Interrupt System
 *
 * @param   enableIRQ   whether to switch the IRQ on (1) or off (0)
 *
 * @return  If anymotion setting fails '1' is returned, if successfully '0' is returned
 */
uint8_t BMA150_set_anymotion_IRQ(uint8_t enableIRQ)
{
   uint8_t status;
   int16_t reg;

   /* read back control register 2, since we want to change a single bit */
   reg = BMA150_read_reg(BMA150_ADR_CTRL_2);
   if(reg == -1){ return 1; }

   uint8_t ctrl_reg_2 = (uint8_t)reg;
   /* remove anymotion bit value */
   ctrl_reg_2 &= ~BMA150_ANYMOTION_IRQ_ENABLE;
   /* set anymotion bit, depending on user selection 'enableIRQ' */
   ctrl_reg_2 |= (enableIRQ ? BMA150_ANYMOTION_IRQ_ENABLE : 0x00);

   /* write anymotion value to register */
   status = twi_start(BMA150_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(BMA150_ADR_CTRL_2);
   if(status){return 1;}
   status = twi_write(ctrl_reg_2);
   if(status){return 1;}

   /* read back control register 3, since we want to change a single bit */
   reg = BMA150_read_reg(BMA150_ADR_CTRL_3);
   if(reg == -1){ return 1; }

   uint8_t ctrl_reg_3 = (uint8_t)reg;
   /* set advanced IRQ bit */
   ctrl_reg_3 |= BMA150_ADV_IRQ_ENABLE;

   /* write advanced IRQ value to register */
   status = twi_start(BMA150_ADDRESS, TWI_WRITE);
   if(status){return 1;}
   status = twi_write(BMA150_ADR_CTRL_3);
   if(status){return 1;}
   status = twi_write(ctrl_reg_3);
   if(status){return 1;}

   return 0;
}




