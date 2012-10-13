/**
 * @file nwk_config.h
 *
 * @brief These are all timer IDs which are used in an example NWK stack layer.
 *        This file needs to be adjusted accordingly.
 *
 * $Id: vendor_nwk_config.h 22644 2010-07-21 14:12:37Z sschneid $
 */

/* Prevent double inclusion */
#ifndef VENDOR_NWK_CONFIG_H
#define VENDOR_NWK_CONFIG_H

/* === Includes ============================================================= */

#include "mac_config.h"

/* === Macros =============================================================== */


#define NWK_FIRST_TIMER_ID              (MAC_LAST_TIMER_ID + 1)


/* === Types ================================================================ */


/* Timer ID's used by NWK - Example */
typedef enum
{
    /* NWK Timers start from NWK_FIRST_TIMER_ID */
    T_NWK_1 = (NWK_FIRST_TIMER_ID),
    T_NWK_x = (NWK_FIRST_TIMER_ID + 1)
    /* Add more if required */
} nwk_timer_t;

/* Adjust according to number of timers defined above. */
#define NUMBER_OF_NWK_TIMERS            (2)

#define NWK_LAST_TIMER_ID               (MAC_LAST_TIMER_ID + NUMBER_OF_NWK_TIMERS)

#ifdef ENABLE_QUEUE_CAPACITY
/**
 * Macro configuring the queue capacities.
 */
#define NHLE_NWK_QUEUE_CAPACITY         (x)
#define NWK_NHLE_QUEUE_CAPACITY         (y)
#define MAC_NWK_QUEUE_CAPACITY          (z)
#endif  /* ENABLE_QUEUE_CAPACITY */

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VENDOR_NWK_CONFIG_H */
/* EOF */
