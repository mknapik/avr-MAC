/**
 * @file sio_handler.c
 *
 * @brief Implements the SIO (UART and USB) write and read functionalities
 *
 * $Id: sio_handler.c,v 1.2.2.3 2010/09/09 16:39:30 dam Exp $
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

#include <stdio.h>
#include "pal.h"
#include "sio_handler.h"
#include "pal_types.h"

#if ((defined SIO_HUB) || (PAL_GENERIC_TYPE == ARM7))

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
#if (PAL_GENERIC_TYPE == ARM7) && !defined (SIO_HUB)
    DBGU_PutChar(data);
#elif (PAL_GENERIC_TYPE == SAM3) && !defined (SIO_HUB)
    UART_PutChar(data);
#else
    
    uint8_t c = data;

#if 0
    // we dont like line end replacements
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
#endif

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
#endif
    return (0);
}



#if ((defined __ICCAVR__) || (defined __ICCARM__))
int _sio_getchar(void)
#else
int _sio_getchar(FILE *dummy)
#endif
{
#if (PAL_GENERIC_TYPE == ARM7) && !defined (SIO_HUB)
    return DBGU_GetChar();
#elif (PAL_GENERIC_TYPE == SAM3) && !defined (SIO_HUB)
    return UART_GetChar();
#else

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
#endif
}



#if ((defined __ICCAVR__) || (defined __ICCARM__))
int _sio_getchar_nowait(void)
#else
int _sio_getchar_nowait(FILE *dummy)
#endif
{
#if (PAL_GENERIC_TYPE == ARM7) && !defined (SIO_HUB)
    if(!DBGU_IsRxReady())
        return -1;
    else
        return DBGU_GetChar();
#elif (PAL_GENERIC_TYPE == SAM3) && !defined (SIO_HUB)
    if(!UART_IsRxReady())
        return -1;
    else
        return UART_GetChar();
#else

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
#endif
}

#endif  /* ((defined SIO_HUB) || (PAL_GENERIC_TYPE == ARM7)) */

/* EOF */
