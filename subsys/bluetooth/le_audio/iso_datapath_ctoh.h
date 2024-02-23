/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _ISO_DATAPATH_CTOH_H
#define _ISO_DATAPATH_CTOH_H

/**
 * @file
 * @brief BLE isochronous data path in Controller --> Host direction
 */

#include <zephyr/kernel.h>
#include "sdu_queue.h"

/**
 * @brief Create an instance of isochronous datapath in controller --> host direction
 *
 * @param stream_lid The stream local ID on the controller to bind with
 * @param sdu_queue The SDU queue to send SDUs to
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
struct iso_datapath_ctoh *iso_datapath_ctoh_create(uint8_t stream_lid, struct sdu_queue *sdu_queue);

/**
 * @brief Notify ISO datapath that an SDU has been processed
 *
 * @param datapath Pointer to datapath associated with the processed SDU
 * @param timestamp Timestamp of the SDU
 * @param sdu_seq Sequence number of SDU
 */
void iso_datapath_ctoh_notify_sdu_done(void *datapath, uint32_t timestamp, uint16_t sdu_seq);

/**
 * @brief Delete an instance of isochronous datapath in controller --> host direction
 *
 * @param datapath The datapath instance to delete
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int iso_datapath_ctoh_delete(struct iso_datapath_ctoh *datapath);

#endif /* _ISO_DATAPATH_CTOH_H */
