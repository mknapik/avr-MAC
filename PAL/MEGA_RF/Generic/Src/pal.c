/**
 * @file pal.c
 *
 * @brief General PAL functions for AVR 8-Bit Mega RF Single Chips
 *
 * This file implements generic PAL function and performs
 * USB related handling if required for AVR 8-Bit Mega RF Single Chips.
 *
 * $Id: pal.c 21659 2010-04-16 13:06:40Z uwalter $
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
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "pal_config.h"
#include "pal_timer.h"
#include "pal_internal.h"
#include "app_config.h"

#ifdef USB0
#include "pal_usb.h"
#endif /* USB0 */

/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Initialization of PAL
 *
 * This function initializes the PAL. The RC Oscillator is calibrated.
 *
 * @return MAC_SUCCESS  if PAL initialization is successful, FAILURE otherwise
 */
retval_t pal_init(void)
{
    mcu_init();
#if (EXTERN_EEPROM_AVAILABLE == 1)
    extern_eeprom_init();
#endif
    timer_init();
#ifdef RELAY_SUPPORT
    RELAY_PIN_1_INIT();
    RELAY_PIN_2_INIT();
#endif

    return MAC_SUCCESS;
}



/**
 * @brief Services timer and sio handler
 *
 * This function calls sio & timer handling functions.
 */
void pal_task(void)
{
#if (TOTAL_NUMBER_OF_TIMERS > 0)
    timer_service();
#endif

/*
 * If the serial communication is done using USB, only then the 'usb_handler'
 * needs to be called. For UART, the data handling is done in the UART ISR.
 */
#ifdef USB0
    usb_handler();
#endif /* USB0 */
}



/**
 * @brief Get data from persistence storage
 *
 * @param[in]  ps_type Persistence storage type
 * @param[in]  start_addr Start offset within EEPROM
 * @param[in]  length Number of bytes to read from EEPROM
 * @param[out] value Data from persistence storage
 *
 * @return MAC_SUCCESS  if everything went OK else FAILURE
 */
retval_t pal_ps_get(ps_type_t ps_type, uint16_t start_addr, uint16_t length, void *value)
{
#if (EXTERN_EEPROM_AVAILABLE == 1)
    if (ps_type == EXTERN_EEPROM)
    {
        return extern_eeprom_get(start_addr, length, value);
    }
    else
#endif

    if (ps_type == INTERN_EEPROM)
    {
        uint16_t index;
        uint8_t *data_ptr;

        if ((start_addr + length) > (E2END + 1))
        {
            return FAILURE;
        }

        data_ptr = (uint8_t *)(value);
        for (index = 0; index < length; index++)
        {
            EEGET(*data_ptr, start_addr);
            start_addr++;
            data_ptr++;
        }
    }
    else    // no internal eeprom and no external eeprom available
    {
        return MAC_INVALID_PARAMETER;
    }

    return MAC_SUCCESS;
}



/**
 * @brief Write data to persistence storage
 *
 * @param[in]  start_addr Start address offset within EEPROM
 * @param[in]  length Number of bytes to be written to EEPROM
 * @param[in]  value Data to persistence storage
 *
 * @return MAC_SUCCESS  if everything went OK else FAILURE
 */
retval_t pal_ps_set(uint16_t start_addr, uint16_t length, void *value)
{
    uint8_t *data_ptr;
    uint16_t i;
    uint8_t read_data;

    if ((start_addr + length) > (E2END + 1))
    {
        return FAILURE;
    }

    data_ptr = (uint8_t *)(value);
    for (i = 0; i < length; i++)
    {
        EEGET(read_data, start_addr);
        if (read_data != *data_ptr)
        {
            EEPUT(start_addr, *data_ptr);
        }
        data_ptr++;
        start_addr++;
    }

    return MAC_SUCCESS;
}



/*
 * @brief Alert indication
 *
 * This Function can be used by any application to indicate an error condition.
 * The function is blocking and does never return.
 */
void pal_alert(void)
{
#if (DEBUG > 0)
    bool debug_flag = false;
#endif
    ALERT_INIT();

    while (1)
    {
        pal_timer_delay(0xFFFF);
        ALERT_INDICATE();

#if (DEBUG > 0)
        /* Used for debugging purposes only */
        if (debug_flag == true)
        {
            break;
        }
#endif
    }
}


/* EOF */
