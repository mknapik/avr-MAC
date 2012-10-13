/**
 * @file BMA150.h
 *
 * @brief Application header file for BMA150.c.
 *
 * Low Level functions for BMA150 acceleration sensor access on:
 *    - deRFnode
 *
 * $Id: BMA150.h,v 1.1.6.1 2010/09/07 17:39:29 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

#ifndef BMA150_H_
#define BMA150_H_

/* === INCLUDES ============================================================ */

/* === MACROS / DEFINES ==================================================== */

/********************************************************************
 * BMA150 TWI/I2C Address
 *******************************************************************/

#define BMA150_ADDRESS                    (0x38) ///< TWI Address

/********************************************************************
 * BMA150 Register Map
 *******************************************************************/

#define BMA150_ADR_CHIP_ID                (0x00) ///< Chip ID Register
#define BMA150_ADR_VERSION                (0x01) ///< Version Register

#define BMA150_ADR_ACC_X_LSB              (0x02) ///< Acceleration X register (LSB part)
#define BMA150_ADR_ACC_X_MSB              (0x03) ///< Acceleration X register (MSB part)
#define BMA150_ADR_ACC_Y_LSB              (0x04) ///< Acceleration Y register (LSB part)
#define BMA150_ADR_ACC_Y_MSB              (0x05) ///< Acceleration Y register (MSB part)
#define BMA150_ADR_ACC_Z_LSB              (0x06) ///< Acceleration Z register (LSB part)
#define BMA150_ADR_ACC_Z_MSB              (0x07) ///< Acceleration Z register (MSB part)

#define BMA150_ADR_TEMP                   (0x08) ///< Temperature register

#define BMA150_ADR_ANY_MOTION_THRES       (0x10) ///< Anymotion threshold register
#define BMA150_ADR_ANY_MOTION_DUR         (0x11) ///< Anymotion duration register

#define BMA150_ADR_CTRL_1                 (0x0A) ///< Control Register 1
#define BMA150_ADR_CTRL_2                 (0x0B) ///< Control Register 2
#define BMA150_ADR_CTRL_3                 (0x15) ///< Control Register 3


#define BMA150_ANYMOTION_IRQ_ENABLE       (0x40) ///< Anymotion IRQ enable bit
#define BMA150_ADV_IRQ_ENABLE             (0x40) ///< Advanced IRQ enable bit

/********************************************************************
 * BMA150 Defines
 *******************************************************************/

#define BMA150_IRQ_ENABLE                 (1)    ///< enable IRQ
#define BMA150_IRQ_DISABLE                (0)    ///< disable IRQ

/**
 * Define a structure where acceleration values can be saved to.
 */
typedef struct
{
   uint8_t  acc_x_sign;          ///< is acc_x value positive (0) or negative (1)?
   uint8_t  acc_x_integral;      ///< integral digit of acc_x
   uint8_t  acc_x_fractional;    ///< fractional digit of acc_x
   uint8_t  acc_y_sign;          ///< is acc_y value positive (0) or negative (1)?
   uint8_t  acc_y_integral;      ///< integral digit of acc_y
   uint8_t  acc_y_fractional;    ///< fractional digit of acc_y
   uint8_t  acc_z_sign;          ///< is acc_z value positive (0) or negative (1)?
   uint8_t  acc_z_integral;      ///< integral digit of acc_z
   uint8_t  acc_z_fractional;    ///< fractional digit of acc_z
} BMA150_data_t;


/* === TYPES ================================================================ */

/* === PROTOTYPES =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

uint8_t BMA150_init();
uint8_t BMA150_read_acc(BMA150_data_t* data);
uint8_t BMA150_set_anymotion_IRQ(uint8_t enableIRQ);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */


#endif /* BMA150_H_ */
