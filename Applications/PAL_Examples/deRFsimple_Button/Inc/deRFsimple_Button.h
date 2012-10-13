/**
 * @file deRFsimple_Button.h
 *
 * @brief Application header file for PAL Demo Application "deRFsimple_Button".
 *
 * SensTermBoard Example:
 * The PAL Example "deRFsimple_Button" demonstrates the access to resources 
 * of the SensorTerminalBoard (LED/Button) via PAL layer.
 * The example code toggles LED_0 and LED_1 every time a button is pressed.
 *
 * $Id: deRFsimple_Button.h,v 1.1.6.1 2010/09/07 17:39:38 dam Exp $
 *
 * @author    dresden elektronik: http://www.dresden-elektronik.de
 * @author    Support email: support@dresden-elektronik.de
 *
 *
 * Copyright (c) 2009, dresden elektronik All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * - Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

/* Prevent double inclusion */
#ifndef deRFsimple_Button_h
#define deRFsimple_Button_h

/* === Includes ============================================================= */

#include "tal_types.h"      // ATMEGARF_TAL_1

#if ((defined TAL_TYPE) && (TAL_TYPE == ATMEGARF_TAL_1))
#include "stack_config.h"   // Layer definitions
#include "atmega128rfa1.h"
#endif

/* === Macros =============================================================== */

/** Interval for button debounce intervall in µs. */
#define DEBOUNCE_PERIOD (10000)

/** Number of necessarry consecutive "button pressed" detections, before a button event is signalled. */
#define KEY_UP_COUNT (15)

#ifndef ASSERT
  /** ASSERT macro, which blocks if the expression "expr" is not true. */
# define ASSERT(expr) do{while(!expr);}while(0);
#endif

/* === Types ================================================================ */

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

int main(void);
void app_init(void);
void app_task(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* === INLINE FUNCTIONS ==================================================== */

#endif /* deRFsimple_Button_h */

/* EOF */
