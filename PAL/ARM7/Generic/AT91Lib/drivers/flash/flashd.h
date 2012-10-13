/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 * ----------------------------------------------------------------------------
 * Copyright (c) 2007, Atmel Corporation

 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaiimer below.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation and/or
 * other materials provided with the distribution.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

#ifndef FLASHD_H
#define FLASHD_H

//------------------------------------------------------------------------------
//         Functions
//------------------------------------------------------------------------------

extern void FLASHD_Initialize(unsigned int mck);

extern unsigned char FLASHD_Erase(void);

extern unsigned char FLASHD_Write(
    unsigned int address,
    const void *pBuffer,
    unsigned int size);

extern unsigned char FLASHD_Lock(
    unsigned int start,
    unsigned int end,
    unsigned int *pActualStart,
    unsigned int *pActualEnd);

extern unsigned char FLASHD_Unlock(
    unsigned int start,
    unsigned int end,
    unsigned int *pActualStart,
    unsigned int *pActualEnd);

extern unsigned char FLASHD_IsLocked(
    unsigned int start,
    unsigned int end);

extern unsigned char FLASHD_SetGPNVM(unsigned char gpnvm);

extern unsigned char FLASHD_ClearGPNVM(unsigned char gpnvm);

extern unsigned char FLASHD_IsGPNVMSet(unsigned char gpnvm);

#endif //#ifndef FLASHD_H

