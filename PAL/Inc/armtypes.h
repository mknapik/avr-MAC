/**
 * @file armtypes.h
 *
 * @brief Compatibility definitions for compilers (IAR, GCC)
 *
 * This file contains ARM type definitions that enable Atmel's 802.15.4
 * stack implementation to build using multiple compilers.
 *
 * $Id: armtypes.h 22828 2010-08-10 07:29:54Z sschneid $
 *
 */
/**
 *  @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef ARMTYPES_H
#define ARMTYPES_H

/* === Includes ============================================================= */

/*
 * The if defined(__ICCARM__) section below applies to the IAR compiler.
 */
#if defined(__ICCARM__)

#include <intrinsics.h>

/*
 * The elif defined(__GNUC__) section below applies to the GNUC compiler.
 */
#elif defined(__GNUC__)

/*
 * The else section below applies to unsupported compiler. This is where support
 * for compilers other than IAR and GNUC would be placed.
 */
#else
#error Unsupported compiler
#endif /* compiler selection */

/* === Externals ============================================================ */


/* === Macros =============================================================== */

#ifndef _BV
/**
 * Bit value -- compute the bitmask for a bit position
 */
#define _BV(x) (1 << (x))
#endif

/*
 * This block contains just the documentation for all
 * compiler-dependent macros.
 */
#if defined(DOXYGEN)

/**
 * Globally enable interrupts.
 */
#define sei()

/**
 * Globally disable interrupts.
 */
#define cli()

/**
 * Read contents of EEPROM cell \c addr into variable \c var.
 */
#define EEGET(var, addr)

/**
 * Null operation: just waste one CPU cycle.
 */
#define nop()

/**
 * Attribute to apply to struct tags in order to force it into an 8-bit
 * alignment.
 */
#define ALIGN8BIT

/**
 * Attribute to apply to an enum declaration to force it into the smallest
 * type size required to represent all values.
 */
#define SHORTENUM

#endif /* defined(DOXYGEN) */

#if defined(__ICCARM__)

/* Intrinsic functions provided by IAR to enable and disable IRQ interrupts. */
#define sei()               __enable_irq()
#define cli()               __disable_irq()

#define sef()               __enable_fiq()
#define clf()               __disable_fiq()

/* Intrinsic functions provided by IAR to get the current program status word */
#define GET_CPSR(irq)       (irq = __get_CPSR())

/* Intrinsic functions provided by IAR to set the current program status word */
#define SET_CPSR(irq)       __set_CPSR(irq)

#define nop()               __no_operation()

#define EEGET(var, addr)    nop()

#define PROGMEM

/* program memory space abstraction */
#define FLASH_EXTERN(x) extern const x
#define FLASH_DECLARE(x)  const x
#define FUNC_PTR void *
#define FLASH_STRING(x) ((const char *)(x))
#define FLASH_STRING_T char const *
#define PGM_READ_BYTE(x) *(x)
#define PGM_READ_WORD(x) *(x)
#define PGM_READ_BLOCK(dst, src, len) memcpy((dst), (src), (len))
#define PGM_STRLEN(x) strlen(x)
#define PGM_STRCPY(dst, src) strcpy((dst), (src))

#define PUTS(s) { static const char c[] = s; printf(c); }
#define PRINTF(fmt, ...) { static const char c[] = fmt; printf(c, __VA_ARGS__); }

/* This is the obsolete align 8 bit macro. */
#define ALIGN8BIT /**/

uint16_t crc_ccitt_update(uint16_t crc, uint8_t data);
#define _crc_ccitt_update(crc, data)    crc_ccitt_update(crc, data)

/*
 * Extended keyword provided by IAR to set the alignment of members of
 * structure and union to 1.
 */
#define SHORTENUM           __packed

#define RAMFUNCTION         __ramfunc

#define FORCE_INLINE(type, name, ...) \
static inline type name(__VA_ARGS__)


#endif /* defined(__ICCARM__) */

#if defined(__GNUC__)

#define asm                 __asm__

#define volatile            __volatile__

#define FORCE_INLINE(type, name, ...) \
static inline type name(__VA_ARGS__) __attribute__((always_inline)); \
static inline type name(__VA_ARGS__)

/*
 * No intrinsic function is provided by GCC to enable and disable interrupt,
 * hence they are implemented in assembly language
 */
#define sei()   do                          \
{                                           \
    asm volatile ("MRS R0, CPSR");          \
    asm volatile ("BIC R0, R0, #0x80");     \
    asm volatile ("MSR CPSR_c, R0");        \
}while (0);

