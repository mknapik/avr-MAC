/**
 * @file pal_usb.c
 *
 * @brief Functions for native USB support on ARM-platforms.
 *
 * This file implements functionality for handling native USB
 * on ARM platforms.
 *
 * $Id: pal_usb.c,v 1.1.8.1 2010/09/07 17:38:25 dam Exp $
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

#ifdef USB0

/* === Includes ============================================================ */

#include "pal.h"
#include "pal_usb.h"
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <utility/trace.h>
#include <string.h>

//USB includes from AT91Lib
#include <usb/device/core/USBD.h>
#include <usb/device/cdc-serial/CDCDSerialDriver.h>
#include <usb/device/core/USBDDriver.h>
#include <usb/common/cdc/CDCLineCoding.h>
#include <usb/common/cdc/CDCGenericRequest.h>
#include <usb/common/cdc/CDCSetControlLineStateRequest.h>


/* === Macros =============================================================== */

/* === Globals ============================================================== */

static uint8_t usb_0_tx_temp_buf[USB_TX_BUF_MAX_SIZE];
static uint8_t usb_0_rx_temp_buf[USB_RX_BUF_MAX_SIZE];

static uint8_t usb_0_rx_disable_flag;

static usb_communication_buffer_t usb_0_buffer;


/* === Prototypes =========================================================== */

static void usb0_tx_complete_handler(   unsigned int unused,
                                        unsigned char status,
                                        unsigned int transferred,
                                        unsigned int remaining);

static void usb0_rx_complete_handler(   unsigned int unused,
                                        unsigned char status,
                                        unsigned int received,
                                        unsigned int remaining);

/* === Implementation ======================================================= */

#if defined(PIN_USB_VBUS)
// VBus pin instance.
static const Pin pinVbus = PIN_USB_VBUS;

// optional VBus monitoring
#define VBUS_CONFIGURE()  VBus_Configure()
#else
#define VBUS_CONFIGURE()    USBD_Connect()
#endif /* PIN_USB_VBUS */

#if defined(PIN_USB_VBUS)
/**
 * @brief Handles interrupts coming from PIO controllers.
 */
static void ISR_Vbus(const Pin *pPin)
{
    /* Check current level on VBus */
    if (PIO_Get(&pinVbus))
    {
        TRACE_INFO("VBUS conn\n\r");
        USBD_Connect();
    }
    else
    {
        TRACE_INFO("VBUS discon\n\r");
        USBD_Disconnect();
    }
}


/**
 * @brief Configures the VBus pin to trigger an interrupt when the level on
 * that pin changes.
 */
static void VBus_Configure( void )
{
    TRACE_INFO("VBus configuration\n\r");

    // Configure PIO
    PIO_Configure(&pinVbus, 1);
    PIO_ConfigureIt(&pinVbus, ISR_Vbus);
    PIO_EnableIt(&pinVbus);

    // Check current level on VBus
    if (PIO_Get(&pinVbus)) {

        // if VBUS present, force the connect
        TRACE_INFO("VBUS conn\n\r");
        USBD_Connect();
    } else {
        USBD_Disconnect();
        TRACE_INFO("VBUS DISconn\n\r");
    }
}
#endif



/**
 * @brief Bytes free in the buffer
 *
 * This function computes the free buffer size in a cyclic buffer.
 *
 * @param head          index of the head
 * @param tail          index of the tail
 * @param max_buf_size  maximum size of the used buffer
 *
 * @return Number of bytes available in the buffers
 */
uint8_t get_buffer_size(uint8_t head, uint8_t tail, uint8_t max_buf_size)
{
    uint8_t size;
    if (tail >= head)
    {
        size = (max_buf_size) - (tail - head);
    }
    else
    {
        size = (head - 1) - tail;
    }

    return size;
}



/**
 * @brief Initializes USB 0
 *
 * This function initializes the USB0 Channel and the ring buffer parameters.
 *
 */
void sio_usb_0_init(void)
{
    usb_0_buffer.tx_buf_head = 0;
    usb_0_buffer.tx_buf_tail = 0;
    usb_0_buffer.tx_count = 0;

    usb_0_buffer.rx_buf_head = 0;
    usb_0_buffer.rx_buf_tail = 0;
    usb_0_buffer.rx_count = 0;

    usb_0_rx_disable_flag = 1; //disable rx for startup

    /* CDC serial driver initialization */
    CDCDSerialDriver_Initialize();

    /* connect if needed */
    VBUS_CONFIGURE();

    while (USBD_GetState() < USBD_STATE_CONFIGURED)
        ;

    CDCDSerialDriver_Read(usb_0_rx_temp_buf, sizeof(usb_0_rx_temp_buf),
            (TransferCallback) usb0_rx_complete_handler, 0);
}



