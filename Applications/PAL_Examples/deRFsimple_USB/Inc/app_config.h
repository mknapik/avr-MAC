/**
 * @file app_config.h
 *
 * @brief These are application-specific resources which are used
 *        in the application in addition to the under laying stack.
 *
 * $Id: app_config.h,v 1.1.6.1 2010/09/07 17:39:31 dam Exp $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* === Includes ============================================================= */

/* === Macros =============================================================== */
/** Number of used PAL timers by this application. */

#define TOTAL_NUMBER_OF_TIMERS  (0)

/*
 * USB transmit buffer size
 */
#define USB_TX_BUF_SIZE          (127)

/*
 * USB receive buffer size
 */
#define USB_RX_BUF_SIZE          (127)

/*
 * USB-specific definitions
 */

/*
 * USB Vendor ID (16-bit number)
 */
#define USB_VID                   0x03EB /* Atmel's USB vendor ID */

/*
 * USB Product ID (16-bit number)
 */
#define USB_PID                   0x2018 /* RZ USB stick product ID */

/*
 * USB Release number (BCD format, two bytes)
 */
#define USB_RELEASE               { 0x00, 0x01 } /* 01.00 */

/*
 * Maximal number of UTF-16 characters used in any of the strings
 * below.  This is only used for compilers that cannot handle the
 * initialization of flexible array members within structs.
 */
#define USB_STRING_SIZE            (10)

/*
 * String representation for the USB vendor name.
 */
#define USB_VENDOR_NAME L"DE"

/*
 * String representation for the USB product name.
 */
#define USB_PRODUCT_NAME L"TESTSTRING"

/*
 * UART transmit buffer size
 */
#define UART_MAX_TX_BUF_LENGTH      (10)

/*
 * UART receive buffer size
 */
#define UART_MAX_RX_BUF_LENGTH      (10)

/* Offset of IEEE address storage location within EEPROM */
#define EE_IEEE_ADDR                (0)

/* === Types ================================================================ */

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_CONFIG_H */
/* EOF */