#define cli()   do                          \
{                                           \
    asm volatile ("MRS R0, CPSR");          \
    asm volatile ("ORR R0, R0, #0x80");     \
    asm volatile ("MSR CPSR_c, R0");        \
}while (0);

/* Gets the current program status word */
#define GET_CPSR(sreg)  asm volatile ("MRS %0, CPSR" : "=r" (sreg) :);

/* Sets the current program status word */
#define SET_CPSR(sreg)  asm volatile ("MSR CPSR_c, %0" : : "r" (sreg));

#define nop()               do { asm volatile ("nop"); } while (0)

#define EEGET(var, addr)    nop()

#define ALIGN8BIT /**/

#define PROGMEM

#define RAMFUNCTION

/*
 * Provided by GCC to set the alignment of members of structure and union to 1.
 */
#define SHORTENUM           __attribute__((packed))

/*
 * Some defines taken from the IAR part of the file. Not included here although just generic casting stuff.
 */

#define FLASH_DECLARE(x)    const x
#define FUNC_PTR            void *
#define PGM_READ_BYTE(x)    *(x)

#endif /* defined(__GNUC__) */

#define ADDR_COPY_DST_SRC_16(dst, src)  ((dst) = (src))
#define ADDR_COPY_DST_SRC_64(dst, src)  ((dst) = (src))

/*
 * @brief Converts a 2 Byte array into a 16-Bit value
 *
 * @param data Specifies the pointer to the 2 Byte array
 *
 * @return 16-Bit value
 * @ingroup apiPalApi
 */
static inline uint16_t convert_byte_array_to_16_bit(uint8_t *data)
{
    return (data[0] | ((uint16_t)data[1] << 8));
}
/*
#define convert_byte_array_to_16_bit(data) \
    ((uint16_t)(data)[0] | ((uint16_t)(data)[1] << 8))
*/

/**
 * @brief Converts a 4 Byte array into a 32-Bit value
 *
 * @param data Specifies the pointer to the 4 Byte array
 *
 * @return 32-Bit value
 * @ingroup apiPalApi
 */
static inline uint32_t convert_byte_array_to_32_bit(uint8_t *data)
{
    union
    {
        uint32_t u32;
        uint8_t u8[4];
    }long_addr;

    uint8_t index;

    for (index = 0; index < 4; index++)
    {
        long_addr.u8[index] = *data++;
    }

    return long_addr.u32;
}


/**
 * @brief Converts a 8 Byte array into a 64-Bit value
 *
 * @param data Specifies the pointer to the 8 Byte array
 *
 * @return 64-Bit value
 * @ingroup apiPalApi
 */
static inline uint64_t convert_byte_array_to_64_bit(uint8_t *data)
{
    union
    {
        uint64_t u64;
        uint8_t u8[8];
    }long_addr;

    uint8_t index;

    for (index = 0; index < 8; index++)
    {
        long_addr.u8[index] = *data++;
    }

    return long_addr.u64;
}


/**
 * @brief Converts a 16-Bit value into  a 2 Byte array
 *
 * @param[in] value 16-Bit value
 * @param[out] data Pointer to the 2 Byte array to be updated with 16-Bit value
 * @ingroup apiPalApi
 */
static inline void convert_16_bit_to_byte_array(uint16_t value, uint8_t *data)
{
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
}
/*
#define convert_16_bit_to_byte_array(value, data) \
    do {                                          \
        (data)[0] = (value) & 0xFF;               \
        (data)[1] = ((value) >> 8) & 0xFF;        \
    } while(0)
*/

/**
 * @brief Converts a 32-Bit value into  a 4 Byte array
 *
 * @param[in] value 32-Bit value
 * @param[out] data Pointer to the 4 Byte array to be updated with 32-Bit value
 * @ingroup apiPalApi
 */
static inline void convert_32_bit_to_byte_array(uint32_t value, uint8_t *data)
{
    uint8_t index = 0;

    while (index < 4)
    {
        data[index++] = value & 0xFF;
        value = value >> 8;
    }
}


/**
 * @brief Converts a 64-Bit value into  a 8 Byte array
 *
 * @param[in] value 64-Bit value
 * @param[out] data Pointer to the 8 Byte array to be updated with 64-Bit value
 * @ingroup apiPalApi
 */
static inline void convert_64_bit_to_byte_array(uint64_t value, uint8_t *data)
{
    uint8_t index = 0;

    while (index < 8)
    {
        data[index++] = value & 0xFF;
        value = value >> 8;
    }
}

/* === Types ================================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef cplusplus
} /* extern "C" */
#endif

#endif /* ARMTYPES_H */
/* EOF */

