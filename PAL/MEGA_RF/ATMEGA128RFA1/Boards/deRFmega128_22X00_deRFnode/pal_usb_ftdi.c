/**
 * @file pal_usb_ftdi.c
 *
 * @brief PAL USB (FTDI) related functions for the ATmega128RFA1.
 *
 * This file implements USB (FTDI) related transmission and reception
 * functions for deRFmega128 radio module on deRFnode boards.
 * It was required to move this file away from the generic FTDI implementation
 * since on our board the 8 address/data lines are split over several ports.
 *
 * $Id: pal_usb_ftdi.c,v 1.1.6.1 2010/09/07 17:38:24 dam Exp $
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

#include "pal_boardtypes.h"

#if (BOARD_TYPE == deRFmega128_22X00_deRFnode)

#include <string.h>
#include "pal.h"
#include "return_val.h"
#include "pal_usb.h"
#include "app_config.h"


/* === Macros =============================================================== */

/*
 * This is the time interval in us after which the USB Tx is considered busy
 * and all USb tx handling will be suspended.
 */
#define USB_TX_BUSY_TIME            (5000)

/*
 * Various helper macros for the individual IO ports related to
 * attaching the FT245.
 */
/* set all portpins to input */
#define USB_DATA_SETINP() do {             \
    USB_D0_DDR &= ~_BV(USB_D0_PIN);        \
    USB_D1_DDR &= ~_BV(USB_D1_PIN);        \
    USB_D2_DDR &= ~_BV(USB_D2_PIN);        \
    USB_D3_DDR &= ~_BV(USB_D3_PIN);        \
    USB_D4_DDR &= ~_BV(USB_D4_PIN);        \
    USB_D5_DDR &= ~_BV(USB_D5_PIN);        \
    USB_D6_DDR &= ~_BV(USB_D6_PIN);        \
    USB_D7_DDR &= ~_BV(USB_D7_PIN);        \
} while(0)

/* set all portpins to output */
#define USB_DATA_SETOUTP() do {           \
    USB_D0_DDR |= _BV(USB_D0_PIN);        \
    USB_D1_DDR |= _BV(USB_D1_PIN);        \
    USB_D2_DDR |= _BV(USB_D2_PIN);        \
    USB_D3_DDR |= _BV(USB_D3_PIN);        \
    USB_D4_DDR |= _BV(USB_D4_PIN);        \
    USB_D5_DDR |= _BV(USB_D5_PIN);        \
    USB_D6_DDR |= _BV(USB_D6_PIN);        \
    USB_D7_DDR |= _BV(USB_D7_PIN);        \
} while(0)

/* toggle RD# to L */
#define USB_CTRL_RD_LO()    do{ USB_RD_PORT &= ~_BV(USB_RD_PIN); }while(0)

/* toggle RD# to H */
#define USB_CTRL_RD_HI()    do{ USB_RD_PORT |= _BV(USB_RD_PIN); }while(0)

/* toggle WR# to L */
#define USB_CTRL_WR_LO()    do{ USB_WR_PORT &= ~_BV(USB_WR_PIN); }while(0)

/* toggle WR# to H */
#define USB_CTRL_WR_HI()    do{ USB_WR_PORT |= _BV(USB_WR_PIN); }while(0)

/* === Globals ============================================================= */

/**
 * USB transmit buffer
 */
static uint8_t usb_tx_buf[USB_TX_BUF_SIZE];

/**
 * USB receive buffer
 */
static uint8_t usb_rx_buf[USB_RX_BUF_SIZE];

/**
 * Transmit buffer head
 */
static uint8_t usb_tx_buf_head;

/**
 * Transmit buffer tail
 */
static uint8_t usb_tx_buf_tail;

/**
 * Receive buffer head
 */
static uint8_t usb_rx_buf_head;

/**
 * Receive buffer tail
 */
static uint8_t usb_rx_buf_tail;

/**
 * Number of bytes in transmit buffer
 */
static uint8_t usb_tx_count;

/**
 * Number of bytes in receive buffer
 */
static uint8_t usb_rx_count;

/**
 * USB Tx busy flag: This is required in order to detect a Tx busy condition if
 * the terminal program is not running, so that the FTDI chip cannot deliver
 * its data.
 */
