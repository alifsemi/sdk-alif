/* Copyright (C) 2022-2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

 /******************************************************************************
 * @file    aipl_arm_mve.h
 * @brief   AIPL wrapper over arm_mve.h with redefinition of some Helium intrinsics
 *          to work around a GCC 12.2.0 compiler bug
 *
******************************************************************************/

#ifndef AIPL_ARM_MVE_H
#define AIPL_ARM_MVE_H

#if defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE
#include <arm_mve.h>

#define GCC_VERSION (__GNUC__ * 10000\
                     + __GNUC_MINOR__ * 100\
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION == 120200

#undef __arm_vsriq
#define __arm_vsriq(p0,p1,p2) ({ __typeof(p0) __p0 = (p0); \
    __typeof(p1) __p1 = (p1); \
    _Generic( (int (*)[__ARM_mve_typeid(__p0)][__ARM_mve_typeid(__p1)])0, \
    int (*)[__ARM_mve_type_int8x16_t][__ARM_mve_type_int8x16_t]: __arm_patched_vsriq_n_s8 (__ARM_mve_coerce(__p0, int8x16_t), __ARM_mve_coerce(__p1, int8x16_t), p2), \
    int (*)[__ARM_mve_type_int16x8_t][__ARM_mve_type_int16x8_t]: __arm_patched_vsriq_n_s16 (__ARM_mve_coerce(__p0, int16x8_t), __ARM_mve_coerce(__p1, int16x8_t), p2), \
    int (*)[__ARM_mve_type_int32x4_t][__ARM_mve_type_int32x4_t]: __arm_patched_vsriq_n_s32 (__ARM_mve_coerce(__p0, int32x4_t), __ARM_mve_coerce(__p1, int32x4_t), p2), \
    int (*)[__ARM_mve_type_uint8x16_t][__ARM_mve_type_uint8x16_t]: __arm_patched_vsriq_n_u8 (__ARM_mve_coerce(__p0, uint8x16_t), __ARM_mve_coerce(__p1, uint8x16_t), p2), \
    int (*)[__ARM_mve_type_uint16x8_t][__ARM_mve_type_uint16x8_t]: __arm_patched_vsriq_n_u16 (__ARM_mve_coerce(__p0, uint16x8_t), __ARM_mve_coerce(__p1, uint16x8_t), p2), \
    int (*)[__ARM_mve_type_uint32x4_t][__ARM_mve_type_uint32x4_t]: __arm_patched_vsriq_n_u32 (__ARM_mve_coerce(__p0, uint32x4_t), __ARM_mve_coerce(__p1, uint32x4_t), p2));})

__extension__ extern __inline int8x16_t
__attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
__arm_patched_vsriq_n_s8 (int8x16_t __a, int8x16_t __b, const int __imm)
{
    int8x16_t __res = __a;
    __asm("vsri.8 %q[res], %q[b], %[imm]"
          : [res] "+w" (__res)
          : [b] "w" (__b), [imm] "n" (__imm)
    );
    return __res;
}

__extension__ extern __inline int16x8_t
__attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
__arm_patched_vsriq_n_s16 (int16x8_t __a, int16x8_t __b, const int __imm)
{
    int16x8_t __res = __a;
    __asm("vsri.16 %q[res], %q[b], %[imm]"
          : [res] "+w" (__res)
          : [b] "w" (__b), [imm] "n" (__imm)
    );
    return __res;
}

__extension__ extern __inline int32x4_t
__attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
__arm_patched_vsriq_n_s32 (int32x4_t __a, int32x4_t __b, const int __imm)
{
    int32x4_t __res = __a;
    __asm("vsri.32 %q[res], %q[b], %[imm]"
          : [res] "+w" (__res)
          : [b] "w" (__b), [imm] "n" (__imm)
    );
    return __res;
}

__extension__ extern __inline uint8x16_t
__attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
__arm_patched_vsriq_n_u8 (uint8x16_t __a, uint8x16_t __b, const int __imm)
{
    uint8x16_t __res = __a;
    __asm("vsri.8 %q[res], %q[b], %[imm]"
          : [res] "+w" (__res)
          : [b] "w" (__b), [imm] "n" (__imm)
    );
    return __res;
}

__extension__ extern __inline uint16x8_t
__attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
__arm_patched_vsriq_n_u16 (uint16x8_t __a, uint16x8_t __b, const int __imm)
{
    uint16x8_t __res = __a;
    __asm("vsri.16 %q[res], %q[b], %[imm]"
          : [res] "+w" (__res)
          : [b] "w" (__b), [imm] "n" (__imm)
    );
    return __res;
}

__extension__ extern __inline uint32x4_t
__attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
__arm_patched_vsriq_n_u32 (uint32x4_t __a, uint32x4_t __b, const int __imm)
{
    uint32x4_t __res = __a;
    __asm("vsri.32 %q[res], %q[b], %[imm]"
          : [res] "+w" (__res)
          : [b] "w" (__b), [imm] "n" (__imm)
    );
    return __res;
}

#endif /* GCC_VERSION == 120200 */

#endif /* defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE */

#endif /* AIPL_ARM_MVE_H */
