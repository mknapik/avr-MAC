/**
 * @file i2c.c
 *
 * @brief  Implementation of a library for accessing sensors on
 *             dresden elektronik deRFnode/gateway boards.
 *
 *
 * $Id: i2c.c,v 1.1.6.1 2010/09/07 17:39:32 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 *
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

/* ==== INCLUDES =========================================================== */
#include "i2c.h"

#include <stdio.h>
#include <board.h>
#include <utility/trace.h>
#include <drivers/twi/twid.h>
#include <peripherals/twi/twi.h>
#include <aic/aic.h>
#include <pio/pio.h>


/* ==== MACROS ============================================================= */

/* ==== TYPES ============================================================== */

/* ==== EXTERNALS ========================================================== */

/* ==== VARIABLES ========================================================== */
/** buffer used for twi transmissions */
static unsigned char ucBuf[2];

/** global status variable, used in nearly all functions */
static unsigned int status=0;

/** TWI driver instance. */
static Twid twid;


/* ==== PROTOTYPES ========================================================= */
static i2c_status_t BMA150_ReadReg(unsigned int address, unsigned int* value);


/* ==== IMPLEMENTATION ===================================================== */

/**
 * @brief TWI interrupt handler.
 * Forwards the interrupt to the TWI driver handler.
 */
void ISR_Twi(void)
{
    TWID_Handler(&twid);
}


/**
 * @brief Configures the TWI interface
 */
void Configure_TWI(void)
{
    const Pin pins[] = { PINS_TWI };
    PIO_Configure(pins, PIO_LISTSIZE(pins));

    // enable twi peripheral
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TWI;
    TWI_ConfigureMaster(AT91C_BASE_TWI, BOARD_TWI_CLK, BOARD_MCK);
    // initialize the 'twid'-datastructure
    TWID_Initialize(&twid, AT91C_BASE_TWI);
    // configure and enable twi interrupt
    AIC_ConfigureIT(AT91C_ID_TWI, 0, ISR_Twi);
    AIC_EnableIT(AT91C_ID_TWI);
}


// ==== DS620 =================================================================

