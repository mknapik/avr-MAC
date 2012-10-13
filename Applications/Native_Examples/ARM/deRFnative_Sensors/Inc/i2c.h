/**
 * @file i2c.h
 *
 * @brief  Header file for i2c.c
 *
 * Provides Register bit definitions for the three sensors.
 *
 *
 * $Id: i2c.h,v 1.1.6.1 2010/09/07 17:39:38 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 *
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

#ifndef I2C_H_
#define I2C_H_

/* ==== INCLUDES =========================================================== */
#include <stdbool.h>

/* ==== EXTERNALS ========================================================== */

/* ==== MACROS ============================================================= */
#define DS620_CR0                    (0xAC)    // config register 0
#define DS620_TR0                    (0xAA)    // temp. register 0
#define DS620_START_CONV             (0x51)    // start conversion command

#define DS620_AUTOCONV               (0x02)    // auto-conversion
#define DS620_RES13BIT               (0x0A)    // 13bit temperature resolution
#define DS620_CONTCONV               (0x00)    // continuous temperature conversion
#define DS620_ONESHOOT               (0x00)    // temperature conversion only upon request

#define ISL29020_CR                  (0x00)    // configuration register
#define ISL29020_DR                  (0x01)    // data register, MSB

#define ISL29020_CR_ENABLE           (0x80)    // conf. reg. bit values
#define ISL29020_CR_CONTINUOUS       (0x40)    // continuous conversion mode
#define ISL29020_CR_IR_SENSING       (0x20)    // infrared sensing
#define ISL29020_CR_RES_4BIT         (0x0A)    // 4bit resolution
#define ISL29020_CR_RES_8BIT         (0x08)    // 8bit resolution
#define ISL29020_CR_RES_12BIT        (0x04)    // 12bit resolution
#define ISL29020_CR_RES_16BIT        (0x00)    // 16bit resolution
#define ISL29020_CR_RANGE_1K_LUX     (0x00)    // sensing range 0...1K lux
#define ISL29020_CR_RANGE_4K_LUX     (0x01)    // sensing range 0...4K lux
#define ISL29020_CR_RANGE_16K_LUX    (0x02)    // sensing range 0...16K lux
#define ISL29020_CR_RANGE_64K_LUX    (0x03)    // sensing range 0...64K lux

#define BMA150_VER1                  (0x00)    // version register 1
#define BMA150_VER2                  (0x01)    // version register 2
#define BMA150_TEMP                  (0x08)    // temperature register
#define BMA150_SR                    (0x09)    // status register
#define BMA150_CR1                   (0x0A)    // ctrl register 1
#define BMA150_CR2                   (0x0B)    // ctrl register 2
#define BMA150_CR3                   (0x15)    // ctrl register 3
#define BMA150_ACCRZH                (0x07)    // acceleration register, Z-axis, upper part
#define BMA150_ACCRZL                (0x06)    // acceleration register, Z-axis, lower part
#define BMA150_ACCRYH                (0x05)    // acceleration register, Y-axis, upper part
#define BMA150_ACCRYL                (0x04)    // acceleration register, Y-axis, lower part
#define BMA150_ACCRXH                (0x03)    // acceleration register, X-axis, upper part
#define BMA150_ACCRXL                (0x02)    // acceleration register, X-axis, lower part

#define BMA150_ADV_IRQ_ENABLE        (0x40)    // advanced IRQ enable (see 3.2.3)
#define BMA150_ANYMOTION_IRQ_ENABLE  (0x40)    // any motion generates an interrupt (see 3.2.4)
#define BMA150_LG_IRQ_ENABLE         (0x01)    // low gravity will generate interrupts (see 3.2.1)
#define BMA150_HG_IRQ_ENABLE         (0x02)    // high gravity will generate interrupts (see 3.2.2)
#define BMA150_ANYMOTION_ALERT       (0x80)    // any motion sets the BMA150 into alert mode
                                               // this _cannot_ be turned on simultaneously with
                                               // BMA150_ANYMOTION_IRQ_ENABLE (see. 3.2.5)
#define BMA150_NEWDATA_INT_ENABLE    (0x20)    // new acceleration values will generate an interrupt

#define BMA150_ANYMOTION_THRESHOLD   (0x10)
#define BMA150_ANYMOTION_DURATION    (0x11)
#define BMA150_EE_W                  (0x10)    // enable eeprom write access
#define BMA150_RESET_INT             (0x40)    // reset irq flag


/* ==== TYPES ============================================================== */
typedef enum {

    I2C_SUCC,
    I2C_TIMEOUT

} i2c_status_t;


/* ==== PROTOTYPES ========================================================= */
void Configure_TWI(void);

i2c_status_t DS620_Configure(void);
i2c_status_t DS620_Read_Temperature(float* temperature);

i2c_status_t ISL29020_Configure(void);
i2c_status_t ISL29029_Read_Luminosity(unsigned int* luminosity);

i2c_status_t BMA150_Configure(void);
i2c_status_t BMA150_Configure_Anymotion(void);
i2c_status_t BMA150_Set_Anymotion_IRQ(bool on);
i2c_status_t BMA150_Read_Acceleration(unsigned int *ac_x, unsigned int *ac_y, unsigned int *ac_z);
i2c_status_t BMA150_Read_Temperature(float* temperature);

#endif /* I2C_H_ */
