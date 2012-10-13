/**
 * @file pal_usb.h
 *
 * @brief PAL usb internal functions prototypes
 *
 * This header contains the function prototypes for handling native USB
 * on ARM platforms.
 *
 * $Id: pal_usb.h,v 1.1.8.1 2010/09/07 17:38:25 dam Exp $
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

/* prevent double inclusion */
#ifndef PAL_USB_H
#define PAL_USB_H

#if defined USB0

/* === Includes ============================================================= */

#include "pal.h"
#include "pal_config.h"
#include "app_config.h"

/* === Externals ============================================================ */

/* === Macros =============================================================== */

/*
 * USB buffer sizes
 */
#define USB_TX_BUF_MAX_SIZE 128
#define USB_RX_BUF_MAX_SIZE 128
//#define USB_TX_BUF_MAX_SIZE            USB_TX_BUF_SIZE
//#define USB_RX_BUF_MAX_SIZE            USB_RX_BUF_SIZE

#ifdef USE_FTDI_USB
/*
 * Max bytes transmitted over USB at one instance;
 * Used for FTDI only; FTDI's tx buffer size is 384 bytes,
 * but an uint8_t variable is used. Therefore, the value is reduced to 255.
 */
#define MAX_BYTES_FOR_USB_TX    (255)
#endif

/* Size of the Endpoint */
#define USB_TX_MAX_SIZE 64

/* === Types ================================================================ */

/*
 * Structure containing the transmit and receive buffer
 * and also the buffer head, tail and count
 */
typedef struct usb_communication_buffer_tag
{
    /* Transmit buffer */
    uint8_t tx_buf[USB_TX_BUF_MAX_SIZE];

    /* Receive buffer */
    uint8_t rx_buf[USB_RX_BUF_MAX_SIZE];

    /* Transmit buffer head */
    uint8_t tx_buf_head;

    /* Transmit buffer tail */
    uint8_t tx_buf_tail;

    /* Receive buffer head */
    uint8_t rx_buf_head;

    /* Receive buffer tail */
    uint8_t rx_buf_tail;

    /* Number of bytes in transmit buffer */
    uint8_t tx_count;

    /* Number of bytes in receive buffer */
    uint8_t rx_count;

} usb_communication_buffer_t;

/* === Prototypes =========================================================== */

void sio_usb_0_init(void);
uint8_t sio_usb_0_tx(uint8_t *data, uint8_t length);
uint8_t sio_usb_0_rx(uint8_t *data, uint8_t max_length);

#ifdef USE_FTDI_USB
void usb_handler(void);
#endif

#endif /* defined USB0 */

#endif /*PAL_USB_H*/
/* EOF */
