/**
 * @file CDCDSerialDriverDescriptors.h
 *
 * @brief header file for CDCDSerialDriverDescriptors.c
 *
 * Definition of the USB descriptors required by a CDC device serial
 driver.
 *
 *
 * $Id: CDCDSerialDriverDescriptors.h,v 1.1.4.1 2010/09/07 17:39:32 dam Exp $
 *
 *  @author
 *      dresden elektronik: http://www.dresden-elektronik.de
 *      Support email: support@dresden-elektronik.de
 *
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 * Licensed under dresden elektronik's Limited License Agreement --> deEULA.txt
 */

#ifndef CDCDSERIALDRIVERDESCRIPTORS_H
#define CDCDSERIALDRIVERDESCRIPTORS_H

/* === Includes ============================================================ */
#include <usb/device/core/USBDDriverDescriptors.h>


/* === Macros =============================================================== */

/* Endpoint numbers used for the CDC-Serial Device */
#define CDCDSerialDriverDescriptors_DATAOUT             1
#define CDCDSerialDriverDescriptors_DATAIN              2
#define CDCDSerialDriverDescriptors_NOTIFICATION        3

/* === Types ================================================================ */

/* === Globals ============================================================== */

/* List of descriptors for a CDC device serial driver. */
extern USBDDriverDescriptors cdcdSerialDriverDescriptors;

/* === Prototypes =========================================================== */

/* === Implementation ======================================================= */

#endif //#ifndef CDCDDRIVERDESCRIPTORS_H

/* EOF */
