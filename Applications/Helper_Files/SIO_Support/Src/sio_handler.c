/**
 * @file sio_handler.c
 *
 * @brief Implements the SIO (UART and USB) write and read functionalities
 *
 * $Id: sio_handler.c 16792 2009-07-29 15:56:05Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================ */

#ifdef SIO_HUB
#include <stdio.h>
#include "pal.h"
#include "sio_handler.h"

/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */


#if ((defined __ICCAVR__) || (defined __ICCARM__))
int _sio_putchar(char data)
#else
int _sio_putchar(char data, FILE *dummy)
#endif
{
    uint8_t c = data;

    if (c == '\n')
    {
        c = '\r';

        while (0 == pal_sio_tx(SIO_CHANNEL, &c, 1))
        {
#ifdef USB0
            /*
             * For USB we need to call the USB handler via pal_task()
             * within this while loop.
             */
            pal_task();
#endif
        }

        c = data;
    }

    while (0 == pal_sio_tx(SIO_CHANNEL, &c, 1))
    {
#ifdef USB0
        /*
         * For USB we need to call the USB handler via pal_task()
         * within this while loop.
         */
        pal_task();
#endif
    }

    return (0);
}



#if ((defined __ICCAVR__) || (defined __ICCARM__))
int _sio_getchar(void)
#else
int _sio_getchar(FILE *dummy)
#endif
{
    uint8_t c;

    while (0 == pal_sio_rx(SIO_CHANNEL, &c, 1))
    {
#ifdef USB0
        /*
         * For USB we need to call the USB handler via pal_task()
         * within this while loop.
         */
        pal_task();
#endif
    }

    return c;
}



#if ((defined __ICCAVR__) || (defined __ICCARM__))
int _sio_getchar_nowait(void)
#else
int _sio_getchar_nowait(FILE *dummy)
#endif
{
    uint8_t c;

#ifdef USB0
    /*
     * In case this is used in a while loop with USB,
     * we need to call the USB handler via pal_task().
     */
    pal_task();
#endif

    int back = pal_sio_rx(SIO_CHANNEL, &c, 1);

    if (back == 1)
    {
        return c;
    }
    else
    {
        return (-1);
    }
}

#endif  /* SIO_HUB */

/* EOF */
