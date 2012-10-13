/**
 * @file twi_master.h
 *
 * @brief Application header file for twi_master.c.
 *
 * This provides functions to access any TWI device, when MCU should
 * be act as master and device as slave.
 *
 * $Id: twi_master.h,v 1.1.6.1 2010/09/07 17:39:29 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, Dresden Elektronik All rights reserved.
 *
 * Licensed under Dresden Elektronik Limited License Agreement --> deEULA.txt
 */

#ifndef TWI_MASTER_H_
#define TWI_MASTER_H_

/* === Includes ============================================================= */

/* === Macros =============================================================== */

/****************************************************************************
  TWI Control definitions
****************************************************************************/
#define TWI_READ                 1     ///< send read condition
#define TWI_WRITE                0     ///< send write condition

/****************************************************************************
  TWI State codes
****************************************************************************/

// General TWI Master staus codes
#define TWI_START                0x08  ///< START has been transmitted
#define TWI_REP_START            0x10  ///< Repeated START has been transmitted
#define TWI_ARB_LOST             0x38  ///< Arbitration lost

// TWI Master Transmitter staus codes
#define TWI_MT_SLA_ACK           0x18  ///< SLA+W has been tramsmitted and ACK received
#define TWI_MT_SLA_NACK          0x20  ///< SLA+W has been tramsmitted and NACK received
#define TWI_MT_DATA_ACK          0x28  ///< Data byte has been tramsmitted and ACK received
#define TWI_MT_DATA_NACK         0x30  ///< Data byte has been tramsmitted and NACK received

// TWI Master Receiver staus codes
#define TWI_MR_SLA_ACK           0x40  ///< SLA+R has been tramsmitted and ACK received
#define TWI_MR_SLA_NACK          0x48  ///< SLA+R has been tramsmitted and NACK received
#define TWI_MR_DATA_ACK          0x50  ///< Data byte has been received and ACK tramsmitted
#define TWI_MR_DATA_NACK         0x58  ///< Data byte has been received and NACK tramsmitted

/* === Globals ============================================================== */

/* === Prototypes =========================================================== */

uint8_t twi_init(uint32_t twi_bitrate);
uint8_t twi_start(uint8_t address, uint8_t mode);
void twi_stop(void);
uint8_t twi_write(uint8_t byte);
uint8_t twi_readAck(void);
uint8_t twi_readNack(void);

/* === Implementation ======================================================= */

#endif /* TWI_MASTER_H_ */

/*
 * EOF
 */

