/**
 * @file pal_uart.c
 *
 * @brief PAL UART related functions
 *
 * This file implements ARM7 UART related functions
 * and reception functions.
 *
 * $Id: pal_uart.c 22853 2010-08-11 08:10:53Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================= */

#if ((defined UART0) || (defined UART1))
#include <stdint.h>
#include "pal.h"
#include "pal_config.h"
#include "pal_uart.h"

/* === Macros =============================================================== */


/* === Globals ============================================================== */

#if (defined UART0)
static uart_communication_buffer_t uart_0_buffer;
#endif

#if (defined UART1)
static uart_communication_buffer_t uart_1_buffer;
#endif

/* === Prototypes =========================================================== */

#ifdef UART0
static void uart0_irq_handler(void);
#endif

#ifdef UART1
static void uart1_irq_handler(void);
#endif

/* === Implementation ======================================================= */

#ifdef UART0
/**
 * @brief Initializes UART 0
 *
 * This function initializes the UART channel 0.
 *
 * @param baud_rate Baud rate Register value corresponding baud rate
 */
void sio_uart_0_init(uint32_t baud_rate)
{
    uint32_t uart_id;
    AT91PS_USART uart_ptr;
    uart_irq_handler_t irq_handler;

    /* ID of the UART0 peripheral. */
    uart_id = AT91C_ID_US0;

    /* Pointer to the UART0 base. */
    uart_ptr = AT91C_BASE_US0;

    /* Iinterrupt handler for UART0. */
    irq_handler = uart0_irq_handler;

    /*
     * The RXD0 and TXD0 pins on the PIO A are configured to be
     * controlled by on chip UART peripheral.
     */
    AT91C_BASE_PIOA->PIO_ASR = (PIN_UART_0_RXD | PIN_UART_0_TXD);
    AT91C_BASE_PIOA->PIO_PDR = (PIN_UART_0_RXD | PIN_UART_0_TXD);


    /* The clock for selected UART channel is enabled. */
    AT91C_BASE_PMC->PMC_PCER = _BV(uart_id);

    /*
     * The selected UART channel is configured, and the baud rate is also
     * set to 9.6kbps.
     */
    uart_ptr->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX |
                      AT91C_US_RXDIS | AT91C_US_TXDIS;

    /*
     * UART is configured in normal mode, 8 bits per transfer, no parity bit
     * and one stop bit (per transfer).
     */
    uart_ptr->US_MR = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK |
                      AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE |
                      AT91C_US_NBSTOP_1_BIT;

    /*
     * The divisior to generate a baud rate of 9.6kbps is programmed in the
     * US_BRGR register.
     */
    uart_ptr->US_BRGR = UART_BAUD_DIVISOR;

    /* The AIC is set up for the UART interrupts. */
    AIC_CONFIGURE(uart_id, AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, irq_handler);

    /* The receive complete interrupt is enabled for specified UART channel. */
    ENABLE_UART_0_RX_INT();

    /* The UART interrupt is enabled in AIC. */
    AT91C_BASE_AIC->AIC_IECR = _BV(uart_id);

    /* Transmitter and receiver of the UART channel are enabled. */
    uart_ptr->US_CR = AT91C_US_TXEN;
    uart_ptr->US_CR = AT91C_US_RXEN;

    baud_rate = baud_rate;  /* Keep compiler happy. */
}
#endif  /* UART0 */



#ifdef UART1
/**
 * @brief Initializes UART 1
 *
 * This function initializes the UART channel 1.
 *
 * @param baud_rate Baud rate Register value corresponding baud rate
 */
