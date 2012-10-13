/**
 * @file pal_timer.c
 *
 * @brief Timer related functions for ARM7 MCUs
 *
 * This file implements timer related functions for ARM7 MCUs.
 *
 * $Id: pal_timer.c,v 1.2.2.2 2010/09/07 17:38:25 dam Exp $
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

#endif  /* #if (TOTAL_NUMBER_OF_TIMERS > 0) */

/* This variable holds the id of high priority timer */
#ifdef ENABLE_HIGH_PRIO_TMR
static volatile uint8_t high_priority_timer_id;
#endif
/* This flag gives indication of whether compare register is loaded or not */
static volatile bool reg_rc_loaded;

/* === Prototypes =========================================================== */

#if (TOTAL_NUMBER_OF_TIMERS > 0)
static void prog_reg_rc(void);
static void start_absolute_timer(uint8_t timer_id,
                                 uint32_t point_in_time,
                                 FUNC_PTR handler_cb,
                                 void *parameter);
#endif  /* #if (TOTAL_NUMBER_OF_TIMERS > 0) */
static inline uint32_t gettime(void);

/* === Implementation ======================================================= */

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
    return ((t2 - t1) < INT32_MAX);
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
    ENTER_CRITICAL_REGION();
    internal_timer_handler();
    LEAVE_CRITICAL_REGION();

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
    uint32_t now;
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

    now = gettime();

    /* The timeout is pre scaled according to the clock period. */
    timer_count = (uint32_t)(timer_count / CLOCK_PERIOD);

    switch(timeout_type)
    {
        case TIMEOUT_RELATIVE:
        {
            // FIXME
            // Limit timers that run longer that allowed to the max. possible timeout.
            // This e.g. happens when starting a Timer waiting for Beacons
            // while BO==15 && F_CPU>32MHz
            // [refer to mac_start_missed_beacon_timer @mac_sync.c]
            if (timer_count > MAX_TIMEOUT)
            {
                timer_count = MAX_TIMEOUT;
            }

            if ( (timer_count > MAX_TIMEOUT) || (timer_count < MIN_TIMEOUT))
            {
                return PAL_TMR_INVALID_TIMEOUT;
            }

            point_in_time = ADD_TIME(timer_count, now);
        }
        break;

        case TIMEOUT_ABSOLUTE:
        {
            uint32_t timeout;

            timeout = SUB_TIME(timer_count, now);

            if ((timeout > MAX_TIMEOUT) || (timeout < MIN_TIMEOUT))
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
    {
        uint32_t tc_status;

        ENTER_CRITICAL_REGION();

        high_priority_timer_id = timer_id;
        /*
         * The corresponding running timer queue's timer index is updated
         * with the new values.
         */
        timer_count = (uint32_t)(timer_count / CLOCK_PERIOD);
        timer_array[timer_id].abs_exp_timer = timer_count;
        timer_array[timer_id].timer_cb = timer_cb;
        timer_array[timer_id].param_cb = param_cb;
        timer_array[timer_id].next_timer_in_queue = NO_TIMER;

        LEAVE_CRITICAL_REGION();

        /*
         * The status register is read to clear any pending compare match
         * interrupts.
         */
        tc_status = AT91C_BASE_TC1->TC_SR;

        /* The timer channel 1 interrupt in AIC is enabled. */
        AT91C_BASE_AIC->AIC_IECR = _BV(AT91C_ID_TC1);

        /* The compare match interrupt is enabled for timer channel 1. */
        AT91C_BASE_TC1->TC_IER = AT91C_TC_CPCS;

        /* The compare register is programmed. */
        AT91C_BASE_TC1->TC_RC = timer_count;

        /* Clock from PMC, for timer channel 1 is enabled here. */
        AT91C_BASE_PMC->PMC_PCER = _BV(AT91C_ID_TC1);

        /* Clock is enabled at tht timer level. */
        AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN;

        /* Timer channel 1 is triggered. */
        AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG ;

        /*
         * Done to avoid compiler warning about variable being not used after
         * setting.
         */
        tc_status = tc_status;
    }

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
                     * The value in TC_RC corresponds to the timeout pointed
                     * by the 'running_timer_queue_head'. As the head has
                     * changed here, TC_RC needs to be loaded by the new
                     * timeout value, if any.
                     */
                    prog_reg_rc();
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
    uint32_t tc_status;

    ENTER_CRITICAL_REGION();

    if (timer_id == high_priority_timer_id)
    {
        /*
         * The clock to the timer channel 1 is disabled here. It will be
         * re-enabled again while starting the high priority timer next time.
         */
        AT91C_BASE_PMC->PMC_PCDR = _BV(AT91C_ID_TC1);

        /* The compare match interrupt of timer channel 1 is disabled. */
        AT91C_BASE_TC1->TC_IDR = AT91C_TC_CPCS;

        /*
         * The status register is read to clear any pending compare match
         * interrupts.
         */
        tc_status = AT91C_BASE_TC1->TC_SR;

        /*
         * Done to avoid compiler warning about variable being not used after
         * setting.
         */
        tc_status = tc_status;

        /* The timer channel 1 interrupts in AIC is disabled. */
        AT91C_BASE_AIC->AIC_IDCR = _BV(AT91C_ID_TC1);

        timer_array[high_priority_timer_id].next_timer_in_queue = NO_TIMER;
        timer_array[high_priority_timer_id].timer_cb = NULL;
        high_priority_timer_id = NO_TIMER;

        timer_stop_status = MAC_SUCCESS;
    }

    LEAVE_CRITICAL_REGION();

    return timer_stop_status;
}
#endif /* #if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN)) */



/*
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
    sys_time = 0;

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

    /* Do non-generic/PAL specific actions here. */
    timer_init_non_generic();
}



