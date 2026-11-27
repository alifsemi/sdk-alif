/**
 ****************************************************************************************
 *
 * @file co_math.h
 *
 * @brief Common optimized math functions
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef CO_MATH_H_
#define CO_MATH_H_

#include "rom_build_cfg.h"

/**
 *****************************************************************************************
 * @defgroup CO_MATH Math functions
 * @ingroup COMMON
 * @brief  Optimized math functions and other computations.
 *
 * @{
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>        // standard integer definitions
#include <stdbool.h>       // boolean definitions
#include <stdlib.h>        // standard library
#include "compiler.h"      // for __INLINE
#include "arch.h"          // for ASSERT_ERR

/*
 * MACROS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Return value with one bit set.
 *
 * @param[in] pos Position of the bit to set.
 *
 * @return Value with one bit set.  There is no return type since this is a macro and this
 * will be resolved by the compiler upon assignment to an l-value.
 ****************************************************************************************
 */
#define CO_BIT(pos) (1UL<<(pos))


/**
 ****************************************************************************************
 * @brief Create a mask composed of a given number of trailing 1s followed by a number or trailing 0s
 * Example CO_MASK(5,0) = b11111000;
 *
 * @param[in]   nb_bits     Number of bits to 1 in the mask
 * @param[in]   lsb         Less significant bit of the mask
 * @return      result
 ****************************************************************************************
 */
#define CO_MASK(nb_bits, lsb) (((1UL << (nb_bits)) - 1UL) << (lsb))

/**
 ****************************************************************************************
 * @brief Return value bit into a bit field.
 *
 * @param[in] bf  Bit Field
 * @param[in] pos Position of the bit
 *
 * @return value of a bit into a bit field
 ****************************************************************************************
 */
#define CO_BIT_GET(bf, pos) (((((uint8_t*)bf)[((pos) >> 3)])>>((pos) & 0x7)) & 0x1)

/**
 ****************************************************************************************
 * @brief Update value bit into a bit field.
 *
 * @param[in] bf  Bit Field
 * @param[in] pos Position of the bit
 * @param[in] val New value of the bit (0 or 1)
 ****************************************************************************************
 */
#define CO_BIT_SET(bf, pos, val) (((uint8_t*)bf)[((pos) >> 3)]) = ((((uint8_t*)bf)[((pos) >> 3)]) & ~CO_BIT(((pos) & 0x7))) \
                                                                | (((val) & 0x1) << ((pos) & 0x7))


/**
 ****************************************************************************************
 * @brief Align val on the multiple of size equal or nearest higher.
 * @param[in] val  Value to align.
 * @param[in] size of memory alignment (1, 2 or 4)
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN_HI(val, size) ((((uint32_t) val)+((size)- 1))&~((size)- 1))

/**
 ****************************************************************************************
 * @brief Align val on the multiple of 4 equal or nearest higher.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN4_HI(val) (((val)+3u)&~3u)


/**
 ****************************************************************************************
 * @brief Align val on the multiple of 4 equal or nearest lower.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN4_LO(val) ((val)&~3u)

/**
 ****************************************************************************************
 * @brief Align val on the multiple of 2 equal or nearest higher.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN2_HI(val) (((val)+1u)&~1u)


/**
 ****************************************************************************************
 * @brief Align val on the multiple of 2 equal or nearest lower.
 * @param[in] val Value to align.
 * @return Value aligned.
 ****************************************************************************************
 */
#define CO_ALIGN2_LO(val) ((val)&~1u)

/**
 ****************************************************************************************
 * Perform a division and ceil up the result
 *
 * @param[in] val Value to divide
 * @param[in] div Divide value
 * @return ceil(val/div)
 ****************************************************************************************
 */
#define CO_DIVIDE_CEIL(val, div) (((val) + ((div) - 1))/ (div))

/**
 ****************************************************************************************
 * Perform a division and round the result
 *
 * @param[in] val Value to divide
 * @param[in] div Divide value
 * @return round(val/div)
 ****************************************************************************************
 */
#define CO_DIVIDE_ROUND(val, div) (((val) + ((div) >> 1))/ (div))

/**
 ****************************************************************************************
 * Perform a modulo operation
 *
 * @param[in] val    Dividend
 * @param[in] div    Divisor
 * @return  val/div)
 ****************************************************************************************
 */
#if(0) // unused code
#define CO_MOD(val, divisor) ((val) % (divisor))
#endif // unused code
__STATIC_FORCEINLINE uint32_t co_mod(uint32_t val, uint32_t divisor)
{
   ASSERT_ERR(divisor);
   return ((val) % (divisor));
}
#define CO_MOD(val, divisor) co_mod(val, divisor)