void sio_uart_1_init(uint32_t baud_rate)
{
    uint32_t uart_id;
    AT91PS_USART uart_ptr;
    uart_irq_handler_t irq_handler;

    /* ID of the UART1 peripheral. */
    uart_id = AT91C_ID_US1;

    /* Pointer to the UART1 base. */
    uart_ptr = AT91C_BASE_US1;

    /* Iinterrupt handler for UART1. */
    irq_handler = uart1_irq_handler;

    /*
     * The RXD1 and TXD1 pins on the PIO A are configured to be
     * controlled by on chip UART peripheral.
     */
    AT91C_BASE_PIOA->PIO_ASR = (PIN_UART_1_RXD | PIN_UART_1_TXD);
    AT91C_BASE_PIOA->PIO_PDR = (PIN_UART_1_RXD | PIN_UART_1_TXD);

    /* The clock for selected UART channel is enabled. */
    AT91C_BASE_PMC->PMC_PCER = _BV(uart_id);

    /*
     * The selected UART channel is configured, and the baud rate is also
     * set to 9.6kbps.
     */
    uart_ptr->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX |
                      AT91C_US_RXDIS | AT91C_US_TXDIS;

    /*
     * UART is configured in normal mode, 8 bits per transfer, no parity bit
     * and one stop bit (per transfer).
     */
    uart_ptr->US_MR = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK |
                      AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE |
                      AT91C_US_NBSTOP_1_BIT;

    /*
     * The divisior to generate a baud rate of 9.6kbps is programmed in the
     * US_BRGR register.
     */
    uart_ptr->US_BRGR = UART_BAUD_DIVISOR;

    /* The AIC is set up for the UART interrupts. */
    AIC_CONFIGURE(uart_id, AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, irq_handler);

    /* The receive complete interrupt is enabled for specified UART channel. */
    ENABLE_UART_1_RX_INT();

    /* The UART interrupt is enabled in AIC. */
    AT91C_BASE_AIC->AIC_IECR = _BV(uart_id);

    /* Transmitter and receiver of the UART channel are enabled. */
    uart_ptr->US_CR = AT91C_US_TXEN;
    uart_ptr->US_CR = AT91C_US_RXEN;

    baud_rate = baud_rate;  /* Keep compiler happy. */
}
#endif  /* UART1 */



#ifdef UART0
/**
 * @brief Transmit data via UART 0
 *
 * This function transmits data via UART channel 0.
 *
 * @param data Pointer to the buffer where the data to be transmitted is present
 * @param length Number of bytes to be transmitted
 *
 * @return Number of bytes actually transmitted
 */
uint8_t sio_uart_0_tx(uint8_t *data, uint8_t length)
{
    uint8_t bytes_to_be_written;
    uint8_t head;
    uint8_t tail;
    uint8_t size;
    uint8_t back;

    /* The transmit interrupt is disabled. */
    DISABLE_UART_0_TX_INT();

    /*
     * Calculate available buffer space
     */
    head = uart_0_buffer.tx_buf_head;
    tail = uart_0_buffer.tx_buf_tail;

    if (tail >= head)
    {
        size = (UART_MAX_TX_BUF_LENGTH - 1) - (tail - head);
    }
    else
    {
        size = (head - 1) - tail;
    }

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
        uart_0_buffer.tx_buf[uart_0_buffer.tx_buf_tail] = *data;

        if ((UART_MAX_TX_BUF_LENGTH - 1) == uart_0_buffer.tx_buf_tail)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            uart_0_buffer.tx_buf_tail = 0;
        }
        else
        {
            /*
             * Increment the index to point the next character to be
             * inserted.
             */
            uart_0_buffer.tx_buf_tail++;
        }

        bytes_to_be_written--;
        data++;
    }

    /*
     * Check whether there is a transmission ongoing. Otherwise write
     * data into the UART data register. Transmission of subsequent
     * bytes / data will be taken care in the ISR.
     */
    if (uart_0_buffer.tx_count == 0)
    {
        WRITE_TO_UART_0(uart_0_buffer.tx_buf[head]);
        uart_0_buffer.tx_count = 1;
    }

    /* The transmit interrupt is enabled. */
    ENABLE_UART_0_TX_INT();

    return back;
}
#endif  /* UART0 */



