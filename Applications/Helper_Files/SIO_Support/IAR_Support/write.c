/**
 * @file write.c
 *
 * @brief Implements the WRITE function used by standard library.
 *
 * This is a template implementation of the "__write" function used by
 * the standard library.  Replace it with a system-specific
 * implementation.
 *
 * The "__write" function should output "size" number of bytes from
 * "buffer" in some application-specific way. It should return the
 * number of characters written, or _LLIO_ERROR on failure.
 *
 * If "buffer" is zero then __write should perform flushing of
 * internal buffers, if any. In this case "handle" can be -1 to
 * indicate that all handles should be flushed.
 *
 * The template implementation below assumes that the application
 * provides the function "low_level_putchar". It should return the
 * character written, or -1 on failure.
 *
 * $Id: write.c 16779 2009-07-29 13:15:48Z sschneid $
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

#if ((defined __ICCAVR__) || (defined __ICCARM__))

#include <yfuns.h>
#include <stdint.h>
#include "sio_handler.h"

_STD_BEGIN

  #pragma module_name = "?__write"

/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

int low_level_putchar(int x)
{
    return _sio_putchar((char) x);
}


/*
 * If the __write implementation uses internal buffering, uncomment
 * the following line to ensure that we are called with "buffer" as 0
 * (i.e. flush) when the application terminates.
 */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    size_t nChars = 0;

    if (buffer == 0)
    {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return. (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return 0;
    }

    /*
     * This template only writes to "standard out" and "standard err",
     * for all other file handles it returns failure.
     */
    if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR)
    {
        return _LLIO_ERROR;
    }

    for (/* Empty */; size != 0; --size)
    {
        if (low_level_putchar(*buffer++) < 0)
        {
            return _LLIO_ERROR;
        }

        ++nChars;
    }

    return nChars;
}

_STD_END

#endif  /* ((defined __ICCAVR__) || (defined __ICCARM__)) */

/* EOF */
