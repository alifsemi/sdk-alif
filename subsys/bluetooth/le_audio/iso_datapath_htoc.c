/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "gapi_isooshm.h"
#include "iso_datapath_htoc.h"
#include "presentation_compensation.h"

LOG_MODULE_REGISTER(iso_datapath_htoc, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct iso_datapath_htoc {
	gapi_isooshm_dp_t dp;
	struct sdu_queue *sdu_queue;
	gapi_isooshm_sdu_buf_t *current_sdu;
	uint32_t sdu_underrun_count;
	uint32_t sdu_sent_count;
	struct k_msgq sdu_timing_msgq;
	uint8_t *sdu_timing_msgq_buffer;
	uint16_t last_sdu_seq;
	bool timing_master_channel;
	bool awaiting_sdu;
};

struct sdu_timing_info {
	uint32_t capture_timestamp;
	uint16_t seq_num;
};

static void finish_last_sdu(struct iso_datapath_htoc *datapath)
{
	if (datapath->current_sdu != NULL) {
		k_mem_slab_free(&datapath->sdu_queue->slab, datapath->current_sdu);
		datapath->current_sdu = NULL;
	}
}

static void send_next_sdu(struct iso_datapath_htoc *datapath)
{
	int ret = k_msgq_get(&datapath->sdu_queue->msgq, (void *)&datapath->current_sdu, K_NO_WAIT);

	if (ret) {
		datapath->awaiting_sdu = true;
		datapath->sdu_underrun_count++;
		return;
	}

	uint16_t err = gapi_isooshm_dp_set_buf(&datapath->dp, datapath->current_sdu);

	if (err) {
		LOG_ERR("Failed to set next ISO buffer, err %u", err);
		datapath->awaiting_sdu = true;
	}

	datapath->sdu_sent_count++;
}

static void on_dp_transfer_complete(gapi_isooshm_dp_t *dp, gapi_isooshm_sdu_buf_t *buf)
{
	struct iso_datapath_htoc *datapath = CONTAINER_OF(dp, struct iso_datapath_htoc, dp);

	finish_last_sdu(datapath);
	send_next_sdu(datapath);
}

struct iso_datapath_htoc *iso_datapath_htoc_create(uint8_t stream_lid, struct sdu_queue *sdu_queue,
					      bool timing_master_channel)
{
	if (sdu_queue == NULL) {
		LOG_ERR("Invalid parameter");
		return NULL;
	}

	struct iso_datapath_htoc *datapath =
		(struct iso_datapath_htoc *)calloc(1, sizeof(struct iso_datapath_htoc));

	if (datapath == NULL) {
		LOG_ERR("Failed to allocate data path");
		return NULL;
	}

	datapath->sdu_queue = sdu_queue;
	datapath->timing_master_channel = timing_master_channel;

	if (timing_master_channel) {
		/* Timing queue should be slightly larger than SDU queue, as SDUs will be held for a
		 * short while on the controller before being sent
		 */
		size_t sdu_timing_queue_size = sdu_queue->item_count + 2;

		datapath->sdu_timing_msgq_buffer =
			(uint8_t *)malloc(sizeof(struct sdu_timing_info) * sdu_timing_queue_size);
		if (datapath->sdu_timing_msgq_buffer == NULL) {
			LOG_ERR("Failed to allocate timing queue");
			iso_datapath_htoc_delete(datapath);
			return NULL;
		}

		k_msgq_init(&datapath->sdu_timing_msgq, datapath->sdu_timing_msgq_buffer,
			    sizeof(struct sdu_timing_info), sdu_timing_queue_size);
		datapath->last_sdu_seq = UINT16_MAX;
	}

	uint16_t ret = gapi_isooshm_dp_init(&datapath->dp, on_dp_transfer_complete);

	if (ret != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to init datapath with err %u", ret);
		iso_datapath_htoc_delete(datapath);
		return NULL;
	}

	ret = gapi_isooshm_dp_bind(&datapath->dp, stream_lid, GAPI_DP_DIRECTION_INPUT);
	if (ret != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to bind datapath with err %u", ret);
		iso_datapath_htoc_delete(datapath);
		return NULL;
	}

	/* Flag that datapath is waiting for first SDU */
	datapath->awaiting_sdu = true;

	return datapath;
}

static void store_sdu_timing_info(struct iso_datapath_htoc *iso_dp, uint32_t capture_timestamp,
				  uint16_t sdu_seq)
{
	struct sdu_timing_info info = {
		.seq_num = sdu_seq,
		.capture_timestamp = capture_timestamp,
	};

	int ret = k_msgq_put(&iso_dp->sdu_timing_msgq, &info, K_NO_WAIT);

	if (ret) {
		LOG_ERR("Failed to put SDU timing to msgq, err %d", ret);
	}
}

static int get_sdu_timing(struct iso_datapath_htoc *iso_dp, uint16_t sdu_seq,
			  struct sdu_timing_info *info)
{
	/* Loop through SDU queue until we either find a matching SDU or we know that the matching
	 * SDU does not exist in the queue
	 */
	while (1) {
		int ret = k_msgq_peek(&iso_dp->sdu_timing_msgq, info);

		if (ret) {
			/* No messages remaining in msgq. Timing info cannot be found */
			LOG_WRN("SDU timing info cannot be found (no messages)");
			return -ENOMSG;
		}

		if (info->seq_num == sdu_seq) {
			/* Matching SDU timing info is found. Pop from queue and return */
			k_msgq_get(&iso_dp->sdu_timing_msgq, info, K_NO_WAIT);
			return 0;
		}

		if (((int32_t)sdu_seq) - info->seq_num > 0) {
			/* Timing info matches a previous SDU. Pop from queue and move on to next */
			k_msgq_get(&iso_dp->sdu_timing_msgq, info, K_NO_WAIT);
		} else {
			/* Timing info matches a future SDU, stop traversing SDU timing queue */
			LOG_WRN("SDU timing info cannot be found");
			return -ENOMSG;
		}
	}
}

void iso_datapath_htoc_notify_sdu_available(void *datapath, uint32_t capture_timestamp,
					    uint16_t sdu_seq)
{
	if (datapath == NULL) {
		LOG_ERR("null datapath");
		return;
	}

	struct iso_datapath_htoc *iso_dp = (struct iso_datapath_htoc *)datapath;

	if (iso_dp->awaiting_sdu) {
		iso_dp->awaiting_sdu = false;
		send_next_sdu(iso_dp);
	}

	if (!iso_dp->timing_master_channel) {
		/* Timing is controlled by another channel, so no action to take on SDU timing info
		 */
		return;
	}

	/* Store timing info of the SDU that was just enqueued */
	store_sdu_timing_info(iso_dp, capture_timestamp, sdu_seq);

	/* Get timing info of the last SDU that was sent by controller */
	gapi_isooshm_sdu_sync_t sync_info;
	uint16_t err = gapi_isooshm_dp_get_sync(&iso_dp->dp, &sync_info);

	if (err) {
		/* No SDU has been processed by the controller yet */
		return;
	}

	if (sync_info.seq_num == iso_dp->last_sdu_seq) {
		/* Timing info has already been processed for this SDU */
		return;
	}

	iso_dp->last_sdu_seq = sync_info.seq_num;

	struct sdu_timing_info capture_info;
	int ret = get_sdu_timing(iso_dp, sync_info.seq_num, &capture_info);

	if (ret) {
		/* Timing info not found for this SDU */
		return;
	}

	/* LOG_INF("Successful sdu"); */
	/* uint32_t presentation_delay = sync_info.sdu_anchor - capture_info.capture_timestamp; */

	/* presentation_compensation_notify_timing(presentation_delay); */
}

int iso_datapath_htoc_delete(struct iso_datapath_htoc *datapath)
{
	if (datapath == NULL) {
		return -EINVAL;
	}

	if (datapath->sdu_timing_msgq_buffer != NULL) {
		free(datapath->sdu_timing_msgq_buffer);
	}

	gapi_isooshm_dp_unbind(&datapath->dp, NULL);
	free(datapath);

	return 0;
}