/**
 * @brief Transmit data via USB 0
 *
 * This function transmits data via USB channel 0.
 *
 * @param data Pointer to the buffer where the data to be transmitted is present
 * @param length Number of bytes to be transmitted
 *
 * @return Number of bytes actually transmitted
 */
uint8_t sio_usb_0_tx(uint8_t *data, uint8_t length)
{
    uint8_t bytes_to_be_written = 0;
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t size = 0;
    uint8_t back = 0;

    uint8_t tx_length = 0;
    uint8_t i = 0;

    /* The transmit interrupt is disabled. */
    //ENTER_CRITICAL_REGION();

    /*
     * Initialize head and tail
     */
    head = usb_0_buffer.tx_buf_head;
    tail = usb_0_buffer.tx_buf_tail;

    size = get_buffer_size( usb_0_buffer.tx_buf_head,
                            usb_0_buffer.tx_buf_tail,
                            USB_TX_BUF_MAX_SIZE);

    if (size < length)
    {
        /* Not enough buffer space available. Use the remaining size. */
        bytes_to_be_written = size;
    }
    else
    {
        bytes_to_be_written = length;
    }

    /* Remember the number of bytes transmitted. */
    back = bytes_to_be_written;

    /* The data is copied to the transmit buffer. */
    while (bytes_to_be_written > 0)
    {
        usb_0_buffer.tx_buf[usb_0_buffer.tx_buf_tail] = *data;

        if ((USB_TX_BUF_MAX_SIZE - 1) == usb_0_buffer.tx_buf_tail)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            usb_0_buffer.tx_buf_tail = 0;
        }
        else
        {
            /*
             * Increment the index to point the next character to be
             * inserted.
             */
            usb_0_buffer.tx_buf_tail++;
        }

        bytes_to_be_written--;
        data++;
    }

    /*
     * Check whether there is a transmission ongoing. Otherwise write
     * data into the UART data register. Transmission of subsequent
     * bytes / data will be taken care in the ISR.
     */
    if (usb_0_buffer.tx_count == 0)
    {
        /*
         * Prepare a linear buffer for submitting data
         */
        head = usb_0_buffer.tx_buf_head;
        tail = usb_0_buffer.tx_buf_tail;

        while (head != tail)
        {
            usb_0_tx_temp_buf[i] = usb_0_buffer.tx_buf[head];
            if (i == USB_TX_MAX_SIZE) //cannot transmit more than USB_TX_MAX_SIZE at once
                break;
            if ((head) == (USB_TX_BUF_MAX_SIZE - 1))
                head = 0;
            else
                head++;
            i++;
        }
        tx_length = i;

        /* The transmit interrupt is enabled. */
        //LEAVE_CRITICAL_REGION();

        usb_0_buffer.tx_count = 1;
        while (CDCDSerialDriver_Write(usb_0_tx_temp_buf, tx_length,
                (TransferCallback) usb0_tx_complete_handler, 0)
                != USBD_STATUS_SUCCESS)
            ;

    }

    return back;
}



/**
 * @brief Receives data from USB0
 *
 * This function receives data from USB0
 *
 * @param data pointer to the buffer where the received data will be stored
 * @param max_length maximum length of data to be received
 *
 * @return actual number of bytes received
 */
