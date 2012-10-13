/**
 * @file pal_timer.c
 *
 * @brief Timer related functions for the ATmega128RFA1 Mega RF Single Chip
 *
 * This file implements timer related functions for the ATmega128RFA1.
 *
 * $Id: pal_timer.c 22386 2010-06-30 15:51:38Z sschneid $
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "pal_timer.h"
#include "app_config.h"

/* === Globals ============================================================== */

/*
 * Check the number of required timers or change the number of timers that are
 * provided by the PAL. This is a kind of error handling to reduce the
 * number of used timer and therefore the RAM usage.
 */
#if (TOTAL_NUMBER_OF_TIMERS > MAX_NO_OF_TIMERS)
#error "Number of used timers is greater than the number of timer provided by PAL."
#endif

#if (TOTAL_NUMBER_OF_TIMERS > 0)

/*
 * This is the timer array.
 *
 * TOTAL_NUMBER_OF_TIMERS is calculated in file app_config.h within the Inc
 * directory of each application depending on the number of timers required
 * by the stack and the application.
 */
timer_info_t timer_array[TOTAL_NUMBER_OF_TIMERS];

/* This is the counter of all running timers. */
static uint8_t running_timers;

/* This flag indicates an expired timer. */
static volatile bool timer_trigger;

/* This is the reference to the head of the running timer queue. */
static uint_fast8_t running_timer_queue_head;

/* This is the reference to the head of the expired timer queue. */
static uint_fast8_t expired_timer_queue_head;

/* This is the reference to the tail of the expired timer queue. */
static uint_fast8_t expired_timer_queue_tail;

/* Time when the SCOCR1 interrupt triggered last time */
static uint32_t timer_last_trigger;
#endif  /* #if (TOTAL_NUMBER_OF_TIMERS > 0) */

/* This variable holds the id of high priority timer */
#ifdef ENABLE_HIGH_PRIO_TMR
static volatile uint8_t high_priority_timer_id;
#endif

/* === Macros ============================================================== */

/** String concatenation by preprocessor used to create proper register names. */
#define CONCAT(a,b) a##b

/** Creates proper subregister names and reads the corresponding values. */
#define SC_READ32(reg)                  sc_read32(&CONCAT(reg,HH), \
                                                  &CONCAT(reg,HL), \
                                                  &CONCAT(reg,LH), \
                                                  &CONCAT(reg,LL))

/** Creates proper subregister names and writes the corresponding values. */
#define SC_WRITE32(reg,val) \
do { \
    union { uint8_t a[4]; uint32_t v; } x; \
    x.v = val; \
    CONCAT(reg,HH) = x.a[3]; \
    CONCAT(reg,HL) = x.a[2]; \
    CONCAT(reg,LH) = x.a[1]; \
    CONCAT(reg,LL) = x.a[0]; \
} while(0)

/**
 * Maximum delay in us that function pal_timer_delay() is running
 * with interrupts disabled.
 * If the delay is larger, interrupts need to be enabled inbetween.
 */
#define DELAY_WITHOUT_IRQS_US           (100)


/*
 * Since all layers above PAL currently require a timer tick of 1us,
 * but the used hardware timer runs at symbol rate (1tick = 16us),
 * the upper 4 bits of the hardware timer are not used.
 *
 * In order to cope with this a variety of defines are required.
 */
/** Unmask upper 4 bits. */
#define MASK_LOW28                      (0x0FFFFFFFuL)

/** Unmask upper 5 bits. */
#define MASK_LOW27                      (0x07FFFFFFuL)

/** Conversion factor: us per symbol for 2,4 GHz. */
#define PAL_US_PER_SYMBOLS              (16)

/* === Prototypes =========================================================== */

#if (TOTAL_NUMBER_OF_TIMERS > 0)
static void prog_ocr(void);
static void start_absolute_timer(uint8_t timer_id,
                                 uint32_t point_in_time,
                                 FUNC_PTR handler_cb,
                                 void *parameter);
#endif  /* #if (TOTAL_NUMBER_OF_TIMERS > 0) */

/* === Implementation ======================================================= */

