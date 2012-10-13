/**
 * @file app_config.h
 *
 * @brief These are application-specific resources which are used
 *        in the application in addition to the underlying stack.
 *
 * $Id: app_config.h,v 1.1.4.1 2010/09/07 17:39:37 dam Exp $
 *
 * @author    de: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 */
/*
 * Copyright (c) 2010, dresden elektronik All rights reserved.
 *
 */


/* Prevent double inclusion */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* === Includes ============================================================= */

#include "stack_config.h"

/* === Macros =============================================================== */

#if (NUMBER_OF_TOTAL_STACK_TIMERS == 0)
#define APP_FIRST_TIMER_ID          (0)
#else
#define APP_FIRST_TIMER_ID          (LAST_STACK_TIMER_ID + 1)
#endif

/* === Types ================================================================ */

/* Timer ID's used by the Application */
//#if 0   /* Unused */
typedef enum
{
    /* App Timers start from APP_FIRST_TIMER_ID */

    /* App timer 1 */
    TIMER_DATA_FWD = (APP_FIRST_TIMER_ID)

} app_timer_t;
//#endif

#define NUMBER_OF_APP_TIMERS        (1)

#define TOTAL_NUMBER_OF_TIMERS      (NUMBER_OF_APP_TIMERS + NUMBER_OF_TOTAL_STACK_TIMERS)

/* Additional buffers used by the application */
#define NUMBER_OF_LARGE_APP_BUFS    (0)
#define NUMBER_OF_SMALL_APP_BUFS    (0)

#define TOTAL_NUMBER_OF_LARGE_BUFS  (NUMBER_OF_LARGE_APP_BUFS + NUMBER_OF_LARGE_STACK_BUFS)
#define TOTAL_NUMBER_OF_SMALL_BUFS  (NUMBER_OF_SMALL_APP_BUFS + NUMBER_OF_SMALL_STACK_BUFS)

#define TOTAL_NUMBER_OF_BUFS        (TOTAL_NUMBER_OF_LARGE_BUFS + TOTAL_NUMBER_OF_SMALL_BUFS)

/*
 * USB transmit buffer size
 */
#define USB_TX_BUF_SIZE             (0xFF)

/*
 * USB receive buffer size
 */
#define USB_RX_BUF_SIZE             (0xFF)

/*
 * UART transmit buffer size
 */
#define UART_MAX_TX_BUF_LENGTH      (0xFF)

/*
 * UART receive buffer size
 */
#define UART_MAX_RX_BUF_LENGTH      (0xFF)

/* Offset of IEEE address storage location within EEPROM */
#define EE_IEEE_ADDR                (0)

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