uint8_t sio_usb_0_rx(uint8_t *data, uint8_t max_length)
{
    uint8_t data_received = 0;
    uint8_t size = 0;

    if (usb_0_buffer.rx_count == 0)
    {
        /* usb receive buffer is empty. */
        return 0;
    }

    /* The receive interrupt is disabled. */
    ENTER_CRITICAL_REGION();

    if (USB_RX_BUF_MAX_SIZE <= usb_0_buffer.rx_count)
    {
        /*
         * Bytes between head and tail are overwritten by new data.
         * The oldest data in buffer is the one to which the tail is
         * pointing. So reading operation should start from the tail.
         */
        usb_0_buffer.rx_buf_head = usb_0_buffer.rx_buf_tail;

        /*
         * This is a buffer overflow case. But still only bytes equivalent to
         * full buffer size are useful.
         */
        usb_0_buffer.rx_count = USB_RX_BUF_MAX_SIZE;

        /* Bytes received is more than or equal to buffer. */
        if (USB_RX_BUF_MAX_SIZE <= max_length)
        {
            /*
             * Requested receive length (max_length) is more than the
             * max size of receive buffer, but at max the full
             * buffer can be read.
             */
            max_length = USB_RX_BUF_MAX_SIZE;
        }
    }
    else
    {
        /* Bytes received is less than receive buffer maximum length. */
        if (max_length > usb_0_buffer.rx_count)
        {
            /*
             * Requested receive length (max_length) is more than the data
             * present in receive buffer. Hence only the number of bytes
             * present in receive buffer are read.
             */
            max_length = usb_0_buffer.rx_count;
        }
    }

    data_received = max_length;

    while (max_length > 0)
    {
        /* Start to copy from head. */
        *data = usb_0_buffer.rx_buf[usb_0_buffer.rx_buf_head];
        usb_0_buffer.rx_buf_head++;
        usb_0_buffer.rx_count--;
        data++;
        max_length--;
        if ((USB_RX_BUF_MAX_SIZE) == usb_0_buffer.rx_buf_head)
        {
            usb_0_buffer.rx_buf_head = 0;
        }
    }

    size = get_buffer_size( usb_0_buffer.rx_buf_head,
                            usb_0_buffer.rx_buf_tail,
                            USB_RX_BUF_MAX_SIZE);

    /* The receive interrupt is enabled back. */
    LEAVE_CRITICAL_REGION();

    if (size > 60)
    {
        CDCDSerialDriver_Read(  usb_0_rx_temp_buf,
                                sizeof(usb_0_rx_temp_buf),
                                (TransferCallback) usb0_rx_complete_handler,
                                0);

        usb_0_rx_disable_flag = 0;
    }

    return data_received;
}



/*
 * This function is called when the USB transfer from the
 * device to the host is finished.
 */
static void usb0_tx_complete_handler(   unsigned int unused,
                                        unsigned char status,
                                        unsigned int transferred,
                                        unsigned int remaining)
{
    ENTER_CRITICAL_REGION();
    while (transferred--)
    {
        if ((USB_TX_BUF_MAX_SIZE - 1) == usb_0_buffer.tx_buf_head)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            usb_0_buffer.tx_buf_head = 0;
        }
        else
        {
            /*
             * * Increment the index to point the next character to be
             * * transmitted.
             * */
            usb_0_buffer.tx_buf_head++;
        }
    }

    if (usb_0_buffer.tx_buf_head != usb_0_buffer.tx_buf_tail)
    {
        /*
         * Prepare a linear buffer for submitting data
         * remaining data in the buffer
         */
        uint8_t tx_length = 0;
        uint8_t i = 0;
        uint8_t head = 0;
        uint8_t tail = 0;
        head = usb_0_buffer.tx_buf_head;
        tail = usb_0_buffer.tx_buf_tail;
        while (head != tail)
        {
            usb_0_tx_temp_buf[i] = usb_0_buffer.tx_buf[head];
            if (i == (USB_TX_MAX_SIZE - 1)) //cannot transmit more than USB_TX_MAX_SIZE at once
                break;
            if ((head) == (USB_TX_BUF_MAX_SIZE - 1))
                head = 0;
            else
                head++;
            i++;
        }
        tx_length = i;
        usb_0_buffer.tx_count = 1;
        while (CDCDSerialDriver_Write(usb_0_tx_temp_buf,
                                tx_length,
                                (TransferCallback) usb0_tx_complete_handler,
                                0)
                != USBD_STATUS_SUCCESS)
            ;
    } else
    {
        /* No more data for transmission */
        usb_0_buffer.tx_count = 0;
    }
    LEAVE_CRITICAL_REGION();
}



/*
 * This function is called when the USB transfer from the host to the
 * device is finished
 */
static void usb0_rx_complete_handler(   unsigned int unused,
                                        unsigned char status,
                                        unsigned int received,
                                        unsigned int remaining)
{
    uint8_t tail = usb_0_buffer.rx_buf_tail;
    uint8_t i = 0;

    ENTER_CRITICAL_REGION();
    while (received--)
    {
        /* Count of bytes received through USB 0 channel is incremented. */
        usb_0_buffer.rx_count++;
        usb_0_buffer.rx_buf[tail] = usb_0_rx_temp_buf[i];
        i++;
        if ((USB_RX_BUF_MAX_SIZE - 1) == usb_0_buffer.rx_buf_tail)
        {
            /* Revert back to beginning of buffer after reaching end of buffer. */
            usb_0_buffer.rx_buf_tail = 0;
            tail = 0;
        }
        else
        {
            usb_0_buffer.rx_buf_tail++;
            tail++;
        }
    }
    LEAVE_CRITICAL_REGION();
}

#endif /* USB0 */

/* EOF */