static bool usb_tx_busy = false;
/**
 * USB Tx busy time: This is required in order to store the time when a
 * USB tx busy condition has been detected. If this condition remains longer
 * than USB_TX_BUSY_TIME, USB Tx will be suspended.
 */
static uint32_t usb_tx_busy_start_time;


/* === Prototypes =========================================================== */


/* === Implementation ======================================================= */

/**
 * @brief Initializes USB port
 *
 * This function initializes the USB port. The receive and transmit interrupt
 * pins are made input pins.
 */
void sio_usb_init(void)
{
    /* Set HIGH before switching to output to prevent RD cycle */
    USB_RD_PORT |= _BV(USB_RD_PIN);
    USB_RD_DDR |= _BV(USB_RD_PIN);

    /* Set HIGH before switching to output to prevent WR cycle */
    USB_WR_PORT |= _BV(USB_WR_PIN);
    USB_WR_DDR |= _BV(USB_WR_PIN);

    /* make sure RXF and TXE are inputs */
    USB_RXF_DDR &= ~_BV(USB_RXF_PIN);
    USB_TXE_DDR &= ~_BV(USB_TXE_PIN);

    /* Flush all input data from FIFO. */
    USB_DATA_SETINP();
    while ((USB_RXF_PINS & _BV(USB_RXF_PIN)) == 0)
    {
        USB_CTRL_RD_LO();
        USB_CTRL_RD_HI();
    }
}


/**
 * @brief Copies the transmission data to the specified PAL USB buffer
 *
 * This function copies the transmission data to the PAL USB buffer.
 *
 * @param data Pointer to the buffer where the data to be transmitted is present
 * @param length Number of bytes to be transmitted
 *
 * @return number of bytes transmitted
 */
uint8_t sio_usb_tx(uint8_t *data, uint8_t length)
{
    uint8_t bytes_to_be_written;
    uint8_t size;
    uint8_t back;

    /*
     * USB Tx has been marked as busy, e.g. maybe the terminal is not open.
     * In order to continue the execution of the application even if
     * USB Tx data cannot be delivered, we are going to return here
     * immediately.
     * Once USB Tx is not busy anymore, regular USB Tx procedures will
     * be resumed.
     */
    if (usb_tx_busy)
    {
        return (length);
    }

    /*
     * Calculate available buffer space
     */
    if (usb_tx_buf_tail >= usb_tx_buf_head)
    {
        size = (USB_TX_BUF_SIZE - 1) - (usb_tx_buf_tail - usb_tx_buf_head);
    }
    else
    {
        size = (usb_tx_buf_head - 1) - usb_tx_buf_tail;
    }

    if (size < length)
    {
        /* Not enough buffer space available. Use the remaining size */
        bytes_to_be_written = size;
    }
    else
    {
        bytes_to_be_written = length;
    }

    /* Remember the number of bytes transmitted. */
    back = bytes_to_be_written;
    usb_tx_count += bytes_to_be_written;

    /* The data is copied to the transmit buffer. */
    while (bytes_to_be_written > 0)
    {
        usb_tx_buf[usb_tx_buf_tail] = *data;

        if ((USB_TX_BUF_SIZE - 1) == usb_tx_buf_tail)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            usb_tx_buf_tail = 0;
        }
        else
        {
            /* Increment the index to point the next character is to be inserted. */
            usb_tx_buf_tail++;
        }

        bytes_to_be_written--;
        data++;
    }

    /*
     * If a transmission needs to be started, pal usb_handler() takes
     * care about that once it is run.
     */

    return back;
}


/**
 * @brief Helper function for writing a byte to FTDI245
 */