/**
 * @brief Selects timer clock source
 *
 * This function selects the clock source of the timer.
 *
 * @param source
 * - @ref TMR_CLK_SRC_DURING_TRX_SLEEP if clock source during sleep is to be selected, and
 * - @ref TMR_CLK_SRC_DURING_TRX_AWAKE if clock source while being awake is selected.
 */
void pal_timer_source_select(source_type_t source)
{
    if (TMR_CLK_SRC_DURING_TRX_SLEEP == source)
    {
        TIMER_SRC_DURING_TRX_SLEEP();
    }
    else
    {
        TIMER_SRC_DURING_TRX_AWAKE();
    }
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
    *current_time = (uint32_t)((gettime() * CLOCK_PERIOD));
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
    /*
     * Any interrupt occurring during the delay calculation will introduce
     * additional delay and can also affect the logic of delay calculation.
     * Hence the delay implementation is put under critical region.
     */

    ENTER_CRITICAL_REGION();

    delay = (uint32_t)(delay / CLOCK_PERIOD);
    uint16_t target_time =  AT91C_BASE_TC0->TC_CV + delay;

    if (target_time > AT91C_BASE_TC0->TC_CV)
    {
        /* Target time is within current span of 16 bit count (0xFFFF). */
        while (target_time > AT91C_BASE_TC0->TC_CV)
        {
            /* No operation, till condition is met. */
        }
    }
    else
    {
        /*
         * Target time has rolled over and has gone to next span of 16 bit
         * count.
         */
        while (target_time < AT91C_BASE_TC0->TC_CV)
        {
            /* No operation, till condition is met. */
        }
        while (target_time > AT91C_BASE_TC0->TC_CV)
        {
            /* No operation, till condition is met. */
        }
    }

    LEAVE_CRITICAL_REGION();
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



/**
 * @brief Gets actual system time
 *
 * This function is called to get the system time
 *
 * @return Time in microseconds
 */
static inline uint32_t gettime(void)
{
    uint16_t current_sys_time;
    uint32_t current_time;

    do
    {
        current_sys_time = sys_time;
        current_time = current_sys_time;
        current_time = current_time << 16;
        current_time = current_time | AT91C_BASE_TC0->TC_CV;
        /*
         * This calculation is valid only if the timer has not rolled over.
         * The sys_time variable may have changed in the timer overflow ISR.
         */
    } while (current_sys_time != sys_time);

    return current_time;
}



#if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN))
/**
 * @brief Programs the TC_RC compare register of timer channel 0
 *
 * This function programs the TC_RC compare register of timer channel 0 with
 * the timeout value of the timer present at the head of the running queue.
 */
static void prog_reg_rc(void)
{
    uint16_t timeout_high;
    uint16_t timeout_low;
    uint32_t timeout;
    uint32_t tc_status;

    ENTER_CRITICAL_REGION();

    if (NO_TIMER != running_timer_queue_head)
    {
        timeout = timer_array[running_timer_queue_head].abs_exp_timer;
        timeout_high = (uint16_t)(timeout >> SYS_TIME_SHIFT_MASK);

        if (timeout_high == sys_time)
        {
            timeout_low = (uint16_t)(timeout & HW_TIME_MASK);

            /*
             * The status register is read to clear any pending compare match
             * interrupts.
             */
            tc_status = AT91C_BASE_TC0->TC_SR;

            /* The compare register is programmed */
            AT91C_BASE_TC0->TC_RC = timeout_low;

            /*
             * The output compare match of timer channel 0 interrupt is
             * enabled.
             */
            AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;

            /*
             * As the timer status register read above, it is necessary to check
             * whether the overflow bit is set. If the counter has overflowed
             * when the execution has just entered the function, no interrupt
             * will be generated, as all the callees of this function call this
             * function when in critical region. Reading the status register
             * will clear the overflow bit and hence it has to be acknowledged
             * here.
             */
            if (AT91C_TC_COVFS == tc_status)
            {
                sys_time++;
            }

            reg_rc_loaded = true;
        }
        else
        {
            /*
             * The output compare match of timer channel 0 interrupt is
             * disabled.
             */
            AT91C_BASE_TC0->TC_IDR = AT91C_TC_CPCS;

            reg_rc_loaded = false;
        }

        uint32_t current_time = gettime();

        /* Safety check: Trigger timer, if next_trigger is in the past. */
        if (compare_time(timer_array[running_timer_queue_head].abs_exp_timer, current_time + 1))
        {
            timer_trigger = true;
        }
    }

    LEAVE_CRITICAL_REGION();
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



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
    /*
     * Flag was set once a timer has expired by the timer ISR or
     * by function prog_reg_rc().
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
             * As a timer has expired, the TC_RC is programmed (if possible)
             * with the new timeout value of the timer pointed by running
             * timer queue head
             */
            prog_reg_rc();
        }
    }
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



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

    /* Check is done to see if any timer has expired */
    internal_timer_handler();

    bool load_reg_rc = false;

    if (NO_TIMER == running_timer_queue_head)
    {
        running_timer_queue_head = timer_id;
        timer_array[timer_id].next_timer_in_queue = NO_TIMER;
        /*
         * This is the only timer running in the timer queue, hence load the
         * TC_RC register.
         */
        load_reg_rc = true;
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
                        /*
                         * Timer is inserted at the head of the queue, hence
                         * load the TC_RC.
                         */
                        load_reg_rc = true;
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

    /*
     * If there is only one timer in the timer queue
     * the timeout should be loaded immediately
     */
    if (load_reg_rc)
    {
        prog_reg_rc();
    }

    LEAVE_CRITICAL_REGION();
}
#endif  /* #if ((TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)) */



