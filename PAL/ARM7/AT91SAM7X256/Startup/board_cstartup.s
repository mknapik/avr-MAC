/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
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

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

.set IRQ_STACK_SIZE,   8*3*4

/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs (program status registers)      */
.set  ARM_MODE_USR, 0x10                /* Normal User Mode                                              */
.set  ARM_MODE_FIQ, 0x11                /* FIQ Processing Fast Interrupts Mode                           */
.set  ARM_MODE_IRQ, 0x12                /* IRQ Processing Standard Interrupts Mode                       */
.set  ARM_MODE_SVC, 0x13                /* Supervisor Processing Software Interrupts Mode                */
.set  ARM_MODE_ABT, 0x17                /* Abort Processing memory Faults Mode                           */
.set  ARM_MODE_UND, 0x1B                /* Undefined Processing Undefined Instructions Mode              */
.set  ARM_MODE_SYS, 0x1F                /* System Running Priviledged Operating System Tasks  Mode       */
.set  I_BIT, 0x80                       /* when I bit is set, IRQ is disabled (program status registers) */
.set  F_BIT, 0x40                       /* when F bit is set, FIQ is disabled (program status registers) */

/* Addresses and offsets of AIC and PIO  */
.set  AT91C_BASE_AIC, 0xFFFFF000        /* (AIC) Base Address                         */
.set  AT91C_PIOA_CODR, 0xFFFFF434       /* (PIO) Clear Output Data Register           */
.set  AT91C_AIC_IVR, 0xFFFFF100         /* (AIC) IRQ Interrupt Vector Register        */
.set  AT91C_AIC_FVR, 0xFFFFF104         /* (AIC) FIQ Interrupt Vector Register        */
.set  AIC_IVR, 256                      /* IRQ Vector Register offset from base above */
.set  AIC_FVR, 260                      /* FIQ Vector Register offset from base above */
.set  AIC_EOICR, 304                    /* End of Interrupt Command Register          */

//------------------------------------------------------------------------------
//         Startup routine
//------------------------------------------------------------------------------

            .align      4
            .arm

/* Exception vectors
 *******************/
            .section    .vectors, "a"

resetVector:
        ldr     pc, =resetHandler       /* Reset */
undefVector:
        b       undefVector             /* Undefined instruction */
swiVector:
        b       swiVector               /* Software interrupt */
prefetchAbortVector:
        b       prefetchAbortVector     /* Prefetch abort */
dataAbortVector:
        b       dataAbortVector         /* Data abort */
reservedVector:
        b       reservedVector          /* Reserved for future use */
irqVector:
        b       irqHandler              /* Interrupt */
fiqVector:
                                        /* Fast interrupt */
/*
   Handles a fast interrupt request by branching to the address defined in the
   AIC.
 */
fiqHandler:
        b       fiqHandler

/*
   Handles incoming interrupt requests by branching to the corresponding
   handler, as defined in the AIC. Supports interrupt nesting.
 */
irqHandler:

/* Save interrupt context on the stack to allow nesting */
        sub     lr, lr, #4
        stmfd   sp!, {lr}
        mrs     lr, SPSR
        stmfd   sp!, {r0, lr}

/* Write in the IVR to support Protect Mode */
        ldr     lr, =AT91C_BASE_AIC
        ldr     r0, [r14, #AIC_IVR]
        str     lr, [r14, #AIC_IVR]

/* Branch to interrupt handler in Supervisor mode */
        msr     CPSR_c, #ARM_MODE_SVC
        stmfd   sp!, {r1-r3, r12, lr}
        mov     lr, pc
        bx      r0
        ldmia   sp!, {r1-r3, r12, lr}
        msr     CPSR_c, #ARM_MODE_IRQ | I_BIT

/* Acknowledge interrupt */
        ldr     lr, =AT91C_BASE_AIC
        str     lr, [r14, #AIC_EOICR]

/* Restore interrupt context and branch back to calling code */
        ldmia   sp!, {r0, lr}
        msr     SPSR_cxsf, lr
        ldmia   sp!, {pc}^

/*
   Initializes the chip and branches to the main() function.
 */
            .section    .text
            .global     entry
            .extern     arm7_low_level_init

entry:
resetHandler:

/* Dummy access to the .vectors section so it does not get optimized */
        ldr     r0, =resetVector

/* Set pc to actual code location (i.e. not in remap zone) */
        ldr     pc, =1f

/* Perform low-level initialization of the chip using LowLevelInit() */
1:
        ldr     r4, =_sstack
        mov     sp, r4
        ldr     r0, =arm7_low_level_init
        mov     lr, pc
        bx      r0

/* Initialize the relocate segment */

        ldr     r0, =_efixed
        ldr     r1, =_srelocate
        ldr     r2, =_erelocate
1:
        cmp     r1, r2
        ldrcc   r3, [r0], #4
        strcc   r3, [r1], #4
        bcc     1b

/* Clear the zero segment */
        ldr     r0, =_szero
        ldr     r1, =_ezero
        mov     r2, #0
1:
        cmp     r0, r1
        strcc   r2, [r0], #4
        bcc     1b

/* Setup stacks
 **************/
/* IRQ mode */
        msr     CPSR_c, #ARM_MODE_IRQ | I_BIT | F_BIT
        mov     sp, r4
        sub     r4, r4, #IRQ_STACK_SIZE

/* Supervisor mode (interrupts enabled) */
        msr     CPSR_c, #ARM_MODE_SVC | F_BIT
        mov     sp, r4

/* Branch to main()
 ******************/
        ldr     r0, =main
        mov     lr, pc
        bx      r0

/* Loop indefinitely when program is finished */
1:
        b       1b