static void write_byte(uint8_t data)
{
    if(data & _BV(0))    { USB_D0_PORT |= _BV(USB_D0_PIN);     }
    else                 { USB_D0_PORT &= ~_BV(USB_D0_PIN);    }

    if(data & _BV(1))    { USB_D1_PORT |= _BV(USB_D1_PIN);     }
    else                 { USB_D1_PORT &= ~_BV(USB_D1_PIN);    }

    if(data & _BV(2))    { USB_D2_PORT |= _BV(USB_D2_PIN);     }
    else                 { USB_D2_PORT &= ~_BV(USB_D2_PIN);    }

    if(data & _BV(3))    { USB_D3_PORT |= _BV(USB_D3_PIN);     }
    else                 { USB_D3_PORT &= ~_BV(USB_D3_PIN);    }

    if(data & _BV(4))    { USB_D4_PORT |= _BV(USB_D4_PIN);     }
    else                 { USB_D4_PORT &= ~_BV(USB_D4_PIN);    }

    if(data & _BV(5))    { USB_D5_PORT |= _BV(USB_D5_PIN);     }
    else                 { USB_D5_PORT &= ~_BV(USB_D5_PIN);    }

    if(data & _BV(6))    { USB_D6_PORT |= _BV(USB_D6_PIN);     }
    else                 { USB_D6_PORT &= ~_BV(USB_D6_PIN);    }

    if(data & _BV(7))    { USB_D7_PORT |= _BV(USB_D7_PIN);     }
    else                 { USB_D7_PORT &= ~_BV(USB_D7_PIN);    }
}

/*
 * @brief Puts a character onto USB FIFO
 *
 * This function transmits a byte over usb.
 */
static void usb_putc(void)
{
    if (usb_tx_count > 0)
    {
        usb_tx_count--;

        USB_DATA_SETOUTP();
        write_byte(usb_tx_buf[usb_tx_buf_head]);
        USB_CTRL_WR_LO();
        USB_CTRL_WR_HI();
        USB_DATA_SETINP();

        if ((USB_TX_BUF_SIZE - 1) == usb_tx_buf_head)
        {
            /* Reached the end of buffer, revert back to beginning of buffer */
            usb_tx_buf_head = 0;
        }
        else
        {
            usb_tx_buf_head++;
        }
    }
}


/**
 * @brief Copies the data received from USB to the user specified location
 *
 * This function copies the data received from USB to the user specified
 * location.
 *
 * @param data pointer to the buffer where the received data is to be stored
 * @param max_length maximum length of data to be received
 *
 * @return actual number of bytes received
 */
uint8_t sio_usb_rx(uint8_t *data, uint8_t max_length)
{
    uint8_t data_received = 0;

    if (usb_rx_count == 0)
    {
        /* USB receive buffer is empty. */
        return 0;
    }

    if (USB_RX_BUF_SIZE <= usb_rx_count)
    {
         /*
          * Bytes between head and tail are overwritten by new data.
          * The oldest data in buffer is the one to which the tail is
          * pointing. So reading operation should start from the tail.
          */
        usb_rx_buf_head = usb_rx_buf_tail;

        /*
         * This is a buffer overflow case. But still only bytes equivalent to
         * full buffer size are useful.
         */
        usb_rx_count = USB_RX_BUF_SIZE;

        /* Bytes received is more than or equal to buffer. */
        if (USB_RX_BUF_SIZE <= max_length)
        {
             /*
              * Requested receive length (max_length) is more than the
              * max size of receive buffer, but at max the full
              * buffer can be read.
              */
             max_length = USB_RX_BUF_SIZE;
        }
    }
    else
    {
        /* Bytes received is less than receive buffer maximum length. */
        if (max_length > usb_rx_count)
        {
            /*
             * Requested receive length (max_length) is more than the data
             * present in receive buffer. Hence only the number of bytes
             * present in receive buffer are read.
             */
            max_length = usb_rx_count;
        }
    }

    data_received = max_length;

    while (max_length > 0)
    {
        /* Start to copy from head. */
        *data = usb_rx_buf[usb_rx_buf_head];
        usb_rx_buf_head++;
        usb_rx_count--;
        data++;
        max_length--;
        if ((USB_RX_BUF_SIZE) == usb_rx_buf_head)
        {
            usb_rx_buf_head = 0;
        }
    }
    return data_received;
}


/**
 * @brief Helper function for reading a byte from FTDI245
 *
 * @return the read byte
 */
static uint8_t read_byte(void)
{
    uint8_t ret = 0;

    if(USB_D0_PINS & _BV(USB_D0_PIN)) { ret |= _BV(0); }
    if(USB_D1_PINS & _BV(USB_D1_PIN)) { ret |= _BV(1); }
    if(USB_D2_PINS & _BV(USB_D2_PIN)) { ret |= _BV(2); }
    if(USB_D3_PINS & _BV(USB_D3_PIN)) { ret |= _BV(3); }
    if(USB_D4_PINS & _BV(USB_D4_PIN)) { ret |= _BV(4); }
    if(USB_D5_PINS & _BV(USB_D5_PIN)) { ret |= _BV(5); }
    if(USB_D6_PINS & _BV(USB_D6_PIN)) { ret |= _BV(6); }
    if(USB_D7_PINS & _BV(USB_D7_PIN)) { ret |= _BV(7); }

    return ret;
}