#ifdef UART1
/**
 * @brief Transmit data via UART 1
 *
 * This function transmits data via UART channel 1.
 *
 * @param data Pointer to the buffer where the data to be transmitted is present
 * @param length Number of bytes to be transmitted
 *
 * @return Number of bytes actually transmitted
 */
uint8_t sio_uart_1_tx(uint8_t *data, uint8_t length)
{
    uint8_t bytes_to_be_written;
    uint8_t head;
    uint8_t tail;
    uint8_t size;
    uint8_t back;

    /* The transmit interrupt is disabled. */
    DISABLE_UART_1_TX_INT();

    /*
     * Calculate available buffer space
     */
    head = uart_1_buffer.tx_buf_head;
    tail = uart_1_buffer.tx_buf_tail;

    if (tail >= head)
    {
        size = (UART_MAX_TX_BUF_LENGTH - 1) - (tail - head);
    }
    else
    {
        size = (head - 1) - tail;
    }

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
        uart_1_buffer.tx_buf[uart_1_buffer.tx_buf_tail] = *data;

        if ((UART_MAX_TX_BUF_LENGTH - 1) == uart_1_buffer.tx_buf_tail)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            uart_1_buffer.tx_buf_tail = 0;
        }
        else
        {
            /*
             * Increment the index to point the next character to be
             * inserted.
             */
            uart_1_buffer.tx_buf_tail++;
        }

        bytes_to_be_written--;
        data++;
    }

    /*
     * Check whether there is a transmission ongoing. Otherwise write
     * data into the UART data register. Transmission of subsequent
     * bytes / data will be taken care in the ISR.
     */
    if (uart_1_buffer.tx_count == 0)
    {
        WRITE_TO_UART_1(uart_1_buffer.tx_buf[head]);
        uart_1_buffer.tx_count = 1;
    }

    /* The transmit interrupt is enabled. */
    ENABLE_UART_1_TX_INT();

    return back;
}
#endif  /* UART1 */



#ifdef UART0
/**
 * @brief Receives data from UART 0
 *
 * This function receives data from UART channel 0.
 *
 * @param data pointer to the buffer where the received data is to be stored
 * @param max_length maximum length of data to be received
 *
 * @return actual number of bytes received
 */
uint8_t sio_uart_0_rx(uint8_t *data, uint8_t max_length)
{
    uint8_t data_received = 0;

    if (uart_0_buffer.rx_count == 0)
    {
        /* UART receive buffer is empty. */
        return 0;
    }

    /* The receive interrupt is disabled. */
    DISABLE_UART_0_RX_INT();

    if (UART_MAX_RX_BUF_LENGTH <= uart_0_buffer.rx_count)
    {
        /*
         * Bytes between head and tail are overwritten by new data.
         * The oldest data in buffer is the one to which the tail is
         * pointing. So reading operation should start from the tail.
         */
        uart_0_buffer.rx_buf_head = uart_0_buffer.rx_buf_tail;

        /*
         * This is a buffer overflow case. But still only bytes equivalent to
         * full buffer size are useful.
         */
        uart_0_buffer.rx_count = UART_MAX_RX_BUF_LENGTH;

        /* Bytes received is more than or equal to buffer. */
        if (UART_MAX_RX_BUF_LENGTH <= max_length)
        {
            /*
             * Requested receive length (max_length) is more than the
             * max size of receive buffer, but at max the full
             * buffer can be read.
             */
            max_length = UART_MAX_RX_BUF_LENGTH;
        }
    }
    else
    {
        /* Bytes received is less than receive buffer maximum length. */
        if (max_length > uart_0_buffer.rx_count)
        {
            /*
             * Requested receive length (max_length) is more than the data
             * present in receive buffer. Hence only the number of bytes
             * present in receive buffer are read.
             */
            max_length = uart_0_buffer.rx_count;
        }
    }

    data_received = max_length;

    while (max_length > 0)
    {
        /* Start to copy from head. */
        *data = uart_0_buffer.rx_buf[uart_0_buffer.rx_buf_head];
        uart_0_buffer.rx_buf_head++;
        uart_0_buffer.rx_count--;
        data++;
        max_length--;
        if ((UART_MAX_RX_BUF_LENGTH) == uart_0_buffer.rx_buf_head)
        {
            uart_0_buffer.rx_buf_head = 0;
        }
    }

    /* The receive interrupt is enabled back. */
    ENABLE_UART_0_RX_INT();

    return data_received;
}
#endif  /* UART0 */