#if defined(DOXYGEN)
/**
 * @brief Reads the 32-bit timer register in the required order of bytes
 *
 * @param hh hh octet of 32-bit register
 * @param hl hl octet of 32-bit register
 * @param lh lh octet of 32-bit register
 * @param ll ll octet of 32-bit register
 *
 * @returns uint32_t Value of timer register
 */
static inline uint32_t sc_read32(volatile uint8_t *hh,
                                 volatile uint8_t *hl,
                                 volatile uint8_t *lh,
                                 volatile uint8_t *ll);
#else
FORCE_INLINE(uint32_t, sc_read32, volatile uint8_t *hh,
                                  volatile uint8_t *hl,
                                  volatile uint8_t *lh,
                                  volatile uint8_t *ll)
{
    union
    {
        uint8_t a[4];
        uint32_t rv;
    }
    x;

    x.a[0] = *ll;
    x.a[1] = *lh;
    x.a[2] = *hl;
    x.a[3] = *hh;

    return x.rv;
}
#endif /* DOXYGEN */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Compares two 32-bit time values
 *
 * This function compares two true 32-bit time values t1 and t2
 * and returns true if t1 is less than t2.
 *
 * @param t1 Time
 * @param t2 Time
 *
 * @return true If t1 is less than t2 when MSBs are same, false otherwise.
 * @ingroup apiPalApi
 */
static inline bool compare_time(uint32_t t1, uint32_t t2)
{
    bool comparison_status = false;

    if ((t1 & ~MASK_LOW27) == (t2 & ~MASK_LOW27))
    {
        if (t1 < t2)
        {
            comparison_status = true;
        }
    }
    else
    {
        /* This is a timer roll case in which t2 is ahead of t1. */
        if ((t1 & MASK_LOW27) > (t2 & MASK_LOW27))
        {
            comparison_status = true;
        }
    }
    return comparison_status;
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Timer handling services
 *
 * This Function performs timer handling services.
 * It calls functions which are responsible
 * 1) to put the expired timer into the expired timer queue, and
 * 2) to service expired timers and call the respective callback.
 */
