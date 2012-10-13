/**
 * @file vendor_stack_config.h
 *
 * @brief Vendor stack configuration parameters
 *
 * This file defines a customer defined stack and needs to be adjusted
 * accordingly.
 *
 * $Id: vendor_stack_config.h 22984 2010-08-19 09:15:38Z sschneid $
 */

/* Prevent double inclusion */
#ifndef VENDOR_STACK_CONFIG_H
#define VENDOR_STACK_CONFIG_H

/* The following defines a customer specific stack layer name. This layer
 * is residing on top of the MAC and thus needs a stack ID which is
 * the increment ID of the MAC layer.
 */
#define VENDOR_NWK_NAME                                 (MAC + 1)

#if (HIGHEST_STACK_LAYER == VENDOR_NWK_NAME)
    /* Reduce the header file dependency by using hard-coded values */
    #define LARGE_BUFFER_SIZE               (160)
    #define SMALL_BUFFER_SIZE               (0)

#endif  /* #if (HIGHEST_STACK_LAYER == ...) */

/* Configuration if VENDOR_NWK_NAME is the highest stack layer */
#if (HIGHEST_STACK_LAYER == VENDOR_NWK_NAME)
#include "vendor_nwk_config.h"
#define NUMBER_OF_TOTAL_STACK_TIMERS        (NUMBER_OF_TAL_TIMERS + NUMBER_OF_MAC_TIMERS + NUMBER_OF_NWK_TIMERS)
#define LAST_STACK_TIMER_ID                 (NWK_LAST_TIMER_ID)
#define NUMBER_OF_LARGE_STACK_BUFS          (5)
#define NUMBER_OF_SMALL_STACK_BUFS          (0)
#endif  /* (HIGHEST_STACK_LAYER == VENDOR_NWK_NAME) */

#endif /* VENDOR_STACK_CONFIG_H */
/* EOF */