#ifdef UART1
/**
 * @brief Receives data from UART 1
 *
 * This function receives data from UART channel 1.
 *
 * @param data pointer to the buffer where the received data is to be stored
 * @param max_length maximum length of data to be received
 *
 * @return actual number of bytes received
 */
uint8_t sio_uart_1_rx(uint8_t *data, uint8_t max_length)
{
    uint8_t data_received = 0;

    if (uart_1_buffer.rx_count == 0)
    {
        /* UART receive buffer is empty. */
        return 0;
    }

    /* The receive interrupt is disabled. */
    DISABLE_UART_1_RX_INT();

    if (UART_MAX_RX_BUF_LENGTH <= uart_1_buffer.rx_count)
    {
        /*
         * Bytes between head and tail are overwritten by new data.
         * The oldest data in buffer is the one to which the tail is
         * pointing. So reading operation should start from the tail.
         */
        uart_1_buffer.rx_buf_head = uart_1_buffer.rx_buf_tail;

        /*
         * This is a buffer overflow case. But still only bytes equivalent to
         * full buffer size are useful.
         */
        uart_1_buffer.rx_count = UART_MAX_RX_BUF_LENGTH;

        /* Bytes received is more than or equal to buffer. */
        if (UART_MAX_RX_BUF_LENGTH <= max_length)
        {
            /*
             * Requested receive length (max_length) is more than the
             * max size of receive buffer, but at max the full
             * buffer can be read.
             */
            max_length = UART_MAX_RX_BUF_LENGTH;
        }
    }
    else
    {
        /* Bytes received is less than receive buffer maximum length. */
        if (max_length > uart_1_buffer.rx_count)
        {
            /*
             * Requested receive length (max_length) is more than the data
             * present in receive buffer. Hence only the number of bytes
             * present in receive buffer are read.
             */
            max_length = uart_1_buffer.rx_count;
        }
    }

    data_received = max_length;

    while (max_length > 0)
    {
        /* Start to copy from head. */
        *data = uart_1_buffer.rx_buf[uart_1_buffer.rx_buf_head];
        uart_1_buffer.rx_buf_head++;
        uart_1_buffer.rx_count--;
        data++;
        max_length--;
        if ((UART_MAX_RX_BUF_LENGTH) == uart_1_buffer.rx_buf_head)
        {
            uart_1_buffer.rx_buf_head = 0;
        }
    }

    /* The receive interrupt is enabled back. */
    ENABLE_UART_1_RX_INT();

    return data_received;
}
#endif  /* UART1 */



#ifdef UART0
/**
 * @brief ISR for UART0 transmission and reception interrupts
 *
 * This service routine is executed when a byte is successfully transmitted or
 * received on UART0.
 */