void timer_service(void)
{
    internal_timer_handler();

    /*
     * Process expired timers.
     * Call the callback functions of the expired timers in the order of their
     * expiry.
     */
    {
        timer_expiry_cb_t callback;
        void *callback_param;
        uint8_t next_expired_timer;

        /* Expired timer if any will be processed here */
        while (NO_TIMER != expired_timer_queue_head)
        {
            ENTER_CRITICAL_REGION();

            next_expired_timer = timer_array[expired_timer_queue_head].next_timer_in_queue;

            /* Callback is stored */
            callback = (timer_expiry_cb_t)timer_array[expired_timer_queue_head].timer_cb;

            /* Callback parameter is stored */
            callback_param = timer_array[expired_timer_queue_head].param_cb;

            /*
             * The expired timer's structure elements are updated and the timer
             * is taken out of expired timer queue
             */
            timer_array[expired_timer_queue_head].next_timer_in_queue = NO_TIMER;
            timer_array[expired_timer_queue_head].timer_cb = NULL;
            timer_array[expired_timer_queue_head].param_cb = NULL;

            /*
             * The expired timer queue head is updated with the next timer in the
             * expired timer queue.
             */
            expired_timer_queue_head = next_expired_timer;

            if (NO_TIMER == expired_timer_queue_head)
            {
                expired_timer_queue_tail = NO_TIMER;
            }

            LEAVE_CRITICAL_REGION();

            if (NULL != callback)
            {
                /* Callback function is called */
                callback(callback_param);
            }
        }
    }
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Starts regular timer
 *
 * This function starts a regular timer and registers the corresponding
 * callback function to handle the timeout event.
 *
 * @param timer_id Timer identifier
 * @param timer_count Timeout in microseconds
 * @param timeout_type @ref TIMEOUT_RELATIVE / @ref TIMEOUT_ABSOLUTE
 * @param timer_cb Callback handler invoked upon timer expiry
 * @param param_cb Argument for the callback handler
 *
 * @return
 * - @ref PAL_TMR_INVALID_ID  if the timer identifier is undefined,
 * - @ref MAC_INVALID_PARAMETER if the callback function for this timer is NULL or
 *   timeout_type is invalid,
 * - @ref PAL_TMR_ALREADY_RUNNING if the timer is already running,
 * - @ref MAC_SUCCESS if timer is started, or
 * - @ref PAL_TMR_INVALID_TIMEOUT if timeout is not within the timeout range.
 */
retval_t pal_timer_start(uint8_t timer_id,
                         uint32_t timer_count,
                         timeout_type_t timeout_type,
                         FUNC_PTR timer_cb,
                         void *param_cb)
{
    uint32_t point_in_time;

    if (timer_id >= TOTAL_NUMBER_OF_TIMERS)
    {
        return PAL_TMR_INVALID_ID;
    }

    if (NULL == timer_cb)
    {
        return MAC_INVALID_PARAMETER;
    }

    if (NULL != timer_array[timer_id].timer_cb)
    {
        /*
         * Timer is already running if the callback function of the
         * corresponding timer index in the timer array is not NULL.
         */
        return PAL_TMR_ALREADY_RUNNING;
    }

    timer_count /= PAL_US_PER_SYMBOLS;

    switch(timeout_type)
    {
        case TIMEOUT_RELATIVE:
        {
            if ((timer_count > MAX_TIMEOUT / PAL_US_PER_SYMBOLS) || (timer_count < MIN_TIMEOUT / PAL_US_PER_SYMBOLS))
            {
                return PAL_TMR_INVALID_TIMEOUT;
            }

            point_in_time = timer_count + SC_READ32(SCCNT);
            point_in_time &= MASK_LOW28;
        }
        break;

        case TIMEOUT_ABSOLUTE:
        {
            uint32_t timeout;

            timeout = timer_count - SC_READ32(SCCNT);
            timeout &= MASK_LOW28;

            if ((timeout > MAX_TIMEOUT / PAL_US_PER_SYMBOLS) || (timeout < MIN_TIMEOUT / PAL_US_PER_SYMBOLS))
            {
                return PAL_TMR_INVALID_TIMEOUT;
            }
            point_in_time = timer_count;
        }
        break;

        default:
            return MAC_INVALID_PARAMETER;
    }

    start_absolute_timer(timer_id, point_in_time, timer_cb, param_cb);
    return MAC_SUCCESS;
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



#if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN))
/**
 * @brief Starts high priority timer
 *
 * This function starts a high priority timer for the specified timeout.
 *
 * @param timer_id Timer identifier
 * @param timer_count Timeout in microseconds
 * @param timer_cb Callback handler invoked upon timer expiry
 * @param param_cb Argument for the callback handler
 *
 * @return
 * - @ref PAL_TMR_INVALID_ID if the identifier is undefined,
 * - @ref MAC_INVALID_PARAMETER if the callback function for this timer is NULL,
 * - @ref PAL_TMR_ALREADY_RUNNING if the timer is already running, or
 * - @ref MAC_SUCCESS if timer is started successfully.
 */
retval_t pal_start_high_priority_timer(uint8_t timer_id,
                                       uint16_t timer_count,
                                       FUNC_PTR timer_cb,
                                       void *param_cb)
{
    timer_expiry_cb_t callback;
    uint32_t ocr;

    if (timer_id >= TOTAL_NUMBER_OF_TIMERS)
    {
        return PAL_TMR_INVALID_ID;
    }

    if (NULL == timer_cb)
    {
        return MAC_INVALID_PARAMETER;
    }

    if (NULL != timer_array[timer_id].timer_cb)
    {
        /*
         * Irrespective of the type, the timer is already running if the
         * callback function of the corresponding timer index in the timer
         * array is not NULL.
         */
        return PAL_TMR_ALREADY_RUNNING;
    }

    /*
     * A high priority timer can be started, as currently
     * there is no high priority timer running.
     */
    timer_count /= PAL_US_PER_SYMBOLS;
    if (timer_count == 0)
    {
        callback = (timer_expiry_cb_t)timer_cb;
        callback(param_cb);
        return MAC_SUCCESS;
    }

    ENTER_CRITICAL_REGION();

    high_priority_timer_id = timer_id;
    /*
     * The corresponding running timer queue's timer index is updated
     * with the new values
     */
    timer_array[timer_id].timer_cb = timer_cb;
    timer_array[timer_id].param_cb = param_cb;
    timer_array[timer_id].next_timer_in_queue = NO_TIMER;

    ocr = SC_READ32(SCCNT);
    ocr += timer_count;
    ocr &= MASK_LOW28;

    SCIRQS = _BV(IRQSCP3);
    SCIRQM |= _BV(IRQMCP3);

    SC_WRITE32(SCOCR3, ocr);

    timer_array[timer_id].abs_exp_timer = ocr;

    LEAVE_CRITICAL_REGION();

    return MAC_SUCCESS;
}
#endif /* #if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN)) */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Stops a running timer
 *
 * This function stops a running timer with the specified timer_id.
 *
 * @param timer_id Timer identifier
 *
 * @return
 * - @ref MAC_SUCCESS if the timer was stopped successfully,
 * - @ref PAL_TMR_NOT_RUNNING if the specified timer is not running,
 * - @ref PAL_TMR_INVALID_ID if the specified timer id is undefined.
 */
retval_t pal_timer_stop(uint8_t timer_id)
{
    bool timer_stop_request_status = false;
    uint8_t curr_index;
    uint8_t prev_index;


    if (timer_id >= TOTAL_NUMBER_OF_TIMERS)
    {
        return (PAL_TMR_INVALID_ID);
    }

    ENTER_CRITICAL_REGION();

    /* Check if any timer has expired. */
    internal_timer_handler();

    /* The requested timer is first searched in the running timer queue */
    if (running_timers > 0)
    {
        uint8_t timer_count = running_timers;
        prev_index = curr_index = running_timer_queue_head;
        while (timer_count > 0)
        {
            if (timer_id == curr_index)
            {
                timer_stop_request_status = true;

                if (timer_id == running_timer_queue_head)
                {
                    running_timer_queue_head =
                        timer_array[timer_id].next_timer_in_queue;
                    /*
                     * The value in OCR corresponds to the timeout pointed
                     * by the 'running_timer_queue_head'. As the head has
                     * changed here, OCR needs to be loaded by the new
                     * timeout value, if any.
                     */
                    prog_ocr();
                }
                else
                {
                    timer_array[prev_index].next_timer_in_queue =
                        timer_array[timer_id].next_timer_in_queue;
                }
                /*
                 * The next timer element of the stopped timer is updated
                 * to its default value.
                 */
                timer_array[timer_id].next_timer_in_queue = NO_TIMER;
                break;
            }
            else
            {
                prev_index = curr_index;
                curr_index = timer_array[curr_index].next_timer_in_queue;
            }
            timer_count--;
        }
        if (timer_stop_request_status)
        {
            running_timers--;
        }
    }

    /*
     * The requested timer is not present in the running timer queue.
     * It will be now searched in the expired timer queue
     */
    if (!timer_stop_request_status)
    {
        prev_index = curr_index = expired_timer_queue_head;
        while (NO_TIMER != curr_index)
        {
            if (timer_id == curr_index)
            {
                if (timer_id == expired_timer_queue_head)
                {
                    /*
                     * The requested timer is the head of the expired timer
                     * queue
                     */
                    if (expired_timer_queue_head == expired_timer_queue_tail)
                    {
                        /* Only one timer in expired timer queue */
                        expired_timer_queue_head = expired_timer_queue_tail =
                            NO_TIMER;
                    }
                    else
                    {
                        /*
                         * The head of the expired timer queue is moved to next
                         * timer in the expired timer queue.
                         */
                        expired_timer_queue_head =
                            timer_array[expired_timer_queue_head].next_timer_in_queue;
                    }
                }
                else
                {
                    /*
                     * The requested timer is present in the middle or at the
                     * end of the expired timer queue.
                     */
                    timer_array[prev_index].next_timer_in_queue =
                        timer_array[timer_id].next_timer_in_queue;

                    /*
                     * If the stopped timer is the one which is at the tail of
                     * the expired timer queue, then the tail is updated.
                     */
                    if (timer_id == expired_timer_queue_tail)
                    {
                        expired_timer_queue_tail = prev_index;
                    }
                }
                timer_stop_request_status = true;
                break;
            }
            else
            {
               prev_index = curr_index;
               curr_index = timer_array[curr_index].next_timer_in_queue;
            }
        }
    }

    if (timer_stop_request_status)
    {
        /*
         * The requested timer is stopped, hence the structure elements of the
         * timer are updated.
         */
        timer_array[timer_id].timer_cb = NULL;
    }

    LEAVE_CRITICAL_REGION();

    if (timer_stop_request_status)
    {
        return (MAC_SUCCESS);
    }

    return (PAL_TMR_NOT_RUNNING);
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



#if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN))
/**
 * @brief Stops a high priority timer
 *
 * This function stops a high priority timer.
 *
 * @param timer_id Timer identifier
 *
 * @return
 * - @ref PAL_TMR_NOT_RUNNING if the timer id does not match with the high priority
 * timer register, or
 * - @ref MAC_SUCCESS otherwise.
 */
retval_t pal_stop_high_priority_timer(uint8_t timer_id)
{
    retval_t timer_stop_status = PAL_TMR_NOT_RUNNING;

    ENTER_CRITICAL_REGION();

    if (timer_id == high_priority_timer_id)
    {
        /* Turn off timer1 SCOCR3 interrupt */
        SCIRQS = _BV(IRQSCP3);
        SCIRQM |= _BV(IRQMCP3);

        timer_array[high_priority_timer_id].next_timer_in_queue = NO_TIMER;
        timer_array[high_priority_timer_id].timer_cb = NULL;
        high_priority_timer_id = NO_TIMER;

        timer_stop_status = MAC_SUCCESS;
    }

    LEAVE_CRITICAL_REGION();

    return timer_stop_status;
}
#endif /* #if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN)) */



/**
 * This function is called to initialize the timer module.
 */
void timer_init(void)
{
#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
    /*
     * Initialize the timer resources like timer arrays
     * queues, timer registers
     */
    uint8_t index;

    running_timers = 0;
    timer_trigger = false;

    running_timer_queue_head = NO_TIMER;
    expired_timer_queue_head = NO_TIMER;
    expired_timer_queue_tail = NO_TIMER;
#ifdef ENABLE_HIGH_PRIO_TMR
    high_priority_timer_id = NO_TIMER;
#endif

    for (index = 0; index < TOTAL_NUMBER_OF_TIMERS; index++)
    {
        timer_array[index].next_timer_in_queue = NO_TIMER;
        timer_array[index].timer_cb = NULL;
    }
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */

    /*
     * Enable the 32 kHz crystal oscillator. This is required in
     * order to allow for the hardware performing a transparent clock
     * switch when the main 16 MHz crystal is turned off (i.e., when
     * the transceiver module is put to sleep).
     */
    ASSR = _BV(AS2);

    /*
     * Start the MAC symbol counter, resetting the prescaler by the
     * same time.  Also enable the rx timestamping feature.
     */
    SCCR0 = _BV(SCTSE) | _BV(SCEN) | _BV(SCRES);

    /*
     * Use SCOCR2 to implement a virtual 28-bit timer. This is needed
     * since all layers above PAL operate at a microsecond level, while
     * the MAC symbol counter operates at a granularity of MAC symbol
     * periods (16 µs).  Due to that, the timer values have to be
     * scaled at the PAL API border, and this scaling effectively
     * reduces the timer span to 28 bits.
     *
     * Whenever the timer is about to cross the 28-bit boundary, it is
     * advanced (by the ISR for SCOCR2) so the hardware will then
     * perform a real rollover at the next timer tick.  That way, the
     * worst effect from blocking interrupts for too long is that all
     * pending timers at the time of rollover will be delayed a bit.
     * If the SCOCR2 interrupt is served within 16 µs, no additional
     * delay will happen.
     */
    SC_WRITE32(SCOCR2, 0x0FFFFFFFul);
    SCIRQS = _BV(IRQSCP2);
    SCIRQM |= _BV(IRQMCP2);
}



/**
 * @brief Selects timer clock source
 *
 * This function selects the clock source of the timer.
 *
 * Unused for the ATmega128RFA1 because the MAC symbol counter
 * automatically switches from XTAL1 to RTC when the transceiver
 * module is put to sleep.
 *
 * @param source
 * - @ref TMR_CLK_SRC_DURING_TRX_SLEEP if clock source during sleep is to be selected, and
 * - @ref TMR_CLK_SRC_DURING_TRX_AWAKE if clock source while being awake is selected.
 */
void pal_timer_source_select(source_type_t source)
{
    /* Keep compiler happy. */
    source = source;
}



/**
 * @brief Gets current time
 *
 * This function returns the current time.
 *
 * @param[out] current_time Current system time
 */
void pal_get_current_time(uint32_t *current_time)
{
    while ((SCSR & _BV(SCBSY)) != 0)
    {
        ;
    }
    *current_time = PAL_US_PER_SYMBOLS * SC_READ32(SCCNT);
}



/**
 * @brief Provides timestamp of the last received frame
 *
 * This function provides the timestamp (in microseconds)
 * of the last received frame.
 *
 * @param[out] timestamp in microseconds
 */
void pal_trx_read_timestamp(uint32_t *timestamp)
{
    *timestamp = PAL_US_PER_SYMBOLS * SC_READ32(SCTSR);
}



/**
 * @brief Performes blocking delay
 *
 * This functions performs a blocking delay of the specified time.
 *
 * @param delay in microseconds
 */
void  pal_timer_delay(uint16_t delay)
{
    bool needs_cli = false;
    uint8_t sreg = 0;

    /*
     * Any interrupt occurring during the delay calculation will introduce
     * additional delay and can also affect the logic of delay calculation.
     * Hence the delay implementation is put under critical region as long
     * as only a small delay time is requested.
     */
    if (delay < DELAY_WITHOUT_IRQS_US)
    {
        needs_cli = true;
        sreg = SREG;
        cli();
    }

    /*
     * The logic below depends on the CPU being clocked with 16 MHz,
     * and timer 5 being set up for a prescaler of 8, so it will be
     * clocked with 2 MHz.  This is the best compromise to achieve the
     * 1 µs resolutions callers of pal_timer_delay() are expecting.
     *
     * In case any other CPU clock frequency should be supported that
     * cannot work this way, some of the logic below needs to be
     * rewritten.  Normally, there should be no real need for a
     * different CPU clock frequency, as the ATmega128RFA1 can operate
     * at 16 MHz down to the minimal possible supply voltage.
     *
     * As a special exception, the CPU frequency 15.3846 MHz will be
     * tolerated as it is used in some test setups.  This results in
     * delays being approximately 4 % too long which is deemed to be
     * acceptable for that particular purpose.
     */
#if (F_CPU != 16000000UL) && (F_CPU != 15384600UL)
#  error "pal_timer_delay() is only implemented for F_CPU = 16 MHz"
#endif
    /*
     * With a 2 MHz timer clock, it is not possible to span the entire
     * delay range of 65 ms using a single timer 5 cycle, so the delay
     * is split into multiple smaller delays if required.
     */
    while (delay != 0)
    {
        uint16_t actval;

        /*
         * Find whether the entire delay time will fit or not.  Note
         * that the 2 MHz figure below must *not* be calculated as
         * F_CPU / (1000000 * 8) because that will not work as
         * expected in the the 15.3 MHz case.
         */
        if (delay > UINT16_MAX / 2 /* MHz timer clock */)
        {
            actval = UINT16_MAX / 2;
        }
        else
        {
            actval = delay;
        }
        delay -= actval;        /* remaining delay for next round */

        /*
         * Perform the actual delay.  Setup TCNT5 so it will overflow
         * after actval cycles.  Then, clear any pending overflow
         * interrupt, and start timer 5 with a prescaler of 8.  Poll
         * for the overflow interrupt flag, and stop the timer again
         * afterwards.
         */
        actval *= 2 /* microseconds to timer 5 clock cycles @ 2 MHz */;
        TCNT5 = (uint16_t)(-actval);
        TIFR5 = _BV(TOV5);
        TCCR5B = _BV(CS51);
        while ((TIFR5 & _BV(TOV5)) == 0)
            /* delay */;
        TCCR5B = 0;
    }

    if (needs_cli)
    {
        SREG = sreg;
    }
}



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Checks if the timer with the requested timer identifier is running
 *
 * @param timer_id Timer identifier
 *
 * @return
 * - true if timer with requested timer id is running,
 * - false otherwise.
 */
bool pal_is_timer_running(uint8_t timer_id)
{
    if (NULL == timer_array[timer_id].timer_cb)
    {
        return false;
    }
    return true;
}
#endif  /* TOTAL_NUMBER_OF_TIMERS > 0 */



#if (DEBUG > 0)
/**
 * @brief Checks if all timers are stopped
 *
 * This function checks whether all timers are stopped or not.
 *
 * @return
 * - true if all timers are stopped,
 * - false otherwise.
 */
bool pal_are_all_timers_stopped(void)
{
#if (TOTAL_NUMBER_OF_TIMERS > 0)
    uint8_t timer_id;

    for (timer_id = 0; timer_id < TOTAL_NUMBER_OF_TIMERS; timer_id++)
    {
        if (NULL != timer_array[timer_id].timer_cb)
        {
            return false;
        }
    }
#endif
    return true;
}
#endif  /* (DEBUG > 0) */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Programs the output compare register SCOCR1
 *
 * This function programs the SCOCR1 register with the timeout value of the
 * timer present at the head of the running timer queue.
 */
static void prog_ocr(void)
{
    uint32_t new_ocr = timer_array[running_timer_queue_head].abs_exp_timer;

    if (running_timers != 0)
    {
        SC_WRITE32(SCOCR1, new_ocr);
        if ((SCIRQM & _BV(IRQMCP1)) == 0)
        {
            SCIRQS = _BV(IRQSCP1);
            SCIRQM |= _BV(IRQMCP1);
        }

        /* Safety check: Trigger timer, if next_trigger is in the past. */
        if (compare_time(new_ocr, SC_READ32(SCCNT) + 1))
        {
            timer_trigger = true;
        }
        else if ((VERSION_NUM <= 7) && (timer_last_trigger + 1 == new_ocr))
        {
            /*
             * ATmega128RFA1 1.x: new SCOCRx value and value where
             * SCOCRx has just triggered an interrupt must differ by
             * at least 2.
             *
             * Just wait a little moment here, and then proceed as if
             * the ISR had just triggered.
             */
            while (SC_READ32(SCCNT) != new_ocr)
                /* wait */;
            timer_trigger = true;
        }
    }
    else
    {
        SCIRQM &= ~_BV(IRQMCP1);
    }
}
#endif  /* TOTAL_NUMBER_OF_TIMERS > 0 */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Internal timer handler
 *
 * This function checks for expired timers and rearranges the
 * running timer queue head and expired timer queue head and tail
 * if there are any expired timers.
 */
void internal_timer_handler(void)
{
    ENTER_CRITICAL_REGION();

    /*
     * Flag was set once a timer has expired by the timer ISR or
     * by function prog_rc().
     */
    if (timer_trigger)
    {
        timer_trigger = false;

        if (running_timers > 0) /* Holds the number of running timers */
        {
            if ((expired_timer_queue_head == NO_TIMER) &&
                (expired_timer_queue_tail == NO_TIMER))
            {
                expired_timer_queue_head = expired_timer_queue_tail =
                                                    running_timer_queue_head;
            }
            else
            {
                timer_array[expired_timer_queue_tail].next_timer_in_queue =
                                                    running_timer_queue_head;

                expired_timer_queue_tail = running_timer_queue_head;
            }

            running_timer_queue_head =
                timer_array[running_timer_queue_head].next_timer_in_queue;

            timer_array[expired_timer_queue_tail].next_timer_in_queue =
                NO_TIMER;

            running_timers--;

            /*
             * As a timer has expired, the SCOCR1 is programmed (if possible)
             * with the new timeout value of the timer pointed by running
             * timer queue head
             */
            prog_ocr();
        }
    }

    LEAVE_CRITICAL_REGION();
}
#endif  /* TOTAL_NUMBER_OF_TIMERS > 0 */



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Start a timer by setting its absolute expiration time
 *
 * This function starts a timer which should expire at the
 * point_in_time value and upon timer expiry the function
 * held by the handler is called.
 *
 * @param timer_id Timer identifier
 * @param point_in_time Absolute expiration time in microseconds
 * @param handler_cb Function called upon timer expiry
 * @param parameter Parameter passed to the expired timer handler
 */
static void start_absolute_timer(uint8_t timer_id,
                          uint32_t point_in_time,
                          FUNC_PTR handler_cb,
                          void *parameter)
{
    ENTER_CRITICAL_REGION();

    point_in_time &= MASK_LOW28;

    /* Check is done to see if any timer has expired */
    internal_timer_handler();

    if (NO_TIMER == running_timer_queue_head)
    {
        running_timer_queue_head = timer_id;
        timer_array[timer_id].next_timer_in_queue = NO_TIMER;
    }
    else
    {
        uint8_t i;
        bool timer_inserted = false;
        uint8_t curr_index = running_timer_queue_head;
        uint8_t prev_index = running_timer_queue_head;

        for (i = 0; i < running_timers; i++)
        {
            if (NO_TIMER != curr_index)
            {
                if (compare_time(timer_array[curr_index].abs_exp_timer,
                                     point_in_time))
                {
                    /*
                     * Requested absolute time value is greater than the time
                     * value pointed by the curr_index in the timer array
                     */
                    prev_index = curr_index;
                    curr_index = timer_array[curr_index].next_timer_in_queue;
                }
                else
                {
                    timer_array[timer_id].next_timer_in_queue = curr_index;
                    if (running_timer_queue_head == curr_index)
                    {
                        /* Insertion at the head of the timer queue. */
                        running_timer_queue_head = timer_id;
                    }
                    else
                    {
                        timer_array[prev_index].next_timer_in_queue = timer_id;
                    }
                    timer_inserted = true;
                    break;
                }
            }
        }
        if (!timer_inserted)
        {
            /* Insertion at the tail of the timer queue. */
            timer_array[prev_index].next_timer_in_queue = timer_id;
            timer_array[timer_id].next_timer_in_queue = NO_TIMER;
        }
    }
    timer_array[timer_id].abs_exp_timer = point_in_time;
    timer_array[timer_id].timer_cb = (FUNC_PTR)handler_cb;
    timer_array[timer_id].param_cb = parameter;
    running_timers++;

    prog_ocr();

    LEAVE_CRITICAL_REGION();
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



#if defined(DOXYGEN)
/**
 * @brief Timer1 COMPA ISR
 *
 * This is the interrupt service routine for timer1 output compare match.
 * It is triggered when a timer expires.
 */
void SCNT_CMP1_vect(void);
#else  /* !DOXYGEN */
ISR(SCNT_CMP1_vect)
{
#if (TOTAL_NUMBER_OF_TIMERS > 0)
    timer_last_trigger = SC_READ32(SCCNT);

    if (running_timers > 0)
    {
        timer_trigger = true;
    }
#endif  /* #if (TOTAL_NUMBER_OF_TIMERS > 0) */
}
#endif /* defined(DOXYGEN) */

#if defined(DOXYGEN)
/**
 * @brief ISR to limit the MAC symbol counter to 28 bits width.
 *
 * The current counter value will be advanced to -1, so it rolls over
 * to 0 at the next timer tick.
 */
void SCNT_CMP2_vect(void);
#else  /* !DOXYGEN */
ISR(SCNT_CMP2_vect)
{
    SC_WRITE32(SCCNT, 0xFFFFFFFF);
}
#endif /* defined(DOXYGEN) */


#if defined(DOXYGEN)
/**
 * @brief ISR for high priority timer, triggered by SCOCR2
 *
 * This is the interrupt service routine for SCOCR2 output compare match.
 * It is triggered when a high priority timer expires.
 */
void SCNT_CMP3_vect(void);
#else  /* !DOXYGEN */
#ifdef ENABLE_HIGH_PRIO_TMR
ISR(SCNT_CMP3_vect)
{
    /* Turn off SCOCR2 interrupt */
    SCIRQM &= ~_BV(IRQMCP3);

    timer_expiry_cb_t callback = (timer_expiry_cb_t)
                           (timer_array[high_priority_timer_id].timer_cb);
    void *param = timer_array[high_priority_timer_id].param_cb;

    timer_array[high_priority_timer_id].timer_cb = NULL;
    timer_array[high_priority_timer_id].next_timer_in_queue = NO_TIMER;

    high_priority_timer_id = NO_TIMER;

    ASSERT(NULL != callback);

    /* The callback function registered for this timer is called */
    callback(param);
}
#endif /* ENABLE_HIGH_PRIO_TMR */
#endif /* defined(DOXYGEN) */

/* EOF */