/**
 * @brief Configures the DS620 temperature sensor after power on.
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t DS620_Configure()
{
    TRACE_DEBUG("DS620: config start \r\n");

    // write config register
    ucBuf[0] = DS620_CR0;
    ucBuf[1] = DS620_AUTOCONV | DS620_CONTCONV | DS620_RES13BIT;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_TEMP, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("DS620: write timeout (1) \r\n");
        return I2C_TIMEOUT;
    }

    // put into continuous running mode
    ucBuf[0] = DS620_START_CONV;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_TEMP, 0x00, 0x00, ucBuf, 0x01, NULL);
    if(status!=0)
    {
        TRACE_ERROR("DS620: write timeout (2) \r\n");
        return I2C_TIMEOUT;
    }

    TRACE_DEBUG("DS620: config success \r\n");
    return I2C_SUCC;
}

/**
 * @brief Read the current temperature from the DS620.
 *
 * @param temperature the current temperature
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t DS620_Read_Temperature(float* temperature)
{
    unsigned int tL, tH;

    TRACE_DEBUG("DS620: read temp start \r\n");

    // request temperature register
    ucBuf[0] = DS620_TR0;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_TEMP, 0x00, 0x00, ucBuf, 0x01, NULL);
    if(status!=0)
    {
        TRACE_ERROR("DS620: write timeout\r\n");
        return I2C_TIMEOUT;
    }

    // either wait some time to ensure conversion really has finished (~200ms)
    // or simply let conversion running and maybe return an outdated value
    TRACE_DEBUG("DS602: [ wait -> ] read: \r\n");
    status = TWID_Read(&twid, BOARD_SENS_ADDR_TEMP, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("DS620: i2c read timeout\r\n");
        return I2C_TIMEOUT;
    }

    // successful read operation
    TRACE_DEBUG("DS620: read succ. \r\n");

    // convert read temperature value
    // MSB := degrees (1st bit: sign), negative temperatures in 2s-complement
    //             2^7 ... 2^1
    // LSB := parts of degrees (2^-1, 2^-2, ...) (only 4 most significant bits)
    //            2^0, 2^-1 ... 2^-4
    tH = ((ucBuf[0]&0x7F)<<1) + ((ucBuf[1]&0x80)>>7);     // prior decimal point
    tL = (ucBuf[1]>>3)&0x0F;                            // after decimal point

    *temperature = tH + (float)tL/(1<<4);

    return I2C_SUCC;
}


// ==== ILS29020 ==============================================================

/**
 * @brief Configures the ILS29020 luminosity sensor from power up.
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t ISL29020_Configure()
{
    TRACE_DEBUG("ILS29020: config start \r\n");

    // configuration must be done in one step, no bus release between !!
    ucBuf[0] = ISL29020_CR;
    ucBuf[1] = ISL29020_CR_ENABLE | ISL29020_CR_CONTINUOUS | ISL29020_CR_RANGE_4K_LUX;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_LIGHT, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("ILS29020 config failed \r\n");
        return I2C_TIMEOUT;
    }
    TRACE_DEBUG("ILS29020: config success \r\n");

    return I2C_SUCC;
}


/**
 * @brief Reads the luminosity from the ISL29020 sensor.
 *
 * @param luminosity the current luminosity
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t ISL29029_Read_Luminosity(unsigned int* luminosity)
{
    TRACE_DEBUG("ISL29020 read lumi start \r\n");

    // request luminosity registers
    ucBuf[0] = ISL29020_DR;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_LIGHT, 0x00, 0x00, ucBuf, 0x01, NULL);
    if(status!=0)
    {
        TRACE_ERROR("ISL29020: i2c write timeout\r\n");
        return I2C_TIMEOUT;
    }

    TRACE_DEBUG("ISL29020 read: \r\n");
    status = TWID_Read(&twid, BOARD_SENS_ADDR_LIGHT, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("ISL29020: i2c read timeout\r\n");
        return I2C_TIMEOUT;
    }

    // successful read operation
    TRACE_DEBUG("ISL29020: read succ. \r\n");

    // read: LSB | MSB
    // TODO: data conversion: out = a * lumi | a==max. resolution
    *luminosity = ucBuf[0] + (ucBuf[1]<<8);

    return I2C_SUCC;
}


// ==== BMA150 ================================================================

/**
 * @brief Initializes the BMA150 sensor
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t BMA150_Configure(void)
{
    return BMA150_Configure_Anymotion();
}

/**
 * @brief Configures BMA150 registers necessary for Anymotion detection,
 * IRQ may be enabled later.
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t BMA150_Configure_Anymotion(void)
{
    TRACE_DEBUG("BMA150: config start \r\n");

    // configure anymotion threshold
    // this is necessary since otherwise IRQ isn't resetted again
    ucBuf[0] = BMA150_ANYMOTION_THRESHOLD;
    ucBuf[1] = 0xFF;    //1200
    status = TWID_Write(&twid, BOARD_SENS_ADDR_ACCEL, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("BMA150: config failed (any_thres) \r\n");
        return I2C_TIMEOUT;
    }

    // configure anymotion duration
    ucBuf[0] = BMA150_ANYMOTION_DURATION;
    ucBuf[1] = (0x00 << 6);    //1,2 also works but less sensitivity
    status = TWID_Write(&twid, BOARD_SENS_ADDR_ACCEL, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("BMA150: config failed (any_dur) \r\n");
        return I2C_TIMEOUT;
    }

    TRACE_DEBUG("BMA150: config success \r\n");
    return I2C_SUCC;
}


/**
 * @brief Enable/disable the BMA150 to throw interrupts upon motion detection.
 *
 * @param enabled whether to switch the IRQ on or off
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t BMA150_Set_Anymotion_IRQ(bool enabled)
{
    TRACE_DEBUG("BMA150: change anymotion detection \r\n");

    // enable anymotion detection
    ucBuf[0] = BMA150_CR2;
    ucBuf[1] = enabled?BMA150_ANYMOTION_IRQ_ENABLE:0; // | BMA150_ANYMOTION_ALERT;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_ACCEL, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("BMA150: cfg failed (anymotion_irq) \r\n");
        return I2C_TIMEOUT;
    }

    // enable advanced interrupt system
    TRACE_DEBUG("BMA150: enable adv_int \r\n");
    ucBuf[0] = BMA150_CR3;
    ucBuf[1] = enabled?BMA150_ADV_IRQ_ENABLE:0;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_ACCEL, 0x00, 0x00, ucBuf, 0x02, NULL);
    if(status!=0)
    {
        TRACE_ERROR("BMA150: cfg failed (adv_int) \r\n");
        return I2C_TIMEOUT;
    }

    TRACE_DEBUG("BMA150: change IRQ success \r\n");
    return I2C_SUCC;
}


/**
 * @brief helper function, read a register byte from BMA150
 *
 * @param address    the register address,
 * @param value      returning the register value
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
static i2c_status_t BMA150_ReadReg(unsigned int address, unsigned int* value)
{
    TRACE_DEBUG("BMA150: read reg %02X start \r\n", address);

    // update the control register
    ucBuf[0] = address & 0xFF;
    status = TWID_Write(&twid, BOARD_SENS_ADDR_ACCEL, 0x00, 0x00, ucBuf, 0x01, NULL);
    if(status!=0)
    {
        TRACE_ERROR("BMA150: write failed \r\n");
        return I2C_TIMEOUT;
    }
    TRACE_DEBUG("BMA150: write succ. \r\n");

    // read out the requested register
    TRACE_DEBUG("BMA150: read start \r\n");
    status = TWID_Read(&twid, BOARD_SENS_ADDR_ACCEL, 0x00, 0x00, ucBuf, 0x01, NULL);
    if(status!=0)
    {
        TRACE_ERROR("BMA150: read failed \r\n");
        return I2C_TIMEOUT;
    }
    TRACE_DEBUG("BMA150: read succ. \r\n");

    *value = ucBuf[0] & 0xFF;
    return I2C_SUCC;
}


/**
 * @brief read out the current acceleration from BMA150
 *
 * @param ac_x    the x-axis acceleration
 * @param ac_y    the y-axis acceleration
 * @param ac_z    the z-axis acceleration
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t BMA150_Read_Acceleration(unsigned int *ac_x, unsigned int *ac_y, unsigned int *ac_z)
{
    unsigned int tmp;
    i2c_status_t status;

    // x-axis
    status = BMA150_ReadReg(BMA150_ACCRXH, &tmp);
    if(status!=I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read failed (XH) \r\n");
        return status;
    }
    *ac_x = tmp<<2;

    status = BMA150_ReadReg(BMA150_ACCRXL, &tmp);
    if(status!=I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read failed (XL) \r\n");
        return status;
    }
    *ac_x |= (tmp&0xA0)>>6;

    // y-axis
    status = BMA150_ReadReg(BMA150_ACCRYH, &tmp);
    if(status!=I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read failed (YH) \r\n");
        return status;
    }
    *ac_y = tmp<<2;

    status = BMA150_ReadReg(BMA150_ACCRYL, &tmp);
    if(status!=I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read failed (YL) \r\n");
        return status;
    }
    *ac_y |= (tmp&0xA0)>>6;

    // z-axis
    status = BMA150_ReadReg(BMA150_ACCRZH, &tmp);
    if(status!=I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read failed (ZH) \r\n");
        return status;
    }
    *ac_z = tmp<<2;

    status = BMA150_ReadReg(BMA150_ACCRZL, &tmp);
    if(status!=I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read failed (ZL) \r\n");
        return status;
    }
    *ac_z |= (tmp&0xA0)>>6;

    return I2C_SUCC;
}


/**
 * @brief Read out the temperature from BMA150.
 *
 * @param temperature the read temperature
 *
 * @return I2C_TIMEOUT in case of read/write timeout, otherwise I2C_SUCC
 */
i2c_status_t BMA150_Read_Temperature(float* temperature)
{
    unsigned int temp_raw;

    i2c_status_t status = BMA150_ReadReg(BMA150_TEMP, &temp_raw);
    if(status != I2C_SUCC)
    {
        TRACE_ERROR("BMA150: read temperature failed \r\n");
        return status;
    }

    // convert temperature:
    // 0x00 == -30°C, 0.5°C resolution
    *temperature = (((float)temp_raw)/2) - 30;

    return I2C_SUCC;
}