/**
 * @brief Timer channel 0 ISR
 *
 * This is the interrupt service routine for timer channel 0 interrupts.
 * The timer counter overflow and compare match interrupts of regular timer
 * will be handled here.
 */
void timer_ch0_irq_handler(void)
{
    uint32_t irq_status;

    /*
     * The interrupt is acknowledged by reading the status register.
     * This clears the corresponding bit in the status register, which triggered
     * the interrupt.
     */
    irq_status = AT91C_BASE_TC0->TC_SR;

#if (TOTAL_NUMBER_OF_TIMERS > 0)
    if (AT91C_TC_CPCS & irq_status)
    {
        /*
         * This is a compare RC match interrupt. The correspondig status bit in
         * the status register is set. But only if the TC_RC was loaded prior to
         * this and reg_rc_loaded set to true, the match will be considered to
         * be a true one. This is because, at any given point of time, TC_RA
         * register will have some value in it and will match with the TC_CV
         * value once every 16bit count cycle of the timer channel 0.
         */
        if ((reg_rc_loaded) && (running_timers > 0))
        {
            reg_rc_loaded = false;
            timer_trigger = true;
        }
    }
#endif

    if (AT91C_TC_COVFS & irq_status)
    {
        /*
         * Counter has overflowed, hence the 16 bit software time is
         * incremented and the TC_RC is tried to be programmed.
         */
        sys_time++;
#if (TOTAL_NUMBER_OF_TIMERS > 0)
        prog_reg_rc();
#endif
    }
}



#if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN))
/**
 * @brief Timer channel 1 ISR
 *
 * This is the interrupt service routine for timer channel 1 interrupt.
 * The compare match interrupt of high priority timer will be handeled here.
 */
void timer_ch1_irq_handler(void)
{
    uint32_t irq_status;

    /* The compare match interrupts are disabled for timer channel 1. */
    AT91C_BASE_TC1->TC_IDR = AT91C_TC_CPCS;

    /*
     * The interrupt is acknowledged by reading the status register. This
     * clears the interrupt at the timer level.
     */
    irq_status = AT91C_BASE_TC1->TC_SR;

    /*
     * The clock to the timer channel 1 is disabled here. It will be re-enabled
     * again while starting the high priority timer next time.
     */
    AT91C_BASE_PMC->PMC_PCDR = _BV(AT91C_ID_TC1);

    /*
     * Done to avoid compiler warning about variable being not used after
     * setting.
     */
    irq_status = irq_status;

    /*
     * The callback installed at the time of starting the timer is called along
     * with the provided callback parameter.
     */
    timer_expiry_cb_t callback = (timer_expiry_cb_t)\
                           (timer_array[high_priority_timer_id].timer_cb);

    void *param = timer_array[high_priority_timer_id].param_cb;

    /*
     * The paramters of the timer array corresponding to the timer id are reset
     * so that the same timer can be started again in the timer callback, if
     * required.
     */
    timer_array[high_priority_timer_id].timer_cb = NULL;
    timer_array[high_priority_timer_id].next_timer_in_queue = NO_TIMER;

    high_priority_timer_id = NO_TIMER;

    ASSERT(NULL != callback);

    /* The callback function registered for this timer is called */
    callback(param);
}
#endif /* #if ((defined ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN)) */

/* EOF */