void uart0_irq_handler(void)
{
    uint32_t uart_irq_cause;

    /*
     * The interrupt is acknowledged by reading the status register. This
     * clears the interrupt at the UART level.
     */
    uart_irq_cause = AT91C_BASE_US0->US_CSR;

    if (AT91C_US_RXRDY & uart_irq_cause)
    {
        uint8_t tail = uart_0_buffer.rx_buf_tail;

        /* Count of bytes received through UART0 channel is incremented. */
        uart_0_buffer.rx_count++;

        READ_FROM_UART_0(uart_0_buffer.rx_buf[tail]);

        if ((UART_MAX_RX_BUF_LENGTH - 1) == uart_0_buffer.rx_buf_tail)
        {
            /*
             * Revert back to beginning of buffer after reaching end of buffer.
             */
            uart_0_buffer.rx_buf_tail = 0;
        }
        else
        {
            uart_0_buffer.rx_buf_tail++;
        }
    }
    else if (AT91C_US_TXRDY & uart_irq_cause)
    {
        /*
         * UART0 transmit interrupt is disabled.
         * If not disabled, an immidiate interrupt after the update of UDR0
         * prevents the execution of remaining instructions.
         */
        DISABLE_UART_0_TX_INT();

        if ((UART_MAX_TX_BUF_LENGTH - 1) == uart_0_buffer.tx_buf_head)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            uart_0_buffer.tx_buf_head = 0;
        }
        else
        {
            /*
             * Increment the index to point the next character to be
             * transmitted.
             */
            uart_0_buffer.tx_buf_head++;
        }

        if (uart_0_buffer.tx_buf_head != uart_0_buffer.tx_buf_tail)
        {
            WRITE_TO_UART_0(uart_0_buffer.tx_buf[uart_0_buffer.tx_buf_head]);

            /*
             * Some more bytes are to be transmitted, hence the transmit
             * interrupt is enabled back.
             */
            ENABLE_UART_0_TX_INT();
        }
        else
        {
            /* No more data for transmission */
            uart_0_buffer.tx_count = 0;
        }
    }
}
#endif  /* UART0 */


#ifdef UART1
/**
 * @brief ISR for UART1 transmission and reception interrupts
 *
 * This service routine is executed when a byte is successfully transmitted or
 * received on UART1.
 */
void uart1_irq_handler(void)
{
    uint32_t uart_irq_cause;

    /*
     * The interrupt is acknowledged by reading the status register. This
     * clears the interrupt at the UART level.
     */
    uart_irq_cause = AT91C_BASE_US1->US_CSR;

    if (AT91C_US_RXRDY & uart_irq_cause)
    {
        uint8_t tail = uart_1_buffer.rx_buf_tail;

        /* Count of bytes received through UART1 channel is incremented. */
        uart_1_buffer.rx_count++;

        READ_FROM_UART_1(uart_1_buffer.rx_buf[tail]);

        if ((UART_MAX_RX_BUF_LENGTH - 1) == uart_1_buffer.rx_buf_tail)
        {
             /*
              * Revert back to beginning of buffer after reaching end of buffer.
              */
            uart_1_buffer.rx_buf_tail = 0;
        }
        else
        {
            uart_1_buffer.rx_buf_tail++;
        }
    }
    else if (AT91C_US_TXRDY & uart_irq_cause)
    {
        /*
         * UART1 transmit interrupt is disabled.
         * If not disabled, an immidiate interrupt after the update of UDR1
         * prevents the execution of remaining instructions.
         */
        DISABLE_UART_1_TX_INT();

        if ((UART_MAX_TX_BUF_LENGTH - 1) == uart_1_buffer.tx_buf_head)
        {
            /* Reached the end of buffer, revert back to beginning of buffer. */
            uart_1_buffer.tx_buf_head = 0;
        }
        else
        {
            /*
             * Increment the index to point the next character to be
             * transmitted.
             */
            uart_1_buffer.tx_buf_head++;
        }

        if (uart_1_buffer.tx_buf_head != uart_1_buffer.tx_buf_tail)
        {
            WRITE_TO_UART_1(uart_1_buffer.tx_buf[uart_1_buffer.tx_buf_head]);

            /*
             * Some more bytes are to be transmitted, hence the transmit
             * interrupt is enabled back.
             */
            ENABLE_UART_1_TX_INT();
        }
        else
        {
            /* No more data for transmission */
            uart_1_buffer.tx_count = 0;
        }
    }
}
#endif  /* UART1 */

#endif  /* ((defined UART0) || (defined UART1)) */

/* EOF */