/*
 * @brief Gets a byte from USB FIFO
 *
 * This function receives a byte from usb.
 */
static void usb_getc(void)
{
    /* The count of characters present in receive buffer is incremented. */
    usb_rx_count++;

    /* Read a character from the USB fifo. */
    USB_DATA_SETINP();
    USB_CTRL_RD_LO();
    nop();
    nop();
    usb_rx_buf[usb_rx_buf_tail] = read_byte();
    USB_CTRL_RD_HI();
    if ((USB_RX_BUF_SIZE - 1) == usb_rx_buf_tail)
    {
        /* Reached the end of buffer, revert back to beginning of buffer. */
        usb_rx_buf_tail = 0x00;
    }
    else
    {
        usb_rx_buf_tail++;
    }
}


/**
 * @brief Services data transmission or reception on USB
 *
 * This function polls for usb for completion of reception or transmission of
 * a byte on USB.
 */
void usb_handler(void)
{
    /*
     * USB Status:
     *  - TXE  write is allowed if Low
     *  - RXF  Data available if Low
     */

    if (LOW == (USB_RXF_PINS & _BV(USB_RXF_PIN)))
    {
        /* The received data is read into the PAL USB buffer */
        usb_getc();
    }

    /* Transmit data, if any */
    if (usb_tx_count > 0)
    {
        uint8_t bytes_tx = MAX_BYTES_FOR_USB_TX;

        while ((usb_tx_count > 0) && (bytes_tx > 0))
        {
            /*
             * To avoid slow transmission (byte per pal_task), a maximum of
             * MAX_BYTES_FOR_USB_TX bytes or number of bytes present in USB
             * buffer, whichever is minimum is transmitted at one go.
             */
            if (LOW == (USB_TXE_PINS & _BV(USB_TXE_PIN)))
            {
                if (usb_tx_busy)
                {
                    /*
                     * Reset USB busy handling variables and mark USB as not busy.
                     */
                    usb_tx_busy = false;
                    usb_tx_busy_start_time = 0;
                }

                usb_putc();

                bytes_tx--;
            }
            else
            {
                /*
                 * In case USB Tx is already marked busy, we can skip the
                 * USB Tx busy detection algorithm.
                 */
                if (!usb_tx_busy)
                {
                    /*
                     * We have a USB Tx busy condition.
                     * So we need to check whether we may have the situation
                     * that we cannot deliver the data to the host,
                     * e.g. because the terminal program is not open.
                     * If this happens for a certain period of time, we have to
                     * suspend data transmission via USB. All data will be
                     * discarded.
                     *
                     * Once the USB Tx is not busy anymore, we will resume
                     * USB Tx handling.
                     */
                    if (usb_tx_busy_start_time == 0)
                    {
                        /*
                         * We are in the USB Tx busy condition the first time,
                         * so we need to read the current time to have an
                         * indication when busy condition starts.
                         */
                        pal_get_current_time(&usb_tx_busy_start_time);
                    }
                    else
                    {
                        uint32_t usb_tx_busy_current_time;
                        /*
                         * Check whether the USB tx busy condition is asserted too long.
                         */
                        pal_get_current_time(&usb_tx_busy_current_time);

                        if (SUB_TIME(usb_tx_busy_current_time, usb_tx_busy_start_time) > USB_TX_BUSY_TIME)
                        {
                            /*
                             * USB Tx is busy too long, stop all data delivery for now.
                             */
                             usb_tx_busy = true;
                        }
                    }
                }
                break;  /* while ((usb_tx_count > 0) && (bytes_tx > 0)) */
            } /* !(LOW == (USB_TXE_PINS & _BV(USB_TXE_PIN))) */
        }   /* while ((usb_tx_count > 0) && (bytes_tx > 0)) */
    }   /* if (usb_tx_count > 0) */
}

#endif /* USB0 */

#endif /* (BOARD_TYPE == deRFmega128_22X00_deRFnode) */

/* EOF */