/*
 * FUNCTION DEFINTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Count leading zeros.
 * @param[in] val Value to count the number of leading zeros on.
 * @return Number of leading zeros when value is written as 32 bits.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_clz(uint32_t val)
{
    #if defined(__arm__)
    return __builtin_clz(val);
    #elif defined(__GNUC__)
    if (val == 0)
    {
        return 32;
    }
    return __builtin_clz(val);
    #else
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (val & CO_BIT(31 - i))
        {
            break;
        }
    }
    return i;
    #endif // defined(__arm__)
}

/**
 ****************************************************************************************
 * @brief Count trailing zeros.
 * @param[in] val Value to count the number of trailing zeros on.
 * @return Number of trailing zeros when value is written as 32 bits.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_ctz(uint32_t val)
{
    #if defined(__arm__)
    return __builtin_ctz(val);
    #elif defined(__GNUC__)
    if (val == 0)
    {
        return 32;
    }
    return __builtin_ctz(val);
    #else
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (val & CO_BIT(i))
        {
            break;
        }
    }
    return i;
    #endif // defined(__arm__)
}
/**
 ****************************************************************************************
 * @brief Function to initialize the random seed.
 * @param[in] seed The seed number to use to generate the random sequence.
 ****************************************************************************************
 */
void co_random_init(uint32_t seed);
/**
 ****************************************************************************************
 * @brief Function to get an 8 bit random number.
 * @return Random byte value.
 ****************************************************************************************
 */
uint8_t co_rand_byte(void);

/**
 ****************************************************************************************
 * @brief Function to get an 16 bit random number.
 * @return Random half word value.
 ****************************************************************************************
 */
uint16_t co_rand_hword(void);

/**
 ****************************************************************************************
 * @brief Function to get an 32 bit random number.
 * @return Random word value.
 ****************************************************************************************
 */
uint32_t co_rand_word(void);

/**
 ****************************************************************************************
 * @brief Function to return the smallest of 2 unsigned 32 bits words.
 * @return The smallest value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the smallest of 2 signed 32 bits words.
 * @return The smallest value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE int32_t co_min_s(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the greatest of 2 unsigned 32 bits words.
 * @return The greatest value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the greatest of 2 signed 32 bits words.
 * @return The greatest value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE int32_t co_max_s(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Function to return the absolute value of a signed integer.
 * @return The absolute value.
 ****************************************************************************************
 */
__STATIC_FORCEINLINE int co_abs(int val)
{
    return (val < 0) ? (0 - val) : val;
}

/**
 ****************************************************************************************
 * @brief Function to return the factorial count of a value.
 * @return The factorial count (n! = n x (n-1) x (n-2) x (n-3) x ... x 1).
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint8_t co_fct(uint8_t n)
{
    // Use static reference table as factorial of only small values <= 4 currently required
    const uint8_t fct_tbl[] = {1,1,2,(3*2),(4*3*2)};
    ASSERT_ERR(n < sizeof(fct_tbl));
    return fct_tbl[n];
}

/**
 ****************************************************************************************
 * @brief Function to return the greatest common divisor.
 * The GCD of two numbers is the largest number that divides both of them without leaving a remainder.
 * @return The gcd gcd(a,b) = gcd(b,a mod b).
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_gcd(uint32_t a, uint32_t b)
{
    while (b != 0)
    {
        uint32_t temp = b;
        // 'b' is updated with the remainder of 'a' divided by 'b'.
        b = a % b;
        // 'a' is updated with the value of 'b' before the above operation.
        a = temp;
    }
    // When 'b' becomes 0, 'a' is the GCD of the initial 'a' and 'b'.
    return a;
}

/**
 ****************************************************************************************
 * @brief Function to return the lowest common multiple of two numbers.
 * The LCM of two integers 'a' and 'b' is the smallest positive integer that is divisible by both 'a' and 'b'.
 * @return The lowest common multiple, LCM(a, b) = (a * b) / GCD(a, b).
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint32_t co_lcm(uint32_t a, uint32_t b)
{
    // Calculate 'a' divided by the GCD of 'a' and 'b', the result is then multiplied by 'b' to get the LCM.
    uint32_t result = CO_DIVIDE_CEIL(a, co_gcd(a,b)) * b;
    return result;
}

/// @} CO_MATH


#endif // CO_MATH_H_
