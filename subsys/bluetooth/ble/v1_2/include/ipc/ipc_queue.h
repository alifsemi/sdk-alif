/**
 ****************************************************************************************
 *
 * @file ipc_queue.h
 *
 * @brief Lock-free single-producer single-consumer queue that can be shared between
 *        multiple cores.
 *
 ****************************************************************************************
 */

#ifndef _IPC_QUEUE_H_
#define _IPC_QUEUE_H_

#include "rom_build_cfg.h"

#include <stddef.h>
#include <stdint.h>
#include "compiler.h"
#include "arch.h"

/**
 * Error codes.
 */
typedef enum {
    /** No error */
    IPC_QUEUE_ERR_NONE = 0,
    /** Invalid size */
    IPC_QUEUE_ERR_SIZE,
    /** Queue is empty */
    IPC_QUEUE_ERR_EMPTY,
    /** Queue is full */
    IPC_QUEUE_ERR_FULL
} ipc_queue_err_t;

/**
 * Lock-free single-producer single-consumer queue.
 *
 * The queue is assumed to be located in uncacheable memory.
 */
typedef struct __ALIGN(4) {
    /** Size of an item, in bytes */
    __ALIGN(4) uint16_t item_size;
    /** Maximum number of items */
    __ALIGN(4) uint16_t item_count;
    /** Read index */
    __ALIGN(4) uint16_t read_ind;
    /** Write index */
    __ALIGN(4) uint16_t write_ind;

    /** Buffer containing the queue's items */
    __ALIGN(4) uint8_t buf[__ARRAY_EMPTY];
} ipc_queue_t;
_Static_assert(sizeof(ipc_queue_t) == 16, "ipc_queue_t size is incorrect");

/**
 * @brief Initializes a queue.
 * @param[in, out] queue The queue.
 * @param[in] item_size Size of an item, in bytes.
 * @param[in] item_count Maximum number of items in the queue. Maximum supported value is 32767.
*/
void ipc_queue_init(ipc_queue_t* queue, uint16_t item_size, uint16_t item_count);

/**
 * @brief Gets the write pointer of the queue.
 * @param[in] queue The queue.
 * @param[out] buf The write pointer of the queue.
 * @return 0 on success, an error code otherwise (@see #ipc_queue_err_t).
 *
 * No item is added to the queue, once data has been written to the item,
 * #ipc_queue_commit can be called to insert the item into the queue.
 */
uint8_t ipc_queue_alloc(ipc_queue_t* queue, void** buf);

/**
 * @brief Commits a write to the queue.
 * @param[in] queue The queue.
 *
 * The queue is assumed not to be full.
 */
void ipc_queue_commit(ipc_queue_t* queue);

/**
 * @brief Adds an item to a queue.
 * @param[in, out] queue The queue.
 * @param[in] data The data to copy into the queue.
 * @param[in] size The size of the data, in bytes.
 * @return 0 on success, an error code otherwise (@see #ipc_queue_err_t).
 *
 * Calling this routine is equivalent to:
 *  1) Calling #ipc_queue_alloc to get the next available item.
 *  2) Writing the data to the buffer.
 *  3) Calling #ipc_queue_commit to commit the write.
 */
uint8_t ipc_queue_write(ipc_queue_t* queue, void const* data, uint16_t size);

/**
 * @brief Gets the read pointer of the queue.
 * @param[in] queue The queue.
 * @param[out] buf The read pointer of the queue.
 * @return 0 on success, an error code otherwise (@see #ipc_queue_err_t).
 *
 * The queue is not altered. Once the item has been processed, it can be removed
 * using #ipc_queue_pop.
 */
uint8_t ipc_queue_peek(ipc_queue_t* queue, void** buf);

/**
 * @brief Removes the oldest item of the queue.
 * @param[in, out] queue The queue.
 *
 * The queue is assumed not to be empty.
 */
void ipc_queue_pop(ipc_queue_t* queue);

/**
 * @brief Retrieves an item from a queue.
 * @param[in, out] queue The queue.
 * @param[out] data The buffer where to copy the item.
 * @return 0 on success, an error code otherwise (@see #ipc_queue_err_t).
 *
 * Calling this routine is equivalent to:
 *  1) Calling #ipc_queue_peek to get the next available item.
 *  2) Copying the data to the buffer.
 *  3) Calling #ipc_queue_commit to commit the write.
 */
uint8_t ipc_queue_read(ipc_queue_t* queue, void* data);

/**
 * The queue index must wrap back to zero at a multiple of the item_count of the queue, for the
 * current item index to be calculated correctly after wrap-around.
 * 
 * However wrapping the queue index back to zero when it reaches the item_count does not allow us
 * to distinguish between a full and empty condition of the queue using only the read and write
 * indices (full and empty would both satisfy the condition read_idx == write_idx). Adding an extra
 * variable to track if the queue is full would mean the queue can no longer be lock free (since 
 * both the producer and consumer would have to write to the "full" variable).
 * 
 * If we wrap the queue index at a multiple of the item count which is greater than 1, then we can
 * maintain a lock-free queue, and calculate whether it is full by checking if the write index is
 * exactly item_count items ahead of the read_index.
 */
#define QUEUE_INDEX_MODULUS(item_count)     (item_count * 2)

/**
 * @brief Indicates whether the queue is full.
 * @param[in] queue The queue.
 * @return true if the queue is full, false otherwise.
 */
__STATIC_FORCEINLINE bool ipc_queue_is_full(ipc_queue_t const* queue)
{
    ASSERT_ERR(queue != NULL);

    /* The queue is full if the write index is exactly item_count ahead of the read index */
    return ((queue->read_ind + queue->item_count) % 
             QUEUE_INDEX_MODULUS(queue->item_count)) == queue->write_ind;
}

/**
 * @brief Indicates whether the queue is empty.
 * @param[in] queue The queue.
 * @return true if the queue is empty, false otherwise.
 */
__STATIC_FORCEINLINE bool ipc_queue_is_empty(ipc_queue_t const* queue)
{
    ASSERT_ERR(queue != NULL);
    return queue->write_ind == queue->read_ind;
}

/**
 * @brief Flushes the content of the queue.
 * @param[in, out] queue The queue.
 */
__STATIC_FORCEINLINE void ipc_queue_flush(ipc_queue_t* queue)
{
    ASSERT_ERR(queue != NULL);
    queue->read_ind = queue->write_ind;
}

#endif // _IPC_QUEUE_H_
