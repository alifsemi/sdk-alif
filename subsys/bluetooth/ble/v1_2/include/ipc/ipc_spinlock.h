/**
 ****************************************************************************************
 *
 * @file ipc_spinlock.h
 *
 * @brief Spinlock that can be shared between two cores.
 *
 * This spinlock implementation uses the Peterson's algorithm hence doesn't
 * require any special atomic instructions like compare-and-swap or
 * test-and-set.
 *
 ****************************************************************************************
 */

#ifndef _IPC_SPINLOCK_H_
#define _IPC_SPINLOCK_H_

#include "rom_build_cfg.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * A spinlock.
 *
 * The spinlock is assumed to be located in uncacheable memory.
 */
typedef struct __ALIGN(4) {
    /** Flags indicating if a core wants to lock the spinlock */
    __ALIGN(4) uint32_t volatile flag[2];
    /** Indicate which core has the priority for locking the spinlock */
    __ALIGN(4) uint8_t volatile turn;
} ipc_spinlock_t;
_Static_assert(sizeof(ipc_spinlock_t) == 12, "ipc_spinlock_t size is incorrect");

/**
 * @brief Initializes a spinlock.
 * @param[in, out] lock The spinlock.
 */
void ipc_spinlock_init(ipc_spinlock_t* lock);

/**
 * @brief Locks a spinlock.
 * @param[in, out] lock The spinlock.
 */
void ipc_spinlock_lock(ipc_spinlock_t* lock);

/**
 * @brief Unlocks a spinlock.
 * @param[in, out] lock The spinlock.
*/
void ipc_spinlock_unlock(ipc_spinlock_t* lock);

#endif // _IPC_SPINLOCK_H_
