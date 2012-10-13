/**
 * @file sio_handler.h
 *
 * @brief This file contains the prototypes for UART related functions.
 *
 * $Id: sio_handler.h,v 1.2.2.3 2010/09/09 16:39:30 dam Exp $
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
#ifndef SIO_HANDLER_H
#define SIO_HANDLER_H

/* === Includes ============================================================= */

#include <stdio.h>
#include "pal_types.h"

/* === Macros =============================================================== */
//#if ((PAL_GENERIC_TYPE == ARM7) || (PAL_GENERIC_TYPE == SAM3))
#ifndef SIO_CHANNEL

// the AT91SAM3*-headers define UARTx themselves, even if not used in Makefile
#if ((defined UART0) && (PAL_GENERIC_TYPE != SAM3))
  #define SIO_CHANNEL     SIO_0
#elif ((defined UART0) && (PAL_GENERIC_TYPE != SAM3))
  #define SIO_CHANNEL     SIO_1
#elif defined USB0
 #define SIO_CHANNEL      SIO_2
#else
 /* define a dummy value */
 #define SIO_CHANNEL      0xFF
#endif /* SIO_CHANNEL definition */

#endif /* #ifndef SIO_CHANNEL */
//#endif /* (PAL_GENERIC_TYPE == ARM7) || (PAL_GENERIC_TYPE == SAM3) */

/* Function aliases allowing IAR and GCC functions use the same way */
#if ((defined __ICCAVR__) || (defined __ICCARM__))
#define sio_putchar(data)       _sio_putchar(data)
#define sio_getchar()           _sio_getchar()
#define sio_getchar_nowait()    _sio_getchar_nowait()
#define fdevopen(a,b)           /*tkr: iar does not use the fdevopen - the __write() (or __read()) must exist instread (here in file write.c)*/
#else
#define sio_putchar(data)       _sio_putchar(data, NULL)
#define sio_getchar()           _sio_getchar(NULL)
#define sio_getchar_nowait()    _sio_getchar_nowait(NULL)
#endif

#if (PAL_GENERIC_TYPE == ARM7) || (PAL_GENERIC_TYPE == SAM3)

    // dummy fdevopen implementation
    #define fdevopen(a,b)      {}

#ifdef SIO_HUB


#include <stdlib.h>
#include <string.h>

    // redirect 'printf' to selected SIO channel
    #define MIN(a, b)       ((a < b) ? a : b)
    //#define BUF_SIZE        MIN(USB_TX_BUF_SIZE, UART_MAX_TX_BUF_LENGTH)
    #define BUF_SIZE        (128)
    #if (BUF_SIZE <= 0)
        #error "unsupported buffer size"
    #endif

    char sio_buf[BUF_SIZE];
    #define printf(fmt, args...) {                                     \
        /*memset(sio_buf, 0, BUF_SIZE);*/                              \
        sprintf(sio_buf, fmt, ##args);                                 \
        pal_sio_tx(SIO_CHANNEL,                                        \
                (uint8_t*)sio_buf, MIN(BUF_SIZE,strlen(sio_buf)));     \
    }


// TODO: the same for 'scanf'

#else
#if (PAL_GENERIC_TYPE == ARM7)
#include <peripherals/dbgu/dbgu.h>
#elif (PAL_GENERIC_TYPE == SAM3)
#include <drivers/usart/uart_console.h>
#endif

// 'printf' is auto-redirected to DBGU, but 'sio_getchar' and
// 'sio_getchar_nowait' need to be redirected from dbgu -> done below

#endif    // SIO_HUB
#endif    // (PAL_GENERIC_TYPE == ARM7)

/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


#if ((defined __ICCAVR__) || (defined __ICCARM__))
int _sio_putchar(char data);
int _sio_getchar(void);
int _sio_getchar_nowait(void);
#else
int _sio_putchar(char data, FILE *dummy);
int _sio_getchar(FILE *dummy);
int _sio_getchar_nowait(FILE *dummy);
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SIO_HANDLER_H */
/* EOF */
