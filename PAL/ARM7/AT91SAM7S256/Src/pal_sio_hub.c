/**
 * @file pal_sio_hub.c
 *
 * @brief Stream I/O API functions
 *
 * This file implements the Stream I/O API functions.
 *
 * $Id: pal_sio_hub.c,v 1.1.8.1 2010/09/07 17:38:26 dam Exp $
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


/* === Includes ============================================================ */
//#ifdef SIO_HUB

#include <stdint.h>
#include "pal.h"
#include "return_val.h"
#include "pal_uart.h"
#include "pal_usb.h"

/* === Globals =============================================================*/

/* === Prototypes ==========================================================*/

/* === Implementation ======================================================*/

/**
 * @brief Initializes the requested SIO unit
 *
 * This function initializes the requested SIO unit.
 *
 * @param sio_unit Specifies the SIO uint to be initialized
 *
 * @return MAC_SUCCESS  if SIO unit is initialized successfully, FAILURE
 *         otherwise
 */
retval_t pal_sio_init(uint8_t sio_unit)
{
    retval_t status = MAC_SUCCESS;

#ifdef SIO_HUB
/* allow applications stay untouched while working with DBGU */

    switch (sio_unit)
    {
#ifdef UART0
        case SIO_0: sio_uart_0_init(UART_BAUD_9k6);
                    break;
#endif
#ifdef UART1
        case SIO_1: sio_uart_1_init(UART_BAUD_9k6);
                    break;
#endif
#ifdef USB0
        case SIO_2: sio_usb_0_init();
        break;
#endif
        default:    status = FAILURE;
                    break;
    }
#endif /* SIO_HUB */

    return status;
}



/**
 * @brief Transmits data through selected SIO unit
 *
 * This function transmits data through the selected SIO unit.
 *
 * @param sio_unit Specifies the SIO unit
 * @param data Pointer to the data to be transmitted is present
 * @param length Number of bytes to be transmitted
 *
 * @return Actual number of transmitted bytes
 */
uint8_t pal_sio_tx(uint8_t sio_unit, uint8_t *data, uint8_t length)
{
    uint8_t number_of_bytes_transmitted;

    switch (sio_unit)
    {
#ifdef UART0
        case SIO_0: number_of_bytes_transmitted = sio_uart_0_tx(data, length);
                    break;
#endif
#ifdef UART1
        case SIO_1: number_of_bytes_transmitted = sio_uart_1_tx(data, length);
                    break;
#endif
#ifdef USB0
        case SIO_2: number_of_bytes_transmitted = sio_usb_0_tx(data, length);
                break;
#endif

        default:    number_of_bytes_transmitted = 0;
                    break;
    }
    return (number_of_bytes_transmitted);
}



/**
 * @brief Receives data from selected SIO unit
 *
 * This function receives data from the selected SIO unit.
 *
 * @param[in] sio_unit Specifies SIO unit
 * @param[out] data Pointer to the buffer to store received data
 * @param[in] max_length Maximum number of bytes to be received
 *
 * @return Actual number of received bytes
 */
uint8_t pal_sio_rx(uint8_t sio_unit, uint8_t *data, uint8_t max_length)
{
    uint8_t number_of_bytes_received;

    switch (sio_unit)
    {
#ifdef UART0
        case SIO_0: number_of_bytes_received = sio_uart_0_rx(data, max_length);
                    break;
#endif
#ifdef UART1
        case SIO_1: number_of_bytes_received = sio_uart_1_rx(data, max_length);
                    break;
#endif
#ifdef USB0
        case SIO_2: number_of_bytes_received = sio_usb_0_rx(data, max_length);
                break;
#endif
        default:    number_of_bytes_received = 0;
                    break;
    }

    return (number_of_bytes_received);
}


//#endif /* SIO_HUB */

/* EOF */
