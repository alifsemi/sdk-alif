/**
 ****************************************************************************************
 *
 * @file isooshm.h
 *
 * @brief Defines an API for exchanging ISO data over shared memory.
 *
 ****************************************************************************************
 */

#ifndef _ISOOSHM_H_
#define _ISOOSHM_H_

#include "rom_build_cfg.h"

#include <stdint.h>
#include "rwip_config.h"
#include "compiler.h"
#include "ipc_queue.h"
#include "ipc_spinlock.h"
#include "co_bt_defines.h"

/** Magic number used to identify the global descriptor */
#define ISOOSHM_MAGIC 0x4D4F5349u
/** Size, in bytes, of the header added to the SDUs when exchanged over shared memory */
#define ISOOSHM_SDU_HDR_LEN (sizeof(isooshm_sdu_buf_t))

/**
 * Note on alignment:
 * 
 * Any data accessed by both ExtSys0 and ExtSys1 processors must be aligned to 32 bits. This 
 * includes structure members. Static assertions are used to ensure that the data structures sizes
 * are identical on both systems.
 */

/**
 * Status of a received SDU.
 */
typedef enum {
    /** SDU content is valid */
    ISOOSHM_SDU_STATUS_VALID,
    /** SDU content might contain errors */
    ISOOSHM_SDU_STATUS_ERROR,
    /** SDU has been lost */
    ISOOSHM_SDU_STATUS_LOST
} isooshm_sdu_status_t;

/**
 * An item of the garbage collector's list.
 */
typedef struct __ALIGN(4) isooshm_gc_item {
    /** Next item in the list */
    __ALIGN(4) struct isooshm_gc_item* next;
    /** Link ID associated with the item */
    __ALIGN(4) uint8_t link_id;
    /** Data path direction associated with the item */
    __ALIGN(4) uint8_t dir;
    /** Reserved for future use */
    __ALIGN(4) uint32_t _reserved;
    /** The queue */
    __ALIGN(4) ipc_queue_t queue;
} isooshm_gc_item_t;
_Static_assert(sizeof(isooshm_gc_item_t) == 32, "isooshm_gc_item_t size is incorrect");

/**
 * List containing the SDU queues that are no more needed and must be
 * destroyed. The list is divided in two sub-lists:
 *  - The pending list, containing the SDU queues that cannot be destroyed for
 *    the moment because they might still be used by the host.
 *  - The released list, containing the SDU queues that are not used anymore
 *    by the host and can be safely destroyed.
 *
 * When a data path is stopped, the associated SDU queue is added first to the
 * pending list and the host is notified. The host is then responsible for
 * stopping any current use of the SDU queue and then moving the SDU queue to
 * the released list to indicate the queue can now be safely destroyed.
 *
 * The garbage collector regularly checks the released list and destroy
 * any queue contained in that list.
 */
typedef struct __ALIGN(4) {
    /** Lock guarding the access to the list */
    __ALIGN(4) ipc_spinlock_t lock;
    /** Head of pending list */
    __ALIGN(4) isooshm_gc_item_t* pending_head;
    /** Head of the released list */
    __ALIGN(4) isooshm_gc_item_t* released_head;
} isooshm_gc_list_t;
_Static_assert(sizeof(isooshm_gc_list_t) == 20, "isooshm_gc_list_t size is incorrect");

/**
 * Timing information of the last outgoing SDU processed by the controller.
 */
typedef struct __ALIGN(4) {
    /** Lock guarding access to the other fields */
    __ALIGN(4) ipc_spinlock_t lock;
    /** Timestamp, in µs, of the SDU Synchronization Reference of the last SDU */
    __ALIGN(4) uint32_t sdu_ref;
    /** Timestamp, in µs, of the Reference Anchor point of the last SDU */
    __ALIGN(4) uint32_t sdu_anchor;
    /** SDU sequence number of the last SDU */
    __ALIGN(4) uint16_t seq_num;
    /** Indicates if the value of the other fields are valid */
    __ALIGN(4) bool valid;
} isooshm_sdu_tx_sync_t;
_Static_assert(sizeof(isooshm_sdu_tx_sync_t) == 28, "isooshm_sdu_tx_sync_t size is incorrect");

/**
 * Structure holding peer drift values for given CIG/BIG group
 */
typedef struct __ALIGN(4) {
    __ALIGN(4) uint32_t drift;         //Drift calculation for the group
    __ALIGN(4) uint8_t iso_grp_id;     //CIG/BIG group ID assigned on the host
} isooshm_peer_drift_t;
_Static_assert(sizeof(isooshm_peer_drift_t) == 8, "isooshm_peer_drift_t size is incorrect");

/**
 * Global descriptor for ISO data over shared memory data paths.
 *
 * This descriptor is written at initialization by the controller at a fixed
 * position in shared memory. Its content doesn't change after initilization and
 * can be accessed by the host once the magic number has been written.
 */
typedef struct __ALIGN(4) {
    /** Magic number */
    __ALIGN(4) uint32_t magic;
    /** Maximum number of ISO streams */
    __ALIGN(4) uint8_t max_stream_count;
    /** Maximum number of ISO groups */
    __ALIGN(4) uint8_t max_group_count;
    /** ISO Timestamp of controller sampled just before GPIO0 is triggered */
    __ALIGN(4) volatile uint32_t ctrl_iso_ts;
    /** Queue containing the events */
    __ALIGN(4) ipc_queue_t* evt_queue;
    /** Queues used to exchange SDUs, indexed by link ID then data path direction */
    __ALIGN(4) ipc_queue_t* (*sdu_queues)[DATA_PATH_DIR_MAX];
    /** List containing the old SDU queues, waiting to be garbage collected */
    __ALIGN(4) isooshm_gc_list_t* queue_gc_list;
    /** Timing information of last outgoing SDUs, indexed by link ID */
    __ALIGN(4) isooshm_sdu_tx_sync_t* sdu_tx_sync;
    /** List of drift calculations for each ISO group*/
    __ALIGN(4) isooshm_peer_drift_t* peer_drifts;
} isooshm_t;
_Static_assert(sizeof(isooshm_t) == 36, "isooshm_t size is incorrect");

/**
 * SDU buffer.
 * 
 * Note: this structure does *not* need all members to be 4-byte aligned, since it will be copied
 * from ExtSys0 to ExtSys1 memory before any members are accessed by ExtSys1.
 */
typedef struct __PACKED __ALIGN(4) {
    /* SDU sequence number */
    uint16_t seq_num;
    /* Size of the SDU, in bytes */
    uint16_t sdu_len;
    /* Timestamp, in µs */
    uint32_t timestamp;

    union {
        /* Rx only: indicate the status of the received SDU */
        uint8_t status;
        /* Tx only: indicate if a timestamp is provided */
        bool has_timestamp;
    };

    /* SDU data */
    __ALIGN(4)
    uint8_t data[__ARRAY_EMPTY];
} isooshm_sdu_buf_t;

/**
 * Entry of the event queue, defining the data path instance on which an event occurred.
 * 
 * Note: this structure does *not* need all members to be 4-byte aligned, since it will be copied
 * from ExtSys0 to ExtSys1 memory before any members are accessed by ExtSys1.
 */
typedef struct __PACKED {
    /** Link ID */
    uint8_t link_id;
    /** Data path direction */
    uint8_t dir;
} isooshm_evt_t;

#endif // _ISOOSHM_H_
