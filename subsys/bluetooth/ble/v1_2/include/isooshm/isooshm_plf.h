/**
 ****************************************************************************************
 *
 * @file isooshm_plf.h
 *
 * @brief Platform layer for ISO over shared memory, HL side.
 *
 ****************************************************************************************
 */

#ifndef _ISOOSHM_PLF_H_
#define _ISOOSHM_PLF_H_

#include "rom_build_cfg.h"

#include <stddef.h>
#include <stdint.h>
#include "isooshm.h"
#include "co_list.h"

typedef struct isooshm_plf_transfer isooshm_plf_transfer_t;

/**
 * @brief Callback invoked when a data transfer is complete.
 * @param[in] transfer The data transfer.
 *
 * This callback is invoked from ISR.
 */
typedef void (*isooshm_plf_transfer_cmp_cb_t)(isooshm_plf_transfer_t* transfer);

/**
 * @brief Callback invoked when the ISO signal is triggered.
 *
 * This callback is invoked from ISR.
 */
typedef void (*isooshm_plf_signal_cb_t)(void);

/**
 * A data transfer.
 */
typedef struct isooshm_plf_transfer {
    /** List header */
    co_list_hdr_t list_hdr;
    /** Source address, must be 4-byte aligned */
    void* src;
    /** Destination address, must be 4-byte aligned */
    void* dst;
    /** Size, in bytes, must be a multiple of 4 */
    size_t size;
    /** Optional callback to invoke when transfer is complete */
    isooshm_plf_transfer_cmp_cb_t cb;
} isooshm_plf_transfer_t;

/**
 * @brief Initializes the platform layer for ISO over shared memory.
 * @param[in] signal_cb Callback invoked when the ISO signal is triggered.
 * @return The ISO over shared memory global descriptor.
 *
 * By default, the ISO signal is masked.
 */
isooshm_t const* isooshm_plf_init(isooshm_plf_signal_cb_t signal_cb);

/**
 * @brief Copies data from source to destination.
 * @param[in, out] transfer The data transfer.
 * @return 0 on success, an error code otherwise (@see #co_error).
 *
 * The data transfer is performed using DMA. The caller is responsible for any
 * required cache maintenance operation.
 */
uint8_t isooshm_plf_copy(isooshm_plf_transfer_t* transfer);

/**
 * @brief Aborts a data transfer.
 * @param[in, out] transfer The data transfer.
 *
 * If the data transfer is not ongoing nor pending, this routine has no effect.
*/
void isooshm_plf_abort_copy(isooshm_plf_transfer_t* transfer);

/**
 * @brief Converts an address in controller's space to an address in host's space.
 * @param[in] addr The address in controller's address space. Must not be NULL.
 * @return The corresponding address in host's address space.
 */
void* isooshm_plf_to_host_addr(void* addr);

/**
 * @brief The host and controller timer are synchronized regularly
 *        The timer value on host is converted/corrected to
 *        retrieve the current controller time. The value is only
 *        valid when one or more ISO datapath is running.
 *
 * @return Returns local controller timestamp(in microseconds)
 */
uint32_t isooshm_plf_get_local_time(void);

/**
 * @brief  This handler should be called whenever the host
 *         synchronization timer captures a timestamp on ISO signal
 */
void isooshm_plf_timer_capture_evt_handler(void);

/**
 * @brief  This handler should be called whenever the host
 *         synchronization timer overflows
 */
void isooshm_plf_timer_overflow_evt_handler(void);
#endif // _ISOOSHM_PLF_H_
