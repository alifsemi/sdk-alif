/**
 ****************************************************************************************
 *
 * @file arch.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * architecture dependent and used by some of the API inline functions.
 *
 ****************************************************************************************
 */

#ifndef __ALIF_ARCH_H__
#define __ALIF_ARCH_H__

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include <zephyr/toolchain.h>

/*
 * CPU Endianness
 ****************************************************************************************
 */

/* ARM is little endian */
#define CPU_LE 1

/*
 * DEFINES used by BLE API
 ****************************************************************************************
 */

#ifndef __PACKED
#define __PACKED __packed
#endif

/* Align to n-byte boundary */
#define __ALIGN(n) __aligned(n)

#ifndef __ARRAY_EMPTY
#define __ARRAY_EMPTY
#endif

#ifndef __STATIC
#define __STATIC static
#endif

#ifndef __STATIC_FORCEINLINE
/* NOTE: This varies based on compiler! */
#define __STATIC_FORCEINLINE static inline __attribute__((always_inline))
#endif

#define PLF_EM_FETCH_TIME_US 40

#define PLF_EM_UPDATE_TIME_US 14

/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */

/*
 * ASSERTION empty defines for some APIs using these
 ****************************************************************************************
 */
/* Assertions showing a critical error that could require a full system reset */
#define ASSERT_ERR(cond)

/* Assertions showing a critical error that could require a full system reset */
#define ASSERT_INFO(cond, param0, param1)

#endif /* __ALIF_ARCH_H__ */
