/**
 * @file pal_usbd_callback.h
 *
 * $Id: pal_usbd_callbacks.c,v 1.1.6.1 2010/09/07 17:38:25 dam Exp $
 *
 * @author
 *      dresden elektronik: http://www.dresden-elektronik.de
 *      Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 * Licensed under dresden elektronik's Limited License Agreement --> deEULA.txt
 */

/* === Includes ============================================================= */

#include <usb/device/cdc-serial/CDCDSerialDriver.h>
#include <usb/device/core/USBDCallbacks.h>

//use custom descriptors
//#include <usb/device/cdc-serial/CDCDSerialDriverDescriptors.h>
#include "CDCDSerialDriverDescriptors.h"

#include <utility/led.h>

/// Use for power management
#define STATE_IDLE    0
/// The USB device is in suspend state
#define STATE_SUSPEND 4
/// The USB device is in resume state
#define STATE_RESUME  5

/* === Types ============================================================== */

/* === Globals ============================================================== */

/// State of USB, for suspend and resume
static unsigned char USBState = STATE_IDLE;


/* === Prototypes =========================================================== */

/* === Implementation ======================================================= */

//------------------------------------------------------------------------------
//         Callbacks re-implementation
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Invoked when the USB device leaves the Suspended state. By default,
/// configures the LEDs.
//------------------------------------------------------------------------------
void USBDCallbacks_Resumed(void)
{
#ifdef USB_DGB
    // Initialize LEDs
    LED_Configure(USBD_LEDPOWER);
    LED_Set(USBD_LEDPOWER);
    LED_Configure(USBD_LEDUSB);
    LED_Clear(USBD_LEDUSB);
#endif

    USBState = STATE_RESUME;
}

//------------------------------------------------------------------------------
/// Invoked when the USB device gets suspended. By default, turns off all LEDs.
//------------------------------------------------------------------------------
void USBDCallbacks_Suspended(void)
{
#ifdef USB_DGB
    // Turn off LEDs
    LED_Clear(USBD_LEDPOWER);
    LED_Clear(USBD_LEDUSB);
#endif

    USBState = STATE_SUSPEND;
}

